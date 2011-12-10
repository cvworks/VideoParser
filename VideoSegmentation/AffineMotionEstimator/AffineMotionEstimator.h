#ifndef _AFFINE_MOTION_ESTIMATOR_H_
#define _AFFINE_MOTION_ESTIMATOR_H_

#include "../MotionEstimator.h"

namespace vpl {

class AffineMotionEstimator : public MotionEstimator
{   
    float sigma, sigma_init, omega, rate;
    int levels, iters, filter, affine, multiple;
    
    float u, v, a0, a1, a2, a3, a4, a5;
    int nx, ny, nEstimatedMotions;
   
    MotionMaps motionMaps[MAX_NUM_MOTIONS];
    MotionMaps curMaps;
    
    bool EnoughInliers(float* mask);
    
public:
    AffineMotionEstimator();
    void SetDefaultParams();
    
    bool Estimate(FloatImg curFrame, FloatImg prevFrame);
    
    int EstimatedMotions() const   { return nEstimatedMotions; }
    
    FloatImg GetMotionMap(int nMotion, MotionMaps::MAPTYPE t) const
    {
    	ASSERT(nMotion >= 0 && nMotion <= MAX_NUM_MOTIONS);
    	return motionMaps[nMotion].GetMap(t);
    }

	virtual int NumOutputImages() const 
	{ 
		return 5; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Mean image";
			case 1: return "Weights";
			case 2: return "Outliers";
			case 3: return "Inliers";
			case 4: return "Warped";
		}

		ASSERT(false);

		return "";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{
		if (!HasOutput())
		{
			dio.imageType = VOID_IMAGE;
			return;
		}

		FloatImg img;

		switch (dii.outputIdx)
		{
			case 0: img = GetMotionMap(0, MotionMaps::MEAN); break;
			case 1: img = GetMotionMap(0, MotionMaps::WEIGHTS); break;
			case 2: img = GetMotionMap(0, MotionMaps::OUTLIERS); break;
			case 3: img = GetMotionMap(0, MotionMaps::USED_PTS); break;
			case 4: img = GetMotionMap(0, MotionMaps::WARPED); break;
			default: ASSERT(false); break;
		}

		dio.imageType = FLOAT_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(img);
	}

	virtual std::string ClassName() const
	{
		return "AffineMotionEstimator";
	}
};

} // namespace vpl

#endif //_AFFINE_MOTION_ESTIMATOR_H_
