/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include "Blob.h"

namespace vpl {

class ImageProcessor;
class BackgroundSubtractor;
class ImageSegmenter;

/*!
	@brief Finds blobs in masks.
*/
class BlobFinder : public VisSysComponent
{
	struct Params
	{
		double minBlobSize;
	};

protected:
	ByteImg m_maskImg;
	FloatImg m_srcGreyImg;

	Params m_params;

	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	std::shared_ptr<const ImageSegmenter> m_pImgSegmenter;
	std::shared_ptr<const BackgroundSubtractor> m_pBkdSubtractor;

	std::vector<std::vector<cv::Point>> m_contours;
	std::vector<cv::Vec4i> m_hierarchy;
	std::vector<BlobPtr> m_blobs;

public:	

	std::vector<BlobPtr> GetBlobs() const
	{
		return m_blobs;
	}

	ByteImg GetBlobMask() const
	{
		// todo: create the mask here
		return ByteImg();
	}

	virtual void Initialize(graph::node v);

	virtual void ReadParamsFromUserArguments();

	virtual void Run();

	/*!
		The parameters range from 0 = ALL regions to the number 
		of regions found. Then, the selection of a specific
		region is given by param - 1.
	*/
	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		//InitArrays(2, pMinVals, pMaxVals, pSteps,
		//	0, m_regionPyr.height(), 1);
	}

	virtual int NumOutputImages() const
	{
		return 2;
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		return (i == 0) ? "Contours" : "Regions";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, 
		DisplayInfoOut& dio) const;

	virtual std::string ClassName() const
	{
		return "BlobFinder";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ImageSegmenter");
		deps.push_back("BackgroundSubtractor");
		deps.push_back("ImageProcessor");

		return deps;
	}

	//virtual void Draw(const DisplayInfoIn& dii) const;
	//virtual bool OnGUIEvent(const UserEventInfo& uei);

	//ByteImg CreateContourImage(int segmentIdx, int regionIdx) const;

	//RGBImg CreateSalientRegionImage(int segmentIdx, int regionIdx) const;
};

} // namespace vpl

