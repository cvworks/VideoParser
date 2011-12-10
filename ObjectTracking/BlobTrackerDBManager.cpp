/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BlobTrackerDBManager.h"
#include <Tools\SQLDatabase.h>
#include <Tools\Time.h>

using namespace vpl;

bool BlobTrackerDBManager::readBlobTraces(int session_id, BlobTraces& traces)
{
	IntList trace_ids;
	
	readTraceIds(session_id, trace_ids);

	traces.resize(trace_ids.size());

	unsigned i = 0;

	for (auto it = trace_ids.begin(); it != trace_ids.end(); ++it)
		readTraceNodes(session_id, *it, traces[i++]);
	
	return !traces.empty();
}

bool BlobTrackerDBManager::readTraceIds(int session_id, std::list<int>& ids)
{
	if (!m_pSQLDatabase->Execute(
		"SELECT "
		"trace.trace_id "
		"FROM tracker.trace where session_id = %d", 
		session_id))
	{
		showDBError("Cannot read trace IDs from the database");
		return false;
	}

	SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();
	SQLDataRow row;

	ids.clear();

	while (row = m_pSQLDatabase->FetchDataRow(res))
	{
		for (unsigned i = 0; i < res.FieldCount(); i++)
		{
			ids.push_back(atoi(row[i]));
		}
	}

	return !ids.empty();
}

/*! 
	Creates a session with the given name. It stores the session ID internally
	so that all other pieces of data creater later can be linked to it.

	If the internal session ID is a valid session, it will be overwritten with
	the newly created session.

	@return true of the session was created succesfully.
*/
bool BlobTrackerDBManager::readTraceNodes(int session_id, int trace_id, std::list<BlobPtr>& nodes)
{
	nodes.clear();

    // Get the largest session id
    if (!m_pSQLDatabase->Execute(
		"SELECT "
		"trace_node.timestamp,"
		"trace_node.xcor,"
		"trace_node.ycor,"
		"trace_node.width,"
		"trace_node.height "
		"FROM tracker.trace_node where session_id = %d and trace_id = %d "
		"ORDER BY trace_node.node_id", 
		session_id, trace_id))
	{
		showDBError("Cannot read trace nodes from the database");
		return false;
	}

    SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();

	if (res.FieldCount() != 5)
		return false;

	SQLDataRow row;
	time_t tmstamp;
	BlobPtr b;

	while (row = m_pSQLDatabase->FetchDataRow(res))
	{
		tmstamp = Time(std::string(row[0]), Time::SQL_DATETIME).toSeconds();

		b.reset(new Blob(tmstamp, atoi(row[1]), atoi(row[2]), 
			atoi(row[3]), atoi(row[4])));

		nodes.push_back(b);
	}

	return !nodes.empty();
}

bool BlobTrackerDBManager::writeSessionEndtime()
{
	ASSERT(hasSession());

	// Create the table and store the max_id
    if (!m_pSQLDatabase->Execute("UPDATE tracker.session SET end_time=NOW() "
		"WHERE session_id=%d", m_sessionID))
	{
		showDBError("Cannot set the end time of the current session");
		return false;
	}

	return true;
}

bool BlobTrackerDBManager::readSessionInfo(int session_id, TrackingSessionInfo& tsi)
{
	// Start by specifying a negative session id in case the operation fails
	tsi.session_id = -1;

	// Get the largest session id
    if (!m_pSQLDatabase->Execute(
		"SELECT name, frame_width, frame_height, start_time, end_time "
		"FROM tracker.session where session_id = %d", session_id))
	{
		showDBError("Cannot read trace nodes from the database");
		return false;
	}

    SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();

	if (res.FieldCount() != 5)
		return false;

	SQLDataRow row;

	while (row = m_pSQLDatabase->FetchDataRow(res))
	{
		tsi.name         = row[0];
		tsi.frame_width  = atoi(row[1]);
		tsi.frame_height = atoi(row[2]);
		tsi.start_time   = Time(std::string(row[3]), Time::SQL_DATETIME).toSeconds();

		// The endtime is optional
		tsi.end_time = (row[4]) ? 
			Time(std::string(row[4]), Time::SQL_DATETIME).toSeconds() : 0;
	}

	tsi.session_id = session_id;

	return true;
}

