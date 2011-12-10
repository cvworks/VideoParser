/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __PIECEWISE_APROX_H__
#define __PIECEWISE_APROX_H__

#include "../MathUtils.h"
#include "../SmartArray.h"
#include "../SmartMatrix.h"
#include "../BasicTypes.h"

namespace vpl {

typedef SmartArray<Point> POINTS;

class MEMDATA
{
	int ptIdx;
	double dMinError;
	bool bEmpty;
	
public:	
	MEMDATA() { bEmpty = true; ptIdx = -1; dMinError = 0; }
	bool IsEmpty() const { return bEmpty; }
	int GetIdx() const { return ptIdx; }
	double GetMinError() const { return dMinError; }
	
	void Set(const double& e, int i)
	{
		ASSERT(i >= 0);
		bEmpty = false;
		ptIdx = i;
		dMinError = e;
	}
	double operator+(const MEMDATA& rhs) const
	{
		ASSERT(!bEmpty && !rhs.bEmpty);
		
		return dMinError + rhs.dMinError;
	}
	
	friend std::ostream& operator<<(std::ostream& os, const MEMDATA& d)
	{
		return os << "(" << d.dMinError << ", " << d.ptIdx << ")";
	}
};

typedef SmartArray< SmartMatrix<MEMDATA> > MEMORY;

struct SLOPE
{
	double m;

	bool bRelSmall;
	
	friend std::ostream& operator<<(std::ostream& os, const SLOPE& s)
	{
		return os << "(" << s.m << ", " << s.bRelSmall << ")";
	}
};

typedef SmartArray<SLOPE> SlopesArray;

template <class SEGMENT> class PiecewiseApprox
{
public:	
	struct KNOT
	{
		int nIndex;
		SEGMENT seg;
		double dError;
		int dir;

		KNOT() { nIndex = 0; }
		KNOT(int idx, double slope) { Set(idx, slope); }
		KNOT(int idx, const SEGMENT& s, const double& err) { Set(idx, s, err); }
		void Set(int idx, const SEGMENT& s, const double& err) { nIndex = idx; seg = s; dError = err; }
		void Set(int idx, double slope) { nIndex = idx; seg.m = slope; }
		
		friend std::ostream& operator<<(std::ostream& os, const KNOT& p)
		{
			return os << "Knot at " << p.nIndex << ", line segment " << p.seg
				<< ", fit error: " << p.dError;
		}
	};

	double m_dMinError, m_dMaxYDiff;
	int m_nMaxSegments;
	bool m_bDbgMode;
	double m_ymin, m_ymax;
	
	SmartArray<KNOT> m_knots;
	POINTS m_points;
	SmartMatrix<SEGMENT> m_segments;
	MEMORY m_minerrors;

public:
	PiecewiseApprox(double dMinError, int nMaxSegments, double dMaxYDiff = -1)
	{
		m_dMinError = dMinError;
		m_nMaxSegments = nMaxSegments;
		m_dMaxYDiff = dMaxYDiff;
		m_bDbgMode = false;
	}
	
	virtual ~PiecewiseApprox() { /*nothing to do here*/ }
	virtual double LeastSquares(const Point* vertices, int n, SEGMENT& s) = 0;
	virtual int GetSegmentDirection(const SEGMENT& s) const = 0;
	virtual void PlotKnots(int seg_num) const = 0;

	double LeastSquares(const Point* vertices, int fromIdx, int toIdx, SEGMENT& s)
	{
		return LeastSquares(vertices + fromIdx, toIdx - fromIdx + 1, s);
	}
	
	void Fit(const POINTS vertices);
	double FindMaxYDiff(int seg_num, int s, int e) const;
	
	MEMDATA Min(int seg_num, int s, int e);
	int AddKnots(int seg_num, int s, int e);
	void SetDbgMode(bool bOn) { m_bDbgMode = bOn; }
	void SetYMinMax();
	double CompAcuteAngle(int seg1, int seg2) const;
	double CompObtuseAngle(int seg1, int seg2) const;

	int GetPointSegmentIndex(int i) const;

	const SEGMENT& GetPointSegment(int i) const;
	
private:
	void FindMaxYDiff(int seg_num, int s, int e, const SEGMENT*& pFirstSeg, 
		const SEGMENT*& pLastSeg, double& dMaxYDiff) const;
};

#include "PiecewiseApprox.hxx" // Implementation of template functions

} //namespace vpl

#endif //__PIECEWISE_APROX_H__
