/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <DDSGraphProject.h>
#include <algorithm>
#include "DDSGraphUtils.h"
#include <Tools/MathUtils.h>

#define SMALL_NUM  0.00000001 // anything that avoids division overflow

//! Derreferences a pointer if not nul and assigns a value to it
#define DEREF_AND_SET(P, V) if (P) *P = V

using namespace sg;

/*!
	@brief Returns +alpha or -alpha depending on the side defined by 
	the vectors t and v. Both vectors must be given normalized.

	@return the angle that makes v aling with t. ie, v.rotate(alpha) == t.
	        To get the opposite boundary point v', just do t.rotate(alpha).
*/
double sg::ComputeObjectAngle(sg::Vector t, sg::Vector v)
{
	ASSERT(t.norm() > 0.99 && t.norm() < 1.01);
	ASSERT(v.norm() > 0.99 && v.norm() < 1.01);

	double alpha = acos(t.dot(v));

	// We don't know whether is alpha or -alpha, so simply check by 
	// This is inefficient but also th safest approach
	v.rotate(alpha);
	double d1 = (t.x - v.x) * (t.x - v.x) + (t.y - v.y) * (t.y - v.y);

	v.rotate(-2 * alpha); //ie, remove previous alpha rotation + new rotation
	double d2 = (t.x - v.x) * (t.x - v.x) + (t.y - v.y) * (t.y - v.y);

	return (d1 < d2) ? alpha:-alpha;
}

/*!
	@brief 
*/
void sg::GetBoundaryInfoAtJoint(const SkelJoint* pJoint, sg::BoundaryInfoArray *pBil, 
								 SkelBranch* pExcludeBranch /* = NULL */)
{
	const SkelBranch* pBranch;
	unsigned int idx;

	pBil->clear();
	pBil->reserve(pJoint->degree());

	forall_const_branches(pBranch, pJoint->edges)
	{
		if (pBranch != pExcludeBranch)
		{
			idx = (pBranch->firstXYPoint() == pJoint->fp.p) 
				? 0 : pBranch->size() - 1;

			ASSERT(pBranch->fluxPoint(idx).p == pJoint->fp.p);

			pBil->push_back(pBranch->boundaryInfo(idx));
		}
	}
}

//! Find the closest boundary points between the branches 
void sg::GetCommonBndryPoint(const SkelBranch* b0, const SkelBranch* b1,
						      SkelPtIndex* pIdx0, SkelPtIndex* pIdx1,
							  char* pSide0, char* pSide1)
{
	GetCommonSkelPtIndex(b0, b1, pIdx0, pIdx1);

	const sg::BoundaryInfoArray& bil0 = b0->getBoundaryInfoArray();
	const sg::BoundaryInfoArray& bil1 = b1->getBoundaryInfoArray();
	ASSERT(bil0.size() > 0 && bil1.size() > 0);

	double d11 = bil0[*pIdx0]['1'].pt.sqDist(bil1[*pIdx1]['1'].pt);
	double d12 = bil0[*pIdx0]['1'].pt.sqDist(bil1[*pIdx1]['2'].pt);
	double d21 = bil0[*pIdx0]['2'].pt.sqDist(bil1[*pIdx1]['1'].pt);
	double d22 = bil0[*pIdx0]['2'].pt.sqDist(bil1[*pIdx1]['2'].pt);

	if (d11 < d12 && d11 < d21 && d11 <= d22) // there may be a tie here
	{
		*pSide0 = '1';
		*pSide1 = '1';
	}
	else if (d12 < d11 && d12 <= d21 && d12 < d22) // there may be a tie here
	{
		*pSide0 = '1';
		*pSide1 = '2';
	}
	else if (d21 < d12 && d21 < d11 && d21 < d22) // no tie is possible
	{
		*pSide0 = '2';
		*pSide1 = '1';
	}
	else
	{
		ASSERT(d22 < d12 && d22 < d21 && d22 < d11); // no tie is possible

		*pSide0 = '2';
		*pSide1 = '2';
	}
}

/*!
	@brief Computes the area of the disk sector defined by the 'spokes'
	of the boundary information associated with the flux point.
*/
double sg::DiskSectorArea(const sg::FluxPoint& fp, const sg::BoundaryInfo& bi)
{
	sg::Vector v1 = bi.first.pt - fp.p;
	sg::Vector v2 = bi.second.pt - fp.p;

	v1.normalize();
	v2.normalize();

	double alpha = acos(v1.dot(v2));

	return 0.5 * (fp.dist * fp.dist) * alpha;
}

