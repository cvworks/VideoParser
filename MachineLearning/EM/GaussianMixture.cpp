#include "GaussianMixture.h"
#include <Tools/BasicUtils.h>

const double sqrt_2pi = 2.506628275f;

using namespace cv;

GaussianMixtureBase::GaussianMixtureBase()
{
    diff = probCoeffs = weightedCoeffs = neg2logProbCoeffs = 0;
}

GaussianMixtureBase::~GaussianMixtureBase()
{
    if (diff) delete [] diff;
    if (probCoeffs) delete [] probCoeffs;
    if (weightedCoeffs) delete [] weightedCoeffs;
    if (neg2logProbCoeffs) delete [] neg2logProbCoeffs;
}


void GaussianMixtureBase::initialize(const Mat & _means, const vector<Mat> & _covs, const Mat & _weights)
{
    ASSERT(_means.rows > 0 && _means.cols > 0);
    ASSERT(_weights.cols == _means.rows && _weights.rows == 1 && _weights.channels() ==1);
    ASSERT(_covs.size() == _means.rows);

    nClusters = _means.rows;
    nDimensions = _means.cols;

    // Set the Gaussian Mixture Parameters
    covs.clear();
    covs_inv.clear();

    // Set the means
    _means.convertTo(means, CV_64FC1);
    mPtr = (double*)means.data;

    // Set the weights
    _weights.convertTo(weights, CV_64FC1);
    wPtr = (double*)weights.data;


    double tmp1, tmp2;

    tmp1 = sqrt_2pi;
    for(int j=1; j<nDimensions; j++) tmp1*=tmp1;

    probCoeffs = new double[nClusters];
    weightedCoeffs = new double[nClusters];
    neg2logProbCoeffs = new double[nClusters];
    diff = new double[nDimensions];

    // For each cluster...
    for(int k=0; k<nClusters; k++)
    {
        // Set the covariance and inverse covariance matrices
        covs.push_back(Mat());
        _covs[k].convertTo(covs[k], CV_64FC1);
        covs_inv.push_back(covs[k].inv());

        // Set the constant probability coefficient
        tmp2 = (double)sqrt(determinant(covs[k]));
        probCoeffs[k] = 1.f / (  tmp1 * tmp2 );

        // Set the weigthed probability coefficient
        weightedCoeffs[k] =  wPtr[k] * probCoeffs[k];

        neg2logProbCoeffs[k] = -2.f * log(probCoeffs[k]);
    }
}

double GaussianMixtureBase::prob(double* x)
{
    double p=0;
    for(int k=0; k<nClusters; k++)
    {
        getExpArg(x, k);
        p += weightedCoeffs[k] * exp( -0.5 * expArg );
    }
    return p;
}

int GaussianMixtureBase::classify(double* x)
{
    getExpArg(x,0);

    double minP = neg2logProbCoeffs[0] + expArg;
    int minID = 0;
    double p;

    for(int k=1; k<nClusters; k++)
    {
        getExpArg(x,k);

        p = neg2logProbCoeffs[k] + expArg;

        if (p < minP)
        {
            minP = p;
            minID = k;
        }
    }

    return minID;
}

void GaussianMixtureBase::copyTo(GaussianMixtureBase & gm)
{
    gm.initialize(means, covs, weights);
}

/***** GaussianMixture1D *************************************************/

void GaussianMixture1D::getExpArg(double* x, int k)
{
    *diff = (double)*x - mPtr[k];
    cPtr = (double*)covs_inv[k].data;
    expArg = (*diff) * (*diff) * (*cPtr);
}

void GaussianMixture1D::classifyPixels(const Mat & src, Mat & dst, const Mat &mask)
{
    if (src.type() != CV_8UC1) return;

    // Generate the training set
    const unsigned char *s, *sEnd, *m;
    unsigned char *d;

    Mat meanColours;
    means.convertTo(meanColours, CV_8UC1);
    unsigned char* c = (unsigned char*)meanColours.data;

    double x;

    dst.create(src.size(), src.type());

    if (mask.total() == src.total())
    {
        for(int y=0; y<src.rows; y++)
        {
            s = src.ptr<unsigned char>(y), sEnd = s+src.cols;
            d = dst.ptr<unsigned char>(y);
            m = mask.ptr<unsigned char>(y);

            for(; s != sEnd; s++, d++, m++)
            {
                if (*m)
                {
                    x = (double)*s;
                    *d = c[classify(&x)];
                }
                else *d=0;
            }
        }
    }
    else
    {
        for(int y=0; y<src.rows; y++)
        {
            s = src.ptr<unsigned char>(y), sEnd = s+src.cols;
            d = dst.ptr<unsigned char>(y);

            for(; s != sEnd; s++, d++)
            {
                x = (double)*s;
                *d = c[classify(&x)];
            }
        }
    }
}


/***** GaussianMixture2D *************************************************/

void GaussianMixture2D::getExpArg(double* x, int k)
{
    diff[0] = (double)x[0] - mPtr[k*2];
    diff[1] = (double)x[1] - mPtr[k*2+1];

    cPtr = (double*)covs_inv[k].data;

    expArg = diff[0]*(cPtr[0]*diff[0] + cPtr[1]*diff[1]) +
             diff[1]*(cPtr[2]*diff[0] + cPtr[3]*diff[1]);
}

