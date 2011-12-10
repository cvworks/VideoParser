/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <VideoRepresentation/GenericVideo.h>
#include <Tools/SimpleDatabase.h>
#include <Tools/VisSysComponent.h>
#include <Tools/VSCDataSerializer.h>
#include "VideoParseMetadata.h"

namespace vpl {

/*!
	Loads and saves data associated with video files.
	
	It works together with the VSCDataSerializer, which 
	Loads and saves any data produced by a vision system component (VSC).

	It takes care of associating each piece of data saved/loaded with 
	the current frame and video.

	The data is saved using a SimpleDatabase object.
*/
class VideoDataBinder
{
	std::string m_videoFilename;
	bool m_validInit;
	bool m_isOpen;

	SimpleDatabase m_videoDataDB;
	VSCDataSerializer m_videoDataSerializer;

	unsigned m_videoMetadataId;
	unsigned m_prevFPMId;
	unsigned m_currFPMId;

	FrameParseMetadata m_prevFPM;
	FrameParseMetadata m_currFPM;

	bool m_currFrameHasMetadata;

public:

	VideoDataBinder()
	{
		m_validInit = false;
		m_isOpen = false;
	}

	void Close()
	{
		m_isOpen = false;
	}

	bool IsOpen() const
	{
		return m_isOpen;
	}

	/*!
		Initializes the data file.
	*/
	void Initialize(std::string dataFilename)
	{
		m_isOpen = false;

		m_videoDataDB.Close();

		if (m_videoDataDB.Create(dataFilename.c_str()))
		{
			m_videoDataSerializer.Initialize(&m_videoDataDB);
			m_validInit = true;
		}
		else
		{
			m_videoDataSerializer.Initialize(NULL);
			m_validInit = false;

			ShowOpenFileError(dataFilename);
		}
	}

	VSCDataSerializer* GetDataSerializer()
	{
		return (m_validInit) ? &m_videoDataSerializer : NULL;
	}

	/*!
		Starts or opens a new binding of video and data.
	*/
	void OpenBinding(std::string videoFilename, fnum_t frameNumber)
	{
		ASSERT(!m_isOpen);

		VideoParseMetadata vpm;

		m_videoFilename = videoFilename;
		m_currFrameHasMetadata = false;

		//DBG_MSG2("Looking for video file: ", videoFilename)
		//DBG_ONLY(m_videoDataDB.List(vpm, std::cout))

		// See if there is data associated with the current video file
		m_videoMetadataId = m_videoDataDB.Find(vpm, videoFilename);

		// Init current and previous frame metadata id's
		m_prevFPMId = INVALID_STORAGE_ID;
		m_currFPMId = INVALID_STORAGE_ID;

		// Due to the hierarchical (linked list) organization of the cash data,
		// we need to find the medatada of the frame previous to the first
		// video frame to retrieve it (it may not have been saved).
		if (m_videoMetadataId != INVALID_STORAGE_ID && 
			vpm.storParentMetadataId != INVALID_STORAGE_ID)
		{
			m_currFPMId = vpm.storParentMetadataId;

			bool success = m_videoDataDB.Load(m_currFPM, m_currFPMId);
			ASSERT(success);

			MoveClosestToFrameData(frameNumber);
		}

		// We must set "m_isOpen" true even when there was a file error. This
		// variable is user to ensure that OpenBinding() has been called.
		m_isOpen = true;
	}

