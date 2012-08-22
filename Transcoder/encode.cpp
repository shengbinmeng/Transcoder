#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
extern "C" {
#include "libavformat/avformat.h"
#include "x264.h"
}
#include "framequeue.h"
#include "encode.h"

int i, j, ret;
FILE *fp_264;

// encode
x264_param_t x264_param;
x264_t *x264_handle = NULL;
static uint8_t x264_headers_buf[1024 * 256];
uint8_t *x264_headers = NULL;
int x264_headers_len = 0;
x264_picture_t x264_pic;
x264_picture_t x264_pic_out;
x264_nal_t *x264_nal;
int x264_nal_count;
int x264_pics_in = 0, x264_pics_out = 0;

extern FrameQueue *frameQueue;

int encode_init()
{
	// x264 encoder initialize
	x264_param_default_preset(&x264_param, "", "ssim");
	x264_param.i_width = 640;
	x264_param.i_height = 480;
	x264_param.i_threads = 4;
	x264_param.i_keyint_max = 150;
	x264_param.i_keyint_min = 25;
	x264_param.analyse.i_me_range = 64;
	x264_param.rc.i_rc_method = X264_RC_ABR;
	x264_param.rc.i_bitrate = 500;
	x264_param.i_fps_num = 30;
	x264_param.i_fps_den = 1;
	x264_param.i_timebase_num = 1;//x264_param.i_fps_den;
	x264_param.i_timebase_den = 1000;//x264_param.i_fps_num;
	x264_handle = x264_encoder_open(&x264_param);
	if ( NULL == x264_handle ) {
		printf("call x264_encoder_open failed!\n");
		return 4;
	}

	return 0;
}

int encode(char *file)
{
	file = "test.264";
	fp_264 = fopen(file, "wb");
	if (fp_264 == NULL) {
		printf("open output bitstream file failed!\n");
		return 4;
	}

	encode_init();

	// encode headers
	ret = x264_encoder_headers(x264_handle, &x264_nal, &x264_nal_count);
	if ( ret > 0 && x264_nal_count > 0 ) {
		x264_headers_len = 0;
		x264_headers = (uint8_t*)(((uint32_t)x264_headers_buf + 0x07ff) & 0xfffff800);
		for ( j = 0; j < x264_nal_count; j++ ) {
			uint8_t nal_type = x264_nal[j].p_payload[4] & 0x1f;
			if ( NAL_SPS != nal_type && NAL_PPS != nal_type )
				continue;
			assert( (x264_headers_len + x264_nal[j].i_payload + 4096) < sizeof(x264_headers_buf) );
			memcpy(x264_headers + x264_headers_len, x264_nal[j].p_payload, x264_nal[j].i_payload);
			x264_headers_len += x264_nal[j].i_payload;
		}

		fwrite(x264_headers, x264_headers_len, 1, fp_264);
	}

	// encode frames
	while (1) {
		VideoFrame *vf = NULL;
		frameQueue->get(&vf, true);
		memset(&x264_pic, 0, sizeof(x264_pic));
		x264_pic.i_type = X264_TYPE_AUTO;
		x264_pic.i_qpplus1 = X264_QP_AUTO;
		x264_pic.i_pts = vf->pts;
		x264_pic.img.i_csp = X264_CSP_I420;
		x264_pic.img.i_plane = 3;
		x264_pic.img.i_stride[0] = vf->linesize_y;
		x264_pic.img.i_stride[1] = x264_pic.img.i_stride[2] = vf->linesize_uv;
		x264_pic.img.plane[0] = vf->yuv_data;
		x264_pic.img.plane[1] = vf->yuv_data + vf->height * vf->linesize_y;
		x264_pic.img.plane[2] = vf->yuv_data + vf->height * vf->linesize_y + vf->height/2 * vf->linesize_uv;

		ret = encode_one_picture (&x264_pic);

		if (ret == 1) break;

		free(vf->yuv_data);
		free(vf);
	}

	x264_encoder_close(x264_handle);
}

int encode_one_picture(x264_picture_t *pic)
{
	ret = x264_encoder_encode(x264_handle, &x264_nal, &x264_nal_count, pic, &x264_pic_out);
	if ( ret < 0 ) {
		fprintf(stderr, "call x264_encoder_encode failed! return %d\n", ret);
	}
	x264_pics_in++;
	printf("input %d frame to encoder, pts = %lld\n", x264_pics_in, x264_pic.i_pts);
	fflush(stdout);

	if (x264_pics_in == 999) return 1;

	if ( ret > 0 && x264_nal_count > 0 ) {
		size_t bs_len = 0;
		x264_pics_out++;
		printf("output %d frame from encoder, pts = %lld\n", x264_pics_out, x264_pic_out.i_pts);
		fflush(stdout);

		for ( j = 0; j < x264_nal_count; j++ ) {
			fwrite(x264_nal[j].p_payload, 1, x264_nal[j].i_payload, fp_264);
		}
		
		printf("encode %d frame, pts = %lld, type = %d\n", i, pic->i_pts, x264_pic_out.i_type);
	}

	return 0;
}
int encode_one_frame(AVFrame *frame)
{
	// x264 encode
	memset(&x264_pic, 0, sizeof(x264_pic));
	x264_pic.i_type = X264_TYPE_AUTO;
	x264_pic.i_qpplus1 = X264_QP_AUTO;
	x264_pic.i_pts = frame->pts;
	x264_pic.img.i_csp = X264_CSP_I420;
	x264_pic.img.i_plane = 3;
	x264_pic.img.i_stride[0] = frame->linesize[0];
	x264_pic.img.i_stride[1] = frame->linesize[1];
	x264_pic.img.i_stride[2] = frame->linesize[2];
	x264_pic.img.plane[0] = frame->data[0];
	x264_pic.img.plane[1] = frame->data[1];
	x264_pic.img.plane[2] = frame->data[2];
	
	return encode_one_picture (&x264_pic);
}

void* encode_entry (void* arg)
{
	encode((char*)arg);
	return NULL;
}