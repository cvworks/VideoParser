/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _PRIORITY_QUEUE_DP_H_
#define _PRIORITY_QUEUE_DP_H_

#include <queue>

#define USE_NEW_THINNING

namespace sg 
{

#ifdef USE_NEW_THINNING
	inline bool compareDivPts(DivPt* p0, DivPt* p1)
	{
		return (p0->val < p1->val);
	}
#endif

#ifdef USE_NEW_THINNING
/*!
	Priority queues of DivPt.

	Note: Do not attempt to check whether a point is already
	in the queue before pushin it in it. The reason is that 
	some points need to be in the queue multiple times. Otherwise,
	they may be pop'ed and then will never get back in.
*/
class PriorityQueueDP : protected std::vector<DivPt*>
{
public:
	PriorityQueueDP(unsigned int maxSize)
	{
		reserve(maxSize);
	}

	bool empty() const
	{
		return std::vector<DivPt*>::empty();
	}

	void push_back(DivPt* p)
	{
		std::vector<DivPt*>::push_back(p);
	}

	void make()
	{
		std::make_heap(begin(), end(), compareDivPts);
	}

	DivPt* top() const
	{
		return front();
	}

	void pop()
	{
		// Move element to pop to back of the array
		std::pop_heap(begin(), end(), compareDivPts);

		// Pop element at the back of the array
		pop_back();
	}

	void push(DivPt* dp)
	{
		// Add the new element at the back of the array
		push_back(dp);

		// Move the point at the back to its appropriate
		// position in the heap
		std::push_heap(begin(), end(), compareDivPts);
	}
};
#else // USE_NEW_THINNING
/*!
	Priority queues of DivPt.

	Note: Do not attempt to check whether a point is already
	in the queue before pushin it in it. The reason is that 
	some points need to be in the queue multiple times. Otherwise,
	they may be pop'ed and then will never get back in.
*/
class PriorityQueueDP : public std::vector<DivPt>
{
public:
	PriorityQueueDP(unsigned int maxSize)
	{
		reserve(maxSize);
	}

	void make()
	{
		std::make_heap(begin(), end());
	}

	const DivPt& top() const
	{
		return front();
	}

	void pop()
	{
		// Move element to pop to back of the array
		std::pop_heap(begin(), end());

		// Pop element at the back of the array
		pop_back();
	}

	void push(const DivPt& dp)
	{
		// Add the new element at the back of the array
		push_back(dp);

		// Move the point at the back to its appropriate
		// position in the heap
		std::push_heap(begin(), end());
	}
};
#endif // USE_NEW_THINNING

} //namespace sg

#endif // _PRIORITY_QUEUE_DP_H_
