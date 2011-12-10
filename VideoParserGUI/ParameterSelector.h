/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <FL/Fl_Group.h>
#include <FL/Fl_Value_Slider.h>
#include <Tools/BasicUtils.h>
#include <Tools/STLUtils.h>

class ParameterSelector : public Fl_Group
{
	//std::vector<Fl_Value_Slider> m_sliders;

	Fl_Value_Slider* ChildSlider(int i)
	{
		return (Fl_Value_Slider*) child(i);
	}

	const Fl_Value_Slider* ChildSlider(int i) const
	{
		return (const Fl_Value_Slider*) child(i);
	}

	static void SliderCallback(Fl_Widget* o, void* v)
	{
		o->parent()->do_callback(o->parent(), (void*)o);
	}

public:
	ParameterSelector(int x, int y, int w, int h, const char* l=0)
		: Fl_Group(x, y, w, h, l) 
	{ 
		// Overwrite the box used in the GUI, which is selected only
		// for locating the widget by hand
		box(FL_FLAT_BOX);

		// Prevent new widgets from being "auto" added to the group
		current(0);
	}

	int NumSliders() const
	{
		return children();
	}

	void Initialize()
	{
		SetValue(0);
	}
	
	DoubleArray GetValues() const 
	{ 
		DoubleArray vals(NumSliders());

		for (unsigned i = 0; i < vals.size(); ++i)
			vals[i] = ChildSlider(i)->value();

		return vals; 
	}

	double GetValue(int i) const 
	{
		ASSERT(i >= 0 && i < NumSliders());

		return ChildSlider(i)->value();
	}

	double GetValue() const 
	{
		return GetValue(0);
	}

	void SetValues(const DoubleArray& vals) 
	{ 
		if (NumSliders() != vals.size())
		{
			// Delete all child widgets from memory
			clear();

			int x = Fl_Group::x();
			int y = Fl_Group::y();
			int w = (int)floor(Fl_Group::w() / double(vals.size()));
			int h = Fl_Group::h();

			Fl_Value_Slider* p;

			for (unsigned i = 0; i < vals.size(); ++i, x += w)
			{
				p = new Fl_Value_Slider(x, y, w, h, 0);
				p->align(FL_ALIGN_LEFT);
				p->type(FL_HOR_NICE_SLIDER); //FL_HORIZONTAL
				p->box(FL_FLAT_BOX);
				p->labeltype(FL_NO_LABEL);
				p->callback(SliderCallback);
				add(p);
			}
		}
		
		for (unsigned i = 0; i < vals.size(); ++i)
			ChildSlider(i)->value(vals[i]);
	}

	void SetValue(const double& val) 
	{ 
		SetValues(DoubleArray(1, val)); 
	}

	void Update(const DoubleArray& vals, const DoubleArray& minVals, 
		const DoubleArray& maxVals, const DoubleArray& steps)
	{
		// First set the new values, so that we have the
		// right number of sliders
		SetValues(vals);

		// Then, set the info for each slider
		Fl_Value_Slider* p;

		for (unsigned i = 0; i < vals.size(); ++i)
		{
			p = ChildSlider(i);

			p->bounds(minVals[i], maxVals[i]);
			p->step(steps[i]);
			//p->value(vals[i]);
			p->redraw();
		}
	}

	void Update(const double& val, const double& minVal, 
		const double& maxVal, const double& step)
	{
		Update(DoubleArray(1, val), DoubleArray(1, minVal), 
			DoubleArray(1, maxVal), DoubleArray(1, step));
	}
};
