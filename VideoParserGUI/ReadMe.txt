///////////////////////////////////////////////////////////////////////////////////////////////		
	void SyncAllViews(VideoControlPanel* pMasterPanel, int viewIdx, 
		const DoubleArray& params);
		
		void VideoParserWindow::SyncAllViews(VideoControlPanel* pMasterPanel, int masterSel, 
		const DoubleArray& masterParams)
{
	for (unsigned i = 0; i < m_panels.size(); ++i)
	{
		VideoControlPanel* pPanel = m_panels[i];

		if (pPanel != pMasterPanel)
		{
			int sel = pPanel->GetViewSelector()->GetSelection();
			DoubleArray params =  pPanel->GetParamSelector()->GetValues();

			if (sel != masterSel || params != masterParams)
				UpdatePanel(pPanel, masterSel, masterParams, false);
		}
	}
}


// See if the component wants its other views to have the same params
	if (checkViewSync && dio.syncDisplayViews)
		SyncAllViews(pPanel, viewIdx, params);
///////////////////////////////////////////////////////////////////////////////////////////////		
{
				pPanel->GetViewSelector()->SetSelection(masterSel);
				//pPanel->GetParamSelector()->Update(masterParams);

				OnViewSelection(pPanel->GetViewSelector());

				pPanel->SetViewSelector(masterSel, masterParams);
				{
					pPanel->GetViewSelector()->SetSelection(masterSel);
					OnViewSelection();
				}

				//UpdateImageView(pPanel, masterSel, masterParams, false);
			}
///////////////////////////////////////////////////////////////////////////////////////////////		
	/*{
		for (unsigned i = 0; i < m_panels.size(); ++i)
			if (m_panels[i] != pPanel)
				UpdateImageView(m_panels[i], viewIdx, params, false);
	}*/
	
void VideoParserWindow::SyncAllViews(VideoControlPanel* pMasterPanel, int viewIdx, 
										const DoubleArray& params)
{
	const int masterSel = pMasterPanel->GetViewSelector()->GetSelection();
	const DoubleArray masterParams = pMasterPanel->GetParamSelector()->GetValues();

	DBG_MSG1("Syncing views")

	for (unsigned i = 0; i = m_panels.size(); ++i)
	{
		VideoControlPanel* pPanel = m_panels[i];

		if (pPanel != pMasterPanel)
		{
			DBG_MSG2("Checking view", i)

			int sel = pPanel->GetViewSelector()->GetSelection();
			DoubleArray params =  pPanel->GetParamSelector()->GetValues();

			if (sel != masterSel || params != masterParams)
				UpdateImageView(pPanel, masterSel, masterParams);
		}
	}
}
///////////////////////////////////////////////////////////////////////////////////////////////		
	/*!
		Maps a global index associated to a component-output to the corresponding 
		local index of the output of a component.

		Note: use it to set DisplayInfoIn::outputIdx
	*/
	int GetOutputIndex(int idx) const
	{
		return GetOutputImageInfo(idx).index;
	}
///////////////////////////////////////////////////////////////////////////////////////////////		
	// Lock the mutex to let the GUI thread know that
	// there parsing thread is active
	m_userSyncMutex.Lock();

	ShowStatus("Processing Video");
	
	if (!m_video.Load(m_params.strFilename))
	{
		ShowError("Cannot read video");
		return;
	}

	//DBG_MSG2("Video length =", m_video.FrameCount())
	
	bool bPrevFrame = false;
	
	// Init all parsing variables
	ResetCurrentVideoData();

	unsigned prevMetaId, currMetaId, compDataId, videoMetaId;
	FrameParseMetadata prevFPM, currFPM;
	bool currFrameHasMetadata, success;

	if (m_params.saveParseData)
	{
		vpl::VideoParseMetadata vpm;

		videoMetaId = m_videoDataDB.Find(vpm, m_loadedVideoFilename);
		prevMetaId = INVALID_STORAGE_ID;

		// We need to find the medatada of the frame "previous" to the first
		// video frame to process, which may not exist
		if (videoMetaId != INVALID_STORAGE_ID && 
			vpm.storParentMetadataId != INVALID_STORAGE_ID)
		{
			currMetaId = vpm.storParentMetadataId;

			while (currMetaId != INVALID_STORAGE_ID)
			{
				success = m_videoDataDB.Load(currMetaId, currFPM);
				ASSERT(success);

				if (currFPM.frameNumber >= m_nFrame)
					break;

				prevMetaId = currMetaId;
				prevFPM = currFPM;
				currMetaId = currFPM.storParentMetadataId;
			}
		}

		// If prevMetaId is not an INVALID_STORAGE_ID, then we have the metadata of a previous frame,
		// although it might not necessarily be that of m_nFrame - 1.
	}

	Timer tic;

	for (ReadFirstFrame(); !IsLastFrame(); ReadNextFrame())
	{
		//m_video.PrintFrameInfo();

		// Update RGB version of the frame
		m_curRGBFrame = m_video.GetCurrentRGBFrame();
		
		// Update grey version of the frame
		m_curGreyFrame = m_video.GetCurrentGreyScaleFrame();
		
		// Check that everything went right
		if (m_curGreyFrame.size() == 0 || !m_curGreyFrame.is_contiguous())
		{
			ShowError("Error reading current frame");
			break;
		}
		
		// If we aren't just playing the video, we have work to do...
		if (!m_bJustPlayVideo)
		{
			ShowStatus2("\nProcessing frame", m_nFrame, "...");

			if (m_params.saveParseData)
			{
				if (currMetaId != INVALID_STORAGE_ID && currFPM.frameNumber == m_nFrame)
				{
					currFrameHasMetadata = true;
					m_pComponents->LoadFrameParseData(currFPM.storaDataId);
				}
				else
				{
					currFrameHasMetadata = false;
					m_pComponents->ClearFrameParseData();
				}
			}

			m_pComponents->ProcessNewFrame(m_video.FramePosition(),
				m_curRGBFrame, m_curGreyFrame);

			//ShowStatus2("Frame", m_nFrame, "is done");

			// See if we need to save the parsing data
			if (m_params.saveParseData && m_pComponents->HasNewFrameParseData())
			{
				// Save (or update) the current frame parse data
				compDataId = m_pComponents->SaveFrameParseData();

				ASSERT(!currFrameHasMetadata || currFPM.storDataId == compDataId);

				// Create a new metadata for the current frame if necessary
				if (!currFrameHasMetadata)
				{
					currFPM.SetStorageInfo(compDataId);
					currFPM.Set(m_nFrame);
					currMetaId = m_videoDataDB.Save(currFPM);
				}

				// If there is is no Video metadata yet, we save one
				// and link it to the current frame metadata
				if (videoMetaId == INVALID_STORAGE_ID)
				{
					// The VPM acts as the root of the metadata graph
					// but is stored as the only leaf of such a graph
					VideoParseMetadata vpm(m_loadedVideoFilename);
			
					// Make the current FPM the parent of the VPM
					vpm.SetStorageInfo(INVALID_STORAGE_ID, currMetaId);

					// Save the VPM
					videoMetaId = m_videoDataDB.Save(vpm);
				}

				// Update the link between the previous metadata and the current one if necesary
				if (prevMetaId != INVALID_STORAGE_ID && prevFPM.storParentMetadataId != currMetaId)
				{
					// Make the current FPM the parent of the previous FPM
					prevFPM.storParentMetadataId = currMetaId;

					// Update the previous FPM to reflect the change
					m_videoDataDB.Update(prevMetaId, prevFPM, true);
				}

				prevMetaId = currMetaId;
				prevFPM = currFPM;
			}
		}
		else // just play back the video at the correct frame rate
		{
			m_pComponents->ProcessPreviewFrame(m_curRGBFrame, m_curGreyFrame);
			
			// Delay the playback of the video if necessary
			if (m_nFrame != m_params.nFirstFrame)
			{ 
				//double deltaTime = 1000 * m_nFrame / m_video.FrameRate();
				double deltaTime = 1000 / m_video.FrameRate();
				double sleepTime = deltaTime - tic.ElapsedTime();

				if (sleepTime > 0)
				{
					//ShowStatus2("Sleeping for ", sleepTime, "milliseconds");
					Sleep((DWORD) sleepTime);
				} 
			}

			tic.Reset();
		}
		
		// ... and update the display to show it
		UpdateDisplay(!m_bJustPlayVideo, false);
		
		// Pause if requested. Use cvWaitKey(milliseconds) or sleep(seconds)
		while (m_bPauseParsing && !m_hasFrameRequest)
			Sleep(100); 
		
		// Stop if requested
		if (m_bStopParsing)
			break;
	}
	
	m_bParsingInProgress = false;
	
	ResetDashboard();

	// Release the mutex to let the GUI thread know that
	// there parsing thread is done
	m_userSyncMutex.Release();
