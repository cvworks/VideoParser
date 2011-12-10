/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <FL/gl.h>
#include <FL/glu.H>
#include <FL/glut.H>
//#include <FL/fl_draw.h> 
#include "DrawingUtils.h"
#include <Tools/BasicUtils.h>
#include <Tools/Num2StrConverter.h>

#define DISK_RESOLUTION 32

#ifndef CALLBACK 
#define CALLBACK
#endif

const char* getPrimitiveType(GLenum type);

void Test0()
{
	GLdouble pts[3][3] = {{290.593, 184.646, 0},
		{303.082,240.44,0},
		{313.407,184.65,0}};

	glBegin(GL_TRIANGLE_FAN);

	for (int i = 0; i < 3; i++)
		glVertex3dv(pts[i]);

	glEnd();
}

void CALLBACK beginCallback(GLenum which)
{
	glBegin(which);

	//DBG_MSG1(getPrimitiveType(which))
}

void CALLBACK endCallback(void)
{
	glEnd();
}

void CALLBACK vertexCallback(GLvoid* p)
{
	glVertex2d(((vpl::Point*) p)->x, ((vpl::Point*) p)->y);

	//glVertex3dv((GLdouble*) vertex);

	//GLdouble* v = (GLdouble*) vertex;
	//std::cout << "\n{" << v[0] << ", " << v[1] << ", " << v[2] << "},";
}

void CALLBACK errorCallback(GLenum errorCode)
{
   const GLubyte *estring;

   estring = gluErrorString(errorCode);

   fprintf (stderr, "Tessellation Error: %s\n", estring);
}

void* GetFont(FONT_FAMILY ff)
{
	switch (ff)
	{
	case CURIER_9_FONT: return GLUT_BITMAP_9_BY_15;       
	case CURIER_8_FONT: return GLUT_BITMAP_8_BY_13;       
	case TIMES_ROMAN_10_FONT: return GLUT_BITMAP_TIMES_ROMAN_10;
	case TIMES_ROMAN_24_FONT: return GLUT_BITMAP_TIMES_ROMAN_24;
	case HELVETICA_10_FONT: return GLUT_BITMAP_HELVETICA_10;  
	case HELVETICA_12_FONT: return GLUT_BITMAP_HELVETICA_12;  
	case HELVETICA_18_FONT: return GLUT_BITMAP_HELVETICA_18;  
	}

	return NULL;
}

int GetFontFace(int id, bool bold, bool italics)
{
	int ff;

	switch (id)
	{
	case 0: ff = FL_HELVETICA; break;
	case 1: ff = FL_TIMES; break;
	case 2: ff = FL_COURIER; break;
	default: ff = 0;
	}

	if (bold)
		ff |= FL_BOLD;

	if (italics)
		ff |= FL_ITALIC;

	return ff;
}

void SetFont(int face, int size)
{
	gl_font(face, size);
}

/*!
	Converts enum of OpenGL primitive type to a string(char*)
	OpenGL supports only 10 primitive types.
*/
const char* getPrimitiveType(GLenum type)
{
    switch(type)
    {
    case 0x0000:
        return "GL_POINTS";
        break;
    case 0x0001:
        return "GL_LINES";
        break;
    case 0x0002:
        return "GL_LINE_LOOP";
        break;
    case 0x0003:
        return "GL_LINE_STRIP";
        break;
    case 0x0004:
        return "GL_TRIANGLES";
        break;
    case 0x0005:
        return "GL_TRIANGLE_STRIP";
        break;
    case 0x0006:
        return "GL_TRIANGLE_FAN";
        break;
    case 0x0007:
        return "GL_QUADS";
        break;
    case 0x0008:
        return "GL_QUAD_STRIP";
        break;
    case 0x0009:
        return "GL_POLYGON";
        break;
    }

	return "invalid primitive";
}

