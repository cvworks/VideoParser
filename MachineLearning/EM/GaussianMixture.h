#pragma once

#include <Tools/cv.h>

class GaussianMixtureBase
{
public:
    GaussianMixtureBase();
    GaussianMixtureBase::~GaussianMixtureBase();

    void initialize(const cv::Mat & _means, const std::vector<cv::Mat> & _covs, 
		const cv::Mat & _weights);

    double prob(double* x);
    int classify(double* x);

    cv::Mat getMeans(){return means;}
    cv::Mat getWeights(){return weights;}
    std::vector<cv::Mat> & getCovs(){return covs;}

    void copyTo(GaussianMixtureBase & gm);


protected:
    virtual void getExpArg(double* x, int k)=0;

    // Gaussian Mixture Parameters
    int nClusters, nDimensions;
    cv::Mat means, weights;
    std::vector<cv::Mat> covs, covs_inv;

    // Derived Parameters
    double *probCoeffs;          // The constant coefficient for each Gaussian in the mixture (before weighting)
    double *weightedCoeffs;      // The weighted coefficient for each Gaussian in the mixture
    double *neg2logProbCoeffs;   // negative two times the log of the probability coefficients
    double *diff;                // temporary array used for computing the probability of a std::vector.
    double *mPtr;                // Pointer to the mean matrix
    double *wPtr;                // Pointer to the weight matrix
    double *cPtr;                // Pointer to an inverse covariance matrix

    double expArg;               // argument inside the exponent in the gaussian expression
};

class GaussianMixture1D : public GaussianMixtureBase
{
public:
    void classifyPixels(const cv::Mat & src, cv::Mat & dst, const cv::Mat  &mask = cv::Mat());
    void classifyPixels(const cv::Mat & src, cv::Mat & dst, const cv::Mat & labelImage, int label);

private:
    void getExpArg(double *x, int k);
};

class GaussianMixture2D : public GaussianMixtureBase
{
public:
    double prob(cv::Point2d & pt);
    int classify(cv::Point2d & pt);

private:
    void getExpArg(double *x, int k);
    double point[2];
};

class GaussianMixture3D : public GaussianMixtureBase
{
public:
    void showClusters(const cv::Mat & src, cv::Mat & dst);
    void showClusters(const cv::Mat & src, cv::Mat & dst, const cv::Mat & labelImage, int label);
    void classifyPixels(const cv::Mat & src, cv::Mat & dst, const cv::Mat  &mask = cv::Mat());
    void classifyPixels(const cv::Mat & src, cv::Mat & dst, const cv::Mat & labelImage, int label);

    void getMeanColour(unsigned char* pixel, unsigned char* meanColour);

private:
    void getExpArg(double *x, int k);
};


