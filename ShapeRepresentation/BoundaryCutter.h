/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _BOUNDARY_CUTTER_H_
#define _BOUNDARY_CUTTER_H_

#include "SkeletalGraph.h"
#include "CornerDetector.h"
#include "BoundaryCut.h"

namespace vpl {

/*!
*/
class BoundaryCutter
{
	BoundaryCutSet m_bndryCuts;

protected:
	void AddMinRadiusPoints(const BoundaryCorner& srcCorner,
		const sg::SymPtListMap& splm, const CornerLabelArray& cla, 
		sg::SymPtSetMap* pMap);

	void AddSymmetricCorners(const BoundaryCorner& srcCorner, 
		const sg::SymPtListMap& splm, const CornerLabelArray& cla, 
		sg::SymPtSetMap* pMap);

public:
	void ComputeCuts(const sg::ShapeBoundary* pShape,
		const BoundaryCornerList& cil, const CornerLabelArray& cla);

	const BoundaryCutSet& GetCuts() const
	{
		return m_bndryCuts;
	}
};

} //namespace vpl

#endif // _BOUNDARY_CUTTER_H_