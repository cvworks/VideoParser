// Cartesian v 1.1
//
// Copyright 2000-2008 by Roman Kantor.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// version 2 as published by the Free Software Foundation.
//
// This library is distributed  WITHOUT ANY WARRANTY;
// WITHOUT even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
#include "Plotter.h"
#include <Tools/Tuple.h>
#include <Tools/BasicUtils.h>
#include <Tools/UserArguments.h>
#include <FL/Fl_File_Chooser.H>
#include <vul/vul_file.h>

#ifdef USE_ROTATED_TEXT
#include <Fl_Rotated_Text/Fl_Rotated_Text.cxx>
#endif

//#include <math.h>
#include <FL/Fl.H>
#include <FL/Fl_Overlay_Window.H>
#include <FL/Fl_Light_Button.H>
#include "Cartesian.h"
#include <FL/fl_draw.H>

extern vpl::UserArguments g_userArgs;

typedef vpl::Tuple<double, 2, '(', ')'> DataValue;

struct ValueList : public std::list<DataValue>
{
};

bool Plotter::ReadData() 
{
	Fl_File_Chooser fc(".", "Results Files (*.{txt})", 
	Fl_File_Chooser::SINGLE, "Open results file");

	fc.preview(0);
	fc.show();

	while (fc.visible())
		Fl::wait();
	
	if (fc.count() > 0)
	{
		m_filename = fc.value(0);

		return m_datafile.ReadParameters(m_filename.c_str());
	}
	else
		return false;
}

void Plotter::PlotMatchingTimes()
{
	double totTime = 0, maxTime = 0;
	int i = 0, maxTimeIdx = 0;

	for (auto it = m_matchingTimes.begin(); 
		it != m_matchingTimes.end(); ++it, ++i)
	{
		if (*it > maxTime)
		{
			maxTime = *it;
			maxTimeIdx = i;
		}

		totTime += *it;

		new Ca_Point(i, *it, FL_RED, CA_SIMPLE, 2);
	}
	
	if (m_y_axis->maximum() < maxTime)
		m_y_axis->maximum(maxTime);

	// Show average time

	totTime /= m_matchingTimes.size();

	std::stringstream ss;

	ss << "\nT = " << totTime;

	strcat(m_outputText, ss.str().c_str());

	DBG_PRINT1(maxTimeIdx);
}

void Plotter::PlotPrecisionAndRecall()
{
	std::stringstream ss;

	for (auto it0 = m_precisionRecallData.begin(); 
		it0 != m_precisionRecallData.end(); ++it0)
	{	
		it0->second.sort();

		for (auto it1 = it0->second.begin(); it1 != it0->second.end(); ++it1)
		{	
			ss << "(" << it1->first << ", " << it1->second << ")\n";
		}
	}
	
	strcat(m_outputText, ss.str().c_str());

	Ca_PolyLine* pl = 0;
	std::vector<Ca_Point*> pts;
	int ptIdx = 0;
	vpl::Point pt;

	for (auto it = m_pPRValues->begin(); it != m_pPRValues->end(); ++it)
	{
		/*pl = new Ca_PolyLine(pl, it->at(0) * 100, it->at(1) * 100, 
			FL_DASHDOT, 2, FL_RED, CA_NO_POINT);*/

		pt.Set(ptIdx++, it->at(1) * 100);

		pts.push_back(new Ca_Point(pt.x, pt.y, FL_BLACK, CA_SIMPLE, 2));

		//DBG_PRINT1(pt)

		//DBG_PRINT2(it->at(0) * 100, it->at(1) * 100)
		//it->Print();
	}

	m_x_axis->maximum(ptIdx);
}

