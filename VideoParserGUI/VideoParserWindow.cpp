/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoParserWindow.h"
#include "ModeSelector.h"
#include "VideoControlPanel.h"
#include "ViewSelector.h"
#include "ImageView.h"
#include "ParameterSelector.h"
#include "TextView.h"
#include "CommandSelector.h"
#include "Plotter.h"
#include "UserArgumentsQuickView.h"
#include <FL/Fl_Button.h>
#include <Tools/VisSysComponent.h>
#include <Tools/UserArguments.h>
#include <Tools/Timer.h>

#ifndef WIN32
#define HAVE_PTHREAD_H 1
#else
#include <tchar.h>
#endif

#include "threads.h"

#define WINDOW_H_MARGIN 30
#define WINDOW_V_MARGIN 30
#define IMAGE_VIEW_H_SPACING 50

using namespace vpl;

extern UserArguments g_userArgs;

///////////////////////////////////////////////////////////////////////////////////////////////
// Global functions and variables

Fl_Thread g_ParseVideoThreadID;
char g_szTextBuffer[1024];

//! Globals function used to create new video parsing threads
void* ParseVideo(void* pWnd)
{
	((VideoParserWindow*)pWnd)->ProcessVideo();
	return 0;
}

void ShowMemoryUsage(fnum_t frameNum, time_t timestamp)
{
	MEMORYSTATUSEX statex;

	statex.dwLength = sizeof (statex);

	if (!GlobalMemoryStatusEx(&statex))
		return;
	
	// Specify the width of the field in which to print the numbers. 
	// The asterisk in the format specifier "%*I64d" takes an integer 
	// argument and uses it to pad and right justify the number.
	const int WIDTH = 7;

	// Use to convert bytes to KB
	const int DIV = 1024;

	StreamMsg("Memory usage at time " << Time(timestamp).str() << 
		" for frame number " << frameNum << "\n");

	_tprintf (TEXT("There is  %*ld percent of memory in use.\n"),
			WIDTH, statex.dwMemoryLoad);

	//_tprintf (TEXT("There are %*I64d total Kbytes of physical memory.\n"),
	//		WIDTH, statex.ullTotalPhys/DIV);
	//_tprintf (TEXT("There are %*I64d free Kbytes of physical memory.\n\n"),
	//		WIDTH, statex.ullAvailPhys/DIV);
	//_tprintf (TEXT("There are %*I64d total Kbytes of paging file.\n"),
	//		WIDTH, statex.ullTotalPageFile/DIV);
	//_tprintf (TEXT("There are %*I64d free Kbytes of paging file.\n"),
	//		WIDTH, statex.ullAvailPageFile/DIV);
	//_tprintf (TEXT("There are %*I64d total Kbytes of virtual memory.\n"),
	//		WIDTH, statex.ullTotalVirtual/DIV);
	//_tprintf (TEXT("There are %*I64d free Kbytes of virtual memory.\n"),
	//		WIDTH, statex.ullAvailVirtual/DIV);

	// Show the amount of extended memory available.

	//_tprintf (TEXT("There are %*I64d free Kbytes of extended memory.\n"),
	//		WIDTH, statex.ullAvailExtendedVirtual/DIV);
}

///////////////////////////////////////////////////////////////////////////////////////////////
// VideoParserWindow::ParsingParams 
void VideoParserWindow::Initialize()
{
	// Initialize all member variabes
	m_bParsingInProgress = false;
	m_bPauseParsing = false;

	m_curImgWidth = -1;
	m_curImgHeight = -1;

	m_videoProcessor.Initialize();

	// Now, the child windows and widgets can be initialized
	for (unsigned int i = 0; i < m_panels.size(); i++)
	{
		m_panels[i]->Initialize();
		m_panels[i]->GetView()->SetScaling(m_zoomLevel);
	}

	m_modeSelector->Initialize();
	m_usrArgMonitor->Initialize();
	
	//m_usrArgMonitor->AddBoolArgument("VideoProcessor", "cacheParseData", "cache");
}

