/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini, Chris Whiten
 *-----------------------------------------------------------------------*/
#pragma once

//#include <boost/geometry.hpp>
#include <Tools/VisSysComponent.h>
#include "RegionPyramid.h"
#include <set>


//#include <boost/geometry/geometries/point_xy.hpp>
//#include <boost/geometry/geometries/polygon.hpp>

namespace vpl {

// Declare prototypes of expected parent components
class ImageSegmenter;
class BackgroundSubtractor;
class ImageProcessor;
class BlobTracker;

/*!
	@brief Analyzer of the region adjacencies in the output 
	of an ImageSegmenter.

	It lets the user select regions and create groups using the GUI.
	The selected regions are saved to a YAML output file. In addition,  
	the user can be allowed to label the data in any order and
	not necessarily in one single processing of the video by turning on
	the  the system's cache. When the cache is active, all the
	labeling is cached (ie, loaded/saved) and can be updated at any time.
	The output of the YAML file should only be requested once it is 
	known that the labeling of each frame will be final. For example, the
	labeling can be done with the cache on and the YAML output off. Once
	all frames have been labeled (perhaps over several days), the YAML 
	output can be turned on (teh cache stays on) and the video can
	be processed without user intervention. In this case, the user's
	labelign will be read from the chache and the sent to the YAML output.
*/
class RegionAnalyzer : public VisSysComponent
{
protected:
	RegionPyramid m_regionPyr;      //!< Current set of regions
	RegionPyramid m_savedRegionPyr; //!< Regions saved from previous processing

	std::vector<IntImg> m_inputImgs;

	ByteImg m_foregroundImg;
	bool m_hasForegroundMask;

	ByteImg m_blobImg;
	bool m_hasBlobMask;

	unsigned m_currentGroupId; //!< Used to group regions selected by the user
	unsigned m_currentSegmentationId; //!< Used to group regions selected by the user
	bool m_saveUserSelection;

	RGBImg m_srcRGBImg;
	FloatImg m_srcGreyImg;

	//Parent components
	std::shared_ptr<const ImageSegmenter> m_pImgSegmenter;
	std::shared_ptr<const BackgroundSubtractor> m_pBkdSubtractor;
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	std::shared_ptr<const BlobTracker> m_pBlobTracker;

protected:
	//void TraceContour(unsigned int regionId);

	RegionArray FindRegions(IntImg inputImg, int num_regions,
		RGBImg coleredSegImg);

	
public:	
	virtual void Initialize(graph::node v);
	std::set<int> getSalientRegions() const;

	virtual void ReadParamsFromUserArguments();

	void GetImgDimensions(unsigned* w, unsigned* h) const
	{
		if (m_inputImgs.empty())
		{
			*w = 0;
			*h = 0;
		}
		else
		{
			*w = m_inputImgs.front().ni();
			*h = m_inputImgs.front().nj();
		}
	}

	const RegionPyramid& GetRegions() const
	{
		return m_regionPyr;
	}

	virtual void Run();

	virtual void Clear()
	{
		m_regionPyr.clear();
		m_inputImgs.clear();

		m_savedRegionPyr.clear();
		m_currentGroupId = 0;
		m_currentSegmentationId = 0;
	}

	/*!
		The parameters range from 0 = ALL regions to the number 
		of regions found. Then, the selection of a specific
		region is given by param - 1.
	*/
	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(2, pMinVals, pMaxVals, pSteps,
			0, m_regionPyr.height(), 1);

		// Param 0 is the number of segmentation minus 1
		pMaxVals->at(0) = (m_regionPyr.empty()) ? 0 : m_regionPyr.height() - 1;

		// Param 1 is the maximum num of regions in any segmentation
		pMaxVals->at(1) = m_regionPyr.maxWidth();
	}

	virtual int NumOutputImages() const
	{
		return 2;
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		return (i == 0) ? "Region Contours" : "Salient Regions";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{	
		ASSERT(dii.params.size() == 2);

		int segIdx = (int) dii.params[0];
		int regIdx = (int) dii.params[1] - 1; // ie, reg ir or, -1 == all regions

		if (dii.outputIdx == 0)
		{
			dio.imageType = BYTE_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(CreateContourImage(segIdx, regIdx));
		}
		else
		{
			dio.imageType = RGB_IMAGE;
			dio.imagePtr = ConvertToBaseImgPtr(CreateSalientRegionImage(segIdx, regIdx));
		}

		std::ostringstream oss;
		oss << "Segmentation " << segIdx + 1 << " out of " 
			<< m_regionPyr.height() << ".";
		dio.message = oss.str();

		dio.specs.draggableContent = false;
		dio.specs.zoomableContent = false;
	}

	bool pointInRegion(int x, int y, DiscreteXYArray &pts) const;

	virtual std::string ClassName() const
	{
		return "RegionAnalyzer";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ImageSegmenter");
		deps.push_back("BackgroundSubtractor");
		deps.push_back("ImageProcessor");
		deps.push_back("BlobTracker");

		return deps;
	}

	virtual void Draw(const DisplayInfoIn& dii) const;
	virtual bool OnGUIEvent(const UserEventInfo& uei);

	ByteImg CreateContourImage(int segmentIdx, int regionIdx) const;
	RGBImg CreateSalientRegionImage(int segmentIdx, int regionIdx) const;

	bool IsWithinBounds(unsigned int x, unsigned int y) const
	{
		if (m_inputImgs.empty())
			return false;
		else
			return (x < m_inputImgs.front().ni() && 
			y < m_inputImgs.front().nj());
    }

	bool IsForeground(unsigned int x, unsigned int y, unsigned int id) const
	{
		return (IsWithinBounds(x, y) && m_inputImgs.front()(x, y) == id);
	}

	bool IsBoundaryPoint(const UICoordinate& c) const
	{
		return false;
	}

	void SetBoundingBox(Region& r);

	void LoadUserData();

	void SaveUserData();
};

} // namespace vpl

