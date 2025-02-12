#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

int RtspServerInit(const char* log_path);

int RtspServerUnInit();

typedef enum {
	RTSP_SERVER_STREAMING_MAIN,
	RTSP_SERVER_STREAMING_SUB,
	RTSP_SERVER_STREAMING_THIRD,
	RTSP_SERVER_STREAMING_MAX,
}RtspServerStreamingType;

void RtspServerStreamingRegister(int chn, int type);

int RtspServerPushStream(int chn, int type, unsigned char* buff, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif