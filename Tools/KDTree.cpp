/**------------------------------------------------------------------------
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>

#include "KDTree.h"

KDTree::KDTree(int nMaxDataPoints, int nDims) : DIM(nDims)
{	
	m_nMaxPts = nMaxDataPoints;	
	m_nPts = 0;
	m_nNNPtsReturned = 0;

	m_queryPt = annAllocPt(DIM);	     		// allocate 1 query point structure
	m_dataPts = annAllocPts(m_nMaxPts, DIM);	// allocate datapoint storage space

	m_kdTree    = NULL;
	m_indices   = NULL;
	m_distances = NULL;
}

KDTree::~KDTree()
{
	delete[] m_indices;
	delete[] m_distances;
	delete m_kdTree; 
	annDeallocPt(m_queryPt);
	annDeallocPts(m_dataPts);
	//annClose(); => deletes static variables in the ANNkd_tree class. do not call it.
}

/*!
	@brief Creates KD-tree for efficient nearest neighbour search

	This function must be called once ALL the points have been added
*/
bool KDTree::Build()
{
	ASSERT(m_kdTree == NULL);

	if (m_kdTree == NULL)
	{
		// Build the tree
		m_kdTree = new ANNkd_tree(m_dataPts, m_nPts, DIM); 

		if (m_kdTree != NULL)
		{
			// Allocated space for query results
			m_indices   = new ANNidx[m_nPts];
			m_distances = new ANNdist[m_nPts];
		}
	}

	return m_kdTree != NULL;
}

/*bool KDTree::Rebuild(newdatapoints)
{
	ASSERT(m_bIsKDTreeBuilt == true);

	delete m_kdTree;

	// Build new tree
	m_kdTree = new ANNkd_tree(m_dataPts, m_nPts, DIM); 

	return (m_bIsKDTreeBuilt = (m_kdTree != NULL));
}*/

/*!
	@brief Finds the boundary points that are closest to (x,y). It returns 
	at least 'atLeast' number of points. For efficiency, it uses a given 
	range to restrict the initial search. The parameter epsilon controls 
	the error margin for the range. 'epsilon' is an in/out variable and can 
	be set to zero (recommended). If the minimum number of points is not reached, 
	then 'epsilon' is incremented by 'epsilonIncrement'.
			
	Ideally, one wants to set range such that the do-loop in this function is done
	only once. However, in some weird cases, setting an appropriate range is difficult 
	and so multiple iterations may be needed.

	@return the number of points found within range and '*epsilon' incremented by 
		'epsilonIncrement' at least once.
*/
int KDTree::RangeSearch(const double& x, const double& y, const double& range, 
						double* epsilon, const double& epsilonIncrement, 
						int atLeast)
{
	const double rangeSquared = range * range;
	int n;

	do
	{
		n = RangeSearch(x, y, rangeSquared + *epsilon);

		*epsilon += epsilonIncrement; // must end up incremented

	} while (n < atLeast && n < m_nPts);

	return n;
}

/*!
	@brief Finds the boundary points that are closest to (x,y). It returns 
	at least 'atLeast' number of points. For efficiency, it uses a given 
	range to restrict the initial search. The parameter epsilon controls 
	the error margin for the range. 'epsilon' is an in/out variable and can 
	be set to zero (recommended). If the minimum number of points is not reached, 
	then 'epsilon' is incremented by 'epsilonIncrement'.
			
	Ideally, one wants to set range such that the do-loop in this function is done
	only once. However, in some weird cases, setting an appropriate range is difficult 
	and so multiple iterations may be needed.

	@return the number of points found within range and '*epsilon' incremented by 
		'epsilonIncrement' at least once.
*/
int KDTree::RangeSearch(const KDTreePoint& pt, const double& range, 
						double* epsilon, const double& epsilonIncrement, 
						int atLeast)
{
	const double rangeSquared = range * range;
	int n;

	do
	{
		n = RangeSearch(pt, rangeSquared + *epsilon);

		*epsilon += epsilonIncrement; // must end up incremented

	} while (n < atLeast && n < m_nPts);

	return n;
}
