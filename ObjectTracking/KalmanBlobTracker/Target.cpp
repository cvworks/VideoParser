/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <Tools/STLUtils.h>
#include <Tools/CvMatView.h>

#include "target.h"
#include "trace.h"
#include <Tools/CvUtils.h>

#define PLCMNT_NONE 0
#define PLCMNT_CENTER 1

using namespace vpl;

int Target::s_targetCount = 0;

/*CvPoint center_placement(XYCoord* point, const char* text, CvFont* font)
{
    CvPoint text_placement;

    //Get the text size
    CvSize textSize;
    int ymin;
    cvGetTextSize(text ,font, &textSize, &ymin);

    text_placement.x = int(point->x - 0.5* textSize.width);
    text_placement.y = int(point->y + 0.5* textSize.height);

    return text_placement;
}

void add_text(RGBImg image, const char* text, XYCoord* point, 
	CvFont* font, cv::Scalar color, int placement)
{
    cv::Point text_placement;

    //Get text placement
    switch (placement)
    {
    case PLCMNT_CENTER :
    // Center the text
        text_placement = center_placement(point, text, font);
        break;

    default :
        // No special placement.
        text_placement = cvPoint(point->x,point->y);
    }

	CvMatView mat(image);

    cv::putText(mat, text, text_placement, font, color);
}*/

Target::Target(unsigned unique_identifier)
{
	s_targetCount++;

	m_id = unique_identifier;
	m_dbID = -1;
}

time_t Target::start_time() const 
{ 
	return m_traces.empty() ? 0 : m_traces.front()->start_time(); 
}

time_t Target::end_time() const   
{ 
	return m_traces.empty() ? 0 : m_traces.back()->end_time(); 
}

void Target::add_trace(TracePtr new_trace)
{
	ASSERT(!new_trace->is_matched());

	new_trace->set_matched(true);

    m_traces.push_back(new_trace);
    
	//if this is not the first trace being added
	//set the color of this trace to the color of the first trace
    if (get_number_of_traces() > 1)     
        new_trace->set_color(m_traces.front()->get_color());
}

fnum_t Target::get_first_frame()
{
    return get_first_trace()->get_first_frame();
}

fnum_t Target::get_last_frame()
{
    return get_last_trace()->get_last_frame();
}

void Target::draw(RGBImg image)
{
    for (auto iter = m_traces.begin(); iter != m_traces.end(); iter++)
    {
        (*iter)->draw(image);
    }

    //For the last trace, get the last blob feature's centroid
    TracePtr last_trace = get_last_trace();
    XYCoord centroid = last_trace->expected_feature().centroid();

    //Display the target's m_id at the centroid
    std::string text;

    append_num(text, m_id);

    //CvFont font;
    //cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, 0.8,0.8,0,2);
    cv::Scalar color = last_trace->get_color();

	CvMatView mat(image);

	DrawCenteredText(mat, text.c_str(), cv::Point(centroid.x, centroid.y), 
				cv::FONT_HERSHEY_COMPLEX, 0.8, color);

    //add_text(image, text.c_str(), &centroid, 
	//	&font, color, PLCMNT_CENTER);
}

#ifdef SMOOTH_TRACES_ONLINE
void Target::Evolve()
{
    get_last_trace()->Evolve();
}
#endif

bool Target::is_mobile() const
{
	for (auto it = m_traces.begin(); it != m_traces.end(); it++)
		if ((*it)->is_mobile())
			return true;
	
	return false;
}

void Target::SaveChangesToDatabase(vpl::BlobTrackerDBManager& dbm)
{
	ASSERT(dbm.hasDB());

	if (m_traces.empty())
		return;

	if (m_dbID < 0)
	{
		m_dbID = dbm.createTarget(m_id, start_time(), get_first_frame());

		if (m_dbID < 0)
		{
			ShowError("Cannot save target to database");
			return;
		}
	}

	m_traces.back()->SaveChangesToDatabase(dbm, m_dbID);
}
