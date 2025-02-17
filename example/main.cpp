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


static long GetTime() {
    struct timeval time_;
    memset(&time_, 0, sizeof(struct timeval));

    gettimeofday(&time_, NULL);
    return time_.tv_sec*1000 + time_.tv_usec/1000;
}

#ifdef CHIP_TYPE_RV1126

#include "rkmedia_api.h"
#include "rkmedia_venc.h"

void StreamPush(MEDIA_BUFFER mb) {
	RtspServerPushStreamInfo info = {0};
	info.chn = 0;
	info.stream_type = RTSP_SERVER_STREAMING_MAIN;
	info.buff = (unsigned char*)RK_MPI_MB_GetPtr(mb);
	info.size = RK_MPI_MB_GetSize(mb);
    RtspServerPushStream(&info);

	RK_MPI_MB_ReleaseBuffer(mb);
}

int StreamInit() {
    int ret = 0;
    const char* device_name = "/dev/video45";
    int cam_id = 0;
    int width = 640;
    int height = 480;
    CODEC_TYPE_E codec_type = RK_CODEC_TYPE_H264;

    RK_MPI_SYS_Init();
    VI_CHN_ATTR_S vi_chn_attr;
    vi_chn_attr.pcVideoNode = device_name;
    vi_chn_attr.u32BufCnt = 3;
    vi_chn_attr.u32Width = width;
    vi_chn_attr.u32Height = height;
    vi_chn_attr.enPixFmt = IMAGE_TYPE_YUYV422;
    vi_chn_attr.enBufType = VI_CHN_BUF_TYPE_MMAP;
    vi_chn_attr.enWorkMode = VI_WORK_MODE_NORMAL;
    ret = RK_MPI_VI_SetChnAttr(cam_id, 0, &vi_chn_attr);
    ret |= RK_MPI_VI_EnableChn(cam_id, 0);
    if (ret) {
        printf("ERROR: create VI[0] error! ret=%d\n", ret);
        return -1;
    }

    VENC_CHN_ATTR_S venc_chn_attr;
    memset(&venc_chn_attr, 0, sizeof(venc_chn_attr));
    switch (codec_type) {
    case RK_CODEC_TYPE_H265:
        venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H265;
        venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H265CBR;
        venc_chn_attr.stRcAttr.stH265Cbr.u32Gop = 30;
        venc_chn_attr.stRcAttr.stH265Cbr.u32BitRate = width * height;
        // frame rate: in 30/1, out 30/1.
        venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateDen = 1;
        venc_chn_attr.stRcAttr.stH265Cbr.fr32DstFrameRateNum = 30;
        venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateDen = 1;
        venc_chn_attr.stRcAttr.stH265Cbr.u32SrcFrameRateNum = 30;
        break;
    case RK_CODEC_TYPE_H264:
    default:
        venc_chn_attr.stVencAttr.enType = RK_CODEC_TYPE_H264;
        venc_chn_attr.stRcAttr.enRcMode = VENC_RC_MODE_H264CBR;
        venc_chn_attr.stRcAttr.stH264Cbr.u32Gop = 30;
        venc_chn_attr.stRcAttr.stH264Cbr.u32BitRate = width * height * 10;
        // frame rate: in 30/1, out 30/1.
        venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateDen = 1;
        venc_chn_attr.stRcAttr.stH264Cbr.fr32DstFrameRateNum = 30;
        venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateDen = 1;
        venc_chn_attr.stRcAttr.stH264Cbr.u32SrcFrameRateNum = 30;
        break;
    }
    venc_chn_attr.stVencAttr.imageType = IMAGE_TYPE_YUYV422;
    venc_chn_attr.stVencAttr.u32PicWidth = width;
    venc_chn_attr.stVencAttr.u32PicHeight = height;
    venc_chn_attr.stVencAttr.u32VirWidth = width;
    venc_chn_attr.stVencAttr.u32VirHeight = height;
    venc_chn_attr.stVencAttr.u32Profile = 77;
    ret = RK_MPI_VENC_CreateChn(0, &venc_chn_attr);
    if (ret) {
        printf("ERROR: create VENC[0] error! ret=%d\n", ret);
        return -1;
    }

	MPP_CHN_S stEncChn;
	stEncChn.enModId = RK_ID_VENC;
	stEncChn.s32DevId = 0;
	stEncChn.s32ChnId = 0;
	ret = RK_MPI_SYS_RegisterOutCb(&stEncChn, StreamPush);
	if (ret) {
		printf("ERROR: register output callback for VENC[0] error! ret=%d\n", ret);
		return 0;
	}

    MPP_CHN_S src_chn;
    src_chn.enModId = RK_ID_VI;
    src_chn.s32DevId = 0;
    src_chn.s32ChnId = 0;
    MPP_CHN_S dest_chn;
    dest_chn.enModId = RK_ID_VENC;
    dest_chn.s32DevId = 0;
    dest_chn.s32ChnId = 0;
    ret = RK_MPI_SYS_Bind(&src_chn, &dest_chn);
    if (ret) {
        printf("ERROR: Bind VI[0] and VENC[0] error! ret=%d\n", ret);
        return -1;
    }

    printf("%s initial finish\n", __FUNCTION__);
    return 0;
}

#else
int StreamInit() {
	return 0;
}
#endif

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

static void* RecordPush(void* arg) {
    while(1) {
        for(__Frame frame : kMng.media_list) {
			long cur_time = GetTime();
			RtspServerPushStreamInfo info = {0};
			info.chn = 0;
			info.stream_type = RTSP_SERVER_STREAMING_SUB;
			info.buff = frame.data;
			info.size = frame.size;
            RtspServerPushStream(&info);
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
	RtspServerInit("./");

	RtspServerStreamingRegisterInfo info[2] = {
		{.chn = 0, .stream_type = RTSP_SERVER_STREAMING_MAIN, .video_info = {.use = 1, .video_type = RTSP_SERVER_VIDEO_H264, .fps = 30}},
		{.chn = 0, .stream_type = RTSP_SERVER_STREAMING_SUB, .video_info = {.use = 1, .video_type = RTSP_SERVER_VIDEO_H264, .fps = 30}},
	};
	RtspServerStreamingRegister(info, 2);

	StreamInit();
	RecordInit();

	while (1){ 
		sleep(1);
	}

	return 0;
}