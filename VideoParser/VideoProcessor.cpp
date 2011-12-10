/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoProcessor.h"
#include <VideoParser/VSCGraph.h>
#include <VideoParser/VisSysComponentCreator.h>
#include <Tools/UserArguments.h>
#include <Tools/UserEvents.h>

#define FIRST_VIDEO_FNAME "Fight_RunAway1_trimmed.avi" //"circle.svf"
#define RESULTS_DIR "Results/"

using namespace vpl;

extern UserArguments g_userArgs;

//! Reads parameters from user's arguments
void VideoProcessor::Params::ReadFromUserArguments()
{
	StrArray modeLabels;

	double maxFrameWidth, maxFrameHeight;

	g_userArgs.ReadArg("VideoProcessor", "maxFrameWidth", 
		"Maximum frame width (0->none)", 640.0, &maxFrameWidth);

	g_userArgs.ReadArg("VideoProcessor", "maxFrameHeight", 
		"Maximum frame height (0->none)", 480.0, &maxFrameHeight);

	// Set the static video parameters right away
	Video::SetMaxFrameSize(maxFrameWidth, maxFrameHeight);

	// See which video we should load first
	g_userArgs.ReadArg("VideoProcessor", "inputFileName", "First video loaded", 
		std::string(FIRST_VIDEO_FNAME), &strFilename);

	g_userArgs.ReadArg("VideoProcessor", "outputDir",  
		"Directory in which the results are saved", 
		std::string(RESULTS_DIR), &strOutputDir);

	StrArray validTasks = ValidTaskLabels();
	int taskId;

	g_userArgs.ReadArg("VideoProcessor", "taskName", validTasks, 
		"Name of the task to perform", 0, &taskId);

	taskName = validTasks[taskId];

	g_userArgs.ReadArg("VideoProcessor", "firstFrame",  "First video frame to read", 
		0, &firstFrame);

	g_userArgs.ReadArg("VideoProcessor", "lastFrame",  "Last video frame to read", 
		-1, &lastFrame);

	g_userArgs.ReadArg("VideoProcessor", "frameStep",  
		"Step used when iterating over frames", 1, &frameStep);

	// Make sure that the step makes sense
	if (frameStep <= 1)
		frameStep = 1;

	//g_userArgs.ReadBoolArg("VideoProcessor", "saveResults", 
	//	"Whether to save video parsing results or not", false, &saveResults);

	g_userArgs.ReadArg("VideoProcessor", "sqlDatabase", 
		"Name of the SQL database to connect to", std::string(), &sqlDBName);

	g_userArgs.ReadBoolArg("VideoProcessor", "sqlNonBlockingMode", 
		"Whether to let the sql queries by handled by a dedicated thread or not", 
		true, &sqlNonBlockingMode);

	g_userArgs.ReadBoolArg("VideoProcessor", "cacheParseData", "Whether to store/load the "
		"state of each vision component after/when parsing a frame", false, &cacheParseData);

	g_userArgs.ReadArg("VideoProcessor", "cacheFileName", 
		"Name of the file where parse data is saved", 
		std::string("video_data.db"), &strCacheFilename);

	// Ensure that the output directory ends with a slash
	if (strOutputDir.length() > 0 && 
		last_char(strOutputDir) != FILE_SEP)
	{
		strOutputDir += FILE_SEP;
	}
}

/*!
	[static]
*/
StrArray VideoProcessor::ValidTaskLabels()
{
	return VSCGraph::GetComponentCreator()->ValidTaskLabels();
}

VideoProcessor::VideoProcessor()
{
	m_pComponents = NULL;
	m_pSQLDatabase = NULL;
	m_runningMode = ONLINE_RUNNING_MODE;
}

VideoProcessor::~VideoProcessor()
{
	ReleaseResources();
}

//! Releases all resources
void VideoProcessor::ReleaseResources()
{
	delete m_pComponents;
	m_pComponents = NULL;

	delete m_pSQLDatabase;
	m_pSQLDatabase = NULL;
}

void VideoProcessor::SetRunningModeFromTaskName(const std::string& taskName)
{
	// Loof for the substring 'offline' with any upper/lower case. Note
	// that we need to keep the original case of the name. So make a copy
	std::string aux = taskName;

	// See if the task name includes the substring "offline"
	// If it does, the videoprocessor should work in offline mode
	std::transform(aux.begin(), aux.end(), aux.begin(), ::tolower);

	if (aux.find("offline") != std::string::npos)
		m_runningMode = OFFLINE_RUNNING_MODE;
	else
		m_runningMode = ONLINE_RUNNING_MODE;
}

