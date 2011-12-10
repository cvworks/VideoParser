/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include "BlobTrackerDBManager.h"

namespace vpl {

// Declare prototypes of expected parent components
class ImageProcessor;
class BlobFinder;

/*!
*/
class BlobTracker : public VisSysComponent
{
protected:

	//Parent components
	std::shared_ptr<const ImageProcessor> m_pImgProcessor;
	//std::shared_ptr<const BackgroundSubtractor> m_pBackSub;
	std::shared_ptr<const BlobFinder> m_pBlobFinder;

	BlobTrackerDBManager m_dbm;

protected:
	


public: // new abstract functions
	virtual ByteImg GetBlobMask() const = 0;

public: // functions from parent class

	/*!
		This function is called by void VSCGraph::Reset()
		before a new video is processed. Therefore, any 
		current database session should be terminated.
	*/
	virtual void Clear()
	{
		if (m_dbm)
			m_dbm.resetSession();
	}

	/*!
		This is called right after a whole video is processed,
		so we can use it to save the end_time of a session.
	*/
	virtual void PostProcessSequence()
	{
		if (m_dbm && m_dbm.hasSession())
			m_dbm.writeSessionEndtime();
	}

	virtual void Initialize(graph::node v);

	virtual void ReadParamsFromUserArguments();

	virtual StrArray Dependencies() const
	{
		StrArray deps;
		
		deps.push_back("ImageProcessor");
		deps.push_back("BlobFinder");

		return deps;
	}

	virtual std::string GenericName() const
	{
		return "BlobTracker";
	}
};

} // namespace vpl

