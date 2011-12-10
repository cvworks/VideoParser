/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VIDEO_PARSER_WINDOW_H_
#define _VIDEO_PARSER_WINDOW_H_

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Menu_Bar.H>
#include "VideoControlPanel.h"
#include <Tools/Mutex.h>
#include <VideoParser/VideoProcessor.h>
#include <Tools/UserEvents.h>

//! This function can be called from any subclass of Fl_window
#define GetMainWindow() ((VideoParserWindow*) window())

class ModeSelector;
class ImageView;
class VideoControlPanel;
class UserArgumentsQuickView;

/*!
*/
class VideoParserWindow : public Fl_Double_Window
{ 
public:
	enum FrameSelection {FIRST_FRAME, LAST_FRAME, 
		ONE_FRAME_FORWARD, ONE_FRAME_BACKWARD, 
		ONE_SECOND_FORWARD, ONE_SECOND_BACKWARD};

	//enum ProcessingMode {PLAY_VIDEO_MODE, RECOGNITION_MODE, 
	//	ONLINE_LEARNING_MODE, OFFLINE_LEARNING_MODE};

  private:
	vpl::VideoProcessor m_videoProcessor;

	std::string m_loadedVideoFilename;

	int m_curImgWidth;
	int m_curImgHeight;

	bool m_bParsingInProgress;
	bool m_bStopParsing;
	bool m_bPauseParsing;

	double m_zoomLevel;

	std::vector<VideoControlPanel*> m_panels;
	Fl_Menu_Bar* m_menuBar;
	ModeSelector* m_modeSelector;

	UserArgumentsQuickView* m_usrArgMonitor;

	vpl::Mutex m_userSyncMutex; //!< Mutex to sync with GUI

  protected:
	void ResetDashboard();
	void UpdateDisplay(bool bBothPanels = true, bool bUpdateSize = false);
	Fl_Group* SetPanelSize(int id, int x);

	//: Sets the m_videoProcessor::m_params.strFilename
	void SetVideoFilename(const std::string& fn) 
	{ 
		ASSERT(!IsParsing());
		m_videoProcessor.SetVideoFilename(fn); 
	}
 
  public:
	VideoParserWindow(int w, int h, const char* title = 0)
		: Fl_Double_Window(w, h, title)
	{
		m_zoomLevel = 1;
	}

	VideoParserWindow(int x, int y, int w, int h, const char* title = 0)
		: Fl_Double_Window(x, y, w, h, title)
	{
		m_zoomLevel = 1;
	}

	void Initialize();

	/*! 
		Releases all resouces. Must be called before the object is destroyed.
		
		In addition, all components are given the opportunity
		to save any GUI-collected user data (e.g., in the cache file),
		as is also done after each frame is processed by ProcessVideo().
	*/
	void Finalize()
	{
		m_videoProcessor.OnGUIEvent(EVENT_FINISHED_FRAME, 0);

		m_videoProcessor.ReleaseResources();
	}
	
	void SetPanels(VideoControlPanel* panel1, VideoControlPanel* panel2)
	{
		m_panels.resize(2);

		m_panels[0] = panel1;
		m_panels[1] = panel2;
	}

	void SetModeSelector(ModeSelector* pSelector)
	{
		m_modeSelector = pSelector;
	}

	void SetMenuBar(Fl_Menu_Bar* pMenu)
	{
		m_menuBar = pMenu;
	}

	void SetUserArgumentMonitor(UserArgumentsQuickView* pMonitor)
	{
		m_usrArgMonitor = pMonitor;
	}

	int GetPanelID(VideoControlPanel* pPanel) const
	{
		for (unsigned i = 0; i < m_panels.size(); ++i)
			if (m_panels[i] == pPanel)
				return i;

		return -1;
	}

	void SetZoomLevel(const double& zl);

	double GetZoomLevel() const
	{
		return m_zoomLevel;
	}

	void UpdateWindowTitle();

	void UpdateImageView(VideoControlPanel* pPanel,	int viewIdx, 
		const DoubleArray& params, bool checkViewSync);

	void UpdatePanel(VideoControlPanel* pPanel, int sel, 
		DoubleArray params, bool checkViewSync);

	void OnModeSelection(ModeSelector* pSelector);
	void OnViewSelection(ViewSelector* pSelector, bool checkViewSync = true);
	void OnParameterSelection(ParameterSelector* pSelector);

	vpl::fnum_t FrameNumber() const  { return m_videoProcessor.FrameNumber(); }
	FloatImg CurrentFrame()  { return m_videoProcessor.CurrentFrame(); }
	RGBImg CurrentRGBFrame() { return m_videoProcessor.CurrentRGBFrame(); }

