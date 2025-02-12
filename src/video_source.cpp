#include <pthread.h>
#include <string.h>
#include <queue>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "video_source.h"
#include "log.h"

#include "base/Logging.h"
#include "base/New.h"

#define VIDEO_SOURCE_CHN_MAX (4)
#define VIDEO_SOURCE_STREAM_TYPE_MAX (3)

#define VIDEO_SOURCE_SIZE_MAX (512*1024)
#define VIDEO_SOURCE_QUEUE_MAX (2)

typedef struct {
	unsigned char video_src[VIDEO_SOURCE_SIZE_MAX];
	unsigned int size;
}VideoSourceInfo;

typedef struct {
	pthread_mutex_t mutex;
	std::queue<VideoSourceInfo> src_info[VIDEO_SOURCE_CHN_MAX][VIDEO_SOURCE_STREAM_TYPE_MAX];
}VideoSourceMng;
static VideoSourceMng kVideoSourceMng = {.mutex = PTHREAD_MUTEX_INITIALIZER};

int VideoSourcePush(int chn, int type, unsigned char* pkt, unsigned int size) {
	pthread_mutex_lock(&kVideoSourceMng.mutex);
	if (kVideoSourceMng.src_info[chn][type].size() >= VIDEO_SOURCE_QUEUE_MAX) {
		VideoSourceInfo info = kVideoSourceMng.src_info[chn][type].front();
		kVideoSourceMng.src_info[chn][type].pop();
		LOG_WRN("chn[%d] type[%d] video queue greater than queue max [%d], remove old video src !!!", chn, type, VIDEO_SOURCE_QUEUE_MAX);
	}

	VideoSourceInfo new_src;
	memcpy(new_src.video_src, pkt, size);
	new_src.size = size;
	kVideoSourceMng.src_info[chn][type].push(new_src);

	pthread_mutex_unlock(&kVideoSourceMng.mutex);
	return 0;
}

int VideoSourcePop(int chn, int type, unsigned char* pkt, unsigned int size) {
	pthread_mutex_lock(&kVideoSourceMng.mutex);
	if (kVideoSourceMng.src_info[chn][type].empty()) {
		pthread_mutex_unlock(&kVideoSourceMng.mutex);
		return -1;
	}

	VideoSourceInfo info = kVideoSourceMng.src_info[chn][type].front();
	if (size < info.size) {
		pthread_mutex_unlock(&kVideoSourceMng.mutex);
		return -1;
	}
	memcpy(pkt, info.video_src, info.size);
	kVideoSourceMng.src_info[chn][type].pop();

	pthread_mutex_unlock(&kVideoSourceMng.mutex);
	return info.size;
}


VideoSource* VideoSource::createNew(UsageEnvironment* env, int chn, int type) {
	return New<VideoSource>::allocate(env, chn, type);
}

VideoSource::VideoSource(UsageEnvironment* env, int chn, int type) : MediaSource(env) {
	setFps(25);

	channel = chn;
	stream_type = type; 

	for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
		mEnv->threadPool()->addTask(mTask);
}

VideoSource::~VideoSource() {

}

void VideoSource::readFrame() {
	MutexLockGuard mutexLockGuard(mMutex);

	if(mAVFrameInputQueue.empty())
		return;

	AVFrame* frame = mAVFrameInputQueue.front();

	frame->mFrameSize = VideoSourcePop(channel, stream_type, frame->mBuffer, FRAME_MAX_SIZE);
	if(frame->mFrameSize < 0)
		return;

	if(frame->mBuffer[0] == 0 && frame->mBuffer[1] == 0 && frame->mBuffer[2] == 1) {
		frame->mFrame = frame->mBuffer+3;
		frame->mFrameSize -= 3;
	} else {
		frame->mFrame = frame->mBuffer+4;
		frame->mFrameSize -= 4;
	}

	mAVFrameInputQueue.pop();
	mAVFrameOutputQueue.push(frame);
}
