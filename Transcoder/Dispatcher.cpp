#include "Dispatcher.h"
#include "stdio.h"

#define FRAME_NUM_PER_IDR 100
Dispatcher::Dispatcher(void)
{
	mFrameCount = 0;
	mEncoderNumber = 0;
	mEncoders = NULL;
}

Dispatcher::~Dispatcher(void)
{
}


int  Dispatcher::initEncoders(Configure* cfg)
{
	mEncoderNumber = cfg->encoderNumber;
	mEncoders = (EncoderInterface ** )malloc(sizeof(EncoderInterface*) * mEncoderNumber);
	for (int i = 0; i < mEncoderNumber; i++) {
		mEncoders[i] = new EncoderInterface();
		char name[256];
		sprintf(name, "MEM_SHARE_FRAMES_%d", i);
		mEncoders[i]->init(name, cfg);
	}

	return 0;
}

void Dispatcher::dispatch()
{
	int idrCount = mFrameCount / FRAME_NUM_PER_IDR;
	int encoderIdx = idrCount % mEncoderNumber;
	EncoderInterface *encoder = mEncoders[encoderIdx];
	encoder->receiveOneFrame();

	mFrameCount ++;
}