/*!
	Draws a sphere using the gluDisk function with style GLU_SILHOUETTE
*/
void DrawCircle(const vpl::Point& c, const double& radius)
{
	GLUquadricObj* q = gluNewQuadric();

	gluQuadricDrawStyle(q, GLU_SILHOUETTE);

	glPushMatrix();
	glTranslated(c.x, c.y, 0);
	gluDisk(q, 0, radius, DISK_RESOLUTION, DISK_RESOLUTION);
	glPopMatrix();

	gluDeleteQuadric(q);
}

/*!
	Draws a sphere using the gluDisk function with the style GLU_FILL
*/
void DrawDisk(const vpl::Point& c, const double& radius)
{
	GLUquadricObj* q = gluNewQuadric();

	gluQuadricDrawStyle(q, GLU_FILL);

	glPushMatrix();
	glTranslated(c.x, c.y, 0);
	gluDisk(q, 0, radius, DISK_RESOLUTION, DISK_RESOLUTION);
	glPopMatrix();

	gluDeleteQuadric(q);
}

/*!
	Draws a sphere using glut instead of glu because glut
	doesn't need a quadric object.
*/
void DrawSphere(const vpl::Point& c, const double& radius)
{
	glPushMatrix();
	glTranslated(c.x, c.y, 0);
	glutSolidSphere(radius, DISK_RESOLUTION, DISK_RESOLUTION);
	glPopMatrix();
}

void SetDefaultDrawingColor()
{
	glColor4f(0, 0, 0, 1.0);
}

void SetDrawingColor(const RGBColor& c, const double& alpha)
{
	glColor4d(c.R() / 255.0, c.G() / 255.0, c.B() / 255.0, alpha);
}

void SetDrawingColor(const RGBAColor& c)
{
	glColor4ub(c.R(), c.G(), c.B(), c.A());
}

void DrawPolygon(const PointList& pts)
{
	PointList::const_iterator it;

	glBegin(GL_LINE_LOOP);
		for (it = pts.begin(); it != pts.end(); ++it)
			glVertex2d(it->x, it->y);
	glEnd();
}

void DrawFilledPolygon(const PointList& pts)
{
	// create tessellator
	GLUtesselator* tess = gluNewTess();

	// register callback functions
	gluTessCallback(tess, GLU_TESS_BEGIN,  (GLvoid (CALLBACK*)()) beginCallback);
	gluTessCallback(tess, GLU_TESS_END,    (GLvoid (CALLBACK*)()) endCallback);
	gluTessCallback(tess, GLU_TESS_VERTEX, (GLvoid (CALLBACK*)()) vertexCallback);
	gluTessCallback(tess, GLU_TESS_ERROR,  (GLvoid (CALLBACK*)()) errorCallback);

	PointList::const_iterator it;
	GLdouble coords[3];

	coords[2] = 0;

	// describe non-convex polygon
	gluTessBeginPolygon(tess, NULL);

		// first contour
		gluTessBeginContour(tess);

			for (it = pts.begin(); it != pts.end(); ++it)
			{
				coords[0] = it->x;
				coords[1] = it->y;

				gluTessVertex(tess, coords, (void*)&(*it));
			}

		gluTessEndContour(tess);

	gluTessEndPolygon(tess);

	// delete tessellator after processing
	gluDeleteTess(tess);
}

void DrawLineStrip(const sg::FluxPointArray& fpl)
{
	sg::FluxPointArray::const_iterator it;

	glBegin(GL_LINE_STRIP);
		for (it = fpl.begin(); it != fpl.end(); ++it)
			glVertex2d(it->p.x, it->p.y);
	glEnd();
}

void DrawLines(const LineList& ll, const double& endptRadius)
{
	glBegin(GL_LINES);
		for (auto it = ll.begin(); it != ll.end(); ++it)
		{
			glVertex2d(it->first.x, it->first.y);
			glVertex2d(it->second.x, it->second.y);
		}
	glEnd();

	if (endptRadius > 0)
	{
		for (auto it = ll.begin(); it != ll.end(); ++it)
		{
			DrawDisk(it->first, endptRadius);
			DrawDisk(it->second, endptRadius);
		}
	}
}

