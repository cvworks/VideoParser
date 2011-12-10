/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _NAMED_COLOR_H_
#define _NAMED_COLOR_H_

//#include <map>
#include <string>
#include "ImageUtils.h"

/*!
	Creates an RGB color by name. For example, soma valid names are:

	"Black", "Navy", "Chocolate", "Ivory", etc.

	@see Documentation/color%20palette.html
*/
class NamedColor
{
	struct ColorData
	{
		const char* name;
		unsigned char r, g, b;
	};

	static ColorData s_palette[];
	
	const ColorData* m_pColor;

public:

	NamedColor(const char* name)
	{
		for (m_pColor = s_palette; m_pColor->name != NULL; ++m_pColor)
			if (strcmp(name, m_pColor->name) == 0)
				return;

		m_pColor = NULL;
	}

	NamedColor(int idx)
	{
		ASSERT(idx < NumberOfColors());

		m_pColor = &s_palette[idx];
	}

	int NumberOfColors() const
	{
		int i;

		for (i = 0; s_palette[i].name != NULL; ++i);

		return i;
	}

	operator const RGBColor() const 
	{ 
		if (m_pColor)
			return RGBColor(m_pColor->r, m_pColor->g, m_pColor->b);
		else
			return RGBColor(0, 0, 0); 
	}
};

#endif // _NAMED_COLOR_H_