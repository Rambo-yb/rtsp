#include "h264_media_session.h"
#include "check_common.h"

#include "H264VideoStreamFramer.hh"
#include "H264VideoRTPSink.hh"

H264MediaSession* H264MediaSession::CreateNew(UsageEnvironment& env, Boolean reuse_first_source) {
	return new H264MediaSession(env, reuse_first_source);
}

H264MediaSession::H264MediaSession(UsageEnvironment& env, Boolean reuse_first_source) : OnDemandServerMediaSubsession(env, reuse_first_source) {

}

H264MediaSession::~H264MediaSession() {

}

static void _AfterPlayingDummy(void *clientData)
{
	H264MediaSession *subsess = (H264MediaSession *)clientData;
	subsess->AfterPlayingDummy();
}

void H264MediaSession::AfterPlayingDummy() {
	envir().taskScheduler().unscheduleDelayedTask(nextTask());
	SetDoneFlag();
}

static void _CheckForAuxSDPLine(void *clientData)
{
	H264MediaSession *subsess = (H264MediaSession *)clientData;
	subsess->CheckForAuxSdpLine();
}

void H264MediaSession::CheckForAuxSdpLine() {
	nextTask() = NULL;

	char const *dasl;
	if (aux_sdp_line != NULL) {
		SetDoneFlag();
	} else if (dummy_rtp_sink != NULL && (dasl = dummy_rtp_sink->auxSDPLine()) != NULL) {
		aux_sdp_line = strDup(dasl);
		dummy_rtp_sink = NULL;
		SetDoneFlag();
	} else if (!done_flag) {
		// try again after a brief delay:
		int delay = 100000; // 100 ms
		nextTask() = envir().taskScheduler().scheduleDelayedTask(delay, (TaskFunc *)_CheckForAuxSDPLine, this);
	}
}

char const *H264MediaSession::GetAuxSDPLine(RTPSink *rtp_sink, FramedSource *input_source) {
	if (aux_sdp_line != NULL) {
		return aux_sdp_line;
	}

	if (dummy_rtp_sink == NULL) {
		dummy_rtp_sink = rtp_sink;
		dummy_rtp_sink->startPlaying(*input_source, _AfterPlayingDummy, this);
		_CheckForAuxSDPLine(this);
	}
	envir().taskScheduler().doEventLoop(&done_flag);

	return aux_sdp_line;
}

FramedSource* H264MediaSession::createNewStreamSource(unsigned client_session_id, unsigned& bitrate) {
	bitrate = 5000; // kbps, estimate

	H264VideoSource* video_source = H264VideoSource::CreateNew(envir());
	if (video_source == NULL) {
		return NULL;
	}

	return H264VideoStreamFramer::createNew(envir(), video_source);
}

RTPSink *H264MediaSession::createNewRTPSink(Groupsock* rtp_groupsock, unsigned char rtp_payload, FramedSource* input_source) {
	return H264VideoRTPSink::createNew(envir(), rtp_groupsock, rtp_payload);
}