/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <vector>
#include <FL/Fl_Group.h>
#include <FL/Fl_Button.h>
#include <Tools/BasicUtils.h>
#include <Tools/BasicTypes.h>
#include <Tools/STLUtils.h>
#include <Tools/ZTString.h>

/*!
	Selector of user commands.
*/
class CommandSelector : public Fl_Group
{
	enum SPACES {HSPACE = 3};

	std::list<ZTString> m_strings;

protected:
	Fl_Button* ChildButton(int i)
	{
		return (Fl_Button*)child(i);
	}

	const Fl_Button* ChildButton(int i) const
	{
		return (Fl_Button*)child(i);
	}

	static void ChildButtonCallback(Fl_Widget* o, void* v)
	{
		//Fl_Button* b = (Fl_Button*)o;

		o->parent()->do_callback(o->parent(), v);
	}
	
public:
	CommandSelector(int x, int y, int w, int h, const char* l=0)
		: Fl_Group(x, y, w, h, l) 
	{ 
		// Overwrite the box used in the GUI, which is selected only
		// for locating the widget by hand
		box(FL_NO_BOX);

		// Prevent new widgets from being "auto" added to the group
		current(0);
	}

	int NumButtons() const
	{
		return children();
	}

	void Initialize()
	{
		// Overwrite the box used in the GUI, which is selected only
		// for locating the widget by hand
		box(FL_NO_BOX);

		// Delete all child buttons from memory
		clear();

		// Delete all button labels and tooltips
		m_strings.clear();

		// Tell the parent window to redraw
		parent()->redraw();
	}

	void AddCommand(const vpl::UserCommandInfo& cmd)
	{
		int y = Fl_Group::y();
		int dx = 20;
		int dy = 20;

		int x;

		if (NumButtons() == 0)
		{
			x = Fl_Group::x() + HSPACE;
		}
		else
		{			
			const Fl_Button* prev = ChildButton(NumButtons() - 1);

			x = prev->x() + prev->w() + HSPACE;
		}

		m_strings.push_back(ZTString(cmd.label));
		
		// Due to a FLTK bug, the measuring of labels ends at the
		// first white space, so we replace spaces by underscores
		m_strings.back().replace(' ', '_');

		Fl_Button* p = new Fl_Button(x, y, dx, dy, m_strings.back().c_str());

		std::string shortcut;

		if (cmd.keyCode >= 0 && cmd.keyCode <= 255)
		{
			shortcut.append(" [");
			shortcut.push_back(cmd.keyCode);
			shortcut.push_back(']');
		}

		m_strings.push_back(ZTString(cmd.tooltip + shortcut));

		p->tooltip(m_strings.back().c_str());

		p->argument(cmd.keyCode);

		p->callback(ChildButtonCallback);

		// Add the width of the label
		int dx_label, dy_label;
		
		p->measure_label(dx_label, dy_label);

		p->resize(x, y, MAX(dx_label, 20) + 5, dy);

		add(p);
	}

	void AddCommands(const std::list<vpl::UserCommandInfo>& cmds)
	{
		for (auto it = cmds.begin(); it != cmds.end(); ++it)
			AddCommand(*it);
	}

	void UpdateCommands(const std::list<vpl::UserCommandInfo>& cmds)
	{
		Initialize();
		AddCommands(cmds);
	}
};
