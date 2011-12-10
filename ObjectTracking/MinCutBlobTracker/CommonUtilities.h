#include <Tools/cv.h>

using namespace cv;

void log_clear();
void log_text(const char* fmt, ...);
void outputMatrix(const char* title, Mat & m);
void showLabelImage(Mat & labelImage, Mat & colourImage, int n, const string & windowName);


template <class Tp>
struct MatIterator
{
private:
    Mat m;
    Tp* ptr;
    int x, y, rows, cols, step;
    bool scan;

public:
    void reset()
    {
        x = y = 0;
        ptr = m.ptr<Tp>(0);
        scan = (m.total() > 0);
    }

    void init(const Mat & mat)
    {
        m = mat;
        step = m.channels();
        cols = m.cols;
        rows = m.rows;
        reset();
    }

    MatIterator()
    {
        reset();
    }

    MatIterator(const Mat & mat)
    {
        init(mat);
    }

    MatIterator& operator ++()
    {
        if (++x < cols)
        {
            ptr+=step;
        }
        else
        {
            if (++y < rows)
            {
                x=0;
                ptr = m.ptr<Tp>(y);
            }
            else scan = false;
        }
        return *this;
    }

    bool operator !() const
    {
        return scan;
    }

    Tp& operator *() const
    {
        return *ptr;
    }

    Tp& operator [](ptrdiff_t i) const
    {
        return *(ptr + i);
    }

    const int& row(){return y;}
    const int& col(){return x;}

    int point(Point & p)
    {
        p.x = x;
        p.y = y;
    }
};
