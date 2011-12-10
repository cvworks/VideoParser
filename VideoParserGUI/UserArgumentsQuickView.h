/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <FL/Fl_Group.h>
#include <Tools/BasicUtils.h>
#include <Tools/STLUtils.h>
#include <Tools/Tuple.h>
#include <Tools/ZTString.h>

/*!
	Monitors a selected set of user arguments and displays
	them in the GUI.
*/
class UserArgumentsQuickView : public Fl_Group
{
protected:
	enum WIDGET_TYPE {BOOL_WIDGET, STRING_WIDGET};

	struct WidgetParams 
	{
		typedef vpl::Tuple<std::string, 4> Params;

		Params params;
		ZTString label;
		int x, y, dx, dy;
		WIDGET_TYPE type;

		const std::string& FieldKeyParam() const { return params[0]; }
		const std::string& PropKeyParam() const  { return params[1]; }
		const std::string& TypeParam() const     { return params[2]; }
		const std::string& LabelParam() const    { return params[3]; }
			
		WidgetParams() { }

		WidgetParams(const Params& p)
		{
			// Copy given params
			params = p;
			
			// Convert lbl to local zero-term string
			label = p[3]; 

			// Convert string type to numeric type
			type = ConvertWidgetType(p[2]);
		}

		static WIDGET_TYPE ConvertWidgetType(const std::string& strType)
		{
			if (strType == "bool")
				return BOOL_WIDGET;
			else if (strType == "string")
				return STRING_WIDGET;

			return STRING_WIDGET;
		}

		WidgetParams(const WidgetParams& rhs)
		{
			operator=(rhs);
		}

		void operator=(const WidgetParams& rhs)
		{
			params = rhs.params;
			label = rhs.label;
			x = rhs.x;
			y = rhs.y;
			dx = rhs.dx;
			dy = rhs.dy;
			type = rhs.type;
		}
	};

	typedef std::list<WidgetParams> WidgetParamList;

	WidgetParamList m_widgetParams;

	void AddBoolWidget(WidgetParamList::iterator it);
	void AddStringWidget(WidgetParamList::iterator it);

protected:
	Fl_Widget* ChildWidget(int i)
	{
		return child(i);
	}

	const Fl_Widget* ChildWidget(int i) const
	{
		return child(i);
	}

	static void ChildWidgetCallback(Fl_Widget* o, void* v);
	
public:
	UserArgumentsQuickView(int x, int y, int w, int h, const char* l=0)
		: Fl_Group(x, y, w, h, l) 
	{ 
		// Overwrite the box used in the GUI, which is selected only
		// for locating the widget by hand
		box(FL_NO_BOX);

		// Prevent new widgets from being "auto" added to the group
		current(0);
	}

	void ReadParamsFromUserArguments();

	int NumWidgets() const
	{
		return children();
	}

	void Initialize()
	{
		// Overwrite the box used in the GUI, which is selected only
		// for locating the widget by hand
		box(FL_NO_BOX);

		// Delete all child widgets from memory
		clear();

		// Delete current widget params
		m_widgetParams.clear();
		
		// Read the list of arguments to show from the user arguments
		ReadParamsFromUserArguments();

		for (auto it = m_widgetParams.begin(); it != m_widgetParams.end(); ++it)
		{
			if (it->type == BOOL_WIDGET)
				AddBoolWidget(it);
			else if (it->type == STRING_WIDGET)
				AddStringWidget(it);
		}

		parent()->redraw();
	}

	/*void AddBoolArgument(const std::string& fieldKey, 
		const std::string& propKey, const char* szDisplayLabel);

	void AddStringArgument(const std::string& fieldKey, 
		const std::string& propKey, const char* szDisplayLabel);*/
};
