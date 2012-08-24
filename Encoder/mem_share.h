#pragma once

#include "stdint.h"
#ifndef _WINDOWS_
#include <windows.h>
#endif

#define UNIT_HEADER_LENGTH 8 //8 byte
#define CONTENT( unit ) ((uint8_t *)unit + UNIT_HEADER_LENGTH)

typedef struct
{
	uint32_t content_size;
	uint8_t has_content;
	uint8_t eos; //end of stream
	uint8_t eob; //end of block
	uint8_t reserved; //reserved

	//contents
} share_mem_unit_t;

typedef struct  
{
	HANDLE map_handle;
	LPVOID buffer_handle;
	share_mem_unit_t **share_unit;
	int32_t current_read;
	int32_t current_write;
	int32_t unit_size;
	int32_t unit_count;
} share_mem_info_t;

int share_mem_init( share_mem_info_t *shm, char *file_mapping_name, int unit_size, int unit_count, bool create);
int share_mem_read ( share_mem_info_t *shm, void *buffer, int max_size, int *eos);
int share_mem_write( share_mem_info_t *shm, void *buffer, int size, int eos );
void share_mem_uninit( share_mem_info_t *shm );