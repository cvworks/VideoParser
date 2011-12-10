/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _DDSGRAPH_UTILS_H_
#define _DDSGRAPH_UTILS_H_

#include <Tools/BasicTypes.h>
#include <Tools/HelperFunctions.h>

#define MAX_NUM_PTS_FOR_TANGENT 3

namespace sg {

typedef std::vector< std::pair<DDSEdge*, DDSEdge*> > BranchPairs;
typedef std::vector< std::pair<double, DDSEdge*> > BranchInfoVect;
typedef std::vector< std::pair<Vector, DDSEdge*> > BranchInfoVect2;

//////////////////////////////////////////////////////////////////////////////////////////
// Simple helper functions for data display

#define PRN_JOINT_PTR(J) "(" << J->fp.p.x << "," << J->fp.p.y << ")"

#define PRN_BRANCH_PTR(B) "{" << PRN_JOINT_PTR(B->n1) << ", " \
	<< PRN_JOINT_PTR(B->n2) << "}[" << B->size() << "]"

//////////////////////////////////////////////////////////////////////////////////////////
// Iteration loops for branches in a DDSGrpah

/*! 
	Handy funtion to iterate through branches in a DDSGraph skeleton.

	@param B a DDSEdge* variable used to access each branch
	@param E a DDSEdgeVect container
*/
#define forall_branches(B, E) \
	for (sg::DDSEdgeVect::iterator I = E.begin(); I != E.end() && (B = *I) != NULL; I++)

/*! 
	Handy funtion to iterate through CONST branches in a DDSGraph skeleton.

	@param B a const DDSEdge* variable used to access each branch
	@param E a DDSEdgeVect container (can be const)
*/
#define forall_const_branches(B, E) \
	for (sg::DDSEdgeVect::const_iterator I = E.begin(); I != E.end() && (B = *I) != NULL; I++)

//////////////////////////////////////////////////////////////////////////////////////////
// Iteration loops for endpoints in a DDSGrpah

/*! 
	Handy funtion to iterate through endpoints in a DDSGraph skeleton.

	@param J a DDSNode* variable used to access each endpoint
	@param V a DDSNodeVect container
*/
#define forall_endpts(J, V) \
	for (sg::DDSNodeVect::iterator I = V.begin(); I != V.end() && (J = *I) != NULL; I++)

/*! 
	Handy funtion to iterate through CONST endpoints in a DDSGraph skeleton.

	@param J a const DDSNode* variable used to access each endpoint
	@param V a DDSNodeVect container (can be const)
*/
#define forall_const_endpts(J, V) \
	for (sg::DDSNodeVect::const_iterator I = V.begin(); I != V.end() && (J = *I) != NULL; I++)

//////////////////////////////////////////////////////////////////////////////////////////
// Iteration loops for joints in a DDSGrpah

/*! 
	Handy funtion to iterate through joints in a DDSGraph skeleton. A joint is an
	endpoint with degree greater than 1.

	@param J a DDSNode* variable used to access each joint
	@param V a DDSNodeVect container
*/
#define forall_joints(J, V) \
	for (sg::DDSNodeVect::iterator I = V.begin(); I != V.end() && (J = *I) != NULL; I++) \
		if (J->degree() > 1)

/*! 
	Handy funtion to iterate through CONST joints in a DDSGraph skeleton. A joint is an
	endpoint with degree greater than 1.

	@param J a const DDSNode* variable used to access each joint
	@param V a DDSNodeVect container (can be const)
*/
#define forall_const_joints(J, V) \
	for (sg::DDSNodeVect::const_iterator I = V.begin(); I != V.end() && (J = *I) != NULL; I++) \
		if (J->degree() > 1)

//////////////////////////////////////////////////////////////////////////////////////////
// Iteration loops for terminal points in a DDSGrpah

/*! 
	Handy funtion to iterate through terminals in a DDSGraph skeleton. A terminal is an
	endpoint with degree equal to 1.

	@param T a DDSNode* variable used to access each joint
	@param V a DDSNodeVect container
*/
#define forall_terminals(T, V) \
	for (sg::DDSNodeVect::iterator I = V.begin(); I != V.end() && (T = *I) != NULL; I++) \
		if (T->degree() == 1)

/*! 
	Handy funtion to iterate through CONST terminals in a DDSGraph skeleton. A terminal is an
	endpoint with degree equal to 1.

	@param T a const DDSNode* variable used to access each joint
	@param V a DDSNodeVect container (can be const)
*/
#define forall_const_terminals(T, V) \
	for (sg::DDSNodeVect::const_iterator I = V.begin(); I != V.end() && (T = *I) != NULL; I++) \
		if (T->degree() == 1)

//////////////////////////////////////////////////////////////////////////////////////////
// Typedefs for iterators of nodes and edges in a DDSGrpah

//! Natural name for a branch iterator in a DDSGraph
typedef DDSEdgeVect::iterator BranchIt;

//! Natural name for a const branch iterator in a DDSGraph
typedef DDSEdgeVect::const_iterator BranchConstIt;

//! Natural name for an endpoint iterator in a DDSGraph
typedef DDSNodeVect::iterator EndptIt;

//! Natural name for a const endpoint iterator in a DDSGraph
typedef DDSNodeVect::const_iterator EndptConstIt;

//! Natural name for a joint iterator in a DDSGraph
typedef DDSNodeVect::iterator JointIt;

//! Natural name for a const joint iterator in a DDSGraph
typedef DDSNodeVect::const_iterator JointConstIt;

//////////////////////////////////////////////////////////////////////////////////////////
// Typedefs for common DDSGrpah entities

typedef DDSNode SkelNode;
typedef DDSEdge SkelEdge;

typedef DDSNodeVect SkelNodes;
typedef DDSEdgeVect SkelEdges;

typedef DDSEdge SkelBranch;
typedef DDSNode SkelEndpt;

typedef DDSNode SkelJoint;
typedef DDSNode SkelTerminal;

typedef FluxPoint SkelPt;
typedef BoundaryInfo SkelPtBndryInfo;
typedef Point SkelPtCoord;

typedef unsigned int SkelPtIndex;

//////////////////////////////////////////////////////////////////////////////////////////
// Useful functions related to DDSGraph

inline double IntraSpokesArea(const SkelPtCoord& p0, const SkelPtCoord& b0, 
							  const SkelPtCoord& p1, const SkelPtCoord& b1);

inline double DiskSectorArea(const FluxPoint& fp, const BoundaryInfo& bi);

double ComputeObjectAngle(Vector t, Vector v);

void GetBoundaryInfoAtJoint(const SkelJoint* pJoint, 
	BoundaryInfoArray *pBil, SkelBranch* pExcludeBranch = NULL);

void GetCommonBndryPoint(const SkelBranch* b0, const SkelBranch* b1, 
    SkelPtIndex* pIdx0, SkelPtIndex* pIdx1, char* pSide0, char* pSide1);

double BranchSegmentArea(const FluxPointArray& fpl, const BoundaryInfoArray& bil,
	SkelPtIndex first, bool bSubtractAreaFirst, SkelPtIndex last, bool bSubtractAreaLast);

double ComputeObjectAnglesAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
					     BranchInfoVect& alphas, const SkelBranch* pExcludeBranch = NULL);

void ComputeAreasAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
						 BranchInfoVect& areas);

void ComputeTangentsAtJoint(SkelJoint* pJoint, const SkelEdges& branches, 
					BranchInfoVect2& tangents, const SkelBranch* pExcludeBranch = NULL);

void ComputeTangentAnglesAtJoint(SkelJoint* pJoint, const SkelEdges& branches, 
						std::vector<double>& alphas, BranchPairs& branchPairs,
						const SkelBranch* pExcludeBranch = NULL);

void ComputeObjectAngleVariance(const SkelEdges& branches, BranchInfoVect& variances);

bool DoSpokesIntersect(const Point& S1_P0, const Point& S1_P1,
					   const Point& S2_P0, const Point& S2_P1);

bool DoBranchSpokeIntersect(const SkelBranch* pBranch, const Point& p0, 
							const Point& p1);

double ComputeSpokeAngleAtJoint(const SkelBranch* pBranch, SkelJoint* pJoint);

unsigned int GetUniqueInnerBoundaryPoint(const SkelBranch* pBranch, char side);

//! Gets the opposite side to the given side
inline char FlipSide(char side)
{
	return BoundaryInfo::FlipSide(side);
}

//! Finds the index of the branch that is incident on the given joint
inline unsigned int FindIncidentBranchIndex(SkelJoint* pJoint, 
											SkelBranch* pBranch)
{
	for(unsigned int i = 0; i < pJoint->edges.size(); i++)
	{
		if (pJoint->edges[i] == pBranch)
			return i;
	}

	return pJoint->edges.size();
}

