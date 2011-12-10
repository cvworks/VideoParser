/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/BasicUtils.h>
#include <Tools/SQLDatabase.h>
#include <Tools/InputImageInfo.h>
 #include <ImageSegmentation/Blob.h>

namespace vpl {

struct TrackingSessionInfo
{
	std::string name;
	unsigned frame_width;
	unsigned frame_height;

	int session_id;    //!< Only set when reading from DB
	time_t start_time; //!< Only set when reading from DB
	time_t end_time;   //!< Only set when reading from DB

	TrackingSessionInfo()
	{

	}

	TrackingSessionInfo(const std::string& nm, unsigned width, unsigned height)
		: name(nm)
	{
		frame_width = width;
		frame_height = height;
	}

	void clear()
	{
		name.clear();
		frame_width = 0;
		frame_height = 0;
		session_id = -1;
		start_time = 0;
		end_time = 0;
	}
};

class BlobTrackerDBManager
{
	SQLDatabase* m_pSQLDatabase;
	int m_sessionID;

protected:
	void showDBError(std::string msg) const
	{
		if (m_pSQLDatabase)
			m_pSQLDatabase->showDBError(msg);
		else
			ShowError("The database is not opened");
	}

public:
	BlobTrackerDBManager(SQLDatabase* pDB = NULL)
	{
		m_pSQLDatabase = pDB;
		m_sessionID = -1;
	}

	void resetSession()
	{
		m_sessionID = -1;
	}

	bool hasDB() const
	{
		return m_pSQLDatabase != NULL;
	}

	bool hasSession() const
	{
		return m_sessionID >= 0;
	}

	operator bool() const 
	{ 
		return hasDB(); 
	}

	void init(SQLDatabase* pDB)
	{
		m_pSQLDatabase = pDB;
	}

	bool createSession(const TrackingSessionInfo& tsi);
	int createTarget(int target_id, time_t start_time, fnum_t first_frame);
	int createTrace(int trace_id, time_t start_time, fnum_t first_frame, int target_id);
	bool createTraceNode(int xcor, int ycor, int height, int width, 
		time_t timestamp, int node_id, int trace_id);

	bool writeSessionEndtime();

	bool readSessionInfo(int session_id, TrackingSessionInfo& tsi);
	bool readBlobTraces(int session_id, BlobTraces& traces);
	bool readTraceIds(int session_id, std::list<int>& ids);
	bool readTraceNodes(int session_id, int trace_id, std::list<BlobPtr>& nodes);

	void removeSession(int sid);
	void removeTarget(int session_id, int target_id);
	void removeTrace(int session_id, int trace_id);
};

} // namespace vpl
