#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <stdint.h>
extern "C" {
#include "libavformat/avformat.h"
}
#include "utility.h"
#include "decode.h"
#include "FrameQueue.h"
#include "Transcoder.h"

extern FrameQueue *frameQueue;

int enqueue(FrameQueue *frame_queue, AVFrame *frame)
{
	VideoFrame *vf = (VideoFrame*) malloc(sizeof(VideoFrame));
	vf->width = frame->width;
	vf->height = frame->height;
	vf->linesize_y = frame->linesize[0];
	vf->linesize_uv = frame->linesize[1];
	vf->pts = frame->pts;
	vf->next = NULL;
	vf->yuv_data = (uint8_t*) malloc(vf->height * (vf->linesize_y + vf->linesize_uv));

	//__android_log_print(ANDROID_LOG_DEBUG, TAG, "before copy: %.3f", getms());
	memcpy(vf->yuv_data, frame->data[0], vf->height * vf->linesize_y);
	memcpy(vf->yuv_data + vf->height * vf->linesize_y, frame->data[1], vf->height/2 * vf->linesize_uv);
	memcpy(vf->yuv_data + vf->height * vf->linesize_y + vf->height/2 * vf->linesize_uv, frame->data[2], vf->height/2 * vf->linesize_uv);

	frame_queue->put(vf);

	return 0;
}

int decode(void* arg)
{
	Transcoder *trans = (Transcoder *) arg;
	FILE *fp_yuv;
	int ret, i, j;
	// input demux & decode
	AVFormatContext *ic;
	AVCodecContext *codec_ctx;
	AVCodec *codec;
	AVPacket packet;
	AVFrame *frame;
	int vsid = -1;

	// ffmpeg input initialize
	av_register_all();
	
	ic = avformat_alloc_context();
	assert( NULL != ic );
	
	char *file = trans->mConfigure->inputFile;
	//file = "test.wmv";
	ret = avformat_open_input(&ic, file, NULL, NULL);
	if ( 0 != ret ) {
		printf("call avformat_open_input failed! return %d\n", ret);
		return 2;
	}
	ret = avformat_find_stream_info(ic, NULL);
	if ( ret < 0 ) {
		printf("call avformat_find_stream_info failed! return %d\n", ret);
		return 2;
	}
	av_dump_format(ic, 0, file, 0);
	for ( i = 0; i < ic->nb_streams; i++ ) {
		if ( ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO ) {
			vsid = i;
			break;
		}
	}
	if ( vsid < 0 ) {
		printf("can not find video stream!\n");
		return 3;
	}
	printf("Found video stream, id = %d\n", vsid);
	codec_ctx = ic->streams[vsid]->codec;
	codec = avcodec_find_decoder(codec_ctx->codec_id);
	if ( NULL == codec ) {
		printf("can not find decoder for video stream!");
		return 4;
	}
	if ( codec->capabilities & CODEC_CAP_TRUNCATED )
		codec_ctx->flags |= CODEC_FLAG_TRUNCATED;
	ret = avcodec_open2(codec_ctx, codec, NULL);
	if ( ret < 0 ) {
		printf("call avcodec_open2 failed! return %d\n", ret);
		return 2;
	}
	
	frame = avcodec_alloc_frame();
	fp_yuv = fopen("frames.yuv","wb");

	Dispatcher *dispatcher = new Dispatcher();
	trans->mDispatcher = dispatcher;

	// decode loop
	i = 0;
	while ( i < 1000 && (ret = av_read_frame(ic, &packet)) >= 0 ) {
		int got_frame;
		// skip other packet
		if ( packet.stream_index != vsid )
			continue;

		ret = avcodec_decode_video2(codec_ctx, frame, &got_frame, &packet);
		if ( ret != packet.size ) {
			printf("\tcall avcodec_decode_video2 return %d! input bitstream length is %d\n", ret, packet.size);
		}
		if ( ret >= 0 && got_frame ) {
			if ( frame->format == PIX_FMT_YUV420P ) {
				write_yuv_to_file(frame, fp_yuv);
				enqueue(frameQueue, frame);
				printf("\tdecode %d frames, bs_len=%d, resolution=%dx%d, format=%d\n", i, packet.size, frame->width, frame->height, frame->format);
				
				// update frame counter
				i++;
			}
		}
		av_free_packet(&packet);
	}


	av_free(frame);
	avcodec_close(codec_ctx);
	avformat_close_input(&ic);

	return 0;
}

void* decode_entry (void *arg)
{
	decode (arg);
	return NULL;
}