///////////////////////////////////////////////////////////////////////////////////////////////		
template <typename T> 
void TemplateTesting(const T& x)
{
	std::cout << "x = " << x << ", ";
}

template <typename T> 
void TemplateTesting(const std::vector<T>& x)
{
	std::vector<T>::const_iterator it;

	std_fora (it = x.begin(); it != x.end(); ++it)
		TemplateTesting(*it);
}

std::vector< std::vector<int> > tmp(10, std::vector<int>(5, 2));

	TemplateTesting(tmp);
///////////////////////////////////////////////////////////////////////////////////////////////		
	gluTessCallback(tess, GLU_TESS_BEGIN,   glBegin);
	gluTessCallback(tess, GLU_TESS_END,     glEnd);
	gluTessCallback(tess, GLU_TESS_VERTEX,  glVertex3dv);
///////////////////////////////////////////////////////////////////////////////////////////////		
//ShowMsg("I'm drawing!");

	//glClearColor(1.0, 0.0, 0.0, 1.0);
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//glPushMatrix(); 

	glColor4f(0, 1.0, 0, 1.0);

	glBegin(GL_POINTS);
		//glVertex2f(0.0, 200.0);
		glVertex2f(200.0, 285.0);
		//glVertex2f(200.0, 400.0);
		//glVertex2f(0, 400.0);
	glEnd();

	gl_rectf(200, 200, 50, 50);

	/*glBegin(GL_POLYGON);
		glVertex2f(0.0, 200.0);
		glVertex2f(200.0, 200.0);
		glVertex2f(200.0, 400.0);
		glVertex2f(0, 400.0);
	glEnd();*/

	/*glBegin(GL_POLYGON);
		glVertex2f(180, 420.0);
		glVertex2f(220.0, 420.0);
		glVertex2f(220.0, 380.0);
		glVertex2f(180, 380.0);
	glEnd();*/

	//gl_draw(szTxt, 0, 200);

	gl_color(FL_BLACK);
	//glDisable(GL_DEPTH_TEST);
	gl_draw(szTxt, 200.0f, 200.0f);
	//glEnable(GL_DEPTH_TEST);

	//glPopMatrix();
