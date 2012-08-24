#include<iostream>
#include<fstream>
using namespace std;

#include "mem_share.h"

int main(int argc, char **argv)
{
	cout<<"I will do encoding."<<endl;
	if (argc != 5) return 0;
	cout<<argv[0]<<" "<<argv[1]<<" "<<argv[2]<<" "<<argv[3]<<" "<<argv[4]<<" "<<endl;
	char *framesMappingName = argv[1];
	char *bitstreamMappingName = argv[2];
	int width = atoi(argv[3]);
	int height = atoi(argv[4]);

	char filename[1024];
	sprintf(filename,"%s-read.yuv", framesMappingName);
	FILE *fpYUV = fopen(filename, "wb");
	sprintf(filename,"%s-nal.bin", bitstreamMappingName);
	FILE *fpNAL = fopen(filename, "wb");

	share_mem_info_t framesBuffer, bitstreamBuffer;
	int unitSize = width * height * 3/2;
	int unitCount = 100;
	share_mem_init(&framesBuffer, framesMappingName, unitSize, unitCount, false);
	
	unitSize = 2000; // 2K
	unitCount = 100;
	share_mem_init(&bitstreamBuffer, bitstreamMappingName, unitSize, unitCount, true);

	int maxSize = 8000000; // 8M
	uint8_t *yuvBuffer = (uint8_t*) malloc(maxSize);
	int endFlag = 0, eos = 0;
	while (1) {
		int readSize = share_mem_read(&framesBuffer, yuvBuffer, maxSize, &endFlag);
		cout<<"read a block, size: "<<readSize<<endl;
		Sleep(100); // encoding time
		fwrite(yuvBuffer, readSize, 1, fpYUV);

		// write some nal units
		eos = (endFlag == 1 ? 1 : 0);
		int size = 500 + (rand() % 2000);
		uint8_t *nal = (uint8_t*) malloc(size);
		memset(nal, 0, size);
		//share_mem_write(&bitstreamBuffer, nal, size, eos);
		fwrite(nal, size, 1, fpNAL);
		free (nal);
		if (endFlag == 1) break; // end of stream
	}

	free (yuvBuffer);
	share_mem_uninit(&framesBuffer);
	share_mem_uninit(&bitstreamBuffer);

	return 0;
}