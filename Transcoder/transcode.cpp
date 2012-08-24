#include <stdlib.h>
#include <stdio.h>
#include "Configure.h"
#include "Transcoder.h"

#define FRAME_NUM_PER_IDR 100

int main(int argc, char* argv[])
{
	int ret = 0;
	if ( argc != 3 ) {
		printf("Usage: %s <input_file> <output_file>\n", argv[0]);
		return 1;
	}

	Configure cfg;
	cfg.encoderNumber = 3;
	cfg.inputFile = argv[1];
	cfg.outputFile = argv[2];
	cfg.framesPerIdr = FRAME_NUM_PER_IDR;

	Transcoder transcoder;
	transcoder.mConfigure = &cfg;
	transcoder.startUp();

	return 0;
}
