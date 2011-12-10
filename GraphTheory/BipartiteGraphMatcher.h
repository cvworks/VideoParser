/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <list>
#include <vector>
#include <Tools/LinearAlgebra.h>
#include <Tools/STLMatrix.h>
#include <Tools/BasicUtils.h>

namespace vpl {

/*!
*/
class BipartiteGraphMatcher
{
public:
	//! Necessary virtual destructor
	virtual ~BipartiteGraphMatcher() { }

	virtual void Init(unsigned numNodes1, unsigned numNodes2) = 0;

	virtual double SolveMinCost(const Matrix& costMat) = 0;

	//! Returns the correspondences found by the SolveMinCost() function.
	virtual void GetCorrespondences(UIntVector& rowMap, UIntVector& colMap) = 0;
};

} // namespace vpl
