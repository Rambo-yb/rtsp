#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTSP服务器初始化
 * @param [in] log_path 日志路径，为NULL时默认存储/data/logs
 * @param [in] config_path 配置路径，为NULL时默认存储/data/configs
 * @return 成功返回 0
 *         失败返回 其他值
*/
int RtspServerInit(const char* log_path, const char* config_path);

/**
 * @brief RTSP服务器反初始化
*/
int RtspServerUnInit();

typedef enum {
	RTSP_SERVER_STREAMING_MAIN,		// 主码流
	RTSP_SERVER_STREAMING_SUB,		// 次码流
	RTSP_SERVER_STREAMING_THIRD,	// 第三视频流
}RtspServerStreamingType;

typedef enum {
	RTSP_SERVER_FRAME_VIDEO,		// 视频帧
	RTSP_SERVER_FRAME_AUDIO,		// 音频帧
}RtspServerFrameType;

typedef enum {
	RTSP_SERVER_VIDEO_H264,
	RTSP_SERVER_VIDEO_H265,
}RtspServerVideoType;

typedef struct {
	int use;					// 信息是否使用
	unsigned int video_type;	// 传输视频格式，RtspServerAudioType
	unsigned int fps;			// 帧率
}RtspServerVideoInfo;

typedef enum {
	RTSP_SERVER_AUDIO_AAC,
	RTSP_SERVER_AUDIO_G711A,
}RtspServerAudioType;

typedef struct {
	int use;					// 信息是否使用
	unsigned int audio_type;	// RtspServerAudioType
	unsigned int sample_rate;	// 采样率
	unsigned int channels;		// 通道数，单通道/双通道
}RtspServerAudioInfo;

typedef struct {
	int chn;							// 音视频通道，【-1：录像回放通道】
	unsigned int stream_type;			// 码流类型，RtspServerStreamingType
	RtspServerVideoInfo video_info;		// 视频信息
	RtspServerAudioInfo audio_info;		// 音频信息
}RtspServerStreamingRegisterInfo;

/**
 * @brief RTSP服务器码流注册函数，录像回放通道只需注册一次
 * @param [in] info 码流注册信息数组
 * @param [in] size 码流注册信息个数
 */
void RtspServerStreamingRegister(RtspServerStreamingRegisterInfo* info, unsigned int size);

typedef struct {
	int chn;					// 音视频通道，【-1：录像回放通道】
	int frame_type;				// 帧类型，RtspServerFrameType
	unsigned int stream_type;	// 码流类型，RtspServerStreamingType
	uint64_t pts;				// 【0：内部计算】【 != 0：使用该值】
	unsigned int size;			// 音频单次传输大小需要小于1400字节
	unsigned char* buff;
}RtspServerPushStreamInfo;

/**
 * @brief RTSP服务器推流函数
 * @param [in] info 码流信息
 * @return 成功返回 0
 *         失败返回 其他值
 */
int RtspServerPushStream(RtspServerPushStreamInfo* info);

typedef enum {
	RTSP_SERVER_GET_RECORD_STOP,	// 结束预览录像
	RTSP_SERVER_GET_RECORD_BY_FILE,	// 通过文件预览录像
	RTSP_SERVER_GET_RECORD_BY_TIME,	// 通过时间预览录像
}RtspServerGetRecordMode;

typedef struct {
	unsigned int year;	// 年
	unsigned int month;	// 月
	unsigned int day;	// 日
	unsigned int hour;	// 时
	unsigned int min;	// 分
	unsigned int sec;	// 秒
}RtspServerTime;

typedef struct {
	int mode;					// RtspServerGetRecordMode

	// RTSP_SERVER_GET_RECORD_BY_FILE
	char filename[128];				// 文件名
	int offset_time;				// 录像偏移时间，单位：秒

	// RTSP_SERVER_GET_RECORD_BY_TIME
	int chn;					// 通道
	RtspServerTime start_time;	// 开始时间
	RtspServerTime end_time;	// 结束时间
}RtspServerRecordInfo;

typedef struct {
	unsigned char reserve;
}RtspServerForceIdr;

typedef enum {
	RTSP_SERVER_OPERATION_RECORDING = 0x10000,		// 录像处理 RtspServerRecordInfo
	RTSP_SERVER_OPERATION_FORCE_IDR,				// 强制I帧 RtspServerForceIdr
}RtspServerOperationType;
typedef int (*RtspServerOperationCb)(int, void* );	// type, st

/**
 * @brief RTSP服务器回调注册函数
 * @param [in] cb 回调函数
 */
void RtspServerOperationRegister(void* cb);

#ifdef __cplusplus
}
#endif

#endif