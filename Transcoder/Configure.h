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
public:
	Configure(void);
	~Configure(void);
};
