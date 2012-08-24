#include "EncoderInterface.h"
#include <tchar.h>

EncoderInterface::EncoderInterface(void)
{
}

EncoderInterface::~EncoderInterface(void)
{
}

int EncoderInterface::init(char* picMappingName, char* nalMappingName, Configure *cfg)
{
	int unitSize = 0, unitCount = 0;

	mConfigure = cfg;

	unitSize = PIC_UNIT_SIZE;
	unitCount = cfg->framesPerIdr * 6; // PIC_UNIT_COUNT = 600
	share_mem_init(&mPicBuffer, picMappingName, unitSize, unitCount, unitCount, 1);

	unitSize = NAL_UNIT_SIZE;
	unitCount = cfg->framesPerIdr;
	share_mem_init(&mNalBuffer, nalMappingName, unitSize, unitCount, unitCount, 1);

	//open encoder
	char args[1024];
	sprintf(args, "Encoder.exe %s %s %d %d %d", picMappingName, nalMappingName, cfg->width, cfg->height, 32);
	wchar_t args_w[1024];
	MultiByteToWideChar( CP_ACP, 0, args, -1, args_w, 400 );
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	// Start the child process
	BOOL ret = 0;
	ret = CreateProcess(NULL, args_w, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
	if(ret == 0) {
		printf("CreateProcess failed! error: %d", GetLastError());
	}
	return 0;
}

void EncoderInterface::inputOneFrame(AVFrame *frame, int eos)
{
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
}