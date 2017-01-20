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

	int init(Configure *cfg);
	void setEncoders(EncoderInterface **encoders);
	void dispatch(AVFrame *frame, int eos);
};
