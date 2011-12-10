/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <windows.h>
#include "Semaphore.h"
#include "BasicUtils.h"

using namespace vpl;

Semaphore::Semaphore(int initialCount, int maximumCount)
{
	m_hSemaphore = CreateSemaphore(NULL, initialCount, maximumCount, NULL);
}

bool Semaphore::WaitOne()
{
	DWORD returnCode = WaitForSingleObject(m_hSemaphore, INFINITE);

	return (returnCode == WAIT_OBJECT_0);
}

void Semaphore::Release()
{
	BOOL success = ReleaseSemaphore(m_hSemaphore, 1, NULL);

	ASSERT(success);
}
