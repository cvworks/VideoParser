/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _PARAM_FILE_
#define _PARAM_FILE_

#include <list>
#include <map>
#include <utility>
#include <set>

#include <Tools/BasicUtils.h>
#include <Tools/STLUtils.h>
#include <Tools/Exceptions.h>
#include <Tools/Serialization.h>

// Disable "decorated name length exceeded, name was truncated" message
#pragma warning(disable:4503)

namespace vpl {

/*!
	Reads fields from a text file. 

	A field is defined as a keyword and	a set of associated properties. 
	There are two types of properties, named and unnamed. For example, 
	in the following assignment,

	keyword = val0 val1 val2 ...

	the values 'val0 val1 val2 ...' define an unnamed property of the field 'keyword'.

	Named properties are defined as follows:

	keyword {
	property_name0 = val0 val1 val2 ...
	property_name1 = val0 val1 val2 ...
	property_name2 = val0 val1 val2 ...
	...
	}

	The unnamed properties are treated as properties named "_unnamed". That is, 
	the commands (a) and (b) bellow are equivalent.

	(a) keyword = val0 val1 val2 ...

	(b) keyword {
	    _unnamed = val0 val1 val2 ...
		}

	The text file can contain comments (lines that begin with #).

	Commands that span multiple lines must have all but the last line
	ending with a backslash character (ie, '\'). ie, like a multiline
	#define statement in c.

	ParamFile throws an exception if a parameter is not provided, 
	which must be caught if the parameter is optional. In contrast,
    the derived class UserArguments assumes that all parameters are
	optional and may be more appropriate for some tasks.

	Example of how to read tuples:

	typedef vpl::Tuple<std::string, 4> Params;
	typedef std::list<Params> ParamList;

	ParamList pl;

	try {
		paramFile.GetTypedValues("FieldName", "PropertyName", &pl);
	}
	catch(BasicException e) 
	{
		// the 'FieldName/PropertyName' does not exist
	}

	@see UserArguments
*/
class ParamFile
{
public:
	typedef std::string Keyword;
	typedef std::map<Keyword, StrList> PropertyMap;
	typedef std::map<Keyword, PropertyMap> FieldMap;
	typedef std::set<Keyword> FieldKeySet;

	static const Keyword UNNAMED_PROPERTY;

protected:
	std::string m_inputFilename;
	FieldMap m_fields;
	FieldKeySet m_serializableFields; //!< Selected fields to (de)serialize
	int m_nErrors;
	
	void AddField(const Keyword& fieldKey, const Keyword& propKey, 
		const StrList& values)
	{
		m_fields[fieldKey][propKey.empty() ?
			UNNAMED_PROPERTY : propKey] = values;
	}

	template <class T> void AddTypedField(const Keyword& fieldKey, 
		const Keyword& propKey, const std::list<T>& valueList)
	{
		std::list<T>::const_iterator it;
		std::ostringstream oss;
		StrList values;

		for (it = valueList.begin(); it != valueList.end(); ++it)
		{
			oss << *it;
			values.push_back(oss.str());
			oss.clear();
		}

		AddField(fieldKey, propKey, values);
	}

public:
	ParamFile(const char* szFilename = NULL) 
	{ 
		if (szFilename) 
			ReadParameters(szFilename); 
	}

	void Clear()
	{
		m_inputFilename.clear();
		m_fields.clear();
		m_serializableFields.clear();
		m_nErrors = 0;
	}

	int GetErrorNumber() const 
	{ 
		return m_nErrors; 
	}

	void ShowParamUsage(const Keyword& fieldKey, const Keyword& propKey, 
		const std::string& value, const StrArray& enumLabels) const
	{
		std::string msg = "The file " + m_inputFilename +
			" assigns the invalid value '" + value + "' to ";
		
		if (!propKey.empty())
			msg += "the property " + propKey + " in ";

		msg += "field " + fieldKey;

		ShowError(msg);

		msg = "Possible valid values are '" + 
			ConvertValuesToString(enumLabels) + "'\n";

		ShowUsage(msg);
	}
	
	int GetValueCount(const Keyword& fieldKey, const Keyword& propKey) const 
	{ 
		return GetStrValues(fieldKey, propKey).size(); 
	}

	/*!
		Counts the number of field names that start with the
		given prefix.
	*/
	unsigned CountFieldPrefix(const Keyword& fieldKeyPrefix) const
	{
		const unsigned sz = fieldKeyPrefix.size();
		unsigned count = 0;

		for (auto fieldIt = m_fields.begin(); fieldIt != m_fields.end(); ++fieldIt)
		{
			if (fieldIt->first.compare(0, sz, fieldKeyPrefix) == 0)
				++count;
		}

		return count;
	}

