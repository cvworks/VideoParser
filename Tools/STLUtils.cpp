/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "STLUtils.h"
#include <sstream>

using namespace vpl;

void append_num(std::string& str, int n)
{
	std::ostringstream oss;
	oss << str << n;

	str = oss.str();
}

void append_num(std::string& str, unsigned n)
{
	std::ostringstream oss;
	oss << str << n;

	str = oss.str();
}

