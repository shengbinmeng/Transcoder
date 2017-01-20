#include "Collector.h"

Collector::Collector(void)
{
	mFrameCount = 0;
	mEncoderNumber = 0;
	mFramesPerIdr = 0;
	mEncoders = NULL;
	mOutputFile = NULL;
}

Collector::~Collector(void)
{
}

int  Collector::init(Configure* cfg)
{
	mEncoderNumber = cfg->encoderNumber;
	mFramesPerIdr = cfg->framesPerIdr;
	mOutputFile = fopen(cfg->outputFile, "wb");
	if (mOutputFile == NULL) {
		printf("open output file failed! \n");
		return -1;
	}
	return 0;
}

void Collector::setEncoders(EncoderInterface **encoders)
{
	mEncoders = encoders;
}


int Collector::collect()
{
	int eos = 0, eosCount = 0, idrCount = 0, encoderIdx = 0;
	int maxSize = 1<<16;
	uint8_t *buffer = (uint8_t*) malloc(maxSize);
	while (mRunning) {
		encoderIdx = idrCount % mEncoderNumber;
		EncoderInterface *encoder = mEncoders[encoderIdx];
		int dataSize = encoder->outputBitsOfOneFrame(buffer, maxSize, &eos);
		if (dataSize > 0) {
			fwrite(buffer, dataSize, 1, mOutputFile);
			mFrameCount++;
			if (mFrameCount == mFramesPerIdr) {
				idrCount++;
			}
		} else {
			Sleep(10);
		}

		if (eos == 1) {
			eosCount++;
			if (eosCount == mEncoderNumber) {
				// All encoders have given eos.
				break;
			}
			idrCount++;
			eos = 0;
		}
	}

	fclose(mOutputFile);

	return 0;
}

int Collector::startCollecting()
{
	int ret = pthread_create(&mThread, NULL, threadEntry, this);
    if (ret != 0) {
        printf("create collecting thread failed, return %d", ret);
        return -1;
    }
	return 0;
}

void* Collector::threadEntry(void* ptr)
{
	Collector *c = (Collector*) ptr;
	c->mRunning = 1;
	c->collect();
	c->mRunning = 0;

	return NULL;
}

int Collector::finishCollecting()
{
	if (mRunning == 0) {
		// already stop
		return 0;
	}
	return pthread_join(mThread, NULL); // return 0 if success
}