void DrawLine(const vpl::Point& p0, const vpl::Point& p1)
{
	glBegin(GL_LINES);
		glVertex2d(p0.x, p0.y);
		glVertex2d(p1.x, p1.y);
	glEnd();
}

void DrawString(const char* szTxt, const vpl::Point& pt)
{
	gl_draw(szTxt, (float)pt.x, (float)pt.y);
}

void DrawNumber(int n, const vpl::Point& pt, int maxNumDigits)
{
	Num2StrConverter sc(maxNumDigits);

	DrawString(sc.toCharPtr(n), pt);
}

/*!
	Returns the size of drawing the string representing
	the given number.
*/
vpl::Point NumberDrawingSize(int n, void* font, int maxNumDigits)
{
	Num2StrConverter sc(maxNumDigits);

	return StringDrawingSize(sc.toCharPtr(n), font);
}

/*!
	Draws a number centered at the given point.

	@return the size of the string drawn
*/
vpl::Point DrawNumberCentered(int n, const vpl::Point& pt, 
							  void* font, int maxNumDigits)
{
	Num2StrConverter sc(maxNumDigits);

	vpl::Point sz = StringDrawingSize(sc.toCharPtr(n), font);

	int dx = (int)(pt.x - sz.x * 0.5);
	int dy = (int)(pt.y + sz.y * 0.25);

	DrawString2D(sc.toCharPtr(n), dx, dy, font);

	return sz;
}

/*!
	Draws a number within a rectangle specified by its top-left 
	and bottom-right corners.

	@parm maxNumDigits is used to create a buffer to hold *each* number.
*/
void DrawNumbersInBox(const std::vector<unsigned>& nums, const vpl::Point& tl, 
					  const vpl::Point& br, void* font, 
					  TEXT_H_ALIGNMENT hor_align, TEXT_V_ALIGNMENT ver_align,
					  const char* szSep, int maxNumDigits)
{
	std::vector<int> signedNums;

	signedNums.assign(nums.begin(), nums.end());

	DrawNumbersInBox(signedNums, tl, br, font, hor_align, 
		ver_align, szSep, maxNumDigits);
}

/*!
	Draws a number within a rectangle specified by its top-left 
	and bottom-right corners.

	@parm maxNumDigits is used to create a buffer to hold *each* number.
*/
void DrawNumbersInBox(const std::vector<int>& nums, const vpl::Point& tl, 
					  const vpl::Point& br, void* font, 
					  TEXT_H_ALIGNMENT hor_align, TEXT_V_ALIGNMENT ver_align,
					  const char* szSep, int maxNumDigits)
{
	const vpl::Point sepSize = StringDrawingSize(szSep, font);
	std::vector<vpl::Point> szs(nums.size());
	Num2StrConverter sc(maxNumDigits);
	std::vector<unsigned> lastIdxs;
	
	lastIdxs.reserve(nums.size());

	const double dx = br.x - tl.x;
	const double dy = br.y - tl.y;

	double cumSz = 0;
	double maxCumSz = 0;

	for (unsigned i = 0; i < nums.size(); ++i)
	{
		szs[i] = StringDrawingSize(sc.toCharPtr(nums[i]), font);
		
		// theres is at least one number per line
		if (cumSz > 0 && cumSz + szs[i].x > dx)
		{
			lastIdxs.push_back(i);
			cumSz = szs[i].x + sepSize.x;
		}
		else
		{
			cumSz += szs[i].x + sepSize.x;
		}

		if (cumSz - sepSize.x > maxCumSz)
			maxCumSz = cumSz - sepSize.x;
	}

	lastIdxs.push_back(nums.size());

	vpl::Point p0, sz;
	double left, top;
	
	switch (hor_align)
	{
	case LEFT_ALIGNMENT: left = 0; break;
	case CENTER_ALIGNMENT: left = (dx - maxCumSz) / 2.0; break;
	case RIGHT_ALIGNMENT: left = dx - maxCumSz; break;
	}

	switch (ver_align)
	{
	case TOP_ALIGNMENT: top = 0; break;
	case MIDDLE_ALIGNMENT: top = (dy - sepSize.y * lastIdxs.size()) / 2.0; break;
	case BOTTOM_ALIGNMENT: top = dy - sepSize.y * lastIdxs.size(); break;
	}

	if (top < 0)
		top = 0;

	p0.x = tl.x + left;
	p0.y = tl.y + top + sepSize.y * 0.75;

	std::stringstream ss;

	for (unsigned i = 0, k = 0; k < lastIdxs.size(); ++k)
	{
		ss.clear();

		for (; i < lastIdxs[k]; ++i)
		{
			ss << nums[i];

			if (i + 1 < lastIdxs[k])
				ss << ", ";
		}

		DrawString2D(ss.str().c_str(), (int)p0.x, (int)p0.y, font);

		p0.y += sepSize.y;
	}
}

