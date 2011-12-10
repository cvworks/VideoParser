/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <string>
#include <list>
#include "SafeQueue.h"

struct st_mysql;
struct st_mysql_res;

//typedef st_mysql_res* SQLQueryResult;
typedef char** SQLDataRow;

namespace vpl {

void* ProcessQueries(void* pParam);

class SQLQueryResult
{
	st_mysql_res* m_pResult;

	//SQLQueryResult& operator=(const SQLQueryResult& rhs) = delete;
	SQLQueryResult& operator=(const SQLQueryResult& rhs);
	SQLQueryResult(const SQLQueryResult& rhs);

public:
	SQLQueryResult(st_mysql_res* pRes = NULL) 
	{ 
		m_pResult = pRes; 
	}

	SQLQueryResult(SQLQueryResult&& rhs)
	{
		m_pResult = rhs.m_pResult;
		rhs.m_pResult = NULL;
	}

	unsigned FieldCount();

	~SQLQueryResult();

	//! Deletes results right away instead of waiting for destructor
	void Free();

	operator st_mysql_res*() { return m_pResult; }

	//operator bool() const { return m_pResult != NULL; }
};

class SQLThreadData
{
public:
	enum STATE { READY, RUNNING, STOP_REQUESTED, STOPPED };

private:
	st_mysql* pMySql;
	SafeQueue<std::string, 2048> queries;
	volatile STATE state;

	friend void* vpl::ProcessQueries(void* pParam);

public:
	SQLThreadData(st_mysql* p)             { pMySql = p; state = READY; }
	st_mysql* sql()                        { return pMySql; }
	void request_stop()                    { state = STOP_REQUESTED; }
	STATE get_state() const                { return state; }
	bool empty() const                     { return queries.empty(); }

	bool push_back(const std::string& val);
	void wait_for_queries();
};

/*!
	An SQL database.
*/
class SQLDatabase
{		
protected:
	enum {CMD_BUFFER_SIZE = 1024};

	std::string m_name;
	st_mysql* m_pMySql;          //!< pointer to connection handler
	bool m_isOpen;

	bool m_nonBlockingMode;
	SQLThreadData* m_pThreadData;
	unsigned long m_threadId;

	char m_buffer[CMD_BUFFER_SIZE];

protected:
	bool CreateQueryThread();

public:
	enum {INVALID_ID = -1};

public:
	SQLDatabase();
	~SQLDatabase();

	std::string Name() const
	{
		return m_name;
	}

	bool Open(std::string name, bool nonBlockingMode);
	void Close();

	bool IsOpen() const
	{
		return m_isOpen;
	}

	bool Execute(const char* cmd, ...);

	bool Query(const char* cmd);

	SQLQueryResult StoreQueryResult();

	SQLDataRow FetchDataRow(SQLQueryResult& res);

	bool FetchData(SQLQueryResult& res, int* value)
	{
		SQLDataRow row = FetchDataRow(res);

		if (row && row[0])
		{
			*value = atoi(row[0]);
			return true;
		}
	
		return false;
	}

	void showDBError(std::string msg = std::string()) const;
};

} //namespace vpl