/*!
	Sets the name of the target task, determines if it's an
	online or offline task, and re-initialized the graph of 
	vis sys components IFF the current task name is different
	from 'taskName'.

	The online / offline mode of the task is determined from its
	name. If the name includes the substring "offline" in 
	with any combination of upper or lower case letters, then the 
	mode is set to offline. Otherwise, an online mode is assumed.
*/
void VideoProcessor::SetTargetTaskAndMode(const std::string& taskName)
{
	// The graph of components has to be initialized
	ASSERT(m_pComponents);

	// If the graph of vis sys components was already set to
	// the requested task, there is nothing else to do
	if (m_params.taskName != taskName)
	{
		SetRunningModeFromTaskName(taskName);

		// Update the name of the target task
		m_params.taskName = taskName;

		// Save the current data serializer selection
		VSCDataSerializer* pDS = m_pComponents->GetDataSerializer();

		// Create a new graph of vis sys components
		delete m_pComponents;
		m_pComponents = new VSCGraph;
		m_pComponents->Initialize(m_runningMode, m_params.taskName, pDS, m_pSQLDatabase);

		ShowStatus1("The target task is now", m_params.taskName);
	}
}

void VideoProcessor::OpenSQLDatabase()
{
	if (m_params.sqlDBName.empty())
	{
		// Make sure that there is no database
		if (m_pSQLDatabase)
		{
			delete m_pSQLDatabase;
			m_pSQLDatabase = NULL;
		}

		return;
	}

	// See if we already opened the database
	if (m_pSQLDatabase && m_pSQLDatabase->Name() == m_params.sqlDBName)
	{
		ASSERT(m_pSQLDatabase->IsOpen());
		return;
	}
	else if (m_pSQLDatabase)
	{
		m_pSQLDatabase->Close();
	}
	else
	{
		m_pSQLDatabase = new SQLDatabase();
	}

	if (m_pSQLDatabase && !m_pSQLDatabase->Open(m_params.sqlDBName, 
		m_params.sqlNonBlockingMode))
	{
		ShowError1("Cannot open database", m_params.sqlDBName);

		delete m_pSQLDatabase;
		m_pSQLDatabase = NULL;
	}

	ShowStatus1("Connected to the SQL database", m_params.sqlDBName);
}

/*!
	Initializes all the components of the system.
*/
void VideoProcessor::Initialize()
{
	std::string oldFilename = m_params.strFilename;

	// Clear things and read user arguments
	m_params.ReadFromUserArguments();

	if (!oldFilename.empty())
		m_params.strFilename = oldFilename;

	OpenSQLDatabase();

	// Create a new graph of vis sys components
	delete m_pComponents;
	m_pComponents = new VSCGraph;

	VSCDataSerializer* pSerializer = NULL;

	if (m_params.cacheParseData)
	{
		// Initialize components using user arguments
		m_videoDataBinder.Initialize(m_params.strCacheFilename);
		
		pSerializer = m_videoDataBinder.GetDataSerializer();
	}

	SetRunningModeFromTaskName(m_params.taskName);

	m_pComponents->Initialize(m_runningMode, m_params.taskName, 
		pSerializer, m_pSQLDatabase);
}

void VideoProcessor::GetParameterInfo(int idx, DoubleArray* pMinVals, 
									  DoubleArray* pMaxVals, 
									  DoubleArray* pSteps) const
{
	m_pComponents->GetParameterInfo(idx, pMinVals, pMaxVals, pSteps);
}

std::list<UserCommandInfo> VideoProcessor::GetUserCommands(int idx) const
{
	return m_pComponents->GetUserCommands(idx);
}

void VideoProcessor::GetDisplayInfo(int idx, DisplayInfoIn* dii, DisplayInfoOut* dio) const
{
	m_pComponents->GetDisplayInfo(idx, *dii, *dio);
}

const OutputImageInfo& VideoProcessor::GetOutputImageInfo(int idx) const
{
	return m_pComponents->GetOutputImageInfo(idx);
}

void VideoProcessor::GetVisionComponentLabels(std::vector<std::string>& lbls)
{
	m_pComponents->GetOutputImageLabels(lbls);
}

