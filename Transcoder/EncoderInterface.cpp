#include "EncoderInterface.h"
#include <tchar.h>

EncoderInterface::EncoderInterface(void)
{
}

EncoderInterface::~EncoderInterface(void)
{
}

int EncoderInterface::startEncoding(Configure *cfg, int index, int totalNumber)
{

	char picMappingName[256], nalMappingName[256];
	sprintf(picMappingName, "MEM_SHARE_PIC_%d", index);
	sprintf(nalMappingName, "MEM_SHARE_NAL_%d", index);

	int unitSize = 0, unitCount = 0;

	mConfigure = cfg;

	unitSize = PIC_UNIT_SIZE;
	unitCount = PIC_UNIT_COUNT;
	share_mem_init(&mPicBuffer, picMappingName, unitSize, unitCount, unitCount, 1);

	unitSize = NAL_UNIT_SIZE;
	unitCount = cfg->framesPerIdr;
	share_mem_init(&mNalBuffer, nalMappingName, unitSize, unitCount, unitCount, 1);

	//open encoder
	char args[1024];
	sprintf(args, "Encoder.exe %s %s %d %d %d %d %d", picMappingName, nalMappingName, cfg->width, cfg->height, 32, index, totalNumber);
	wchar_t args_w[1024];
	MultiByteToWideChar( CP_ACP, 0, args, -1, args_w, 400 );

	ZeroMemory( &mPI, sizeof(mPI) );
	ZeroMemory( &mSI, sizeof(mSI) );
	mSI.cb = sizeof(mSI);
	// Start the child process
	BOOL ret = 0;
	ret = CreateProcess(NULL, args_w, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &mSI, &mPI);
	if(ret == 0) {
		printf("CreateProcess failed! error: %d", GetLastError());
	}
	return 0;
}

void EncoderInterface::inputOneFrame(AVFrame *frame, int eos)
{
	if (eos == 1) {
		share_mem_write(&mPicBuffer, NULL, 0, 1);
		return ;
	}
	int ySize = frame->height * frame->width;
	share_mem_write(&mPicBuffer, frame->data[0], ySize, eos);
	share_mem_write(&mPicBuffer, frame->data[1], ySize * 1/4, eos);
	share_mem_write(&mPicBuffer, frame->data[2], ySize * 1/4, eos);
}

int EncoderInterface::outputBitsOfOneFrame(uint8_t *buffer, int maxSize, int* eos)
{
	int readSize = share_mem_read(&mNalBuffer, buffer, maxSize, eos);
	if (readSize != -1) {
		return readSize;
	}

	return -1;
}

int EncoderInterface::cleanUp()
{
	share_mem_uninit(&mPicBuffer);
	share_mem_uninit(&mNalBuffer);

	TerminateProcess(mPI.hProcess, 0);
	return 0;
}