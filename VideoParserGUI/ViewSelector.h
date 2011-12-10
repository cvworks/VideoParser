/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VIEW_SELECTOR_H_
#define _VIEW_SELECTOR_H_

#include <FL/Fl_Choice.H>
#include <Tools/BasicUtils.h>

class ViewSelector : public Fl_Choice
{
public:
	ViewSelector(int x,int y,int w,int h,const char* l = 0)
		: Fl_Choice(x, y, w, h, l)
	{
	}

	void Initialize();
	
	int GetSelection() const
	{
		return (mvalue()) ? ((int) mvalue()->user_data()) : 0;
	}

	void SetSelection(int sel)
	{
		const Fl_Menu_Item* options = menu();

		for (int i = 0; i < size(); ++i)
		{
			if (sel == (int) options[i].user_data())
			{
				SetSelectionIndex(i);
				return;
			}
		}

		WARNING(sel, "Value not found");
	}

	int GetSelectionIndex() const
	{
		return value();
	}

	void SetSelectionIndex(int idx)
	{
		value(idx);
	}
};

#endif //_VIEW_SELECTOR_H_
