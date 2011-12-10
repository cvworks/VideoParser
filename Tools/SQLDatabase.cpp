/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <iostream>
#include <windows.h>
#include <mysql.h>
#include <sstream>
#include <list>

#include "SQLDatabase.h"
#include "BasicUtils.h"
#include "Exceptions.h"

#define NONBLOCK_MODE_SLEEP_TIME 100

///////////////////////////////////////////////////////////////
// Begin multithreading functions and vaiables
#ifndef WIN32
#define HAVE_PTHREAD_H 1
#endif

#include <VideoParserGUI\threads.h>

using namespace vpl;

//! Globals function used to create new video parsing threads
void* vpl::ProcessQueries(void* pParam)
{
	SQLThreadData* pData = (SQLThreadData*) pParam;

	pData->state = SQLThreadData::RUNNING;
	std::string cmd;

	while (pData->state == SQLThreadData::RUNNING)
	{
		while (pData->queries.Pop(cmd))
		{
			if (::mysql_query(pData->sql(), cmd.c_str()) != 0)
			{
				ShowError(mysql_error(pData->sql()));
				return pParam;
			}
		}

		Sleep(NONBLOCK_MODE_SLEEP_TIME);
	}

	pData->state = SQLThreadData::STOPPED;

	return 0;
}

bool SQLThreadData::push_back(const std::string& val)
{
	if (state == READY || state == RUNNING)
	{
		// If the push operation fails, it means that the queue is full, so
		// we sleep in order to wait for the consumer thread to make room in the queue.
		// Note that this case the consumer thread has SIZE number of queries pending,
		// so waiting until in completes a few shouldn't be a problem
		while (!queries.Push(val))
		{
			DBG_LINE
			Sleep(10); 
		}

		return true;
	}

	return false;
}


/*!
	In non-blocking mode, there might be pending queries
	that must be executed before storing the results
	ie, this function will block the exceution until
	all pending queries are processed, such that
	we can get the result of the last query.
*/
void SQLThreadData::wait_for_queries()
{
	while (!empty())
		Sleep(NONBLOCK_MODE_SLEEP_TIME);
}

// End multithreading functions and vaiables
///////////////////////////////////////////////////////////////

bool SQLDatabase::Open(std::string name, bool nonBlockingMode)
{
	if (m_isOpen)
	{
		ShowError("There is a database already opened");
		return false;
	}

	mysql_init(m_pMySql);
   
	if (!m_pMySql)
    {
		showDBError("Cannot init database");

        return false;
    }

	MYSQL* status = mysql_real_connect (
            m_pMySql,       // pointer to connection handler
            "localhost",    // host to connect to
            "viva10",       // user name
            "pass10",       // password
            "tracker",      // database to use
            0,              // port (default 3306)
            NULL,           // socket or /var/lib/mysql.sock
            0 );            // flags (none)

    if (!status)
    {
		showDBError("Cannot connect to database");

        return false;
    }

	mysql_autocommit(m_pMySql, true);

	m_isOpen = true;

	m_nonBlockingMode = nonBlockingMode;

	return true;
}

unsigned SQLQueryResult::FieldCount() 
{
	ASSERT(m_pResult);

	return mysql_num_fields(m_pResult);
}

void SQLQueryResult::Free()
{
	mysql_free_result(m_pResult);
	m_pResult = NULL;
}

SQLQueryResult::~SQLQueryResult()
{
	mysql_free_result(m_pResult);
}

SQLDatabase::SQLDatabase()
{
	m_pMySql = new MYSQL;
	m_isOpen = false;

	m_nonBlockingMode = false;
	m_pThreadData = NULL;
}

SQLDatabase::~SQLDatabase()
{
	Close();

	delete m_pMySql;

	delete m_pThreadData;
}

void SQLDatabase::Close()
{
	if (m_nonBlockingMode && m_pThreadData)
	{
		m_pThreadData->request_stop();

		delete m_pThreadData;

		m_pThreadData = NULL;
	}

	if (m_pMySql)
	{
		mysql_commit(m_pMySql);

        mysql_close(m_pMySql);
	}

	m_isOpen = false;
}

bool SQLDatabase::CreateQueryThread()
{
	ASSERT(!m_pThreadData);

	m_pThreadData = new SQLThreadData(m_pMySql);

	int stat = fl_create_thread(m_threadId, ProcessQueries, m_pThreadData);

	if (stat <= 0)
	{
		ShowError("Cannot run in non-blocking mode");
		m_nonBlockingMode = false;
	}

	return true;
}

void SQLDatabase::showDBError(std::string msg) const
{
	if (!msg.empty())
		ShowError1("Database operation failed:", msg);
	
	ShowError(mysql_error(m_pMySql));
}

bool SQLDatabase::Execute(const char* cmd, ...)
{
	va_list args;

	va_start(args, cmd);

	vsnprintf(m_buffer, CMD_BUFFER_SIZE, cmd, args);
	   
	va_end(args);

	return Query(m_buffer);
}

bool SQLDatabase::Query(const char* cmd)
{
	if (m_nonBlockingMode && !m_pThreadData)
		m_nonBlockingMode = CreateQueryThread();

	if (m_nonBlockingMode && m_pThreadData->push_back(cmd))
		return true; //ie, query will be handled by thread
	
	// Execute the query now
	if (::mysql_query(m_pMySql, cmd) != 0)
	{
		showDBError("Cannot process sql query");
		return false;
	}

	return true;
}

/*!
	Gets the result of the last query that was executed.
*/
SQLQueryResult SQLDatabase::StoreQueryResult()
{
	// In non-blocking mode, there might be pending queries
	// that must be executed before storing the results
	if (m_nonBlockingMode)
		m_pThreadData->wait_for_queries();

	return SQLQueryResult(mysql_store_result(m_pMySql));
}

SQLDataRow SQLDatabase::FetchDataRow(SQLQueryResult& res)
{
	return (res) ? mysql_fetch_row(res) : NULL;
}

