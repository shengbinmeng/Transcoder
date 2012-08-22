#include <stdlib.h>
#include <stdio.h>
#include "SharedMemory.h"

int SharedMemory::init(char *fileMappingName, bool isCreating, int unitSize, int unitCount)
{
	mUnitCount = unitCount;
	mUnitSize = unitSize;
	mReadCursor = mWriteCursor = 0;
	wchar_t mapping[400];
	MultiByteToWideChar( CP_ACP, 0, fileMappingName, -1, mapping, 400 );
	int totalSize = unitCount * (unitSize + SharedMemory::unitHeaderLength);
	if (isCreating) {
		mHandle = CreateFileMapping( INVALID_HANDLE_VALUE, NULL,
		PAGE_READWRITE, 0, totalSize, mapping );
	} else {
		mHandle = OpenFileMapping( FILE_MAP_ALL_ACCESS, FALSE, mapping );
	}

	if ( mHandle == NULL ) {
		printf( "Can't open file mapping, error %d\n", GetLastError() );
		return -1;
	}

	// can treat mBuffer as a file
	mBuffer = MapViewOfFile( mHandle, FILE_MAP_ALL_ACCESS, 0, 0, 0 );
	if( mBuffer == NULL )
	{
		printf( "Can't mapp view of file, error %d\n", GetLastError() ); 
		CloseHandle( mHandle );
		return -1;
	}

	uint8_t *buffer = (uint8_t*) mBuffer;

	mUnits = (share_mem_unit_t **) malloc(mUnitCount * sizeof(share_mem_unit_t*));
	for (int i = 0; i < mUnitCount; i++) {
		mUnits[i] =(share_mem_unit_t *)(buffer + (mUnitSize + SharedMemory::unitHeaderLength) *  i);
	}

	return 0;
}

// write a block; will wait if no place to put data; return 0 if no error
int SharedMemory::writeBlock(uint8_t* data, int dataSize, int eos)
{
	while (dataSize > 0) {
		share_mem_unit_t *u = mUnits[mWriteCursor];
		while (u->has_content == 1) {
			printf("in writing, wait for space; cursor: %d \n", mWriteCursor);
			Sleep(10);
		}
		int size = dataSize <= mUnitSize ? dataSize : mUnitSize;
		uint8_t *buffer = SharedMemory::contentOf(u);
		memcpy(buffer, data, size);
		u->has_content = 1;
		u->content_size = size;
		u->eob = (dataSize <= mUnitSize ? 1 : 0);
		if (u->eob && eos) u->eos = 1;
		else u->eos = 0;

		dataSize -= size;
		data += size;
		mWriteCursor = (mWriteCursor + 1) % mUnitCount;
	}
	
	return 0;
}

// read a block; set endFlag = 1, if end of all stream; set it to -1 if maxSize not enough to hold a block
int SharedMemory::readBlock(uint8_t* buffer, int *endFlag, int maxSize)
{
	int readSize = 0, endOfBlock = 0, endOfStream = 0;
	while (endOfBlock == 0 && maxSize > 0) {
		share_mem_unit_t *u = mUnits[mReadCursor];
		while (u->has_content != 1) {
			printf("waiting for content, cursor: %d \n", mReadCursor);
			Sleep(10);
		}
		int size = u->content_size;
		if (size > maxSize) {
			// this should not happen in normal use
			size = maxSize;
			memcpy(buffer, SharedMemory::contentOf(u), size);
			readSize += size;
			break;
		}
		memcpy(buffer, SharedMemory::contentOf(u), size);
		u->has_content = 0;
		readSize += size;

		if (u->eob == 1) endOfBlock = 1;
		if (u->eos == 1) endOfStream = 1;

		maxSize -= size;
		mReadCursor = (mReadCursor + 1) % mUnitCount;
	}

	if (endOfBlock == 0) *endFlag = -1;
	else if (endOfStream == 1) *endFlag = 1;
	else *endFlag = 0;

	return readSize;
}