	/*!
		Counts the number of property names that start with the
		given prefix and belong to the provided field.
	*/
	unsigned CountPropertyPrefix(const Keyword& fieldKey, 
		const Keyword& propKeyPrefix) const
	{
		FieldMap::const_iterator fieldIt = m_fields.find(fieldKey);
		unsigned count = 0;

		if (fieldIt != m_fields.end())
		{
			const unsigned sz = propKeyPrefix.size();
		
			for (auto propIt = fieldIt->second.begin(); 
				propIt != fieldIt->second.end(); ++propIt)
			{
				if (propIt->first.compare(0, sz, propKeyPrefix) == 0)
					++count;
			}
		}

		return count;
	}

	/*!
		Finds the minimum and maximum integer numbers following
		the given prefix in all field names. eg, if there are fields

		name1xyz { }
		name2xyy { }
		other3yyy { } 
		name7zyx { }

		Then, GetFieldPrefixMinMax(string("name"), &m0, &m1);

		has m0 = 1 and m1 = 7

		@return true if at least one field with the given prefix exists.
		        Otherwise, it returns false and sets min as greater than max.
	*/
	bool GetFieldPrefixMinMax(const Keyword& fieldKeyPrefix, 
		int* pMin, int *pMax) const
	{
		const unsigned sz = fieldKeyPrefix.size();
		int n;
		
		*pMin = INT_MAX;
		*pMax = INT_MIN;

		for (auto fieldIt = m_fields.begin(); fieldIt != m_fields.end(); ++fieldIt)
		{
			if (fieldIt->first.compare(0, sz, fieldKeyPrefix) == 0)
			{
				n = atoi(fieldIt->first.substr(sz).c_str());

				if (n < *pMin) *pMin = n;
				if (n > *pMax) *pMax = n;
			}
		}

		return (*pMin <= *pMax);
	}

	/*!
		Finds the minimum and maximum integer numbers following
		the given prefix in all property names under the given field. 
		eg, if there are properties

		fn { 
		  prop1xyz = ...
		  other1xyz = ...
		  prop7zyx = ...
		}

		Then, GetPropertyPrefixMinMax(string("fn"), string("prop"), &m0, &m1);

		has m0 = 1 and m1 = 7

		@return true if at least one property with the given prefix exists.
		        Otherwise, it returns false and sets min as greater than max.
	*/
	bool GetPropertyPrefixMinMax(const Keyword& fieldKey, 
		const Keyword& propKeyPrefix, int* pMin, int *pMax) const
	{
		FieldMap::const_iterator fieldIt = m_fields.find(fieldKey);
		int n;
		
		*pMin = INT_MAX;
		*pMax = INT_MIN;

		if (fieldIt != m_fields.end())
		{
			const unsigned sz = propKeyPrefix.size();
		
			for (auto propIt = fieldIt->second.begin(); 
				propIt != fieldIt->second.end(); ++propIt)
			{
				if (propIt->first.compare(0, sz, propKeyPrefix) == 0)
				{
					n = atoi(propIt->first.substr(sz).c_str());

					if (n < *pMin) *pMin = n;
					if (n > *pMax) *pMax = n;
				}
			}
		}

		return (*pMin <= *pMax);
	}

	std::string GetInputFilename() const
	{
		return m_inputFilename;
	}

