#include "utility.h"

void write_yuv_to_file(AVFrame* frame, FILE *fp) {
	int i = 0;
	int width = frame->width, height = frame->height;
	for (i = 0; i < height; i++) {
		fwrite(frame->data[0] + frame->linesize[0] * i, width, 1, fp);
	}
	for (i = 0; i < height/2;i++) {
		fwrite(frame->data[1] + frame->linesize[1] * i, width/2, 1, fp);
	}
	for (i = 0; i < height/2; i++) {
		fwrite(frame->data[2] + frame->linesize[2] * i, width/2, 1, fp);
	}
	fflush(fp);
}