void DrawSquare(const vpl::Point& c, const double& sz)
{
	double d = sz / 2;

	glBegin(GL_LINE_LOOP);
		glVertex2d(c.x - d, c.y + d);
		glVertex2d(c.x + d, c.y + d);
		glVertex2d(c.x + d, c.y - d);
		glVertex2d(c.x - d, c.y - d);
	glEnd();
}

/*!
	Computes the width and hight of the string as if it were drawn.

	@return the width and hight in pixels of the string
*/
vpl::Point StringDrawingSize(const char* str, void* font)
{
	vpl::Point sz(0, 0);

    // loop all characters in the string
    /*while(*str)
    {
		sz.x += glutBitmapWidth(font, *str);
        ++str;
    }*/

	sz.x = glutBitmapLength(font, (const unsigned char*)str);
	sz.y = glutBitmapHeight(font);

	return sz;
}

/*!
	Writes 2d text using GLUT.

	The projection matrix must be set to orthogonal before call this function.
*/
void DrawString2D(const char* str, int x, int y, void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glRasterPos2i(x, y);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

/*!
	Draws a string in 3D space.
*/
void DrawString3D(const char* str, float pos[3], void* font)
{
    glPushAttrib(GL_LIGHTING_BIT | GL_CURRENT_BIT); // lighting and color mask
    glDisable(GL_LIGHTING);     // need to disable lighting for proper text color

    glRasterPos3fv(pos);        // place text position

    // loop all characters in the string
    while(*str)
    {
        glutBitmapCharacter(font, *str);
        ++str;
    }

    glEnable(GL_LIGHTING);
    glPopAttrib();
}

/*!
	Draws a rectangle.
*/
void DrawRectangle(const vpl::Point& top_left, const vpl::Point& bottom_right)
{
	PointList pts;

	pts.push_back(top_left);
	pts.push_back(vpl::Point(bottom_right.x, top_left.y));
	pts.push_back(bottom_right);
	pts.push_back(vpl::Point(top_left.x, bottom_right.y));

	DrawPolygon(pts);
}

/*!
	Draws a filled rectangle.
*/
void DrawFilledRectangle(const vpl::Point& top_left, const vpl::Point& bottom_right)
{
	PointList pts;

	pts.push_back(top_left);
	pts.push_back(vpl::Point(bottom_right.x, top_left.y));
	pts.push_back(bottom_right);
	pts.push_back(vpl::Point(top_left.x, bottom_right.y));

	DrawFilledPolygon(pts);
}

void DrawSelection(const vpl::Point& top_left, const vpl::Point& bottom_right,
	RGBColor color)
{
	PointList pts;

	pts.push_back(top_left);
	pts.push_back(vpl::Point(bottom_right.x, top_left.y));
	pts.push_back(bottom_right);
	pts.push_back(vpl::Point(top_left.x, bottom_right.y));

	SetDrawingColor(color, 0.5);
	DrawFilledPolygon(pts);

	SetDrawingColor(color);
	DrawPolygon(pts);

	SetDefaultDrawingColor();
}
