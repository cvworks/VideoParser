/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/PersistentHierarchicalMetadata.h>
#include <Tools/Num2StrConverter.h>
#include <sstream>

namespace vpl {

/*!
	Represents a N-to-1 relation between "object class"
	information in a hierarchy of object classes, subclasses,
	subsubclasses, etc.

	In order to make the file read/write operations more efficient, 
	the name of the class is fixed to ObjectClassMetadata::MAX::NAME_SIZE.

	The lowest level class usually represents the actual object name.
*/
struct ObjectClassMetadata : public PersistentHierarchicalMetadata
{
	enum {ID_NOT_SET = -1, NAME_SIZE = 50};

	char name[51]; //!< Set size as NAME_SIZE + 1 (for null char)
	int id;

	//! Sets str as the object class name and 0 as its id
	void Set(const std::string& str, int objId = 0)
	{
		string_copy(str, name, NAME_SIZE + 1, NAME_SIZE);

		id = objId;
	}

	void SetEmpty()
	{
		id = ID_NOT_SET;
		name[0] = '\0';
	}
};

/*!
	Represents a N-to-1 relation between shape views of an object
	and the lowest level class information of the object (ie, the
	object name).
*/
struct ShapeViewMetadata : public PersistentHierarchicalMetadata
{
	enum {ID_NOT_SET = -1, ID_NAME_SIZE = 10, NUM_PROPERTIES = 3};

	//! Id associated with the view property
	int propId[3]; 

	//! Name of each property sized as ID_NAME_SIZE + 1 (for null char)
	char propName[3][11]; 

	/*!
		Set the properties in the order defined by the list.
	*/
	void Set(const StrIntList& props)
	{
		WARNING(props.size() > NUM_PROPERTIES, 
			"There are too many object view properties. Some will be ignored.");

		// Init all property values
		SetEmpty();

		auto it = props.begin();

		for (int i = 0; it != props.end() && i < NUM_PROPERTIES; ++it, ++i)
		{
			string_copy(it->first, propName[i], ID_NAME_SIZE + 1, ID_NAME_SIZE);
			propId[i] = it->second;
		}
	}

	StrIntList Get() const
	{
		StrIntList props;

		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
		{
			props.push_back(std::make_pair(
				std::string(propName[i]), propId[i]));
		}

		return props;
	}

	StrIntPair Get(int i) const
	{
		ASSERT(i < NUM_PROPERTIES);

		return std::make_pair(std::string(propName[i]), propId[i]);
	}

	void SetEmpty()
	{
		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
		{
			propId[i] = ID_NOT_SET;
			propName[i][0] = '\0';
		}
	}

	/*!
		Returns true if there is a one-to-one mapping
		between the property of the view and the
		properties in the list.
	*/
	bool Compare(StrIntList props) const
	{
		StrIntList::iterator it;

		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
		{
			for (it = props.begin(); it != props.end(); ++it)
			{
				if (it->first == propName[i] && it->second == propId[i])
					break;
			}

			if (it != props.end())
				props.erase(it);
			else
				return false;
		}

		return props.empty();
	}

	std::string ToString() const
	{
		std::ostringstream oss;
		
		for (unsigned i = 0; i < NUM_PROPERTIES; i++)
		{
			if (i > 0) 
				oss << ",";

			oss << propName[i] << "," << propId[i];
		}

		return oss.str();
	}
};

/*!
	Represents a N-to-1 relation between parses of a shape
	view and their source shape view.
*/
struct ShapeParseMetadata : public PersistentHierarchicalMetadata
{
	unsigned parseId;

	void Set(int id)
	{
		parseId = id;
	}
};

} // namespace vpl

// Declare the serialization functions for all types of medatada
DECLARE_BASIC_SERIALIZATION(vpl::ObjectClassMetadata)
DECLARE_BASIC_SERIALIZATION(vpl::ShapeViewMetadata)
DECLARE_BASIC_SERIALIZATION(vpl::ShapeParseMetadata)
