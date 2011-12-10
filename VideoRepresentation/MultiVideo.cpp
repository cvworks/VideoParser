/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "MultiVideo.h"

#include <Tools/DirWalker.h>
#include <Tools/Tuple.h>
#include <Tools/Exceptions.h>
#include <Tools/FileAttributes.h>

using namespace vpl;

bool MultiVideo::LoadVideo()
{
	ASSERT(m_vidIt != m_vids.end());

	m_pVideo = ConstructVideoObject(m_vidIt->filename);

	if (!m_pVideo || !m_pVideo->Load(m_vidIt->filename))
		return false;

	// Update the video info with our own timestamp
	m_pVideo->SetTimestamp(m_vidIt->timestamp);
		
	return true;
}

bool MultiVideo::Load(std::string strFilename)
{
	Clear();

	if (!m_params.ReadParameters(strFilename.c_str()))
	{
		ShowOpenFileError(strFilename);
		return false;
	}

	ShowStatus("Multi video file loaded");

	std::string timeOpt;

	m_params.ReadArg("timestamp", "Start time attribute for all videos. "
		"Can be: 'creation', 'filename', or a formated string. " 
		"eg, filename: ANY_PREFIX_20110511_1341.avi. "
		"eg, formated string: '6 Dec 2001 12:33:45'.",
		std::string("filename"), &timeOpt);

	std::string rootDir;
	
	// First, try reading all optional parameters so that their usage strings are known
	//m_params.ReadArg("count", "Number of images to read [-1 for all]", -1, &m_nFrameCount);
	m_params.ReadArg("rootDir", "Root directory", std::string("."), &rootDir);

	// See if the files have relative directories and if this file
	// has an absolute path
	if (!rootDir.empty() && rootDir[0] == '.') // it's relative dir
	{
		std::string path = DirWalker::GetPath(strFilename.c_str());

		if (!path.empty() && path[0] != '.') // it's absolute dir
			rootDir = DirWalker::ReplacePath(rootDir.c_str(), path.c_str());
	}

	StrList vidPaths;
	bool autoMode;

	m_params.ReadBoolArg("autoMode", "Add all video files in the root directory", 
		false, &autoMode);

	if (autoMode)
	{
		vidPaths = CollectVideoFiles(rootDir);
	}
	else
	{
		vidPaths = ReadVideoFilenames();
	}

	VideoInfo vi;
	unsigned videoId = 0;

	for (auto it = vidPaths.begin(); it != vidPaths.end(); ++it, ++videoId)
	{
		vi.filename = *it; //DirWalker::ConcatPathElements(rootDir,*it);

		vi.timestamp = GetTimestamp(videoId, vi.filename, timeOpt);

		m_vids.push_back(vi);
	}

	ROISequence boxes;

	m_params.ReadArgs("roi", "List of regions of interest {xmin,xmax,ymin,ymax}", 
		boxes, &boxes);

	std::string strStart, strEnd;

	m_params.ReadArg("startTime", "Start date/time of the video", std::string(), &strStart);
	m_params.ReadArg("endTime", "End date/time of the video", std::string(), &strEnd);

	if (!FilterVideos(strStart, strEnd))
		return false;

	m_vidIt = m_vids.begin();

	if (vidPaths.empty())
	{
		ShowError("There are no videos that meet the start-end time specs");
		return false;
	}

	if (!LoadVideo())
		return false;

	SetVideoInfo(strFilename, m_pVideo->FrameCount(), m_pVideo->StartTime());

	if (!boxes.empty())
		SetROISequence(boxes);

	return true;
}

bool MultiVideo::FilterVideos(std::string strStart, std::string strEnd)
{
	try {
		m_startTime = Time(strStart).toSeconds(); // empty str makes time = -1
		m_endTime = Time(strEnd).toSeconds();     // empty str makes time = -1
	}
	catch (BasicException e)
	{
		e.Print();
		return false;
	}

	if (m_startTime >= 0)
	{
		// Last video to remove (star by removing ALL videos)
		auto endIt = m_vids.end();

		auto it1 = m_vids.begin();
		auto it0 = it1++;

		for (; it1 != m_vids.end(); ++it0, ++it1)
		{
			// If vid ends before t0, then remove it
			if (it1->timestamp < m_startTime)
				endIt = it1; // not that it1 is not removed
			else
				break; // current vid starts at or after start time
		}

		// If the current end is the list's end, then
		// it means that endIt was not changed in the loop
		if (endIt != m_vids.end())
			m_vids.erase(m_vids.begin(), endIt);
	}

	if (m_endTime >= 0)
	{
		auto it = m_vids.begin();

		// If vid starts before or at t1, then keep it
		for (; it != m_vids.end() && it->timestamp <= m_endTime; ++it)
			;

		m_vids.erase(it, m_vids.end());
	}

	/*if (m_startTime >= 0 || m_endTime >= 0)
	{
		DBG_PRINT2(m_startTime, m_endTime)

		if (m_startTime >= 0)
			DBG_PRINT1(Time(m_startTime).str())
		
		if (m_endTime >= 0)
			DBG_PRINT1(Time(m_endTime).str())

		DBG_PRINT2(m_vids.front().filename, m_vids.back().filename)
		DBG_PRINT2(Time(m_vids.front().timestamp).str(), Time(m_vids.back().timestamp).str())
			
		ReadMetadata();
	}*/

	return true;
}

