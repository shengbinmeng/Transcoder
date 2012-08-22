#pragma once
#include "EncoderInterface.h"
#include "Configure.h"
class Dispatcher
{
	int mFrameCount;
	int mEncoderNumber;
	int mFramesPerIdr;
	EncoderInterface **mEncoders;
public:
	Dispatcher(void);
	~Dispatcher(void);

	int initEncoders(Configure *cfg);
	void dispatch(AVFrame *frame);
};
