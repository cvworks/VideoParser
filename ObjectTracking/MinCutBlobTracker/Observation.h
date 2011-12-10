#pragma once

#include <memory>
#include "CCExtractor.h"

struct BasicBlob
{
    cv::Point2d meanFlow;
    GaussianMixture3D colourModel;
    GaussianMixture2D motionModel;

    virtual int ID()=0;
};

struct Object : public BasicBlob
{
    int id;
    cv::Rect rect;
    int size;
    cv::Scalar colour;
    cv::Mat mask;

    Object(int _id, cv::Scalar & _colour)
    {
        id = _id;
        //cv::RNG rng; //! Random number generator
        colour = _colour;
        mask.create(250,200,CV_8UC1);
        mask.setTo(cv::Scalar(0));
    }

    int ID(){return id;}
};

struct Observation : public BasicBlob
{
    ConnectedComponentPtr cc;
    std::vector<std::shared_ptr<Object>> associatedObjects;

    Observation()
    {
        cc = 0;
    }
    Observation(ConnectedComponentPtr _cc)
    {
        cc = _cc;
    }

    int ID()
	{
		return cc->id;
	}
};
