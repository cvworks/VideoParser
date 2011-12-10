/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <Tools/MathUtils.h>
#include <algorithm>
#include "BoundaryPointMapping.h"
#include "PartnerBranch.h"

#define MIN_PTS_RETURNED 4
#define EPSILON_INC 16
#define INIT_EPSILON 16

//!< Uncomment in order to output all warnings (if dbg mode)
//#define DBG_SHOW_WARNINGS

using namespace sg;

/////////////////////////////////////////////////////////////////////////////////////////////////
// Global functions

/*!
	@brief Returns +alpha or -alpha depending on the side defined by the 
	NORMALIZED vectors v and t.

	Note: v is modified, and ends up rotated by -alpha.
*/
inline double ComputeAlphaAngle(sg::Vector& v, const sg::Vector& t)
{
	ASSERT(t.norm() > 0.99 && t.norm() < 1.01);

	return vpl::SignedVectorAngle(v.x, v.y, t.x, t.y, 1.0, 1.0);

	/*double alpha = acos(t.dot(v));

	// We don't know whether is alpha or -alpha, so simply check by
	v.rotate(alpha);
	double d1 = (t.x - v.x) * (t.x - v.x) + (t.y - v.y) * (t.y - v.y);

	v.rotate(-2 * alpha); //ie, remove previous alpha rotation + new rotation
	double d2 = (t.x - v.x) * (t.x - v.x) + (t.y - v.y) * (t.y - v.y);

	return (d1 < d2) ? alpha:-alpha;*/
}

/*!
	@brief Computes the angle between (annPt - fp.p) and the normalized tangent t.
	
	@return +alpha or -alpha depending on the side defined by the tangent t
*/
inline double ComputeAlphaAngle(const sg::FluxPoint& fp, const double* annPt, const sg::Vector& t)
{
	sg::Vector v(annPt[0] - fp.p.x, annPt[1] - fp.p.y);

	v.normalize();

	return ComputeAlphaAngle(v, t);
}

/*!
	@brief Computes the angle between (p1 - p0) and the normalized tangent t.
	
	@return +alpha or -alpha depending on the side defined by the tangent t
*/
inline double ComputeAlphaAngle(const sg::Point& p0, const sg::Point& p1, const sg::Vector& t)
{
	sg::Vector v(p1.x - p0.x, p1.y - p0.y);

	v.normalize();

	return ComputeAlphaAngle(v, t);
}

/*!
	See dog100 without this cases accounted for. Basically, there is a size-2 terminal
	branch that is not an accurate skeletal point and the spokes are crossing
	in a funny way.
*/
void BoundaryPointMapping::FixSpecialCrossingCase(SkelBranch* pBranch, char side)
{
	const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
	sg::BoundaryInfoArray& bil    = pBranch->getBoundaryInfoArray();

	sg::BoundaryPoint& bp0 = bil.front()[side];
	sg::BoundaryPoint& bp1 = bil.back()[side];

	const bool bp1IsCloser = (fpl.front().p.sqDist(bp1.pt) < 
		                      fpl.back().p.sqDist(bp0.pt));

	PartnerBranch firstPB, lastPB;

	firstPB.FindPartnerBranchOf(pBranch, GetFirstJoint(pBranch), side);
	lastPB.FindPartnerBranchOf(pBranch, GetLastJoint(pBranch), side);

	if (bp1IsCloser)
	{
#ifdef DBG_SHOW_WARNINGS
		DBG_STREAM("\n\tSpecial case: correcting spoke endpoint in (1) with that of (2)")
#endif

		if (firstPB.GetFirstBoundaryPointIndex() == firstPB.GetLastBoundaryPointIndex())
			firstPB.GetLastBoundaryPoint() = bp1;

		firstPB.MoveSharedSpoke(bp1);
	}
	else
	{
#ifdef DBG_SHOW_WARNINGS
		DBG_STREAM("\n\tSpecial case: correcting spoke endpoint in (2) with that of (1)")
#endif

		if (lastPB.GetFirstBoundaryPointIndex() == lastPB.GetLastBoundaryPointIndex())
			lastPB.GetLastBoundaryPoint() = bp0;

		lastPB.MoveSharedSpoke(bp0);
	}
}

/*!

*/
sg::BoundaryPoint BoundaryPointMapping::FindClosestBoundaryPoint(
	const sg::Point& p0, const sg::Point& p1, const BoundaryInterval& bndryInt) const
{
	sg::BoundaryPoint bp;
	sg::Point pt;
	double dist, minDist = -1;

	for (int i = bndryInt.First(); ; i = bndryInt.Succ(i))
	{
		m_bndryPts->GetDataPoint(i, pt.x, pt.y);

		dist = (pt.dist(p0) + pt.dist(p1)) / 2;

		if (minDist == -1 || dist < minDist)
		{
			minDist = dist;
			bp.pt = pt;
			bp.index = i;
		}

		if (i == bndryInt.Last())
			break;
	}

	ASSERT(minDist >= 0);

	return bp;
}

