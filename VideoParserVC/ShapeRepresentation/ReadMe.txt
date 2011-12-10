	for (unsigned i = 0 ; i < params.P.cols(); i++)
	{
		params.P(0, i) = newPts(0, i);
		params.P(0, i) = newPts(0, i);
	}
	
	//! Returns a 2xN matrix with all N data points
	virtual void GetPoints(PointArray* pPts) const
	{
		pPts->clear();
	}

/*!
*/
/*class SPGTransformationMap : public NodeMap<std::pair<Matrix, Matrix>>
{
	typedef NodeMap<std::pair<Matrix, Matrix>> BASE_CLASS;
public:
	SPGTransformationMap(const graph& G) : BASE_CLASS(G)
	{
	}

	void TransformPoints(graph::node v, PointList& pts) const;
};*/

 	void ShapeContextComp::MatchSCFast(ShapeContext sc1, ShapeContext sc2)
{
	const unsigned N1 = sc1.NumPoints();
	const unsigned N2 = sc2.NumPoints();
	const unsigned N = MIN(N1, N2);
	const unsigned D2 = N2 - 1;

	// Init the cost matrix
	costMat.set_size(N, N);

	ComputeCostMatrix(sc1, sc2, costMat);

	// Solve min circular histo distance
	
	unsigned i;
	
	// Init matchCost by summing over diagonal
	double costF = 0, costB = 0;

	for (i = 0; i < N; ++i)
	{
		costF += costMat(i, i);
		costB += costMat(i, D2 - i);
	}

	matchCost = MIN(costF, costB);
	
	unsigned j, offset;

	// See if we can improve min cost by shifting histo2
	// both foreward and backwards (it, with histo2 reversed)
	for (offset = 1; offset < N2; offset++)
	{
		costF = 0;
		costB = 0;

		for (i = 0; i < N; ++i)
		{
			j = (i + offset) % N2;

			costF += costMat(i, j);
			costB += costMat(i, D2 - j);
		}

		if (costF < matchCost)
			matchCost = costF;

		if (costB < matchCost)
			matchCost = costB;
	}

	matchCost += Params().unmatchCost * (MAX(N1, N2) - N);
}
	
	/*!
		Gets an optional value and returns true if the fieldKey-propKey
		exists and false otherwise.
	*/
	template <class T> bool GetOptionalTypedValues(const Keyword& fieldKey, 
		const Keyword& propKey, std::list<T>& valueList) const
	{
		try {
			m_datafile.GetTypedValues(expName, caseName, valueList);
		}
		catch(BasicException e) 
		{
			return false;
		}

		return true;
	}
 
 int len = strlen(wpurl)+1;
  wchar_t *wText = new wchar_t[len];
  if ( wText == 0 )
    return;
  memset(wText,0,len);
  ::MultiByteToWideChar(  CP_ACP, NULL,wpurl, -1, wText,len );

  //now pass wText
  hr = pActiveDesktop->SetWallpaper(wText, 0);

  
  // when finish using wText dont forget to delete it
  delete []wText;

/////////////////////////////////////////////////////////////////////////////
if (idx >= h.size())
				{
					DBG_PRINT5(idx, h.size(), NumSamples(), i, j)
					DBG_PRINT2(s_params.numThetaBins, s_params.numRadiusBins)
					DBG_PRINT2(m_radiusBins(i, j), m_angleBins(i, j))
				}
/////////////////////////////////////////////////////////////////////////////
int numPts;

	::Serialize(os, numPts);

	std::vector<CvPoint> pts(numPts);

	for (auto it = pts.begin(); it != pts.end(); ++it)
		::Serialize(os, *it);

		void ContourTree::Deserialize(InputStream& is)
{
	int numPts;

	::Deserialize(is, numPts);

	std::vector<CvPoint> pts(numPts);

	for (auto it = pts.begin(); it != pts.end(); ++it)
		::Deserialize(is, *it);
}
/////////////////////////////////////////////////////////////////////////////
	// Make sure that we don't have memory alloc
	/*delete (std::vector<cv::Point2f>*) m_pPtsArray;
	delete m_pPtsMat;

	m_pPtsArray = new std::vector<cv::Point2f>(pts.size());

	std::vector<cv::Point2f>* pPts = (std::vector<cv::Point2f>*) m_pPtsArray;
	
	for (unsigned i = 0; i < pts.size(); ++i)
	{
		(*pPts)[i].x = (float)pts[i].x;
		(*pPts)[i].y = (float)pts[i].y;
	}

	m_pPtsMat = new cv::Mat(*pPts);*/

/////////////////////////////////////////////////////////////////////////////

/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptor.h"

//struct CvMoments;

class cv::Mat;