/*!
	@brief Computes the area in between spokes p0-b0, p1-b1. 

	The spokes define the true area if the boundary-to-axis ratio is small.
	In the case of large BAR, the boundary is more an arc than a line, and
	so the area we need is that of a disk segment. We compute a disk
	segment area only if boundary is three times the axis length or more.

	Note: when computing the actual disck sector area, we need to
	choose a radius. This choice must be invariant to the order
	in which the parameters are given. Then, we always choose the
	point with maximum radius.
*/
inline double sg::IntraSpokesArea(const SkelPtCoord& p0, const SkelPtCoord& b0, 
							  const SkelPtCoord& p1, const SkelPtCoord& b1)
{
	double b0b1 = b0.dist(b1);
	double p0p1 = p0.dist(p1);

	double p0b0 = p0.dist(b0);
	double p1b1 = p1.dist(b1);

	double p0b1 = p0.dist(b1);

	double upperArea = vpl::TriangleArea(p0b0, b0b1, p0b1);
	double lowerArea = vpl::TriangleArea(p0b1, p1b1, p0p1);

	double area = upperArea + lowerArea;

	// If we have large boundary-to-axis ratio, check if the disk segment
	// gives a larger area than the triangle approximation. This may not be
	// the case because the diagonal of the triangle is always larger than
	// the radius of the disk. ie, p0b1 > p0b0 && p0b1 > p1b1.
	if (b0b1 > 3 * p0p1)
	{
		double area1, area2;

		// We want to use the point with greatest radius
		if (p0b0 >= p1b1)
		{
			sg::Vector v1 = b0 - p0;
			sg::Vector v2 = b1 - p0;

			area1 = 0.5 * p0b0 * p0b0 * acos(v1.dot(v2) / (p0b0 * p0b1));
			area2 = lowerArea;
		}
		else
		{
			double p1b0 = p1.dist(b0);

			sg::Vector v1 = b0 - p1;
			sg::Vector v2 = b1 - p1;

			area1 = 0.5 * p1b1 * p1b1 * acos(v1.dot(v2) / (p1b0 * p1b1));
			area2 = vpl::TriangleArea(p1b0, p0b0, p0p1);
		}

		if (area1 + area2 > area) 
		{
			//DBG_MSG4("Updating area", area, area1 + area2, p0)
			area = area1 + area2;
		}
	}

	return area;
}

/*!
	@brief Computes the area of the disk sector defined by the 'spokes'
	of the boundary information associated with the flux points.
*/
double sg::BranchSegmentArea(const sg::FluxPointArray& fpl, 
							  const sg::BoundaryInfoArray& bil,
							  SkelPtIndex first, bool bSubtractAreaFirst,
							  SkelPtIndex last, bool bSubtractAreaLast)
{
	if (first == last)
	{
		return (bSubtractAreaFirst || bSubtractAreaLast) 
			? 0 : (M_PI * fpl[first].dist * fpl[first].dist);
	}

	ASSERT(first < last);

	// Compute total area of the segment
	double area = 0;

	for (SkelPtIndex i = first, j = first + 1; j <= last; i++, j++)
	{
		area += IntraSpokesArea(fpl[i].p, bil[i].first.pt, fpl[j].p, bil[j].first.pt);

		area += IntraSpokesArea(fpl[i].p, bil[i].second.pt, fpl[j].p, bil[j].second.pt);
	}

	// Compute the endpoint areas (EPA)
	double epa0, epaN;

	epa0 = DiskSectorArea(fpl[first], bil[first]);
	epaN = DiskSectorArea(fpl[last], bil[last]);

	// Subtract disk sector area of endpoints or add the COMPLEMET area
	if (bSubtractAreaFirst)
		area -= epa0;
	else
		area += M_PI * fpl[first].dist * fpl[first].dist - epa0;

	if (bSubtractAreaLast)
		area -= epaN;
	else
		area += M_PI * fpl[last].dist * fpl[last].dist - epaN;

	return area;
}

