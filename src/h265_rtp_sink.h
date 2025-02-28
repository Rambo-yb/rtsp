#ifndef __H265_RTP_SINK_H__
#define __H265_RTP_SINK_H__

#include "net/RtpSink.h"

class H265RtpSink : public RtpSink
{
public:
    static H265RtpSink* createNew(UsageEnvironment* env, MediaSource* mediaSource);
    
    H265RtpSink(UsageEnvironment* env, MediaSource* mediaSource);
    virtual ~H265RtpSink();

    virtual std::string getMediaDescription(uint16_t port);
    virtual std::string getAttribute();
    virtual void handleFrame(AVFrame* frame);

private:
    RtpPacket mRtpPacket;
    int mClockRate;
    int mFps;

};

#endif //__H265_RTP_SINK_H__