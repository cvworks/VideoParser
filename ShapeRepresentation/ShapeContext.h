/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeDescriptor.h"

namespace vpl
{
/*!
	The shape context is a shape representation and algorithm for matching shape contours.

	References:

	- S. Belongie and J. Malik. "Matching with Shape Contexts". 2000
	- S. Belongie, J. Malik, and J. Puzicha. "Shape Matching and Object Recognition 
	Using Shape Contexts". 2002.

	The algorithm gets two sets of countour points. The length of the two sets must be the same.
		
	The output of the algorithms is an array of indices indicating, for each point in 
	the original set, its corresponding point in the target set.
		
	The computation of shape distance becomes more accurate with each iteration of the matching algorithm.

	@see http://en.wikipedia.org/wiki/Shape_context
*/
class ShapeContext : public ShapeDescriptor
{
public:
	//! Parameters for the construction of a shape context
	struct Params
	{
		// Creation params
		int minPointsForPolygon;
		double minRadius, maxRadius;
		int numRadiusBins, numThetaBins;

		// Matching params
		int maxNumIterations; //!< Max number of alignment iterations during matching
		double betaInit; 
		double annealingRate;
		double unmatchCost;
		bool fastMatch;
		bool doTPSWarp;

		void ReadFromUserArguments();
	};

protected:
	Matrix m_dataPts; //!< A 2xN matrix that stores the given sample points
	DoubleArray m_tangents;

	std::vector<DoubleVector> m_histogram;

	BoolArray m_inliersMap;
	unsigned m_numInliers;

	Matrix m_angles;
	Matrix m_radii;

	IntMatrix m_angleBins;
	IntMatrix m_radiusBins;

	double m_meanRadius;

	static Params s_params; //!< Static parameters for the construction of shape contexts

protected:

	//! Number of sample points given to compute the shape context
	unsigned NumSamples() const
	{
		return m_dataPts.cols(); //ie, don't use size, as it's rows*cols
	}

	void QuantizeRadii(const double& givenMeanRadius);
	void QuantizeAngles();

	void SetAngleMatrix();

	void SetRadiusMatrix();

	void SetJointHistogram();

	void CreateHistogram(const double& meanRadius);

	void CreateHistogram(const DoubleArray& tangents, 
		const BoolArray& inliers, const double& meanRadius)
	{
		m_tangents = tangents;
		SetInliers(inliers);
		CreateHistogram(meanRadius);
	}

	void ComputeMeanRadius();

public:
	static const Params& GetParams()
	{
		return s_params;
	}

	/*! 
		Returns the cost or similarity value that corresponds to
		the matching of this descriptor agains a nil descriptor.
	*/
	virtual double NilMatchValue() const
	{
		return s_params.unmatchCost * NumPoints();
	}

	//! Reads static parameters for the creation of the shape descriptor
	virtual void ReadClassParameters() const
	{
		s_params.ReadFromUserArguments();
	}

	//! Draws some visualization of the shape descriptor
	virtual void Draw(const RGBColor& color) const;

	void operator=(const ShapeContext& rhs)
	{
		// Call the assignment operator in the base class
		ShapeDescriptor::operator=(rhs);

		m_dataPts   = rhs.m_dataPts;
		m_histogram = rhs.m_histogram;

		m_inliersMap = rhs.m_inliersMap;
		m_numInliers = rhs.m_numInliers; 
			
		m_angles = rhs.m_angles;
		m_radii  = rhs.m_radii;
			
		m_angleBins  = rhs.m_angleBins;
		m_radiusBins = rhs.m_radiusBins;
			
		m_meanRadius = rhs.m_meanRadius;
	}

	/*! 
		Specifies which data points are inliers. If the inliers 
		array is empty, all data points are considered to be inliers
	*/
	void SetInliers(const BoolArray& inliers)
	{
		if (inliers.empty())
		{
			m_inliersMap.resize(NumSamples(), true);
			m_numInliers = m_inliersMap.size();
		}
		else
		{
			m_inliersMap = inliers;

			m_numInliers = 0;

			// Count how many 'inliers' there are in the map
			for (unsigned i = 0; i < m_inliersMap.size(); ++i)
				if (m_inliersMap[i])
					++m_numInliers;
		}
	}

	//! Stores the given sample points and their tangents
	void SetDataPoints(const Matrix& pts, const DoubleArray& tangents)
	{
		ASSERT(pts.rows() == 2);
		ASSERT(pts.cols() == tangents.size());

		m_dataPts = pts;
		m_tangents = tangents;
	}

	//! Creates a shape context. The map of inlier points is optional.
	/*void Create(const Matrix& pts, const DoubleArray& tangents, 
		const double& meanRadius = -1,
		const BoolArray& inliers = BoolArray())
	{
		ASSERT(pts.rows() == 2);
		ASSERT(pts.cols() == tangents.size());

		m_dataPts = pts;

		CreateHistogram(tangents, inliers, meanRadius);
	}*/