/*! 
	Creates a session with the given name. It stores the session ID internally
	so that all other pieces of data creater later can be linked to it.

	If the internal session ID is a valid session, it will be overwritten with
	the newly created session.

	@return true of the session was created succesfully.
*/
bool BlobTrackerDBManager::createSession(const TrackingSessionInfo& tsi)
{
	// Reset the session ID to indicate that there is no session in
	// case the operation fails
	resetSession();

    // Get the largest session id
    if (!m_pSQLDatabase->Query("select max(session_id) from session"))
	{
		showDBError("Cannot query database in order to create session");
		return false;
	}

    SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();
	int max_id = 0;

	m_pSQLDatabase->FetchData(res, &max_id);

    // Increment it by one
    max_id++;

    // Create the table and store the max_id
    if (!m_pSQLDatabase->Execute("insert into session(session_id, name, "
		"start_time, frame_width, frame_height) values(%d, '%s', NOW(), %d, %d)", 
		max_id, tsi.name.c_str(), tsi.frame_width, tsi.frame_height))
	{
		showDBError("Cannot create session");
		return false;
	}
	
	m_sessionID = max_id;

	return true;
}

//! Creates a period with a given sid, length, filename
int BlobTrackerDBManager::createTarget(int target_id, time_t start_time, fnum_t first_frame)
{
	ASSERT(hasSession());

	std::string tm = Time(start_time).str();

    //create the table and return the max_id if successful
    if (!m_pSQLDatabase->Execute("insert into target"
		"(target_id, session_id, start_time) "
		"values(%d, %d, '%s')", target_id, m_sessionID, tm.c_str()))
	{
		showDBError("Cannot create target");
		return -1;
	}
	
	return target_id;
}

//! Creates a new trace with a given length and start_time
int BlobTrackerDBManager::createTrace(int trace_id, time_t start_time, 
	fnum_t first_frame, int target_id)
{
	std::string tm = Time(start_time).str();

    // Create the table and return the max_id if successful
    if (!m_pSQLDatabase->Execute("insert into trace"
		"(trace_id, session_id, start_time, first_frame, target_id) "
		"values(%d, %d, '%s', %d, %d)", 
		trace_id, m_sessionID, tm.c_str(), (int)first_frame, target_id))
	{
		showDBError("Cannot create trace");
		return -1;
	}
	
	return trace_id;
}

//! Creates a trace node associated to a trace with tid
bool BlobTrackerDBManager::createTraceNode(int xcor, int ycor, int height, int width, 
		time_t timestamp, int node_id, int trace_id)
{
	std::string tm = Time(timestamp).str();

    //create the table row and return 0 if successful
    if (!m_pSQLDatabase->Execute("insert into trace_node"
		"(xcor, ycor, height, width, timestamp, node_id, trace_id, session_id) "
		"values(%d, %d, %d, %d, '%s', %d, %d, %d)", 
		xcor, ycor, height, width, tm.c_str(), node_id, trace_id, m_sessionID))
	{
		showDBError("Cannot create trace node");
		return false;
	}

	return false;
}

//! Removes a trace with a given trace_id
void BlobTrackerDBManager::removeTrace(int session_id, int trace_id)
{
    // Remove trace nodes
	m_pSQLDatabase->Execute("delete from trace_node where session_id=%d and trace_id=%d", 
		session_id, trace_id);

    // Remove traces
    m_pSQLDatabase->Execute("delete from trace where session_id=%d and trace_id=%d", 
		session_id, trace_id);
}

//! Removes a target and its associated traces, and associated trace nodes.
void BlobTrackerDBManager::removeTarget(int session_id, int target_id)
{
	// Get all the associate trace_id's to this target_id
    m_pSQLDatabase->Execute("select trace_id from trace where session_id=%d and target_id=%d", 
		session_id, target_id);

    SQLQueryResult res = m_pSQLDatabase->StoreQueryResult();
	int trace_id;

	// Remove all the traces and trace nodes associated with this target_id
    while (m_pSQLDatabase->FetchData(res, &trace_id))
        removeTrace(session_id, trace_id);

    // Remove targets
    m_pSQLDatabase->Execute("delete from target where session_id=%d and target_id=%d", 
		session_id, target_id);
}

//! Removes a session with a given sid
void BlobTrackerDBManager::removeSession(int session_id)
{
	// Remove trace nodes
	m_pSQLDatabase->Execute("delete from trace_node where session_id=%d", session_id);

    // Remove traces
    m_pSQLDatabase->Execute("delete from trace where session_id=%d", session_id);

	// Remove targets
    m_pSQLDatabase->Execute("delete from target where session_id=%d", session_id);
    
    // Remove the sessions
    m_pSQLDatabase->Execute("delete from session where session_id=%d", session_id);
}
