#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

#include <stdint.h>

int VideoSourcePush(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t pts);

int VideoSourcePop(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t* pts);

#include "net/UsageEnvironment.h"
#include "net/MediaSource.h"
#include "base/ThreadPool.h"

class VideoSource : public MediaSource
{
public:
    static VideoSource* createNew(UsageEnvironment* env, int chn, int type, int fps);
    
    VideoSource(UsageEnvironment* env, int chn, int type, int fps);
    ~VideoSource();

protected:
    virtual void readFrame();
	
private:
	int channel;
	int stream_type;
};

#endif