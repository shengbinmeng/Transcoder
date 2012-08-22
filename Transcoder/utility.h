extern "C" {
#include "libavformat/avformat.h"
}

void write_yuv_to_file(AVFrame* frame, FILE *fp);