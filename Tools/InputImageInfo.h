/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/Time.h>
#include <Tools/ImageUtils.h>
#include <Tools/BasicTypes.h>

namespace vpl {

typedef long long int fnum_t; //!< Frame number type

/*!
	Array of Regions of Interest.
*/
struct ROISequence : public std::list<UIBoundingBox>
{
	typedef UIBoundingBox Region;

	void FillMask(unsigned i, ByteImg& mask) const;
};

/*!
	Basic information about an input video frame or static image 
*/
struct InputImageInfo
{
	fnum_t frameNumber;    //!< Frame number
	double framePos;       //!< Frame position in time (seconds)
	std::string frameInfo; //!< Basic info associated with the frame (eg, file path)

	RGBImg rgbFrame;       //!< RGB representation of the current frame
	FloatImg greyFrame;    //!< Grey scale representation of the current frame

	time_t timestamp;      //!< Timestamp of the frame in the video

	ROISequence roiSequence;     //!< Array of regions of interest

	fnum_t numFramesProcessed;  //!< Number of frames processed in the video

	void operator=(const InputImageInfo& rhs)
	{
		frameNumber = rhs.frameNumber;
		framePos    = rhs.framePos;
		frameInfo   = rhs.frameInfo;
		rgbFrame    = rhs.rgbFrame;
		greyFrame   = rhs.greyFrame;
		timestamp   = rhs.timestamp;
		roiSequence = rhs.roiSequence;

		numFramesProcessed = rhs.numFramesProcessed;
	}

	void Clear()
	{
		frameNumber = 0;
		framePos = 0;

		frameInfo.clear();

		rgbFrame = RGBImg();
		greyFrame = FloatImg();

		timestamp = 0;

		roiSequence.clear();

		numFramesProcessed = 0;
	}
};

} // vpl namespcace

