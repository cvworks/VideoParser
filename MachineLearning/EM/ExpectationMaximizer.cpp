#include "ExpectationMaximizer.h"

const double sqrt_2pi = 2.506628274631000502415765284811;

using namespace cv;

ExpectationMaximizer::ExpectationMaximizer()
{
}

ExpectationMaximizer::ExpectationMaximizer(int _nClusters, int _nDimensions, int bufferSize)
{
    initialize(_nClusters, _nDimensions, bufferSize);
}

void ExpectationMaximizer::initialize(int nClusters, int nDimensions, int bufferSize)
{
    trainingSet.create(bufferSize, nDimensions, CV_32FC1);

    // initialize model's parameters
    params.covs      = NULL;
    params.means     = NULL;
    params.weights   = NULL;
    params.probs     = NULL;
    params.nclusters = nClusters;
    params.cov_mat_type       = CvEM::COV_MAT_SPHERICAL;
    params.start_step         = CvEM::START_AUTO_STEP;
    params.term_crit.max_iter = 30;
    params.term_crit.epsilon  = 0.01;
    params.term_crit.type     = CV_TERMCRIT_ITER|CV_TERMCRIT_EPS;
}


void ExpectationMaximizer::fit(const Mat &image, int sampleStep)
{
    // Generate the training set
    tsCount = 0;
    float* ts = (float*)trainingSet.data;
    const unsigned char *i, *iEnd;
    int channels = image.channels();
    int imageStep = channels * sampleStep;
    int j;

    tsCount=0;
    for(int y=0; y<image.rows; y+=sampleStep)
    {
        for(i = image.ptr<unsigned char>(y), iEnd = i+image.cols*channels; i < iEnd; i+=imageStep, ts+=channels)
        {
            for (j=0; j<channels; j++) ts[j] = (float)i[j];
            tsCount++;
        }
    }

    fitData();
}

void ExpectationMaximizer::fit(const Mat &image, const Mat &mask)
{
    // Generate the training set
    tsCount = 0;
    float* ts = (float*)trainingSet.data;
    const unsigned char *i, *iEnd, *m;
    int step = image.channels();
    int j;

    tsCount=0;
    for(int y=0; y<image.rows; y++)
    {
        i = image.ptr<unsigned char>(y), iEnd = i+image.cols*step;
        m = mask.ptr<unsigned char>(y);

        for(; i != iEnd; i+=step, m++)
        {
            if (*m)
            {
                for (j=0; j<step; j++) ts[j] = (float)i[j];
                ts+=step;
                tsCount++;
            }
        }
    }

    fitData();
}

void ExpectationMaximizer::fit(const Mat & image, const Mat & labelImage, int label)
{
    // Generate the training set
    tsCount = 0;

    float* ts = (float*)trainingSet.data;
    const unsigned char *i, *iEnd;
    const int *l;
    int step = image.channels();
    int j;

    tsCount = 0;

    for(int y = 0; y < image.rows; y++)
    {
        i = image.ptr<unsigned char>(y), iEnd = i + image.cols * step;
        l = labelImage.ptr<int>(y);

        for(; i != iEnd; i += step, l++)
        {
            if (*l == label)
            {
                for (j = 0; j < step; j++) 
					ts[j] = i[j];

                ts += step;
                tsCount++;
            }
        }
    }

    fitData();
}

void ExpectationMaximizer::fit(const vector<Point2d> & points)
{
    tsCount = points.size();

    float *ts = (float*)trainingSet.data;

    for(auto p=points.begin(), pEnd=points.end(); p!=pEnd; p++, ts+=2)
    {
        ts[0] = (float)(*p).x;
        ts[1] = (float)(*p).y;
    }

    fitData();
}

void ExpectationMaximizer::setTrainingSet(const Mat & _trainingSet)
{
    tsCount = _trainingSet.rows;

    _trainingSet.convertTo(trainingSet.rowRange(0,tsCount), CV_64FC1);

    fitData();
}

void ExpectationMaximizer::fitData()
{
    // Estimate the Gaussian Mixture
    em.train(trainingSet.rowRange(0, tsCount), Mat(), params );
}

Mat ExpectationMaximizer::getMeans()
{
    return em.getMeans();
}

vector<Mat> ExpectationMaximizer::getCovs()
{
    vector<Mat> covs;
    em.getCovs(covs);
    return covs;
}

Mat ExpectationMaximizer::getWeights()
{
    return em.getWeights();
}

