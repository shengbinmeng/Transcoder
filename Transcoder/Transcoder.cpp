#include "Transcoder.h"

Transcoder::Transcoder(void)
{
}

Transcoder::~Transcoder(void)
{
}

int Transcoder::initDispatcher()
{
	return mDispatcher->initEncoders(mConfigure);
}
