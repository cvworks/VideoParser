/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "Timer.h"

#include <Tools/cv.h>
//#include <vul/vul_timer.h>

using namespace vpl;

bool Timer::s_enableTimers = false;

Timer::Timer()
{
	tick_freq = cvGetTickFrequency();
	start_time = cvGetTickCount();
}

void Timer::Reset()
{
	start_time = cvGetTickCount();
}

/*!
	Returns enlapsed time in milliseconds
*/
double Timer::ElapsedTime()
{
	// Quotient gives microseconds, but we want milliseconds
	return ((cvGetTickCount() - start_time) / tick_freq) / 1000.0;
}