/*!
	Starts or stops video parsing. If there is a paring in progress, this
	functions acts as if it was called StopParsing().

	The parsing model is obtained from the modeSelector widget.
*/
void VideoParserWindow::StartParsing()
{
	// See if we have to stop a current parsing process or if...
	if (m_bParsingInProgress)
	{
		ShowStatus("Stopping parsing process...");
		m_bStopParsing = true;
		m_bPauseParsing = false;
	}
	else // ... we must start a new parsing process
	{
		// Do not set m_bPauseParsing because the user may want to
		// pause on the first frame.
		m_bParsingInProgress = true;
		m_bStopParsing = false;

		fl_create_thread(g_ParseVideoThreadID, ParseVideo, this);
	}
}

void VideoParserWindow::PauseParsing()
{
	m_bPauseParsing = !m_bPauseParsing;
}

/*!
	Updates the ImageView in the given control panel with
	the i'th component output for the given parameters
*/
void VideoParserWindow::UpdateImageView(VideoControlPanel* pPanel, int viewIdx, 
										const DoubleArray& params, bool checkViewSync)
{
	// Get the appropriate image from the components' graph
	vpl::DisplayInfoIn dii;
	vpl::DisplayInfoOut dio;

	dii.params = params;
	dii.displayId = GetPanelID(pPanel);

	// The other fields of dii are populated by the video processor
	m_videoProcessor.GetDisplayInfo(viewIdx, &dii, &dio);

	// Update the selected view with the new image
	pPanel->GetView()->UpdateImage(dio.imagePtr, dio.imageType, 
		m_videoProcessor.GetStatusText(), dio.specs);

	// Update the output text
	pPanel->GetTextView()->SetText(dio.message.c_str());

	// See if the component also wants to draw something
	const OutputImageInfo& oii = m_videoProcessor.GetOutputImageInfo(viewIdx);

	if (oii.component->HasDrawFunction())
		pPanel->GetView()->UpdatePainter(oii.component, dii);
	else
		pPanel->GetView()->RemovePainter();

	// See if the component wants its other views to have the same params
	if (checkViewSync && dio.syncDisplayViews)
	{
		for (unsigned i = 0; i < m_panels.size(); ++i)
		{
			if (m_panels[i] != pPanel)
				UpdatePanel(m_panels[i], viewIdx, params, false);
		}
	}
}

/*!
	Updates the current processing mode.
*/
void VideoParserWindow::OnModeSelection(ModeSelector* pSelector)
{
	// There is nothing to do other that try to stop any current
	// parsing process, so that the parse button acts as "play"
	// instead of "stop".
	while (IsParsing())
		StopParsing(30);

	// Change the video processor's task unless the selection is
	// the playback mode, which is an additional mode and not
	// a valid processign task
	if (!pSelector->PlaybackMode())
		m_videoProcessor.SetTargetTaskAndMode(pSelector->GetSelection());
}

/*!
	Updates the ImageView associated with the given ParameterSelector
	according to the current parameter selection
*/
void VideoParserWindow::OnParameterSelection(ParameterSelector* pSelector)
{
	VideoControlPanel* pPanel = GetPanel(pSelector);
	ASSERT(pPanel);

	int sel = pPanel->GetViewSelector()->GetSelection();
	DoubleArray params = pSelector->GetValues();

	// Update the selected view with the new image
	UpdateImageView(pPanel, sel, params, true);
}

/*!
	Updates both the ImageView and the ParameterSelector associated
	with the given ViewSelector according to the current view selection 
*/
void VideoParserWindow::OnViewSelection(ViewSelector* pSelector, bool checkViewSync)
{
	VideoControlPanel* pPanel = GetPanel(pSelector);
	ASSERT(pPanel);

	// Get the component selection 
	int sel = pSelector->GetSelection();

	// Get the current parameter from the ParameterSelector
	DoubleArray params = pPanel->GetParamSelector()->GetValues();

	UpdatePanel(pPanel, sel, params, checkViewSync);
}

