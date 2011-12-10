/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VPL_SEMAPHORE_H_
#define _VPL_SEMAPHORE_H_

namespace vpl {

class Semaphore
{
	typedef void* HANDLE;

	HANDLE m_hSemaphore;

public:
	Semaphore(int initialCount, int maximumCount);

	bool WaitOne();

	void Release();
};

} // namespace vpl

#endif // _VPL_SEMAPHORE_H_