/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VXL_VIDEO_H_
#define _VXL_VIDEO_H_

//#define HAS_VXL_VIDEO 1

#include "Video.h"

#ifdef HAS_VXL_VIDEO

//#include <vidl/vidl_io.h>
//#include <vidl/vidl_frame.h>
//#include <vidl/vidl_movie.h>

class vidl_dshow_file_istream;

namespace vpl {

class VXLVideo : public Video
{
	//vidl_movie_sptr m_movie;
	//vidl_movie::frame_iterator m_frmIt;

	vidl_dshow_file_istream* m_pVideoStream;

	BaseImgPtr m_curImg;
	
public:
	//VXLVideo() : m_frmIt(m_movie.ptr()) { }
	
	bool Load(std::string strFilename);
	void ReadFirstFrame();
	void ReadNextFrame();

	void ReadFrame(fnum_t i)
	{
		ShowError("Cannot seek frames");
	}
	
	bool IsLastFrame() const;
	RGBImg GetCurrentRGBFrame() const;
	FloatImg GetCurrentGreyScaleFrame() const;
};

} // namespace vpl

#endif // HAS_VXL_VIDEO

#endif //_VXL_VIDEO_H_