/*!
	Updates the ImageView and the ParameterSelector associated
	with the given panel.
*/
void VideoParserWindow::UpdatePanel(VideoControlPanel* pPanel, int sel, 
		DoubleArray params, bool checkViewSync)
{
	// Get the potentially new parameter range from the component
	DoubleArray minVals, maxVals, steps;

	m_videoProcessor.GetParameterInfo(sel, &minVals, &maxVals, &steps);

	// Ensure that the current parameters are within the "new" ranges
	if (params.size() == minVals.size())
	{
		for (unsigned i = 0; i < params.size(); i++)
		{
			if (params[i] > maxVals[i])
				params[i] = maxVals[i];

			if (params[i] < minVals[i])
				params[i] = minVals[i];
		}
	}
	else
	{
		params = minVals;
	}

	// Update the selected view with the new image
	UpdateImageView(pPanel, sel, params, checkViewSync);

	// Update the displayed parameter value and range
	pPanel->GetParamSelector()->Update(params, minVals, maxVals, steps);

	pPanel->GetCommandSelector()->UpdateCommands(
		m_videoProcessor.GetUserCommands(sel));
}

/*! 
	bBothPanels = false: Updates the image in the first view panel only.
    bBothPanels = true: Used in parsing mode to update the second view panel too.
	bUpdateSize = true: needs to be used only once for each input with new dimensions
*/
void VideoParserWindow::UpdateDisplay(bool bBothPanels /*=true*/, bool bUpdateSize /*=false*/)
{
	OnViewSelection(m_panels[0]->GetViewSelector());

	if (bBothPanels)
		OnViewSelection(m_panels[1]->GetViewSelector(), false);
	
	if (bUpdateSize || 
		m_curImgWidth != CurrentFrame().ni() || 
		m_curImgHeight != CurrentFrame().nj())
	{
		VideoControlPanel* pPanel;
		int dx, dy;

		m_curImgWidth = CurrentFrame().ni();
		m_curImgHeight = CurrentFrame().nj();

		// Set the size of the panels and their widget based on the
		// size of the current frame
		pPanel = m_panels[0];
		pPanel->ResizeGroup(m_curImgWidth, m_curImgHeight, WINDOW_H_MARGIN, m_zoomLevel);
		dx = pPanel->x() + pPanel->w() + IMAGE_VIEW_H_SPACING;
		
		pPanel = m_panels[1];
		pPanel->ResizeGroup(m_curImgWidth, m_curImgHeight, dx, m_zoomLevel);
		dx = pPanel->x() + pPanel->w() + WINDOW_H_MARGIN;
		dy = pPanel->y() + pPanel->h() + WINDOW_V_MARGIN;

		// Update the width of the main menue bar
		m_menuBar->size(dx, m_menuBar->h());

		// Update the size of the window
		size(dx, dy);
	}

	redraw(); // seems to be necessary to call redraw()
	
	// Wake up the drawing thread
	Fl::awake();
}

void VideoParserWindow::SetZoomLevel(const double& zl)
{
	m_zoomLevel = zl;

	UpdateWindowTitle();

	UpdateDisplay(true, true);
}

/*!
	Sets the window title using the current loaded video
	file name and zoom level.
*/
void VideoParserWindow::UpdateWindowTitle()
{
	std::string wndLbl = m_loadedVideoFilename;

	if (m_zoomLevel != 1)
	{
		std::ostringstream oss;

		oss << " [" << m_zoomLevel << "] - Video Parser";

		wndLbl += oss.str();
	}

	wndLbl += " - Video Parser";

	label(wndLbl.c_str());
}

void VideoParserWindow::ResetDashboard()
{
	const char* lbl;
	
	for (int n = 0; n < children(); n++)
		if ((lbl = child(n)->label()))
			if (!strcmp(lbl, "@>") || !strcmp(lbl, "@||") || !strcmp(lbl, "@->"))
				((Fl_Button*)child(n))->value(0);

	// Wake up the drawing thread
	Fl::awake();
}

/*void VideoParserWindow::GetSelectedImage(ImageView* pView, DisplayInfoOut& dio)
{
	VideoControlPanel* pPanel = GetPanel(pView);
	ASSERT(pPanel);

	int sel = pPanel->GetViewSelector()->GetSelection();

	m_videoProcessor.GetDisplayInfo(sel, dio);
}*/

