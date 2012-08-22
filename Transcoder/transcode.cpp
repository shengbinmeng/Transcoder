#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pthread.h"
#include "decode.h"
#include "encode.h"
#include "framequeue.h"
#include "Configure.h"
#include "Transcoder.h"

FrameQueue *frameQueue = NULL;

int main(int argc, char* argv[])
{
	pthread_mutex_t             lock;
    pthread_cond_t				condition;
	pthread_t					decode_thread;
	pthread_t					encode_thread;
	int ret = 0;
	
	if ( argc != 3 ) {
		printf("Usage: %s <input_file> <output_file>\n", argv[0]);
		return 1;
	}

	Configure cfg;
	cfg.encoderNumber = 3;
	cfg.inputFile = argv[1];
	cfg.outputFile = argv[2];

	Transcoder transcoder;
	transcoder.mConfigure = &cfg;
	frameQueue = new FrameQueue();

	ret = pthread_create(&decode_thread, NULL, decode_entry, &transcoder);
	if (ret != 0) {
		printf("create decode thread failed, return %d", ret);
		return 1;
	}

	ret =  pthread_create(&encode_thread, NULL, encode_entry, &transcoder);
		
	pthread_join(decode_thread, NULL);
	pthread_join(encode_thread, NULL);
	return 0;
}
