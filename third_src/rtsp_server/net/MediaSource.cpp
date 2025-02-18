#include "net/MediaSource.h"
#include "base/Logging.h"
#include "base/New.h"

MediaSource::MediaSource(UsageEnvironment* env) :
    mEnv(env)
{
    mMutex = Mutex::createNew();
    for(int i = 0; i < DEFAULT_FRAME_NUM; ++i)
        mAVFrameInputQueue.push(&mAVFrames[i]);
    
    mTask.setTaskCallback(taskCallback, this);
}

MediaSource::~MediaSource()
{
    //delete mMutex;
    Delete::release(mMutex);
}

AVFrame* MediaSource::getFrame()
{
    MutexLockGuard mutexLockGuard(mMutex);

    if(mAVFrameOutputQueue.empty())
    {
		// 源码修改：队列为空重新加入线程池，否则无法取数据
    	mEnv->threadPool()->addTask(mTask);
        return NULL;
    }

    AVFrame* frame = mAVFrameOutputQueue.front();    
    mAVFrameOutputQueue.pop();

    return frame;
}

void MediaSource::putFrame(AVFrame* frame)
{
    MutexLockGuard mutexLockGuard(mMutex);

    mAVFrameInputQueue.push(frame);
    
    mEnv->threadPool()->addTask(mTask);
}


void MediaSource::taskCallback(void* arg)
{
    MediaSource* source = (MediaSource*)arg;
    source->readFrame();
}
