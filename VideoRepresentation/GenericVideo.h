/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "MultiVideo.h"

namespace vpl {

class GenericVideo
{
	VideoPtr m_pVideo;
	VideoType m_type;

public:
	GenericVideo() : m_pVideo(ConstructVideoObject(CV_VIDEO))
	{
		m_type = CV_VIDEO;
	}

	void Clear()
	{
		m_pVideo->Clear();
	}

	//!	Returns the file path used to load the video
	std::string Filename() const
	{
		return (m_pVideo) ? m_pVideo->Filename() : std::string();
	}

	bool Load(std::string filename)
	{
		VideoType type = GetVideoTypeFromFileName(filename);

		if (type != m_type)
		{
			m_pVideo = ConstructVideoObject(filename);
			m_type = type;
		}
			
		ASSERT(m_pVideo);

		return m_pVideo->Load(filename);
	}

	time_t StartTime() const           { return m_pVideo->StartTime(); }
	time_t CurrentFrameTime() const    { return m_pVideo->CurrentFrameTime(); }

	void SetScalingFactor(const double& s) { m_pVideo->SetScalingFactor(s); }
	const double& GetScalingFactor() const { return m_pVideo->GetScalingFactor(); }

	fnum_t FrameNumber() const               { return m_pVideo->FrameNumber(); }
	
	fnum_t FrameCount() const                { return m_pVideo->FrameCount(); }

	double FramePosition() const          { return m_pVideo->FramePosition(); }
	
	double FrameRate() const              { return m_pVideo->FrameRate(); }
	int CodecCode() const                 { return m_pVideo->CodecCode(); }
	
	void ReadFirstFrame()                 { m_pVideo->ReadFirstFrame(); }
	void ReadNextFrame()                  { m_pVideo->ReadNextFrame(); }
	void ReadFrame(fnum_t i)                 { m_pVideo->ReadFrame(i); }

	void PrintFrameInfo() const           { m_pVideo->PrintFrameInfo(); }

	const ROISequence& GetROISequence() const   { return m_pVideo->GetROISequence(); }
	
	bool IsLastFrame() const              { return m_pVideo->IsLastFrame(); }
	RGBImg GetCurrentRGBFrame() const     { return m_pVideo->GetCurrentRGBFrame(); }
	FloatImg GetCurrentGreyScaleFrame() const { return m_pVideo->GetCurrentGreyScaleFrame(); }

	//!	Returns a string with basic information about the current frame
	const std::string& FrameInfo() const
	{
		return m_pVideo->FrameInfo();
	}

	//! Gets the info associated with the current video frame
	void GetCurrentFrameInfo(InputImageInfo& imgInfo) const
	{
		m_pVideo->GetCurrentFrameInfo(imgInfo);
	}
};

} // namespace vpl

