/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _USER_ARGUMENTS_SHOW_GROUP_
#define _USER_ARGUMENTS_SHOW_GROUP_

#include <FL/Fl_Group.H>
#include <FL/Fl_Round_Button.H>
#include "UserArgumentsEditor.h"

class UserArgumentsShowGroup : public Fl_Group
{
	UserArgumentsEditor* m_pEditor;
	int m_currentRadioIndex;

public:
	UserArgumentsShowGroup(int x, int y, int w, int h, const char* l = 0)
		: Fl_Group(x, y, w, h, l)
	{
		m_pEditor = NULL;
		m_currentRadioIndex = 0;
	}

	void SetEditor(UserArgumentsEditor* pEditor)
	{
		m_pEditor = pEditor;
	}

	int Selection() const
	{
		return m_currentRadioIndex;
	}

	void SelectRadioButton(std::string lbl)
	{
		for (int n = 0; n < children(); n++)
		{
			if (lbl == child(n)->label())
				((Fl_Round_Button*) child(n))->value(1);
			else
				((Fl_Round_Button*) child(n))->value(0);
		}
	}

	void UpdateText()
	{
		switch (m_currentRadioIndex)
		{
		case 0: m_pEditor->PasteText(); break;
		case 1: m_pEditor->ShowFieldsAndvalues(); break;
		case 2: m_pEditor->ShowDefaultValues(); break;
		case 3: m_pEditor->ShowValueOptions(); break;
		}
	}

	void Update(int radioIndex)
	{
		//DBG_PRINT2(m_currentRadioIndex, radioIndex)

		// If the input text is going to "lose focus", then
		// we need to copy it so that we can retrieve it later
		if (m_currentRadioIndex == 0)
			m_pEditor->CopyText();

		m_currentRadioIndex = radioIndex;

		UpdateText();
	}
};

#endif // _USER_ARGUMENTS_SHOW_GROUP_