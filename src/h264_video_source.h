#ifndef __H264_VIDEO_SOURCE__
#define __H264_VIDEO_SOURCE__

#include "net/UsageEnvironment.h"
#include "net/MediaSource.h"
#include "base/ThreadPool.h"

class H264VideoSource : public MediaSource
{
public:
    static H264VideoSource* createNew(UsageEnvironment* env);
    
    H264VideoSource(UsageEnvironment* env);
    ~H264VideoSource();

protected:
    virtual void readFrame();
};

#endif