#pragma once
#include "EncoderInterface.h"
#define HAVE_STRUCT_TIMESPEC
#include "pthread.h"

class Collector
{
public:
	int mFrameCount;
	int mEncoderNumber;
	int mFramesPerIdr;
	EncoderInterface **mEncoders;
	pthread_t mThread;
	int mRunning;
	FILE *mOutputFile;
public:
	Collector(void);
	~Collector(void);

	int init(Configure *cfg);
	void setEncoders(EncoderInterface **encoders);
	int startCollecting();
	int finishCollecting();
	int collect();
	static void* threadEntry(void* ptr);
};
