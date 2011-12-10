/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <ctime>
#include <string>
#include "BasicUtils.h"

namespace vpl {

/*!
	tm structure returned or used by asctime(), gmtime(), localtime() and mktime(). 
	The tm structure definition is:

	  // structure to store date and time information 
	  typedef struct {
		 int tm_hour;   // hour (0 - 23) 
		 int tm_isdst;  // daylight saving time enabled/disabled 
		 int tm_mday;   // day of month (1 - 31) 
		 int tm_min;    // minutes (0 - 59) 
		 int tm_mon;    // month (0 - 11 : 0 = January) 
		 int tm_sec;    // seconds (0 - 59) 
		 int tm_wday;   // Day of week (0 - 6 : 0 = Sunday) 
		 int tm_yday;   // Day of year (0 - 365) 
		 int tm_year;   // Year less 1900
	  }

	@see http://www1.gantep.edu.tr/~cpp/misc-time.php

	Note 1: Daylight Saving Time (DST)
	It is the practice of temporarily advancing clocks during the summertime so that 
	afternoons have more daylight and mornings have less. Typically clocks are 
	adjusted forward one hour near the start of spring and are adjusted backward in autumn.
	That is, 15:00 in the spring/summer corresponds to 14:00 in the fall/winter.

	Note 2: gmtime vs local time
	When given a time_t value, a Time object is constrcuted using the localtime() function.
	If a UTC (or GMT timezone) is desired instead, then the time_t value must first be converted
	to a tm object. There are several ways of construction a Time object using local 
	or universal times. For example,
	
	time_t a = time(NULL);

	Time b(*gmtime(&a)); // This expresses 'a' as a UTC time.

	Time d(*localtime(&t)); // This expresses 'a' as a local time.

	Time c(a); // This expresses 'a' as a local time.

	Time d(true, a); // This expresses 'a' as a local time. 

	Time e(false, a); // This expresses 'a' as a UTC time.

	// Note that since 'a' is the current time, the objects d and e 
	// could have been created as

	Time d(true); // This expresses the current time as a local time. 

	Time e(false); // This expresses the current time as a UTC time.

	Note 3: The TM_YEAR_BASE and the year 1969
	A time_t value equal to 0 corresponds to the time 1969-12-31. This is
	not directly related to the TM_YEAR_BASE, wich is 1900.
*/
struct Time : public tm
{
	enum {CURRENT_LOCAL_TIME, CURRENT_UNIVERSAL_TIME};
	enum WEEKDAY {SUNDAY, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY};

	enum STR_TIME_FORMAT {ALPHA_MONTH, SQL_DATETIME};

	Time()
	{
		clear();
	}

	/*!
		Init the Time to 't' or current time by default. It expresses
		the time as either local time or universal time.
	*/
	Time(bool initToLocalTime, time_t t = time(NULL)) 
		: tm(initToLocalTime ? *localtime(&t) : *gmtime(&t))
	{
	}

	Time(const tm& t) : tm(t)
	{
	}

	//! Uses localtime() to convert the time_t value
	Time(const time_t& t) : tm(*localtime(&t))
	{
	}

	Time(std::string str, STR_TIME_FORMAT format = ALPHA_MONTH);

	void clear()
	{
		tm_hour = 0; 
		tm_isdst = 0;
		tm_mday = 0;
		tm_min = 0;
		tm_mon = 0;
		tm_sec = 0; 
		tm_wday = 0;
		tm_yday = 0;
		tm_year = 0;
	}

	int hour() const 
	{ 
		ASSERT(tm_hour >= 0 && tm_hour <= 23);
		return tm_hour; 
	}

	int day() const 
	{ 
		ASSERT(tm_mday >= 1 && tm_mday <= 31); 
		return tm_mday; 
	}

	int minutes() const  
	{
		ASSERT(tm_min >= 0 && tm_min <= 59); 
		return tm_min; 
	}

	int month() const  
	{ 
		ASSERT(tm_mon >= 0 && tm_mon <= 11); 
		return tm_mon; 
	}

	int seconds() const  
	{ 
		ASSERT(tm_sec >= 0 && tm_sec <= 59); 
		return tm_sec; 
	}

	WEEKDAY weekday() const 
	{
		ASSERT(tm_wday >= 0 && tm_wday <= 6); 
		return (WEEKDAY) tm_wday; 
	}

	int yearday() const
	{
		ASSERT(tm_yday >= 0 && tm_yday <= 365); 
		return tm_yday;
	}

	bool empty() const
	{
		return tm_year == 0;
	}
	
	//! Current year. eg, 2011
	void set_year(int n);

	//! Month from 1 to 12
	void set_month(int n)
	{
		ASSERT(n >= 1 && n <= 12); 
		tm_mon = n - 1;
	}

	//! day of month (1 - 31)
	void set_day(int n)
	{
		ASSERT(n >= 1 && n <= 31); 
		tm_mday = n;
	}

	//! hour (0 - 23) 
	void set_hour(int n)
	{
		ASSERT(n >= 0 && n <= 23); 
		tm_hour = n;
	}

	//! minutes (0 - 59) 
	void set_minutes(int n)
	{
		ASSERT(n >= 0 && n <= 59); 
		tm_min = n;
	}

	/*! 
		Adjusts the values if the ones provided are not in the possible range 
		or they are incomplete or mistaken.
		
	    @param useCurrentDSTValue if true, the field tm_isdst is set
		using according to time() prior to adjustiong other fields.
	*/
	time_t adjust(bool useCurrentDSTValue)
	{
		if (useCurrentDSTValue)
		{
			Time t(true);

			tm_isdst = t.tm_isdst;
		}

		return mktime(this);
	}

	//! True if hour() is greater or equal to start and smaller than end.
	bool hour_between(int start, int end) const
	{
		return (hour() >= start && hour() < end);
	}

	std::string str() const;
	std::string pretty_str() const;

	time_t toSeconds() const
	{
		if (empty())
			return -1;

		Time t = *this;

		return mktime(&t);
	}

	bool operator==(const Time& rhs) const
	{
		return toSeconds() == rhs.toSeconds();
	}

	bool operator!=(const Time& rhs) const
	{
		return toSeconds() != rhs.toSeconds();
	}

	bool operator>(const Time& rhs) const
	{
		return toSeconds() > rhs.toSeconds();
	}

	bool operator<(const Time& rhs) const
	{
		return toSeconds() < rhs.toSeconds();
	}

	bool operator>=(const Time& rhs) const
	{
		return toSeconds() >= rhs.toSeconds();
	}

	bool operator<=(const Time& rhs) const
	{
		return toSeconds() <= rhs.toSeconds();
	}
};

} // namespace vpl


