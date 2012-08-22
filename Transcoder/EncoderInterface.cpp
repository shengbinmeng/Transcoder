#include "EncoderInterface.h"

EncoderInterface::EncoderInterface(void)
{
}

EncoderInterface::~EncoderInterface(void)
{
}

int EncoderInterface::init(char* fileMappingName, Configure *cfg)
{
	int unitSize = cfg->height * cfg->width * 3/2;
	int unitCount = cfg->framesPerIdr;
	mFrameBuffer = new SharedMemory();
	mFrameBuffer->init(fileMappingName,true, unitSize, unitCount);
	return 0;
}

void EncoderInterface::receiveOneFrame()
{

}