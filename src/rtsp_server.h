#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

int RtspServerInit();

int RtspServerUnInit();

int RtspServerPushStream(unsigned char* buff, unsigned int size);

#ifdef __cplusplus
}
#endif

#endif