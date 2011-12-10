#pragma once

// ExpectationMaximizer is a wrapper class for OpenCV's ExpectationMaximization class.
// It's function is to estimate a Gaussian Mixture from N-dimensional data.
#include <Tools/cv.h>
#include <opencv2/ml/ml.hpp>

class ExpectationMaximizer
{
public:
    ExpectationMaximizer();
    ExpectationMaximizer(int _nClusters, int nDimensions, int bufferSize);
    void initialize(int nClusters, int nDimensions, int bufferSize);
    void fit(const cv::Mat & image, int sampleStep=1);
    void fit(const cv::Mat & image, const cv::Mat & mask);
    void fit(const cv::Mat & image, const cv::Mat & labelImage, int label);
    void fit(const std::vector<cv::Point2d> & points);
    void setTrainingSet(const cv::Mat & _trainingSet);

    // Getters
    cv::Mat getMeans();
    std::vector<cv::Mat> getCovs();
    cv::Mat getWeights();

private:
    void fitData();

    // OpenCV EM objects for estimating the Gaussian Mixture
    cv::EMParams params;
    cv::ExpectationMaximization em;

    // Training set for Gaussian Mixture
    cv::Mat trainingSet;
    int tsCount;
};

