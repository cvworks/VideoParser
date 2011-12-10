/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BoundaryCorner.h"
#include "FluxSkeleton\include\DDSGraphProject.h"
#include "FluxSkeleton\include\DDSGraphUtils.h"

using namespace vpl;

void BoundaryCorner::Print(const sg::ShapeBoundary* pShape, 
						   const sg::SymPtListMap& splm,
						   std::ostream& os) const
{
	sg::SymPtListMap::const_iterator branchIt;
	sg::SymPtList::const_iterator ptIt;
	Point pt;

	for (int i = interval.First(); ; i = interval.Succ(i))
	{
		pShape->getPoint(i, &pt);

		os << "\nCorner pt = " << pt << ", index " << i;

		for (branchIt = splm.begin(); branchIt != splm.end(); ++branchIt)
		{
			os << "\nBranch: " << PRN_BRANCH_PTR(branchIt->first);

			os << "\nNum sym pts: " << pShape->getSpokesMap(i).NumPoints(branchIt->first);

			const sg::SymPtList& spl = branchIt->second;

			os << "\n\tBranch pts: "; 

			for (ptIt = spl.begin(); ptIt != spl.end(); ++ptIt)
			{
				os << "\n\t\t";
				ptIt->Print(os);
			}
		}

		os << std::endl;

		if (i == interval.Last())
			break;
	}
}
