#include <pthread.h>
#include <string.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "audio_source.h"
#include "log.h"
#include "check_common.h"

#include "base/Logging.h"
#include "base/New.h"

#define VIDEO_SOURCE_CHN_MAX (4)
#define VIDEO_SOURCE_STREAM_TYPE_MAX (3)

#define AUDIO_SOURCE_SIZE_MAX (1024)
#define SOURCE_QUEUE_MAX (2)

typedef struct {
	uint64_t pts;
	unsigned char audio_src[AUDIO_SOURCE_SIZE_MAX];
	unsigned int size;
}AudioSourceInfo;

typedef struct {
	pthread_mutex_t mutex;

	std::queue<AudioSourceInfo> record_audio_src;
	std::queue<AudioSourceInfo> real_audio_src[VIDEO_SOURCE_CHN_MAX][VIDEO_SOURCE_STREAM_TYPE_MAX];
}AudioSourceMng;
static AudioSourceMng kAudioSourceMng = {.mutex = PTHREAD_MUTEX_INITIALIZER};

int AudioSourcePush(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t pts) {
	CHECK_POINTER(pkt, return -1);
	CHECK_LE(size, 0, return -1);
	CHECK_GT(size, AUDIO_SOURCE_SIZE_MAX, return -1);

	pthread_mutex_lock(&kAudioSourceMng.mutex);
	std::queue<AudioSourceInfo>* src_info_queue = nullptr;
	if(chn == -1) {
		src_info_queue = &kAudioSourceMng.record_audio_src;
	} else {
		src_info_queue = &kAudioSourceMng.real_audio_src[chn][type];
	}

	if(src_info_queue->size() >= SOURCE_QUEUE_MAX) {
		AudioSourceInfo info = src_info_queue->front();
		src_info_queue->pop();
		LOG_WRN("chn[%d] type[%d] audio queue greater than queue max [%d], remove old video src !!!", chn, type, SOURCE_QUEUE_MAX);
	}

	AudioSourceInfo new_src;
	memcpy(new_src.audio_src, pkt, size);
	new_src.size = size;
	new_src.pts = pts;
	src_info_queue->push(new_src);

	pthread_mutex_unlock(&kAudioSourceMng.mutex);
	return 0;
}

int AudioSourcePop(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t* pts) {
	pthread_mutex_lock(&kAudioSourceMng.mutex);
	std::queue<AudioSourceInfo>* src_info_queue = nullptr;
	if(chn == -1) {
		src_info_queue = &kAudioSourceMng.record_audio_src;
	} else {
		src_info_queue = &kAudioSourceMng.real_audio_src[chn][type];
	}

	if (src_info_queue->empty()) {
		pthread_mutex_unlock(&kAudioSourceMng.mutex);
		return -1;
	}

	AudioSourceInfo info = src_info_queue->front();
	if (size < info.size) {
		pthread_mutex_unlock(&kAudioSourceMng.mutex);
		return -1;
	}
	memcpy(pkt, info.audio_src, info.size);
	*pts = info.pts;
	src_info_queue->pop();

	pthread_mutex_unlock(&kAudioSourceMng.mutex);
	return info.size;
}

AudioSource* AudioSource::createNew(UsageEnvironment* env, int chn, int type, int fps) {
	return New<AudioSource>::allocate(env, chn, type, fps);
}

AudioSource::AudioSource(UsageEnvironment* env, int chn, int type, int fps) : MediaSource(env) {
	setFps(fps);

	channel = chn;
	stream_type = type; 

	for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
		mEnv->threadPool()->addTask(mTask);
}

AudioSource::~AudioSource() {

}

void AudioSource::readFrame() {
	MutexLockGuard mutexLockGuard(mMutex);

	if(mAVFrameInputQueue.empty())
		return;

	AVFrame* frame = mAVFrameInputQueue.front();

	frame->mFrameSize = AudioSourcePop(channel, stream_type, frame->mBuffer, FRAME_MAX_SIZE, &frame->pts);
	if(frame->mFrameSize < 0)
		return;
    frame->mFrame = frame->mBuffer;

	mAVFrameInputQueue.pop();
	mAVFrameOutputQueue.push(frame);
}