/*!
	@brief Corrects endpoint spokes that cross one another

	There are some uncommon shapes that may give rise to crossing spokes.
	Since later in the code spokes are assumed non-crossing, it is
	necessary to correct the problem before processing these branches.

	\image html ValidateEndpointSpokes.gif
*/
void BoundaryPointMapping::FixCrossingEndpointSpokes(SkelBranch* pBranch, char side)
{
	const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
	sg::BoundaryInfoArray& bil    = pBranch->getBoundaryInfoArray();

	sg::BoundaryPoint& bp0 = bil.front()[side];
	sg::BoundaryPoint& bp1 = bil.back()[side];

#ifdef DBG_SHOW_WARNINGS
	DBG_MSG1("Correcting intersection of spokes:")
	DBG_STREAM("\n\t(1) from " << fpl.front().p << " to " << bp0.pt)
	DBG_STREAM("\n\t(2) from " << fpl.back().p << " to " << bp1.pt)
#endif

	// Check if bp1 is closer to the first flux point that bp0
	// is to the last flux point (this is use to decide special cases)
	const bool bp1IsCloser = (fpl.front().p.sqDist(bp1.pt) < 
		                      fpl.back().p.sqDist(bp0.pt));

	if (!pBranch->isTerminal())
	{
		PartnerBranch firstPB, lastPB;
		bool testBP0, testBP1;

		firstPB.FindPartnerBranchOf(pBranch, GetFirstJoint(pBranch), side);
		lastPB.FindPartnerBranchOf(pBranch, GetLastJoint(pBranch), side);

		ASSERT(firstPB.m_pBranch != NULL && lastPB.m_pBranch != NULL);

		int firstIdx = firstPB.GetLastBoundaryPointIndex();
		int lastIdx = lastPB.GetLastBoundaryPointIndex();

		if (firstIdx == bp0.index || lastIdx == bp1.index)
		{
			FixSpecialCrossingCase(pBranch, side);
			return;
		}

		BoundaryInterval bndryInt(m_bndryPts->Size(), firstIdx, lastIdx);

		char otherSide = FlipSide(side);

		bool bValidInt = bndryInt.MakeExclude(
			bil.front()[otherSide].index, bil.back()[otherSide].index);

		ASSERT(bValidInt);

		testBP0 = bndryInt.Includes(bp0.index);
		testBP1 = bndryInt.Includes(bp1.index);

		// Use bp0 to replace bp1 if either bp1 is an invalid replacement, or
		// if both points are valid replacements and bp1 is closer to the
		// source branch endpoint of bp0 than the otehr way around
		if (!testBP0 && !testBP1)
		{
#ifdef DBG_SHOW_WARNINGS
			DBG_STREAM("\n\tCorrecting spoke endpoints with new point")
#endif
			sg::BoundaryPoint bp;

			bp = FindClosestBoundaryPoint(fpl.front().p, fpl.back().p, bndryInt);

			firstPB.MoveSharedSpoke(bp);
			lastPB.MoveSharedSpoke(bp);
		}
		else if (!testBP0 || (testBP1 && bp1IsCloser))
		{
#ifdef DBG_SHOW_WARNINGS
			DBG_STREAM("\n\tCorrecting spoke endpoint in (1) with that of (2)")
#endif
			firstPB.MoveSharedSpoke(bp1);
		}
		else
		{
#ifdef DBG_SHOW_WARNINGS
			DBG_STREAM("\n\tCorrecting spoke endpoint in (2) with that of (1)")
#endif
			lastPB.MoveSharedSpoke(bp0);
		}
	}
	// If only last endpoint is a joint, or if both endpoints are terminals and
	// last boundary point is closer to first flux point, then keep last bndry point
	else if (GetLastJoint(pBranch)->degree() > 1 ||
		(GetFirstJoint(pBranch)->degree() == 1 && bp1IsCloser))
	{
#ifdef DBG_SHOW_WARNINGS
		DBG_STREAM("\n\tCorrecting spoke endpoint in (1) with that of (2)")
#endif
		bp0 = bp1;
	}
	else // otherwise, keep first boundary point
	{
#ifdef DBG_SHOW_WARNINGS
		DBG_STREAM("\n\tCorrecting spoke endpoint in (2) with that of (1)")
#endif
		bp1 = bp0;
	}
}