///////////////////////////////////////////////////////////////////////////////////////////////		
if (HasBorrowedMemory(m_curByteImage)
			
		else
			m_curByteImage = imgPtr;
///////////////////////////////////////////////////////////////////////////////////////////////		
if (m_nFrame < m_params.nFirstFrame) 
		{
			m_nFrame++;
			continue;
		}
		else if (m_params.nLastFrame >= 0 && m_nFrame > m_params.nLastFrame)
		{
			break;
		}
		
		
	// Now we can say that we finished reading the new frame...
		m_nFrame++;
///////////////////////////////////////////////////////////////////////////////////////////////		
int n = m_video.FrameNumber();

			switch (fs)
			{
				case FIRST_FRAME: 
					m_video.ReadIndexedFrame(0); 
					break;
				//case LAST_FRAME: 
					//m_video.ReadFrame(-1); 
					//break;
				case ONE_FRAME_FORWARD: 
					m_video.ReadIndexedFrame(n + 1); 
					break;
				case ONE_FRAME_BACKWARD: 
					if (n > 0)
						m_video.ReadIndexedFrame(n - 1); 
					break;
				case ONE_SECOND_FORWARD: 
					m_video.ReadIndexedFrame((int)(n + m_video.FrameRate())); 
					break;
				case ONE_SECOND_BACKWARD:
					if (n >= m_video.FrameRate())
						m_video.ReadIndexedFrame((int)(n - m_video.FrameRate())); 
					break;
			}
///////////////////////////////////////////////////////////////////////////////////////////////		
		/*for (int n = 0; n < children(); n++)
		{
			if (!strcmp(child(n)->label(), "Main menue"))
			{
				//int offset = w() - child(n)->w();
				child(n)->size(dx, child(n)->h());
				break;
			}
		}*/
///////////////////////////////////////////////////////////////////////////////////////////////
//#include <FL/glu.H>

/*std::vector<GLUquadricObj*> g_quadrics;

void ClearQuadrics()
{
	for (unsigned int i = 0; i < g_quadrics.size(); i++)
		gluDeleteQuadric(g_quadrics[i]);

	g_quadrics.clear();
}*/

	/*g_quadrics.push_back(gluNewQuadric());

	glPushMatrix();
	
	glTranslated(c.x, c.y, 0);

	gluDisk(g_quadrics.back(), 0, radius, 16, 16);

	glPopMatrix();*/


///////////////////////////////////////////////////////////////////////////////////////////////
// VideoParserWindow::ParsingParams 

struct ParsingParams
	{
		vcl_string strFilename, strOutputDir;
		int nFirstFrame, nLastFrame;
		bool bSaveResults;
		
		ParsingParams();
		void operator=(const ParsingParams& rhs);
	};
	
VideoParserWindow::ParsingParams::ParsingParams() 
	: strFilename(FIRST_VIDEO_FNAME), strOutputDir(RESULTS_DIR)
{
	nFirstFrame = 0;
	nLastFrame = -1;
	bSaveResults = false;
}

void VideoParserWindow::ParsingParams::operator=(const ParsingParams& rhs)
{
	strFilename = rhs.strFilename;
	strOutputDir = rhs.strOutputDir;
	nFirstFrame = rhs.nFirstFrame;
	nLastFrame = rhs.nLastFrame;
	bSaveResults = rhs.bSaveResults;
	
	if (strOutputDir.length() > 0 && strOutputDir[strOutputDir.length() - 1] != '/')
		strOutputDir += "/";
}

///////////////////////////////////////////////////////////////////////////////////////////////
-----------------------------------------------------------------------------------------
/*RGBImg curRGBFrame;

	IplImageToVXLImage(m_curFrame, curRGBFrame);

	FImg curGreyFrame;

	vil_convert_planes_to_grey(curRGBFrame, curGreyFrame);

	return curGreyFrame;*/
-----------------------------------------------------------------------------------------
DBG_PRINT2(m_pBackgroundModel->background->nChannels, m_pBackgroundModel->background->depth)
		DBG_PRINT2(m_pBackgroundModel->foreground->nChannels, m_pBackgroundModel->foreground->depth)

		//cvShowImage("BG", m_pBackgroundModel->background);
        //cvShowImage("FG", m_pBackgroundModel->foreground);
-----------------------------------------------------------------------------------------
vil_image_view<vxl_byte> vxlImg;

	IplImageToVXLImage(m_curFrame, vxlImg);
	
		
	//RGBImg curRGBFrame = vxlImg;
	//FImg curFrame;// = vxlImg;

	//FImg curFrame = ConvertToGreyImage(vxlImg);

	//curFrame = vil_copy_deep(curFrame);
-----------------------------------------------------------------------------------------
	/*vil_image_view<vxl_byte> vxlImg;

	IplImageToVXLImage(m_curFrame, vxlImg);

	RGBImg curRGBFrame = vxlImg;

	return curRGBFrame;*/

-----------------------------------------------------------------------------------------
	if (m_pBackgroundModel)
	{
		cvReleaseBGStatModel(&m_pBackgroundModel);
		m_pBackgroundModel = NULL;
	}
-----------------------------------------------------------------------------------------
//IplImage* tmp_frame = NULL;

	//CvImage img;

	//VXLImageToIplImage(m_pDataStack->front().rgbFrame, img);

	//IplImageView iplView(m_pDataStack->front().greyFrame);

-----------------------------------------------------------------------------------------
class ByteIplImageView : public IplImageView, BImg
{
public:
	IplImageRGBView(RGBImg& img) : BImg(img)
	{
		CreateHeader((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_8U, 1);
	}

	operator const IplImage*() const { return m_pIplImage; }

    operator IplImage*() { return m_pIplImage; }
};

class FloatIplImageView : public IplImageView, FImg
{
public:
	IplImageRGBView(RGBImg& img) : FImg(img)
	{
		CreateHeader((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_32F, 1);
	}

	operator const IplImage*() const { return m_pIplImage; }

    operator IplImage*() { return m_pIplImage; }
};
-----------------------------------------------------------------------------------------
/*!
	Wrapper for an IplImage whose data is owned by a VXL image.

	It is similar to the CvImage class declared in cxcore.hpp. However,
	CvImage counts references to the wrapped IplImage, while ShallowIplImage
	is just a IplImage* and assumes that the *imageData is ownd by some
	VXL image. A ShallowIplImage object is only valid while the VXL image
	does not change.
*/
class IplImageView
{
	IplImage* m_pIplImage;

protected:
	void Create(char* imageData, int width, int height, 
		int depth, int channels)
	{
		CvSize sz;
		
		sz.height = height;
		sz.width = width;

		// Look at the VXL data with IPL eyes by creating a header for it
		IplImage* vxlData = cvCreateImageHeader(sz, depth, channels);
		vxlData->imageData = imageData;

		// Copy the image in IPL format (ie, with R and B swapped)
		m_pIplImage = cvCreateImage(sz, depth, channels);
		cvConvertImage(vxlData, m_pIplImage, CV_CVTIMG_SWAP_RB);

		// We are now done with the temporal header
		cvReleaseImageHeader(&vxlData);
	}

public:
	VXLIplImage(RGBImg& img)
	{
		Create((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_8U, 3);
	}

	VXLIplImage(BImg& img)
	{
		Create((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_8U, 1);
	}

	VXLIplImage(FImg& img)
	{
		Create((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_32F, 1);
	}

	~VXLIplImage()
	{
		cvReleaseImage(&m_pIplImage);
	}

	operator const IplImage*() const 
	{ 
		return m_pIplImage; 
	}

    operator IplImage*() 
	{ 
		return m_pIplImage; 
	}
};
-----------------------------------------------------------------------------------------
/*!
	Converts the given VXL data to a new IPL image.

	The VXL data can be obtained using img.top_left_ptr()

	Note: the returned IPL image pointer can be used to construct and
	object of the CvImage class (declared in cxcore.hpp), which
	is a reference-count wrapper for IPL images.
*/
IplImage* VXLImageToIplImage(char* pVXLImgData, int width, int height, 
		int depth, int channels)
{
	CvSize sz;
		
	sz.height = height;
	sz.width = width;

	// Look at the VXL data with IPL eyes by creating a header for it
	IplImage* tempIplImg = cvCreateImageHeader(sz, depth, channels);
	
	tempIplImg->imageData = pVXLImgData;

	// Copy the image in IPL format (ie, with R and B swapped)
	IplImage* newIplImage = cvCreateImage(sz, depth, channels);

	cvConvertImage(tempIplImg, newIplImage, CV_CVTIMG_SWAP_RB);

	// We are now done with the temporal header
	cvReleaseImageHeader(&tempIplImg);

	return newIplImage;
}
-----------------------------------------------------------------------------------------
/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IPL_IMAGE_HANDLE_H_
#define _IPL_IMAGE_HANDLE_H_

#include <cxcore.h>
#include "ImageUtils.h"

/*!
	Wrapper for an IplImage.

	It is similar to the CvImage class declared in cxcore.hpp. However,
	CvImage counts references to the wrapped IplImage, while IplImageHandle
	does not. This behavior can be obtained by using SharedPtr<IplImageHandle>.
*/
class IplImageHandle
{
	IplImage* m_pIplImage;
	bool m_releaseHeader;
	bool m_releaseImage;

public:
	IplImageHandle(bool releaseImage = true, bool releaseHeader = false)
	{
		m_pIplImage = NULL;
		SetReleaseMode(releaseImage, releaseHeader);
	}

	void SetReleaseMode(bool releaseImage, bool releaseHeader = false)
	{
		m_releaseImage = releaseImage;
		m_releaseHeader = releaseHeader;
	}

	void Release()
	{
		if (m_releaseHeaderAndData)
			cvReleaseImage(&m_pIplImage);
		else if (m_releaseHeader)
			cvReleaseImageHeader(&m_pIplImage);
	}

	~IplImageHandle()
	{
		Release();
	}

	void SetImage(IplImage* pIplImg, bool bCopy, bool bSwapRB, bool bFlipRows)
	{
		if (m_pIplImage)
			Release();

		if (bCopy)
		{
			CvSize sz;

			s.width = pIplImg->width;
			s.height = pIplImg->height;

			m_pIplImage = cvCreateImage(sz, pIplImg->depth, pIplImg->nChannels);
			
			SetReleaseMode(true); // owns the IPL image
		}
		else
		{
			m_pIplImage = pIplImg;

			// the realase mode depends on the constructor or
			// a call to SetReleaseMode()
		}

		if (bCopy || bSwapRB || bFlipRows)
		{
			int flipFlag = (bFlipRows) ? CV_CVTIMG_FLIP : 0;
			int swapFlag = (bSwapRB) ? CV_CVTIMG_SWAP_RB : 0;

			cvConvertImage(pIplImg, m_pIplImage, flipFlag + swapFlag);
		}
	}

	IplImageHandle(RGBImg& img)
	{
		CreateVXLWrapper((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_8U, 3);
	}

	IplImageHandle(BImg& img)
	{
		CreateVXLWrapper((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_8U, 1);
	}

	IplImageHandle(FImg& img)
	{
		CreateVXLWrapper((char*) img.top_left_ptr(), 
			img.ni(), img.nj(), IPL_DEPTH_32F, 1);
	}

	void CreateVXLWrapper(char* imageData, int width, int height, 
		int depth, int channels)
	{
		CvSize size;
		
		size.height = height;
		size.width = width;

		m_pIplImage = cvCreateImageHeader(size, depth, channels);

		SetReleaseMode(false, true);
	}
};

#endif // _IPL_IMAGE_HANDLE_H_
-----------------------------------------------------------------------------------------
VideoParserWindow::VideoParserWindow(int w, int h, const char *title)
	: Fl_Double_Window(w, h, title)
{
	m_nFrame = 0;
	m_bParsingInProgress = false;
	m_pComponents = new VSCGraph;

	cerr << "Constructor 1" << endl;
}

VideoParserWindow::VideoParserWindow(int x, int y, int w, int h, const char* title) 
	: Fl_Double_Window(x, y, w, h, title)
{

}
-----------------------------------------------------------------------------------------
//copy_image(p.warped, image1, nx, ny);
	//subtract_image(p.warped, image2, nx, ny);
-----------------------------------------------------------------------------------------
m_pDataStack->front().rgbFrame = rgbFrame;
m_pDataStack->front().greyFrame = greyFrame;


m_pDataStack->front().rgbFrame.deep_copy(rgbFrame);
	m_pDataStack->front().greyFrame.deep_copy(greyFrame);
-----------------------------------------------------------------------------------------
	/*VideoControlPanel* GetPanel(unsigned int i)
	{
		return (VideoControlPanel*)m_panels[i];
	}*/
-----------------------------------------------------------------------------------------
	~GRAPH()
	{
		node v;

		forall_nodes(v, *this)
			delete GetAttrNodeData(v);

		edge e;

		forall_edges(e, *this)
			delete GetAttrEdgeData(e);
	}
-----------------------------------------------------------------------------------------
std::list<NodeData>::iterator it = m_nodes.begin();

	std::cout << it->index;

	VSCPtr* pp = (VSCPtr*)(it->pData);

	std::cout << pp;
	
	/*for (v = (*this).m_nodes.begin(); v != (*this).m_nodes.end(); ++v)
	{
		//VSCPtr* pp = ((VSCPtr*)v->pData);
		//VSCPtr& ptr = *pp;

		std::cout << (*pp)->Active();

		//std::cout << operator[](v)->Active();
		//Component(v)->Initialize(m_pDataStack);
		DBG_MSG1("hello")
	}*/
-----------------------------------------------------------------------------------------
		AttrNodeData& GetAttrNodeData(node v)
	{
		try {
			return dynamic_cast<AttrNodeData&>(*v);
		}
		catch(std::bad_cast) {
			ShowError("Can't cast node\n");
			return AttrNodeData();
		}
	}

	AttrEdgeData& GetAttrEdgeData(edge e)
	{
		try {
			return dynamic_cast<AttrEdgeData&>(*e);
		}
		catch(std::bad_cast) {
			ShowError("Can't cast edge\n");
			return AttrEdgeData();
		}
	}
-----------------------------------------------------------------------------------------		
		
		/*AttrNodeData(const AttrNodeData& rhs)
		{
			operator=(rhs);
		}

		AttrNodeData& operator=(const AttrNodeData& rhs)
		{
			NodeData::operator=(rhs);

			pData = new V_TYPE(rhs.data());

			return *this;
		}*/
		
				//! Do nothing or have a virtual destructor
		~AttrNodeData() 
		{
			// Cannot delete pData because the pointer may be shared
			//delete &data();
		}
		
		~AttrEdgeData() 
		{ 
			// Cannot delete pData because the pointer may be shared
			//delete &data(); 
		}
-----------------------------------------------------------------------------------------
		//m_nodes.push_front(AttrNodeData(x));
		//return m_nodes.begin();
-----------------------------------------------------------------------------------------
/*!
	Sets the position and size of panel 'id' and those of its child
	widgets based on the size of the current frame 'm_curFrame'. 

	@return a pointer to the modified panel (Group object)
*/
Fl_Group* VideoParser::SetPanelSize(int id, int x)
{
	// Find the panel window
	const char* lbl;
	Fl_Group* pPanel = NULL;
	char szPanelID[20];

	sprintf(szPanelID, "Panel %d", id);
	
	for (int n = 0; n < children(); n++)
	{
		if ((lbl = child(n)->tooltip()) && !strcmp(lbl, szPanelID))
		{
			pPanel = (Fl_Group*)child(n);
			break;
		}
	}

	if (!pPanel)
	{
		ShowError1("Cannot find widget with tooltip: ", szPanelID);
		return NULL;
	}

	std::vector< std::pair<int, Fl_Widget*> > widgets;
	Fl_Widget* pWidget;
	int hightSum = 0;

	const int imgWidth = MAX(m_curFrame.ni(), 20);
	const int imgHight = MAX(m_curFrame.nj(), 20);

	for (int n = 0; n < pPanel->children(); n++)
	{
		pWidget = pPanel->child(n);
		widgets.push_back(std::make_pair(pWidget->y(), pWidget));
		hightSum += (n == 0) ? imgHight : pWidget->h();
	}

	// Set the width of the panel and all its children
	std::sort(widgets.begin(), widgets.end());

	int y = pPanel->y();
	const int spacing = WIDGET_V_SPACING;

	hightSum += spacing * widgets.size();

	pPanel->resize(x, y, imgWidth, hightSum);

	for (unsigned int i = 0; i < widgets.size(); i++)
	{
		pWidget = widgets[i].second;

		if (i == 0)
		{
			// This is the image view widget
			pWidget->resize(pPanel->x(), y, imgWidth, imgHight);
		}
		else
		{
			// Keep widget's hight
			pWidget->resize(pPanel->x() + WIDGET_H_MARGIN, y, 
				imgWidth - 2 * WIDGET_H_MARGIN, pWidget->h());
		}
		
		y += pWidget->h() + spacing;
	}

	return pPanel;
}

-----------------------------------------------------------------------------------------
//#include <unistd.h>


void SetViews(Fl_Window* view1, Fl_Window* view2)
	{
		m_outViews.resize(2);

		m_outViews[0] = view1;
		m_outViews[1] = view2;
	}


	Fl_Group* pPanel = pView->parent();
	ViewSelector* pSelector = NULL;
	const char* lbl;

	// Find selector widget
	for (int n = 0; n < pPanel->children(); n++)
	{
		if (child(n)->tooltip())
			DBG_MSG1(child(n)->tooltip())

		if ((lbl = child(n)->tooltip()) && !strcmp(lbl, "ViewSelector"))
		{
			pSelector = (ViewSelector*) pPanel->child(n);
			break;
		}
	}

	if (!pSelector)
	{
		ShowError("Cannot find selector widget");
		return BaseImgPtr();
	}
-----------------------------------------------------------------------------------------
/*#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Button.H>*/

	/*ImageView::VIEW_TYPE GetViewSelection() const 
	{
		if (mvalue())
			return (ImageView::VIEW_TYPE) (int) mvalue()->user_data();
		else
			return ImageView::CUR_FRAME;
	}*/
-----------------------------------------------------------------------------------------
/*#include <vxl_config.h>
#include <vil/vil_rgb.h>
#include <vil/vil_load.h>
#include <vil/vil_image_view.h>

#include <list>*/

//#include <Tools/SharedPtr.h>
#include <Tools/ImageUtils.h>

#include <FL/Fl.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

//#include <stdlib.h>

//typedef SharedPtr<Bone> BonePtr;
//typedef std::list<BonePtr> BoneList;
-----------------------------------------------------------------------------------------
imageView2->UpdateImage(o->GetViewSelection(), -1);
imageView1->UpdateImage(-1, o->value());
-----------------------------------------------------------------------------------------
	void UpdateSize();
	
/*!
	Sets the size of the image view using the size of
	the current image 'm_curImage'.
*/
void ImageView::UpdateSize()
{
	size(m_curImage.ni(), m_curImage.nj()); 
}

public:
	enum VIEW_TYPE {CUR_FRAME, PREV_FRAME, MEAN_IMG, WEIGHT_IMG, 
		OUTLIER_IMG, WARPED_IMG, USED_PTS_IMG, CUR_RGB_FRAME, SEGMENT_RGB_IMG,
		REGION_IMG};
-----------------------------------------------------------------------------------------
ImageView::ImageView(int x,int y,int w,int h,const char* l) 
	: BaseImageView(x, y, w, h, l)
{
#ifdef USE_OPEN_GL_WINDOW
	m_bUseOpenGL = true; // must always be true
#else
	m_bUseOpenGL = false; // may be true or false
#endif

	m_nViewType = CUR_FRAME;
	m_nMotion = 0;

	m_curImage = vil_convert_to_grey_using_average(vil_load("foo.ppm"));
	
	UpdateSize();

#ifndef USE_OPEN_GL_WINDOW
	// From man page: "Fl_Window is a subclass of Fl_Group so make sure your constructor 
	// calls end() unless you actually want children added to your window."
	end(); 
#endif
}
-----------------------------------------------------------------------------------------

void SetShowBones(bool bShowBones) { m_bShowBones = bShowBones; }
	void SetParamSelector(double curVal, double minVal, double maxVal, double step);
	
void ImageView::SetParamSelector(double curVal, double minVal, 
								 double maxVal, double step)
{
	// Find the valuator object
	Fl_Group* pParent = parent();
	const char* lbl;
	Fl_Valuator* pValuator = NULL;

	for (int n = 0; n < pParent->children(); n++)
	{
		if ((lbl = pParent->child(n)->tooltip()) && !strcmp(lbl, "Parameters"))
		{
			pValuator = (Fl_Valuator*)pParent->child(n);
			break;
		}
	}

	if (pValuator)
	{
		pValuator->bounds(minVal, maxVal);
		pValuator->step(step);
		pValuator->value(curVal);
		pValuator->redraw();
	}
	else
	{
		ShowError("Cannot find valuator widget");
	}
}
-----------------------------------------------------------------------------------------
bool bNewView = nNewViewType >= 0;
	bool bNewParam = dParamVal >= 0;

	// Update the view type if given
	if (bNewView)
		m_nViewType = (VIEW_TYPE)nNewViewType;
	
	// Deal with reading/updating parameter value according to view type
	if (m_nViewType == MEAN_IMG || m_nViewType == WEIGHT_IMG 
		|| m_nViewType == OUTLIER_IMG || m_nViewType == WARPED_IMG)
	{
		if (bNewParam) // store new value for current AND later use
			m_nMotion = (int) dParamVal;

		if (bNewView) // set appropriate values for new view
			SetParamSelector(m_nMotion, 0, 2, 1);
	}
	else if (!bNewParam) // param is not what changed; then, it must be updated
	{
		if (m_nViewType == SEGMENT_RGB_IMG)
		{
			dParamVal = pParentWnd->GetImgSegScale(0);

			if (bNewView)
				SetParamSelector(pParentWnd->GetImgSegScale(0), 0.1, 3, 0.05);
		}
		else if (bNewView)
		{
			SetParamSelector(0, 0, 0, 0);
		}
	}

	FImg newView;
	m_curRGBImage.clear(); // We update either curImage (newView) or curRGBImage
	
	MotionEstimator& me = *pParentWnd->MotionEst();
	
	switch (m_nViewType)
	{
		case CUR_RGB_FRAME:
			m_curRGBImage = pParentWnd->CurrentRGBFrame(); break;
		case SEGMENT_RGB_IMG:
			m_curRGBImage = pParentWnd->CurrentRGBSegmentation(dParamVal); break;
		case CUR_FRAME:
			newView = pParentWnd->CurrentFrame(); break;
		case REGION_IMG:
			newView = pParentWnd->CurrentRegion(); break;
		case MEAN_IMG:
			newView = me.GetMotionMap(m_nMotion, MotionMaps::MEAN); break;
		case WEIGHT_IMG:
			newView = me.GetMotionMap(m_nMotion, MotionMaps::WEIGHTS); break;
		case OUTLIER_IMG:
			newView = me.GetMotionMap(m_nMotion, MotionMaps::OUTLIERS); break;
		case USED_PTS_IMG:
			newView = me.GetMotionMap(m_nMotion, MotionMaps::USED_PTS); break;
		case WARPED_IMG:
			newView = me.GetMotionMap(m_nMotion, MotionMaps::WARPED); break;
		default:
			cerr << "Error: Invalid view type." << endl; break;
	}
	
	vil_convert_stretch_range(newView, m_curImage);
-----------------------------------------------------------------------------------------
#include <vil/algo/vil_gauss_filter.h>
#include <vil/algo/vil_orientations.h>

#include <vil/vil_new.h>
#include <vidl/vidl_clip.h>
#include <vil/vil_save.h>
#include <vil/vil_view_as.h>

-----------------------------------------------------------------------------------------
void VideoParser::FindStaticFeatures()
{
	ShowStatus("Finding static features...");

	/*cvCreateFGDStatModel( tmp_frame );
	cvUpdateBGStatModel( tmp_frame, bg_model );*/

	const unsigned int numSegs = m_pImgSegment->NumSegmentations();

	for (unsigned int i = 0; i < numSegs; i++)
	{
		//m_pImgSegment->SetScaleParameter(0, m_dCurSegScale); //0.65 is good
	}

	m_pImgSegment->Segment(m_curRGBFrame);
}

void VideoParser::FindMotionFeatures()
{
	ShowStatus("Finding motion features...");
	m_pMotionEst->Estimate(m_prevFrame, m_curFrame);
}

void VideoParser::CombineStaticAndMotionFeatures()
{
	/*RGBImg segImg = m_pImgSegment->Regions();
	RegionGraph rg;
	
	// Create a region graph from the given segmentation	
	rg.Create(segImg);
	
	// For visualization and debugging, we create a contour map weighted by motion outliers
	//CreateContourMap(m_contourMap, rg);
	//MultiplyContourMapByMotionOutliers();
	
	// Get the motion outliers
	int nMotion = 0;
	FImg motionOutlier = m_pMotionEst->GetMotionMap(nMotion, MotionMaps::OUTLIERS);
	
	////
	//FImg smoothedOutliers;	
	//vil_gauss_filter_2d(motionOutlier, smoothedOutliers, 1, 1);
	//motionOutlier = smoothedOutliers;
	////

	//CreateContourMap(m_contourMap, rg, 5);
	//rg.AccumulateContourVotes(m_curFrame, motionOutlier);
	//rg.PrintContourVotes();

	m_contourMap.set_size(rg.getCols(), rg.getRows());
	GenerateSyntheticOutliers();
	
////m_contourMap.set_size(rg.getCols(), rg.getRows());
////GenerativeModel genModel;
////genModel.ComputeMotionDependencies(rg);
////genModel.ShowRegionRelations();
////genModel.SetOutputImage(m_contourMap.top_left_ptr(), m_contourMap.nj(), m_contourMap.ni());
////genModel.GenerateOutputImage(rg);
	
	//GenerativeModel genModel;
	//int nFrameNum = 1; // TODO: chabge this
	//genModel.SetOutputImage(m_contourMap.top_left_ptr(), m_contourMap.nj(), m_contourMap.ni());
	//genModel.SetOutlierImage(motionOutlier);
	//genModel.SaveModelData(rg, "prefix", nFrameNum);
	
	//rg.Display();
	
	//rg.ComputeMerges();
	//CreateContourMap(m_contourMap, rg, -1);
	//rg.DrawCountourImage(m_curRGBFrame, 0);
	*/
}

void VideoParser::UpdatePolyboneModel()
{

}

/*void CreateContourMap(FImg& contourMap, RegionGraph& rg, int minArea)
{
	contourMap.set_size(rg.getCols(), rg.getRows());
	
	if (minArea >= 0)
		//rg.DrawCountourImage(contourMap.top_left_ptr(), contourMap.nj(), contourMap.ni(), minArea);
		rg.GenerateOutliersFromCountours(contourMap.top_left_ptr(), contourMap.nj(), contourMap.ni());
	//else
		//rg.DrawCountourImage(contourMap.top_left_ptr(), contourMap.nj(), contourMap.ni());
}*/

void VideoParser::MultiplyContourMapByMotionOutliers(bool bSmoothOutliers /*= false*/)
{
	int nMotion = 0;
	FImg outliers = m_pMotionEst->GetMotionMap(nMotion, MotionMaps::OUTLIERS);
	
	if (bSmoothOutliers)
	{
		FImg smoothedOutliers;	
		vil_gauss_filter_2d(outliers, smoothedOutliers, 2, 5);
		outliers = smoothedOutliers;
	}
	
	vil_math_image_product(m_contourMap, outliers, m_contourMap);
}

/*!
	Here we need to use the masks from the sythetic video to
	assign motion information to each region in the segmentation.
	The important thing is to realize the one mask may end up devided into
	multiple regions due to occlussion. Thise difines a non-to-many
	correspondence between masks and regions. Simce we know the motion of
	each mask, we must propagate this info to each corresponding region.
*/
void VideoParser::GenerateSyntheticOutliers()
{
	/*std::vector<MotionData> mdVec;
	RGBImg segImg = m_video.MakeRGBFrameFromMaskOnly(mdVec);

	// Create a region graph from the "mask segmentation"
	RegionGraph rg;
	rg.Create(segImg);

	// Assign motion data to each region from mask info
	leda::node v;
	PixelCoord pc;
	RGBColor color, white(255);
	
	forall_nodes(v, rg)
	{
		pc = rg.GetContourPoint(v, 0);
		color = segImg(pc.x(), pc.y());
		rg.SetMotion(v, (color == white) ? MotionData(0,0):mdVec[rgb2ind(color)]);
	}

	GenerativeModel genModel;
	genModel.ComputeMotionDependencies(rg);
	genModel.SetOutputImage(m_contourMap.top_left_ptr(), m_contourMap.nj(), m_contourMap.ni());
	genModel.GenerateOutputImage(rg);*/
}


-----------------------------------------------------------------------------------------
/*RGBImg m_curRGBFrame;
	FImg m_curGreyFrame;
	
	RGBImg m_prevRGBFrame;
	FImg m_prevGreyFrame;*/
	
		/*VideoParsingData() { }
	VideoParsingData(RGBImg rgbImg, FImg greyImg)
	{
		rgbFrame = rgbImg;
		greyFrame = greyImg;
	}*/
-----------------------------------------------------------------------------------------
// Save the member "current" frames as "previous" frames
	m_prevRGBFrame = m_curRGBFrame;
	m_prevGreyFrame = m_curGreyFrame;
	
	// Update RGB version of the frame
	m_curRGBFrame = rgbFrame;

	// Update grey version of the frame
	m_curGreyFrame = greyFrame;
-----------------------------------------------------------------------------------------
typedef leda::node VSCNode;
typedef leda::edge VSCEdge;

-----------------------------------------------------------------------------------------
if (!m_bJustPlayVideo)
		{
			sprintf(g_szTextBuffer, "Processing frame %d...", m_nFrame);
			ShowStatus(g_szTextBuffer);
			
			FindStaticFeatures();
			
			if (bPrevFrame)
			{
				FindMotionFeatures();
				CombineStaticAndMotionFeatures();
				UpdatePolyboneModel();
				StorePartialResults();
			}
			else
				bPrevFrame = true;
				
			sprintf(g_szTextBuffer, "Frame %d is done", m_nFrame);
			ShowStatus(g_szTextBuffer);
		}
-----------------------------------------------------------------------------------------
sprintf(g_szTextBuffer, "Video length =  %d", m_video.FrameCount());
	ShowStatus(g_szTextBuffer);
-----------------------------------------------------------------------------------------
VideoParser::VideoParser(int x,int y,int w,int h,const char* title) 
	: Fl_Double_Window(x, y, w, h, title)
{
	m_nFrame = 0;
	m_bParsingInProgress = false;
	m_pMotionEst = new AffineMotionEstimator;
	m_pImgSegment = new FHImageSegmentation(3);
}
-----------------------------------------------------------------------------------------
//#include <cvaux.h>
//#include <highgui.h>
//#include "RegionGraph.h"

//#include "RegionGraph.h"
//#include "GenerativeModel.h"

-----------------------------------------------------------------------------------------
//#include <VideoSegmentation/MotionEstimation/AffineMotionEstimator/AffineMotionEstimator.h>
//#include <ImageSegmentation/FHSegmenter/FHImageSegmentation.h>
//#include <Tools/ImageUtils.h>

MotionEstimator* MotionEst()    { return m_pMotionEst; }

	RGBImg CurrentRGBSegmentation(double scale) 
	{
		if (m_pImgSegment->GetScaleParameter(0) != scale)
		{
			m_pImgSegment->SetScaleParameter(0, scale);

			if (m_curRGBFrame.size() > 0)
				m_pImgSegment->Segment(m_curRGBFrame);
		}
 
		return m_pImgSegment->Regions(); 
	}
	
	FImg CurrentRegion()            { return m_contourMap; }
	
	
void FindStaticFeatures();
	void FindMotionFeatures();
	void CombineStaticAndMotionFeatures();
	void UpdatePolyboneModel();
	void GenerateSyntheticOutliers();
	
	void MultiplyContourMapByMotionOutliers(bool bSmoothOutliers = false);
	
	double GetImgSegScale(unsigned int i) const 
	{ 
		return m_pImgSegment->GetScaleParameter(i); 
	}

	void SetImgSegScale(unsigned int i, double scale) 
	{ 
		m_pImgSegment->SetScaleParameter(i, scale); 
	}
	
	

	void GetVisionComponentLabels(std::vector<std::string>& lbls) const
	{
		lbls.resize(m_vci.size());

		for (unsigned int i = 0; i < m_vci.size(); i++)
			lbls = m_vci[i].label;
	}

	void RegisterVisionComponent(const VisionComponentInfo& ci)
	{
		m_vci.push_back(ci);
	}
	
	MotionEstimator* m_pMotionEst;
	ImageSegmentation* m_pImgSegment;
	
	FImg m_mainRegion;
	FImg m_contourMap;
	
	Fl_Window* m_outView1;
	Fl_Window* m_outView2;
-----------------------------------------------------------------------------------------
class VSCHolder
{
public:
	VisSysComponent* pComp;
	bool active;
	
	VSCHolder(VisSysComponent* p = NULL) { pComp = p; }
	~VSCHolder() { delete pComp; }
};

	struct VisionComponentInfo
	{
		std::string label;
		int minval, maxVal, step;
	};
	
		std::vector<VisionComponentInfo> m_vci;
-----------------------------------------------------------------------------------------
struct CvGraphVertexFields 
{
	CV_GRAPH_VERTEX_FIELDS() //!< Inherit member variables from CvGraphVtx
};

struct CvGraphEdgeFields
{
	CV_GRAPH_EDGE_FIELDS() //!< Inherit memeber variables from CvGraphEdge
};

struct node
{
	CvGraphVertexFields* ptr;

	node(CvGraphVertexFields* p = NULL) { ptr = p; }

	operator const CvGraphVtx*() const { return ptr; }
	operator CvGraphVtx*() { return ptr; }
};

struct edge
{
	CvGraphEdgeFields* ptr;

	node(CvGraphEdgeFields* p = NULL) { ptr = p; }

	operator const CvGraphEdge*() const { return ptr; }
	operator CvGraphEdge*() { return ptr; }
};

CV_GRAPH_FIELDS() //!< Inherit memeber variables from CvGraph

struct NodeAttribute
	{
		CV_GRAPH_VERTEX_FIELDS() //!< Inherit member variables from CvGraphVtx
		V data;                  //!< Declare the additional node attribute data

		NodeAttribute(const V& v) : data(v) { };

		//operator const CvGraphVtx() const { return *this; }
		//operator CvGraphVtx() { return *this; }
	};

	struct EdgeAttribute
	{
		CV_GRAPH_EDGE_FIELDS() //!< Inherit memeber variables from CvGraphEdge
		E data;                //!< Declare the additional edge attribute data

		EdgeAttribute(const E& e) : data(e) { };

		//operator const CvGraphEdge() const { return *this; }
		//operator CvGraphEdge() { return *this; }
	};
	
	public:
	Graph(bool directed)
	{
		m_pCvGraph = cvCreateGraph(directed ? CV_ORIENTED_GRAPH : CV_GRAPH, 
			sizeof(CvGraph), sizeof(NodeAttribute), sizeof(EdgeAttribute),
			storage);
	}

-----------------------------------------------------------------------------------------
	/* add("Current frame", 0, 0, (void*)ImageView::CUR_FRAME);
	 add("Mean image", 0, 0, (void*)ImageView::MEAN_IMG);
	 add("Weights", 0, 0, (void*)ImageView::WEIGHT_IMG);
	 add("Inliers", 0, 0, (void*)ImageView::USED_PTS_IMG);
	 add("Outliers", 0, 0, (void*)ImageView::OUTLIER_IMG);
	 add("Warped", 0, 0, (void*)ImageView::WARPED_IMG);
	 add("Current RGB frame", 0, 0, (void*)ImageView::CUR_RGB_FRAME);
	 add("Image segmentation", 0, 0, (void*)ImageView::SEGMENT_RGB_IMG);
	 add("Region outlines", 0, 0, (void*)ImageView::REGION_IMG);*/
-----------------------------------------------------------------------------------------
/*Bone* pBone;
		
	pBone = new SuperQuadBone();
	pBone->SetPosition(140, 170, 0);
	pBone->SetSize(90, 40, 20);
	pBone->SetRotation(0, 0, 0);
	pBone->Update();
	m_bones.push_back(pBone);
	
	pBone = new SuperQuadBone();
	pBone->SetPosition(240, 190, 0);
	pBone->SetSize(40, 30, 20);
	pBone->SetRotation(0, 0, 30);
	pBone->Update();
	m_bones.push_back(pBone);
	
	pBone = new SuperQuadBone();
	pBone->SetPosition(280, 180, 0);
	pBone->SetSize(50, 20, 20);
	pBone->SetRotation(0, 0, 300);
	pBone->Update();
	m_bones.push_back(pBone);*/
-----------------------------------------------------------------------------------------
	//double scale = (width > 0) ? double(width) / m_curImage.ni() : 1;

	//DBG_PRINT2(width, scale)
	//DBG_PRINTL3("Updating size:", m_curImage.ni(), m_curImage.nj());
	
	// DIEGO "+1" is due to a bug somewhere
	//size(ROUND_NUM(m_curImage.ni() * scale), 
	//	 ROUND_NUM(m_curImage.nj() * scale + 1)); 
-----------------------------------------------------------------------------------------
		//((ImageView*)m_outView1)->UpdateSize();

		//if (bBothPanels)
		//	((ImageView*)m_outView2)->UpdateSize();
-----------------------------------------------------------------------------------------
int VideoParser::GetImagePanelWidth()
{
	const char* lbl;
	
	for (int n = 0; n < children(); n++)
	{
		if ((lbl = child(n)->label()))
		{
			DBG_PRINT1(lbl)
			if (!strcmp(lbl, "ImageView1"))
			{
				return ((Fl_Group*)child(n))->w();
			}
		}
	}

	return 0;
}
-----------------------------------------------------------------------------------------
int d, hightSum = 0;

	std::vector< std::pair<int, Fl_Widget*> > widgets;

	for (int n = 0; n < pPanel->children(); n++)
	{
		widgets.push_back(pPanel->child(n));
		//hightSum += widgets.back()->h();
	}

	std::sort(widgets.begin(), widgets.end());

	hightSum -= hight;

	int spacing = hightSum / (pPanel->children() - 1);
-----------------------------------------------------------------------------------------
// 	cerr << "width = " << m_curImage.ni() << ", hight = "
// 		<< m_curImage.nj() << ", planes = " << m_curImage.nplanes() 
// 		<< ", size = " << m_curImage.size()
// 		<< ", contiguous = " << m_curImage.is_contiguous() << endl;
// 		
// 	cerr << "width = " << m_curRGBImage.ni() << ", hight = "
// 		<< m_curRGBImage.nj() << ", planes = " << m_curRGBImage.nplanes() 
// 		<< ", size = " << m_curRGBImage.size()
// 		<< ", contiguous = " << m_curRGBImage.is_contiguous() << endl;
-----------------------------------------------------------------------------------------
	// Use black to clear the GL canvas
	//glClearColor(1.0, 0.0, 0.0, 1.0);
	
	//glShadeModel(GL_FLAT);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	//glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
-----------------------------------------------------------------------------------------
void ImageView::draw()
{	
	if (!valid()) 
	{
		glViewport(0, 0, w(), h());
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity(); 
		
		//glOrtho(0, w(), 0, h(), -1.0, 1.0);
		//glOrtho(0, w(), 0, h(), -20.0, 10.0);
		glOrtho(0, w(), 0, h(), -20000,10000);
		//glOrtho(-10,10,-10,10,-20000,10000);
		
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		
		// 3D drawing
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		
		// Lighting
		GLfloat model_ambient[] = {0.2, 0.2, 0.2, 1.0};
		glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
		
		//GLfloat light_position[] = {220, 180, -200, 1.0};
		GLfloat light_position[] = {0.0, 0.0, 1.0, 0};
		glLightfv(GL_LIGHT0, GL_POSITION, light_position);
		
		GLfloat light_ambient[] = {0.0, 0.0, 0.1, 1.0};
		glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
		
		//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
		
		glEnable(GL_LIGHTING);
		glEnable(GL_LIGHT0);
		//glEnable(GL_NORMALIZE);
		
		// Materials
		glEnable(GL_COLOR_MATERIAL);
		glColorMaterial(GL_FRONT, GL_DIFFUSE);
		//glColorMaterial(GL_FRONT, GL_SPECULAR);
	}
	
	// Use black to clear the GL canvas
	glClearColor(1.0, 0.0, 0.0, 1.0);
	
	glShadeModel(GL_FLAT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPixelZoom(1.0, -1.0);
	
// 	cerr << "width = " << m_curImage.ni() << ", hight = "
// 		<< m_curImage.nj() << ", planes = " << m_curImage.nplanes() 
// 		<< ", size = " << m_curImage.size()
// 		<< ", contiguous = " << m_curImage.is_contiguous() << endl;
// 		
// 	cerr << "width = " << m_curRGBImage.ni() << ", hight = "
// 		<< m_curRGBImage.nj() << ", planes = " << m_curRGBImage.nplanes() 
// 		<< ", size = " << m_curRGBImage.size()
// 		<< ", contiguous = " << m_curRGBImage.is_contiguous() << endl;

	glRasterPos2f(0,h());

	//DBG_PRINTL3("Drawing canvas size:", w(), h());
	//DBG_PRINTL3("Image size:", m_curImage.ni(), m_curImage.nj());
	
	if (m_curImage.is_contiguous() && m_curImage.size() > 0)
		glDrawPixels(m_curImage.ni(), m_curImage.nj(), GL_LUMINANCE,
			GL_UNSIGNED_BYTE, m_curImage.top_left_ptr());
	else if (m_curRGBImage.is_contiguous())
		glDrawPixels(m_curRGBImage.ni(), m_curRGBImage.nj(), GL_RGB,
			GL_UNSIGNED_BYTE, m_curRGBImage.top_left_ptr());
	//else
	//	glDrawPixels(curImage.ni(), curImage.nj(), GL_LUMINANCE,
	//		GL_FLOAT, curImage.top_left_ptr());
	
	if (m_bShowBones)
	{
		/*BoneList::iterator it;
		
		for (it = m_bones.begin(); it != m_bones.end(); it++)
			(*it)->Draw();*/
	}
	
	glFlush();
}

-----------------------------------------------------------------------------------------
//curFrame = vil_convert_to_grey_using_average(m_curImg, float());
//curFrame = vil_convert_stretch_range(float(),vil_convert_to_grey_using_average(m_curImg));

#define ConvertToGreyImage(I) vil_convert_cast(float(), vil_convert_to_grey_using_average(I))
//#define ConvertToGreyImage(I) vil_convert_to_grey_using_average(vil_convert_cast(float(),I))
-----------------------------------------------------------------------------------------
float min_b,max_b;
vil_math_value_range(newView,min_b,max_b);

std::cerr << "\nMot=" << m_nMotion << " Min=" << min_b << " Max=" << max_b << std::endl;