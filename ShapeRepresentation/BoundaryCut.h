/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _BOUNDARY_CUT_H_
#define _BOUNDARY_CUT_H_

#include "FluxSkeleton\include\DDSEdge.h"
#include <set>

namespace vpl {

/*!
*/
class BoundaryCut
{
	int m_from;
	int m_to;

	const sg::DDSEdge* m_symAxis;
	unsigned int m_medialPtIdx;

public:
	BoundaryCut() 
	{ 
		m_symAxis = NULL; 
	}

	/*!
		Sets the parameters of the boundary cut such that
		'from' is MIN(idx0, idx1) and 'to' is MAX(idx0, idx1)
	*/
	void Set(int idx0, int idx1, const sg::DDSEdge* symAxis,
		unsigned int medialPtIdx)
	{
		// Ensure that m_from < m_to
		if (idx0 < idx1)
		{
			m_from = idx0;
			m_to = idx1;
		}
		else
		{
			//DBG_PRINT4(idx0, idx1, medialPtIdx, symAxis)
			ASSERT(idx0 != idx1);

			m_from = idx1;
			m_to = idx0;
		}

		m_symAxis = symAxis;
		m_medialPtIdx = medialPtIdx;
	}

	int From() const { return m_from; }
	int To() const   { return m_to; }

	const sg::DDSEdge* SymmetryAxis() const { return m_symAxis; }
	unsigned int MedialPointIdx() const     { return m_medialPtIdx; }

	const Point& MedialPoint() const 
	{ 
		return m_symAxis->fluxPoint(m_medialPtIdx).p; 
	}

	/*!
		Two cuts are equal iff their 'from', 'to' and
		'symmetry axis' fields are equal.
	*/
	bool operator==(const BoundaryCut& rhs) const
	{
		return (m_from == rhs.m_from && m_to == rhs.m_to
			&& m_symAxis == rhs.m_symAxis);
	}

	/*!
		The cuts are sorted by their 'from', 'to' and
		'symmetry axis' fields, respectively.
	*/
	bool operator<(const BoundaryCut& rhs) const
	{
		// There are three "fields" to compare. This could be grouped
		// all together in one line, but it's more readable in this way
		if (m_from < rhs.m_from)
			return true;
		else if (m_from == rhs.m_from && m_to < rhs.m_to)
			return true;
		else if (m_from == rhs.m_from && m_to == rhs.m_to && m_symAxis < rhs.m_symAxis)
			return true;

		// We don't care about the particular m_medialPtIdx, as it does
		// not make a cut different than another. ie, see operator==()

		return false;
	}

	/*!
		Returns the opposite boundary cut endpoint to the one given.
	*/
	int GetOtherEndpoint(int idx) const
	{
		ASSERT(idx == m_from || idx == m_to);

		return (idx == m_from) ? m_to : m_from;
	}
};

//! Set of bounday cuts
typedef std::set< BoundaryCut, std::less<BoundaryCut> > BoundaryCutSet;

} //namespace vpl

#endif // _BOUNDARY_CUT_H_