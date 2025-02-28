#ifndef __G711A_RTP_SINK_H__
#define __G711A_RTP_SINK_H__

#include "net/UsageEnvironment.h"
#include "net/RtpSink.h"
#include "net/MediaSource.h"

class G711aRtpSink : public RtpSink
{
public:
    static G711aRtpSink* createNew(UsageEnvironment* env, MediaSource* mediaSource, int sampleRate, int channels);;
    
    G711aRtpSink(UsageEnvironment* env, MediaSource* mediaSource, int sampleRate, int channels);
    virtual ~G711aRtpSink();

    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();

protected:
    virtual void handleFrame(AVFrame* frame);

private:
    RtpPacket mRtpPacket;
    uint32_t mSampleRate;   // 采样频率
    uint32_t mChannels;         // 通道数
    int mFps;
};

#endif //_AAC_RTP_SINK_H_