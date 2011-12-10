#ifndef _DDS_MAKER_UTILS_H_
#define _DDS_MAKER_UTILS_H_

#include "DivergenceMap.h"
#include "DDSGraph.h"
#include "DivArr.h"
#include "Tools/BasicUtils.h"
#include "NeighborhoodArray.h"
#include "PriorityQueueDP.h"

namespace sg 
{

struct NodeOrder{
	DDSNode *n;
	double t;

	NodeOrder(DDSNode *node, double ct){
		n = node; t = ct;
	}

	bool operator<(const NodeOrder &no1) const{
		return t < no1.t;
	}
};

//! BEGIN: Matthijs 14-04-2006
struct SpecialNeighbors
{
	const int* m_nbrs;
	int m_i0, m_i1;

	SpecialNeighbors(int* nbrs) :m_nbrs(nbrs) { m_i0 = -1; m_i1 = -1; }

	void SetSwap(int i0, int i1) { m_i0 = i0; m_i1 = i1; }

	const int* operator[](int i) const
	{
		i = (i == m_i0) ? m_i1:((i == m_i1) ? m_i0:i);
		return m_nbrs + i * 2; // same as &m_nbrs[i * 2] == &(*(m_nbrs + i * 2))
	}
};
//! END: Matthijs 14-04-2006

///////////////////////////////////////////////////////////////
/// Global Variables

// the neighbourhood is
// 8 1 2
// 7 P 3
// 6 5 4
// and these are the offsets
int nbrs[][2] = {{0,1},  {1,1},  {1,0},   // 1 2 3
{1,-1}, {0,-1}, {-1,-1}, // 4 5 6
{-1,0}, {-1,1}           // 7 8
};

SpecialNeighbors specialNbrs((int*)nbrs);
/*int specialNbrs[][2] = {{0,1},  {1,1},  {1,0},   // 1 2 3
{1,-1}, {0,-1}, {-1,-1}, // 4 5 6
{-1,0}, {-1,1}           // 7 8
};*/

/*!
*/
class DivPtVector : public std::vector<DivPt> 
{
public:
	DDSNode *n1, *n2;

