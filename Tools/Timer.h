/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

namespace vpl {
	
class Timer
{
	double tick_freq; 
	__int64 start_time; 

public:
	static bool s_enableTimers;

	Timer();

	void Reset();
	
	double ElapsedTime();
};

} // namespace vpl