/*!
	Loads the first requested frame (usually needed for display purposes).
*/
bool VideoProcessor::LoadPreviewFrame()
{
	GenericVideo tempVideo;
	
	// Init all parsing variables
	ResetCurrentVideoData();
	
	if (tempVideo.Load(m_params.strFilename))
	{
		tempVideo.ReadFirstFrame();

		tempVideo.GetCurrentFrameInfo(m_imgInfo);

		m_imgInfo.numFramesProcessed = 0;

		m_pComponents->ProcessFrameForPlayback(m_imgInfo);

		return true;
	}
	else
	{
		ShowError1("Cannot load video:", m_params.strFilename);

		return false;
	}
}

void VideoProcessor::ResetCurrentVideoData()
{
	m_hasFrameRequest = false;
	
	//m_imgInfo.greyFrame.clear();

	m_imgInfo.Clear();

	m_pComponents->Reset();
}

bool VideoProcessor::InitializeVideoProcessing()
{
	ShowStatus("Processing Video");
	
	if (!m_video.Load(m_params.strFilename))
	{
		ShowError("Cannot read video");
		return false;
	}
	
	//DBG_MSG2("Video length =", m_video.FrameCount())
	
	// Init all parsing variables
	ResetCurrentVideoData();

	// Let the components know which video they are processing
	// so that they can log data appropriately (if needed).
	m_pComponents->SetVideoFilename(m_params.strFilename);
	
	if (m_params.cacheParseData)
		m_videoDataBinder.OpenBinding(m_params.strFilename, FrameNumber());

	return true;
}

void VideoProcessor::PlaybackFrame()
{
	m_video.GetCurrentFrameInfo(m_imgInfo);
	
	m_pComponents->ProcessFrameForPlayback(m_imgInfo);
}

void VideoProcessor::ProcessFrame()
{
	//m_video.PrintFrameInfo();

	m_video.GetCurrentFrameInfo(m_imgInfo);
	
	// Check that everything went right
	if (m_imgInfo.greyFrame.size() == 0 || !m_imgInfo.greyFrame.is_contiguous())
	{
		ShowError("Error reading current frame");
		return;
	}

	ShowStatus2("\nProcessing frame", FrameNumber(), "...");

	// See if there is pre-saved frame data and, if there is, 
	// use it to process the frame
	if (m_params.cacheParseData)
		m_videoDataBinder.LoadFrameData(FrameNumber());

	// Parse the video frame
	m_pComponents->ProcessNewFrame(m_imgInfo);

	// See if we have to save parse data
	if (m_params.cacheParseData)
		m_videoDataBinder.SaveFrameData(FrameNumber());

	// Update the number of frames processed
	m_imgInfo.numFramesProcessed++;
}

/*!
	It initializes the video and processes all frames in it. That is, this
	function is all that needs to be called to process a video
	without user intervention (ie, no GUI).
*/
bool VideoProcessor::ProcessVideo()
{
	if (!InitializeVideoProcessing())
		return false;

	for (ReadFirstFrame(); !IsLastFrame(); ReadNextFrame())
		ProcessFrame();

	PostProcessVideo();

	return true;
}

/*!
	Calls the PostProcess() function of each component. It is useful
	for doing postprocessing at the end of the video.
*/
void VideoProcessor::PostProcessVideo()
{
	m_pComponents->PostProcessSequence();
}

/*!
	Calls the PostProcess() function of each component. It is useful
	for doing postprocessing at the end of the video.
*/
void VideoProcessor::ProcessDataOffline()
{
	m_pComponents->ProcessDataOffline();
}

void VideoProcessor::SaveResults(bool status)
{
	if (status)
	{
		//m_loadedVideoFilename + ".wmv"
		m_pComponents->StartRecordingResults("testvideo.avi");

		//m_video.CodecCode(), m_video.FrameRate(), 
		//	m_imgInfo.rgbFrame.ni(), m_imgInfo.rgbFrame.nj()
	}
	else
	{
		m_pComponents->StopRecordingResults();
	}
}

/*!
	Calls the OnGUIEvent() function of each component. It is useful
	for doing postprocessing of each frame.

	This function is called when there is a GUI event that some
	component might want to know about. It is delivered to all
	components in their dependency-sorted order.
*/
void VideoProcessor::OnGUIEvent(int id, int value)
{
	m_pComponents->OnGUIEvent(id, value);

	// See if we have to save parse data created after
	// the event EVENT_FINISHED_FRAME was processed by all components
	if (id == EVENT_FINISHED_FRAME && m_params.cacheParseData)
		m_videoDataBinder.SaveFrameData(FrameNumber());
}
