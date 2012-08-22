#include<iostream>
#include<fstream>
using namespace std;
#include "SharedMemory.h"
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
	cout<<"I will do encoding."<<endl;
	if (argc != 5) return 0;
	cout<<argv[0]<<argv[1]<<argv[2]<<argv[3]<<argv[4]<<endl;
	char *framesMappingName = argv[1];
	char *bitstreamMappingName = argv[2];
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);

	char filename[1024];
	sprintf(filename,"%s-read.yuv", framesMappingName);
	FILE *fpYUV = fopen(filename, "wb");
	sprintf(filename,"%s-nal.bin", bitstreamMappingName);
	FILE *fpNAL = fopen(filename, "wb");

	SharedMemory *framesBuffer = new SharedMemory();
	int unitSize = width * height * 3/2;
	int unitCount = 100;
	framesBuffer->init(framesMappingName, false, unitSize, unitCount);

	SharedMemory *bitstreamBuffer = new SharedMemory();
	unitSize = 2000; // 2K
	unitCount = 100;
	bitstreamBuffer->init(bitstreamMappingName, false, unitSize, unitCount);
	int maxSize = 8000000; // 8M
	uint8_t *yuvBuffer = (uint8_t*) malloc(maxSize);
	int endFlag = 0, eos = 0;
	while (1) {
		int readSize = framesBuffer->readBlock(yuvBuffer, &endFlag, maxSize);
		cout<<"read a block, size: "<<readSize<<endl;
		Sleep(100);
		fwrite(yuvBuffer, readSize, 1, fpYUV);
		// write some bits
		eos = (endFlag == 1 ? 1 : 0);
		int nalSize = 500 + (rand() % 2000);
		uint8_t *nal = (uint8_t*) malloc(nalSize);
		memset(nal, 0, nalSize);
		//bitstreamBuffer->writeBlock(nal, nalSize, eos);
		fwrite(nal, nalSize, 1, fpNAL);
		if (endFlag == 1) break; // end of stream
	}

	return 0;
}