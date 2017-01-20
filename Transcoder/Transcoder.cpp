#include <stdlib.h>
#include <stdint.h>
#include "Transcoder.h"
extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/avutil.h"
}

Transcoder::Transcoder(void)
{
	mDispatcher = new Dispatcher();
	mCollector = new Collector();
	mEncoders = NULL;
	mConfigure = NULL;
}

Transcoder::~Transcoder(void)
{
	delete mDispatcher;
	delete mCollector;
}

int Transcoder::prepare()
{
	if (mConfigure == NULL) {
		printf("not configured!\n");
		return -1;
	}
	mEncoders = (EncoderInterface **)malloc(sizeof(EncoderInterface*) * mConfigure->encoderNumber);
	int number = mConfigure->encoderNumber;
	for (int i = 0; i < number; i++) {
		mEncoders[i] = new EncoderInterface();
	}

	mDispatcher->init(mConfigure);
	mDispatcher->setEncoders(mEncoders);

	mCollector->init(mConfigure);
	mCollector->setEncoders(mEncoders);

	return 0;
}


int Transcoder::work()
{
	int ret, i;

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
	if (ic == NULL) {
		printf("call avformat_alloc_context failed!");
		return 2;
	}

	char *file = mConfigure->inputFile;
	//file = "test.wmv";
	ret = avformat_open_input(&ic, file, NULL, NULL);
	if (0 != ret) {
		printf("call avformat_open_input failed! return %d\n", ret);
		return 2;
	}
	ret = avformat_find_stream_info(ic, NULL);
	if (ret < 0) {
		printf("call avformat_find_stream_info failed! return %d\n", ret);
		return 2;
	}
	av_dump_format(ic, 0, file, 0);
	for (i = 0; i < ic->nb_streams; i++) {
		if (ic->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			vsid = i;
			break;
		}
	}
	if (vsid < 0) {
		printf("can not find video stream!\n");
		return 3;
	}
	printf("Found video stream, id = %d\n", vsid);
	codec_ctx = ic->streams[vsid]->codec;
	codec = avcodec_find_decoder(codec_ctx->codec_id);
	if (NULL == codec) {
		printf("can not find decoder for video stream!");
		return 4;
	}
	if (codec->capabilities & CODEC_CAP_TRUNCATED) {
		codec_ctx->flags |= CODEC_FLAG_TRUNCATED;
	}
	ret = avcodec_open2(codec_ctx, codec, NULL);
	if (ret < 0) {
		printf("call avcodec_open2 failed! return %d\n", ret);
		return 2;
	}

	mConfigure->height = codec_ctx->height;
	mConfigure->width = codec_ctx->width;

	for (int i = 0; i < mConfigure->encoderNumber; i++) {
		mEncoders[i]->startEncoding(mConfigure, i, mConfigure->encoderNumber);
	}

	mCollector->startCollecting();

	frame = avcodec_alloc_frame();
	i = 0;
	// decode loop
	while (av_read_frame(ic, &packet)) {
		int got_frame;
		// skip other packet
		if (packet.stream_index != vsid) {
			continue;
		}

		ret = avcodec_decode_video2(codec_ctx, frame, &got_frame, &packet);
		if (ret != packet.size) {
			printf("call avcodec_decode_video2 return %d! input bitstream length is %d\n", ret, packet.size);
		}
		if (ret >= 0 && got_frame) {
			if (frame->format == PIX_FMT_YUV420P) {
				mDispatcher->dispatch(frame, 0);

				// update frame counter
				i++;
				printf("decode %d frames, resolution=%dx%d\n", i, frame->width, frame->height);
			}
		}
		av_free_packet(&packet);

		if (mConfigure->frameNumber > 0 && i > mConfigure->frameNumber) {
			break;
		}
	}

	mDispatcher->dispatch(NULL, 1);

	av_free(frame);
	avcodec_close(codec_ctx);
	avformat_close_input(&ic);

	mCollector->finishCollecting();

	return 0;
}

int Transcoder::clean()
{
	for (int i = 0; i < mConfigure->encoderNumber; i++) {
		EncoderInterface *encoder = mEncoders[i];
		encoder->cleanUp();
		free(encoder);
	}
	free(mEncoders);

	return 0;
}

