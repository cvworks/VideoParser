/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <windows.h>
#include "Mutex.h"
#include "BasicUtils.h"

using namespace vpl;

Mutex::Mutex(const char* szName)
{
	m_hMutex = CreateMutexA(NULL, FALSE, szName);
	ASSERT(m_hMutex != NULL);
}

void Mutex::Close()
{
	ASSERT(m_hMutex != NULL);

	CloseHandle(m_hMutex);

	m_hMutex = NULL;
}

/*!
	Returns true iff the lock is granted
*/
bool Mutex::Lock()
{
	DWORD returnCode = WaitForSingleObject(m_hMutex, INFINITE);

	return (returnCode == WAIT_OBJECT_0);
}

/*!
	Returns true if the lock is granted and false if the
	wait to get the lock timed out.
*/
bool Mutex::Lock(unsigned long timeoutMilliseconds)
{
	DWORD returnCode = WaitForSingleObject(m_hMutex, timeoutMilliseconds);

	return (returnCode == WAIT_OBJECT_0);
}

void Mutex::Release()
{
	BOOL success = ReleaseMutex(m_hMutex);

	ASSERT(success);
}
