#ifndef __H264_MEDIA_SESSION_H__
#define __H264_MEDIA_SESSION_H__

#include "h264_video_source.h"
#include "OnDemandServerMediaSubsession.hh"

class H264MediaSession : public OnDemandServerMediaSubsession {
public:
	static H264MediaSession* CreateNew(UsageEnvironment& env, Boolean reuse_first_source);
	void CheckForAuxSdpLine();
	void AfterPlayingDummy();

	// int H264VideoStreamPush(unsigned char* data, int size);

protected:
	H264MediaSession(UsageEnvironment& env, Boolean reuse_first_source);
	virtual ~H264MediaSession();

	void SetDoneFlag() { done_flag = ~0; };
	virtual char const *GetAuxSDPLine(RTPSink *rtp_sink, FramedSource *input_source);

protected:
	virtual FramedSource* createNewStreamSource(unsigned client_session_id, unsigned& bitrate);
	virtual RTPSink* createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload, FramedSource* input_source);

private:
	char* aux_sdp_line;
	char done_flag;
	RTPSink* dummy_rtp_sink;
};

#endif