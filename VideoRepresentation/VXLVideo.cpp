/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VXLVideo.h"

#ifdef HAS_VXL_VIDEO

#include <fstream>
#include <vil/vil_load.h>
#include <vil/vil_flip.h>
#include <vidl/vidl_dshow_file_istream.h>
#include <Tools/Num2StrConverter.h>
#include <Tools/Exceptions.h>

using namespace vpl;

bool VXLVideo::Load(std::string strFilename)
{
	Clear();

	//m_movie = vidl_io::load_movie(strFilename);
	m_pVideoStream = new vidl_dshow_file_istream(strFilename);

	if (!m_pVideoStream->is_open())
		return false;

	//if (m_movie.ptr() == 0)
	//	return false;

	// Set the required video information in the base class
	//SetVideoInfo(strFilename, m_movie->length());
	SetVideoInfo(strFilename, m_pVideoStream->num_frames()); 

	return true;
}

bool VXLVideo::IsLastFrame() const
{
	//return m_frmIt == m_movie->last();
	return m_pVideoStream->frame_number() == m_pVideoStream->num_frames();
}

void VXLVideo::ReadFirstFrame()
{
	/*m_frmIt = m_movie->first();
	m_nFrame = 0;
	m_curImg = m_frmIt->get_view();*/

	m_nFrame = 0;
	m_pVideoStream->read_frame();
	//vidl_frame_sptr
}

void VXLVideo::ReadNextFrame()
{
	/*ASSERT(m_nFrame >= 0);
	++m_frmIt;
	++m_nFrame;
	
	if (!IsLastFrame())
		m_curImg = m_frmIt->get_view();*/

	++m_nFrame;
	m_pVideoStream->read_frame();
}

RGBImg VXLVideo::GetCurrentRGBFrame() const
{
	RGBImg curRGBFrame;

	// Assume we have an RGB image and make it contiguous (see OldCode.txt for alternative)
	curRGBFrame = vil_convert_to_component_order(m_curImg);

	// The images come reversed, so we must flip them (or draw them differently)
	curRGBFrame = vil_copy_deep((m_bFlipFrames) ? vil_flip_ud(curRGBFrame) : curRGBFrame);

	return curRGBFrame;
}

FloatImg VXLVideo::GetCurrentGreyScaleFrame() const
{
	FloatImg curFrame;

	curFrame = ConvertToGreyImage(m_curImg);

	curFrame = vil_copy_deep((m_bFlipFrames) ? vil_flip_ud(curFrame) : curFrame);

	return curFrame;
}

#endif // HAS_VXL_VIDEO
