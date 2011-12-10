/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SKELETAL_INFO_H_
#define _SKELETAL_INFO_H_

#include <vector>
#include <list>
#include <map>
#include <set>
#include "Point.h"
#include <Tools/BasicUtils.h>

namespace sg {

class DDSEdge;

/*!
	Skeletal spoke represented as a point index and a side.
*/
struct SpokeInfo
{
	unsigned int skelPtIndex;
	char side;

	SpokeInfo() { }

	SpokeInfo(int i, char c)
	{
		skelPtIndex = i;
		side = c;
	}
};

//!< List of spokes encoded as branch point index and side
typedef std::list<SpokeInfo> SpokeInfoList;

//!< Map from skeletal branches to lists of spokes
typedef std::map<const DDSEdge*, SpokeInfoList> SpokesMapParent;

//!< Information about a symmetrix point (wrt a skeletal branch)
struct SymmetricPoint
{
	int index; //!< Boundary index of the point
	unsigned int medialPtIdx; //!< Medial pt index along the symmetry axis
	double radius; //!< Distance between the symmtry axis and the point

	bool operator==(const SymmetricPoint& rhs) const
	{
		return index == rhs.index;
	}

	bool operator<(const SymmetricPoint& rhs) const
	{
		return index < rhs.index;
	}

	void Print(std::ostream& os) const
	{
		os << "(" << index << ", " << medialPtIdx 
			<< ", " << radius << ")";
	}
};

//!< Set of symmetric points
typedef std::set< SymmetricPoint, std::less<SymmetricPoint> > SymPtSet;

//!< Map from skeletal branches to sets of symmetric points
typedef std::map<const DDSEdge*, SymPtSet> SymPtSetMap;

//!< List of symmetric points
typedef std::list<SymmetricPoint> SymPtList;

//!< Parent class of the SymPtListMap class
typedef std::map<const DDSEdge*, SymPtList> SymPtListMap;

/*!
	Index along a skeletal branch representing a skeletal point.
*/
struct BranchPointIndex
{
	const DDSEdge* pBranch;
	unsigned int index;

	void GetSpokeIndices(unsigned int* pIdx1, unsigned int* pIdx2) const;
	const Point& GetSkelPointCoord() const;
};

/*!
	Map of skeletal branches to list of skeletal point indices
	and a sides (relative to each branch). This mapping is
	used to represent all the spokes ending at a boundary point,
	which might be emanated from different branches. That is,
	each boundary point has a SpokeMap associated with it.

	@see SkeletalInfo
*/
class SpokesMap : public SpokesMapParent
{
protected:
	SpokeInfoList& operator[](const DDSEdge* pBranch)
	{
		return SpokesMapParent::operator[](pBranch);
	}

	friend class SkeletalInfo;

public:
	typedef SpokesMapParent::iterator iterator;
	typedef SpokesMapParent::const_iterator const_iterator;

	bool FindMaxLengthSpokeOrigin(BranchPointIndex* pBPI) const;
	
	void FindSymmetricPoints(SymPtListMap* pSymMap) const;

	unsigned int NumPoints(const DDSEdge* pBranch) const
	{
		const_iterator it = find(pBranch);

		return (it != end()) ? it->second.size() : 0;
	}

	int SpokeCount() const
	{
		const_iterator it;
		int count = 0;

		for (it = begin(); it != end(); ++it)
			count += it->second.size();

		return count;
	}
};

/*!
	Skeletal information associated with each contour point of a shape.
	It represents a mapping from boundary points to skeletal points.

	Each boundary point has a SpokeMap associated with it, which represents
	all the spokes (possibly from different branches) ending at the
	boundary point.

	@see SpokesMap
*/
class SkeletalInfo : public std::vector<SpokesMap>
{
	typedef std::vector<SpokesMap>::iterator iterator;
	typedef std::vector<SpokesMap>::const_iterator const_iterator;

public:
	void Initialize(unsigned int sz)
	{
		resize(sz);
	}

	void Add(int bndryPtIdx, const DDSEdge* pBranch, 
		unsigned int skelPtIdx, char side)
	{
		at(bndryPtIdx)[pBranch].push_back(SpokeInfo(skelPtIdx, side));
	}
};

} // namespace sg

#endif // _SKELETAL_INFO_H_
