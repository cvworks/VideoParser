/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <sstream>
#include "../ImageSegmenter.h"
#include "FHGraphSegmenter.h"
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
class FHImageSegmenter : public ImageSegmenter
{
	struct Params {
		float soomthSigma; //!< Used to smooth the input image before segmenting it
		float scalePref;   //!< Scale preference for the threshold function
		int minSize;       //!< Minimum component size enforced by post-processing

		Params(float s = 0.5, float k = 500, int m = 20) 
		{
			soomthSigma = s;
			scalePref   = k;
			minSize     = m;
		}

		bool operator>(const Params& rhs) const
		{
			return (soomthSigma > rhs.soomthSigma || 
				scalePref > rhs.scalePref || minSize > rhs.minSize);
		}

		bool operator<(const Params& rhs) const
		{
			return (soomthSigma < rhs.soomthSigma || 
				scalePref < rhs.scalePref || minSize < rhs.minSize);
		}

		/*!
			Writes a text representation of a Param object using the format '{sig,k,sz}'. 
		*/
		friend std::ostringstream& operator<<(std::ostringstream& os, const Params& p)
		{ 
			os << "{" << p.soomthSigma << "," 
				<< p.scalePref << "," << p.minSize << "}"; 

			return os;
		}
		
		/*!
			Reads a text representation of a Param object '{sig,k,sz}'. It validates
			the position of the brackets and commans and throws an exception of thay
			are invalid. For example, since spaces are not allowed, the text '{1, 2,3}'
			will raise an exception.
		*/
		friend std::istringstream& operator>>(std::istringstream& is, Params& p) 
		{ 
			char sep[] = "{,,}";
			std::string str(sep);
			int i = 0;

			is >> sep[0] >> p.soomthSigma >> sep[1] 
				>> p.scalePref >> sep[2] >> p.minSize >> sep[3];

			if (str != sep)
				THROW_BASIC_EXCEPTION("Invalid FHImageSegmenter::Param format");

			return is;
		}
	};

	std::vector<Params> m_params;
	
	FHGraphSegmenter m_gs;

protected:
	int CreateGraph(FHGraphSegmenter::Edges& edges, RGBImg img, double sigma) const;

	IntImg segment_image(const RGBImg img, float sigma, float c, 
		int min_size, int* num_ccs);

public:
	virtual void ReadParamsFromUserArguments();
	virtual void Initialize(graph::node v);
	virtual bool OnGUIEvent(const UserEventInfo& uei);

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
		m_params[i].scalePref = (float) k; 
	}

	/*!
		Gets the scale parameter.

		@see SetScaleParameter
	*/
	double GetScaleParameter(unsigned int i) const 
	{ 
		return m_params[i].scalePref; 
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
		m_params[i].soomthSigma = (float) sig;
	}

	/*!
		Gets the smoothing parameter.

		@see SetSmoothingParameter
	*/
	double GetSmoothingParameter(unsigned int i) const 
	{ 
		return m_params[i].soomthSigma; 
	}

	virtual std::string ClassName() const
	{
		return "FHImageSegmenter";
	}
};

} // namespace vpl