	//! Creates a shape context for the boundary 'pts'
	virtual void Create(const PointArray& pts, const DoubleArray& tangents, unsigned boundaryLength)
	{
		m_boundaryLength = boundaryLength;
		m_dataPts.set_size(2, pts.size());

		for (unsigned i = 0; i < pts.size(); ++i)
			pts[i].Get(&m_dataPts(0, i), &m_dataPts(1, i));

		// Pass empty inliers and negative average radius so that
		// both are computed
		CreateHistogram(tangents, BoolArray(), -1);
	}

	//! Defines the serialize function required by the abstract base class
	virtual void Serialize(OutputStream& os) const;

	//! Defines the deserialize function required by the abstract base class
	virtual void Deserialize(InputStream& is);

	//! Recomputes the shape context using the current inliers
	void Recompute(const double& meanRadius = -1.0)
	{
		// Bin the radii into a histogram (depends on inliers and mean dist)
		QuantizeRadii(meanRadius);

		// Create joint (theta,r) histogram by mapping the radius histogram 
		// and the angles histogram of EACH sample point onto a 0-1 matrix.
		SetJointHistogram();
	}

	//! Recomputes the shape context using modified data points
	void Recompute(const Matrix& pts, const double& meanRadius = -1.0)
	{
		// Ensure that the matrices have the same dimensions
		CHECK_ROWS_AND_COLS(m_dataPts, pts);

		// Copy points but leave the tangents intact
		m_dataPts = pts;

		CreateHistogram(meanRadius);
	}

	const double& MeanRadius() const { return m_meanRadius; }

	/*!
		Updates the inlier points given a mapping and assuming that
		an outlier is a point that maps to a "dummy" or "padding" point.
			
		Any point source that maps to a target point whose index is 
		less than the number of target points is an inlier.
		
		In other words, if point 'i' is an inlier, then m[i] < numTgtPts.
	*/
	void UpdateInliersUsingMapping(const UIntVector& m, unsigned numTgtPts)
	{
		ASSERT(m_inliersMap.size() <= m.size());

		m_numInliers = 0;

		for (unsigned i = 0; i < m_inliersMap.size(); ++i)
		{
			if (m[i] < numTgtPts)
			{
				m_inliersMap[i] = true;
				++m_numInliers;
			}
			else
			{
				m_inliersMap[i] = false;
			}
		}
	}

	//! Returns the number of inlier data points
	unsigned InlierCount() const
	{
		return m_numInliers;
	}

	//! Returns the number of inlier data points within the first 'numPts' points
	unsigned InlierCount(unsigned numPts) const
	{
		ASSERT(numPts <= m_inliersMap.size());

		unsigned n = 0;

		// Count how many 'inliers' there are within 
		//the first numPts in teh map
		for (unsigned i = 0; i < numPts; ++i)
			if (m_inliersMap[i])
				++n;

		return n;
	}

	//! Returns true iff the i'th data point is an inlier
	bool IsInlier(unsigned i) const
	{
		return m_inliersMap[i];
	}

	const double& X(unsigned i) const
	{
		return m_dataPts(0, i);
	}

	const double& Y(unsigned i) const
	{
		return m_dataPts(1, i);
	}

	//! Returns a 2xN matrix with all the N data points
	virtual const Matrix& Points() const
	{
		return m_dataPts;
	}

	//! Returns the i'th data point
	void GetPoint(unsigned i, double* pX, double* pY) const
	{
		*pX = X(i);
		*pY = Y(i);
	}

	//! Populates the point array with all N data points
	virtual void GetPoints(PointArray* pPts) const
	{
		pPts->resize(m_dataPts.cols());

		for (unsigned i = 0; i < m_dataPts.cols(); i++)
			GetPoint(i, &(*pPts)[i].x, &(*pPts)[i].y);
	}

	/*! 
		Number of rows in the histogram. It's equal to NumSamples(), but
		would still work in the event that we decide not to store the 
		data points within the shape contetx.
	*/
	virtual unsigned NumPoints() const
	{
		return m_histogram.size();
	}

	unsigned NumBins() const 
	{
		return (!m_histogram.empty()) ? m_histogram.front().size() : 0;
	}

	const double& operator()(unsigned i, unsigned j) const
	{
		ASSERT(!m_histogram.empty() && !m_histogram.front().empty());

		return m_histogram[i][j];
	}

	virtual ShapeDescPtr Transform(const PointTransform& params) const;

	void Print(vcl_ostream& os) const;

	void PrintAll(vcl_ostream& os) const;
};

} //namespace vpl

