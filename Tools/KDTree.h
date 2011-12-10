/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _KD_TREE_H_
#define _KD_TREE_H_

#include <vector>
#include "ANNSearch/include/ANN/ANN.h"
#include "BasicUtils.h"
#include "STLUtils.h"

//! k-Dimensional datapoint used by the KDTree
typedef std::vector<ANNcoord> KDTreePoint;

/*!
	@brief Wrapper class for the structures and algorithms in the ANN library
*/
class KDTree
{
protected:
	const int DIM;              //!< Number of dimensions of the point space

	int m_nPts;                 //!< actual number of data points 
	int m_nMaxPts;              //!< maximum number of data points 
	int m_nNNPtsReturned;		//!< number of NN points returned by the last search
	
	ANNpointArray m_dataPts;    //!< data points 
	ANNpoint m_queryPt;         //!< query point 
	ANNkd_tree* m_kdTree;       //!< search structure

public:
	ANNidx* m_indices;          //!< array of indices for points in m_dataPts
	ANNdist* m_distances;       //!< array of NN squared distances for points in m_dataPts

public:
	/*!
		Constructs a KDTree with a maximum of 'nMaxDataPoints'
		and a space of 'nDims' dimensions.
	*/
	KDTree(int nMaxDataPoints, int nDims = 2);

	~KDTree();

	bool Build();

	int Size() const 
	{ 
		return m_nPts; 
	}
	
	int NumDims() const
	{
		return DIM;
	}

	void KNNSearch(const double& x, const double& y, int k, 
		const double& epsilon = 0)
	{
		m_queryPt[0] = x; 
		m_queryPt[1] = y;
		
		m_distances[0] = -1;

		m_kdTree->annkSearch(m_queryPt, k, m_indices, m_distances, 
			epsilon);

		ASSERT(m_distances[0] >= 0);

		m_nNNPtsReturned = k;
	}

	int RangeSearch(const double& x, const double& y, const double& range, 
		double* epsilon, const double& epsilonIncrement, int atLeast);

	int RangeSearch(const KDTreePoint& pt, const double& range, 
		double* epsilon, const double& epsilonIncrement, int atLeast);

	//!< Finds all points within range 'range' from position 'm_queryPt'
	int RangeSearch(const double& range)
	{
		m_nNNPtsReturned = m_kdTree->annkFRSearch(m_queryPt, range, 
			m_nPts, m_indices, m_distances);

		return m_nNNPtsReturned;
	}

	//!< Finds all points within range 'range' from position x,y. Use GetNN*() to get the points.
	int RangeSearch(const double& x, const double& y, const double& range)
	{
		ASSERT(m_kdTree != NULL);
		ASSERT(2 == DIM);

		m_queryPt[0] = x; 
		m_queryPt[1] = y;

		return RangeSearch(range);
	}

	//!< Finds all points within range 'range' from position x,y. Use GetNN*() to get the points.
	int RangeSearch(const KDTreePoint& pt, const double& range)
	{
		ASSERT(m_kdTree != NULL);
		ASSERT(pt.size() == DIM);

		for (unsigned int i = 0; i < pt.size(); i++)
			m_queryPt[i] = pt[i];

		return RangeSearch(range);
	}

	// NOTE: Functions that index over the m_indices array are 
	// identified by the 'NN' in their name

	ANNpoint GetNNPoint(int i)  
	{ 
		ASSERT(i >= 0 && i < m_nNNPtsReturned); 
		return m_dataPts[m_indices[i]]; 
	}

	ANNidx GetNNIndex(int i)    
	{ 
		ASSERT(i >= 0 && i < m_nNNPtsReturned); 
		return m_indices[i]; 
	}

	double GetNNDistance(int i) 
	{ 
		ASSERT(i >= 0 && i < m_nNNPtsReturned); 
		return sqrt(m_distances[i]); 
	}