/*!
	@brief Gets the forward tangent and the flow direction in a more robust way
	than that of GetEndpointForwardTangent(...) for the special case of short branches.

	@param pTan it is set to the normalized tangent at the requested endpoint

	@return true iff the radii decrease in the direction of the tangent

*/
bool GetRobustEndpointForwardTangent(SkelBranch* pBranch, SkelPtIndex idx, sg::Vector* pTan)
{
	// The robust procedure here is only valid for short branches
	ASSERT(pBranch->size() <= MAX_NUM_PTS_FOR_TANGENT);
	
	// Get the index of the other endpoint and its info
	SkelPtIndex idx0             = (idx == 0) ? pBranch->size() - 1 : 0;
	const sg::FluxPoint& fp0     = pBranch->fluxPoint(idx0);
	const sg::BoundaryPoint& bp0 = pBranch->boundaryInfo(idx0).first;
	
	// Since the branch is short, we can compute the tangent at either endpoint
	GetEndpointForwardTangent(pBranch, idx0, pTan);

	// If we know the object angle on the other endpoint, we assume
	// it is the same for the current endpoint
	if (bp0.index != -1)
	{
		double alpha = ComputeAlphaAngle(fp0.p, bp0.pt, *pTan);

		return fabs(alpha) <= M_PI_2; // then, it is decreasing radius
	}
	else // we have no extra info, so just compare the radii
	{
		bool bDec = fp0.radius() < pBranch->fluxPoint(idx).radius();

		return (idx == 0) ? bDec : !bDec;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Inline functions

//! Finds the index of the boundary point associated with the given indices
int BoundaryPointMapping::GetValidBoundaryIndex(const SkelBranch* pBranch, 
											   char side, const sg::BoundaryPoint& bp)
{
	if (bp.index >= 0) // we already have it
	{
		return bp.index;
	}
	else // it's a gap point, find closest boundary point
	{
		const sg::BoundarySegmentArray& bsl = pBranch->getBoundarySegments();

		sg::Point pt = bsl[side][bp.subindex][-bp.index - 1];
		double epsilon = 0;

		int n = m_bndryPts->RangeSearch(pt.x, pt.y, 2, &epsilon, 2, 1);
		ASSERT(n > 0);

		return m_bndryPts->GetNNIndex(0);
	}
}

/*! 
	Returns true if a boundary interval could be set correctly with the given boundary indices

	\image html TestBoundaryInterval.gif
*/
bool BoundaryPointMapping::TestBoundaryInterval(const int firstIdx, const int lastIdx,
											   const int otherIdx0, const int otherIdxN,
											   const int testIndex) const
{
	BoundaryInterval bndryInt(m_bndryPts->Size(), firstIdx, lastIdx);

	if (bndryInt.Inside(otherIdx0) || bndryInt.Inside(otherIdxN))
	{
		bndryInt.Swap();

		// If swap didn't fix things, the interval is invalid
		if (bndryInt.Inside(otherIdx0) || bndryInt.Inside(otherIdxN))
			return false;
	}

	return bndryInt.Includes(testIndex);
}

void BoundaryPointMapping::SetBoundaryInterval(const int firstIdx, const int lastIdx,
											  const int otherIdx0, const int otherIdxN,
											  BoundaryInterval* pBndryInt) const
{
	ASSERT(firstIdx >= 0 && lastIdx >= 0);
	ASSERT(otherIdx0 >= 0 && otherIdxN >= 0);

	pBndryInt->Set(m_bndryPts->Size(), firstIdx, lastIdx);

	if (pBndryInt->Inside(otherIdx0) || pBndryInt->Inside(otherIdxN))
		pBndryInt->Swap();

#ifdef DBG_SHOW_WARNINGS
	if (pBndryInt->Inside(otherIdx0) || pBndryInt->Inside(otherIdxN))
	{
		DBG_PRINT5("We have a problem", firstIdx, lastIdx, otherIdx0, otherIdxN)
	}
#endif

	//ASSERT(!pBndryInt->Inside(otherIdx0) && !pBndryInt->Inside(otherIdxN));
	WARNING(pBndryInt->Inside(otherIdx0) || pBndryInt->Inside(otherIdxN),
		"Invalid boundary interval. This can lead to incorrect spokes.")
}

void BoundaryPointMapping::SetBoundaryInterval(const char side, 
											  const sg::BoundaryInfo& bi0,
											  const sg::BoundaryInfo& biN,
											  BoundaryInterval* pBndryInt) const
{
	const char otherSide = FlipSide(side);

	SetBoundaryInterval(bi0[side].index, biN[side].index,
		bi0[otherSide].index, biN[otherSide].index, pBndryInt);
}

//! Sets the limits of the boundary interval wrt the given branch and side
void BoundaryPointMapping::SetBoundaryInterval(const SkelBranch* pBranch, const char side, 
											  BoundaryInterval* pBndryInt)
{
	const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();
	const char otherSide = FlipSide(side);

	SetBoundaryInterval(
		GetValidBoundaryIndex(pBranch, side, bil.front()[side]),
		GetValidBoundaryIndex(pBranch, side, bil.back()[side]),
		GetValidBoundaryIndex(pBranch, otherSide, bil.front()[otherSide]),
		GetValidBoundaryIndex(pBranch, otherSide, bil.back()[otherSide]),
		pBndryInt);
}

//! Returns true if the new index forms a valid interval with the other endpt spoke on 'side'
void BoundaryPointMapping::EnsureValidInterval(const sg::BoundaryInfo& bi0, sg::BoundaryInfo* pBiN)
{
	if (bi0.first.index < 0 && bi0.second.index < 0)
		return; // both should be -1 if they aren't set yet

	ASSERT(bi0.first.index >= 0 && bi0.second.index >= 0);
	ASSERT(pBiN->first.index >= 0 && pBiN->second.index >= 0);

	BoundaryInterval bndryInt(m_bndryPts->Size(), bi0.first.index, pBiN->first.index);

	if (bndryInt.Inside(bi0.second.index) || bndryInt.Inside(pBiN->second.index))
	{
		bndryInt.Swap();

		// If the swap didn't fix things, there is a problem with pBiN
		if (bndryInt.Inside(bi0.second.index) || bndryInt.Inside(pBiN->second.index))
		{
			// There is a problem at the terminal point. In general, this is related to
			// low resolution regions and inexact skeletal points coordinates.
			// It should be fixed by swaping the boundary points on either side.
			std::swap(pBiN->first, pBiN->second);

#ifdef _DEBUG
			bndryInt.Set(m_bndryPts->Size(), bi0.first.index, pBiN->first.index);

			if (bndryInt.Inside(bi0.second.index) || bndryInt.Inside(pBiN->second.index))
				bndryInt.Swap();

			ASSERT(!bndryInt.Inside(bi0.second.index) && !bndryInt.Inside(pBiN->second.index));
#endif
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Public functions

/*!
	@brief Assigns the boundary information of every skeletal point in the given graph.
*/
void BoundaryPointMapping::AssignBoundaryPoints(sg::DDSGraph* pDDSGraph)
{
	JointAngles angles;
	sg::Vector tangent;
	SkelBranch* pBranch;
	SkelJoint* pJoint;
	SkelTerminal* pTerminal;
	BoundaryInterval bndryInt[2];
	SkelPtIndex idx, neigPtIdx, i;
	double angle;
	bool bDecRadius;

	// Make sure that all boundary info lists are initialized
	// and their endpoints are cleared (their indices are set to -1)
	forall_branches(pBranch, pDDSGraph->getEdges())
	{
		pBranch->initBoundaryInfoArray();
	}

	// FIRST: Find boundary info at each junction point
	forall_joints(pJoint, pDDSGraph->getNodes())
	{
		// Find the tangent of each branch incident at the junction
		angles.resize(pJoint->degree());
		i = 0;

		forall_branches(pBranch, pJoint->getEdges())
		{
			sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

			// Get the index of the joint point on the branch
			idx = GetEndpointIndex(pBranch, pJoint);

			// Get the CCW angle [-PI, PI] of the tangent at endpoint idx
			// and convert it to the interval [0, 2PI]
			angle = M_PI + GetBranchAngleAtEndpoint(pBranch, idx);

			// Get the tangent with direction defined by an increasing
			// ordering of the skeletal points' indices
			GetEndpointForwardTangent(pBranch, idx, &tangent);

			// Assign the bndry info, tangent, and tanget angle info
			angles[i++].Set(&bil[idx], angle, tangent, pBranch);
		}

		// Sort by incresing values of angles in [0, 2PI]
		std::sort(angles.begin(), angles.end());

		// Set the boundary info of each branch incident at the joint
		// using the information provided by their relative angles
		SetBoundaryInfoAtJoint(pJoint, angles);

		// Sort the branch references at the joint so that neighboring
		// branches in 'angles' have consecutive indices. This ordering
		// is assumed by the ParnerBranch class and by the 
		// function(s) that find rooted ligature
		for (unsigned int i = 0; i < angles.size(); i++)
			pJoint->setEdge(i, angles[i].Branch());
	}

	// SECOND: Set the boundary info of each terminal point in each branch
	forall_terminals(pTerminal, pDDSGraph->getNodes())
	{
		pBranch = pTerminal->getEdges().front();

		idx = GetEndpointIndex(pBranch, pTerminal);

		const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
		sg::BoundaryInfoArray& bil    = pBranch->getBoundaryInfoArray();
		
		if (fpl.size() > MAX_NUM_PTS_FOR_TANGENT)
		{
			bDecRadius = GetEndpointForwardTangent(pBranch, idx, &tangent);
		}
		else
		{
			bDecRadius = GetRobustEndpointForwardTangent(pBranch, idx, &tangent);
		}

		neigPtIdx = (idx == 0) ? 1 : idx - 1;

		SetBoundaryInfoAtTerminal(fpl[idx], tangent, bDecRadius, 
			fpl[neigPtIdx], (pDDSGraph->edgeCount() > 1) ? pBranch : NULL, bil[idx]);

		// If the tangent direction is wrong, bil[idx] defines an incorrect interval
		// This needs to be checked and corrected by calling...
		EnsureValidInterval(bil[(idx == 0) ? bil.size() - 1 : 0], &bil[idx]);
	}

	bool bSpokesFixed;
	int iterCount = 0;

	do {
		bSpokesFixed = false;

		ASSERT(iterCount < 3);

		// Validate (and fix) the spokes of ALL branches before assigning inner points
		forall_branches(pBranch, pDDSGraph->getEdges())
		{
			if (!ValidateEndpointSpokes(pBranch, '1'))
			{
				FixCrossingEndpointSpokes(pBranch, '1');
				bSpokesFixed = true;
			}

			if (!ValidateEndpointSpokes(pBranch, '2'))
			{
				FixCrossingEndpointSpokes(pBranch, '2');
				bSpokesFixed = true;
			}
		}
	} while (bSpokesFixed && iterCount++ < 3);

	// Assign the boundary info to each inner point in each branch
	forall_branches(pBranch, pDDSGraph->getEdges())
	{
		AssignInnerBoundaryPoints(pBranch->getFluxPoints(), 
			pBranch->getBoundaryInfoArray());
	}
}

/*!
	@brief Assigns the boundary information of every skeletal point in the given branch.
	This functions overwrites the boundary info at the branch endpoints using bi0 and biN.
	
	The inner point search is restricted to the boundary intervals defined by each side 
	of the given bi0 and biN.

	@param bi0 boundary info of the first point in the branch
	@param biN boundary info of the last point in the branch
*/
void BoundaryPointMapping::AssignBoundaryPoints(const sg::FluxPointArray& fpl, sg::BoundaryInfoArray& bil,
											   const sg::BoundaryInfo& bi0, const sg::BoundaryInfo& biN)
{
	ASSERT(!bil.empty());

	const int N = fpl.size() - 1;

	bil[0] = bi0;
	bil[N] = biN;

	AssignInnerBoundaryPoints(fpl, bil);
}

/*!
	@brief Finds closest points using priority search. If the KDTree is not build,
	it builds it but does not check for errors. 
	
	If memory errors when building the tree are a concern, build the tree before 
	calling this function.

	If parameters pBi0 and pBiN are given, they are used as the first and last 
	boundary info elements. In such a case, the point search is restricted
	to the boundary intervals defined by each side of the given *pBi0 and *pBiN.
*/
void BoundaryPointMapping::AssignInnerBoundaryPoints(const sg::FluxPointArray& fpl, sg::BoundaryInfoArray& bil)
{
	ASSERT(!bil.empty());

	const int N = fpl.size() - 1;

	// Set intervals assuming a clock-wise ordering
	BoundaryInterval side1, side2;

	SetBoundaryInterval('1', bil[0], bil[N], &side1);
	SetBoundaryInterval('2', bil[0], bil[N], &side2);

	SetInnerBoundaryInfo(fpl, side1, side2, bil);
	
	AssignBoundaryDistanceInfo(fpl, side1, '1', bil);
	AssignBoundaryDistanceInfo(fpl, side2, '2', bil);

	AssignAxisDistanceInfo(fpl, bil);
}

/*!
	@brief Assigns radius values to each point in the flux point list 'fpl' using the information in the
	boundary info list 'bil'. Call AssignBoundaryPoints() first to populate 'bil' before assigning
	radius values.

	The radius assigned to each point is the smallest distance from the point to any of its boundary points.
*/
void BoundaryPointMapping::AssignRadiusValues(const sg::BoundaryInfoArray& bil, sg::FluxPointArray& fpl)
{
	double dx, dy, r1r1, r2r2;

	ASSERT(bil.size() == fpl.size());

	for (unsigned int i = 0; i < bil.size(); i++)
	{
		dx = bil[i].first.pt.x - fpl[i].p.x;
		dy = bil[i].first.pt.y - fpl[i].p.y;
		r1r1 = dx * dx + dy * dy;

		dx = bil[i].second.pt.x - fpl[i].p.x;
		dy = bil[i].second.pt.y - fpl[i].p.y;
		r2r2 = dx * dx + dy * dy;

		if (r1r1 > 0 && r2r2 > 0)
			fpl[i].dist = sqrt(MIN(r1r1, r2r2));
		else if (r1r1 > 0)
			fpl[i].dist = sqrt(r1r1);
		else if (r2r2 > 0)
			fpl[i].dist = sqrt(r2r2);
		else
			fpl[i].dist = 0.1;

		ASSERT(fpl[i].dist > 0);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Static functions

/*!
	@brief Calculate the overlap between the two endpoints. If they overlap and they
	overlap by at least 75% we will consider this edge
	to be a ligature edge.
*/
/*static*/ double BoundaryPointMapping::ComputeEndPointOverlap(const sg::FluxPointArray& fpl)
{
	const sg::FluxPoint& fp0 = fpl[0];
	const sg::FluxPoint& fp1 = fpl[fpl.size() - 1];
	
	double r0 = fabs(fp0.dist);
	double r1 = fabs(fp1.dist);

	double dx = fp1.p.x - fp0.p.x;
	double dy = fp1.p.y - fp0.p.y;

	double c = sqrt(dx * dx + dy * dy); // distance between centers
	
	if (c <= r0 + r1) // circles overlap
	{
		double CBA, CBD, CAB, CAD, overlapArea, smallArea;
		double r0r0 = r0 * r0;
		double r1r1 = r1 * r1;
		double cc = c * c;

		CBA = acos((r1r1 + cc - r0r0) / (2 * r1 * c));
		CBD = 2 * CBA;

		CAB = acos((r0r0 + cc - r1r1) / (2 * r0 * c));
		CAD = 2 * CAB;

		overlapArea = 0.5 * CBD * r1r1 - 0.5 * r1r1 * sin(CBD) +
					0.5 * CAD * r0r0 - 0.5 * r0r0 * sin(CAD);

		smallArea = (r0 < r1) ? (M_PI * r0r0) : (M_PI * r1r1);
		
		return overlapArea / smallArea;
	}
	
	return 0; // circles do not overlap
}

/////////////////////////////////////////////////////////////////////////////////////////////////
// Protected functions

/*!
	@brief Sets the boundary info of each branch endpoint incident on the given branch junction

	@param pJoint pointer to the branch junction node
	@param angles array of angles [0, 2PI] for each branch tangent incident on the junction
*/
void BoundaryPointMapping::SetBoundaryInfoAtJoint(SkelJoint* pJoint, const JointAngles& angles)
{
	const SkelPt& fp0 = pJoint->fp;
	double alpha0, alpha1, epsilon = INIT_EPSILON;
	double* annPt, angle;
	unsigned int j, anglesFound;
	int i, n;

	WARNING1(angles.size() < 3, "There is a joint with less that 3 angles", pJoint->fp);

	//ASSERT(angles.size() >= 3);

	i = 0; // init outside loop so that each NN is check only once
	anglesFound = 0;

	do
	{
		// Use a robust approach to find a boundary point withing an iteratively
		// increasing radius of the skeletal point
		n = m_bndryPts->RangeSearch(fp0.p.x, fp0.p.y, fabs(fp0.dist), &epsilon, 
			EPSILON_INC, MIN_PTS_RETURNED); //epsilon is updated here

		// For all bndry points found within radius, find the one
		// the is associated with two neighbouring branches
		for (; i < n && anglesFound < angles.size(); i++)
		{
			// Get the endpoint of the spoke from the the NN point
			annPt = m_bndryPts->GetNNPoint(i);

			// Find the CCW angle of the spoke [-Pi,PI] and shift it to [0,2PI] 
			angle = M_PI + atan2(annPt[1] - fp0.p.y, annPt[0] - fp0.p.x);

			// Find the angle interval that containts the spoke. Since the angle
			// array should be circular but it isn't, first check the interval 
			// defined by the first and last angles (ie, the special case).
			if (angle < angles.front().Angle() || angle > angles.back().Angle())
			{
				// Get the object angles of each branch endpoint to determine sides
				alpha0 = ComputeAlphaAngle(fp0, annPt, angles.back().Tangent());
				alpha1 = ComputeAlphaAngle(fp0, annPt, angles.front().Tangent());

				// Make sure that alpha angles are valid (avoid 0.0 or -0.0)
				if (fabs(alpha0) > 0 && fabs(alpha1) > 0)
				{
					// Get the boundary points on the appropriate branch sides
					sg::BoundaryPoint& bp0 = angles.back().GetBndryPt(alpha0);
					sg::BoundaryPoint& bp1 = angles.front().GetBndryPt(alpha1);

					//ASSERT(bp0.index == bp1.index);

					if (bp0.index != bp1.index)
					{
#ifdef DBG_SHOW_WARNINGS
						WARNING1(true, "Strange angles at ", pJoint->fp.p);
#endif
					}
					else if (bp0.index == -1)
					{
						bp0.set(annPt[0], annPt[1], m_bndryPts->GetNNIndex(i));
						bp1 = bp0;
						anglesFound++;
					}
				}
			}
			else // now, check the inner intervals
			{
				// The angle should be after the first angle and before the last
				for (j = 1; j < angles.size(); j++)
				{
					if (angle < angles[j].Angle() && angle > angles[j - 1].Angle())
					{
						// Get the object angles of each branch endpoint to determine sides
						alpha0 = ComputeAlphaAngle(fp0, annPt, angles[j - 1].Tangent());
						alpha1 = ComputeAlphaAngle(fp0, annPt, angles[j].Tangent());

						// Make sure that alpha angles are valid (avoid 0.0 or -0.0)
						if (fabs(alpha0) > 0 && fabs(alpha1) > 0)
						{
							// Get the boundary points on the appropriate branch sides
							sg::BoundaryPoint& bp0 = angles[j - 1].GetBndryPt(alpha0);
							sg::BoundaryPoint& bp1 = angles[j].GetBndryPt(alpha1);

							//ASSERT(bp0.index == bp1.index);

							if (bp0.index != bp1.index)
							{
#ifdef DBG_SHOW_WARNINGS
								WARNING1(true, "Strange angles at ", pJoint->fp.p);
#endif
							}
							else if (bp0.index == -1)
							{
								bp0.set(annPt[0], annPt[1], m_bndryPts->GetNNIndex(i));
								bp1 = bp0;
								anglesFound++;
							}
						}

						break;
					}
				}
			}
		}
	} while (anglesFound < angles.size() && n < m_bndryPts->Size());

	ASSERT(anglesFound == angles.size());
}

/*!
	@brief Sets the boundary points that correspond the the endpoint of a branch, fp0, and
	that are closest to the skeleton point fp1 that is adjacent to fp0 in the current branch.
	In addition, the points returned are the pair of points that are closest to fp1 BUT that
	do not have a distance to fp1 that is smaller than their distance to fp0.

	Note: we try to choose the point (on each side) that is closest to both fp0 and fp1. 
	In some cases, this compromise does not yield the desired "first boundary point". We check 
	for this cases by requiring that the closest point to fp1 and the compromised point are not 
	too far apart. If this is the case, we prefer choosing the closest point to fp1, since it's 
	probably a more accurate approximation to the true "first boundary point" than the other point.

	@param fp0 must be a branch endpoint
	@param tangent must be the NORMALIZED tangent at point fp0 (a good approx should be enough)
	@param fp1 must be the skeleton point adjacent to fp1
	@param pBranch if not NULL, the not-crossing endpoint-spokes property is ensured
	@return bi0 by setting the index and coordinates of the boundary points
*/
void BoundaryPointMapping::SetBoundaryInfoAtTerminal(const sg::FluxPoint& fp0, sg::Vector tangent, 
													bool bDecreasingRadius, const sg::FluxPoint& fp1,
													const SkelBranch* pBranch, sg::BoundaryInfo& bi0)
{
	double alpha, dist0, dist1;
	double epsilon = INIT_EPSILON;
	double* annPt;
	int i, n;

	// The tangent is given in the forward direction, as is needed for knowing the
	// "sides". However, we ALSO need it expressed in the direction of decreassing radius
	if (!bDecreasingRadius)
		tangent.scale(-1);

	// Boundary point indices should be initialized already
	ASSERT(bi0.first.index == -1 && bi0.second.index == -1);

	i = 0;       // init outside loop so that each NN is check only once

	do {
		n = m_bndryPts->RangeSearch(fp0.p.x, fp0.p.y, fabs(fp0.dist), &epsilon, 
			EPSILON_INC, MIN_PTS_RETURNED); //epsilon is updated here

		for (; i < n; i++)
		{
			annPt = m_bndryPts->GetNNPoint(i);
			alpha = ComputeAlphaAngle(fp0, annPt, tangent);

			// Get distance from fp0 to boundary points
			dist0 = m_bndryPts->GetNNDistance(i);

			// There are some incorrect cases in which the skeletal point
			// is basically touching the boundary. In this case, the object
			// angle is unreliable and is better to skip the point.
			if (dist0 < 1)
			{
				bi0.first.index = m_bndryPts->GetNNIndex(i);
				bi0.second.index = m_bndryPts->GetNNIndex(i);

				m_bndryPts->GetDataPoint(bi0.first.index, bi0.first.pt.x, bi0.first.pt.y);
				m_bndryPts->GetDataPoint(bi0.second.index, bi0.second.pt.x, bi0.second.pt.y);

				return;
			}

			// Compute distance from fp1 to boundary point
			dist1 = sg::Vector(annPt[0] - fp1.p.x, annPt[1] - fp1.p.y).norm();

			// The object angle should be smaller or equal to pi/2 and, as an extra
			// precaution, we make sure that dist0 is smaller than dist1 before skipping 
			// Also, make sure that the point is not on the boundary (dist >= 1)
			if (fabs(alpha) > M_PI_2 && dist0 < dist1 /*&& dist0 >= 1*/)
				continue;

			// Make the sign of alpha be expressed wrt the original tangent direction
			if (!bDecreasingRadius)
				alpha = -alpha;

			// Choose the closest to both fp0 and fp1 on each side
			if (alpha >= 0 && bi0.first.index == -1)
			{
				// get the point's index in the boundary curve
				bi0.first.index = m_bndryPts->GetNNIndex(i);

				// copy the point's coordinates
				m_bndryPts->GetDataPoint(bi0.first.index, bi0.first.pt.x, bi0.first.pt.y);
				
				if (pBranch && !ValidateEndpointSpokes(pBranch, '1'))
				{
					if (bi0.first.index = pBranch->firstBndryInfo().first.index)
						bi0.first = pBranch->lastBndryInfo().first;
					else
						bi0.first = pBranch->firstBndryInfo().first;

					//bi0.first.index = -1;
					//continue;
				}

				if (bi0.second.index != -1)
					return; // we are done
			}
			else if (alpha < 0 && bi0.second.index == -1)
			{
				// get the point's index in the boundary curve
				bi0.second.index = m_bndryPts->GetNNIndex(i);

				// copy the point's coordinates
				m_bndryPts->GetDataPoint(bi0.second.index, bi0.second.pt.x, bi0.second.pt.y);

				if (pBranch && !ValidateEndpointSpokes(pBranch, '2'))
				{
					if (bi0.second.index = pBranch->firstBndryInfo().second.index)
						bi0.second = pBranch->lastBndryInfo().second;
					else
						bi0.second = pBranch->firstBndryInfo().second;

					//bi0.second.index = -1;
					//continue;
				}

				if (bi0.first.index != -1)
					return; // we are done
			}
		}
	} while (n < m_bndryPts->Size());

	ASSERT(false);
}

/*!
	@brief Sets the boundary points that correspond the the inner endpoints
	of a branch. It assumes that FindExtremeBoundaryPoints() has been previously
	called for the given branch.
*/
void BoundaryPointMapping::SetInnerBoundaryInfo(const sg::FluxPointArray& fpl, 
											   const BoundaryInterval& side1, 
											   const BoundaryInterval& side2, 
											   sg::BoundaryInfoArray& bil)
{
	const int nLast = fpl.size() - 1;
	bool bLookingForSide1, bLookingForSide2;
	int k, i, n, nnIdx;
	double epsilon;

	for (k = 1; k < nLast; k++)
	{
		const sg::FluxPoint& fp = fpl[k];
		sg::BoundaryInfo& bi = bil[k];

		bLookingForSide1 = true;
		bLookingForSide2 = true;
		epsilon = INIT_EPSILON;

		do 
		{
			n = m_bndryPts->RangeSearch(fp.p.x, fp.p.y, fabs(fp.dist), &epsilon, 
				EPSILON_INC, MIN_PTS_RETURNED); //epsilon is incremented each time
			
			for (i = 0; i < n && (bLookingForSide1 || bLookingForSide2); i++)
			{
				nnIdx = m_bndryPts->GetNNIndex(i);

				if (bLookingForSide1 && side1.Includes(nnIdx))
				{
					bLookingForSide1 = false;
					bi.first.index = nnIdx;
				}
				else if (bLookingForSide2 && side2.Includes(nnIdx))
				{
					bLookingForSide2 = false;
					bi.second.index = nnIdx;
				}
			}
		} while ((bLookingForSide1 || bLookingForSide2) && n < m_bndryPts->Size());

		ASSERT(!bLookingForSide1 && !bLookingForSide2);

		// Done. Just copy the points' coordinates
		m_bndryPts->GetDataPoint(bi.first.index, bi.first.pt.x, bi.first.pt.y);
		m_bndryPts->GetDataPoint(bi.second.index, bi.second.pt.x, bi.second.pt.y);
	}
}

/*!
	@brief Finds the wrap-around distance between boundary point with index nFrom 
	to point with index nTo. If nTo < nFrom, the distance is computed as 
	dist(nFrom, m_nPts-1) + dist(0,nTo)

	Note: this function could use recursion or other techniques. However, this bit 
	of code duplication makes the function efficient and good for inlining.
*/
double BoundaryPointMapping::BoundaryDistance(int nFrom, int nTo) const
{
	double dist = 0;
	int i;

	if (nTo < nFrom) // must wrap around
	{
		for (i = 0; i < nTo; i++)
			dist += m_bndryPts->DataPointDistance(i, i + 1);

		nTo = m_bndryPts->Size() - 1;
	}

	for (i = nFrom; i < nTo; i++)
		dist += m_bndryPts->DataPointDistance(i, i + 1);

	return dist;
}

/*!
	@brief Assigns the boundary axis ratio (BAR) and the cumulative boundary distance 
	(cumBndryDist) to the boundary information of each flux point.

	The boundary distance is always computed by starting from the first point in the flux point list
*/
void BoundaryPointMapping::AssignBoundaryDistanceInfo(const sg::FluxPointArray& fpl, 
                                               const BoundaryInterval& interval, 
                                               char cSide,
                                               sg::BoundaryInfoArray& bil)
{
	ASSERT(cSide == '1' || cSide == '2');

	int idx0, idx1, currIdx, prevIdx;
	double cumBndryDist, bndryDist;

	bil[0][cSide].cumBndryDist = 0;

	prevIdx = bil[0][cSide].index;
	cumBndryDist = 0;

	// Check whether we should walk the "boundary" points in a forward/backward direction
	bool bForwardDir = (prevIdx == interval.First());

	// Note: the skeleton branch is always walked in a forward direction
	for (unsigned int i = 1; i < fpl.size(); i++, prevIdx = currIdx)
	{
		currIdx = bil[i][cSide].index;

		if (bForwardDir)
		{
			idx0 = prevIdx;
			idx1 = currIdx;
		}
		else
		{
			idx0 = currIdx;
			idx1 = prevIdx;
		}

		if (interval.IsSmallerThan(idx0, idx1))
		{
			bndryDist = BoundaryDistance(idx0, idx1);
			cumBndryDist += bndryDist;

			bil[i][cSide].cumBndryDist = cumBndryDist;
		}
		else // idx0 and idx1 should be equal then...
		{
			bil[i][cSide].cumBndryDist = cumBndryDist;

			if (idx0 != idx1)
			{
#ifdef DBG_SHOW_WARNINGS
				DBG_STREAM("Warning: rays cross. Data = {" << fpl[i] << bil[i][cSide] 
					<< "[" << idx0 << ", " << idx1 << "] in interval " 
					<< interval << "}\nFixing wrong data...\n")
#endif

				bil[i][cSide].pt = bil[i - 1][cSide].pt; // fix crossing rays
				bil[i][cSide].index = prevIdx;
				currIdx = prevIdx;
			}
		}
	}
}

/*!
	@brief Assigns skeleton distance information, both local and cumulative, to each skeleton point.
	The local distance is used to obtain the BAR ratio. using the precomputed local boundary distance.
	The cumulative distance is stored in the cumAxisDist field of each BoundaryInfo element.

	The axis distance is always computed by starting from the first point in the flux point list
*/
void BoundaryPointMapping::AssignAxisDistanceInfo(const sg::FluxPointArray& fpl, 
                                               sg::BoundaryInfoArray& bil)
{
	double dist, cumDist = 0;

	bil[0].cumAxisDist = 0;

	for (unsigned int i = 1; i < fpl.size(); i++)
	{
		dist = SkeletonDistance(fpl[i - 1], fpl[i]);
		cumDist += dist;

		ASSERT(dist > 0);

		bil[i].cumAxisDist = cumDist;
	}
}