/*!
*/
void sg::ComputeTangentsAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
									 BranchInfoVect2& tangents, 
									 const SkelBranch* pExcludeBranch /*= NULL*/)
{
	SkelBranch* pBranch;
	SkelPtIndex idx, N, d;
	sg::Vector t;

	if (pExcludeBranch == NULL)
		tangents.resize(branches.size());
	else
		tangents.resize(branches.size() - 1);

	unsigned int i = 0;

	forall_const_branches(pBranch, branches)
	{
		if (pBranch == pExcludeBranch)
			continue;

		const sg::FluxPointArray& fpl    = pBranch->getFluxPoints();

		N = fpl.size() - 1;
		d = MIN(3, N);
		idx = (fpl[0].p == pJoint->fp.p) ? 0 : N;

		ASSERT(fpl[idx].p == pJoint->fp.p);

		// Tangents must flow ayaw from the joint
		if (idx == 0)
			t.set(fpl[d].p.x - fpl[0].p.x, fpl[d].p.y - fpl[0].p.y);
		else
			t.set(fpl[N - d].p.x - fpl[N].p.x, fpl[N - d].p.y - fpl[N].p.y);

		// We return normalized tangents
		t.normalize();

		tangents[i++] = std::make_pair(t, pBranch);
	}

	ASSERT((!pExcludeBranch && i == branches.size()) || 
		    (pExcludeBranch && i == branches.size() - 1));
}

/*!
	@brief Branches are paired up such that the angles between them are smallest.
*/
void sg::ComputeTangentAnglesAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
									 std::vector<double>& alphas, BranchPairs& branchPairs, 
									 const SkelBranch* pExcludeBranch /*= NULL*/)
{
	BranchInfoVect2 tangents;
	
	ComputeTangentsAtJoint(pJoint, branches, tangents, pExcludeBranch);

	alphas.clear();
	branchPairs.clear();

	if (tangents.size() == 2)
	{
		alphas.push_back(acos(tangents[0].first.dot(tangents[1].first)));
		branchPairs.push_back(std::make_pair(tangents[0].second, tangents[1].second));
	}
	else if (tangents.size() > 2)
	{
		BranchInfoVect2::iterator it0, it1, minIt;
		double minAngle, angle;
		
		while (tangents.size() > 1)
		{
			minAngle = 2 * M_PI;

			it1 = tangents.begin();
			it0 = it1++;
			
			for (; it1 != tangents.end(); it1++)
			{
				angle = acos(it0->first.dot(it1->first));

				if (angle < minAngle)
				{
					minAngle = angle;
					minIt = it1;
				}
			}

			ASSERT(minAngle < 2 * M_PI);

			alphas.push_back(minAngle);
			branchPairs.push_back(std::make_pair(it0->second, minIt->second));

			tangents.erase(it0);
			tangents.erase(minIt);
		}
	}
}

/*!
	@brief Computes the object angles of the endpoint of each given 
	branch that is equal the the joint point.

	The 'alphas' vector is filled with angle-branch pairs, and is sorted in ascending order
	by angle. Thus, alphas.back().first is the maximum angle.

	@param pExcludeBranch is optional. If given, it must be an incident branch
	to the joint. This branch is ignored when computing th eobject angles.

	@return the sum of all angles, which should be (approx) equal to PI.
*/
double sg::ComputeObjectAnglesAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
									 BranchInfoVect& alphas, 
									 const SkelBranch* pExcludeBranch /*= NULL*/)
{
	SkelBranch* pBranch;
	SkelPtIndex idx, N, d;
	double alpha, alpha1, alpha2;
	sg::Vector t, v1, v2;

	if (pExcludeBranch == NULL)
		alphas.resize(branches.size());
	else
		alphas.resize(branches.size() - 1);

	double angleSum = 0;
	unsigned int i = 0;

	forall_const_branches(pBranch, branches)
	{
		if (pBranch == pExcludeBranch)
			continue;

		const sg::FluxPointArray& fpl    = pBranch->getFluxPoints();
		const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

		N = fpl.size() - 1;
		d = MIN(3, N);
		idx = (fpl[0].p == pJoint->fp.p) ? 0 : N;

		ASSERT(fpl[idx].p == pJoint->fp.p);

		v1 = bil[idx].first.pt - fpl[idx].p;
		v2 = bil[idx].second.pt - fpl[idx].p;

		if (idx == 0)
			t.set(fpl[d].p.x - fpl[0].p.x, fpl[d].p.y - fpl[0].p.y);
		else
			t.set(fpl[N - d].p.x - fpl[N].p.x, fpl[N - d].p.y - fpl[N].p.y);

		v1.normalize();
		v2.normalize();
		t.normalize();

		alpha1 = ComputeObjectAngle(t, v1);
		alpha2 = ComputeObjectAngle(t, v2);

		/*std::cerr << "\n-----------------------------------\n";
		DBG_PRINT2(bil[idx].first.pt, bil[idx].second.pt)
		DBG_PRINT3(v1, v2, t)
		DBG_PRINT2(alpha1, alpha2)*/

		alpha = (fabs(alpha1) + fabs(alpha2)) / 2.0;

		angleSum += alpha;

		alphas[i++] = std::make_pair(alpha, pBranch);
	}

	ASSERT((!pExcludeBranch && i == branches.size()) || 
		    (pExcludeBranch && i == branches.size() - 1));

	std::sort(alphas.begin(), alphas.end());

	return angleSum;
}

