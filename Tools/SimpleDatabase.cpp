/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "SimpleDatabase.h"
#include "Exceptions.h"

using namespace vpl;
using namespace std;

//! Declare the Deserialization/Serialization functions for FileTail
//DECLARE_BASIC_SERIALIZATION(vpl::SimpleDatabase::FileTail)

bool SimpleDatabase::ReadDataIndexMap()
{
	ASSERT(m_stream.is_open());
	ASSERT(!IsModified());

	// Go to the end of the file
	MoveGetPtrToEnd();

	// Store the position of the end
	std::streamoff sz = m_stream.tellg();

	if (sz > 0)
	{
		// Read the info located at the end of the file
		MoveGetPtrTo(sz - sizeof(FileTail));
		
		m_pTail->Deserialize(m_stream);

		if (m_stream.gcount() != sizeof(FileTail))
		{
			ShowError("Can't read basic tail information from database");
			return false;
		}

		if (!m_pTail->IsValid())
		{
			ShowError("The database tail information is invalid");
			return false;
		}

		MoveGetPtrTo(m_pTail->dataMapOffset);

		m_tables.Deserialize(m_stream);
	}
	else // ensure that the data map is clear (it should, but just in case)
	{
		m_tables.Clear();
	}

	return true;
}

void SimpleDatabase::WriteDataIndexMap()
{
	ASSERT(IsModified());

	m_stream.seekp(m_pTail->dataMapOffset);

	m_pTail->MakeValid();

	// Serialize the tables first!
	m_tables.Serialize(m_stream);

	m_pTail->Serialize(m_stream);

	m_stream.flush();

	SetModified(false);
}

bool SimpleDatabase::RebuildDataIndexMap()
{
	return false;
	/*String strClassName;
	streampos pos;
	DAGPtr ptrDag;
	DAGINFO dagInfo;

	MoveGetPtrToEnd();
	DBInfo().m_nTailOffset = tellg();
	DBInfo().m_nMaxTSVDimension = 0;

	MoveGetPtrToBeg();

	while(!fail())
	{
		pos = tellg();
		strClassName.Read(*this);

		ptrDag = ReadDAG(pos, strClassName);

		if (ptrDag.IsNull())
			break;

		dagInfo.nOffset = pos;
		dagInfo.nType = GetClassID(strClassName);

		m_dagInfo.AddTail(dagInfo);

		if (ptrDag->GetMaxTSVDimension() > DBInfo().m_nMaxTSVDimension)
			DBInfo().m_nMaxTSVDimension = ptrDag->GetMaxTSVDimension();
	}

	return SetModified(m_dagInfo.GetSize() > 0);*/
}
