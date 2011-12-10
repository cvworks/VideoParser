/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

namespace vpl {

/*!
	A thread-safe consumer producer queue that uses no synchronization.

	It is safe as long as there is exactly one consumer and one produced thread.
*/
template<class T, int SIZE> class SafeQueue
{
private:
	volatile int m_read;
	volatile int m_write;
	T m_data[SIZE];

public:
	SafeQueue()
	{
		m_read = 0;
		m_write = 0;
	}

	bool empty() const
	{
		return (m_read == m_write);
	}

    /*! 
		Push a new element in the circular queue.

		@return true if the element is inserted and false if there
		is no free space in the queue (the consumer thread must wait)
	*/
	bool Push(const T& e)
	{
		int nextElement = (m_write + 1) % SIZE;

		if (nextElement != m_read) 
		{
			m_data[m_write] = e;
			m_write = nextElement;
			return true;
		}
		else
			return false;
	}

    /*! 
		Remove the next element from the circualr queue.

		@return true of the element was removed from the queue and
		false if the queue is empty.
	*/
	bool Pop(T& e)
	{
		if (m_read == m_write)
			return false;

		int nextElement = (m_read + 1) % SIZE;

		e = m_data[m_read];

		m_read = nextElement;

		return true;
	}
};

} //namespace vpl
