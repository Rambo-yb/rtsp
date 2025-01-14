#include <pthread.h>

#include "log.h"
#include "check_common.h"
#include "rtsp_server.h"
#include "h264_media_session.h"
#include "video_source.h"

#include "liveMedia.hh"
#include "BasicUsageEnvironment.hh"

typedef struct {
	TaskScheduler* scheduler;
	UsageEnvironment* env;
	RTSPServer* server;
}RtspServerMsg;
static RtspServerMsg kRtspServerMsg;

static void* Proc(void* arg) {
	kRtspServerMsg.env->taskScheduler().doEventLoop();
	return NULL;
}

int RtspServerInit() {
	log_init("rtsp.log", 512*1024, 3, 3);

	kRtspServerMsg.scheduler = BasicTaskScheduler::createNew();
    kRtspServerMsg.env = BasicUsageEnvironment::createNew(*kRtspServerMsg.scheduler);

	kRtspServerMsg.server = RTSPServer::createNew(*kRtspServerMsg.env, 554);
	CHECK_POINTER(kRtspServerMsg.server, return -1);

	ServerMediaSession* sms = ServerMediaSession::createNew(*kRtspServerMsg.env, "stream", "Stream", "Session streamed by LIVE555 RTSP server");
	sms->addSubsession(H264MediaSession::CreateNew(*kRtspServerMsg.env, true));
	kRtspServerMsg.server->addServerMediaSession(sms);
	char* url = kRtspServerMsg.server->rtspURL(sms);
	LOG_INFO("%s", url);
	delete[] url;

	pthread_t pthread_id;
	pthread_create(&pthread_id, NULL, Proc, NULL);

	return 0;
}

int RtspServerUnInit() {
	return 0;
}

int RtspServerPushStream(unsigned char* buff, unsigned int size) {
	
	return VideoSourcePush(0, buff, size);
}