void Plotter::ParseData()
{
	int field_min, field_max, prop_min, prop_max;

	char expName[100], caseName[100];
	int maxNumParses;

	std::string key;

	m_pPRValues.reset(new ValueList);
	m_matchingTimes.clear();
	m_outputText[0] = '\0';

	try {
		if (!m_datafile.GetFieldPrefixMinMax(std::string("Experiment"), 
			&field_min, &field_max))
		{
			ShowError("The requested experiment does not exist.");
			return;
		}

		ASSERT(field_min <= field_max);
	}
	catch(BasicException e) 
	{
		e.Print();
		return;
	}

	// Iterate over all Experiments fields. We plot the recognition 
	// performance as a function of maxNumParses
	for (int expId = field_min; expId <= field_max; expId++)
	{
		sprintf(expName, "Experiment%d", expId);

		ShowStatus1("Plotting: ", expName);
		
		// Form the experiment's unique 'key' and get its associated 
		// maximum number of parses parameter.
		try {
			if (!m_datafile.GetPropertyPrefixMinMax(std::string(expName), 
				std::string("frame"), &prop_min, &prop_max))
			{
				ShowError("There are no data cases to plot.");
				return;
			}

			ASSERT(prop_min <= prop_max);

			key.clear();

			key += m_datafile.GetStrValue(expName, "video_filename") + '_';
			key += m_datafile.GetStrValue(expName, "model_database") + '_';
			key += m_datafile.GetStrValue(expName, "shape_descriptor") + '_';
			key += m_datafile.GetStrValue(expName, "matching_algorithm");

			// Get the valut of K
			maxNumParses = m_datafile.GetIntValue(expName, "max_num_parses");

			//DBG_PRINT2(key, maxNumParses)
		}
		catch(BasicException e) 
		{
			e.Print();
			return;
		}

		// Given the experiment key and the value K, read 
		// the results of each frame processes in the experiment

		// Read precission and recall data
		double recPerf = 0;
		bool hasData;

		for (int caseId = prop_min; caseId <= prop_max; caseId++)
		{
			sprintf(caseName, "frame%dquery0_precision_recall", caseId);

			ValueList vl;

			hasData = m_datafile.GetOptionalTypedValues(expName, caseName, vl);

			if (hasData && !vl.empty())
			{
				if (vl.front().at(1) == 0)
					ShowError(caseName);

				recPerf += vl.front().at(1);

				m_pPRValues->push_back(vl.front());
			}
			else
			{
				ShowError1("Missing precision and recall data", caseName);
			}
		}

		recPerf /= (prop_max - prop_min + 1);

		m_precisionRecallData[key].push_back(std::make_pair(maxNumParses, recPerf));

		// Read matching times
		double time;

		for (int caseId = prop_min; caseId <= prop_max; caseId++)
		{
			sprintf(caseName, "frame%dquery0_matchingTime", caseId);

			hasData = m_datafile.GetOptionalDoubleValue(expName, caseName, time);

			if (hasData)
				m_matchingTimes.push_back(time);
			else
				ShowError1("Missing matching time", caseName);
		}
	}
}

/*!
	Creates the window and its child widgets
*/
void Plotter::SetUpWindow(int argc, char** argv)
{
#ifdef USE_ROTATED_TEXT
    Fl_Rotated_Text current_label("Current [mA]",FL_HELVETICA,14,0, 1);
    Fl_Rotated_Text phase_label("Phase [rad]",FL_HELVETICA,14,0, 3);
#endif

	Fl_Double_Window* w = new Fl_Double_Window(580, 380, m_filename.c_str());

    w->size_range(450,250);

	Fl_Group* c =new Fl_Group(0, 35, 580, 345 );

    c->box(FL_DOWN_BOX);
    c->align(FL_ALIGN_TOP|FL_ALIGN_INSIDE);

	m_subcanvas = new Ca_Canvas(10, 70, 75, 235, "Output");
	m_subcanvas->box(FL_DOWN_BOX);
    m_subcanvas->color(7);
    m_subcanvas->align(FL_ALIGN_TOP);
    Fl_Group::current()->resizable(m_subcanvas); // or do: w->resizable(m_canvas);
    m_subcanvas->border(15);
	new Ca_X_Axis(0, 235, 70, 235, "Unused");
	new Ca_Y_Axis(0, 0, 70, 235, "Unused");

    m_canvas = new Ca_Canvas(180, 75, 300, 225, "Precision and Recall Curve (PRC)");
    m_canvas->box(FL_DOWN_BOX);
    m_canvas->color(7);
    m_canvas->align(FL_ALIGN_TOP);
    Fl_Group::current()->resizable(m_canvas); // or do: w->resizable(m_canvas);
    m_canvas->border(15);

    m_x_axis = new Ca_X_Axis(180, 305, 300, 30, "Recall [%]");
    m_x_axis->labelsize(14);
    m_x_axis->align(FL_ALIGN_BOTTOM);
	
	m_x_axis->scale(CA_LIN);

    m_x_axis->minimum(0);
    m_x_axis->maximum(100);
    
     
    m_y_axis = new Ca_Y_Axis(137, 70, 43, 235);

	m_y_axis->minimum(0);
    m_y_axis->maximum(100);

#ifdef USE_ROTATED_TEXT
    m_y_axis->image(&current_label);
    m_y_axis->align(FL_ALIGN_LEFT);
#else
    m_y_axis->label("Precision [%]");
    m_y_axis->align(FL_ALIGN_RIGHT|FL_ALIGN_TOP);
#endif
   
	c->end();

#ifdef FL_DEVICE	
	Fl_Button *b2 = new Fl_Button(5,5, 90, 25, "Print to file");
	b2->callback(print,c);

	Fl_Button *b3 = new Fl_Button(105,5, 90, 25, "Print");
	b3->callback(print2,c);
#endif

	Fl_Group::current()->resizable(c);

    w->end();

	w->show(argc, argv);
}

int Plotter::Create(int argc, char** argv) 
{
	// Read the input file name and its data
	if (!ReadData())
	{
		ShowError("Cannot read data values");
		return 1;
	}

	char title[MAX_PATH_SIZE];
	char* args[1] = {title};

	//m_filename.copy(title, m_filename.size());
	string_copy(m_filename, title, MAX_PATH_SIZE, m_filename.size());

	SetUpWindow(1, args);

	ParseData();

	PlotPrecisionAndRecall();
	PlotMatchingTimes();

	// Display output text
	Ca_Canvas::current(m_subcanvas);
	m_text = new Ca_Text(1, 1, m_outputText);
	Ca_Canvas::current(m_canvas);
	
	Fl::run();

	return 0;
}
