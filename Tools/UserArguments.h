/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _USER_ARGUMENTS_
#define _USER_ARGUMENTS_

#include <sstream>
#include "ParamFile.h"
#include "Tuple.h"

namespace vpl {

/*!
	The UserArguments class inherits from the ParamFile class. It
	adds additional member variables and functions to store and
	display the usage information of each possible user argument.
	An important difference between UserArguments and
	ParamFile is that UserArguments considers that ALL parameters
	are optional, while ParamFile throws an exception if a 
	parameter is not provided (which must be caught if the param
	is optional). 

	General usage:

	- To read any argument type T using its text-based operator>> use
	  ReadArg(T). Note that T might be any type that defines:

	  friend std::istringstream& operator>>(std::istringstream& is, T& p)

	  and

	  friend std::ostringstream& operator<<(std::ostringstream& os, const Params& p)

    - To read a boolean argument provided as a {"no", "yes"} values use
	  ReadBoolArg(bool). Note that yes/no are lower case.

    - To read any predefined set of string values, use 
	  ReadArg(StrArray str, int), where str = {"enum0", "enum1", ...}.

	- To read a list of arguments instead of a single one, use the 
	  same format above but append an 's' to the function name. e.g, 
	  ReadArgs(std::list<T>).

	  Example of how to read tuples:

		typedef vpl::Tuple<std::string, 4> Params;
		typedef std::list<Params> ParamList;

		ParamList defVal, pl;
		Params t;

		g_userArgs.ReadArgs("FieldName", "PropertyName", "Reads tuples", defVal, &pl);

	  @see ParamFile
*/
class UserArguments : public ParamFile
{
protected:
	struct UsageInfo
	{
		std::string m_usageMsg;
		std::string m_defVals;
		std::string m_minVal, m_maxVal;
		StrList m_enumVals;

		void SetUsage(const std::string& usageMsg, const std::string& defVals)
		{
			m_usageMsg = usageMsg;
			m_defVals  = defVals;
		}

		void SetRange(const std::string& minVal, const std::string& maxVal,
			const StrList& enumVals)
		{
			m_minVal   = minVal;
			m_maxVal   = maxVal;
			m_enumVals = enumVals;
		}

		void operator=(const UsageInfo& rhs)
		{
			SetUsage(rhs.m_usageMsg, rhs.m_defVals);
			SetRange(rhs.m_minVal, rhs.m_maxVal, rhs.m_enumVals);
		}
	};

	typedef std::list< std::pair<std::string, const UsageInfo*> > UsageInfoList;
	typedef std::map<Keyword, UsageInfo> PropertyUsageMap;
	typedef std::map<Keyword, PropertyUsageMap> FieldUsageMap;

	FieldUsageMap m_usageInfo;

	UsageInfo& AddUsageField(const Keyword& fieldKey, 
		const Keyword& propKey)
	{
		if (propKey.empty())
			return m_usageInfo[fieldKey][UNNAMED_PROPERTY];
		else
			return m_usageInfo[fieldKey][propKey];
	}

public:
	UserArguments(const char* szFilename = NULL)
		: ParamFile(szFilename)
	{

	}

	void Clear()
	{
		ParamFile::Clear();

		m_usageInfo.clear();
	}

	bool ReadParameters(const char* szFilename)
	{
		Clear();

		return ParamFile::ReadParameters(szFilename);
	}

	///////////////////////////////////////////////////////////////////////
	// Usage info functions

	/*!
		Stores the usage information associated with a field:property.

		If szUsageMsg is NULL, no usage info is added.
	*/
	template <class T> void AddUsageInfo(const Keyword& fieldKey, 
		const Keyword& propKey, const char* szUsageMsg, 
		const std::list<T>& defVals)
	{
		if (szUsageMsg == NULL)
			return;

		std::ostringstream oss;
		std::list<T>::const_iterator it;

		// Collect all default values as a string
		for (it = defVals.begin(); it != defVals.end(); ++it)
		{
			oss << *it << ' ';
		}

		AddUsageField(fieldKey, propKey).SetUsage(szUsageMsg, oss.str()); 
	}

	/*!
		Sets range as a [min, max] interval.
	*/
	template <class T> void AddRangeInfo(const Keyword& fieldKey, 
		const Keyword& propKey, const T& minVal, const T& maxVal,
		bool bAddMin = true, bool bAddMax = true)
	{
		std::ostringstream oss0, oss1;
		StrList sl; // empty list
		
		if (bAddMin)
			oss0 << minVal;

		if (bAddMax)
			oss1 << maxVal;
		
		AddUsageField(fieldKey, propKey).SetRange(oss0.str(), oss1.str(), sl);
	}

	/*!
		Sets range as a list of possible enum values.
	*/
	void AddRangeInfo(const Keyword& fieldKey, const Keyword& propKey, 
		const StrArray& enumLabels)
	{
		StrList enumVals;
		std::string s; // empty string

		for (unsigned int i = 0; i < enumLabels.size(); i++)
			enumVals.push_back(enumLabels[i]);

		AddUsageField(fieldKey, propKey).SetRange(s, s, enumVals);
	}

