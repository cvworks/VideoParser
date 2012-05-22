/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "UserArguments.h"

#include <iomanip>

#define SPACING "  "
#define FIELD_WIDTH 20
#define FIELD_SPACING 3 // at least 2 to account for ' {' characters
#define VALUE_WIDTH 8
#define USAGE_WIDTH 40

#define SetMaxSize(V,S) if (V < S.size()) { V = S.size(); }

unsigned int g_maxFPWidth; //!< Maximum column width for printing fields/properties


/*! 
	Prints the usage message of each field and property.
*/
void vpl::UserArguments::ShowArgumentUsage(std::ostream& os) const
{
	UsageInfoList uil;
	UsageInfoList::const_iterator it;

	GetUsageList(uil);

	ShowUsage("\n");

	for (it = uil.begin(); it != uil.end(); ++it)
	{
		os << it->first << "\t" << it->second->m_usageMsg << "\n";
	}

	os << std::endl;
}

/*! 
	Gets a list of field, properties and their usage info.
*/
void vpl::UserArguments::GetUsageList(UsageInfoList& uil) const
{
	FieldUsageMap::const_iterator fieldIt;
	PropertyUsageMap::const_iterator propIt;
	std::string fname, pname;

	g_maxFPWidth = 0;

	for (fieldIt = m_usageInfo.begin(); 
		fieldIt != m_usageInfo.end(); ++fieldIt)
	{
		fname = fieldIt->first;

		propIt = fieldIt->second.begin(); // get first property

		// If there is only a single unnamed property,
		// just add the field name
		if (fieldIt->second.size() == 1 && 
			propIt->first == UNNAMED_PROPERTY)
		{
			uil.push_back(std::make_pair(fname, &propIt->second));
		}
		else // iterate over all properties
		{
			uil.push_back(std::make_pair(fname + " {", (const UsageInfo*)NULL));

			for (; propIt != fieldIt->second.end(); ++propIt)
			{
				pname = "-" + propIt->first;

				uil.push_back(std::make_pair(pname, &propIt->second));

				SetMaxSize(g_maxFPWidth, pname);
			}

			uil.push_back(std::make_pair("}", (const UsageInfo*)NULL));
		}

		SetMaxSize(g_maxFPWidth, fname);
	}

	g_maxFPWidth += FIELD_SPACING;
}

/*! 
	Gets the default values of all fields and properties 
	as a single string.
*/
std::string vpl::UserArguments::GetAllDefaultValues() const
{
	UsageInfoList uil;
	UsageInfoList::const_iterator it;
	std::ostringstream oss;

	GetUsageList(uil);

	oss.flags(std::ios::left);

	// Print column headers
	oss << std::setw(g_maxFPWidth) << "Field/Property"
		<< std::setw(USAGE_WIDTH) << "Usage Message"
		<< "Default Values" << "\n\n";

	unsigned int first;
	const int w = USAGE_WIDTH - 2;

	for (it = uil.begin(); it != uil.end(); ++it)
	{
		oss << std::setw(g_maxFPWidth) << it->first;

		if (it->second != NULL)
		{
			// Print multiple message lines
			const std::string& str = it->second->m_usageMsg;

			first = 0;
		
			do {
				if (first > 0)
					oss << "\n" << std::setw(g_maxFPWidth) << " ";
				
				oss << std::setw(USAGE_WIDTH) << str.substr(first, w);

				if (first == 0)
					oss << it->second->m_defVals;

				first += w;
			} 
			while (first < str.size());
		}

		oss << "\n";
	}

	return oss.str();
}

/*! 
	Gets the possible values of all fields and properties 
	as a single string.
*/
std::string vpl::UserArguments::GetAllValueOptions() const
{
	UsageInfoList uil;
	UsageInfoList::const_iterator it;
	std::ostringstream oss;

	GetUsageList(uil);

	oss.flags(std::ios::left);

	// Print column headers
	oss << std::setw(g_maxFPWidth) << "Field/Property"
			<< std::setw(VALUE_WIDTH) << "Min"
			<< std::setw(VALUE_WIDTH) << "Max"
			<< "Enumeration Values" << "\n\n";

	for (it = uil.begin(); it != uil.end(); ++it)
	{
		oss << std::setw(g_maxFPWidth) << it->first;

		if (it->second != NULL)
			oss << std::setw(VALUE_WIDTH) << it->second->m_minVal
				<< std::setw(VALUE_WIDTH) << it->second->m_maxVal
				<< ConvertValuesToString(it->second->m_enumVals);

		oss << "\n";
	}

	return oss.str();
}

/*!
	Read the "mandatory" field "version = N". If it exists, N is returned. Otherwise,
	an error is shown and -1 is returned.

	@param maximum version number that should be considered valid (zero if no check 
	is desired)
*/
int vpl::UserArguments::ReadVersionNumber(int maxVersion) const
{
	// Next, make sure that the 'version' field is provided
	int version = -1;

	try {
		version = GetIntValue("version");

		if (version < 0 || (maxVersion > 0 && version > maxVersion))
		{
			ShowError1("Unknown version number in", m_inputFilename);

			return -1;
		}
	}
	catch(BasicException e)
	{
		ShowError1("Missing mandatory parameter: 'version = N' in", m_inputFilename);

		ShowMsg("Additional optional parameters:\n");

		std::cerr << GetAllDefaultValues() << std::endl;

		return -1;
	}

	return version;
}