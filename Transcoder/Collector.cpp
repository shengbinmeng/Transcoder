#include "Collector.h"

Collector::Collector(void)
{
	mFrameCount = 0;
	mEncoderNumber = 0;
	mFramesPerIdr = 0;
	mEncoders = NULL;
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


void* collect (void* arg)
{
	Collector *c = (Collector*) arg;
	FILE *outputFile = c->mOutputFile;

	int eos = 0, idrCount = 0, encoderIdx = 0;
	int maxSize = 1<<14;
	uint8_t *buffer = (uint8_t*) malloc(maxSize);
	while (1) {
		idrCount = c->mFrameCount / c->mFramesPerIdr;
		encoderIdx = idrCount % c->mEncoderNumber;
		EncoderInterface *encoder = c->mEncoders[encoderIdx];
		int dataSize = encoder->outputBitsOfOneFrame(buffer, maxSize, &eos);
		if (dataSize > 0) {
			fwrite(buffer, dataSize, 1, outputFile);
		}

		if (eos == 1) {
			break;
		}

		c->mFrameCount ++;
	}
	return NULL;
}

int Collector::startCollecting()
{
	int ret = pthread_create(&mThread, NULL, collect, this);
    if (ret != 0) {
        printf("create decode thread failed, return %d", ret);
        return -1;
    }
}
