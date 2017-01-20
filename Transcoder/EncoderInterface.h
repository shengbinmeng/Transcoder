#pragma once

extern "C" {
#include "libavformat/avformat.h"
}
#include "Configure.h"
#include "SharedMemory.h"

class EncoderInterface
{
private:
	SharedMemory mPicBuffer;
	SharedMemory mNalBuffer;

	Configure *mConfigure;
	PROCESS_INFORMATION mPI;
public:
	EncoderInterface(void);
	~EncoderInterface(void);

	int startEncoding(Configure* cfg, int index, int totalNumber);
	void inputOneFrame(AVFrame *frame, int eos);
	int outputBitsOfOneFrame(uint8_t *buffer, int maxSize, int *eos);
	int cleanUp();
};