	bool HasField(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const
	{
		FieldMap::const_iterator fieldIt = m_fields.find(fieldKey);

		if (fieldIt == m_fields.end())
			return false;

		PropertyMap::const_iterator propIt;
		
		propIt = fieldIt->second.find(
			propKey.empty() ? UNNAMED_PROPERTY : propKey);

		return (propIt != fieldIt->second.end());
	}

	bool ReadParameters(const char* szFilename);
	
	std::string GetAllFieldsAndValues() const;

	/*!
		Creates a string will all the values in the given list or
		array of strings. i.e., T must either be a StrList or a
		StrArray.
	*/
	template <class T> 
	std::string ConvertValuesToString(const T& values) const
	{
		T::const_iterator it;
		std::string strVals;

		for (it = values.begin(); it != values.end(); ++it)
			strVals += *it + " ";

		return strVals;
	}

	///////////////////////////////////////////////////////////////////////
	// Read multiple values

	const StrList& GetStrValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const;

	/*!
		Returns the index of each value in the array 'enumLabels' 
		of valid string values.
	*/
	IntList GetEnumValues(const Keyword& fieldKey, 
		const Keyword& propKey, const StrArray& enumLabels) const
	{
		const StrList& values = GetStrValues(fieldKey, propKey);
		
		return GetEnumIndices(values, enumLabels);
	}

	/*!
		Converts each string value to a 'string'. While this seems redundant,
		it is necessary as an specialization of the GetTypedValues<T>() 
		function, which does not allow for strings with spaces in them.

		The output list 'valueList' is cleared before addign the values.
	*/
	void GetTypedValues(const Keyword& fieldKey, 
		const Keyword& propKey, StrList& valueList) const
	{
		valueList = GetStrValues(fieldKey, propKey);
	}

	/*!
		Converts each string value to a specified type T by reading it
		using an std::istringstream. This means that the operator>>
		must be defined for the type T.

		The output list 'valueList' is cleared before addign the values.
	*/
	template <class T> void GetTypedValues(const Keyword& fieldKey, 
		const Keyword& propKey, std::list<T>& valueList) const
	{
		const StrList& strValues = GetStrValues(fieldKey, propKey);
		StrList::const_iterator it;
		T val;

		valueList.clear();

		for (it = strValues.begin(); it != strValues.end(); ++it)
		{
			std::istringstream iss(*it);
			
			iss >> val;

			valueList.push_back(val);
		}
	}

	/*!
		Gets the typed values and checks if each value is within
		a valid range. Either pMinVal or pMaxVal may be NULL.
	*/
	template <class T> void GetBoundedTypedValues(
		const Keyword& fieldKey, const Keyword& propKey, 
		std::list<T>& valueList, const T& minVal, const T& maxVal, 
		bool bCheckMin = true, bool bCheckMax = true) const
	{
		GetTypedValues(fieldKey, propKey, valueList);

		for (std::list<T>::iterator it = valueList.begin(); 
			it != valueList.end(); ++it) 
		{
			if (bCheckMin && *it < minVal)
				THROW_BASIC_EXCEPTION("Value smaller than specified bound");

			if (bCheckMax && *it > maxVal)
				THROW_BASIC_EXCEPTION("Value greater than specified bound");
		}
	}

	IntList GetBoolValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const
	{
		StrArray enumLabels(2);

		enumLabels[0] = "no";
		enumLabels[1] = "yes";

		return GetEnumValues(fieldKey, propKey, enumLabels);
	}

	IntList GetIntValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		int minVal = 0, int maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		IntList valList;

		GetBoundedTypedValues(fieldKey, propKey, valList, 
			minVal, maxVal, bCheckMin, bCheckMax);

		return valList;
	}

	UIntList GetUIntValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		unsigned minVal = 0, unsigned maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		UIntList valList;

		GetBoundedTypedValues(fieldKey, propKey, valList, 
			minVal, maxVal, bCheckMin, bCheckMax);

		return valList;
	}

	FloatList GetFloatValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		float minVal = 0, float maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		FloatList valList;

		GetBoundedTypedValues(fieldKey, propKey, valList,
			minVal, maxVal, bCheckMin, bCheckMax);

