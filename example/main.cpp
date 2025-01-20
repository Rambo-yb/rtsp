#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <list>

#include "rtsp_server.h"

typedef struct {
    int size;
    unsigned char* data;
}__Frame;

typedef struct {
	unsigned char* media_data;
	std::list<__Frame> media_list;
}Mng;
static Mng kMng;

static unsigned char* FindFrame(const unsigned char* buff, int len, int* size) {
	unsigned char *s = NULL;
	while (len >= 3) {
		if (buff[0] == 0 && buff[1] == 0 && buff[2] == 1 && (buff[3] == 0x41 || buff[3] == 0x67)) {
			if (!s) {
				if (len < 4)
					return NULL;
				s = (unsigned char *)buff;
			} else {
				*size = (buff - s);
				return s;
			}
			buff += 3;
			len  -= 3;
			continue;
		}
		if (len >= 4 && buff[0] == 0 && buff[1] == 0 && buff[2] == 0 && buff[3] == 1 && (buff[4] == 0x41 || buff[4] == 0x67)) {
			if (!s) {
				if (len < 5)
					return NULL;
				s = (unsigned char *)buff;
			} else {
				*size = (buff - s);
				return s;
			}
			buff += 4;
			len  -= 4;
			continue;
		}
		buff ++;
		len --;
	}
	if (!s)
		return NULL;
	*size = (buff - s + len);
	return s;
}

static long GetTime() {
    struct timeval time_;
    memset(&time_, 0, sizeof(struct timeval));

    gettimeofday(&time_, NULL);
    return time_.tv_sec*1000 + time_.tv_usec/1000;
}

static void* RecordPush(void* arg) {
    while(1) {
        for(__Frame frame : kMng.media_list) {
			long cur_time = GetTime();
            RtspServerPushStream(frame.data, frame.size);
			while(cur_time + 40 > GetTime()) {
            	usleep(1*1000);
			}
        }
    }
}

static int RecordInit() {
    FILE* fp = fopen("recordfile01.h264", "r");
    if (fp == NULL) {
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    int size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    kMng.media_data = (unsigned char*)malloc(size+1);
    if (kMng.media_data == NULL) {
        fclose(fp);
        return -1;
    }
    memset(kMng.media_data, 0, size+1);
    int ret = fread(kMng.media_data, 1, size, fp);
    if (ret != size) {
        printf("fread fail ,size:%d-%d\n", ret, size);
        fclose(fp);
        free(kMng.media_data);
        return -1;
    }

    fclose(fp);

    int start = 0;
    while(start < size) {
        __Frame frame;
        frame.data = FindFrame(kMng.media_data+start, size-start, &frame.size);
        if (frame.data == NULL) {
            break;
        }

        kMng.media_list.push_back(frame);
        start = frame.data - kMng.media_data + frame.size;
    }

    printf("media size:%d len:%d\n", kMng.media_list.size(), size);

    pthread_t thread_id;
    pthread_create(&thread_id, NULL, RecordPush, NULL);
    return 0;
}

int main(int argv, char** argc) {
	RtspServerInit();
	RecordInit();

	while (1){ 
		sleep(1);
	}

	return 0;
}