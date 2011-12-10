/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <vil/vil_convert.h>
#include <FL/fl_draw.h> // Raster drawing functions
#include "VideoParserWindow.h"
#include "ImageView.h"
#include <iostream>
#include <Tools/VisSysComponent.h>
#include <Tools/UserArguments.h>
#include <Tools/UserEvents.h>

double ImageView::s_zoomStepFactor = 0.25;

extern vpl::UserArguments g_userArgs;

void ImageView::ReadParamsFromUserArguments()
{
	g_userArgs.ReadArg("GUI", "zoomStepFactor", 
		"Step factor used for zooming in and out", 
		0.25, &s_zoomStepFactor);

	g_userArgs.CheckMinValues("GUI", "zoomStepFactor", 0.01);
}

ImageView::ImageView(int x,int y,int w,int h,const char* l) 
	: BaseImageView(x, y, w, h, l)
{
#ifdef USE_OPEN_GL_WINDOW
	m_bUseOpenGL = true; // must always be true
#else
	m_bUseOpenGL = false; // may be true or false

	// From man page: "Fl_Window is a subclass of Fl_Group 
	// so make sure your constructor calls end() unless you 
	// actually want children added to your window."
	end(); 
#endif

	m_pPainter = NULL;
	m_showCoordinates = false;
}

/*!
	This function is designed to be run by a different thread 
	than Draw(), so we lock a mutex before changing the
	current images displayed.
*/
void ImageView::UpdateImage(BaseImgPtr imgPtr, ImageType type,
							const std::string& imageInfo,
							vpl::DisplaySpecs& specs)
{
	m_mutex.Lock();

	// We update either curImage or curRGBImage
	m_curByteImage.clear();
	m_curRGBImage.clear(); 
	m_curImgInfo = imageInfo;
	m_dispSpecs = specs;

	if (type == RGB_IMAGE)
	{
		m_curRGBImage.deep_copy(imgPtr);
	}
	else if (type == BYTE_IMAGE)
	{
		// We could more efficient and only copy the
		// image if its memory is borrowed (same for RGB). However,
		// it could happen that we need to redraw while the data is being changed
		// but before UpdateImage is called, and that may lead to strange outputs
		m_curByteImage.deep_copy(imgPtr);
	}
	else if (type == FLOAT_IMAGE)
	{
		FloatImg floatImg;

		floatImg.deep_copy(imgPtr);

		/*float min_b,max_b;
		
		vil_math_value_range(floatImg, min_b, max_b);

		DBG_PRINT2(min_b, max_b)*/

		vil_convert_stretch_range(floatImg, m_curByteImage);
	}
	else if (type == INT_IMAGE)
	{
		IntImg intImg;

		intImg.deep_copy(imgPtr);

		vil_convert_stretch_range(intImg, m_curByteImage);
	}
	else if (type == VOID_IMAGE)
	{
		//ShowError("The component has no image output");
	}
	else
	{
		ASSERT(false);
	}

	m_mutex.Release();

	parent()->label(m_curImgInfo.c_str());

	redraw();
}

void ImageView::RasterDraw()
{
	m_mutex.Lock();

	if (m_curByteImage.is_contiguous() && m_curByteImage.size() > 0)
	{
		fl_draw_image_mono(m_curByteImage.top_left_ptr(), 0, 0, 
			m_curByteImage.ni(), m_curByteImage.nj());
	}
	else if (m_curRGBImage.is_contiguous())
	{
		fl_draw_image((const unsigned char*) m_curRGBImage.top_left_ptr(), 0, 0, 
			m_curRGBImage.ni(), m_curRGBImage.nj());
	}

	m_mutex.Release();
}

/*!
	To set a valid raster position outside the viewport, first set a valid
    raster position inside the viewport, then call glBitmap with NULL
    as the bitmap parameter and with xmove and ymove set to
    the offsets of the new raster position. This technique is useful when
    panning an image around the viewport.
	(see http://www.opengl.org/sdk/docs/man/xhtml/glBitmap.xml and
	http://www.opengl.org/sdk/docs/man/xhtml/glDrawPixels.xml)
*/
void ImageView::GLDraw()
{	
#ifdef USE_OPEN_GL_WINDOW
	if (!valid())
		GLViewSetup();
#else
	gl_start();
#endif

	// Invert image
	//glPixelZoom(1.0f, -1.0f);
	glPixelZoom((float) m_scaling, (float) -m_scaling);

	// Set image origin
	glRasterPos2i(0, h());

	// Check that the raster position is valid
	GLboolean rasVal;

	glGetBooleanv(GL_CURRENT_RASTER_POSITION_VALID, &rasVal);

	if (!rasVal)
	{
		ShowError("Invalid raster position. No image will be shown");
	}

	// If image is scaled down, the view needs to be fully erased
	//if (m_zoom < 1)
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
	
	m_mutex.Lock();

	if (m_curByteImage.is_contiguous() && m_curByteImage.size() > 0)
		glDrawPixels(m_curByteImage.ni(), m_curByteImage.nj(), GL_LUMINANCE,
			GL_UNSIGNED_BYTE, m_curByteImage.top_left_ptr());
	else if (m_curRGBImage.is_contiguous())
		glDrawPixels(m_curRGBImage.ni(), m_curRGBImage.nj(), GL_RGB,
			GL_UNSIGNED_BYTE, m_curRGBImage.top_left_ptr());
	else
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_mutex.Release();

	// Call the current vision system component (if there is one)
	// to draw over the current image
	if (m_pPainter)
	{
		glPushMatrix();
		glTranslated(m_offset.x, m_offset.y + h(), 0);
		glScaled(m_zoom, -m_zoom, 1);
		glColor4f(0, 0, 0, 1.0);
		m_pPainter->DrawWithMutex(m_painterInfo);
		glPopMatrix();
	}

#ifdef USE_OPEN_GL_WINDOW
	glFlush();
#else
	gl_finish();
#endif
}

