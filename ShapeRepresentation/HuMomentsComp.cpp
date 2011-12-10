/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "HuMomentsComp.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>

#include <Tools/cv.h>

using namespace vpl;

extern UserArguments g_userArgs;

/*!

*/
double HuMomentsComp::Match(const ShapeDescriptor& sd1, 
	const ShapeDescriptor& sd2)
{
	static int methods[3] = {CV_CONTOURS_MATCH_I1, CV_CONTOURS_MATCH_I2, 
		CV_CONTOURS_MATCH_I3};

	const HuMoments& hm1 = CastDescriptor<HuMoments>(sd1);
	const HuMoments& hm2 = CastDescriptor<HuMoments>(sd2);

	int id = HuMoments::GetParams().methodId;

	ASSERT(false); // OpenCv 2.3.1 change something so comment out code below
	/*if (id < 3)
		return cv::matchShapes(hm1, hm2, methods[id], 0);
	else if (id == 3)
		return cv::matchShapes(hm1, hm2, methods[0], 0) + 
			cv::matchShapes(hm1, hm2, methods[1], 0);
	else if (id == 4)
		return cv::matchShapes(hm1, hm2, methods[0], 0) + 
			cv::matchShapes(hm1, hm2, methods[2], 0);*/

	ASSERT(false);

	return 0;
}