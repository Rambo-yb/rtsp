#include <pthread.h>
#include <string.h>
#include <queue>

#include "video_source.h"
#include "log.h"

#define VIDEO_SOURCE_CHN_MAX (4)
#define VIDEO_SOURCE_QUEUE_MAX (10)

typedef struct {
	unsigned char* video_src;
	unsigned int size;
}VideoSourceInfo;

typedef struct {
	pthread_mutex_t mutex;

	std::queue<VideoSourceInfo> src_info[VIDEO_SOURCE_CHN_MAX];
}VideoSourceMng;
static VideoSourceMng kVideoSourceMng = {.mutex = PTHREAD_MUTEX_INITIALIZER};

int VideoSourceInit() {
	return 0;
}

int VideoSourceUninit() {
	return 0;
}

int VideoSourcePush(int chn, unsigned char* pkt, unsigned int size) {
	pthread_mutex_lock(&kVideoSourceMng.mutex);

	if (kVideoSourceMng.src_info[chn].size() >= VIDEO_SOURCE_QUEUE_MAX) {
		VideoSourceInfo info = kVideoSourceMng.src_info[chn].front();
		delete[] info.video_src;
		kVideoSourceMng.src_info[chn].pop();
		LOG_WRN("chn[%d] video queue greater than queue max [%d], remove old video src !!!", chn, VIDEO_SOURCE_QUEUE_MAX);
	}

	VideoSourceInfo new_src;
	new_src.video_src = new unsigned char[size+1];
	memcpy(new_src.video_src, pkt, size);
	new_src.size = size;
	kVideoSourceMng.src_info[chn].push(new_src);

	pthread_mutex_unlock(&kVideoSourceMng.mutex);
	return 0;
}

int VideoSourcePop(int chn, unsigned char** pkt, unsigned int* size) {
	pthread_mutex_lock(&kVideoSourceMng.mutex);
	if (kVideoSourceMng.src_info[chn].empty()) {
		pthread_mutex_unlock(&kVideoSourceMng.mutex);
		return -1;
	}

	VideoSourceInfo info = kVideoSourceMng.src_info[chn].front();
	*pkt = info.video_src;
	*size = info.size;
	kVideoSourceMng.src_info[chn].pop();

	pthread_mutex_unlock(&kVideoSourceMng.mutex);
	return 0;
}