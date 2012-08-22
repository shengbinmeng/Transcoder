#pragma once
#include "SharedMemory.h"
#include "Configure.h"

class EncoderInterface
{
private:
	SharedMemory *mFrameBuffer;
public:
	EncoderInterface(void);
	~EncoderInterface(void);

	int init(char* fileMappingName, Configure* cfg);
	void receiveOneFrame();
};
