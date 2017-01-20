#pragma once

class Configure
{
public:
	int encoderNumber;
	int width;
	int height;
	char *inputFile;
	char *outputFile;
	int framesPerIdr;
	int frameNumber;
public:
	Configure(void);
	~Configure(void);
};
