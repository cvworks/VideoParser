/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _MOTION_ESTIMATOR_H_
#define _MOTION_ESTIMATOR_H_

#include <Tools/VisSysComponent.h>

namespace vpl {

#define MAX_NUM_MOTIONS 3
#define NUM_MAPS 10

class MotionMaps
{
	FloatImg maps[NUM_MAPS];
	
public:
    enum MAPTYPE {
    	OUTLIERS, WEIGHTS, 
    	LAP1, LAP2, IX, IY, IT, 
    	WARPED, MEAN, USED_PTS
    };
    
    struct PTRS {
	    float *outlier_img, *weights;
	    float *lap1, *lap2, *Ix, *Iy, *It;
	    float *warped, *mean, *used_pts;
    };
        
    void Init(int nx, int ny);
    void operator=(const MotionMaps& rhs);
    PTRS GetPtrs();
    
    FloatImg GetMap(MAPTYPE t) const
    { 
    	ASSERT(t >= 0 && t <= NUM_MAPS); 
    	return maps[t]; 
    }
};

/*!
	Abstract motion estimation algorithm
*/
class MotionEstimator : public VisSysComponent
{
private:
	bool m_hasOutput;

protected:
	bool HasOutput() const
	{
		return m_hasOutput;
	}

public:
	virtual ~MotionEstimator() { }
	
	virtual bool Estimate(FloatImg curFrame, FloatImg prevFrame) = 0;
	
	virtual FloatImg GetMotionMap(int nMotion, MotionMaps::MAPTYPE t) const = 0;

	virtual void Run();

	virtual StrArray Dependencies() const
	{
		return StrArray(1, "ImageProcessor");
	}
};

} // namespace vpl

#endif //_MOTION_ESTIMATOR_H_
