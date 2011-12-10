/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoParserWindow.h"
#include "ModeSelector.h"
#include <Tools/UserArguments.h>

extern vpl::UserArguments g_userArgs;

void ModeSelector::Initialize()
{
	VideoParserWindow* pParentWnd = GetMainWindow();

	unsigned initValue = pParentWnd->GetProcessingModes(m_modeLabels);

	// Add an extra "Play video" mode
	m_modeLabels.push_back("Play video");

	// Ensure that the widget's list is empty
	clear();

	// Copy the string to some memory owned by us
	StrList words;
	char itemName[100];

	for (unsigned int i = 0; i < m_modeLabels.size(); i++)
	{
		sprintf(itemName, "\t%s", m_modeLabels[i].c_str());

		add(itemName, 0, 0, (void*)i);
	}
	 
	// Set the current value of the ModeSelector to the current option
	// The option is zero if there where no labels, which is okay
	// because we added an extra label
	value(initValue);
}
