#pragma once
#include "SharedMemory.h"
#include "Configure.h"
#include "libavformat/avformat.h"

class EncoderInterface
{
private:
	SharedMemory *mFrameBuffer;
	Configure *mConfigure;
public:
	EncoderInterface(void);
	~EncoderInterface(void);

	int init(char* fileMappingName, Configure* cfg);
	void receiveOneFrame(AVFrame *frame);
};