	DivPtVector () 
	{ 
		n1=0; 
		n2=0; 
	}
};

///////////////////////////////////////////////////////////////
/// Inline functions

/*!
	A point is on the contour if one of its 8-nbrs is of BACKGROUND_COL
*/
inline bool is_on_contour(const DivPt& dp, const DivArr& da)
{
	if(dp.col != BACKGROUND_COL)
	{
		for(int i = 0; i < 8; i++)
			if(da(dp.x + nbrs[i][0], dp.y + nbrs[i][1]).col == BACKGROUND_COL)
				return true;
	}

	return false;
}

/*!
	Computes the index of the 256 entry NeighborhoodArray.
	That is, the table of truth values for preservation of 
	topology.
*/
inline int get_nbrs_index(int x, int y, const DivArr& da)
{
	int nForegroundNeighbors = 0;

	if(x < 0 || x >= da.getXSize() || 
		y < 0 || y >= da.getYSize())
		return 0;

	if(x == 0 || x == (da.getXSize() - 1) || 
		y == 0 || y == (da.getYSize() - 1)) // on the edge of the picture
		return 0;

	// beyond the edges of the picture DivArr will return the default point
	// which has BACKGROUND_COL

	if(da(x - 1, y).col != BACKGROUND_COL) nForegroundNeighbors += 8;
	if(da(x + 1, y).col != BACKGROUND_COL) nForegroundNeighbors += 16;
	if(da(x, y - 1).col != BACKGROUND_COL) nForegroundNeighbors += 2;
	if(da(x, y + 1).col != BACKGROUND_COL) nForegroundNeighbors += 64;

	if(da(x - 1, y + 1).col != BACKGROUND_COL) nForegroundNeighbors += 32;
	if(da(x + 1, y + 1).col != BACKGROUND_COL) nForegroundNeighbors += 128;
	if(da(x - 1, y - 1).col != BACKGROUND_COL) nForegroundNeighbors += 1;
	if(da(x + 1, y - 1).col != BACKGROUND_COL) nForegroundNeighbors += 4;

	return nForegroundNeighbors;
}

/*!
	computes the number of intersections with the foreground in the
	neighbourhood of (x,y)
*/
inline int num_of_intersections(int x, int y, const DivArr& da)
{
	// we only count the background to foreground transitions
	int num_trans = 0;

	// start with the last point going to the first
	if(da(x+nbrs[7][0], y+nbrs[7][1]).col == BACKGROUND_COL &&     //current
		da(x+nbrs[0][0], y+nbrs[0][1]).col != BACKGROUND_COL)       //next
		num_trans++;
	// then do the rest
	for(int i=0; i< 8-1; i++)
		if(da(x+nbrs[i][0], y+nbrs[i][1]).col == BACKGROUND_COL &&   //current
			da(x+nbrs[i+1][0], y+nbrs[i+1][1]).col != BACKGROUND_COL) //next
			num_trans++;

	return num_trans;
}

/*!
	Count the number of 8-nbrs of (x,y) which are not in the
	BACKGROUND_COL
*/
inline int num_of_nbrs(int x, int y, const DivArr& da)
{
	int n=0;
	for(int i=0; i<8; i++)
		if(da(x + nbrs[i][0], y + nbrs[i][1]).col != BACKGROUND_COL)
			n++;  

	return n;
}

/*!
	We have an end_pt if it has a single neighbour or
	if it has 2 neighbours and there is a single intersection
*/
inline bool is_end_pt(int x, int y, const DivArr& da, const double& thresh)
{
	if(da(x,y).val > thresh)
		return false;

	int nbr_count = 0;
	for(int i=0; i < 8; i++)
		if (da(x+nbrs[i][0], y+nbrs[i][1]).col != BACKGROUND_COL)
			nbr_count++;

	if(nbr_count == 1)
		return true;

	int num_trans = num_of_intersections(x, y, da);  

	if((nbr_count == 2) && num_trans == 1)
		return true;
	else
		return false;
}

/*!
	We must not break the skeleton
*/
inline bool is_removable(int x, int y, const DivArr& da)
{
	if(da(x,y).col == BACKGROUND_COL)
		return true;

	// NeighborhoodArray is the precomputed table for the 8-nbrs
	return (NeighborhoodArray[get_nbrs_index(x, y, da)] == 1);
}

/*!
*/
inline bool is_removable(const DivPt& dp, const DivArr& da)
{
	return is_removable(dp.x, dp.y, da);
}

#ifdef USE_NEW_THINNING
/*!
	Decide wheather to put the given point into the heap or not, and do
	it.  
*/
inline void push_nbr_if(int x, int y, DivArr& da, 
						PriorityQueueDP& the_heap)
{ 
	if (da.checkLimits(x, y))
	{
		DivPt& dp = da(x, y);

		if (dp.col == FOREGROUND_COL && is_removable(x, y, da))
			the_heap.push(&dp);
	}
}
#else // USE_NEW_THINNING
/*!
	Decide wheather to put the given point into the heap or not, and do
	it. Call the const operator() so that limits are checked.
*/
inline void push_nbr_if(int x, int y, const DivArr& da, 
						PriorityQueueDP& the_heap)
{ 
	const DivPt& dp = da(x, y);

	if (dp.col == FOREGROUND_COL && is_removable(x, y, da))
		the_heap.push(dp);
}
#endif // USE_NEW_THINNING

/*!
	Goes through the list of branches and deletes them. They are
	pointers, that's why need this...
*/
inline void destroy_preliminary_branch_list(std::vector<DivPtVector*>& branches)
{
	/*DivPtVector* dpv;

	while(branches.size() > 0)
	{	
		dpv = branches.back();
		delete dpv;
		branches.pop_back();
	}*/

	std::vector<DivPtVector*>::iterator it;

	for (it = branches.begin(); it != branches.end(); ++it)
		delete *it;

	branches.clear();
}

} // namepsace sg

#endif // _DDS_MAKER_UTILS_H_
