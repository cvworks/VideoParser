/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "VideoParserWindow.h"
#include "ViewSelector.h"
#include <Tools/UserArguments.h>

extern vpl::UserArguments g_userArgs;

void ViewSelector::Initialize()
{
	VideoParserWindow* pParentWnd = GetMainWindow();

	std::vector<std::string> lbls;

	pParentWnd->GetVisionComponentLabels(lbls);

	// Ensure that the current list is empty
	clear();

	StrList words;
	StrList::iterator it;
	std::string groupName;
	unsigned int groupIndex = 0;
	char itemName[100];

	for (unsigned int i = 0; i < lbls.size(); i++)
	{
		words.clear();
		Tokenize(lbls[i], ",", words);

		if (words.size() > 1)
		{
			it = words.begin();

			// If the group name changed, start new group
			if (*it != groupName)
			{
				groupName = *it;
				groupIndex = i;

				// Add the group's name
				add(it->c_str(), 0, 0, (void*)i);
			}
			
			//sprintf(itemName, "\t%d- %s", i - groupIndex, (++it)->c_str());
			sprintf(itemName, "\t%s", (++it)->c_str());

			// Add the item's name
			add(itemName, 0, 0, (void*)i);
		}
		else
		{
			groupName.clear();

			add(lbls[i].c_str(), 0, 0, (void*)i);
		}
	}

	int initValue;

	g_userArgs.ReadArg("GUI", "firstViewShown", "Index of the first view type"
		" shown when a new video is loaded", 0, &initValue);

	g_userArgs.CheckValues("GUI", "firstViewShown", 0, ((int)lbls.size()) - 1);
	 
	// Set the current value of the ViewSelector to the first option
	value(initValue);
}