//! Return true iff the skeletal points are connected in an 8-pixel neighborhood
inline bool ArePtsConnected(const SkelPt& fp0, const SkelPt& fp1)
{
	return (fabs(fp0.p.x - fp1.p.x) <= 1 && fabs(fp0.p.y - fp1.p.y) <= 1);
}

//! Assignment operator for Point = sg:Point
inline bool ArePtsEqual(const Point& lhs, const Point& rhs)
{
	return (lhs.x == rhs.x && lhs.y == rhs.y);
}

//! Find the common joint between the branches
inline SkelJoint* GetCommonSkelJoint(const SkelBranch* b0, const SkelBranch* b1)
{	
	if (b0->n1 == b1->n1 || b0->n1 == b1->n2)
		return b0->n1;
	else
	{
		ASSERT(b0->n2 == b1->n1 || b0->n2 == b1->n2);
		return b0->n2;
	}
}

//! Find the common joint between the branches
inline void GetCommonSkelPtIndex(const SkelBranch* b0, const SkelBranch* b1,
								 SkelPtIndex* pIdx0, SkelPtIndex* pIdx1)
{	
	const FluxPointArray& fpl0 = b0->getFluxPoints();
	const FluxPointArray& fpl1 = b1->getFluxPoints();

	if (fpl0.front().p == fpl1.front().p)
	{
		*pIdx0 = 0;
		*pIdx1 = 0;
	}
	else if (fpl0.front().p == fpl1.back().p)
	{
		*pIdx0 = 0;
		*pIdx1 = fpl1.size() - 1;
	}
	else if (fpl0.back().p == fpl1.front().p)
	{
		*pIdx0 = fpl0.size() - 1;
		*pIdx1 = 0;
	}
	else
	{
		ASSERT(fpl0.back().p == fpl1.back().p);

		*pIdx0 = fpl0.size() - 1;
		*pIdx1 = fpl1.size() - 1;
	}
}

//! Returns true iff the segments a0-a1 and a2-a3 intersect
inline bool DoSegmentsIntersect(const Point& a0, const Point& a1,
                                const Point& a2, const Point& a3)
{
	return (FindLineSegmentIntersection(a0, a1, a2, a3, &Point()) == 1);
}

//! Returns true iff the segment p1-p2 and the circle defined by fp intersect
inline bool DoSegmentCircleIntersect(const Point& p1, const Point& p2, 
									 const FluxPoint& fp)
{
	return DoSegmentCircleIntersect(p1, p2, fp.p, fabs(fp.dist));
}

//! Returns the endpoint of pBranch that is not pJoint
inline const SkelJoint* GetOtherEndpoint(const SkelBranch* pBranch, const SkelJoint* pJoint)
{
	ASSERT(pBranch->n1 == pJoint || pBranch->n2 == pJoint);

	return (pBranch->n1 == pJoint) ? pBranch->n2 : pBranch->n1;
}