/*!
	Sets up projection, viewport, etc.
    Notes: window size is in w() and h();
	      valid() is turned on by FLTK after draw() returns.

*/
void ImageView::GLViewSetup()
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
	GLfloat model_ambient[] = {0.2F, 0.2F, 0.2F, 1.0F};
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, model_ambient);
	
	//GLfloat light_position[] = {220, 180, -200, 1.0};
	GLfloat light_position[] = {0.0, 0.0, 1.0, 0};
	glLightfv(GL_LIGHT0, GL_POSITION, light_position);
	
	GLfloat light_ambient[] = {0.0F, 0.0F, 0.1F, 1.0F};
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	
	//glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
	
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	//glEnable(GL_NORMALIZE);
	
	// Materials
	glEnable(GL_COLOR_MATERIAL);
	glColorMaterial(GL_FRONT, GL_DIFFUSE);
	//glColorMaterial(GL_FRONT, GL_SPECULAR);

	// Set the clear color to white (default is black)
	glClearColor(1.0, 1.0, 1.0, 1.0);
	gl_font(FL_HELVETICA_BOLD, 16 );
}

/*!
	When dealing with mouse coordinates, we need to convert them to
	image coordinates, and so must consider the following things:

	1) The mouse has the origin at top-left, but in all calculations we
	  assume an origin at bottom-left, so when reading the mouse, we do
	  mousePt.Set(Fl::event_x(), h() - Fl::event_y()).

    2) In general, the relation between the (bottom-left-origin) mouse point
	   $p_m$ and its corresponding image point $p_i$ is given by:
	   
	   \[ p_m = p_i * S + T \],

	   where S is the scaling and T is the translation. Then, given a mouse 
	   point we get the image point by

	   \[ p_i = (p_m - T) / S \]

	3) When zooming, we want to end up at the same image point,. Then, given a 
	   requested new scaling factor, $S'$, we must find find the appropriate 
	   translation $T'$ for it that will make the mouse point, $p_m$, correspond to
	   the same image point $p_i$.

	   \[ p_m = p_i * S' + T' \], which means that from (2), we can get

	   \[ T' = p_m - p_i * S' \]. We could replace $p_i$ and simplify things a bit, 
	   but that's not really necessary.

	4) In the case of the y coordinate, we have it reflexed (multiplied by -1) and 
	   translated by the window hight, h, to make it agree with the top-left origin 
	   of the displayed images. So, thes coordinate needs some special treatment. Then,
	   the image y-coordinate is given by

	   \[ p_i = (p_m - T - h) / -S \], and the new translation dy is given by

	   \[ dy = p_m - p_i * -S' - h \]. Agaim, some simplifycation could me made here,
	   but that's not necessary, and the point realtion are seen clearer this way.
*/
int ImageView::handle(int eventId)
{
	switch (eventId) 
	{
		// case FL_KEYDOWN: // does not work
		case FL_KEYUP:
			OnKeyboard(Fl::event_key());
			return 1;
		case FL_MOUSEWHEEL:
			OnZoom(Fl::event_dy());
			return 1;
		case FL_PUSH:
			OnMousePush();
			return 1;
		case FL_DRAG:
			OnMouseDrag();
			return 1;
		//case FL_ENTER:
		//	m_savedTitle = parent()->label();
		//	return 1;
		case FL_LEAVE:
			if (m_curImgInfo != parent()->label())
			{
				parent()->label(m_curImgInfo.c_str());
				parent()->redraw();
			}
			return 1;
		case FL_MOVE:
			OnMouseMove();
			return 1;
	}

	return BaseImageView::handle(eventId);
}

void ImageView::OnKeyboard(int key)
{
	// Treat all keys as lower case (note: isalpha wants 0 <= key <= 255)
	if (key >= 0 && key <= 255 && isalpha(key) && isupper(key))
		key = tolower(key);

	if (key == 'c')
	{
		m_showCoordinates = !m_showCoordinates;

		if (Fl::belowmouse() == this)
			OnMouseMove();
	}
	else if (key == '+' || key == '=')
	{
		OnZoom(-1);
	}
	else if (key == '-')
	{
		OnZoom(1);
	}
	else if (key == '0')
	{
		OnZoomAndDragReset();
	}
	else if (m_pPainter)
	{
		if (m_pPainter->UpdateDrawingState(m_painterInfo, key))
			redraw();
	}
}