	///////////////////////////////////////////////////////////////////////
	// Argument modifying functions
	void UpdateArgs(const Keyword& fieldKey, const Keyword& propKey, 
		const StrList& newValues)
	{
		if (HasField(fieldKey, propKey))
			AddField(fieldKey, propKey, newValues);
		else
			StreamError("Invalid field: " << fieldKey << "::" << propKey);
	}

	///////////////////////////////////////////////////////////////////////
	// Argument reading functions

	/*!
		Reads the values associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		values. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	template <class T> bool ReadArgs(const Keyword& fieldKey, 
		const Keyword& propKey, const char* szUsageMsg, 
		const std::list<T>& defVals, std::list<T>* pValues)
	{
		AddUsageInfo(fieldKey, propKey, szUsageMsg, defVals);

		if (HasField(fieldKey, propKey))
		{
			try {
				GetTypedValues(fieldKey, propKey, *pValues);
			}
			catch(BasicException e) 
			{
				ShowParamUsage(fieldKey, propKey, 
					std::string("?"), StrArray());

				e.Print();

				// Set a valid value anyhow
				*pValues = defVals;

				return false;
			}
		}
		else
		{
			*pValues = defVals;
			AddTypedField(fieldKey, propKey, *pValues);
		}

		return true;
	}

	/*!
		Reads the values associated with the field 'fieldKey'. If 
		the field does not exist, it is added with the provided 
		default values. If there is an error reading the field, 
		the usage message is printed.
	*/
	template <class T> bool ReadArgs(const Keyword& fieldKey, 
		const char* szUsageMsg, const std::list<T>& defVals, 
		std::list<T>* pValues)
	{
		return ReadArgs(fieldKey, Keyword(), szUsageMsg, defVals, 
			pValues);
	}

	/*!
		Reads the value associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		value. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	template <class T> bool ReadArg(const Keyword& fieldKey, 
		const Keyword& propKey, const char* szUsageMsg, 
		const T& defVal, T* pValue)
	{
		std::list<T> defVals(1, defVal), values;

		if (!ReadArgs(fieldKey, propKey, szUsageMsg, defVals, &values))
			return false;

		if (values.size() != 1)
		{
			std::string strErr = fieldKey;

			if (!propKey.empty())
				strErr += ":" + propKey;

			ShowError1("Too many parameters for field", strErr);

			ShowUsage(szUsageMsg);

			return false;
		}

		*pValue = values.front();

		return true;
	}

	/*!
		Reads the value associated with the field 'fieldKey'. If 
		the field does not exist, it is added with the provided 
		default value. If there is an error reading the field, 
		the usage message is printed.
	*/
	template <class T> bool ReadArg(const Keyword& fieldKey, 
		const char* szUsageMsg, const T& defVal, T* pValue)
	{
		return ReadArg(fieldKey, Keyword(), szUsageMsg, defVal, pValue);
	}

	/*!
		Reads a list of tuples in which each dimension corresponds
		to an ENUMERATION and validates each enum.
		
		This is a method for reading a very specialized type of argument.

		@return true iff all enum arguments are valid.
	*/
	template <typename T, unsigned DIM> 
	bool ReadEnumTupleArgs(const Keyword& fieldKey, 
		const Keyword& propKey, const std::vector<StrArray>& enumLabels,
		const char* szUsageMsg, const std::list<Tuple<T, DIM, '(', ')'>>& defVals, 
		std::list<Tuple<T, DIM, '(', ')'>>* pValues)
	{
		ASSERT(enumLabels.size() == DIM);

		// Read list of tuples
		ReadArgs(fieldKey, propKey, szUsageMsg, defVals, pValues);

		// Validate the arguments of each tuple
		for (auto valIt = pValues->begin(); valIt != pValues->end(); ++valIt)
		{
			for (unsigned i = 0; i < DIM; i++)
			{
				if (!contains(enumLabels.at(i), valIt->at(i)))
				{
					ShowParamUsage(fieldKey, propKey, szUsageMsg, 
						enumLabels.at(i));

					return false;
				}
			}
		}

		return true;
	}