/*!
	@brief Computes the area of each given branch. The area
	of the region where a branch overlaps the joint's disk is
	subtracted from the total branch's area.

	It returns a vector of area-branch pairs sorted (in ascending order)
	by area. Thus, areas.back().first is the maximum area.
*/
void sg::ComputeAreasAtJoint(SkelJoint* pJoint, const SkelEdges& branches,
						      BranchInfoVect& areas)
{
	SkelBranch* pBranch;
	SkelPtIndex idx, N;
	double area;

	areas.resize(branches.size());

	unsigned int i = 0;

	forall_const_branches(pBranch, branches)
	{
		const sg::FluxPointArray& fpl    = pBranch->getFluxPoints();
		const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

		N = fpl.size() - 1;
		idx = (fpl[0].p == pJoint->fp.p) ? 0 : N;

		ASSERT(fpl[idx].p == pJoint->fp.p);

		area = BranchSegmentArea(fpl, bil, 0, idx == 0, N, idx == N);

		areas[i++] = std::make_pair(area, pBranch);
	}

	std::sort(areas.begin(), areas.end());
}

/*!
	@brief Computes the object angle variances of all branches incident on 
	both endpoint of each given branch, while excluding the angles of the branch.

	The array of variances is sorted in ascending order by variance.
*/
void sg::ComputeObjectAngleVariance(const SkelEdges& branches, BranchInfoVect& variances)
{
	BranchInfoVect alphas1, alphas2;
	SkelBranch* pBranch;
	double meanAngle, var, dx;
	unsigned int i, j, N;

	variances.resize(branches.size());
	j = 0;

	forall_const_branches(pBranch, branches)
	{
		N = pBranch->n1->degree() + pBranch->n2->degree() - 2;

		meanAngle = ComputeObjectAnglesAtJoint(pBranch->n1, 
			pBranch->n1->edges, alphas1, pBranch);

		meanAngle += ComputeObjectAnglesAtJoint(pBranch->n2, 
			pBranch->n2->edges, alphas2, pBranch);

		meanAngle /= N;

		var = 0;

		for (i = 0; i < alphas1.size(); i++)
		{
			dx = alphas1[i].first - meanAngle;
			var += dx * dx;
		}

		for (i = 0; i < alphas2.size(); i++)
		{
			dx = alphas2[i].first - meanAngle;
			var += dx * dx;
		}

		variances[j++] = std::make_pair(var / N, pBranch);
	}

	std::sort(variances.begin(), variances.end());
}

