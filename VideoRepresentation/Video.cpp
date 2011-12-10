/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "Video.h"

using namespace vpl;

double Video::s_maxWidth = 0;
double Video::s_maxHeight = 0;

/*!
	Sets the pixels of the i'th ROI to 255. The other pixes are
	left untouched. The mask must be large enought to fully contain
	the requested ROI.
*/
void ROISequence::FillMask(unsigned i, ByteImg& mask) const
{
	ASSERT(i < size());

	auto it = begin();

	std::advance(it, i);

	// Copy the ROI and regularize it
	UIBoundingBox roi = *it;

	roi.Regularize();

	ASSERT(roi.xmax < mask.ni() && roi.ymax < mask.nj());

	for (unsigned i = roi.xmin; i <= roi.xmax; i++)
		for (unsigned j = roi.ymin; j <= roi.ymax; j++)
			mask(i, j) = 255;
}


