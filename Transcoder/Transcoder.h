#pragma once
#include "Dispatcher.h"
#include "Configure.h"
#include "Collector.h"
class Transcoder
{
public:
	Configure *mConfigure;
	EncoderInterface **mEncoders;
	Collector *mCollector;
	Dispatcher *mDispatcher;
public:
	Transcoder(void);
	~Transcoder(void);

	int prepare();
	int work();
	int clean();
};