/*!
	@param delta is a +/- offset provided by the function caller. e.g., it 
	may be obtained from the mouse wheel and then passed to this function.
*/
void ImageView::OnZoom(int delta)
{
	if (!m_dispSpecs.zoomableContent)
		return;

	vpl::Point mousePt, imgPt;
	double prevZoom;

	// Get "converted" mouse coordinates
	mousePt.Set(Fl::event_x(), h() - Fl::event_y());

	// Get previous and new zoom scaling from pixel scaling and zoom level
	prevZoom = m_scaling + s_zoomStepFactor * m_zoomStep;
	m_zoomStep -= delta;				
	m_zoom = m_scaling + s_zoomStepFactor * m_zoomStep;

	// Get image coordinate from mouse coordinate
	imgPt.x = (mousePt.x - m_offset.x) / prevZoom;
	imgPt.y = (mousePt.y - m_offset.y - h()) / -prevZoom;

	// Compute appropriate offset for new scaling
	m_offset.x = mousePt.x - imgPt.x * m_zoom;
	m_offset.y = mousePt.y - imgPt.y * -m_zoom - h();

	redraw();
}

void ImageView::OnZoomAndDragReset()
{
	ResetZoom();

	redraw();
}

void ImageView::OnMouseMove()
{
	static char szLbl[50];

	if (m_showCoordinates)
	{
		vpl::Point mousePt, imgPt;

		// Get image coordinate from mouse coordinate
		mousePt.Set(Fl::event_x(), h() - Fl::event_y());

		imgPt.x = (mousePt.x - m_offset.x) / m_zoom;
		imgPt.y = (mousePt.y - m_offset.y - h()) / -m_zoom;

		// Make and set the new window title
		sprintf(szLbl, "(%.2f,%.2f)", imgPt.x, imgPt.y);
		parent()->label(szLbl);
		parent()->redraw();
	}
}

void ImageView::OnMousePush()
{
	m_dragOrigin.Set(Fl::event_x(), h() - Fl::event_y());
	m_savedOffset = m_offset;

	if (m_pPainter)
	{
		vpl::UserEventInfo uei(m_painterInfo, 
			FL_PUSH, 
			Fl::event_button(), 
			vpl::Point(Fl::event_x(), Fl::event_y()));

		if (m_pPainter->OnGUIEvent(uei))
			redraw();
	}
}

void ImageView::OnMouseDrag()
{
	if (m_dispSpecs.draggableContent)
	{
		vpl::Point mousePt;

		mousePt.Set(Fl::event_x(), h() - Fl::event_y());
		m_offset = m_savedOffset + mousePt - m_dragOrigin;
	
		redraw();
	}
	else // do nothing, or send event to component, if there is one
	{
		if (m_pPainter)
		{
			vpl::UserEventInfo uei(m_painterInfo, 
				FL_DRAG, 
				Fl::event_button(), 
				vpl::Point(Fl::event_x(), Fl::event_y()));

			if (m_pPainter->OnGUIEvent(uei))
				redraw();
		}
	}
}


/*! 
	To set a valid raster position outside the viewport, first set a valid
    raster position inside the viewport, then call glBitmap with NULL
    as the bitmap parameter and with xmove and ymove set to
    the offsets of the new raster position. This technique is useful when
    panning an image around the viewport.
        

	From: http://www.mesa3d.org/brianp/sig97/gotchas.htm
	If glRasterPos() evaluates to a position outside of the viewport 
	the raster position becomes invalid. Subsequent glBitmap() and 
	glDrawPixels() calls will have no effect.

	Solution; extend the viewport beyond the window bounds or use 
	glBitmap() with an NULL bitmap and your desired delta X,Y movement 
	from the current, valid raster position. Be sure to restore the 
	viewport to a normal position before rendering other primitives.

	The following function will set the raster position to an 
	arbitrary window coordinate:
*/
/*void window_pos(GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   GLfloat fx, fy;

   // Push current matrix mode and viewport attributes
   glPushAttrib( GL_TRANSFORM_BIT | GL_VIEWPORT_BIT );

   // Setup projection parameters
   glMatrixMode( GL_PROJECTION );
   glPushMatrix();
   glLoadIdentity();
   glMatrixMode( GL_MODELVIEW );
   glPushMatrix();
   glLoadIdentity();

   glDepthRange( z, z );
   glViewport( (int) x - 1, (int) y - 1, 2, 2 );

   // set the raster (window) position
   fx = x - (int) x;
   fy = y - (int) y;
   glRasterPos4f( fx, fy, 0.0, w );

   // restore matrices, viewport and matrix mode
   glPopMatrix();
   glMatrixMode( GL_PROJECTION );
   glPopMatrix();

   glPopAttrib();
}*/
