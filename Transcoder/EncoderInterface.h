#pragma once
extern "C" {
	#include "mem_share.h"
	#include "libavformat/avformat.h"
}
#include "Configure.h"

class EncoderInterface
{
private:
	share_mem_info_t mPicBuffer;
	share_mem_info_t mNalBuffer;
	Configure *mConfigure;
	STARTUPINFO mSI;
	PROCESS_INFORMATION mPI;
public:
	EncoderInterface(void);
	~EncoderInterface(void);

	int startEncoding(Configure* cfg, int index, int totalNumber);
	void inputOneFrame(AVFrame *frame, int eos);
	int outputBitsOfOneFrame(uint8_t *buffer, int maxSize, int *eos);
	int cleanUp();
};
