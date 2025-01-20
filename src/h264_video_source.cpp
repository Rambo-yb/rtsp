#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#include "h264_video_source.h"
#include "video_source.h"
#include "log.h"

#include "base/Logging.h"
#include "base/New.h"

H264VideoSource* H264VideoSource::createNew(UsageEnvironment* env) {
	return New<H264VideoSource>::allocate(env);
}

H264VideoSource::H264VideoSource(UsageEnvironment* env) : MediaSource(env) {
	setFps(25);

	for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
		mEnv->threadPool()->addTask(mTask);
}

H264VideoSource::~H264VideoSource() {

}

void H264VideoSource::readFrame() {
	MutexLockGuard mutexLockGuard(mMutex);

	if(mAVFrameInputQueue.empty())
		return;

	AVFrame* frame = mAVFrameInputQueue.front();

	frame->mFrameSize = VideoSourcePop(0, frame->mBuffer, FRAME_MAX_SIZE);
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