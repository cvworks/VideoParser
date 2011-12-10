/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ContourTreeComp.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>

//#include <Tools/cv.h>
#include <Tools/CvUtils.h>

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h>

using namespace vpl;

extern UserArguments g_userArgs;

double ContourTreeComp::Match(const ShapeDescriptor& sd1, 
	const ShapeDescriptor& sd2)
{
	const ContourTree& ct1 = CastDescriptor<ContourTree>(sd1);
	const ContourTree& ct2 = CastDescriptor<ContourTree>(sd2);

	double dist = cvMatchContourTrees(ct1, ct2, CV_CONTOUR_TREES_MATCH_I1, 
		ContourTree::GetParams().similarityThreshold);

	ASSERT_VALID_NUM(dist);
	ASSERT(dist >= 0);

	return dist;
}