#include <pthread.h>
#include <iostream>
#include "log.h"
#include "check_common.h"
#include "rtsp_server.h"
#include "h264_media_session.h"
#include "h264_video_source.h"
#include "video_source.h"

#include "base/Logging.h"
#include "net/UsageEnvironment.h"
#include "base/ThreadPool.h"
#include "net/EventScheduler.h"
#include "net/Event.h"
#include "net/RtspServer.h"
#include "net/MediaSession.h"
#include "net/InetAddress.h"
#include "net/H264FileMediaSource.h"
#include "net/H264RtpSink.h"

typedef struct {
	EventScheduler* scheduler;
	ThreadPool* thread_pool;
	UsageEnvironment* env;
	RtspServer* server;
}RtspServerMsg;
static RtspServerMsg kRtspServerMsg;

static void* Proc(void* arg) {
	kRtspServerMsg.env->scheduler()->loop();
	return NULL;
}

int RtspServerInit() {
	log_init("rtsp.log", 512*1024, 3, 3);

	Logger::setLogLevel(Logger::LogWarning);

	kRtspServerMsg.scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
	kRtspServerMsg.thread_pool = ThreadPool::createNew(2);
    kRtspServerMsg.env = UsageEnvironment::createNew(kRtspServerMsg.scheduler, kRtspServerMsg.thread_pool);

	Ipv4Address ipAddr("0.0.0.0", 554);
    kRtspServerMsg.server = RtspServer::createNew(kRtspServerMsg.env, ipAddr);
	CHECK_POINTER(kRtspServerMsg.server, return -1);

    MediaSession* session = MediaSession::createNew("live");
    MediaSource* mediaSource = H264VideoSource::createNew(kRtspServerMsg.env);
    RtpSink* rtpSink = H264RtpSink::createNew(kRtspServerMsg.env, mediaSource);

	session->addRtpSink(MediaSession::TrackId0, rtpSink);
	kRtspServerMsg.server->addMeidaSession(session);
    kRtspServerMsg.server->start();

	LOG_INFO("%s", kRtspServerMsg.server->getUrl(session).c_str());

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