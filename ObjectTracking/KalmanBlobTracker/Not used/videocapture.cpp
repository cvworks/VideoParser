#include "videocapture.h"
#include <iostream>


static void ignore_output(void *ptr, int level, const char *fmt, va_list vl){/* ignore the msg*/}


VideoCapture::VideoCapture(): m(0), n(0), p(0), count(0), i(0), img(NULL), buffer(NULL), pFrameRGB(NULL), pFrame(NULL), pCodecCtx(NULL), pFormatCtx(NULL)
{
}


VideoCapture::VideoCapture(char* filename): m(0), n(0), p(0), count(0), i(0), img(NULL), buffer(NULL), pFrameRGB(NULL), pFrame(NULL), pCodecCtx(NULL), pFormatCtx(NULL)
{
	strcpy(file, filename);
	init();
}

void VideoCapture::set_filename(const char* filename)
{
	strcpy(file, filename);
	init();
}

//initailize
void VideoCapture::init()
{

	av_log_set_callback(ignore_output);

	// Register all formats and codecs
	av_register_all();

	// Open video file
	if(av_open_input_file(&pFormatCtx, file, NULL, 0, NULL)!=0)
		return;

	// Retrieve stream information
	if(av_find_stream_info(pFormatCtx)<0)
		return;

	// Dump information about file onto standard error
	//dump_format(pFormatCtx, 0, file, 0);


	// Find the first video stream
	videoStream=-1;
	for(i=0; i<pFormatCtx->nb_streams; i++)
		if(pFormatCtx->streams[i]->codec->codec_type==CODEC_TYPE_VIDEO)
		{
			videoStream=i;
			break;
		}
		if(videoStream==-1)
			return;

		// Get a pointer to the codec context for the video stream
		pCodecCtx=pFormatCtx->streams[videoStream]->codec;

		// Find the decoder for the video stream
		pCodec=avcodec_find_decoder(pCodecCtx->codec_id);
		if(pCodec==NULL)
			return;

		// Open codec
		if(avcodec_open(pCodecCtx, pCodec)<0)
			return;

		// Allocate video frame
		pFrame=avcodec_alloc_frame();

		// Allocate an AVFrame structure
		pFrameRGB=avcodec_alloc_frame();

		if(!pFrameRGB || !pFrame)
			return;

		// Determine required buffer size and allocate buffer
		numBytes=avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
		buffer=(uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

		avpicture_fill((AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24, pCodecCtx->width, pCodecCtx->height);
		img = cvCreateImage(cvSize(pCodecCtx->width, pCodecCtx->height), IPL_DEPTH_8U, 3);

}

void VideoCapture::reset()
{
	free_mem();
	init();

}

VideoCapture::~VideoCapture(void)
{
	free_mem();
}

//free the memory
void VideoCapture::free_mem()
{
	if (buffer)
		av_free(buffer);

	if (pFrameRGB)
		av_free(pFrameRGB);

	if (pFrame)
		av_free(pFrame);

	if (pCodecCtx)
		avcodec_close(pCodecCtx);

	if (pFormatCtx)
		av_close_input_file(pFormatCtx);

	if (img)
		cvReleaseImage(&img);

}


IplImage*  VideoCapture::retreiveIplImage() {
	int code;
	IplImage* temp;
	do
	{
		temp = retreiveFrame(code);
	} while(temp != NULL && code !=VIDEO_FRAME);

	return temp;

}

IplImage*  VideoCapture::retreiveFrame(int &code)
{

	code=0;
	img_convert_ctx = NULL;

	if (!pFormatCtx)
		return NULL;

	if (av_read_frame(pFormatCtx, &packet)<0)
		return NULL;


	// Is this a packet from the video stream?
	if(packet.stream_index==videoStream)
	{
		code= VIDEO_FRAME;
		// Decode video frame
		avcodec_decode_video(pCodecCtx, pFrame, &frameFinished, packet.data, packet.size);



		// Make sure we have a frame
		if(frameFinished)
		{
			// Convert the image from its native format to IplImage
			img_convert_ctx = sws_getContext(pCodecCtx->width, pCodecCtx->height,pCodecCtx->pix_fmt, pCodecCtx->width, pCodecCtx->height, PIX_FMT_RGB24,SWS_BICUBIC, NULL, NULL, NULL);
			if(img_convert_ctx == NULL)
				return NULL;

			int linesize[4] = {0,0,0,0};
			linesize[0] = img->width*3;
			uint8_t* dst =(uint8_t*) img->imageData;
			sws_scale(img_convert_ctx, pFrame->data,
				pFrame->linesize, 0,
				pCodecCtx->height,
				&dst,
				linesize);
		}
	}

	// Free the packet that was allocated by av_read_frame
	if (img_convert_ctx)
		sws_freeContext(img_convert_ctx);
	av_free_packet(&packet);

	return img;

}

bool VideoCapture::seek_frame(int frame)
{
	int code;

	//at least one frame has to be loaded after initialization
	retreiveFrame(code);


	int org = frame;
	//find the timestamp
	int framerate = pFormatCtx->streams[videoStream]->r_frame_rate.num;
	int timestamp = frame / framerate;

	std::cout << "Frame Rate = " << framerate << endl;
	std::cout << "Time Stamp = " << timestamp << endl;
	std::cout << "Time Stamp int64 = " << (int64_t)(timestamp * AV_TIME_BASE) << endl;

	//seek frame at the timestamp
	int result = av_seek_frame(pFormatCtx, -1, (int64_t)(timestamp * AV_TIME_BASE), 0);
	std::cout << result << "\n";
	int frame_offset = org % pFormatCtx->streams[videoStream]->r_frame_rate.num;
	std::cout << "Frame Offset = " << frame_offset << endl;
	if (result>=0)

		//ignore extra frames in order to get an accurate frame
		for (int i=0;i<frame_offset;i++)
			retreiveFrame(code);
	return result>=0;
}
