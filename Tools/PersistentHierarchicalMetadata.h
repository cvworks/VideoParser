/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/SimpleDatabase.h>

namespace vpl {

/*!
	Metadata representing the relations pieces of
	object information such as:	their class hierarchy, their 
	views and shape parses on disk. 
	
	A PersistentHierarchicalMetadata (PHM) can be seen as a node 
	in a directed (hierarchical) graph stored in a database file.

	If a PHM doesn't have any data stored in a database,
	then its storage Id must be set to INVALID_STORAGE_ID.

	If a PHM doesn't have a parent, then the storage id of
	its parent must be set to INVALID_STORAGE_ID.
*/
struct PersistentHierarchicalMetadata
{
	unsigned storDataId;           //!< Storage ID of the data
	unsigned storParentMetadataId; //!< Storage ID of the parent's metadata

	//! Sets the storage ID's of the data and the metadata of its parent
	PersistentHierarchicalMetadata(unsigned dataId = INVALID_STORAGE_ID, 
		unsigned parentMetadataId = INVALID_STORAGE_ID)
	{
		SetStorageInfo(dataId, parentMetadataId);
	}

	//! Sets the storage ID's of the data and the metadata of its parent
	void SetStorageInfo(unsigned dataId, 
		unsigned parentMetadataId = INVALID_STORAGE_ID)
	{
		storDataId = dataId;
		storParentMetadataId = parentMetadataId;
	}

	DECLARE_BASIC_MEMBER_SERIALIZATION
};

} // namespace vpl
