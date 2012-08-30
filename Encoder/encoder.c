//****************************************************************
//Lentoid_for_transcode.c
//包含一个利用共享内存读入图像写出码流的外壳
//****************************************************************

#include "stdint.h"
#include "Lentoid.h"
#include "mem_share.h"
#include <stdio.h>

#define PIC_MAPPING_COUNT 10

int read_frame( LENT_picture_t *pic, share_mem_info_t *shm )
{
	size_t luma_size = pic->img.i_width[0] * pic->img.i_height[0];
	size_t chroma_size = luma_size >> 2;
	int eos;

	// returned read_size == 0 means "end of stream"
	if( share_mem_read( shm, pic->img.plane[0], luma_size, &eos ) <= 0 )
		return -1;

	if( !eos && share_mem_read( shm, pic->img.plane[1], chroma_size, &eos ) <= 0 )
		return -1;

	if( !eos && share_mem_read( shm, pic->img.plane[2], chroma_size, &eos ) <= 0 )
		return -1;

	return 0;
}

int Encode( LENT_param_t *param, share_mem_info_t *shm_pic, share_mem_info_t *shm_nal )
{
	LENT_HANDLE h;
	LENT_picture_t pic, pic_out;
	int hr = 0;
	int i_nal_size, i_nal;
	LENT_nal_t *nal = NULL;
	int i_width = param->spatial[param->i_spatial_layer - 1].i_width;
	int i_height = param->spatial[param->i_spatial_layer - 1].i_height;

	h = LENT_encoder_open( param );
	if( LENT_picture_alloc( &pic, i_width, i_height ) || !h )
		return -1;

	while( read_frame( &pic, shm_pic ) == 0 )
	{
		i_nal_size = LENT_encoder_encode( h, &nal, &i_nal, &pic, &pic_out );
		if( i_nal_size < 0 ) {
			share_mem_write( shm_nal, NULL, 0, 1 );
			return -1;
		}
		else if( i_nal_size )
			share_mem_write( shm_nal, nal->p_payload, i_nal_size, 0 );
	}
	
	while( LENT_encoder_encoding( h ) )
	{
		i_nal_size = LENT_encoder_encode( h, &nal, &i_nal, NULL, &pic_out );
		if( i_nal_size < 0 ) {
			share_mem_write( shm_nal, NULL, 0, 1 );
			return -1;
		}
		else if( i_nal_size )
			share_mem_write( shm_nal, nal->p_payload, i_nal_size, 0 );
	}

	share_mem_write( shm_nal, NULL, 0, 1 );
	LENT_picture_free( &pic );
	LENT_encoder_close( h );

	return 0;
}

int main( int argc, char **argv )
{
	LENT_param_t param;
	share_mem_info_t shm_pic, shm_nal;
	int ret;

	if( argc != 8 )
		return -1;

	LENT_param_default( &param );
	param.i_spatial_layer = 1;
	param.spatial[0].i_width  = atoi( argv[3] );
	param.spatial[0].i_height = atoi( argv[4] );

	ret = share_mem_init( &shm_pic, argv[1], PIC_UNIT_SIZE, PIC_UNIT_COUNT, PIC_MAPPING_COUNT, 0 );
	if (ret != 0) {
		printf("share_men_int for pic failed! \n");
		return -1;
	}

	ret = share_mem_init( &shm_nal, argv[2], NAL_UNIT_SIZE, NAL_UNIT_COUNT, NAL_UNIT_COUNT, 0 );
	if (ret != 0) {
		printf("share_men_int for nal failed! \n");
		return -1;
	}

	{
		int val = atoi( argv[5] );

		param.spatial[0].i_qp[0] = val;
		param.spatial[0].i_bitrate[0] = val;
	}

	param.i_encoder_index = atoi( argv[6] );
	param.i_encoder_count = atoi( argv[7] );

	ret = Encode( &param, &shm_pic, &shm_nal );

	return 0;
}