double GaussianMixture2D::prob(Point2d & pt)
{
    point[0] = pt.x;
    point[1] = pt.y;
    return GaussianMixtureBase::prob(point);
}

int GaussianMixture2D::classify(Point2d & pt)
{
    point[0] = pt.x;
    point[1] = pt.y;
    return GaussianMixtureBase::classify(point);
}

/***** GaussianMixture3D *************************************************/

void GaussianMixture3D::getExpArg(double* x, int k)
{
    diff[0] = x[0] - mPtr[k*3];
    diff[1] = x[1] - mPtr[k*3+1];
    diff[2] = x[2] - mPtr[k*3+2];

    cPtr = (double*)covs_inv[k].data;

    expArg = diff[0]*(cPtr[0]*diff[0] + cPtr[1]*diff[1] + cPtr[2]*diff[2]) +
             diff[1]*(cPtr[3]*diff[0] + cPtr[4]*diff[1] + cPtr[5]*diff[2]) +
             diff[2]*(cPtr[6]*diff[0] + cPtr[7]*diff[1] + cPtr[8]*diff[2]);
}

void GaussianMixture3D::classifyPixels(const Mat & src, Mat & dst, const Mat &mask)
{
    if (src.type() != CV_8UC3) return;

    // Generate the training set
    const unsigned char *s, *sEnd, *m;
    unsigned char *d;
    const int step = 3;

    Mat meanColours;
    means.convertTo(meanColours, CV_8UC1);
    unsigned char* c = (unsigned char*)meanColours.data;

    double sam[step];
    int colourID;
    int j;

    dst.create(src.size(), src.type());

    if (mask.total() == src.total())
    {
        for(int y=0; y<src.rows; y++)
        {
            s = src.ptr<unsigned char>(y), sEnd = s+src.cols*step;
            d = dst.ptr<unsigned char>(y);
            m = mask.ptr<unsigned char>(y);

            for(; s != sEnd; s+=step, d+=step, m++)
            {
                if (*m)
                {
                    for (j=0; j<step; j++) sam[j] = (double)s[j];
                    colourID = classify(sam)*step;
                    for (j=0; j<step; j++) d[j] = c[colourID+j];
                }
                else
                {
                    for (j=0; j<step; j++) d[j]=0;
                }
            }
        }
    }
    else
    {
        for(int y=0; y<src.rows; y++)
        {
            s = src.ptr<unsigned char>(y), sEnd = s+src.cols*step;
            d = dst.ptr<unsigned char>(y);

            for(; s != sEnd; s+=step, d+=step)
            {
                for (j=0; j<step; j++) sam[j] = (double)s[j];
                colourID = classify(sam)*step;
                for (j=0; j<step; j++) d[j] = c[colourID+j];
            }
        }
    }
}

void GaussianMixture3D::showClusters(const Mat & src, Mat & dst)
{
    if (src.type() != CV_8UC3 || dst.type() != CV_8UC3) return;

    // Generate the training set
    const unsigned char *s, *sEnd;
    unsigned char *d;
    const int step = 3;

    Mat meanColours;
    means.convertTo(meanColours, CV_8UC1);
    unsigned char* c = (unsigned char*)meanColours.data;

    double sf[step];
    int colourID;

    for(int y=0; y<src.rows; y++)
    {
        s = src.ptr<unsigned char>(y), sEnd = s+src.cols*step;
        d = dst.ptr<unsigned char>(y);

        for(; s != sEnd; s+=step, d+=step)
        {
            sf[0] = (double)s[0];
            sf[1] = (double)s[1];
            sf[2] = (double)s[2];

            colourID = classify(sf)*step;

            d[0] = c[colourID];
            d[1] = c[colourID+1];
            d[2] = c[colourID+2];
        }
    }
}

void GaussianMixture3D::showClusters(const Mat & src, Mat & dst, const Mat & labelImage, int label)
{
    if (src.type() != CV_8UC3 || dst.type() != CV_8UC3 || labelImage.type() != CV_32SC1) return;

    // Generate the training set
    const unsigned char *s, *sEnd;
    const int *l;
    unsigned char *d;
    const int step = 3;

    Mat meanColours;
    means.convertTo(meanColours, CV_8UC1);
    unsigned char* c = (unsigned char*)meanColours.data;

    double sf[step];
    int colourID;

    for(int y=0; y<src.rows; y++)
    {
        s = src.ptr<unsigned char>(y), sEnd = s+src.cols*step;
        d = dst.ptr<unsigned char>(y);
        l = labelImage.ptr<int>(y);

        for(; s != sEnd; s+=step, d+=step, l++)
        {
            if (*l == label)
            {
                sf[0] = (double)s[0];
                sf[1] = (double)s[1];
                sf[2] = (double)s[2];

                colourID = classify(sf)*step;

                d[0] = c[colourID];
                d[1] = c[colourID+1];
                d[2] = c[colourID+2];
            }
        }
    }
}
