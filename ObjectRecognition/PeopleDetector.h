/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;

/*!
	@brief Wrapper for a generic people detector algorithm
*/
class PeopleDetector : public VisSysComponent
{
	/*struct BasicParams
	{
		
	};

	BasicParams m_basicParams;*/

protected:
	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;

public:	

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual std::string GenericName() const
	{
		return "PeopleDetector";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ImageProcessor");

		return deps;
	}
};

} // namespace vpl