StrList MultiVideo::CollectVideoFiles(std::string rootDir)
{
	StrList vidPaths;
	unsigned maxLevel;

	m_params.ReadArg("maxRecurseLevels", "Maximum number of subdirectory levels to visit", 
		0u, &maxLevel);

	StrList globs, defaultGlobs;

	defaultGlobs.push_back("*.avi");
	defaultGlobs.push_back("*.mpg");
	defaultGlobs.push_back("*.mpeg");
	defaultGlobs.push_back("*.mp4");
	defaultGlobs.push_back("*.wmv");
	defaultGlobs.push_back("*.svf");
	defaultGlobs.push_back("*.isf");
	defaultGlobs.push_back("*.mvf");
	
	m_params.ReadArgs("glob", "Globbing on filenames (like a command line glob)",
		defaultGlobs, &globs);

	// We can now collect, parse, and sort the filenames
	// To allow for multiple globs, eg, *.avi and *.mpg
	// we collect for each glob separatelly.
	std_forall(globIt, globs)
	{
		DirWalker::CollectFileNames(rootDir, *globIt, maxLevel, vidPaths);
	}

	if (vidPaths.empty())
		ShowError("No video files were found. Is the glob spec correct?");

	// Sort the files names by interpreting numeric characters as numbers
	DirWalker::SortFileNamesNumerically(vidPaths);

	return vidPaths;
}

StrList MultiVideo::ReadVideoFilenames()
{
	int minId, maxId;

	try {
		if (!m_params.GetFieldPrefixMinMax(std::string("video"), 
			&minId, &maxId))
		{
			ShowError("There are no video files specified.");
			return StrList();
		}

		ASSERT(minId <= maxId);
	}
	catch(BasicException e) 
	{
		e.Print();
		return StrList();
	}

	// Iterate over all Experiments fields. We plot the recognition 
	// performance as a function of maxNumParses
	char prefix[MAX_PATH_SIZE];
	StrList vidPaths;
	std::string timeOpt;
	Time startTime;

	for (int id = minId; id <= maxId; id++)
	{
		sprintf(prefix, "video%d", id);

		try {
			vidPaths.push_back(m_params.GetStrValue(prefix, "filename"));
		}
		catch(BasicException e) 
		{
			e.Print();
			return StrList();
		}
	}

	return vidPaths;
}

time_t MultiVideo::GetTimestamp(unsigned id, const std::string& filename,
	std::string timeOpt)
{
	char prefix[MAX_PATH_SIZE];
	Time timestamp;

	sprintf(prefix, "video%d", id);

	// See if there is a timeopt specified for this video
	m_params.ReadArg(prefix, "timestamp", "Start time attribute for the video. "
		"Can be: 'creation', 'filename', or a formated string. " 
		"eg, filename: ANY_PREFIX_20110511_1341.avi. "
		"eg, formated string: '6 Dec 2001 12:33:45'.",
		timeOpt, &timeOpt);

	if (timeOpt == "creation")
	{
		timestamp = DirWalker::ReadCreationTime(filename);
	}
	else if (timeOpt == "filename")
	{
		auto dotIt = filename.find('.');
		
		if (dotIt < 13)
			THROW_BASIC_EXCEPTION("Bad filename format");

		auto str = filename.substr(dotIt - 13, 13);

		auto sep = str.substr(8, 1);

		if (sep.front() != '_')
			THROW_BASIC_EXCEPTION("Invalid time-formated filename");

		auto year = str.substr(0, 4);
		auto month = str.substr(4, 2);
		auto day = str.substr(6, 2);
		auto hour = str.substr(9, 2);
		auto minutes = str.substr(11, 2);

		timestamp.set_year(atoi(year.c_str()));
		timestamp.set_month(atoi(month.c_str()));
		timestamp.set_day(atoi(day.c_str()));

		timestamp.set_hour(atoi(hour.c_str()));
		timestamp.set_minutes(atoi(minutes.c_str()));

		// Pass true to indicate that the time is expressed wrt the current DST state
		timestamp.adjust(true); 
	}
	else
	{
		// Convert string to time by siply assigning it to a Time object
		timestamp = timeOpt;
	}

	//DBG_PRINT1(timestamp.str())

	return timestamp.toSeconds();
}

/*!
	Read the i'h frame from the start of the video.
*/
void MultiVideo::ReadFrame(fnum_t i)
{
	if (m_startTime >= 0)
		i += TimeToFrame(m_startTime);

	//Timer tic;

	// See if the frame is not in the current video
	if (i < m_frameOffset || i >= m_frameOffset + m_pVideo->FrameCount())
	{
		// Make sure that all the videos have their metadata
		if (m_vids.front().frameCount < 0)
			ReadMetadata();

		//StreamMsg("Video metadata read in " << tic.ElapsedTime() << " milliseconds");

		// Look for the video that has the requested frame

		// If we are ahead, restart from the beginning
		if (FrameNumber() > i)
		{
			m_frameOffset = 0;
			m_vidIt = m_vids.begin();
		}

		for (; m_vidIt != m_vids.end(); ++m_vidIt)
		{
			if (i < m_vidIt->frameCount + m_frameOffset)
				break;
			else
				m_frameOffset += m_vidIt->frameCount;
		}

		//StreamMsg("Target video found in " << tic.ElapsedTime() << " milliseconds");

		LoadVideo();

		//StreamMsg("Video loaded in " << tic.ElapsedTime() << " milliseconds");
	}

	// Read the corresponding frame in the video
	m_pVideo->ReadFrame(i - m_frameOffset);

	//StreamMsg("Frame found in " << tic.ElapsedTime() / 1000.0 << " seconds");
}