	/*!
		Reads the ENUMERATION values associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		values. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	bool ReadArgs(const Keyword& fieldKey, 
		const Keyword& propKey, const StrArray& enumLabels,
		const char* szUsageMsg, const IntList& defVals, IntList* pValues)
	{
		// Get an ordered list of the string labels associated with the  
		// enumeration indices in 'defVals'
		StrList strDefVals = GetEnumLabels(defVals, enumLabels);
		StrList strValues;

		// Read list of string with given default values
		if (!ReadArgs(fieldKey, propKey, szUsageMsg, strDefVals, &strValues))
			return false;

		// Add the enums to the range info
		AddRangeInfo(fieldKey, propKey, enumLabels);

		// Get the indices of the values in the array 'enumLabels', 
		// this throws an exception of any string is not a valid enum.
		*pValues = GetEnumIndices(strValues, enumLabels);

		return true;
	}

	/*!
		Reads the ENUMERATION values associated with the field 'fieldKey'. If 
		the field does not exist, it is added with the provided 
		default values. If there is an error reading the field, 
		the usage message is printed.
	*/
	bool ReadArgs(const Keyword& fieldKey, const StrArray& enumLabels,
		const char* szUsageMsg, const IntList& defVals, IntList* pValues)
	{
		return ReadArgs(fieldKey, Keyword(), enumLabels, szUsageMsg, 
			defVals, pValues);
	}

	/*!
		Reads the ENUMERATION value associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		value. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	bool ReadArg(const Keyword& fieldKey, 
		const Keyword& propKey, const StrArray& enumLabels,
		const char* szUsageMsg, int defVal, int* pValue)
	{
		std::string strDefVal = GetEnumLabel(defVal, enumLabels);
		std::string strValue;

		AddRangeInfo(fieldKey, propKey, enumLabels);

		if (!ReadArg(fieldKey, propKey, szUsageMsg, strDefVal, &strValue))
			return false;

		try {
			*pValue = GetEnumIndex(strValue, enumLabels);
		}
		catch(BasicException e) 
		{
			//e.Print();
			ShowParamUsage(fieldKey, propKey, strValue, enumLabels);

			// Set a valid value anyhow
			*pValue = defVal;

			return false;
		}

		return true;
	}

	/*!
		Reads the ENUMERATION value associated with the field 'fieldKey'. If 
		the field does not exist, it is added with the provided 
		default value. If there is an error reading the field, 
		the usage message is printed.
	*/
	bool ReadArg(const Keyword& fieldKey, const StrArray& enumLabels,
		const char* szUsageMsg, int defVal, int* pValue)
	{
		return ReadArg(fieldKey, Keyword(), enumLabels, 
			szUsageMsg, defVal, pValue);
	}

	/*!
		Reads the BOOLEAN value associated with the property 'propKey'
		of the field 'fieldKey'. If the property or the field 
		do not exist, they are added with the provided default
		value. If there is an error reading the field:property, 
		the usage message is printed.
	*/
	bool ReadBoolArg(const Keyword& fieldKey, 
		const Keyword& propKey, const char* szUsageMsg, 
		bool defVal, bool* pValue)
	{
		int val;
		
		if (!ReadArg(fieldKey, propKey, Tokenize("no, yes"),
			szUsageMsg, (defVal) ? 1 : 0, &val))
		{
			return false;
		}

		*pValue = (val != 0);

		return true;
	}

	/*!
		Reads the BOOLEAN value associated with the field 'fieldKey'. If 
		the field does not exist, it is added with the provided 
		default value. If there is an error reading the field, 
		the usage message is printed.
	*/
	bool ReadBoolArg(const Keyword& fieldKey,
		const char* szUsageMsg, bool defVal, bool* pValue)
	{
		return ReadBoolArg(fieldKey, Keyword(),	szUsageMsg, 
			defVal, pValue);
	}

	///////////////////////////////////////////////////////////////////////
	// Check value functions

	/*!
		Throws an exception	if field and property do not exist.
	*/
	template <class T> void CheckValues(const Keyword& fieldKey, 
		const Keyword& propKey, const T& minVal, const T& maxVal, 
		bool bCheckMin = true, bool bCheckMax = true)
	{
		std::list<T> values;

		AddRangeInfo(fieldKey, propKey, minVal, maxVal, 
			bCheckMin, bCheckMax);

		// The following call may throw an exception
		GetBoundedTypedValues(fieldKey, propKey, values, 
			minVal, maxVal, bCheckMin, bCheckMax); 
	}

	/*!
		Throws an exception	if field and property do not exist.
	*/
	template <class T> void CheckMinValues(const Keyword& fieldKey, 
		const Keyword& propKey, const T& minVal)
	{
		CheckValues(fieldKey, propKey, minVal, minVal, true, false);
	}

	/*!
		Throws an exception	if field and property do not exist.
	*/
	template <class T> void CheckMaxValues(const Keyword& fieldKey, 
		const Keyword& propKey, const T& maxVal)
	{
		CheckValues(fieldKey, propKey, maxVal, maxVal, false, true);
	}

	///////////////////////////////////////////////////////////////////////
	// Info printing functions

	void GetUsageList(UsageInfoList& uil) const;
	void ShowArgumentUsage(std::ostream& os) const;
	std::string GetAllDefaultValues() const;
	std::string GetAllValueOptions() const;

	int ReadVersionNumber(int maxVersion = 0) const;
};

} // namespace vpl

#endif //_USER_ARGUMENTS_