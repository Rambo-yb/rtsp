#include <stdio.h>
#include <string.h>

#include "h265_rtp_sink.h"

#include "base/Logging.h"
#include "base/New.h"

H265RtpSink* H265RtpSink::createNew(UsageEnvironment* env, MediaSource* mediaSource)
{
    if(!mediaSource)
        return NULL;

    return New<H265RtpSink>::allocate(env, mediaSource);
}

H265RtpSink::H265RtpSink(UsageEnvironment* env, MediaSource* mediaSource) :
    RtpSink(env, mediaSource, RTP_PAYLOAD_TYPE_H264),
    mClockRate(90000),
    mFps(mediaSource->getFps())
{
    start(1000/mFps);
}

H265RtpSink::~H265RtpSink()
{

}

std::string H265RtpSink::getMediaDescription(uint16_t port)
{
    char buf[100] = {0};
    sprintf(buf, "m=video %hu RTP/AVP %d", port, mPayloadType);

    return std::string(buf);
}

std::string H265RtpSink::getAttribute()
{
    char buf[100];
    sprintf(buf, "a=rtpmap:%d H265/%d\r\n", mPayloadType, mClockRate);
    sprintf(buf+strlen(buf), "a=framerate:%d", mFps);

    return std::string(buf);
}

void H265RtpSink::handleFrame(AVFrame* frame)
{
    RtpHeader* rtpHeader = mRtpPacket.mRtpHeadr;
    uint8_t naluType[2] = {frame->mFrame[0], frame->mFrame[1]};

    if(frame->mFrameSize <= RTP_MAX_PKT_SIZE)
    {
        memcpy(rtpHeader->payload, frame->mFrame, frame->mFrameSize);
        mRtpPacket.mSize = frame->mFrameSize;
        sendRtpPacket(&mRtpPacket);
        mSeq++;

        if (((naluType[0] & 0x7e) >> 1) == 33 || ((naluType[0] & 0x7e) >> 1) == 34) // 如果是SPS、PPS就不需要加时间戳
            return;
    }
    else
    {
        int pktNum = frame->mFrameSize / RTP_MAX_PKT_SIZE;       // 有几个完整的包
        int remainPktSize = frame->mFrameSize % RTP_MAX_PKT_SIZE; // 剩余不完整包的大小
        int i, pos = 2;

        /* 发送完整的包 */
        for (i = 0; i < pktNum; i++)
        {
            rtpHeader->payload[0] = (naluType[0] & 0x81) | (49 << 1);
            rtpHeader->payload[1] = naluType[1];
            rtpHeader->payload[2] = (naluType[0] >> 1) & 0x3f;
            
            if (i == 0) //第一包数据
                rtpHeader->payload[2] |= 0x80; // start
            else if (remainPktSize == 0 && i == pktNum - 1) //最后一包数据
                rtpHeader->payload[2] |= 0x40; // end

            memcpy(rtpHeader->payload+3, frame->mFrame+pos, RTP_MAX_PKT_SIZE);
            mRtpPacket.mSize = RTP_MAX_PKT_SIZE+3;
            sendRtpPacket(&mRtpPacket);

            mSeq++;
            pos += RTP_MAX_PKT_SIZE;
        }

        /* 发送剩余的数据 */
        if (remainPktSize > 0)
        {
            rtpHeader->payload[0] = (naluType[0] & 0x81) | (49 << 1);
            rtpHeader->payload[1] = naluType[1];
            rtpHeader->payload[2] = (naluType[0] >> 1) & 0x3f;
			rtpHeader->payload[2] |= 0x40; // end

            memcpy(rtpHeader->payload+3, frame->mFrame+pos, remainPktSize);
            mRtpPacket.mSize = remainPktSize+3;
            sendRtpPacket(&mRtpPacket);

            mSeq++;
        }
    }
    
	if(frame->pts == 0) {
		mTimestamp += mClockRate/mFps;
	} else {
		mTimestamp = frame->pts;
	}
}
