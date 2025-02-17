#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

int VideoSourceInit();

int VideoSourceUninit();

int VideoSourcePush(int chn, int type, unsigned char* pkt, unsigned int size);

int VideoSourcePop(int chn, int type, unsigned char* pkt, unsigned int size);


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