//! Returns the endpoint of pBranch that is not pJoint
inline SkelJoint* GetOtherEndpoint(SkelBranch* pBranch, SkelJoint* pJoint)
{
	ASSERT(pBranch->n1 == pJoint || pBranch->n2 == pJoint);

	return (pBranch->n1 == pJoint) ? pBranch->n2 : pBranch->n1;
}

//! Gets the index of the branch endpoint incident at the given joint
inline SkelPtIndex GetEndpointIndex(const SkelBranch* pBranch, const SkelJoint* pJoint)
{
	if (pBranch->firstXYPoint() == pJoint->fp.p)
	{
		return 0;
	}
	else
	{
		ASSERT(pBranch->lastXYPoint() == pJoint->fp.p);

		return pBranch->size() - 1;
	}
}

//! Gets the index of the branch endpoint NOT incident at the given joint
inline SkelPtIndex GetOtherEndpointIndex(const SkelBranch* pBranch, const SkelJoint* pJoint)
{
	return GetEndpointIndex(pBranch, GetOtherEndpoint(pBranch, pJoint));
}

//! Gets the index of the branch endpoint incident at the given joint
inline const SkelPtBndryInfo& GetEndpointInfo(const SkelBranch* pBranch, 
											  const SkelJoint* pJoint)
{
	if (pBranch->firstXYPoint() == pJoint->fp.p)
	{
		return pBranch->firstBndryInfo();
	}
	else
	{
		ASSERT(pBranch->lastXYPoint() == pJoint->fp.p);

		return pBranch->lastBndryInfo();
	}
}

/*!
	Finds the bounary side on each branch that corresponds to the 
	boundary point shared by the branches.

	It is assumed that both branches are incident on the given joint.
*/
inline std::pair<char,char> GetSidesOfSharedBndryPt(const SkelBranch* pBranch0,
													const SkelBranch* pBranch1,
													const SkelJoint* pJoint)
{
	ASSERT(pJoint->degree() >= 3);

	const SkelPtBndryInfo& bi0 = GetEndpointInfo(pBranch0, pJoint);
	const SkelPtBndryInfo& bi1 = GetEndpointInfo(pBranch1, pJoint);
	char s0, s1;;

	for (s0 = '1'; s0 <= '2'; s0++)
		for (s1 = '1'; s1 <= '2'; s1++)
			if (bi0[s0].pt == bi1[s1].pt)
				return std::make_pair(s0, s1);

	// ASSERT(false);
	// return std::make_pair('0', '0');

	double d, minSqDist;
	char min_s0, min_s1;

	for (s0 = '1'; s0 <= '2'; s0++)
	{
		for (s1 = '1'; s1 <= '2'; s1++)
		{
			d = bi0[s0].pt.sqDist(bi1[s1].pt);

			if ((s0 == '1' && s1 == '1') || d < minSqDist)
			{
				min_s0    = s0;
				min_s1    = s1;
				minSqDist = d;
			}
		}
	}

	DBG_MSG4("No spokes at joint ", PRN_JOINT_PTR(pJoint), 
		" ends at same boundary endpoint. Closest dist is ", sqrt(minSqDist))

	return std::make_pair(min_s0, min_s1);
}

//! Returns true if pJoint is and endpoint of pBranch, and false otherwise
inline bool IsEndpointOf(const SkelJoint* pJoint, const SkelBranch* pBranch)
{
	return (pBranch->n1 == pJoint || pBranch->n2 == pJoint);
}

//! Returns true iff n1 is the first joint and n2 is the last
inline bool AreJointsOrdered(const SkelBranch* pBranch)
{
	ASSERT(pBranch->n1->fp.p == pBranch->firstXYPoint() ||
		pBranch->n1->fp.p == pBranch->lastXYPoint());

	ASSERT(pBranch->n2->fp.p == pBranch->firstXYPoint() ||
		pBranch->n2->fp.p == pBranch->lastXYPoint());

	return (pBranch->n1->fp.p == pBranch->firstXYPoint());
}

