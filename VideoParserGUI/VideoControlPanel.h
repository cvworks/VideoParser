/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _VIDEO_CONTROL_PANEL_H_
#define _VIDEO_CONTROL_PANEL_H_

#include <FL/Fl_Group.H>

class ImageView;
class ViewSelector;
class ParameterSelector;
class TextView;
class CommandSelector;

class VideoControlPanel : public Fl_Group
{
protected:
	ImageView* m_pImgView;
	ViewSelector* m_pViewSelector;
	ParameterSelector* m_pParamSelector;
	TextView* m_pTextView;
	CommandSelector* m_pCmdSelector;

public:
	VideoControlPanel(int x,int y,int w,int h,const char *l = 0)
		: Fl_Group(x, y, w, h, l)
	{
	}

	void Initialize();

	ImageView* GetView()                  { return m_pImgView; }
	ViewSelector* GetViewSelector()       { return m_pViewSelector; }
	ParameterSelector* GetParamSelector() { return m_pParamSelector; }
	TextView* GetTextView() const         { return m_pTextView; }
	CommandSelector* GetCommandSelector() { return m_pCmdSelector; }

	void SetWidgets(ImageView* pImgView, ViewSelector* pViewSelector, 
		ParameterSelector* pParamSelector, TextView* pTextView, CommandSelector* pCmdSelector)
	{
		m_pImgView = pImgView;
		m_pViewSelector = pViewSelector;
		m_pParamSelector = pParamSelector;
		m_pTextView = pTextView;
		m_pCmdSelector = pCmdSelector;
	}

	void ResizeGroup(int width, int hight, int x, const double& imgScaling);
};

#endif //_VIDEO_CONTROL_PANEL_H_