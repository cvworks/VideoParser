/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <VideoRepresentation/GenericVideo.h>
#include <Tools/SimpleDatabase.h>
#include <Tools/SQLDatabase.h>
#include <Tools/VisSysComponent.h>
#include <Tools/VSCDataSerializer.h>
#include "VideoParseMetadata.h"
#include "VideoDataBinder.h"

namespace vpl {
	
class VSCGraph;

class VideoProcessor
{
public:
	/*!
	*/
	struct Params
	{
		std::string strFilename;
		std::string strOutputDir;
		std::string strCacheFilename;
		std::string taskName;
		
		int firstFrame;
		int lastFrame;
		int frameStep;
		bool cacheParseData;

		std::string sqlDBName;
		bool sqlNonBlockingMode;

		void operator=(const Params& rhs)
		{
			strFilename = rhs.strFilename;
			strOutputDir = rhs.strOutputDir;
			strCacheFilename = rhs.strCacheFilename;
			
			taskName = rhs.taskName;
			
			firstFrame = rhs.firstFrame;
			lastFrame  = rhs.lastFrame;
			frameStep  = rhs.frameStep;

			sqlDBName = rhs.sqlDBName;
			sqlNonBlockingMode = sqlNonBlockingMode;

			//saveResults    = rhs.saveResults;
			cacheParseData = rhs.cacheParseData;
		}

		void ReadFromUserArguments();
	};

private:
	Params m_params;
	
	GenericVideo m_video;
	VSCGraph* m_pComponents;

	RUNNING_MODE m_runningMode;

	InputImageInfo m_imgInfo;

	bool m_hasFrameRequest;
	fnum_t m_requestedFrame;

	VideoDataBinder m_videoDataBinder;
	SQLDatabase* m_pSQLDatabase;

protected:
	void ResetCurrentVideoData();
	void OpenSQLDatabase();
	void GetInputImage(const GenericVideo& vid, InputImageInfo& imgInfo) const;
	void SetRunningModeFromTaskName(const std::string& taskName);

public:
	VideoProcessor();
	~VideoProcessor();
	void Initialize();
	void ReleaseResources();

	void SetTargetTaskAndMode(const std::string& taskName);

	time_t CurrentFrameTime() const
	{
		return m_video.CurrentFrameTime();
	}

	bool OfflineMode() const
	{
		return m_runningMode == OFFLINE_RUNNING_MODE;
	}

	static StrArray ValidTaskLabels();

	std::string CurrentTask() const
	{
		return m_params.taskName;
	}

	bool InitializeVideoProcessing();
	void ProcessFrame();
	void PlaybackFrame();

	bool LoadPreviewFrame();

	bool ProcessVideo();
	void ProcessDataOffline();

	void PostProcessVideo();

	void SaveResults(bool status);

	fnum_t FrameNumber() const  { return m_video.FrameNumber(); }
	double FrameRate() const { return m_video.FrameRate(); }

	FloatImg CurrentFrame()  { return m_imgInfo.greyFrame; }
	RGBImg CurrentRGBFrame() { return m_imgInfo.rgbFrame; }

	const Params& GetParams() const      { return m_params; }
	void SetParams(const Params& params) { m_params = params; }

	std::string GetVideoFilename() const 
	{ 
		return m_params.strFilename; 
	}

	void SetVideoFilename(const std::string& fn)
	{ 
		m_params.strFilename = fn; 
	}

	void GetVisionComponentLabels(std::vector<std::string>& lbls);

	void GetParameterInfo(int idx, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const;

	std::list<UserCommandInfo> GetUserCommands(int idx) const;

	void GetDisplayInfo(int idx, DisplayInfoIn* dii, DisplayInfoOut* dio) const;

	const OutputImageInfo& GetOutputImageInfo(int idx) const;

	//! One line of text with basic info about current frame
	std::string GetStatusText() const
	{
		static char szStatusLine[100];

		Time t = m_video.CurrentFrameTime();

		// Make a status line of text to provide information about the current frame

		sprintf(szStatusLine, "%s (%lld/%lld)", t.str().c_str(), 
			m_video.FrameNumber(), m_video.FrameCount());

		return std::string(szStatusLine);
	}

	bool IsFirstFrame() const
	{
		return (FrameNumber() == GetParams().firstFrame); 
	}

public:
	bool HasFrameRequest() const
	{
		return m_hasFrameRequest;
	}

	void RequestFrame(fnum_t n)
	{
		m_hasFrameRequest = true;
		m_requestedFrame = n;
	}

	void ReadNextFrame()
	{
		if ((!m_hasFrameRequest && m_params.frameStep == 1) ||
			(m_hasFrameRequest && m_requestedFrame == FrameNumber() + 1))
		{
			m_hasFrameRequest = false;

			m_video.ReadNextFrame();
		}
		else if (m_hasFrameRequest)
		{
			m_hasFrameRequest = false;

			m_video.ReadFrame(m_requestedFrame);
		}
		else
		{
			m_video.ReadFrame(FrameNumber() + m_params.frameStep);
		}
	}

	void ReadFirstFrame()
	{
		if (m_params.firstFrame == 0)
			m_video.ReadFirstFrame();
		else
			m_video.ReadFrame(m_params.firstFrame);
	}

	bool IsLastFrame() const
	{
		return (m_video.IsLastFrame() || 
			(m_params.lastFrame >= 0 && FrameNumber() > m_params.lastFrame));
	}

	void OnGUIEvent(int id, int value);
};

} // namespace vpl
