/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/LinearAlgebra.h>

namespace vpl {

/*!
*/
class PointTransform
{
public:
	Matrix A;
	Matrix T;
	PointArray P;

	PointTransform()
	{
	}

	PointTransform(const PointTransform& rhs)
		: A(rhs.A), T(rhs.T), P(rhs.P)
	{
		
	}

	void operator=(const PointTransform& rhs)
	{
		A = rhs.A;
		T = rhs.T;
		P = rhs.P;
	}

	void Clear()
	{
		A.clear();
		T.clear();
		P.clear();
	}
};

} // namespace vpl
