#include <stdlib.h>
#include <stdio.h>
#include "Configure.h"
#include "Transcoder.h"

#define FRAME_NUM_PER_IDR 100

int main(int argc, char* argv[])
{
	int ret = 0;
	if (argc != 3) {
		printf("Usage: %s <input_file> <output_file>\n", argv[0]);
		exit(1);
	}

	Configure cfg;
	cfg.encoderNumber = 3;
	cfg.inputFile = argv[1];
	cfg.outputFile = argv[2];
	cfg.framesPerIdr = FRAME_NUM_PER_IDR;
	cfg.frameNumber = -1; // No limit.

	Transcoder transcoder;
	transcoder.mConfigure = &cfg;
	ret = transcoder.prepare();
	if (ret != 0) {
		printf("prepare failed! ret:%d\n", ret);
	}

	ret = transcoder.work();

	if (ret != 0) {
		printf("can not start up! ret : %d\n", ret);
		return -1;
	}

	transcoder.clean();
	return 0;
}