/*!
	For some reason, it doesn't work to load the video and store it
	in m_movie so that it can be processed later. It seems that it
	may be related to the multithread approach, since LoadVideo is
	a different thread than ProcessVideo.

	@param filename [optional] file name of the video to load. If NULL,
	the video processor parameters are used to determine teh video to load.
*/
void VideoParserWindow::LoadVideo(const char* filename)
{
	// Make sure that there is no active parsing
	while (IsParsing())
		StopParsing(30);

	if (filename)
		SetVideoFilename(std::string(filename));
	
	if (m_videoProcessor.LoadPreviewFrame())
	{
		// Reset the zoom level only of input video is different than current
		if (GetParams().strFilename != m_loadedVideoFilename)
			m_zoomLevel = 1;

		// Update the name of the current loaded video
		m_loadedVideoFilename = GetParams().strFilename;

		// Update the title of the window (uses the loaded video file name)
		UpdateWindowTitle();

		UpdateDisplay(true, true);
	}
}

void VideoParserWindow::ProcessVideo()
{
	// Lock the mutex to let the GUI thread know that
	// the parsing thread is active
	m_userSyncMutex.Lock();

	bool justPlayVideo = m_modeSelector->PlaybackMode();
	bool hasInitVideo;
	
	if (m_videoProcessor.OfflineMode() && !justPlayVideo)
	{
		m_videoProcessor.ProcessDataOffline();
		hasInitVideo = false;
	}
	else
	{
		hasInitVideo = m_videoProcessor.InitializeVideoProcessing();

		if (hasInitVideo)
			m_loadedVideoFilename = GetParams().strFilename;
	}

	if (hasInitVideo)
	{
		Timer tic;
		unsigned frameCounter = 0;

		for (m_videoProcessor.ReadFirstFrame(); !m_videoProcessor.IsLastFrame(); 
			m_videoProcessor.ReadNextFrame())
		{
			if (justPlayVideo) // just play back the video at the correct frame rate
			{
				m_videoProcessor.PlaybackFrame();
			
				// Delay the playback of the video if necessary
				if (!m_videoProcessor.IsFirstFrame())
				{ 
					double deltaTime = 1000 / m_videoProcessor.FrameRate();
					double sleepTime = deltaTime - tic.ElapsedTime();

					if (sleepTime > 0)
					{
						//ShowStatus2("Sleeping for ", sleepTime, "milliseconds");
						Sleep((DWORD) sleepTime);
					} 
				}

				tic.Reset();
			}
			else
			{
				m_videoProcessor.ProcessFrame();
			}

			// ... and update the display to show it. If only playing the video, update
			// one panel only (ie, updateBothPanels == false if PLAY_VIDEO_MODE)
			UpdateDisplay(!justPlayVideo, false);
		
			// Pause if requested. Use cvWaitKey(milliseconds) or sleep(seconds)
			while (m_bPauseParsing && m_bParsingInProgress && 
				!m_bStopParsing && !m_videoProcessor.HasFrameRequest())
			{
				Sleep(100); 
			}

			// Processing is done, so give all components the opportunity
			// to save any GUI-collected user data (e.g., in the cache file)
			// This is also done in Finalize()
			m_videoProcessor.OnGUIEvent(EVENT_FINISHED_FRAME, 0);
		
			if (VerboseLevel() >= VERBOSE_LEVEL_MINIMAL && frameCounter++ > 250)
			{
				//ShowMemoryUsage(m_videoProcessor.FrameNumber(), m_videoProcessor.CurrentFrameTime());
				frameCounter = 0;
			}

			// Stop if requested
			if (m_bStopParsing || !m_bParsingInProgress)
				break;
		}

		if (!justPlayVideo)
		{
			m_videoProcessor.PostProcessVideo();
		}
	}

	m_bParsingInProgress = false;
	
	ResetDashboard();

	// Release the mutex to let the GUI thread know that
	// the parsing thread is done
	m_userSyncMutex.Release();
}


