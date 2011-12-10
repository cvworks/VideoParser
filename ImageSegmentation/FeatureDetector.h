/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/cv.h>

/*namespace cv {
	class FeatureDetector;
}*/

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;

/*!

*/
class FeatureDetector : public VisSysComponent
{
protected:
	typedef std::vector<cv::KeyPoint> Keypoints;
	typedef cv::Ptr<cv::FeatureDetector> Detector;

	struct Params
	{
		int gridRows;
		int gridCols;
	};

protected:
	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;

	RGBImg m_inputImg;

	std::vector<Detector> m_detectors;
	std::vector<std::string> m_types;
	std::vector<cv::Mat> m_masks;

	std::vector<std::vector<Keypoints>> m_keypoints;

	Params m_params;

public:	

	cv::Size GridSize() const
	{
		return cv::Size(m_params.gridCols, m_params.gridRows);
	}

	cv::Size2f CellSize() const
	{
		if (m_inputImg.size() == 0)
			return cv::Size2f(0, 0);
		else
			return cv::Size2f(m_inputImg.ni() / float(m_params.gridCols), 
				m_inputImg.nj() / float(m_params.gridRows));
	}

	unsigned KeypointsSetSize() const
	{
		return m_keypoints.size();
	}

	void GetKeypoints(Keypoints& pts, unsigned maskIdx = 0, 
		unsigned detectorIdx = 0) const
	{
		ASSERT(maskIdx < m_keypoints.size() && 
			detectorIdx < m_keypoints[maskIdx].size());

		pts = m_keypoints[maskIdx][detectorIdx];
	}

	std::string GetDetectorType(unsigned i = 0) const
	{
		return m_types[i];
	}

	void GetMask(cv::Mat& mask, unsigned i = 0) const
	{
		mask = (i < m_masks.size()) ? m_masks[i] : cv::Mat();
	}

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(1, pMinVals, pMaxVals, pSteps, 0, 
				m_masks.empty() ? 0 : m_masks.size() - 1, 1);
	}

	virtual void Clear();

	virtual void Initialize(graph::node v);

	virtual void ReadParamsFromUserArguments();


	virtual void Run();

	virtual int NumOutputImages() const
	{
		return m_types.size();
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		return (m_types.empty()) ? std::string() : m_types[i];
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	virtual std::string ClassName() const
	{
		return "FeatureDetector";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ImageProcessor");

		return deps;
	}

	//virtual void Draw(const DisplayInfoIn& dii) const;
	//virtual bool OnGUIEvent(const UserEventInfo& uei);
};

} // namespace vpl

