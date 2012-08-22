#pragma once

#ifndef _WINDOWS_
#include <windows.h>
#endif
#include "stdint.h"

typedef struct
{
	int content_size;
	uint8_t has_content;
	uint8_t eos; //end of stream
	uint8_t eob; //end of block
	uint8_t res;
	//reserved bits

	//contents
} share_mem_unit_t;

class SharedMemory {
private:
	HANDLE mHandle;
	LPVOID mBuffer;
	share_mem_unit_t **mUnits;
	int mReadCursor;
	int mWriteCursor;
	int mUnitCount;
	int mUnitSize;

public:
	static const int unitHeaderLength = 8;
	static uint8_t* contentOf(share_mem_unit_t * u) {
		return (uint8_t *)u + unitHeaderLength;
	}

	int init(char *fileMappingName, bool isCreating, int unitSize, int unitCount);
	int writeBlock(uint8_t* data, int dataSize, int eos);
	int readBlock(uint8_t* buffer, int *endFlag, int maxSize);
};