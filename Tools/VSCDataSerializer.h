/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "VisSysComponent.h"
#include "SimpleDatabase.h"

namespace vpl {

/*!
	Loads and saves any data produced by a vision system component (VSC).

	It takes care of associating each piece of data saved/loaded with 
	the current frame and video.
*/
class VSCDataSerializer
{
	typedef std::map<std::string, unsigned> LabeledPersistData;
	typedef std::map<std::string, LabeledPersistData> PersistDataMap;

	SimpleDatabase* m_pVideoDataDB; //!< Pointer to a database or NULL. DB is Not owned!

	PersistDataMap m_persistDataMap;
	unsigned m_dataMapStorageId;
	bool m_isDataMapModified;

public:
	VSCDataSerializer()
	{
		m_pVideoDataDB = NULL;
	}

	/*!
		Initializes the VSC data serializer.

		@param pDB Optional parameter that specifies a database to
			   store/retrieve the data of a VSC component. It *must*
			   be NULL if storage/retrieval operations are not desired.
	*/
	void Initialize(SimpleDatabase* pDB)
	{
		m_pVideoDataDB = pDB;
		m_dataMapStorageId = INVALID_STORAGE_ID;
		m_isDataMapModified = false;
	}

	template <typename T> 
	unsigned SaveComponentData(const T& x, const VisSysComponent* pComp, std::string label)
	{
		if (!m_pVideoDataDB)
			return INVALID_STORAGE_ID;

		unsigned dataDBId;

		// Find out the actual class name of the polymorphic type VisSysComponent
		LabeledPersistData& lpd = m_persistDataMap[vpl_TypeName(*pComp)];

		// See whether a piece of data with the same label is saved already
		LabeledPersistData::iterator it = lpd.find(label);

		if (it == lpd.end()) // there is no previous data with the same label
		{
			dataDBId = m_pVideoDataDB->Save(x);

			lpd[label] = dataDBId;
		}
		else // there was already a piece of data with this className-label
		{
			dataDBId = it->second;

			m_pVideoDataDB->Update(dataDBId, x);
		}

		// Turn on the modified flag
		m_isDataMapModified = true;

		return dataDBId;
	}

	template <typename T> 
	bool LoadComponentData(T& x, const VisSysComponent* pComp, std::string label) const
	{
		if (!m_pVideoDataDB || m_persistDataMap.empty())
			return false;

		// See whether the class name exists or not
		PersistDataMap::const_iterator it0 = m_persistDataMap.find(vpl_TypeName(*pComp));

		// If the clases isn't in the DB, the data can't be loaded
		if (it0 == m_persistDataMap.end())
			return false;
		
		// See whether the data label exists for any object of the target class
		LabeledPersistData::const_iterator it1 = it0->second.find(label);

		// If the class::label isn't in the DB, the data can't be loaded
		if (it1 == it0->second.end())
			return false;

		// The data exists, so load it
		return m_pVideoDataDB->Load(x, it1->second);
	}

	/*!
		
	*/
	void SetVideoDataDatabase(SimpleDatabase* pVideoDataDB)
	{
		m_pVideoDataDB = pVideoDataDB;
	}

	/*!
		Returns true if there is some FrameParseData that needs to be saved.
	*/
	bool HasNewFrameParseData() const
	{
		return m_isDataMapModified;
	}

	/*!
		
	*/
	void ClearFrameParseData()
	{
		m_persistDataMap.clear();
		m_dataMapStorageId = INVALID_STORAGE_ID;
		m_isDataMapModified = false;
	}

	/*!
		
	*/
	unsigned SaveFrameParseData() const
	{
		ASSERT(m_pVideoDataDB);
		ASSERT(m_isDataMapModified);

		unsigned id = m_dataMapStorageId;

		if (id == INVALID_STORAGE_ID)
			id = m_pVideoDataDB->Save(m_persistDataMap);
		else
			m_pVideoDataDB->Update(id, m_persistDataMap);

		return id;
	}

	/*!
		
	*/
	void LoadFrameParseData(unsigned id)
	{
		ASSERT(m_pVideoDataDB);

		m_pVideoDataDB->Load(m_persistDataMap, id);

		m_dataMapStorageId = id;
		m_isDataMapModified = false;
	}
};

} // namespace vpl
