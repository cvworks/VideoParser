/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _CV_PYR_IMAGE_SEGMENTER_H_
#define _CV_PYR_IMAGE_SEGMENTER_H_

#include <sstream>
#include "ImageSegmenter.h"
#include <Tools/Exceptions.h>

namespace vpl {

/*!
	@brief Wrapper for the Felzenszwalb-Huttenlocher image segmentation algorithm.

	For color images the segmentation algorithm runs three times, once for each of 
	the red, green and blue color planes, and then it intersects these three sets 
	of components. Speciffically, it puts two neighboring pixels in the same 
	component when they appear in the same component in all three of the color 
	plane segmentations.

	The algorith is described in the paper "Effcient Graph-Based Image Segmentation"
	by Pedro F. Felzenszwalb and Daniel P. Huttenlocher.
*/
class CvPyrImageSegmenter : public ImageSegmenter
{	
	double m_threshold1;
	double m_threshold2;
	int m_level;
		
public:
	CvPyrImageSegmenter();

	void Segment(const RGBImg inputImg);

	/*!
		Sets the scale parameter.

		The value of k that is used to compute the threshold function $\pi$. 
		The algorithms uses the function $\pi(C) = k / |C|$, where $|C|$ is the 
		number of elements in $C$. Thus k effectively sets a scale of
		observation, in that a larger k causes a preference for larger components.

		Recomended values: for 128x128 images (eg, the COIL database), use k = 150. 
		For 320x240 or larger images, use k = 300.
	*/
	void SetScaleParameter(unsigned int i, const double& k) 
	{
		ASSERT(k > 0); 
		//m_params[i].scalePref = (float) k; 
	}

	/*!
		Gets the scale parameter.

		@see SetScaleParameter
	*/
	double GetScaleParameter(unsigned int i) const 
	{ 
		//return m_params[i].scalePref; 
		return 0;
	}

	/*!
		Sets the smoothing parameter.

		It uses a Gaussian ¯lter to smooth the image slightly before 
		computing the edge weights, in order to compensate for digitization 
		artifacts. A good approach is to use a Gaussian with $\sigma = 0.8$, 
		which does not produce any visible change to the image but helps remove artifacts.	

		@param sig sigma for teh Gaussian function. If zero, no smoothing is performed.
	*/
	void SetSmoothingParameter(unsigned int i, const double& sig)
	{
		ASSERT(sig >= 0); 
		//m_params[i].soomthSigma = (float) sig;
	}

	/*!
		Gets the smoothing parameter.

		@see SetSmoothingParameter
	*/
	double GetSmoothingParameter(unsigned int i) const 
	{ 
		//return m_params[i].soomthSigma; 
		return 0;
	}

	virtual std::string ClassName() const
	{
		return "CvPyrImageSegmenter";
	}
};

} // namespace vpl

#endif //_CV_PYR_IMAGE_SEGMENTER_H_
