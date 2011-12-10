#pragma once

#define VIDEO_FRAME 1

extern "C"
{
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}
#include "cv.h"
#include "highgui.h"

using namespace std;



class VideoCapture
{
private:
	AVFormatContext *pFormatCtx;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	AVFrame         *pFrame;
	AVFrame         *pFrameRGB;
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer;
	uint8_t			*ptr1, *ptr2;
	int				m, n, p, count;
	IplImage*		img;
	struct SwsContext *img_convert_ctx;
	void free_mem();
	void init();
	char file[256];

public:
	VideoCapture();
	VideoCapture(char* filename);
	void set_filename(const char* filename);
	IplImage* retreiveIplImage();
	void reset();
	IplImage* retreiveFrame(int &code);
	bool seek_frame(int f);

	~VideoCapture(void);
};
