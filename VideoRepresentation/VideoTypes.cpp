/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "CvVideo.h"
#include "VXLVideo.h"
#include "ImgSeqVideo.h"
#include "SyntheticVideo.h"
#include "MultiVideo.h"

using namespace vpl;

VideoPtr vpl::ConstructVideoObject(VideoType type)
{
	switch (type)
	{
		case MULTI_VIDEO:     return VideoPtr(new MultiVideo());
		case SYNTHETIC_VIDEO: return VideoPtr(new SyntheticVideo());
		case IMGSEQ_VIDEO:    return VideoPtr(new ImgSeqVideo());
		case CV_VIDEO:        return VideoPtr(new CvVideo());

		#ifdef HAS_VXL_VIDEO
		case VXL_VIDEO:       return VideoPtr(new VXLVideo());
		#endif
	}

	return VideoPtr();
}



