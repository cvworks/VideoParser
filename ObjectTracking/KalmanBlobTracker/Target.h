/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "Trace.h"
//#include <ImageSegmentation/Blob.h>
//#include "../BlobTrackerDBManager.h"

//struct _IplImage;
//typedef struct _IplImage IplImage; // for draw()

namespace vpl {

class Target;

typedef std::shared_ptr<Trace> TracePtr;
typedef std::shared_ptr<Target> TargetPtr;

class Target
{
protected:
    unsigned m_id;
    std::list<TracePtr> m_traces;

	int m_dbID; //!< ID of the target in the database

public:
	static int s_targetCount;

    Target(unsigned unique_identifier);

	~Target()
	{
		s_targetCount--;
	}

    unsigned get_number_of_traces() const
	{
		return m_traces.size();
	}

	TracePtr get_first_trace()
	{
		ASSERT(!m_traces.empty());

		return m_traces.front();
	}

	TracePtr get_last_trace()
	{
		ASSERT(!m_traces.empty());

		return m_traces.back();
	}

	unsigned get_id()
	{
		return m_id;
	}

    void add_trace(TracePtr new_trace);
    
	fnum_t get_first_frame();
    fnum_t get_last_frame();

    void draw(RGBImg image);

#ifdef SMOOTH_TRACES_ONLINE
    void Evolve();
#endif

    bool is_mobile() const;

	time_t start_time() const;
	time_t end_time() const;

	void SaveChangesToDatabase(vpl::BlobTrackerDBManager& dbm);
};

} // namespace vpl
