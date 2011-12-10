/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _BOUNDARY_POINTS_FINDER_H_
#define _BOUNDARY_POINTS_FINDER_H_

#include <vector>
#include <DDSGraphProject.h>
#include <DDSGraphUtils.h>
#include <Tools/KDTree.h>
#include <Tools/BasicUtils.h>

namespace sg {

/*!
	@brief Use an object of this class to find the pair of
	associated boundary points to each skeleton point. 

	Since joint points are repeated in each branch, each version of a joint
	point will be assigned a pair of boundary points that depends on the
	branch the joint is in.
*/
class BoundaryPointMapping
{
	/*!
		Represents the tangent, and CCW tangent angle [0,2PI] of a skeletal 
		point, and a pointer to its boundary info, so that it can be be updated
		using the stored tangent and angle info.
	*/
	class AngleInfo
	{
		double m_dAngle;                  //<! CCW tangent angle in [0,2PI]
		sg::BoundaryInfo* m_pBndryInfo;   //<! pointer to bndryinfo to update
		sg::Vector m_tangent;             //<! normalized tangent
		SkelBranch* m_pBranch;            //<! branch that contains bndry info pt

	public:
		void Set(sg::BoundaryInfo* pBndryInfo, const double& angle, 
			const sg::Vector& tangent, SkelBranch* pBranch)
		{
			m_pBndryInfo = pBndryInfo;
			m_dAngle = angle;
			m_tangent = tangent;
			m_pBranch = pBranch;
		}

		const double& Angle() const
		{
			return m_dAngle;
		}

		const sg::Vector& Tangent() const
		{
			return m_tangent;
		}

		SkelBranch* Branch() const
		{
			return m_pBranch;
		}

		bool operator<(const AngleInfo& rhs) const
		{
			return m_dAngle < rhs.m_dAngle;
		}

		/*!
			Retrieves the boundary information on the side corresponding 
			to the sign of a given object angle.
		*/
		sg::BoundaryPoint& GetBndryPt(const double& objectAngle) const
		{
			return (objectAngle >= 0) ? m_pBndryInfo->first : m_pBndryInfo->second;
		}
	};

	typedef std::vector<AngleInfo> JointAngles;

protected:
	KDTree* m_bndryPts;

protected:
	// Inline functions
	inline int GetValidBoundaryIndex(const SkelBranch* pBranch, char side, 
		const sg::BoundaryPoint& bp);

	inline bool TestBoundaryInterval(const int firstIdx, const int lastIdx,
		const int otherIdx0, const int otherIdxN, const int testIndex) const;

	inline void SetBoundaryInterval(const int firstIdx, const int lastIdx, const int otherIdx0, 
		const int otherIdxN, BoundaryInterval* pBndryInt) const;

	inline void SetBoundaryInterval(const char side, const sg::BoundaryInfo& bi0, 
		const sg::BoundaryInfo& biN, BoundaryInterval* pBndryInt) const;

	inline void SetBoundaryInterval(const SkelBranch* pBranch, const char side,
		BoundaryInterval* pBndryInt);

	inline void EnsureValidInterval(const sg::BoundaryInfo& bi0, sg::BoundaryInfo* pBiN);

	//! Returns true iff the endpoint spokes on the given side don't cross one another
	bool ValidateEndpointSpokes(const SkelBranch* pBranch, char side)
	{
		return !DoSpokesIntersect(pBranch->firstXYPoint(), 
		                          pBranch->firstBndryInfo()[side].pt, 
		                          pBranch->lastXYPoint(), 
		                          pBranch->lastBndryInfo()[side].pt);
	}

	// Non-inline functions

	void FixCrossingEndpointSpokes(SkelBranch* pBranch, char side);

	void FixSpecialCrossingCase(SkelBranch* pBranch, char side);

	sg::BoundaryPoint FindClosestBoundaryPoint(const sg::Point& p0, 
		const sg::Point& p1, const BoundaryInterval& bndryInt) const;

	void SetBoundaryInfoAtJoint(SkelJoint* pJoint, const JointAngles& angles);

	void SetBoundaryInfoAtTerminal(const sg::FluxPoint& fp0, sg::Vector tangent, 
		bool bDecreasingRadius, const sg::FluxPoint& fp1, const SkelBranch* pBranch,
		sg::BoundaryInfo& bi0);

	void SetInnerBoundaryInfo(const sg::FluxPointArray& fpl, const BoundaryInterval& side1, 
		const BoundaryInterval& side2, sg::BoundaryInfoArray& bil);

	void AssignInnerBoundaryPoints(const sg::FluxPointArray& fpl, sg::BoundaryInfoArray& bil);

	inline double BoundaryDistance(int nFrom, int nTo) const;

	double SkeletonDistance(const sg::FluxPoint& fp0, const sg::FluxPoint& fp1) const
	{
		double dx = fp1.p.x - fp0.p.x;
		double dy = fp1.p.y - fp0.p.y;

		return sqrt(dx * dx + dy * dy);
	}

	void AssignBoundaryDistanceInfo(const sg::FluxPointArray& fpl, const BoundaryInterval& interval, 
		char cSide, sg::BoundaryInfoArray& bil);

	void AssignAxisDistanceInfo(const sg::FluxPointArray& fpl, sg::BoundaryInfoArray& bil);

public:
	BoundaryPointMapping(KDTree* bndryPts)
	{	
		m_bndryPts = bndryPts;
	}
	
	void AssignRadiusValues(const sg::BoundaryInfoArray& bil, sg::FluxPointArray& fpl);

	void AssignBoundaryPoints(const sg::FluxPointArray& fpl, sg::BoundaryInfoArray& bil,
		const sg::BoundaryInfo& bi0, const sg::BoundaryInfo& biN);

	void AssignBoundaryPoints(sg::DDSGraph* pDDSGraph);

	void GetBoundaryPoint(int i, sg::Point* p) const
	{
		m_bndryPts->GetDataPoint(i, p->x, p->y);
	}

	static double ComputeEndPointOverlap(const sg::FluxPointArray& fpl);
};

} // namespace vpl

#endif //_BOUNDARY_POINTS_FINDER_H_