/*!
	@brief Returns true iff the spoke [S1_P0, S1_P1] crosses the spoke
	[S2_P0, S2_P1].

	@param S1_P0 origin of the first spoke
	@param S1_P1 endpoint of the first spoke
	@param S2_P0 origin of the second spoke
	@param S2_P1 endpoint of the second spoke

    @return true iff the spokes intersect anywere but at their endpoints
*/
bool sg::DoSpokesIntersect(const sg::Point& S1_P0, const sg::Point& S1_P1,
							const sg::Point& S2_P0, const sg::Point& S2_P1)
{
	if (S1_P1 == S2_P1)
		return false; // spokes have the same endpoint. not an intersection.

	vpl::Vector2D u(S1_P1.x - S1_P0.x, S1_P1.y - S1_P0.y);
    vpl::Vector2D v(S2_P1.x - S2_P0.x, S2_P1.y - S2_P0.y);
    vpl::Vector2D w(S1_P0.x - S2_P0.x, S1_P0.y - S2_P0.y);

    double D = u.Perp(v);

	// test if they are parallel (includes either being a point)
    if (fabs(D) < SMALL_NUM)            // S1 and S2 are parallel
	{          
        if (u.Perp(w) != 0 || v.Perp(w) != 0) 
		{
            return false;                   // they are NOT collinear
        }

        // they are collinear or degenerate
        // check if they are degenerate points
        double du = u.Dot(u);
        double dv = v.Dot(v);

        if (du == 0 && dv == 0)         // both segments are points
		{
            if (S1_P0 != S2_P0)         // they are distinct points
                return false;
  
            return true;
        }

        if (du == 0)                    // S1 is a single point
			return false;
		/*{
            if (!IsInSegment(S1_P0, S2_P0, S2_P1))  // but is not in S2
                return false;

            return true;
        }*/

        if (dv == 0)                    // S2 a single point
			return false;
		/*{
            if (!IsInSegment(S2_P0, S1_P0, S1_P1))  // but is not in S1
                return false;

            return true;
        }*/

        // they are collinear segments - get overlap (or not)
        double t0, t1;                   // endpoints of S1 in eqn for S2
		vpl::Vector2D w2(S1_P1.x - S2_P0.x, S1_P1.y - S2_P0.y);

        if (v.x != 0) 
		{
                t0 = w.x / v.x;
                t1 = w2.x / v.x;
        }
        else 
		{
                t0 = w.y / v.y;
                t1 = w2.y / v.y;
        }

        if (t0 > t1)                    // must have t0 smaller than t1
		{
                double t = t0; 

				t0 = t1; 
				t1 = t;    // swap if not
        }

        if (t0 > 1 || t1 < 0)
            return false;     // NO overlap

		return true;
    }

    // the segments are skew and may intersect in a point
    // get the intersect parameter for S1
    double sI = v.Perp(w) / D;

	// get the intersect parameter for S2
    double tI = u.Perp(w) / D;

    return (sI >= 0 && sI <= 1 && tI >= 0 && tI <= 1);
}

/*!
	Computes the unsigned angle between the pair of spokes with
	origin at he joint point.
*/
double sg::ComputeSpokeAngleAtJoint(const SkelBranch* pBranch, SkelJoint* pJoint)
{
	SkelPtIndex i = GetEndpointIndex(pBranch, pJoint);

	const SkelPt& fp = pBranch->fluxPoint(i);
	const SkelPtBndryInfo& bi = pBranch->boundaryInfo(i);

	sg::Point v0 = bi.first.pt - fp.p;
	sg::Point v1 = bi.second.pt - fp.p;

	double n0 = v0.norm();
	double n1 = v1.norm();

	if (n0 == 0 || n1 == 0)
		return M_PI;

	// return an unsigned angle
	return fabs(SignedVectorAngle(v0, v1, n0, n1));
}

/*!
	
*/
bool sg::DoBranchSpokeIntersect(const SkelBranch* pBranch, 
								 const sg::Point& p0, const sg::Point& p1)
{
	const sg::FluxPointArray& fpl = pBranch->getFluxPoints();
	sg::Point intPt;

	for (unsigned int i = 1; i < fpl.size(); i++)
	{
		if (FindLineSegmentIntersection(fpl[i - 1].p, fpl[i].p, 
			p0, p1, &intPt) == 1)
		{
			return true;
		}
	}

	return false;
}

/*!
	Finds a boundary point on the given side that is different from the endpoints
	first and last spokes of the branch on that side.

	//Non gap poins are chosen over gap points whenever possible.

	return zero if no unique point is found. Otherwise, it returns an index greater 
	than zero and smaller that one minus the total number of points.
*/
unsigned int sg::GetUniqueInnerBoundaryPoint(const SkelBranch* pBranch, char side)
{
	const sg::BoundaryInfoArray& bil = pBranch->getBoundaryInfoArray();

	// Ensure that list isn't empty
	if (bil.empty())
		return 0;

	const sg::BoundaryPoint& firstPt = bil.front()[side];
	const sg::BoundaryPoint& lastPt = bil.back()[side];

	const unsigned int N = bil.size() - 1; // bil is not empty!
	unsigned int gapIdx = 0;

	for (unsigned int i = 1; i < N; i++)
	{
		const sg::BoundaryPoint& bp = bil[i][side];

		if (bp.index >= 0) // not a gap point
		{
			if (bp.index != firstPt.index && bp.index != lastPt.index)
				return i; // we are done
		}
		else if (gapIdx == 0)
		{
			if (bp.pt != firstPt.pt && bp.pt != lastPt.pt)
				gapIdx = i; // we may still find a non gap point
		}
	}

	return gapIdx;
}
