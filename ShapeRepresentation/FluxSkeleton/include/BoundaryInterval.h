/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef __BOUNDARY_INTERVAL_H__
#define __BOUNDARY_INTERVAL_H__

#include <Tools/BasicUtils.h>

namespace sg {

/*!
	@brief Defines an interval of the boundary curve associated
	with a DDSGraph.

	The successor direction is always in increasing order. So, for example, 
	for a boundary of length 200, the interval [10, 100] defines the points 
	{10, 11, ..., 99, 100}, while the interval [100, 10] defines the point
	{100, 101, ..., 199, 0, 1, ..., 10}.

	Then, the interval is defined such that the "first" point can reach the "last"
	point by ONLY visiting interval points via multiple calls to the successor
	operation. Similarly, the predecessor operation connects the first and last
	point without visiting ANY interval point.

	@note Use IsGreaterThan() and IsSmallerThan() to compare indices. 
	
	To iterate through the entire interval, there are some options:

	(a)
		for (int i = bndryInt.First(); ; i = bndryInt.Succ(i))
		{
			// Safest option, but its a bit ugly

			if (i == bndryInt.Last())
				break;
		}
	(b)
		for (int i = bndryInt.First(); bndryInt.Includes(i); i = bndryInt.Succ(i))
		{
		    // Nicer option, but may lead to an infinite loop
			// Warning: Infinite loop if Succ(Last()) == First()
		}
*/
class BoundaryInterval
{
	int length, start, end;

public:
	BoundaryInterval() { }
	BoundaryInterval(int len, int s, int e) { length = len; start = s; end = e; }
	void Set(int len, int s, int e)         { length = len; start = s; end = e; }

	void SetFirst(int s)            { start = s; }
	void SetLast(int e)             { end = e; }
	void SetLimits(int s, int e)    { start = s; end = e; }
	void SetBoundarySize(int len)   { length = len; }

	int BoundarySize() const { return length; }

	//! Return true iff 'i' is within the boundary interval (including endpoints)
	bool Includes(int i) const 
	{
		ASSERT(i >= 0);

		if (end < start)
			return (i >= start && i < length) || (i >= 0 && i <= end);
		else
			return (i >= start && i <= end);
	}

	//! Return true iff 'i' is within the boundary interval (excluding endpoints)
	bool Inside(int i) const
	{
		return (i == start || i == end) ? false : Includes(i);
	}

	void Swap() { int aux = start; start = end; end = aux; }
	int First() const { return start; }
	int Last() const  { return end; }

	/*!
		Checks that the indices idx0 and idx1 are not INSIDE the interval. If 
		they are, the interval is swapped.

		@return true iff the given indices are not inside the interval.
	*/
	bool MakeExclude(int idx0, int idx1)
	{
		if (Inside(idx0) || Inside(idx1))
		{
			Swap();

			// If swap didn't fix things, the interval is invalid
			if (Inside(idx0) || Inside(idx1))
				return false;
		}

		return true;
	}

	int Size() const
	{
		return (end < start) ? (length - start + end + 1) : (end - start + 1);
	}

	int Pred(int i) const 
	{ 
		ASSERT(i >= 0 && i < length);

		return (i == 0) ? length - 1 : i - 1;
	}

	int Succ(int i) const 
	{ 
		ASSERT(i >= 0 && i < length);

		return (i == length - 1) ? 0 : i + 1; 
	}

	//! Computes (a > b) for indices a and b using the interval's information
	bool IsGreaterThan(int a, int b) const 
	{
		return ((a >= start && b >= start) || (a < start && b < start)) ? (a > b) : (a < b);
	}

	//! Computes (a < b) for indices a and b using the interval's information
	bool IsSmallerThan(int a, int b) const 
	{
		return ((a >= start && b >= start) || (a < start && b < start)) ? (a < b) : (a > b);
	}

	//! Converts a circular index into an index in the interval [0, lenght)
	int TranslateCircularIndex(int i) const
	{
		if (i < 0)
			return length - (abs(i) % length);
		else
			return (i % length);
	}

	friend std::ostream& operator<<(std::ostream& os, const BoundaryInterval& bi)
	{
		return os << "0:" << bi.length-1 << "->[" << bi.start << ',' << bi.end << ']';
	}
};

} //namespace sg

#endif //__BOUNDARY_INTERVAL_H__
