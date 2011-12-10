/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <FL/Fl_Value_Slider.h>

class ParameterSelectorOld : public Fl_Value_Slider
{
public:
	ParameterSelectorOld(int x,int y,int w,int h,const char *l=0)
		: Fl_Value_Slider(x, y, w, h, l) { }

	void Initialize()
	{
		value(0);
	}
	
	double GetValue() const { return value(); }

	void SetValue(const double& val) { value(val); }

	void Update(const double& val, const double& minVal, 
		const double& maxVal, const double& step)
	{
		bounds(minVal, maxVal);
		Fl_Value_Slider::step(step);
		value(val);
		redraw();
	}
};
