/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/PersistentHierarchicalMetadata.h>
#include <Tools/DirWalker.h>

namespace vpl {

/*!
	Information used to identify a video. 
	
	This comparison operators can be used to check if the
	information about two video files correspond to
	the same video. 

	Right now, only the filename (without the path) is used
	to determine if the vido info comapred is the same.
*/
struct VideoParseMetadata : public PersistentHierarchicalMetadata
{
	std::string filename; 

	void Set(const std::string& str)
	{
		filename = str;
	}

	bool operator==(const std::string& fn) const
	{
		//return (filename == fn);

		return (DirWalker::GetName(filename.c_str()) 
			== DirWalker::GetName(fn.c_str()));
	}

	bool operator!=(const std::string& fn) const
	{
		return !operator==(fn);
	}

	void Print(std::ostream& os) const
	{
		os << "Video file name: " << filename << "\n";
	}

	void Serialize(OutputStream& os) const
	{
		PersistentHierarchicalMetadata::Serialize(os);

		::Serialize(os, filename);
	}

	void Deserialize(InputStream& is)
	{
		PersistentHierarchicalMetadata::Deserialize(is);

		::Deserialize(is, filename);
	}
};

/*!
	
*/
struct FrameParseMetadata : public PersistentHierarchicalMetadata
{
	fnum_t frameNumber;

	void Set(fnum_t frame_num)
	{
		frameNumber = frame_num;	
	}

	void Serialize(OutputStream& os) const
	{
		PersistentHierarchicalMetadata::Serialize(os);

		::Serialize(os, frameNumber);
	}

	void Deserialize(InputStream& is)
	{
		PersistentHierarchicalMetadata::Deserialize(is);

		::Deserialize(is, frameNumber);
	}
};

} // namespace vpl

