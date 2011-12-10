/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "HuMoments.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/NamedColor.h>

#include <Tools/cv.h>

using namespace vpl;

extern UserArguments g_userArgs;

HuMoments::Params HuMoments::s_params;

DECLARE_BASIC_SERIALIZATION(cv::Point2f);

/*!
	Reads parameters from global user's arguments
*/
void HuMoments::Params::ReadFromUserArguments()
{
	StrArray options;

	options.push_back("I1");
	options.push_back("I2");
	options.push_back("I3");
	options.push_back("I1I2");
	options.push_back("I1I3");

	g_userArgs.ReadArg("HuMomentsComp", "method", options,
		"Contour comparison method", 0, &methodId);
}

void HuMoments::Serialize(OutputStream& os) const
{
	ASSERT(m_pPtsMat);

	// make sure that we serialize an int
	int numPts = m_pPtsMat->rows; 

	::Serialize(os, numPts);

	int i = 0;

	for (auto it = m_pPtsMat->begin<cv::Point2f>(); 
		it != m_pPtsMat->end<cv::Point2f>(); ++it)
	{
		::Serialize(os, *it);
		i++;
	}

	ASSERT(i == numPts);
}
		
void HuMoments::Deserialize(InputStream& is)
{
	int numPts;

	::Deserialize(is, numPts);

	delete m_pPtsMat;

	m_pPtsMat = new cv::Mat(numPts, 1, 
		cv::DataType<cv::Point2f>::type); 

	for (auto it = m_pPtsMat->begin<cv::Point2f>(); 
		it != m_pPtsMat->end<cv::Point2f>(); ++it)
	{
		::Deserialize(is, *it);
	}
}

/*!
	Search for findHomography
*/
void HuMoments::Create(const PointArray& pts, const DoubleArray& tangents)
{
	ASSERT(!m_pPtsMat);
	
	delete m_pPtsMat;

	m_pPtsMat = new cv::Mat(pts.size(), 1, 
		cv::DataType<cv::Point2f>::type); //CV_64FC2 or CV_32FC2?

	for (unsigned i = 0; i < pts.size(); ++i)
	{
		cv::Point2f& pt = m_pPtsMat->at<cv::Point2f>(i);

		pt.x = (float)pts[i].x;
		pt.y = (float)pts[i].y;
	}

	//Draw();
	//DBG_PRINT1(m_pPtsMat->channels())
}

HuMoments::~HuMoments()
{
	delete m_pPtsMat;
}

void HuMoments::Draw(const RGBColor& color) const
{
	if (!m_pPtsMat)
		return;

	cv::Moments m = moments(*m_pPtsMat, false);
	double h[7];

	cv::HuMoments(m, h);

	ShowMsg("------------------Points------------------");

	DBG_PRINT2(m_pPtsMat->size().width, m_pPtsMat->size().height)

	for (auto it = m_pPtsMat->begin<cv::Point2f>(); 
		it != m_pPtsMat->end<cv::Point2f>(); ++it)
	{
		std::cout << "(" << (*it).x << "," << (*it).y << "), ";
	}

	std::cout << "\n";

	//! spatial moments
	ShowMsg("----------------Hu Moments----------------");
	ShowMsg("spatial moments");
    DBG_PRINT5(m.m00, m.m10, m.m01, m.m20, m.m11)
	DBG_PRINT5(m.m02, m.m30, m.m21, m.m12, m.m03)

    //! central moments
	ShowMsg("central moments");
    DBG_PRINT7(m.mu20, m.mu11, m.mu02, m.mu30, m.mu21, m.mu12, m.mu03)

    //! central normalized moments
	ShowMsg("central normalized moments");
    DBG_PRINT7(m.nu20, m.nu11, m.nu02, m.nu30, m.nu21, m.nu12, m.nu03)
	ShowMsg("------------------------------------------");
	
	//DrawDisk(Point(m.nu10, m.), 1);
	

	SetDrawingColor(NamedColor("Red"));

	/*

	for (unsigned i = 0; i < m_dataPts.cols(); ++i)
	{
		DrawDisk(Point(m_dataPts[0][i], m_dataPts[1][i]), 1);
	}
	*/
	SetDefaultDrawingColor();
}
