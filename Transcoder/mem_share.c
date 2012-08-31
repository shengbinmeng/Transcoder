#include "mem_share.h"
#include<stdio.h>

int share_mem_init( share_mem_info_t *shm, char *file_mapping_name, int unit_size, int unit_count, int mapping_count, int create)
{
	wchar_t mapping[400];

	mapping_count = mapping_count > unit_count ? unit_count : mapping_count;
	
	shm->current_read = shm->current_write = 0;
	shm->current_read_map = shm->current_write_map = unit_count;
	shm->unit_size = unit_size;
	shm->unit_count = unit_count;
	shm->mapping_count = mapping_count;

	MultiByteToWideChar( CP_ACP, 0, file_mapping_name, -1,
		mapping, 400 );
	if (create) {
		shm->map_handle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
			PAGE_READWRITE, 0, (shm->unit_size + UNIT_HEADER_LENGTH) * unit_count, mapping );
	} else {
		shm->map_handle = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, mapping );
	}

	if( shm->map_handle == NULL ) {
		printf( "Can't create/open file mapping, error %d\n", GetLastError() );
		return -1;
	}
	
	shm->buffer_handle = NULL;

	shm->share_unit = (share_mem_unit_t **) malloc(unit_count * sizeof(share_mem_unit_t*));

	return 0;
}

int share_mem_read ( share_mem_info_t *shm, void *buffer, int max_size, int *eos)
{
	int b_end = 0, b_eos = 0, read_size = 0;
	uint8_t *addr = (uint8_t *)buffer;

	while( b_end == 0 && max_size > 0 )
	{
		share_mem_unit_t *u;
		int content_size;

		if( shm->current_read - shm->current_read_map >= shm->mapping_count 
			|| shm->current_read - shm->current_read_map < 0 ) //remapping
		{
			int i, i_max_unit;
			uint8_t *memory;

			shm->current_read_map = shm->current_read - shm->current_read % shm->mapping_count;
			if( shm->buffer_handle )
				UnmapViewOfFile( shm->buffer_handle );
			shm->buffer_handle = MapViewOfFile( shm->map_handle, FILE_MAP_ALL_ACCESS, 0, 
				shm->current_read_map * (shm->unit_size + UNIT_HEADER_LENGTH), shm->mapping_count * (shm->unit_size + UNIT_HEADER_LENGTH) );
			i_max_unit = shm->current_read_map + shm->mapping_count;
			i_max_unit = shm->unit_count > i_max_unit ? i_max_unit : shm->unit_count;

			memory = (uint8_t *)shm->buffer_handle;
			for( i = shm->current_read_map; i < i_max_unit; i ++ ) 
				shm->share_unit[i] = (share_mem_unit_t *)(memory + (shm->unit_size + UNIT_HEADER_LENGTH) * (i - shm->current_read_map));
		}

		u = shm->share_unit[shm->current_read];

		while( u->has_content == 0 )
		{
			printf("in reading, wait for content, cursor: %d \n", shm->current_read);
			Sleep( 50 );
		}
		content_size = u->content_size;
		if (max_size < content_size) {
			// should not change our content; user needs to increase max size
			return -1;
		}

		b_end    = u->eob;
		b_eos    = u->eos;
		memcpy( addr, CONTENT(u), content_size );
		u->has_content = 0;

		printf("read a block: %d \n",shm->current_read);

		shm->current_read = (shm->current_read + 1) % shm->unit_count;
		addr += content_size;
		max_size -= content_size;
		read_size += content_size;
	}

	*eos = b_eos;

	return read_size;
}

int share_mem_write( share_mem_info_t *shm, void *buffer, int size, int eos )
{
	int unit_size = shm->unit_size;
	uint8_t *addr = (uint8_t *)buffer;

	while( size >= 0 )
	{
		int content_size = size > unit_size ? unit_size : size;
		share_mem_unit_t *u;

		if( shm->current_write - shm->current_write_map >= shm->mapping_count 
			|| shm->current_write - shm->current_write_map < 0 ) //remapping
		{
			int i, i_max_unit;
			uint8_t *memory;

			shm->current_write_map = shm->current_write - shm->current_write % shm->mapping_count;
			if( shm->buffer_handle )
				UnmapViewOfFile( shm->buffer_handle );
			shm->buffer_handle = MapViewOfFile( shm->map_handle, FILE_MAP_ALL_ACCESS, 0, 
				shm->current_write_map * (shm->unit_size + UNIT_HEADER_LENGTH), shm->mapping_count * (shm->unit_size + UNIT_HEADER_LENGTH) );
			i_max_unit = shm->current_write_map + shm->mapping_count;
			i_max_unit = shm->unit_count > i_max_unit ? i_max_unit : shm->unit_count;

			memory = (uint8_t *)shm->buffer_handle;
			for( i = shm->current_write_map; i < i_max_unit; i ++ ) 
				shm->share_unit[i] = (share_mem_unit_t *)(memory + (shm->unit_size + UNIT_HEADER_LENGTH) * (i - shm->current_write_map) );
		}

		u = shm->share_unit[shm->current_write];

		while( u->has_content != 0 )
		{
			printf("in writing, wait for space; cursor: %d \n", shm->current_write);
			Sleep( 50 );
		}

		u->content_size  = content_size;
		u->eob = size <= unit_size ? 1 : 0;
		u->eos = (u->eob && eos) ? 1 : 0;

		memcpy( CONTENT(u), addr, content_size );
		printf("write a block %d, size %d \n",shm->current_write, content_size);

		u->has_content = 1;

		shm->current_write = (shm->current_write + 1) % shm->unit_count;
		addr += content_size;
		size -= content_size;
		if( size == 0 ) {
			// end of stream
			break;
		}
	}

	return 0;
}

void share_mem_uninit( share_mem_info_t *shm )
{
	free( shm->share_unit );
	if( shm->buffer_handle )
		UnmapViewOfFile( shm->buffer_handle );
	CloseHandle( shm->map_handle );
}
