/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/STLUtils.h>
#include <Tools/ImageUtils.h>
#include <ShapeRepresentation\FluxSkeleton\include\FluxPoint.h>

#define NUM_TO_STRING_BUFFER_SIZE 20

//#include <FL/gl.h>

//namespace vpl {
enum FONT_FAMILY {CURIER_9_FONT, CURIER_8_FONT, TIMES_ROMAN_10_FONT, 
	TIMES_ROMAN_24_FONT, HELVETICA_10_FONT, HELVETICA_12_FONT, HELVETICA_18_FONT};

enum TEXT_H_ALIGNMENT {LEFT_ALIGNMENT, CENTER_ALIGNMENT, RIGHT_ALIGNMENT};
enum TEXT_V_ALIGNMENT {TOP_ALIGNMENT, MIDDLE_ALIGNMENT, BOTTOM_ALIGNMENT};

typedef std::list< std::pair<vpl::Point, vpl::Point> > LineList;

int GetFontFace(int id, bool bold = false, bool italics = false);

void SetFont(int face, int size);

void* GetFont(FONT_FAMILY ff);

void SetDrawingColor(const RGBColor& c, const double& alpha = 1.0);
void SetDrawingColor(const RGBAColor& c);
void SetDefaultDrawingColor();

void DrawString(const char* szTxt, const vpl::Point& pt);

vpl::Point StringDrawingSize(const char* str, void* font);
void DrawString2D(const char* str, int x, int y, void* font);
void DrawString3D(const char* str, float pos[3], void* font);

void DrawNumber(int n, const vpl::Point& pt, 
	int maxNumDigits = NUM_TO_STRING_BUFFER_SIZE);

vpl::Point NumberDrawingSize(int n, void* font, 
	int maxNumDigits = NUM_TO_STRING_BUFFER_SIZE);

vpl::Point DrawNumberCentered(int n, const vpl::Point& pt, void* font, 
							  int maxNumDigits = NUM_TO_STRING_BUFFER_SIZE);

void DrawNumbersInBox(const std::vector<unsigned>& nums, const vpl::Point& tl, 
		      const vpl::Point& br, void* font, 
			  TEXT_H_ALIGNMENT hor_align, TEXT_V_ALIGNMENT ver_align, 
			  const char* szSep = ", ",
			  int maxNumDigits = NUM_TO_STRING_BUFFER_SIZE);

void DrawNumbersInBox(const std::vector<int>& nums, const vpl::Point& tl, 
		      const vpl::Point& br, void* font, 
			  TEXT_H_ALIGNMENT hor_align, TEXT_V_ALIGNMENT ver_align, 
			  const char* szSep = ", ",
			  int maxNumDigits = NUM_TO_STRING_BUFFER_SIZE);

void DrawPolygon(const PointList& pts);

void DrawFilledPolygon(const PointList& pts);

void DrawLineStrip(const sg::FluxPointArray& fpl);

void DrawLine(const vpl::Point& p0, const vpl::Point& p1);

void DrawLines(const LineList& ll, const double& endptRadius = 0.0);

void DrawCircle(const vpl::Point& center, const double& radius);

void DrawDisk(const vpl::Point& center, const double& radius);

void DrawSphere(const vpl::Point& center, const double& radius);

void DrawSquare(const vpl::Point& center, const double& sz);

void DrawRectangle(const vpl::Point& top_left, const vpl::Point& bottom_right);

void DrawFilledRectangle(const vpl::Point& top_left, const vpl::Point& bottom_right);

void DrawSelection(const vpl::Point& top_left, const vpl::Point& bottom_right,
	RGBColor color = RGBColor(153, 204, 255));

/*inline void MyDrawText(const char* szTxt, int x, int y)
{
	gl_draw(szTxt, x, y);
}*/

//} // namespace vpl
