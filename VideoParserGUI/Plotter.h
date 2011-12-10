/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/ParamFile.h>
#include <Tools/STLUtils.h>
#include <Tools/Tuple.h>

class Ca_X_Axis;
class Ca_Y_Axis;
class Ca_Canvas;
class Ca_Text;

struct ValueList;

/*!
	
*/
class Plotter
{
	Ca_Canvas* m_canvas;
	Ca_Canvas* m_subcanvas;

	Ca_X_Axis* m_x_axis;
	Ca_Y_Axis* m_y_axis;

	Ca_Text* m_text;

	char m_outputText[1024];

protected:
	vpl::ParamFile m_datafile;
	std::string m_filename;

	std::map<std::string, IntDoubleList> m_precisionRecallData;
	std::shared_ptr<ValueList> m_pPRValues;

	DoubleList m_matchingTimes;

protected:
	bool ReadData();
	void SetUpWindow(int argc, char** argv);
	void PlotPrecisionAndRecall();
	void PlotMatchingTimes();
	void ParseData();

public:
	int Create(int argc, char** argv);
};