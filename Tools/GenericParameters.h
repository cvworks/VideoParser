/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <memory>

namespace vpl {

/*!
	Abstract class of user parameters
*/
struct GenericParameters
{
	//! The necessary virtual destructor
	virtual ~GenericParameters() { }

	//! Reads user parameters from user arguments in a file or command line
	virtual void ReadFromUserArguments() = 0;
};

typedef std::shared_ptr<GenericParameters> GenericParametersPtr;

} // namespace vpl
