/*
 *   Copyright (c) 2007 John Weaver
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#pragma once

#include "BipartiteGraphMatcher.h"
#include <Tools/LinearAlgebra.h>

#include <list>
#include <utility>

namespace vpl {

/*!
*/
class HungarianBGMatcher : public BipartiteGraphMatcher
{
private:
	IntMatrix mask_matrix;
	DoubleMatrix matrix;
	bool* row_mask;
	bool* col_mask;
	unsigned saverow, savecol;

protected:
	inline bool find_uncovered_in_matrix(const double&, unsigned&, unsigned&);
	inline bool pair_in_list(const std::pair<int,int> &, const std::list<std::pair<int,int> > &);
	int step1(void);
	int step2(void);
	int step3(void);
	int step4(void);
	int step5(void);
	int step6(void);

public:
	virtual void Init(unsigned numNodes1, unsigned numNodes2)
	{
		// there is no init step needed
	}

	virtual double SolveMinCost(const DoubleMatrix& m);

	/*! 
		Returns the correspondences found by the SolveMinCost() function.
		
		Stores results in the row and col maps s.t.
		row2colMap(r) = c means that col c is assigned to row r
		col2rowMap(c) = r means that row r is assigned to column c
	*/
	virtual void GetCorrespondences(UIntVector& row2colMap, UIntVector& col2rowMap);

	//! Check that there are no invalid numbers in the mapping vector
	bool CheckMapping(const UIntVector& row2colMap, const UIntVector& col2rowMap) const
	{	
		unsigned i;

		for (i = 0; i < row2colMap.size(); i++)
			if (row2colMap[i] >= matrix.cols() || col2rowMap[row2colMap[i]] != i)
				return false;

		for (i = 0; i < col2rowMap.size(); i++)
			if (col2rowMap[i] >= matrix.rows() || row2colMap[col2rowMap[i]] != i)
				return false;
		
		return true;
	}
};

} // namespace vpl
