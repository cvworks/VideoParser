/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BlobFinder.h"
#include <ImageSegmentation/ImageSegmenter.h>
#include <VideoSegmentation/BackgroundSubtractor.h>
#include <Tools/VSCDataSerializer.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserEvents.h>
#include <Tools/CvMatView.h>
#include <array>

using namespace vpl;

extern UserArguments g_userArgs;

void BlobFinder::Initialize(graph::node v)
{
	// Base init calls Clear(), ReadParamsFromUserArguments(),...
	VisSysComponent::Initialize(v);

	m_pImgProcessor = FindParentComponent(ImageProcessor);

	m_pImgSegmenter = FindParentComponent(ImageSegmenter);

	m_pBkdSubtractor = FindParentComponent(BackgroundSubtractor);
}

void BlobFinder::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "minBlobSize", 
		"Minimum size of a connected component that is detected", 
		200.0, &m_params.minBlobSize);

	//g_userArgs.ReadBoolArg(Name(), "saveUserSelection", 
	//	"Whether to save the user selected regions or not", 
	//	false, &m_saveUserSelection);
}

void BlobFinder::Run()
{
	if (!m_pImgProcessor)
	{
		ShowMissingDependencyError(m_pImgProcessor);
		return;
	}

	m_srcGreyImg = m_pImgProcessor->GetGreyImage();

	if (m_pBkdSubtractor)
	{
		// Get a deep copy of the foreground...
		m_maskImg = m_pBkdSubtractor->Foreground();
		
		CvMatView mask(m_maskImg);

		cv::findContours(mask, m_contours, m_hierarchy, 
			CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		m_blobs.clear();
		m_blobs.reserve(m_contours.size());

		BlobPtr ptrBlob;

		for (auto it = m_contours.begin(); it != m_contours.end(); ++it)
		{
			ptrBlob.reset(new Blob(*it, Timestamp(), m_srcGreyImg));

			if (ptrBlob->size() >= m_params.minBlobSize)
				m_blobs.push_back(ptrBlob);
		}
	}
	else if (m_pImgSegmenter)
	{
	}
}

void BlobFinder::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{	
	//ASSERT(dii.params.size() == 2);

	//int segIdx = (int) dii.params[0];
	//int regIdx = (int) dii.params[1] - 1; // ie, reg ir or, -1 == all regions

	if (m_maskImg.size() == 0 || m_contours.empty())
		return;



	if (dii.outputIdx == 0)
	{
		RGBImg blobImg(m_maskImg.ni(), m_maskImg.nj());

		blobImg.fill(RGBColor(0, 0, 0));

		for (auto it = m_blobs.begin(); it != m_blobs.end(); ++it)
			(*it)->draw(blobImg);

		/*CvMatView blobMat(blobImg);

		cv::drawContours(blobMat, m_contours, -1, cv::Scalar(255));*/

		dio.imageType = RGB_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(blobImg);
	}
	else
	{
		ByteImg blobImg(m_maskImg.ni(), m_maskImg.nj());

		blobImg.fill(0);

		for (auto it = m_blobs.begin(); it != m_blobs.end(); ++it)
			(*it)->drawMask(blobImg);

		/*CvMatView blobMat(blobImg);

		std::vector<const cv::Point*> pts(m_contours.size(), 0);
		std::vector<int> npts(m_contours.size(), 0);

		for (unsigned i = 0; i < m_contours.size(); i++)
		{
			pts[i] = m_contours[i].data();
			npts[i] = m_contours[i].size();
		}


		cv::fillPoly(blobMat, pts.data(), npts.data(), 
			m_contours.size(), cv::Scalar(255));*/

		dio.imageType = BYTE_IMAGE;
		dio.imagePtr = ConvertToBaseImgPtr(blobImg);
	}

	std::ostringstream oss;
		
	//oss << "Segmentation " << segIdx + 1 << " out of " 
	//	<< m_regionPyr.height() << ".";

	dio.message = oss.str();

	dio.specs.draggableContent = false;
	dio.specs.zoomableContent = false;
}

