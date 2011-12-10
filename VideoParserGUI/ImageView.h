/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _IMAGEVIEW_H_
#define _IMAGEVIEW_H_

#include <FL/Fl.H>
#include <FL/gl.h>

#define USE_OPEN_GL_WINDOW 1

#ifdef USE_OPEN_GL_WINDOW
#include <FL/Fl_Gl_Window.H>
typedef Fl_Gl_Window BaseImageView;
#else
typedef Fl_Window BaseImageView;
#endif

#include <Tools/ImageUtils.h>
#include <Tools/BasicTypes.h>
#include <Tools/Mutex.h>
#include <Tools/VisSysComponent.h>

/*namespace vpl {
	class VisSysComponent;
}*/

class ImageView : public BaseImageView
{
protected:
	bool m_bUseOpenGL;        //!< Use OpenGL for drawing

	ByteImg m_curByteImage;   //!< Current Byte image drawn
	RGBImg m_curRGBImage;     //!< Current RGB image drawn
	std::string m_curImgInfo; //!< Current image information
	vpl::DisplaySpecs m_dispSpecs; //!< Display specs requested

	vpl::ConstVSCPtr m_pPainter;       //!< Specialized drawing is done by a VSC
	vpl::DisplayInfoIn m_painterInfo; //!< Output index and parameters for the current VSC

	int m_zoomStep;
	double m_zoom;
	double m_scaling;
	vpl::Vector2D m_offset, m_savedOffset;
	vpl::Point m_dragOrigin;
	//std::string m_savedTitle;
	bool m_showCoordinates;

	vpl::Mutex m_mutex;

	static double s_zoomStepFactor;
		
public:
	static void ReadParamsFromUserArguments();

	ImageView(int x,int y,int w,int h,const char *l = 0);

	void Initialize()
	{
		m_curImgInfo.clear();

		// Make sure that there is no current image painter
		RemovePainter();

		SetScaling(1);

		// Read (new) user arguments
		ReadParamsFromUserArguments();
	}

	//! Sets the overall pixel scaling
	void SetScaling(const double& s)
	{
		m_scaling = s;
		ResetZoom();
	}

	//! Sets the zoom params to match the current scaling
	void ResetZoom()
	{
		m_zoom = m_scaling;
		m_zoomStep = 0;
		m_offset.Set(0, 0);
	}

	/*!
		Sets the current vision system component that is called to
		draw over the current image.
	*/
	void UpdatePainter(vpl::ConstVSCPtr pComp, const vpl::DisplayInfoIn& dii)
	{
		m_pPainter = pComp;
		m_painterInfo = dii;
	}

	//! Removes the current vision system component used for "specialized" drawing
	void RemovePainter()
	{
		m_pPainter = NULL;
	}

	void UpdateImage(BaseImgPtr imgPtr, ImageType type, 
		const std::string& imageInfo, vpl::DisplaySpecs& specs);

	void GLDraw();
	void GLViewSetup();

	void RasterDraw();

	void draw()
	{
		(m_bUseOpenGL) ? GLDraw() : RasterDraw();
	}

	void DoKeyboardEvent(int keyCode)
	{
		OnKeyboard(keyCode);
	}

	void OnMouseMove();
	void OnMousePush();
	void OnMouseDrag();
	void OnZoom(int delta);
	void OnZoomAndDragReset();
	void OnKeyboard(int key);

	virtual int handle(int event);
};

#endif //_IMAGEVIEW_H_
