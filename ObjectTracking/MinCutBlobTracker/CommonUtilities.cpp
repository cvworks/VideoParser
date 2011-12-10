#include "CommonUtilities.h"
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
//#include "highgui.h"

void log_clear()
{
    remove("logfile.txt");
}

void log_text(const char* fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    FILE* fp  = fopen("logfile.txt", "a");

    vfprintf(fp, fmt, ap);
    fclose(fp);

    va_end(ap);
}

void outputMatrix(const char* title, Mat & m)
{
    log_text("%s\trows=%i\tcols=%i\ttype=%i\n", title, m.rows, m.cols, CV_64FC1);

    if (m.type() == CV_8UC1)
    {
        for (int i=0; i<m.rows; i++)
        {
            unsigned char* p = m.ptr<unsigned char>(i);
            for (int j=0; j<m.cols; j++)
            {
                log_text("%i\t",(int)p[j]);
            }
            log_text("\n");
        }
    }
    else if (m.type() == CV_32FC1)
    {
        for (int i=0; i<m.rows; i++)
        {
            float* p = m.ptr<float>(i);
            for (int j=0; j<m.cols; j++)
            {
                log_text("%f\t",p[j]);
            }
            log_text("\n");
        }
    }
    else if (m.type() == CV_32SC1)
    {
        for (int i=0; i<m.rows; i++)
        {
            int* p = m.ptr<int>(i);
            for (int j=0; j<m.cols; j++)
            {
                log_text("%i\t",p[j]);
            }
            log_text("\n");
        }
    }
    else if (m.type() == CV_64FC1)
    {
        for (int i=0; i<m.rows; i++)
        {
            double* p = m.ptr<double>(i);
            for (int j=0; j<m.cols; j++)
            {
                log_text("%e\t",p[j]);
            }
            log_text("\n");
        }
    }
}
