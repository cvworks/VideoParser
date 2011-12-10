/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <list>
#include "STLMatrix.h"
#include "BasicUtils.h"

/*! 
	Table of multi-line strings.
	
	Internally, the zero row and column correspond to the row and colomn
	header respectively. When specifyin the size of the table, it is not 
	nesserary to add 1 for the row and column header. This is done 
	by the constructor. Similar, the Get(i,j,false) function adds 1 
	to i and j so that Get(0,0) maps to the (1,1) position and so on. The
	Get(i,j,true) does not add 1, and so it can be used to access the 
	header cells. The AddTextLine(.., bool) functions work similarly.
*/
class MultiLineTable : public STLMatrix<std::vector<std::string>>
{
public:
	typedef std::vector<std::string> CELL;
	typedef std::vector<CELL> CELL_ARRAY;
	typedef STLMatrix<CELL> BASE_CLASS;

protected:
	unsigned m_colWidth;

	enum {DEF_COL_WIDTH = 0};

	unsigned FindMaxColWidth() const;

public:
	MultiLineTable()
	{
		m_colWidth = DEF_COL_WIDTH;
	}

	MultiLineTable(unsigned ni, unsigned nj)
		: BASE_CLASS(ni + 1, nj + 1)
	{
		m_colWidth = DEF_COL_WIDTH;
	}

	MultiLineTable(unsigned ni, unsigned nj, const CELL& val)
		: BASE_CLASS(ni + 1, nj + 1, val)
	{
		m_colWidth = DEF_COL_WIDTH;
	}

	void operator=(const MultiLineTable& rhs)
	{
		BASE_CLASS::operator=(rhs);

		m_colWidth = rhs.m_colWidth;
	}

	void SetRowHeader(const CELL_ARRAY& rowHeader)
	{
		ASSERT(ni() == rowHeader.size());

		for (unsigned i = 0; i < ni(); ++i)
			Get(i, 0, true) = rowHeader[i];
	}

	void SetColumnHeader(const CELL_ARRAY& colHeader)
	{
		ASSERT(nj() == colHeader.size());

		for (unsigned j = 0; j < nj(); ++j)
			Get(0, j, true) = colHeader[j];
	}

	void SetColumnWidth(unsigned colWidth)
	{
		m_colWidth = colWidth;
	}

	CELL& Get(unsigned i, unsigned j, 
		bool includeHeaders)
	{
		if (!includeHeaders)
		{
			++i;
			++j;
		}
		
		return BASE_CLASS::operator()(i, j);
	}

	const CELL& Get(unsigned i, unsigned j, 
		bool includeHeaders) const
	{
		if (!includeHeaders)
		{
			++i;
			++j;
		}

		return BASE_CLASS::operator()(i, j);
	}

	CELL& operator()(unsigned i, unsigned j)
	{
		return Get(i, j, false);
	}

	const CELL& operator()(unsigned i, unsigned j) const
	{
		return Get(i, j, false);
	}

	void Print(std::ostream& os) const;

	void AddRowHeader(unsigned i, const std::string& str)
	{
		AddText(i + 1, 0, str, true);
	}

	void AddColHeader(unsigned j, const std::string& str)
	{
		AddText(0, j + 1, str, true);
	}

	void AddCornerHeader(const std::string& str)
	{
		AddText(0, 0, str, true);
	}

	void AddText(unsigned i, unsigned j, const std::string& str,
		bool includeHeaders = false)
	{
		Get(i, j, includeHeaders).push_back(str);
	}

	template<class T> void AddValue(unsigned i, unsigned j,
		const T& val, bool includeHeaders = false)
	{
		std::ostringstream oss;

		oss << val;

		AddText(i, j, oss.str(), includeHeaders);
	}

	template<class T> void AddValues(unsigned i, unsigned j,
		const std::vector<T>& vec, const char* sep = ",", 
		bool includeHeaders = false)
	{
		if (vec.empty())
		{
			AddText(i, j, std::string(" "), includeHeaders);
		}
		else
		{
			std::ostringstream oss;

			auto it = vec.begin();

			oss << *it;

			for (++it; it != vec.end(); ++it)
				oss << sep << *it;
		
			AddText(i, j, oss.str(), includeHeaders);
		}
	}

	template<class T> void AddValues(unsigned i, unsigned j,
		const std::list<T>& vec, const char* sep = ",", 
		bool includeHeaders = false)
	{
		if (vec.empty())
		{
			AddText(i, j, std::string(" "), includeHeaders);
		}
		else
		{
			std::ostringstream oss;

			auto it = vec.begin();

			oss << *it;

			for (++it; it != vec.end(); ++it)
				oss << sep << *it;
		
			AddText(i, j, oss.str(), includeHeaders);
		}
	}
};
