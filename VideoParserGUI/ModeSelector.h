/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <FL/Fl_Choice.H>
#include <Tools/BasicUtils.h>

class ModeSelector : public Fl_Choice
{
	std::vector<std::string> m_modeLabels;

public:
	ModeSelector(int x,int y,int w,int h,const char* l = 0)
		: Fl_Choice(x, y, w, h, l)
	{
	}

	void Initialize();
	
	std::string GetSelection() const
	{
		return m_modeLabels.at(value());
	}

	bool PlaybackMode() const
	{
		// The playback mode is always the last option
		return value() == m_modeLabels.size() - 1;
	}
};


