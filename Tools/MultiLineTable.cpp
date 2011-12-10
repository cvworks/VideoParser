/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "MultiLineTable.h"
#include "STLPrint.h"

//! Prints a horizontal table boundary
void PrintHorizontalLine(std::ostream& os, unsigned nj, unsigned colWidth)
{
	int tableWidth = (colWidth + 3) * (nj - 1);

	// Skip row header space
	os << std::setfill(' ') << std::setw(colWidth + 1) << ' ';

	// Print line
	os << std::setfill('-') << std::setw(tableWidth + 1) << '-';

	// Restor the defautl fill
	os << std::setfill(' ') << "\n";
}

unsigned MultiLineTable::FindMaxColWidth() const
{
	unsigned colWidth = 0;

	for (unsigned i = 0; i < ni(); ++i)
	{
		for (unsigned j = 0; j < nj(); ++j)
		{
			const CELL& c = Get(i, j, true);

			for (unsigned n = 0; n < c.size(); ++n)
			{
				if (colWidth < c[n].size())
					colWidth = c[n].size();
			}
		}
	}

	return colWidth;
}

//! Prints the table.
void MultiLineTable::Print(std::ostream& os) const
{
	unsigned numLines, colWidth;

	if (m_colWidth == 0)
		colWidth = FindMaxColWidth();
	else
		colWidth = m_colWidth;

	// For each row
	for (unsigned i = 0; i < ni(); ++i)
	{
		// Reset max num lines in row
		numLines = 0;

		// Count num of lines for i'th row
		for (unsigned j = 0; j < nj(); ++j)
		{
			if (numLines < Get(i, j, true).size())
				numLines = Get(i, j, true).size();
		}

		// Process all lines 
		for (unsigned n = 0; n < numLines; ++n)
		{
			// For each column
			for (unsigned j = 0; j < nj(); ++j)
			{
				if (j > 0)
				{
					if (i > 0)
						os << "| ";
					else
						os << "  "; // leave similar spacing
				}

				os << std::setw(colWidth);

				const CELL& c = Get(i, j, true);

				if (n < c.size() && !c[n].empty())
					os << c[n];
				else
					os << " ";

				os << " ";

				if (i > 0 && j + 1 == nj())
					os << "|";
			}

			os << "\n";
		}

		PrintHorizontalLine(os, nj(), colWidth);
	}
}
