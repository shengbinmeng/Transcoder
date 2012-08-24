#include "Dispatcher.h"

Dispatcher::Dispatcher(void)
{
	mFrameCount = 0;
	mEncoderNumber = 0;
	mFramesPerIdr = 0;
	mEncoders = NULL;
}

Dispatcher::~Dispatcher(void)
{
}


int  Dispatcher::init(Configure* cfg)
{
	mEncoderNumber = cfg->encoderNumber;
	mFramesPerIdr = cfg->framesPerIdr;

	return 0;
}

void Dispatcher::setEncoders(EncoderInterface **encoders)
{
	mEncoders = encoders;
}

void Dispatcher::dispatch(AVFrame *frame, int eos)
{
	int idrCount = mFrameCount / mFramesPerIdr;
	int encoderIdx = idrCount % mEncoderNumber;
	EncoderInterface *encoder = mEncoders[encoderIdx];
	encoder->inputOneFrame(frame, eos);

	mFrameCount ++;
}
