/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "VideoTypes.h"
#include <Tools/UserArguments.h>

namespace vpl {

class MultiVideo : public Video
{
public:
	struct VideoInfo
	{
		std::string filename;
		time_t timestamp;
		fnum_t frameCount;

		VideoInfo()
		{
			// Init to neg number to indicate that we haven't
			// event tried to read this metadata
			frameCount = -1; 
		}
		
		void operator=(const VideoInfo& rhs)
		{
			filename = rhs.filename;
			timestamp = rhs.timestamp;
			frameCount = rhs.frameCount;
		}

		void ReadVideoMetadata()
		{
			VideoPtr pVid = ConstructVideoObject(filename);

			if (pVid && pVid->Load(filename))
			{
				frameCount = pVid->FrameCount();
			}
			else
			{
				// Set to 0 to indicate that we did try to read this 
				// metadata but couldn't.
				frameCount = 0;
			}
		}
	};

protected:
	typedef std::list<VideoInfo> VidList;
	typedef VidList::iterator iterator;

	VidList m_vids; //!< Ordered sequence of videos
	iterator m_vidIt;

	VideoPtr m_pVideo;

	fnum_t m_frameOffset;

	time_t m_startTime; //!< Start of the video in seconds from Jan 1st, 1970
	time_t m_endTime;   //!< End of the video in seconds from Jan 1st, 1970

	UserArguments m_params;

protected:
	StrList CollectVideoFiles(std::string rootDir);
	StrList ReadVideoFilenames();
	time_t GetTimestamp(unsigned id, const std::string& filename, std::string timeOpt);
	bool FilterVideos(std::string strStart, std::string strEnd);
	bool LoadVideo();

	void ReadMetadata()
	{
		m_frameCount = 0;

		std_forall(it, m_vids)
		{
			it->ReadVideoMetadata();

			m_frameCount += it->frameCount;
		}
	}

	/*! 
		Updates the frame count unless it has already been set,
		eg, using ReadMetadata(), and so it's greather than offset.
	*/
	void UpdateFrameCount()
	{
		if (m_frameCount <= m_frameOffset)
			m_frameCount = m_frameOffset + m_pVideo->FrameCount();
	}

public:
	MultiVideo()
	{
		m_vidIt = m_vids.end();

		m_frameOffset = 0;

		m_startTime = -1;
		m_endTime = -1;
	}

	void Clear()
	{
		Video::Clear();

		m_vids.clear();
		m_vidIt = m_vids.end();

		m_frameOffset = 0;

		m_startTime = -1;
		m_endTime = -1;
	}

	//!	Returns the file path used to load the video
	std::string Filename() const
	{
		return (m_pVideo) ? m_pVideo->Filename() : std::string();
	}

	bool Load(std::string filename);

	void SetScalingFactor(const double& s) 
	{ 
		ASSERT(m_vids.size() == 1);
		m_pVideo->SetScalingFactor(s); 
	}

	const double& GetScalingFactor() const 
	{
		ASSERT(m_vids.size() == 1);
		return m_pVideo->GetScalingFactor(); 
	}

	time_t CurrentFrameTime() const
	{
		return m_pVideo->CurrentFrameTime();
	}

	fnum_t FrameNumber() const    { return m_frameOffset + m_pVideo->FrameNumber(); }
	
	fnum_t FrameCount() const     { return m_frameCount; }

	double FramePosition() const  { return m_pVideo->FramePosition(); }
	
	double FrameRate() const      { return m_pVideo->FrameRate(); }
	int CodecCode() const         { return m_pVideo->CodecCode(); }

	bool IsLastFrame() const
	{ 
		if (m_vids.empty())
			return true;

		iterator it = m_vidIt;

		if (++it == m_vids.end())
		{
			// First check if it is the video's last frame
			// since the m_endTime might be passed to the true last frame
			if (m_pVideo->IsLastFrame())
				return true;
			else if (m_endTime >= 0)
				return (m_pVideo->FrameNumber() > m_pVideo->TimeToFrame(m_endTime));
			else
				return false;
		}
		else
		{
			return false;
		}

		//return (++it == m_vids.end()) ? m_pVideo->IsLastFrame() : false; 
	}

	void ReadFirstFrame()                 
	{
		ASSERT(!m_vids.empty());
		
		if (m_vidIt != m_vids.begin())
		{
			m_vidIt = m_vids.begin();

			LoadVideo();
		}
		
		m_frameOffset = 0;

		if (m_startTime < 0)
			m_pVideo->ReadFirstFrame(); // It's faster than ReadFrame(0)
		else
			m_pVideo->ReadFrame(TimeToFrame(m_startTime));

		UpdateFrameCount();
	}

	void ReadNextFrame()
	{ 
		// It should not be the last frame of the multi-video
		ASSERT(!IsLastFrame());

		// See if it is the last frame of the current video
		if (m_pVideo->IsLastFrame())
		{
			m_frameOffset += m_pVideo->FrameCount();

			++m_vidIt;

			LoadVideo();

			m_pVideo->ReadFirstFrame();

			UpdateFrameCount();
		}
		else
		{
			m_pVideo->ReadNextFrame(); 

			// Note, the "last" frame of a video is not a valid frame,
			// so if we are there, we call this function recursively
			if (m_pVideo->IsLastFrame() && !IsLastFrame())
				ReadNextFrame();
		}
	}

	void ReadFrame(fnum_t i);

	void PrintFrameInfo() const           { m_pVideo->PrintFrameInfo(); }
	
	RGBImg GetCurrentRGBFrame() const     { return m_pVideo->GetCurrentRGBFrame(); }
	FloatImg GetCurrentGreyScaleFrame() const { return m_pVideo->GetCurrentGreyScaleFrame(); }

	//!	Returns a string with basic information about the current frame
	const std::string& FrameInfo() const
	{
		return m_pVideo->FrameInfo();
	}
};

} // namespace vpl

