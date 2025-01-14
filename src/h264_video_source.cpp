#include "h264_video_source.h"
#include "check_common.h"
#include "video_source.h"

H264VideoSource* H264VideoSource::CreateNew(UsageEnvironment& env) {
  return new H264VideoSource(env);
}

H264VideoSource::H264VideoSource(UsageEnvironment& env) : FramedSource(env) {
}

H264VideoSource::~H264VideoSource() {

}

unsigned int H264VideoSource::maxFrameSize() const {
	return 100000;
}

static long GetTime() {
    struct timeval time_;
    memset(&time_, 0, sizeof(struct timeval));

    gettimeofday(&time_, NULL);
    return time_.tv_sec*1000 + time_.tv_usec/1000;
}

void H264VideoSource::doGetNextFrame() {
	static long curtime = GetTime();
	LOG_INFO("send stream time:%ld", GetTime() - curtime);
	curtime = GetTime();

	unsigned char* buff = NULL;
	unsigned int size = 0;
	if (VideoSourcePop(0, &buff, &size) < 0) {
		return ;
	}
	LOG_INFO("size:%d max:%d", size, fMaxSize);

	if (size > fMaxSize) {
		fFrameSize = fMaxSize;
		fNumTruncatedBytes = size - fMaxSize;
	} else {
		fFrameSize = size;
	}

	gettimeofday(&fPresentationTime, NULL);
	memcpy(fTo, buff, fFrameSize);
	delete[] buff;

	FramedSource::afterGetting(this);
}

void H264VideoSource::doStopGettingFrames() {
	
}