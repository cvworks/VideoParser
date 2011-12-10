/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Video.h"
#include <memory>

namespace vpl {

typedef std::shared_ptr<Video> VideoPtr;

enum VideoType {CV_VIDEO, VXL_VIDEO, IMGSEQ_VIDEO, SYNTHETIC_VIDEO, MULTI_VIDEO};

inline const char* VideoFileExtensions()
{
	return "Video Files (*.{avi,mpg,mpeg,mp4,wmv,svf,isf,mvf})";
}

inline VideoType GetVideoTypeFromFileName(std::string filename)
{
	int sz = filename.size();

	std::string ext = filename.substr(filename.rfind(".") + 1, sz);

	if (ext == "mvf") // multi video file
		return MULTI_VIDEO;
	else if (ext == "svf") // synthetic video file
		return SYNTHETIC_VIDEO;
	else if (ext == "isf") // image sequence file
		return IMGSEQ_VIDEO;
#ifdef HAS_VXL_VIDEO
	else if (ext == "avi2")
		return VXL_VIDEO;
#endif // HAS_VXL_VIDEO
	else
		return CV_VIDEO;
}

VideoPtr ConstructVideoObject(VideoType type);

inline VideoPtr ConstructVideoObject(std::string filename)
{
	return ConstructVideoObject(GetVideoTypeFromFileName(filename));
}

} // namespace vpl