	double GetNNSquaredDistance(int i) 
	{ 
		ASSERT(i >= 0 && i < m_nNNPtsReturned); 
		return m_distances[i]; 
	}

	void GetNNInfo(int i, int* idx, vpl::Point* p, double* dist)
	{
		*idx = m_indices[i];
		*dist = sqrt(m_distances[i]);
		p->Set(m_dataPts[*idx][0], m_dataPts[*idx][1]);
	}

	/*! 
		Gets the n'th coordinate of the data point with index ptIdx
	*/
	double GetNNCoord(int ptIdx, int n) 
	{ 
		return GetNNPoint(ptIdx)[n]; 
	}

	/*! 
		Gets the coordinate of all 2D data points
	*/
	void GetDataPoints(PointArray& pts) const
	{
		ASSERT(2 == DIM);

		pts.resize(m_nPts);

		for (int i = 0; i < m_nPts; ++i)
			pts[i].Set(m_dataPts[i][0], m_dataPts[i][1]);
	}

	/*! 
		Gets the coordinate of a 2D data point given in index
	*/
	void GetDataPoint(ANNidx idx, double& x, double& y) const
	{
		ASSERT(2 == DIM);
		ASSERT(idx >= 0 && idx < m_nPts);

		x = m_dataPts[idx][0];
		y = m_dataPts[idx][1];
	}

	/*! 
		Gets the coordinate of a data point given in index
	*/
	void GetDataPoint(ANNidx idx, KDTreePoint& pt) const
	{
		ASSERT(pt.size() == DIM);
		ASSERT(idx >= 0 && idx < m_nPts);

		for (unsigned int i = 0; i < pt.size(); i++)
			pt[i] = m_dataPts[idx][i]; 
	}

	/*! 
		Adds a data point to the array of points (before the tree is built) 
	*/
	void AddDataPoint(const double& x, const double& y)
	{
		ASSERT(2 == DIM);           // check pt dimension
		ASSERT(m_kdTree == NULL);   // tree must not be built
		ASSERT(m_nPts < m_nMaxPts); // needs enought room for pt

		m_dataPts[m_nPts][0] = x; 
		m_dataPts[m_nPts][1] = y;

		m_nPts++;
	}

	/*! 
		Adds a data point to the array of points (before the tree is built) 
	*/
	void AddDataPoint(const KDTreePoint& pt)
	{
		ASSERT(pt.size() == DIM);   // check pt dimension
		ASSERT(m_kdTree == NULL);   // tree must not be built
		ASSERT(m_nPts < m_nMaxPts); // needs enought room for pt

		for (unsigned int i = 0; i < pt.size(); i++)
			m_dataPts[m_nPts][i] = pt[i]; 
		
		m_nPts++;
	}

	/*! 
		Adds a data point to the array of points (before the tree is built) 
	*/
	void AddDataPoints(const PointArray& pts)
	{
		ASSERT(2 == DIM);           // check pt dimension
		ASSERT(m_kdTree == NULL);   // tree must not be built
		ASSERT(m_nPts + (int)pts.size() <= m_nMaxPts); // needs enought room for pts

		PointArray::const_iterator it;

		for (it = pts.begin(); it != pts.end(); ++it)
		{
			m_dataPts[m_nPts][0] = it->x; 
			m_dataPts[m_nPts][1] = it->y;
			m_nPts++;
		}
	}

	/*!
		@brief Computes the straight distance from the point 
		with index i0, to the point with index i1.
	*/
	double DataPointDistance(int nFrom, int nTo) const
	{
		ASSERT(nFrom >= 0 && nFrom < m_nPts);
		ASSERT(nTo >= 0 && nTo < m_nPts);

		double n, sum = 0;

		for (int i = 0; i < DIM; i++)
		{
			n = m_dataPts[nTo][i] - m_dataPts[nFrom][i];
			sum += n * n;
		}

		return sqrt(sum);
	}
};

#endif //_KD_TREE_H_
