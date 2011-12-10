/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "UserArgumentsQuickView.h"
#include <Tools/UserArguments.h>
#include <FL/Fl_Window.h>
#include <FL/Fl_Check_Button.h>
#include <FL/Fl_Output.h>

#define HSPACE 10

extern vpl::UserArguments g_userArgs;

void UserArgumentsQuickView::ReadParamsFromUserArguments()
{
	std::list<WidgetParams::Params> defVal;
	WidgetParams::Params t;

	// Set the default values for each widget
	std::istringstream ss(
		"{VideoProcessor,cacheParseData,bool,cache}");

	while (!ss.eof())
	{
		try {
			ss >> t;
			defVal.push_back(t);
		}
		catch(...)
		{
			break;
		}
	}

	// Read the widget parameters specified by the user. If there aren't any,
	// the dafault values are used
	std::list<WidgetParams::Params> widgetParams;
	
	g_userArgs.ReadArgs("QuickArgumentView", "arguments", 
		"Tuples specifying which argumens must be displayed", 
		defVal, &widgetParams);

	for (auto it = widgetParams.begin(); it != widgetParams.end(); ++it)
		m_widgetParams.push_back(WidgetParams(*it));

	/*std::ostringstream oss;

	for (auto it = m_widgetParams.begin(); it != m_widgetParams.end(); ++it)
	{
		oss << it->params << "--";
	}

	oss << '.';

	DBG_PRINT1(oss.str())*/
}

/*! 
	[static] 
*/
void UserArgumentsQuickView::ChildWidgetCallback(Fl_Widget* o, void* v)
{
	WidgetParams* pWP = static_cast<WidgetParams*>(v);
	StrList newValues;
	std::string val;

	ASSERT(pWP);

	//DBG_PRINT1("child widget callback")

	if (pWP->type == BOOL_WIDGET)
	{
		Fl_Button* pButton = dynamic_cast<Fl_Button*>(o);
		
		val = (pButton->value()) ? "yes" : "no";

		newValues.push_back(val);
	}
	else if (pWP->type == STRING_WIDGET)
	{
		Fl_Input* pText = dynamic_cast<Fl_Input*>(o);

		val = pText->value();

		newValues.push_back(val);
	}

	try {
		g_userArgs.UpdateArgs(pWP->FieldKeyParam(), pWP->PropKeyParam(), newValues);
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	// Let the main wondow know that the params have changed.
	// Right now, the callback calls VideoParserUI::cb_usrArgMonitor(...)
	// which in turns calls ideoParserUI::cb_usrArgMonitor_i(...) and
	// which simply calls mainWindow->Initialize()
	o->parent()->do_callback(o->parent(), (void*)o);
}

void UserArgumentsQuickView::AddBoolWidget(WidgetParamList::iterator it)
{
	bool val;

	try {
		val = g_userArgs.GetBoolValue(it->FieldKeyParam(), it->PropKeyParam());
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	it->y = Fl_Group::y();
	it->dx = 25;
	it->dy = 25;

	if (it == m_widgetParams.begin())
	{
		it->x = Fl_Group::x() + HSPACE;
	}
	else
	{
		WidgetParamList::iterator it0 = it;

		--it0;

		it->x = it0->x + it0->dx + HSPACE;
	}

	Fl_Button* p = new Fl_Check_Button(it->x, it->y, it->dx, it->dy, 
		it->label.c_str());

	p->user_data((void*)&(*it));

	//p->align(FL_ALIGN_RIGHT);
	p->value(val);
	p->callback(ChildWidgetCallback);

	add(p);

	// Add the width of the label
	int dx_label, dy_label;
		
	p->measure_label(dx_label, dy_label);

	it->dx += dx_label;
}

void UserArgumentsQuickView::AddStringWidget(WidgetParamList::iterator it)
{
	std::string val;

	try {
		val = g_userArgs.GetStrValue(it->FieldKeyParam(), it->PropKeyParam());
	}
	catch (BasicException e)
	{
		e.Print();
		return;
	}

	it->y = Fl_Group::y() + 4;
	it->dx = 100;
	it->dy = 21;

	if (it == m_widgetParams.begin())
	{
		it->x = Fl_Group::x() + HSPACE;
	}
	else
	{
		WidgetParamList::iterator it0 = it;

		--it0;

		it->x = it0->x + it0->dx + HSPACE;
	}

	Fl_Input* p = new Fl_Input(it->x, it->y, it->dx, it->dy, 
		it->label.c_str());

	p->user_data((void*)&(*it));

	p->color(fl_rgb_color(235, 235, 235));
	p->box(FL_FLAT_BOX);
	p->align(FL_ALIGN_RIGHT);
	p->value(val.c_str());
	p->callback(ChildWidgetCallback);

	add(p);

	// Add the width of the label
	int dx_label, dy_label;
		
	p->measure_label(dx_label, dy_label);

	it->dx += dx_label;
}

