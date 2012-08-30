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
	if (eos == 1) {
		//end of stream
		for (int i = 0; i < mEncoderNumber; i++) {
			mEncoders[i]->inputOneFrame(NULL, 1);
		}
		return ;
	}

	int idrCount = mFrameCount / mFramesPerIdr;
	int encoderIdx = idrCount % mEncoderNumber;
	EncoderInterface *encoder = mEncoders[encoderIdx];
	encoder->inputOneFrame(frame, eos);

	mFrameCount ++;
}
