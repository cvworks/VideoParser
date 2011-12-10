/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <sstream>
#include <array>
#include "STLUtils.h"

namespace vpl {

/*!
	Sparse truth table with probabilities. If the probability of a
	state is to set explicitly in the table, it is assumed to be zero.
*/
template<unsigned NUM_VARS>
class TruthTable : public std::map<std::array<unsigned char, NUM_VARS>, double>
{
public:
	typedef std::array<unsigned char, NUM_VARS> States;

	typedef std::map<States, double> BASE_CLASS;

public:

	unsigned NumVariables() const
	{
		return NUM_VARS;
	}
	
	double operator[](const States& states) const
	{
		ASSERT(states.size() == NUM_VARS);

		auto it = find(states);

		return (it == end()) ? 0 : it->second;
	}

	void AddStates(const States& states, const double& pr)
	{
		BASE_CLASS::operator[](states) = pr;
	}

	static bool ReadRow(std::istringstream& is, 
		typename TruthTable<NUM_VARS>::States& st, double& pr)
	{
		char ch;
		std::string buffer;

		// Read all fields and their separators
		for (unsigned i = 0; i < st.size() + 1; ++i)
		{
			buffer.clear();

			while (!is.eof()) 
			{
				is >> ch;
			
				if (ch != ',' && ch != ';' && ch != '}')
					buffer.push_back(ch);
				else
					break;
			}

			if (i < st.size() && ch == ',')
				st[i] = (unsigned char) atoi(buffer.c_str());
			else if (i == st.size() && (ch == ';' || ch == '}'))
				pr = atof(buffer.c_str());
			else
				THROW_BASIC_EXCEPTION("TruthTable has missing field(s)");	
		}

		return ch == '}';
	}
};

template <unsigned NUM_VARS>
std::ostringstream& operator<<(std::ostringstream& os, const TruthTable<NUM_VARS>& tt)
{
	std::string st;

	os << "{";
		
	for (auto it = tt.begin(); it != tt.end(); ++it)
	{
		for (unsigned j = 0; j < it->first.size(); ++j)
		{
			st += (int)it->first.at(j);

			os << st;

			if (j + 1 < it->first.size())
				 os << ",";
		}

		 os << ";";
	}
		
	os << "}"; 

	return os;
}
		
template <unsigned NUM_VARS>
std::istringstream& operator>>(std::istringstream& is, TruthTable<NUM_VARS>& tt)
{
	char ch;
	TruthTable<NUM_VARS>::States st;
	double pr;

	// Read the 'begin bracked'
	is >> ch;

	if (ch != '{')
		THROW_BASIC_EXCEPTION("Invalid TruthTable 'begin bracket'");

	unsigned maxNumRows = (unsigned) pow(2.0, (int)NUM_VARS);
	bool last = false;

	for (unsigned i = 0; i < maxNumRows; i++)
	{
		last = TruthTable<NUM_VARS>::ReadRow(is, st, pr);

		tt.AddStates(st, pr);

		if (last)
			break;
	}
	
	// The last character read must be the 'end bracket'
	if (!last)
		THROW_BASIC_EXCEPTION("Invalid TruthTable 'end bracket'");

	return is;
}

} // namespace vpl
