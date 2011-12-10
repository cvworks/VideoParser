/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _TEXT_VIEW_H_
#define _TEXT_VIEW_H_

#include <FL/Fl_Text_Display.H>
#include <string>

typedef Fl_Text_Display TextViewParent;

class TextView : public TextViewParent
{
	Fl_Text_Buffer m_buffer;

public:
	TextView(int x,int y,int w,int h,const char* l = 0)
		: TextViewParent(x, y, w, h, l)
	{
		buffer(&m_buffer);
	}

	void Initialize()
	{
		m_buffer.text("");
	}
	
	void SetText(const char* szTextOut)
	{
		m_buffer.text(szTextOut);
	}
};

#endif //_TEXT_VIEW_H_
