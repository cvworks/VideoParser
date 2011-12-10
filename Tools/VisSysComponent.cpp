/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VisSysComponent.h"
#include <Tools/UserArguments.h>

using namespace vpl;

extern UserArguments g_userArgs;

void VisSysComponent::ReadParamsFromUserArguments()
{
	g_userArgs.ReadArg(Name(), "verbose", 
		"Print extra processing info", false, &m_verbose);

	g_userArgs.ReadArgs(Name(), "saveImages", 
			"Output images specified as a list of (index,param) values", 
			std::list<OutputImageParam>(), &m_outImgsParams);

	m_saveOutputImages = !m_outImgsParams.empty();
}

/*! 
	Returns the requested output image in RBG format 
	regarless of its actual type. That it, if it's not
	an RGB image, it is converted to one.
*/
RGBImg VisSysComponent::GetRGBOutputImage(const DisplayInfoIn& dii) const
{
	DisplayInfoOut dio;

	GetDisplayInfo(dii, dio);

	// If the image is a float, strecj its value to a byte first
	if (dio.imageType == FLOAT_IMAGE)
	{
		// The format of this function is: dest = conv(dest-type, src)
		dio.imagePtr = vil_convert_stretch_range(vxl_byte(), dio.imagePtr);
	}

	return vil_convert_to_component_order(
			vil_convert_to_n_planes(3, dio.imagePtr));
}

/*!
	Appends all the images that must be saved at the back of the given list
*/
void VisSysComponent::GetOutputImagesToSave(std::list<RGBImg>* pOutImgs) const
{
	std::list<OutputImageParam>::const_iterator it;
	DisplayInfoIn dii;

	for (it = m_outImgsParams.begin(); it != m_outImgsParams.end(); ++it)
	{
		dii.outputIdx = it->index;
		dii.params = it->params;

		pOutImgs->push_back(GetRGBOutputImage(dii));
	}
}