		return valList;
	}

	PointList GetPointValues(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const;

	///////////////////////////////////////////////////////////////////////
	// Read single values

	std::string GetStrValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const
	{
		return GetStrValues(fieldKey, propKey).front();
	}

	bool GetBoolValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const
	{
		return GetBoolValues(fieldKey, propKey).front() == 1;
	}

	int GetIntValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		int minVal = 0, int maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		return GetIntValues(fieldKey, propKey, 
			minVal, maxVal, bCheckMin, bCheckMax).front();
	}

	unsigned GetUIntValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		unsigned minVal = 0, unsigned maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		return GetIntValues(fieldKey, propKey, 
			minVal, maxVal, bCheckMin, bCheckMax).front();
	}

	float GetFloatValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword(), 
		float minVal = 0, float maxVal = 0,
		bool bCheckMin = false, bool bCheckMax = false) const
	{
		return GetFloatValues(fieldKey, propKey, 
			minVal, maxVal, bCheckMin, bCheckMax).front();
	}

	Point GetPointValue(const Keyword& fieldKey, 
		const Keyword& propKey = Keyword()) const
	{
		return GetPointValues(fieldKey, propKey).front();
	}

	///////////////////////////////////////////////////////////////////////
	// Get optional values
	/*!
		Gets a list of optional values. Returns true if the fieldKey-propKey
		exists and false otherwise.
	*/
	template <class T> bool GetOptionalTypedValues(const Keyword& fieldKey, 
		const Keyword& propKey, std::list<T>& valueList) const
	{
		try {
			GetTypedValues(fieldKey, propKey, valueList);
		}
		catch(BasicException e) 
		{
			return false;
		}

		return true;
	}

	/*!
		Gets an optional double value. Returns true if the fieldKey-propKey
		exists and false otherwise.
	*/
	bool GetOptionalDoubleValue(const Keyword& fieldKey, 
		const Keyword& propKey, double& value) const
	{
		std::list<double> valueList;

		bool hasData = GetOptionalTypedValues(fieldKey, propKey, valueList);

		if (hasData)
			value = valueList.front();

		return hasData;
	}

	///////////////////////////////////////////////////////////////////////
	// Serialization functions

	/*!
		Adds the field key to the set of fields to serialize if/when
		Serialize(os) is called.

		Only the fields in m_serializableFields (and all their properties)
		are serialized.
	*/
	void AddSerializableFieldKey(Keyword fieldKey)
	{
		m_serializableFields.insert(fieldKey);
	}

	/*!
		Serializes only the serializable fields in m_serializableFields.
	*/
	void Serialize(OutputStream& os) const
	{
		unsigned numFields = m_serializableFields.size();

		// Serialize num fields (as unsigned int)
		::Serialize(os, numFields);

		for (auto keyIt = m_serializableFields.begin(); 
			keyIt != m_serializableFields.end(); ++keyIt)
		{
			// Save whether or not the "requested" field has properties
			FieldMap::const_iterator fieldIt = m_fields.find(*keyIt);
			bool hasData = (fieldIt != m_fields.end());
		
			// if it does, save the properties
			::Serialize(os, hasData);

			// If it has data, save tha pair (field-name, properties)
			if (hasData)
				::Serialize(os, *fieldIt);

		}
	}
	
	/*!
		Deserializes all saved fields and overrrides their existing contents.

		The existing fields that are not deserialized ar unaffected by
		this operation.
	*/
	void Deserialize(InputStream& is)
	{
		unsigned numFields;
		std::pair<Keyword, PropertyMap> data;
		bool hasData;

		::Deserialize(is, numFields);

		for (; numFields > 0; numFields--)
		{
			::Deserialize(is, hasData);

			if (hasData)
			{
				::Deserialize(is, data);

				m_fields[data.first] = data.second;
			}
		}
	}

	///////////////////////////////////////////////////////////////////////
	// Static functions

	/*!
		Returns true if the string value corresponds to
		one of the enum labels.
	*/
	static bool ValidateEnum(const std::string& value, 
		const StrArray& enumLabels)
	{
		for (unsigned int i = 0; i < enumLabels.size(); i++)
			if (value == enumLabels[i])
				return true;

		return false;
	}

	/*!
		Gets the index of the value in the array 'enumLabels' 
		of valid string values.
	*/
	static int GetEnumIndex(const std::string& value, 
		const StrArray& enumLabels)
	{
		for (unsigned int i = 0; i < enumLabels.size(); i++)
			if (value == enumLabels[i])
				return i;

		THROW_BASIC_EXCEPTION("Invalid enum value");

		return -1;
	}

	/*!
		Gets the indices of the values in the array 'enumLabels' 
		of valid string values.
	*/
	static IntList GetEnumIndices(const StrList& values, 
		const StrArray& enumLabels)
	{
		StrList::const_iterator it;
		IntList idxList;

		for (it = values.begin(); it != values.end(); ++it)
			idxList.push_back(GetEnumIndex(*it, enumLabels));

		return idxList;
	}

	/*! 
		Gets the labels associated with the given enumeration index. It 
		also checks the array limits.
	*/
	static const std::string& GetEnumLabel(int index, 
		const StrArray& enumLabels)
	{
		return enumLabels[index];
	}

	/*!
		Gets the ordered list of labels associated with the given 
		enumeration indices.
	*/
	static StrList GetEnumLabels(const IntList& indices, 
		const StrArray& enumLabels)
	{
		StrList lbls;
		IntList::const_iterator it;

		for (it = indices.begin(); it != indices.end(); ++it)
			lbls.push_back(GetEnumLabel(*it, enumLabels));

		return lbls;
	}
};

} // namespace vpl

#endif //_PARAM_FILE_
