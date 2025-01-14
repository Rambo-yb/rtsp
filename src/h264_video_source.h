#ifndef __H264_VIDEO_SOURCE__
#define __H264_VIDEO_SOURCE__

#include "FramedSource.hh"

class H264VideoSource : public FramedSource {
public:
	static H264VideoSource* CreateNew(UsageEnvironment& env);
	unsigned int maxFrameSize() const;

protected:
	H264VideoSource(UsageEnvironment& env);
	~H264VideoSource();

private:
	virtual void doGetNextFrame();
	virtual void doStopGettingFrames();

};

#endif