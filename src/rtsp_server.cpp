#include <pthread.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <list>

#include "log.h"
#include "check_common.h"
#include "rtsp_server.h"
#include "video_source.h"

#include "base/Logging.h"
#include "net/UsageEnvironment.h"
#include "base/ThreadPool.h"
#include "net/EventScheduler.h"
#include "net/Event.h"
#include "net/RtspServer.h"
#include "net/MediaSession.h"
#include "net/InetAddress.h"
#include "net/H264RtpSink.h"
#include "net/AACRtpSink.h"

#define RTSP_LIB_VERSION ("V1.0.0")
#define RTSP_DEFAULT_LOGS_PATH "/data/logs"

typedef struct {
	EventScheduler* scheduler;
	ThreadPool* thread_pool;
	UsageEnvironment* env;
	RtspServer* server;
	std::list<RtspServerStreamingRegisterInfo> reg_list;
}RtspServerMsg;
static RtspServerMsg kRtspServerMsg;

static void* RtspServerProc(void* arg) {
	kRtspServerMsg.env->scheduler()->loop();
	return NULL;
}

static int RtspServerCreateLogPath(const char* path) {
	char _p[256] = {0};
    strncpy(_p, path, sizeof(_p));
 
    for(int i = 0; i < strlen(_p); i++) {
		if (_p[i] == '/') {
			_p[i] = '\0';
			if (strlen(_p) > 0 && access(_p, F_OK) != 0) {
				mkdir(_p, 0755);
			}
			_p[i] = '/';
		}
    }
 
	if (strlen(_p) > 0 && access(_p, F_OK) != 0) {
		mkdir(_p, 0755);
	}
 
    return 0;
}

static void RtspServerLogInit(const char* log_path) {
	char file_path[256] = {0};
	if (log_path == NULL) {
		if (access(RTSP_DEFAULT_LOGS_PATH, F_OK)) {
			RtspServerCreateLogPath(RTSP_DEFAULT_LOGS_PATH);
		}
    	snprintf(file_path, sizeof(file_path), "%s/rtsp.log", RTSP_DEFAULT_LOGS_PATH);
	} else {
		if (access(log_path, F_OK)) {
			RtspServerCreateLogPath(log_path);
		}
		snprintf(file_path, sizeof(file_path), "%s/rtsp.log", log_path);
	}
	
    LogInit(file_path, 512*1024, 3, 3);
}

int RtspServerInit(const char* log_path) {
	RtspServerLogInit(log_path);

	Logger::setLogLevel(Logger::LogWarning);

	kRtspServerMsg.scheduler = EventScheduler::createNew(EventScheduler::POLLER_SELECT);
	kRtspServerMsg.thread_pool = ThreadPool::createNew(2);
    kRtspServerMsg.env = UsageEnvironment::createNew(kRtspServerMsg.scheduler, kRtspServerMsg.thread_pool);

	Ipv4Address ipAddr("0.0.0.0", 554);
    kRtspServerMsg.server = RtspServer::createNew(kRtspServerMsg.env, ipAddr);
	CHECK_POINTER(kRtspServerMsg.server, return -1);

    kRtspServerMsg.server->start();

	pthread_t pthread_id;
	pthread_create(&pthread_id, NULL, RtspServerProc, NULL);

    LOG_INFO("rtsp server init success! compile time:%s %s, ver:%s", __DATE__, __TIME__, RTSP_LIB_VERSION);
	return 0;
}

int RtspServerUnInit() {

	return 0;
}

void RtspServerStreamingRegister(RtspServerStreamingRegisterInfo* info, int size) {
	for (int i = 0; i < size; i++) {
		RtspServerStreamingRegisterInfo _info;
		memcpy(&_info, &info[i], sizeof(RtspServerStreamingRegisterInfo));

		char sess_name[16] = {0};
		snprintf(sess_name, sizeof(sess_name), "streaming/%03d", _info.chn*100+_info.stream_type+1);
	
		MediaSession* session = MediaSession::createNew(sess_name);
		if (_info.video_info.use) {
			MediaSource* source = VideoSource::createNew(kRtspServerMsg.env, _info.chn, _info.stream_type, _info.video_info.fps);
			RtpSink* sink = NULL;
			if (_info.video_info.video_type == RTSP_SERVER_VIDEO_H264) {
				sink = H264RtpSink::createNew(kRtspServerMsg.env, source);
			}
	
			session->addRtpSink(MediaSession::TrackId0, sink);
		}
	
		if (_info.audio_info.use) {
			// MediaSource* source = VideoSource::createNew(kRtspServerMsg.env, _info.chn, _info.stream_type, _info.video_info.fps);
			MediaSource* source = NULL;
			RtpSink* sink = NULL;
			if (_info.video_info.video_type == RTSP_SERVER_VIDEO_H264) {
				sink = AACRtpSink::createNew(kRtspServerMsg.env, source);
			}
	
			session->addRtpSink(MediaSession::TrackId1, sink);
		}
	
		kRtspServerMsg.server->addMeidaSession(session);
		LOG_INFO("%s", kRtspServerMsg.server->getUrl(session).c_str());
		kRtspServerMsg.reg_list.push_back(_info);
	}
}

void RtspServerRecordingRegister(int chn) {
	char sess_name[16] = {0};
	snprintf(sess_name, sizeof(sess_name), "recording/%03d", chn*100+1);

    MediaSession* session = MediaSession::createNew(sess_name);
    MediaSource* mediaSource = VideoSource::createNew(kRtspServerMsg.env, chn, 1, 30);
    RtpSink* rtpSink = H264RtpSink::createNew(kRtspServerMsg.env, mediaSource);

	session->addRtpSink(MediaSession::TrackId0, rtpSink);
	kRtspServerMsg.server->addMeidaSession(session);
}

int RtspServerPushStream(RtspServerPushStreamInfo* info) {
	int flag = false;
	for(RtspServerStreamingRegisterInfo reg_item : kRtspServerMsg.reg_list) {
		if (reg_item.chn == info->chn && reg_item.stream_type == info->stream_type) {
			flag = true;
			break;
		}
	}

	if (flag) {
		return VideoSourcePush(info->chn, info->stream_type, info->buff, info->size);
	} else {
		LOG_ERR("unknown chn:%d, type:%d", info->chn, info->stream_type);
		return -1;
	}
}