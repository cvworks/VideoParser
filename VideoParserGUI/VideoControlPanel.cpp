/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoControlPanel.h"
#include "ViewSelector.h"
#include "ParameterSelector.h"
#include "TextView.h"
#include "CommandSelector.h"
#include "ImageView.h"
#include <FL/Fl_Valuator.H>

#define WIDGET_V_SPACING 20
#define WIDGET_H_MARGIN 20

void VideoControlPanel::Initialize()
{
	m_pImgView->Initialize();
	m_pViewSelector->Initialize();
	m_pParamSelector->Initialize();
	m_pTextView->Initialize();
	m_pCmdSelector->Initialize();
}

/*!
	Sets the size and position of the panel and its child widgets

	The size is given by 'width' and 'height'. The position is given
	by 'x' and the current 'y' position. That is, the top position 
	of the group doesn't change.

	It is assumed that the child widgets can be ordered vertically. Ie,
	that the group is like a vertical array of widgets.

	It is also assumed that the first widget is the image view widget.

	The hights of the widgets other that the image view are maintained. 
	The width of the widgets is determined from the given image width 
	minus some left/right margins.
*/
void VideoControlPanel::ResizeGroup(int width, int height, int x, 
									const double& imgScaling)
{
	std::vector< std::pair<int, Fl_Widget*> > widgets;
	Fl_Widget* pWidget;
	int hightSum = 0;

	// Set the current image scaling into the image view widget
	m_pImgView->SetScaling(imgScaling);

	width = int(width * imgScaling);
	height = int(height * imgScaling);

	// Copy the widh and size of the image displayed. Ensure that
	// it's not too small
	const int imgWidth = MAX(width, 20);
	const int imgHight = MAX(height, 20);

	// Save the y-coord of each child widget and sum the height 
	// of all child widgets
	for (int n = 0; n < children(); n++)
	{
		pWidget = child(n);
		widgets.push_back(std::make_pair(pWidget->y(), pWidget));
		hightSum += (n == 0) ? imgHight : pWidget->h();
	}

	// Use the y-coord of each widget to sort them, so that
	// we can laytherm out in their original order
	std::sort(widgets.begin(), widgets.end());

	// Add the vertical spacing used to separate the widgets
	int y = this->y(); // the top position of the group doesn't change
	const int spacing = WIDGET_V_SPACING;

	hightSum += spacing * (widgets.size() + 1);

	// Resize the widget group
	resize(x, y, imgWidth, hightSum);

	// Resize each widget
	for (unsigned int i = 0; i < widgets.size(); i++)
	{
		pWidget = widgets[i].second;

		if (i == 0)
		{
			// This is the image view widget
			// @todo +3 is due to a bug in OpenGL draw pixel (for some GPUs)
			pWidget->resize(this->x(), y, imgWidth, imgHight + 3);
		}
		else
		{
			// Keep widget's height
			pWidget->resize(this->x() + WIDGET_H_MARGIN, y, 
				imgWidth - 2 * WIDGET_H_MARGIN, pWidget->h());
		}
		
		y += pWidget->h() + spacing;
	}
}
