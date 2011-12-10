/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VIDEO_WRITER_H_
#define _VIDEO_WRITER_H_

#include <Tools/ImageUtils.h>

struct CvVideoWriter;

namespace vpl {

class VideoWriter
{
	CvVideoWriter* m_writer;

public:
	VideoWriter()
	{
		m_writer = NULL;
	}

	VideoWriter(const std::string& filename, int width, int height,
		double fps, int fourcc)
	{
		Open(filename, width, height, fps, fourcc);
	}

	~VideoWriter()
	{
		Close();
	}

	void Open(const std::string& filename, int width, int height,
		double fps = -1, int fourcc = -1);

	void Close();

	bool IsOpen() const
	{
		return m_writer != NULL;
	}

	int WriteFrame(RGBImg img);
};

} //namespace vpl

#endif // _VIDEO_WRITER_H_