namespace vpl
{
/*!
	
*/
class HuMoments : public ShapeDescriptor
{
public:
	//! Parameters for the construction of a shape context
	struct Params
	{
		double threshold;

		void ReadFromUserArguments();
	};

protected:
	/*CvMoments* m_pMoments;     //!< OpenCV Hu moments
	CvContour* m_pContourSeqHeader; //!< Pointer to a CvContour object (ie, a specialized CVSeq)
	CvPoint* m_pContourPts;     //!< Array of contour points
	unsigned m_numPoints;       //!< Number of contour points
	*/

	cv::Mat* m_pPts;

	static Params m_params; //!< Static parameters for the construction of shape contexts

public:
	HuMoments()
	{
		m_pPts = NULL;
		/*m_pMoments = NULL;
		m_pContourSeqHeader = NULL;
		m_pContourPts = NULL;*/
	}

	~HuMoments();
	
	//! Cast the wrapper class
	/*operator const CvMoments*() const 
	{ 
		return m_pMoments; 
	}*/

	//! Reads static parameters for the creation of the shape descriptor
	static void ReadStaticParameters()
	{
		m_params.ReadFromUserArguments();
	}

	//! Reads static parameters for the creation of the shape descriptor
	virtual void ReadClassParameters() const
	{
		m_params.ReadFromUserArguments();
	}

	//! Draws some visualization of the shape descriptor
	virtual void Draw() const;

	void operator=(const HuMoments& rhs)
	{
		// Call the assignment operator in the base class
		ShapeDescriptor::operator=(rhs);

		m_params = rhs.m_params;
	}

	//! Creates a shape context for the boundary 'pts'
	virtual void Create(const PointArray& pts);

	//! Defines the serialize function required by the abstract base class
	virtual void Serialize(OutputStream& os) const;

	//! Defines the deserialize function required by the abstract base class
	virtual void Deserialize(InputStream& is);
};

} //namespace vpl


/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include "HuMoments.h"
#include <Tools/HelperFunctions.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/NamedColor.h>

#include <Tools/cv.h>
#include <Tools/cv_legacy.h>
#include <Tools/CvUtils.h>

using namespace vpl;

extern UserArguments g_userArgs;

HuMoments::Params HuMoments::m_params;

/*!
	Reads parameters from global user's arguments
*/
void HuMoments::Params::ReadFromUserArguments()
{
	g_userArgs.ReadArg("HuMoments", "threshold", 
		"TBD", 0.0, &threshold);
}

void HuMoments::Serialize(OutputStream& os) const
{
	//::Serialize(os, m_histogram);
}
		
void HuMoments::Deserialize(InputStream& is)
{
	//::Deserialize(is, m_histogram);
}

void HuMoments::Create(const PointArray& pts)
{
	ASSERT(!m_pMoments && !m_pContourSeqHeader && !m_pContourPts);

	// Copy contour points in OpenCV sequence
	m_numPoints = pts.size();

	int block_size = m_numPoints * 16;
	
	m_pContourSeqHeader = new CvContour;

	m_pContourPts = new CvPoint[m_numPoints];

	for (unsigned i = 0; i < m_numPoints; ++i)
	{
		m_pContourPts[i].x = pts[i].x;
		m_pContourPts[i].y = pts[i].y;
	}

	// Contour initializing
	CvSeqBlock contourBlockHeader;

    cvMakeSeqHeaderForArray(CV_SEQ_POLYGON, sizeof(CvContour), sizeof(CvPoint),
              (char*) m_pContourPts, m_numPoints, 
			  (CvSeq*) m_pContourSeqHeader, &contourBlockHeader);

	// Create Hu Moments

	m_pMoments = new CvMoments;
		
	cvMoments((CvArr*) m_pContourSeqHeader, m_pMoments, 0);
}

HuMoments::~HuMoments()
{
	delete m_pMoments;

	delete m_pContourSeqHeader;

	delete[] m_pContourPts;
}

void HuMoments::Draw() const
{
	CvTermCriteria criteria; // criteria for the contour restoring

	PointArray::const_iterator it;

	SetDrawingColor(NamedColor("Red"));

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

/////////////////////////////////////////////////////////////////////////////
// Find symmetric point with minimum radius
		ptIt = spl.begin();
		minPtIt = ptIt;

		for (++ptIt; ptIt != spl.end(); ++ptIt)
		{
			// See if the current radius is smaller that the smallest seen so far and if
			// the point is not within the source corner interval (rarely happens, but)
			if (ptIt->radius < minPtIt->radius && !srcCorner.interval.Includes(ptIt->index))
				minPtIt = ptIt;
		}

/////////////////////////////////////////////////////////////////////////////
