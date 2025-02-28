cmake_minimum_required(VERSION 2.8)
project(rtsp)

add_definitions(-DUSE_EASYLOGGER)
#set(CMAKE_POSITION_INDEPENDENT_CODE ON)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/example MAIN_SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/src SRC)

aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/general_src/common SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/general_src/list SRC)

#aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_src/cjson SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_src/easylogger SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_src/rtsp_server/net SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_src/rtsp_server/net/poller SRC)
aux_source_directory(${CMAKE_CURRENT_SOURCE_DIR}/third_src/rtsp_server/base SRC)


include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/general_src/common)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/general_src/list)

#include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_src/cjson)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_src/easylogger)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/third_src/rtsp_server)


# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/live/include/liveMedia)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/live/include/groupsock)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/live/include/BasicUsageEnvironment)
# include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/live/include/UsageEnvironment)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/live/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/openssl/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/openssl/lib)

if (CHIP_TYPE STREQUAL "rv1126")
add_definitions(-DCHIP_TYPE_RV1126)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/example/rv1126/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/example/rv1126/lib)
endif()

if (MEDIA_TYPE STREQUAL "ffmpeg")
add_definitions(-DFFMPEG_ENABLE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/x264/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/x264/lib)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/ffmpeg/include)
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/_install/ffmpeg/lib)
endif()

add_library(rtsp SHARED ${SRC})

add_executable(rtsp_test ${MAIN_SRC})
target_link_libraries(rtsp_test rtsp pthread m dl resolv rt)
target_link_libraries(rtsp_test ssl crypto)
# target_link_libraries(rtsp_test liveMedia groupsock BasicUsageEnvironment UsageEnvironment ssl crypto)

if (CHIP_TYPE STREQUAL "rv1126")
target_link_libraries(rtsp_test easymedia RKAP_3A RKAP_ANR RKAP_Common rknn_runtime rknn_api md_share rkaiq VSC ArchModelSw NNArchPerf rockx rockface drm)
target_link_libraries(rtsp_test rockchip_mpp liveMedia groupsock BasicUsageEnvironment UsageEnvironment asound v4l2 v4lconvert rga sqlite3 od_share OpenVX GAL)
endif()

if (MEDIA_TYPE STREQUAL "ffmpeg")
target_link_libraries(rtsp_test avdevice avfilter avcodec avformat avutil postproc swresample swscale x264)
endif()

install(TARGETS rtsp_test DESTINATION bin)
install(TARGETS rtsp DESTINATION lib)
install(DIRECTORY include/ DESTINATION include)
