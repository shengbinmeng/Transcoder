#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

#include "pthread.h"
#include "decode.h"
#include "encode.h"
#include "framequeue.h"

FrameQueue *frameQueue = NULL;

int main(int argc, char* argv[])
{
	pthread_t					decode_thread;
	pthread_t					encode_thread;
	int ret = 0;
	
	if ( argc != 3 ) {
		printf("Usage: %s <input_file> <output_file> \n", argv[0]);
		return 1;
	}

	frameQueue = new FrameQueue();

	ret = pthread_create(&decode_thread, NULL, decode_entry, argv[1]);
	if (ret != 0) {
		printf("create decode thread failed, return %d \n", ret);
		return 1;
	}

	ret =  pthread_create(&encode_thread, NULL, encode_entry, argv[2]);
		
	pthread_join(decode_thread, NULL);
	pthread_join(encode_thread, NULL);
	return 0;
}
