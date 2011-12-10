#include "videoloader.h"
#include "main.h"
#include <QMessageBox>


VideoLoader::VideoLoader():img(NULL), start_frame(0), end_frame(0)
{
    reset();
}
void VideoLoader::reset()
{
    reset(config, 1, 0);
}

void VideoLoader::reset(int StartFrame, int EndFrame)
{
    reset(config, StartFrame, EndFrame);
}

void VideoLoader::reset(ConfigStruct* configuration)
{
    reset(configuration, 1, 0);
}

void VideoLoader::set_clip(int StartFrame, int EndFrame)
{
    reset();
    if (StartFrame > 1) start_frame = StartFrame;
    else start_frame = 1;
    end_frame = EndFrame;
    frame_number = start_frame;

    if (!config->imageloader)       //using video files
    {
        //Go to the first frame in the clip
        //for (int i=1; i<start_frame; i++)
        vid.retreiveIplImage();
        vid.seek_frame(start_frame);
    }


}

void VideoLoader::reset(ConfigStruct* configuration, int StartFrame, int EndFrame)
{
    if (img) cvReleaseImage(&img);

    if (StartFrame > 1) start_frame = StartFrame;
    else start_frame = 1;
    end_frame = EndFrame;
    frame_number = start_frame;

    QString filename = configuration->videofilename;
    #ifdef WIN32
           filename.replace("/", "\\");
    #endif

    if (configuration->imageloader)
    {
        int c = configuration->imagefilepattern.count("#");
        number_signs = "";
        for (int i=0;i<c;i++) number_signs += "#";
    }
    else
    {
        vid.set_filename(filename.toAscii().constData());
        if (start_frame > 1)
        {
            //Go to the first frame
            vid.retreiveIplImage();
            vid.seek_frame(start_frame);
        }
    }
}

QString VideoLoader::form_number(int i)
{
    char num[200];
    QString result;
    sprintf(num, "%d", i);
    int c=number_signs.length()-strlen(num);
    for (int i=0;i<c;i++)
        result += "0";
    result += num;
    return result;
}


IplImage* VideoLoader::next_frame()
{
    //if (!config->imageloader && !capture) return NULL;    
    if (end_frame)
    {
        if (frame_number > end_frame) return NULL;
    }

    if (config->imageloader)
    {//loading images
        if (img) cvReleaseImage(&img);
        QString filename = config->imagefilepattern;
        filename.replace(number_signs, form_number(frame_number));
        filename = config->loadingdir + "/" + filename;

        #ifdef WIN32
                filename.replace("/", "\\");
        #endif

        img = cvLoadImage(filename.toAscii().constData());
        frame_number++;
        return img;

    }
    else
    {//loading videos
        img = vid.retreiveIplImage();
        frame_number++;
        return img;
    }

    return NULL;
}


VideoLoader::~VideoLoader()
{
    if (img) cvReleaseImage(&img);
}

int VideoLoader::get_frame_number()
{
    return frame_number;
}

IplImage* VideoLoader::get_image_at(int i)
{
    if (config->imageloader)
    {//loading images
        if (img) cvReleaseImage(&img);
        QString filename = config->imagefilepattern;
        filename.replace(number_signs, form_number(i));
        filename = config->loadingdir + "/" + filename;

        #ifdef WIN32
                filename.replace("/", "\\");
        #endif

        img = cvLoadImage(filename.toAscii().constData());
        //frame_number++;
        return img;
    }
    else
    {
        //vid.retreiveIplImage();
    }
    return NULL;
}

int VideoLoader::get_image_height()
{
    IplImage* first_image = get_image_at(1);
    return first_image->height;
}

int VideoLoader::get_image_width()
{
    IplImage* first_image = get_image_at(1);

    return first_image->width;

}
