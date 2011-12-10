/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ParamFile.h"
#include <sstream>
#include <vul/vul_awk.h>

using namespace vpl;

const ParamFile::Keyword ParamFile::UNNAMED_PROPERTY("_unnamed"); 

/*!
	Reads fields with properties and values from a text file. 

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

	Returns false if the file can't be read, but shows not error message
	since it is not known whether the file should exist or not. Caller 
	must call ShowOpenFileError(szFilename) if return value is false
	and	that is an actual error.
*/
bool ParamFile::ReadParameters(const char* szFilename)
{
	// Make sure that there are no previous params
	Clear();

	std::ifstream fs(szFilename);

	// Don't shown an error if can't open file
	// because it might be okay. Just return false.
	if (fs.fail())
		return false;

	// Save the given file name
	m_inputFilename = szFilename;

	Keyword fieldName;
	Keyword propertyName;

	bool bReadingNamedValues = false;

	// @todo There is a bug in VXL awk, and we need to set vul_awk::strip_comments
	// to make things work, even if we don't want to

	for (vul_awk awk(fs, vul_awk::strip_comments); awk; ++awk)
	{
		// Ignore empty lines and comment lines that begin with '#'
		if (awk.NF() == 0 || awk[0][0] == '#')
			continue;

		// Check for the begin-named-value command
		if (!bReadingNamedValues && awk.NF() == 2 && awk[1][0] == '{' && awk[1][1] == '\0')
		{
			bReadingNamedValues = true;
			
			fieldName = awk[0];
		}
		// Check for the end-named-value command
		else if (bReadingNamedValues && awk.NF() == 1 && awk[0][0] == '}' && awk[0][1] == '\0')
		{
			bReadingNamedValues = false;
		}
		// Check for assignment command "var = val val ...'
		// which may be stand alone or within a property environment
		else if (awk.NF() >= 3 && awk[1][0] == '=' && awk[1][1] == '\0')
		{
			if (bReadingNamedValues)
			{
				propertyName = awk[0];
			}
			else
			{
				fieldName = awk[0];
				propertyName = UNNAMED_PROPERTY;
			}

			// Insert a new field and a new property
			StrList& values = m_fields[fieldName][propertyName];
			
			values.clear();

			// Read all the values after the equal sign. Also, check for quotes
			// in order to allow for values with spaces in them.
			std::string val_line(awk.line_from(2));

			if (val_line.front() == '\"' && val_line.back() == '\"')
			{
				values.push_back(val_line.substr(1, val_line.size() - 2));
			}
			else
			{
				for (int i = 2; i < awk.NF(); i++)
				{
					// Allow for backslash continuations of lines
					if (awk[i][0] == '\\' && awk[i][1] == '\0')
					{
						++awk;
						i = -1;
					}
					else
						values.push_back(awk[i]);
				}
			}
		}
		else
		{
			StreamError("Syntax error in file " << szFilename 
				<< " line " << awk.NR() << ": '" << awk.line() 
				<< "'\nValid lines must have one of the following forms:"
				"\n  'keyword = value1 value2 ...'"
				"\n  'keyword {\n  keyword = value1 value2 ..."
				"\n  keyword = value1 value2 ...\n...  \n  }\n'"
				"\nIn addition, there must be an end-of-line at the end of the document.");

			return false;
		}
	}

	return true;
}

/*!
	Returns all fields, properties and values as a single
	string.
*/
std::string ParamFile::GetAllFieldsAndValues() const
{
	FieldMap::const_iterator fieldIt;
	PropertyMap::const_iterator propIt;
	StrList::const_iterator valueIt;
	std::ostringstream oss;
	
	for (fieldIt = m_fields.begin(); fieldIt != m_fields.end(); ++fieldIt)
	{
		// Print the field name
		oss << "\n" << fieldIt->first;

		// Get the first property in the property map
		propIt = fieldIt->second.begin();

		// If the field only has an unnamed property, print it using a single line
		if (fieldIt->second.size() == 1 && propIt->first == UNNAMED_PROPERTY)
		{
			oss << " = " << ConvertValuesToString(propIt->second) << "\n";
		}
		else // the field has named properties
		{
			oss << " {";

			for (; propIt != fieldIt->second.end(); ++propIt)
			{
				oss << "\n  " << propIt->first << " = " 
					<< ConvertValuesToString(propIt->second);
			}

			oss << "\n}\n";
		}
	}

	return oss.str();
}

/*!
	An empty property key is equivalent to passing a property key that
	is equal to the fieldKey.
*/
const StrList& ParamFile::GetStrValues(const Keyword& fieldKey, 
												  const Keyword& propKey) const
{
	FieldMap::const_iterator fieldIt = m_fields.find(fieldKey);

	if (fieldIt == m_fields.end())
		THROW_BASIC_EXCEPTION("Field '" + fieldKey + "' does not exist");

	PropertyMap::const_iterator propIt;
	
	propIt = (propKey.empty()) ? 
		fieldIt->second.find(UNNAMED_PROPERTY) : fieldIt->second.find(propKey);

	if (propIt == fieldIt->second.end())
		THROW_BASIC_EXCEPTION("Property '" + propKey + 
			" of field '" + fieldKey + "' does not exist");

	return propIt->second;
}

PointList ParamFile::GetPointValues(const Keyword& fieldKey,
									const Keyword& propKey) const
{
	const StrList& values = GetStrValues(fieldKey, propKey);
	StrList::const_iterator it;
	PointList ptList;
	Point pt;
	char a, b, c;

	for (it = values.begin(); it != values.end(); it++)
	{
		std::istringstream iss(*it);
		
		iss >> a >> pt.x >> b >> pt.y >> c;
		
		if (a != '(' || b != ',' || c != ')')
			THROW_BASIC_EXCEPTION("Invalid syntax in Point type value");

		ptList.push_back(pt);
	}

	return ptList;
}

