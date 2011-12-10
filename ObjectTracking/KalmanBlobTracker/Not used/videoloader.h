#ifndef VIDEOLOADER_H
#define VIDEOLOADER_H

#include "cv.h"
#include "highgui.h"
#include <QString>
#include "VideoCapture.h"
#include "main.h"
#include <iostream>

using namespace std;

const double TIME_STEP = 0.1;



class VideoLoader
{
public:
    VideoLoader();
    IplImage* next_frame();
    ~VideoLoader();
    void reset();
    void reset(ConfigStruct* configuration);
    void reset(int StartFrame, int EndFrame);
    void reset(ConfigStruct* configuration, int StartFrame, int EndFrame);
    int get_frame_number();
    IplImage* get_image_at(int i);
    int get_image_height();
    int get_image_width();
    void set_clip(int StartFrame, int EndFrame);


private:
    int frame_number;
    int start_frame;
    int end_frame;
    QString form_number(int);
    QString number_signs;
    VideoCapture vid;
    IplImage* img;
    ConfigStruct* temp_con;

};

#endif // VIDEOLOADER_H
