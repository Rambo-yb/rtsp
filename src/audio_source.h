#ifndef __AUDIO_SOURCE_H__
#define __AUDIO_SOURCE_H__

#include <stdint.h>

int AudioSourcePush(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t pts);

int AudioSourcePop(int chn, int type, unsigned char* pkt, unsigned int size, uint64_t pts);

#include "net/UsageEnvironment.h"
#include "net/MediaSource.h"
#include "base/ThreadPool.h"

class AudioSource : public MediaSource
{
public:
    static AudioSource* createNew(UsageEnvironment* env, int chn, int type, int fps);
    
    AudioSource(UsageEnvironment* env, int chn, int type, int fps);
    ~AudioSource();

protected:
    virtual void readFrame();
	
private:
	int channel;
	int stream_type;
};

#endif