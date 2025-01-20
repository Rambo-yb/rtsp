#ifndef __VIDEO_SOURCE_H__
#define __VIDEO_SOURCE_H__

int VideoSourceInit();

int VideoSourceUninit();

int VideoSourcePush(int chn, unsigned char* pkt, unsigned int size);

int VideoSourcePop(int chn, unsigned char* pkt, unsigned int size);

#endif