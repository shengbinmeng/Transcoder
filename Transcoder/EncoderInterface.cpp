#include "EncoderInterface.h"
#include <tchar.h>

#define UNIT_HEADER_LENGTH 8 // 8 byte
#define CONTENT(unit) ((uint8_t *)unit + UNIT_HEADER_LENGTH)

#define PIC_UNIT_SIZE  ((1<<19)-UNIT_HEADER_LENGTH) // a little larger than size of 1920x1080 chroma
#define UNITS_PER_PIC 6 // a pic's yuv data size is 6 times of chroma data size
#define NAL_UNIT_SIZE  ((1<<14)-UNIT_HEADER_LENGTH) // 4K

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

	mConfigure = cfg;

	int unitSize = PIC_UNIT_SIZE;
	int unitCount = UNITS_PER_PIC * cfg->framesPerIdr;
	mPicBuffer.init(picMappingName, unitSize, unitCount, 1);

	unitSize = NAL_UNIT_SIZE;
	unitCount = cfg->framesPerIdr;
	mNalBuffer.init(nalMappingName, unitSize, unitCount, 1);

	// Open encoder.
	char args[1024];
	sprintf(args, "Encoder.exe %s %s %d %d %d %d %d", picMappingName, nalMappingName, cfg->width, cfg->height, 32, index, totalNumber);
	wchar_t args_w[1024];
	MultiByteToWideChar(CP_ACP, 0, args, -1, args_w, 400);

	ZeroMemory(&mPI, sizeof(mPI));
	STARTUPINFO si;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	// Start the child process.
	BOOL ret = CreateProcess(NULL, args_w, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &mPI);
	if (ret == FALSE) {
		printf("CreateProcess failed! error: %d", GetLastError());
	}
	return 0;
}

void EncoderInterface::inputOneFrame(AVFrame *frame, int eos)
{
	if (eos == 1) {
		mPicBuffer.writeBlock(NULL, 0, 1);
		return ;
	}
	int ySize = frame->height * frame->width;
	mPicBuffer.writeBlock(frame->data[0], ySize, 0);
	mPicBuffer.writeBlock(frame->data[1], ySize * 1/4, 0);
	mPicBuffer.writeBlock(frame->data[2], ySize * 1/4, 0);
}

int EncoderInterface::outputBitsOfOneFrame(uint8_t *buffer, int maxSize, int* eos)
{
	int readSize = mNalBuffer.readBlock(buffer, maxSize, eos);
	return readSize;
}

int EncoderInterface::cleanUp()
{
	TerminateProcess(mPI.hProcess, 0);
	mPicBuffer.uninit();
	mNalBuffer.uninit();
	return 0;
}