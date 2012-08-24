#pragma once
#include "EncoderInterface.h"
#include "pthread.h"

class Collector
{
public:
	int mFrameCount;
	int mEncoderNumber;
	int mFramesPerIdr;
	EncoderInterface **mEncoders;
	pthread_t mThread;
	FILE *mOutputFile;
public:
	Collector(void);
	~Collector(void);

	int init(Configure *cfg);
	void setEncoders(EncoderInterface **encoders);
	int startCollecting();
};
