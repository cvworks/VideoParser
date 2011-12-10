/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ImageUtils.h"

/*!
	5 x 22 matrix of colors. It allows for moving along rows
	representing different color palettes.
	
	If a pastel color is requested but the index is out of bounds, we 
	use the RGB matrix instead. This helps deal with the problem 
	choosing unique colors when we run out of pastel colors.

	Moving along columns helps to yield colors that
	are different even though their indices are similar.
*/
class ColorMatrix
{
	struct ColorData
	{
		unsigned char r, g, b;

		void Set(unsigned char _r, unsigned char _g, unsigned char _b)
		{
			r = _r;
			g = _g;
			b = _b;
		}
	};

	static ColorData s_reds[];
	static ColorData s_greens[];
	static ColorData s_blues[];
	static ColorData s_browns[];
	static ColorData s_pastels[];

	static ColorData* s_matrix[];

	static int s_numRows;
	static int s_numColumns;

	ColorData m_color;
public:
	//! Set of color palletes (ordered like the rows of the matrix!!!)
	enum Type {REDS, GREENS, BLUES, BROWNS, PASTELS, RGB, ALL};

public:
	/*!
		Move along the requested row (color palette) and retrieve it's i-th color.
		
		If a pastel is requested and the index is out
		of bounds, we use the RGB matrix instead.

		Moving along rows helps to yield colors that
		are different even though their indices are similar.
	*/
	ColorMatrix(int i, Type t = RGB)
	{
		ASSERT(i >= 0);

		ASSERT((t == ALL && i < s_numRows * s_numColumns) || 
			   (t == RGB && i < 3 * s_numColumns) || 
			   (t <= BROWNS && i < s_numColumns) ||
			   (t == PASTELS && i < 3 * s_numColumns));

		// If a pastel color is requested, but the 
		// index is out of bounds, use the RGB matrix instead
		if (t == PASTELS && i >= s_numColumns)
			t = RGB;

		switch (t)
		{
			case REDS: m_color = s_reds[i]; break;
			case GREENS: m_color = s_greens[i]; break;
			case BLUES: m_color = s_blues[i]; break;
			case BROWNS: m_color = s_browns[i]; break;
			case PASTELS: m_color = s_pastels[i]; break;
			case RGB: m_color = s_matrix[i % 3][i / 3]; break;
			case ALL: m_color = s_matrix[i % s_numRows][i / s_numRows]; break;
			default: m_color.Set(255, 255, 255); break;
		}
	}

	operator const RGBColor() const 
	{ 
		return RGBColor(m_color.r, m_color.g, m_color.b); 
	}
};
