#include <string>
#include <stdio.h>
#include <string.h>

#include "g711a_rtp_sink.h"

#include "base/Logging.h"
#include "base/New.h"

G711aRtpSink* G711aRtpSink::createNew(UsageEnvironment* env, MediaSource* mediaSource, int sampleRate, int channels)
{
    return New<G711aRtpSink>::allocate(env, mediaSource, sampleRate, channels);
}

G711aRtpSink::G711aRtpSink(UsageEnvironment* env, MediaSource* mediaSource, int sampleRate, int channels) :
    RtpSink(env, mediaSource, RTP_PAYLOAD_TYPE_AAC),
    mSampleRate(sampleRate),
    mChannels(channels),
    mFps(mediaSource->getFps())
{
    mMarker = 1;
    start(1000/mFps);
}

G711aRtpSink::~G711aRtpSink()
{

}

std::string G711aRtpSink::getMediaDescription(uint16_t port)
{
    char buf[100] = { 0 };
    sprintf(buf, "m=audio %hu RTP/AVP %d", port, mPayloadType);

    return std::string(buf);
}

std::string G711aRtpSink::getAttribute()
{
    char buf[500] = { 0 };
    sprintf(buf, "a=rtpmap:97 PCMA/%u/%u\r\n", mSampleRate, mChannels);

    return std::string(buf);
}

void G711aRtpSink::handleFrame(AVFrame* frame)
{
    RtpHeader* rtpHeader = mRtpPacket.mRtpHeadr;
	
    memcpy(rtpHeader->payload, frame->mFrame, frame->mFrameSize);
    mRtpPacket.mSize = frame->mFrameSize;

    sendRtpPacket(&mRtpPacket);

    mSeq++;

    /* (1000 / mFps) 表示一帧多少毫秒 */
	if (frame->pts == 0) {
		mTimestamp += mSampleRate * (1000 / mFps) / 1000;
	} else {
		mTimestamp = frame->pts;
	}
}

