/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IMAGE_SEGMENTATION_H_
#define _IMAGE_SEGMENTATION_H_

#include <Tools/VisSysComponent.h>
#include <VideoParser/ImageProcessor.h>

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;

/*!
	@brief Wrapper for a generic image segmentation algorithm
*/
class ImageSegmenter : public VisSysComponent
{
protected:
	struct ColorSpaceParams
	{
		double luminanceWeightFactor;
		double chromaWeightFactor;
		double hueWeightFactor;
	};

	IntImgArray m_segImgs;
	std::vector<int> m_num_ccs;

	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;

	static std::vector<RGBColor> s_colors;

	ColorSpaceParams m_colorSpaceParams;

	//LabImg m_srcLabImg;
	RGBImg m_srcRGBImg;

public:
	//: dissimilarity measure between pixels
	double ColorDiff(const LabColor& Labstd, const LabColor& Labsample) const;

	//: dissimilarity measure between pixels
	double ColorDiff(const RGBColor& c1, const RGBColor& c2) const
	{
		double r = c1.R() - c2.R();
		double g = c1.G() - c2.G();
		double b = c1.B() - c2.B();

		return sqrt(r * r +	g * g +	b * b);
	}

public:	
	ImageSegmenter() 
	{
		// nothing to do
	}

	virtual void ReadParamsFromUserArguments();
	virtual void Initialize(graph::node v);

	virtual std::string GenericName() const
	{
		return "ImageSegmenter";
	}

	unsigned int NumSegmentations() const
	{
		return m_segImgs.size();
	}

	virtual ~ImageSegmenter() { }
	
	virtual void Run()
	{
		if (!m_pImgProcessor)
		{
			ShowMissingDependencyError(ImageProcessor);
			return;
		}

		Segment(m_pImgProcessor->GetRGBImage());
	}

	virtual void Segment(const RGBImg inputImg) = 0;

	virtual void SetScaleParameter(unsigned int i, const double& sig) = 0;
	virtual double GetScaleParameter(unsigned int i) const = 0;

	virtual void SetSmoothingParameter(unsigned int i, const double& sig) = 0;
	virtual double GetSmoothingParameter(unsigned int i) const = 0;
	
	IntImg Regions(unsigned int i = 0) const
	{ 
		return m_segImgs[i]; 
	}

	int NumRegions(unsigned int i = 0) const
	{ 
		return m_num_ccs[i]; 
	}

	RGBImg ColoredSegmentation(int segId) const
	{
		IntImg segImg = m_segImgs[segId];

		unsigned int sz = segImg.size();

		// Make sure that there will be enough colors
		// available by assuming that each pixel is a region
		if (s_colors.size() < sz)
		{
			s_colors.resize(sz);

			for (unsigned int i = 0; i < sz; ++i)
				s_colors[i] = RandomRGBColor();
		}

		RGBImg colImg(segImg.ni(), segImg.nj());
			
		for (unsigned int j = 0; j < segImg.nj(); ++j)
			for (unsigned int i = 0; i < segImg.ni(); ++i)	
				colImg(i, j) = s_colors[segImg(i, j)];

		return colImg;
	}

	virtual int NumOutputImages() const 
	{ 
		return 2; 
	}

	//!	Returns the parameters for a given an output index 'i'
	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		if (i == 0)
		{
			InitArrays(1, pMinVals, pMaxVals, pSteps,
				0, NumSegmentations() - 1, 1);
		}
		else
		{
			InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 0, 1);
		}
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		return (i == 0) ? "Image segmentation" : "Input image";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{
		if (dii.outputIdx == 0)
		{
			ASSERT(dii.params.size() == 1);

			int segIdx = (int)dii.params.front();

			RGBImg colImg = ColoredSegmentation(segIdx);

			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(colImg);

			std::ostringstream oss;
			oss << "Segmentation " << segIdx + 1 << " out of " 
				<< NumSegmentations() << ".";
			dio.message = oss.str();
		}
		else
		{
			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(m_srcRGBImg);
		}
	}

	virtual StrArray Dependencies() const
	{
		return StrArray(1, "ImageProcessor");
	}
};

} // namespace vpl

#endif //_IMAGE_SEGMENTATION_H_
