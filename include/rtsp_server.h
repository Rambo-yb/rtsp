#ifndef __RTSP_SERVER_H__
#define __RTSP_SERVER_H__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RTSP服务器初始化
 * @param [in] log_path 日志路径，为NULL时默认存储/data/logs
 * @return 成功返回 0
 *         失败返回 其他值
*/
int RtspServerInit(const char* log_path);

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
	RTSP_SERVER_VIDEO_H264,
}RtspServerVideoType;

typedef struct {
	int use;					// 信息是否使用
	unsigned int video_type;	// 传输视频格式，RtspServerAudioType
	unsigned int fps;			// 帧率
}RtspServerVideoInfo;

typedef enum {
	RTSP_SERVER_AUDIO_AAC,
}RtspServerAudioType;

typedef struct {
	int use;					// 信息是否使用
	unsigned int audio_type;	// RtspServerAudioType
	unsigned int sample_rate;	// 采样率
	unsigned int channels;		// 通道数，单通道/双通道
}RtspServerAudioInfo;

typedef struct {
	unsigned int chn;					// 音视频通道
	unsigned int stream_type;			// 码流类型，RtspServerStreamingType
	RtspServerVideoInfo video_info;		// 视频信息
	RtspServerAudioInfo audio_info;		// 音频信息，暂不支持
}RtspServerStreamingRegisterInfo;

/**
 * @brief RTSP服务器码流注册函数
 * @param [in] info 码流注册信息
 * @param [in] size 码流注册信息个数
 */
void RtspServerStreamingRegister(RtspServerStreamingRegisterInfo* info, int size);

typedef struct {
	unsigned int chn;			// 音视频通道
	unsigned int stream_type;	// 码流类型，RtspServerStreamingType
	unsigned int pts;
	unsigned int size;
	unsigned char* buff;
}RtspServerPushStreamInfo;

/**
 * @brief RTSP服务器推流函数
 * @param [in] info 码流信息
 * @return 成功返回 0
 *         失败返回 其他值
 */
int RtspServerPushStream(RtspServerPushStreamInfo* info);

#ifdef __cplusplus
}
#endif

#endif