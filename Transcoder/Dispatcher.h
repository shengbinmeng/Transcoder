#pragma once
#include "EncoderInterface.h"
#include "Configure.h"
class Dispatcher
{
	int mFrameCount;
	int mEncoderNumber;
	EncoderInterface **mEncoders;
public:
	Dispatcher(void);
	~Dispatcher(void);

	int initEncoders(Configure *cfg);
	void dispatch();
};