	void GetVisionComponentLabels(std::vector<std::string>& lbls)
	{
		m_videoProcessor.GetVisionComponentLabels(lbls);
	}

	/*!
		Gets the list of processing modes and returns the index
		if the current mode (returns zero if the current task
		is not in the array).
	*/
	unsigned GetProcessingModes(std::vector<std::string>& lbls) const
	{
		lbls = m_videoProcessor.ValidTaskLabels();

		for (unsigned i = 0; i < lbls.size(); i++)
			if (m_videoProcessor.CurrentTask() == lbls[i])
				return i;

		return 0;
	}

	//: Gets the m_videoProcessor::m_params.strFilename
	std::string GetVideoFilename() const 
	{ 
		return m_videoProcessor.GetVideoFilename(); 
	}

	const vpl::VideoProcessor::Params& GetParams() const 
	{ 
		return m_videoProcessor.GetParams(); 
	}

	void SetParams(const vpl::VideoProcessor::Params& params) 
	{ 
		m_videoProcessor.SetParams(params); 
	}

	void SaveResults(bool status)
	{
		m_videoProcessor.SaveResults(status);
	}

	bool IsParsing() const  { return m_bParsingInProgress; }
	bool IsStopping() const { return m_bParsingInProgress && m_bStopParsing;}

	void LoadVideo(const char* filename = NULL);
	void StartParsing();

	/*!
		Attemps to stop the parsing process for up to 
		a maximum of seconds. It returns true of the parsing
		stopped and false otherwise.
	*/
	bool StopParsing(unsigned long timeoutSeconds)
	{
		ShowStatus("Stopping parsing process...");

		// Tell the parsing thread to stop and wait for it
		m_bStopParsing = true;

		// Wait by trying to lock the mutex
		if (!m_userSyncMutex.Lock(timeoutSeconds * 1000))
		{
			// Could not lock the resource. There is some
			// heavy parsing going on...
			ShowStatus("Please wait...");
			return false;
		}
		
		// The parsing thread finished, so release the mutex
		m_userSyncMutex.Release();

		ShowStatus("Parsing process stopped");
		
		return true;
	}

	void ProcessVideo();
	void PauseParsing();

	VideoControlPanel* GetPanel(ImageView* pView)
	{
		for (unsigned int i = 0; i < m_panels.size(); i++)
		{
			if (m_panels[i]->GetView() == pView)
				return m_panels[i];
		}

		return NULL;
	}

	VideoControlPanel* GetPanel(ViewSelector* pSelector)
	{
		for (unsigned int i = 0; i < m_panels.size(); i++)
		{
			if (m_panels[i]->GetViewSelector() == pSelector)
				return m_panels[i];
		}

		return NULL;
	}

	VideoControlPanel* GetPanel(ParameterSelector* pSelector)
	{
		for (unsigned int i = 0; i < m_panels.size(); i++)
		{
			if (m_panels[i]->GetParamSelector() == pSelector)
				return m_panels[i];
		}

		return NULL;
	}

	//void GetSelectedImage(ImageView* pView, DisplayInfoOut& dio);

	void RequestRelativeFrame(FrameSelection fs)
	{
		if (m_videoProcessor.HasFrameRequest())
		{
			ShowStatus("We are still processing a previous frame request");
			return;
		}

		switch (fs)
		{
			case FIRST_FRAME: 
				m_videoProcessor.RequestFrame(0);
				break;
			//case LAST_FRAME: 
				//m_video.ReadFrame(-1); 
				//break;
			case ONE_FRAME_FORWARD: 
				m_videoProcessor.RequestFrame(m_videoProcessor.FrameNumber() + 1);
				break;
			case ONE_FRAME_BACKWARD: 
				if (m_videoProcessor.FrameNumber() > 0)
					m_videoProcessor.RequestFrame(m_videoProcessor.FrameNumber() - 1);
				break;
			case ONE_SECOND_FORWARD: 
				m_videoProcessor.RequestFrame(m_videoProcessor.FrameNumber() + 
					(int)m_videoProcessor.FrameRate());
				break;
			case ONE_SECOND_BACKWARD:
				if (m_videoProcessor.FrameNumber() >= m_videoProcessor.FrameRate())
				{
					m_videoProcessor.RequestFrame(m_videoProcessor.FrameNumber() -
						(int)m_videoProcessor.FrameRate());
				}
				break;
		}
	}
};

#endif //_VIDEO_PARSER_WINDOW_H_

