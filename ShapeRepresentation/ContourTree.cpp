/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ContourTree.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/NamedColor.h>

//#include <Tools/cv.h>
#include <Tools/CvUtils.h>

#if (_MSC_VER)                  // microsoft visual studio
#pragma warning(disable : 4996) // disable all deprecation warnings in cvaux.h
#endif

#include <Tools/cv_legacy.h>

using namespace vpl;

extern UserArguments g_userArgs;

ContourTree::Params ContourTree::s_params;

DECLARE_BASIC_SERIALIZATION(CvPoint);

/*!
	Reads parameters from global user's arguments
*/
void ContourTree::Params::ReadFromUserArguments()
{
	// Creation params
	g_userArgs.ReadArg("ContourTree", "accuracyThreshold", 
		"Creation: Approximation accuracy", 0.0, &accuracyThreshold);

	// Matching params
	g_userArgs.ReadArg("ContourTreeComp", "similarityThreshold", 
		"Matching: Reconstruction is interrupted if difference is less than threshold", 
		0.0, &similarityThreshold);
}

void ContourTree::CopyPointsToOpenCVSeq(const PointArray& pts)
{
	ASSERT(!m_pStorage && !m_pContourSeqHeader && !m_pContourPts);

	// Copy contour points in OpenCV sequence
	m_numPoints = pts.size();

	int block_size = 10000;
	
	m_pContourSeqHeader = new CvContour;

	m_pContourPts = new CvPoint[m_numPoints];

	for (unsigned i = 0; i < m_numPoints; ++i)
	{
		m_pContourPts[i].x = (int)pts[i].x;
		m_pContourPts[i].y = (int)pts[i].y;
	}

	// Contour initializing
	CvSeqBlock contourBlockHeader;

	//CV_SEQ_POLYGON
	//CV_32SC2
	//CV_SEQ_ELTYPE_POINT
    cvMakeSeqHeaderForArray(CV_SEQ_POLYGON, sizeof(CvContour), sizeof(CvPoint),
              (char*) m_pContourPts, m_numPoints, 
			  (CvSeq*) m_pContourSeqHeader, &contourBlockHeader);

	// Contour tree creation
	m_pStorage = cvCreateMemStorage(block_size);
}

void ContourTree::Serialize(OutputStream& os) const
{
	::Serialize(os, m_ptsArray);
}
		
void ContourTree::Deserialize(InputStream& is)
{
	::Deserialize(is, m_ptsArray);
	
	Create();
}

void ContourTree::Create()
{
	ASSERT(!m_pTree);

	CopyPointsToOpenCVSeq(m_ptsArray);

	m_pTree = cvCreateContourTree((CvSeq*) m_pContourSeqHeader, 
		m_pStorage, s_params.accuracyThreshold);
}

void ContourTree::Create(const PointArray& pts, const DoubleArray& tangents)
{
	m_ptsArray = pts;

	Create();
}

ContourTree::~ContourTree()
{
	// m_pTree need not be deleted. Its memory is in storage block.

	if (m_pStorage)
		cvReleaseMemStorage(&m_pStorage);

	delete m_pContourSeqHeader;

	delete[] m_pContourPts;
}

void ContourTree::Draw(const RGBColor& color) const
{
	CvTermCriteria criteria; // criteria for the contour restoring

	PointArray::const_iterator it;

	SetDrawingColor(color);

	criteria.type = CV_TERMCRIT_ITER;
    criteria.max_iter = 100;

	//criteria.type = CV_TERMCRIT_EPS;
    //criteria.epsilon = (float)0.00001;

	//criteria.type = CV_TERMCRIT_ITER + CV_TERMCRIT_EPS;
    //criteria.epsilon = (float)0.00001;
    //criteria.max_iter = 1;

	//criteria.type = CV_TERMCRIT_ITER + CV_TERMCRIT_EPS;
    //criteria.epsilon = 1000.;
    //criteria.max_iter = 100;

	/*contour_h2 = cvContourFromContourTree (tree, storage, criteria);

	for (unsigned i = 0; i < m_dataPts.cols(); ++i)
	{
		DrawDisk(Point(m_dataPts[0][i], m_dataPts[1][i]), 1);
	}
	*/
	SetDefaultDrawingColor();
}
