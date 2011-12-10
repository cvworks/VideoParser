/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VPL_MUTEX_H_
#define _VPL_MUTEX_H_

#include "BasicUtils.h"

namespace vpl {

class Mutex
{
public:
	typedef void* HANDLE;

protected:
	HANDLE m_hMutex;

public:
	Mutex(const char* szName = NULL);

	Mutex(HANDLE hMutex)
	{
		m_hMutex = hMutex;
	}

	HANDLE Handle() const
	{
		return m_hMutex;
	}

	void Close();

	bool Lock();

	bool Lock(unsigned long timeoutMilliseconds);

	void Release();
};

} // namespace vpl

#endif // _VPL_MUTEX_H_