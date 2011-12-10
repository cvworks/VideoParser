/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "NaiveDescriptor.h"
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/NamedColor.h>

using namespace vpl;

void NaiveDescriptor::Draw(const RGBColor& color) const
{
	SetDrawingColor(NamedColor("Red"));

	std_forall(it, m_pts)
	{
		DrawDisk(*it, 1);
	}

	SetDefaultDrawingColor();
}
