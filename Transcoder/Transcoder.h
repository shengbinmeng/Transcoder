#pragma once
#include "Dispatcher.h"
#include "Configure.h"
class Transcoder
{
public:
	Configure *mConfigure;
	Dispatcher *mDispatcher;
public:
	Transcoder(void);
	~Transcoder(void);
	int initDispatcher();
};
