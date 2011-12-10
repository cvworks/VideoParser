/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "FeatureDetector.h"

#include <VideoParser/ImageProcessor.h>
#include <Tools/CvMatView.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserEvents.h>

using namespace vpl;

extern UserArguments g_userArgs;

class TrivialFeatureDetector : public cv::FeatureDetector
{
public:
	virtual void read( const cv::FileNode& fn ) { }
	virtual void write( cv::FileStorage& fs ) const { }

protected:
	/*!
		Image is a cell in a grid.
	*/
	virtual void detectImpl( const cv::Mat& image, std::vector<cv::KeyPoint>& keypoints, 
		const cv::Mat& mask=cv::Mat() ) const
	{
		double x = image.cols / 2.0;
		double y = image.rows / 2.0;
		double avgSize = (image.cols + image.rows) / 2.0;
		double radius = avgSize / 2.0;

		if (mask.empty() || mask.at<uchar>((int)y, (int)x))
			keypoints.assign(1, cv::KeyPoint((float)x, (float)y, (float)radius, 0));
		else
			keypoints.clear();
	}
};

/*!
	Now the following detector types are supported:
	"FAST" – cv::FastFeatureDetector,
	"STAR" – cv::StarFeatureDetector,
	"SIFT" – cv::SiftFeatureDetector,
	"SURF" – cv::SurfFeatureDetector,
	"MSER" – cv::MserFeatureDetector,
	"GFTT" – cv::GfttFeatureDetector,
	"HARRIS" – cv::HarrisFeatureDetector.
	Also combined format is supported: feature detector adapter name ("Grid" – cv::GridAdaptedFeatureDetector,
	"Pyramid" – cv::PyramidAdaptedFeatureDetector) + feature detector name (see above), e.g.
	"GridFAST", "PyramidSTAR", etc.

	There also an additional "GridTrivial" detector, which isn't part of open cv.
*/
void FeatureDetector::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();
	
	// There are too many options. Cannot tokenize them...
	StrList detectTypes(1, std::string("SURF"));

	g_userArgs.ReadArgs(Name(), "detectorTypes", 
		"An OpenCV feature detector: "
		"FAST STAR SIFT SURF MSER GFTT HARRIS GridFAST PyramidSTAR...", 
		detectTypes, &detectTypes);

	m_types.assign(detectTypes.begin(), detectTypes.end());

	g_userArgs.ReadArg(Name(), "gridRows", 
		"Number of rows for the GridTrivial detector", 24, &m_params.gridRows);

	g_userArgs.ReadArg(Name(), "gridCols", 
		"Number of rows for the GridTrivial detector", 32, &m_params.gridCols);
}

/*!
	This function is called by void VSCGraph::Reset()
	before a new video is processed.
*/
void FeatureDetector::Clear()
{
	m_inputImg.clear();
	m_keypoints.clear();

	// Don't clear the detectors
}

void FeatureDetector::Initialize(graph::node v)
{
	// Base init calls Clear(), ReadParamsFromUserArguments(),...
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);

	m_detectors.resize(m_types.size());

	for (unsigned i = 0; i < m_types.size(); i++)
	{
		if (m_types[i] != "GridTrivial")
		{
			m_detectors[i] = cv::FeatureDetector::create(m_types[i]);
		}
		else
		{
			cv::Ptr<cv::FeatureDetector> p1(new TrivialFeatureDetector);

			cv::Ptr<cv::FeatureDetector> p2(new cv::GridAdaptedFeatureDetector(p1, 
				1000, m_params.gridRows, m_params.gridCols));

			m_detectors[i] = p2;
		}
	}
}

void FeatureDetector::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	m_inputImg = m_pImgProcessor->GetRGBImage();

	// If there number of masks changed, we recompute all of them, 
	// but if it's the same, we assume that no ROI changed.
	if (m_pImgProcessor->ROIChanged())
	{
		ROISequence ra = m_pImgProcessor->GetROISequence();

		m_masks.resize(ra.size());
		ByteImg mask(m_inputImg.ni(), m_inputImg.nj());

		for (unsigned i = 0; i < m_masks.size(); i++)
		{
			mask.fill(0);
			ra.FillMask(i, mask);
			m_masks[i] = CvMatView(mask).clone();
		}
	}

	CvMatView mat(m_inputImg);

	if (m_masks.empty())
	{
		m_keypoints.resize(1, std::vector<Keypoints>(m_detectors.size()));

		for (unsigned j = 0; j < m_detectors.size(); j++)
			m_detectors[j]->detect(mat, m_keypoints[0][j]);
	}
	else
	{
		m_keypoints.resize(m_masks.size());

		for (unsigned i = 0; i < m_masks.size(); i++)
		{
			m_keypoints[i].resize(m_detectors.size());

			for (unsigned j = 0; j < m_detectors.size(); j++)
				m_detectors[j]->detect(mat, m_keypoints[i][j], m_masks[i]);
		}
	}
}

void FeatureDetector::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{	
	RGBImg img;

	img.deep_copy(m_inputImg);

	CvMatView mat(img);

	/*RGBImg srcImg;
	srcImg.deep_copy(m_inputImg);
	CvMatView srcMat(srcImg);*/

	unsigned maskIdx = (unsigned) dii.params[0];
	const Keypoints& pts = m_keypoints[maskIdx][dii.outputIdx];

	//cv::drawKeypoints(srcMat, pts, mat);

	for (auto it = pts.begin(); it != pts.end(); ++it)
	{
		cv::circle(mat, it->pt, 4, cv::Scalar(255, 0,0));
	}

	dio.imageType = RGB_IMAGE;
	dio.imagePtr = ConvertToBaseImgPtr(img);

	/*int segIdx = (int) dii.params[0];
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
	dio.specs.zoomableContent = false;*/
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
//void FeatureDetector::Draw(const DisplayInfoIn& dii) const
//{
//	
//}

