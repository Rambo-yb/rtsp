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
	void* record_oper_cb;
}RtspServerMsg;
static RtspServerMsg kRtspServerMsg;

extern int RtspServerUrlParse(const char* cmd, const char* uri, char* sess_name, int size) {
	CHECK_POINTER(cmd, return -1);
	CHECK_POINTER(uri, return -1);
	CHECK_POINTER(sess_name, return -1);

	RtspServerRecordInfo rec_info;
	memset(&rec_info, 0, sizeof(RtspServerRecordInfo));

	char buffer[256] = {0};
    strncpy(buffer, uri, sizeof(buffer));

    char *path = strtok(buffer, "?");
	CHECK_POINTER(path, return -1);

    char *query = strtok(NULL, "/");
	CHECK_POINTER(query, return -1);

	char *saveptr = NULL;
	char *param = strtok_r(query, "&", &saveptr);
	while (param != NULL) {
		char *delim = strpbrk(param, "=");
		if (delim != NULL) {
			*delim = '\0';
			char *key = param;
			char *value = delim + 1;

			if (strcmp(key, "auth") == 0) {
				char username[32] = {0};
				char password[64] = {0};
				sscanf(value, "%64[^:]:%s", username, password);
				LOG_DEBUG("username:%s, password:%s", username, password);

				// TODO 用户校验
			} else if (strcmp(key, "record_file") == 0) {
				rec_info.mode = RTSP_SERVER_GET_RECORD_BY_FILE;
				strncpy(rec_info.filename, value, sizeof(rec_info.filename));
			} else if (strcmp(key, "start_time") == 0) {
				rec_info.mode = RTSP_SERVER_GET_RECORD_BY_TIME;
				sscanf(value, "%04d%02d%02dT%02d%02d%02d", 
					&rec_info.start_time.year, &rec_info.start_time.month, &rec_info.start_time.day, 
					&rec_info.start_time.hour, &rec_info.start_time.min, &rec_info.start_time.sec);
			} else if (strcmp(key, "end_time") == 0) {
				sscanf(value, "%04d%02d%02dT%02d%02d%02d", 
					&rec_info.end_time.year, &rec_info.end_time.month, &rec_info.end_time.day, 
					&rec_info.end_time.hour, &rec_info.end_time.min, &rec_info.end_time.sec);
			}
		}
		param = strtok_r(NULL, "&", &saveptr);
	}
	
    char *type = strtok(path, "/");
    char *id = strtok(NULL, "/");
	CHECK_POINTER(type, return -1);
	CHECK_POINTER(id, return -1);

	if(strcmp(type, "recording") == 0) {
		strncpy(sess_name, type, size);

		rec_info.chn = atoi(id) / 100;
		if (kRtspServerMsg.record_oper_cb != NULL && (strcmp(cmd, "play") == 0 || strcmp(cmd, "teardown") == 0)) {
			if (strcmp(cmd, "teardown") == 0) {
				rec_info.mode = RTSP_SERVER_GET_RECORD_STOP;
			}

			((RtspServerOperationRecording)kRtspServerMsg.record_oper_cb)(&rec_info);
		}
	} else {
		snprintf(sess_name, size, "%s/%s", type, id);
	}

	return 0;
}

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

int RtspServerInit(const char* log_path, const char* config_path) {
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

void RtspServerStreamingRegister(RtspServerStreamingRegisterInfo* info, unsigned int size) {
	CHECK_POINTER(info, return);
	CHECK_LE(size, 0, return);

	for (int i = 0; i < size; i++) {
		RtspServerStreamingRegisterInfo _info;
		memcpy(&_info, &info[i], sizeof(RtspServerStreamingRegisterInfo));

		char sess_name[16] = {0};
		if(_info.chn == -1) {
			snprintf(sess_name, sizeof(sess_name), "recording");
		} else {
			snprintf(sess_name, sizeof(sess_name), "streaming/%03d", _info.chn*100+_info.stream_type+1);
		}

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
		LOG_INFO("%s%s?auth=<username>:<password>&<param>", kRtspServerMsg.server->getUrl(session).c_str(), _info.chn == -1 ? "/<chn*100+1>" : "");
		kRtspServerMsg.reg_list.push_back(_info);
	}
}

int RtspServerPushStream(RtspServerPushStreamInfo* info) {
	CHECK_POINTER(info, return -1);

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

void RtspServerOperationRegister(void* cb) {
	CHECK_POINTER(cb, return);
	kRtspServerMsg.record_oper_cb = cb;
}