	/*!
		Reads metadata objects such that the medatada of the requested frame 
		is eithere the current one or it should be inserted right befor 
		the current one if it is created.
	*/
	void MoveClosestToFrameData(fnum_t frameNumber)
	{
		// The previous metadata should be associated with
		// a frame that comes before 'frameNumber'
		if (m_prevFPMId != INVALID_STORAGE_ID && 
			m_prevFPM.frameNumber > frameNumber)
		{
			// The needed frame metadata might be before
			// the previous metadata loaded. In this case,
			// we have to reload the binding
			Close();
			OpenBinding(m_videoFilename, frameNumber);
		}
		else if (m_currFPMId != INVALID_STORAGE_ID &&
			m_currFPM.frameNumber < frameNumber)
		{
			m_prevFPMId = m_currFPMId;
			m_prevFPM = m_currFPM;
			m_currFPMId = m_currFPM.storParentMetadataId;

			while (m_currFPMId != INVALID_STORAGE_ID)
			{
				bool success = m_videoDataDB.Load(m_currFPM, m_currFPMId);
				ASSERT(success);

				// We want the metadata of the PREVIOUS frame savedm
				// so as soon as we reach or pass the target frame, we are done
				if (m_currFPM.frameNumber >= frameNumber)
					break;

				// Save the info in case it ends up being the previous
				// frame metadata that we are looking for
				m_prevFPMId = m_currFPMId;
				m_prevFPM = m_currFPM;
				m_currFPMId = m_currFPM.storParentMetadataId;
			}
		}
	}

	/*!
		We should currently have the metadata of the data associated
		with the nearest previous frame to frameNumber that
		has data associated with it.
	*/
	void LoadFrameData(fnum_t frameNumber)
	{
		ASSERT(m_isOpen);

		MoveClosestToFrameData(frameNumber);

		if (m_currFPMId != INVALID_STORAGE_ID &&
			m_currFPM.frameNumber == frameNumber)
		{
			m_currFrameHasMetadata = true;

			m_videoDataSerializer.LoadFrameParseData(
				m_currFPM.storDataId);
		}
		else
		{
			m_currFrameHasMetadata = false;
			m_videoDataSerializer.ClearFrameParseData();
		}
	}

	void SaveFrameData(fnum_t frameNumber)
	{
		unsigned compDataId;

		if (m_videoDataSerializer.HasNewFrameParseData())
		{
			// If there is data to save, then the file should be open
			ASSERT(m_isOpen);

			ShowStatus("Saving frame data indices (step 1/2)...");

			// Save (or update) the current frame parse data
			// Ie, the mapping of data labels and database IDs
			compDataId = m_videoDataSerializer.SaveFrameParseData();

			// Either there is yet no metadata created or the ID of the
			// data stored in the existing metadata is the one returned
			// by SaveFrameParseData(). 
			ASSERT(!m_currFrameHasMetadata || 
				m_currFPM.storDataId == compDataId);

			// Create a new metadata for the current *frame* if necessary
			if (!m_currFrameHasMetadata)
			{
				ShowStatus("Creating frame metadata...");

				m_currFPM.SetStorageInfo(compDataId);
				m_currFPM.Set(frameNumber);
				m_currFPMId = m_videoDataDB.Save(m_currFPM);

				m_currFrameHasMetadata = true;
			}

			ASSERT(frameNumber == m_currFPM.frameNumber);

			// If there is no *video* metadata yet, we save one
			// and link it to the current *frame* metadata
			if (m_videoMetadataId == INVALID_STORAGE_ID)
			{
				ShowStatus("Creating video metadata...");

				// The VPM acts as the root of the metadata graph
				// but is stored as the only leaf of such a graph
				VideoParseMetadata vpm;
			
				vpm.Set(m_videoFilename);
	
				// Make the current FPM the parent of the VPM
				vpm.SetStorageInfo(INVALID_STORAGE_ID, m_currFPMId);

				// Save the VPM
				m_videoMetadataId = m_videoDataDB.Save(vpm);
			}

			// Update the link between the previous metadata 
			// and the current one if necesary
			if (m_prevFPMId != INVALID_STORAGE_ID && 
				m_prevFPM.storParentMetadataId != m_currFPMId)
			{
				ShowStatus("Relinking frame metadata...");

				// Make the current FPM the parent of the previous FPM
				m_prevFPM.storParentMetadataId = m_currFPMId;

				// Update the previous FPM to reflect the change
				m_videoDataDB.Update(m_prevFPMId, m_prevFPM, true);
			}

			// Make sure that all the frame data is saved
			m_videoDataDB.FlushEverything();

			ShowStatus("Frame data indices are saved (step 2/2)");
		}
	}
};

} // namespace vpl