//! Returns the joint related to the first flux point in the branch
inline const SkelJoint* GetFirstJoint(const SkelBranch* pBranch)
{
	return (AreJointsOrdered(pBranch)) ? pBranch->n1 : pBranch->n2;
}

//! Returns the joint related to the first flux point in the branch
inline SkelJoint* GetFirstJoint(SkelBranch* pBranch)
{
	return (AreJointsOrdered(pBranch)) ? pBranch->n1 : pBranch->n2;
}

//! Returns the joint related to the last flux point in the branch
inline const SkelJoint* GetLastJoint(const SkelBranch* pBranch)
{
	return (AreJointsOrdered(pBranch)) ? pBranch->n2 : pBranch->n1;
}

//! Returns the joint related to the last flux point in the branch
inline SkelJoint* GetLastJoint(SkelBranch* pBranch)
{
	return (AreJointsOrdered(pBranch)) ? pBranch->n2 : pBranch->n1;
}

inline unsigned int LargestBranchSize(const SkelJoint* pJoint)
{
	const SkelBranch* pBranch;
	unsigned int sz = 0;

	forall_const_branches(pBranch, pJoint->edges)
	{
		if (pBranch->size() > sz)
			sz = pBranch->size();
	}

	return sz;
}

/*! 
	Gets the normalized tangent at the given endpoint in the forward direction
	difined by the indices of the points. That is, the tangent is computed as
	a vector whose origin is a skeletal point with index smaller than the
	skeletal point representing the endpoint of the vector.
*/
inline bool GetEndpointForwardTangent(const SkelBranch* pBranch, SkelPtIndex idx, Vector* pTan)
{
	const FluxPointArray& fpl = pBranch->getFluxPoints();

	ASSERT(idx == 0 || idx == fpl.size() - 1);
	
	const int N = fpl.size() - 1;
	const int d = MIN(3, N);

	if (idx == 0)
	{
		pTan->set(fpl[d].p.x - fpl[0].p.x, fpl[d].p.y - fpl[0].p.y);

		pTan->normalize();

		return fpl[d].radius() <= fpl[0].radius();;
	}
	else
	{
		pTan->set(fpl[N].p.x - fpl[N - d].p.x, fpl[N].p.y - fpl[N - d].p.y);

		pTan->normalize();

		return fpl[N].radius() <= fpl[N - d].radius();;
	}
}

//! Gets the CCW angle of the outward tangent at the given endpoint
inline double GetBranchAngleAtEndpoint(const SkelBranch* pBranch, SkelPtIndex idx)
{
	const FluxPointArray& fpl = pBranch->getFluxPoints();

	ASSERT(idx == 0 || idx == fpl.size() - 1);
	
	const int N = fpl.size() - 1;
	const int d = MIN(MAX_NUM_PTS_FOR_TANGENT, N);

	if (idx == 0)
	{
		return atan2(fpl[d].p.y - fpl[0].p.y, fpl[d].p.x - fpl[0].p.x);
	}
	else
	{
		return atan2(fpl[N - d].p.y - fpl[N].p.y, fpl[N - d].p.x - fpl[N].p.x);
	}
}

//! Computes the signed angle between two vectors
inline double SignedVectorAngle(const Point& p0, const Point& p1, 
						  const double& p0Norm, const double& p1Norm)
{
	return vpl::SignedVectorAngle(p0.x, p0.y, p1.x, p1.y, p0Norm, p1Norm);
}

//! Computes the signed angle between two vectors
inline double SignedVectorAngle(const Vector& p0, const Vector& p1, 
						  const double& p0Norm, const double& p1Norm)
{
	return vpl::SignedVectorAngle(p0.x, p0.y, p1.x, p1.y, p0Norm, p1Norm);
}

} // namespace vpl

#endif //_DDSGRAPH_UTILS_H_
