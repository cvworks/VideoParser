/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BoundaryCutter.h"

using namespace vpl;

/*!
	Adds the symmetric point with minimum radius for each branch into 
	the given set.
*/
void BoundaryCutter::AddMinRadiusPoints(const BoundaryCorner& srcCorner,
	                                    const sg::SymPtListMap& splm, 
										const CornerLabelArray& cla,
										sg::SymPtSetMap* pMap)
{
	sg::SymPtListMap::const_iterator branchIt;
	sg::SymPtList::const_iterator ptIt, minPtIt;
	bool nullMin;

	// Find closest point in each branch
	for (branchIt = splm.begin(); branchIt != splm.end(); ++branchIt)
	{
		const sg::SymPtList& spl = branchIt->second;

		// Find symmetric point with minimum radius
		nullMin = true;

		for (ptIt = spl.begin(); ptIt != spl.end(); ++ptIt)
		{
			// See if the current radius is smaller that the smallest seen so far and if
			// the point is not within the source corner interval (rarely happens, but)
			if ((nullMin || ptIt->radius < minPtIt->radius) && 
				!srcCorner.interval.Includes(ptIt->index))
			{
				minPtIt = ptIt;
				nullMin = false;
			}
		}

		if (nullMin)
			return;

		// Add the min-radius point to the set. If its a corner, but is
		// not a local minimum (angle), add the index of the actual minimum
		if (cla[minPtIt->index].isCorner && !cla[minPtIt->index].isMinimum)
		{
			sg::SymmetricPoint sp = *minPtIt;

			sp.index = cla[minPtIt->index].cornerIt->index;

			(*pMap)[branchIt->first].insert(sp);
		}
		else
		{
			(*pMap)[branchIt->first].insert(*minPtIt);
		}
	}
}

/*!
	Adds the symmetric point that are concave corners into 
	the given set.
*/
void BoundaryCutter::AddSymmetricCorners(const BoundaryCorner& srcCorner,
                                         const sg::SymPtListMap& splm, 
										 const CornerLabelArray& cla,
										 sg::SymPtSetMap* pMap)
{
	sg::SymPtListMap::const_iterator branchIt;
	sg::SymPtList::const_iterator ptIt;
	sg::SymmetricPoint sp;
	sg::SymPtSet::iterator setIt;

	// Visit each point in each branch and add it if it is a corner
	for (branchIt = splm.begin(); branchIt != splm.end(); ++branchIt)
	{
		const sg::SymPtList& spl = branchIt->second;
		sg::SymPtSet& sps = (*pMap)[branchIt->first];

		for (ptIt = spl.begin(); ptIt != spl.end(); ++ptIt)
		{
			// See if it's a corner and also if the point is not
			// within the source corner interval (rarely happens, but)
			if (cla[ptIt->index].isCorner && !srcCorner.interval.Includes(ptIt->index))
			{
				sp = *ptIt;

				sp.index = cla[ptIt->index].cornerIt->index;

				// See if the point is already in the set
				setIt = sps.find(sp);

				if (setIt != sps.end()) // it is
				{
					// Replace value if new pt has smaller radius
					if (setIt->radius > sp.radius)
					{
						// Can't do this "*setIt = sp;" anymore, so...
						// To have an efficient insertion operation,
						// find the element that *precedes* the actual location where 
						// the new element must be inserted
						sg::SymPtSet::iterator insPos = setIt;

						if (insPos != sps.begin())
							--insPos; // there is a predecessor
						else
							++insPos; // no one preceeds the element

						sps.erase(setIt);
						sps.insert(insPos, sp);
					}
				}
				else // it isn't, so add it
				{
					sps.insert(sp);
				}
			}
		}
	}
}

void BoundaryCutter::ComputeCuts(const sg::ShapeBoundary* pShape,
								 const BoundaryCornerList& cil,
								 const CornerLabelArray& cla)
{
	BoundaryCornerList::const_iterator cornerIt;
	sg::SymPtSetMap::const_iterator setMapIt;
	sg::SymPtSet::const_iterator setIt;
	sg::SymPtListMap symPtsMap;
	BoundaryCut bc;
	int i;

	m_bndryCuts.clear();

	//std::fstream fs("corner_info.txt", std::ios::trunc | std::ios::out);

	// For each corner interval
	for (cornerIt = cil.begin(); cornerIt != cil.end(); ++cornerIt)
	{
		//DBG_PRINT2(cornerIt->index, cil.size())
		//unsigned dbgIdx = cornerIt->index;

		// Fill in the map of symmetric points associated with all the points
		// of the current corner
		pShape->getSpokesMap(cornerIt->index).FindSymmetricPoints(&symPtsMap);

		//cornerIt->Print(pShape, symPtsMap, fs);

		// Use the map of symmetric points to add the points that create the 
		// shortest distance cuts into an "empty" set
		sg::SymPtSetMap spsm; // map to empty sets of symmetric points

		// Add the sym point on each branch that has the smallest radius. Provide
		// src corner to ensure that such a point is not within the corner interval
		AddMinRadiusPoints(*cornerIt, symPtsMap, cla, &spsm);

		// Any point in the corner interval that maps to another corner creates
		// a boundary cut. So we add them into an appropriate set of sym pts
		for (i = cornerIt->interval.First(); ; i = cornerIt->interval.Succ(i))
		{
			pShape->getSpokesMap(i).FindSymmetricPoints(&symPtsMap);

			// Add all the symmetric points to this corner point that also
			// happen to be corner points. Provide src corner to ensure that 
			// such points are not within the corner interval
			AddSymmetricCorners(*cornerIt, symPtsMap, cla, &spsm);

			if (i == cornerIt->interval.Last())
				break;
		}

		// Traverse the set of symmetric points and create cuts with them
		for (setMapIt = spsm.begin(); setMapIt != spsm.end(); ++setMapIt)
		{
			for (setIt = setMapIt->second.begin(); setIt != setMapIt->second.end(); ++setIt)
			{
				bc.Set(cornerIt->index, setIt->index, setMapIt->first, setIt->medialPtIdx);
				m_bndryCuts.insert(bc);
			}
		}
	}
}
