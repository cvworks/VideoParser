#include "CvPyrImageSegmenter.h"

//#include <Tools/cv.h>
//#include <Tools/CvUtils.h>
//#include <math.h>

#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

CvPyrImageSegmenter::CvPyrImageSegmenter()
{
	/*std::list<Params> defVals(1), vals;

	g_userArgs.ReadArgs(Name(), "params", 
		"{sigma, scale, min} for each image segmentation. "
		"'sigma' controls the Gaussian smoothing. Larger 'scale' "
		"sets preference for larger regions. "
		"'min' is minimum region size.", defVals, &vals);

	Params minVal(0, 1, 2);

	g_userArgs.CheckMinValues(Name(), "params", minVal);

	const unsigned int n = vals.size();

	m_segImgs.resize(n);
	m_num_ccs.resize(n, 0);

	m_params.reserve(n);
	m_params.assign(vals.begin(), vals.end());*/

	m_threshold1 = 255;
    m_threshold2 = 30;
	m_level = 1;
}

void CvPyrImageSegmenter::Segment(const RGBImg inputImg) 
{
	//cvPyrSegmentation(image0, image1, storage, &comp,
     //                 m_level, m_threshold1 + 1, m_threshold2 + 1);

	/*for (unsigned int i = 0; i < m_segImgs.size(); i++)
	{
		m_num_ccs[i] = 0;

		m_segImgs[i] = segment_image(inputImg, m_params[i].soomthSigma, 
			m_params[i].scalePref, m_params[i].minSize, &m_num_ccs[i]);
	}*/
}

