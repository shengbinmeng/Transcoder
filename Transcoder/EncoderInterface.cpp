#include "EncoderInterface.h"
#include <tchar.h>

EncoderInterface::EncoderInterface(void)
{
}

EncoderInterface::~EncoderInterface(void)
{
}

int EncoderInterface::init(char* fileMappingName, Configure *cfg)
{
	mConfigure = cfg;
	int unitSize = cfg->height * cfg->width * 3/2;
	int unitCount = cfg->framesPerIdr;
	mFrameBuffer = new SharedMemory();
	mFrameBuffer->init(fileMappingName,true, unitSize, unitCount);

	//open encoder
	char args[1024];
	sprintf(args, "Encoder.exe %s %s %d %d", fileMappingName, "TODO", cfg->width, cfg->height);
	wchar_t args_w[1024];
	MultiByteToWideChar( CP_ACP, 0, args, -1, args_w, 400 );
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	ZeroMemory( &pi, sizeof(pi) );
	ZeroMemory( &si, sizeof(si) );
	si.cb = sizeof(si);
	// Start the child process
	if(CreateProcess(NULL, args_w, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi) == NULL) {
		printf("CreateProcess failed! error: %d", GetLastError());
	}
	return 0;
}

void EncoderInterface::receiveOneFrame(AVFrame *frame)
{
	int yuvSize = frame->height * frame->width * 3/2;
	uint8_t *yuv_data = (uint8_t*) malloc(yuvSize);
	memcpy(yuv_data, frame->data[0], frame->height * frame->linesize[0]);
	memcpy(yuv_data + frame->height * frame->linesize[0], frame->data[1], frame->height/2 * frame->linesize[1]);
	memcpy(yuv_data + frame->height * frame->linesize[0] + frame->height/2 * frame->linesize[1], frame->data[2], frame->height/2 * frame->linesize[2]);

	int eos = 0;
	mFrameBuffer->writeBlock(yuv_data, yuvSize, eos);
}