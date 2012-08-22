extern "C" {
#include "libavformat/avformat.h"
}
#include "x264.h"
int encode_init();
int encode();
int encode_one_frame(AVFrame * frame);
int encode_one_picture(x264_picture_t *pic);
void* encode_entry (void* arg);