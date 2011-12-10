/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <fstream>
#include "Serialization.h"

#define INVALID_STORAGE_ID UINT_MAX

namespace vpl {

	/*!
		A database with simple structure and load/save/update functions.

		The structure of the database is as follows:

			{[DataBlockHeader][DATA]}*
			IndexTables
			TailInfo

		@todo add a delete option, "defrag", reuse available block, 
			  add a load function with an ad-hoc "T::deserialize()" parameter.
	*/
	class SimpleDatabase
	{		
	public:
		typedef std::ios_base::openmode OpenMode;

		//! Available file types. 
		// Make sure that UNKNOWN == 0
		enum FileType {UNKNOWN, BINARY, TEXT, XML};

		struct FileTail
		{
			const FileType type;
			const int version;
			std::streampos dataMapOffset;
			int checkSum;

			DECLARE_BASIC_MEMBER_SERIALIZATION

			FileTail(FileType t, int ver) : type(t), version(ver)
			{
				Clear();
			}

			void Clear()
			{
				dataMapOffset = 0;
				checkSum = -1;
			}

			bool IsValid() 
			{ 
				return checkSum == (int(dataMapOffset) + int(type) + version);
			}

			void MakeValid()
			{
				checkSum = int(dataMapOffset) + int(type) + version;
			}
		};

	protected:
		typedef std::vector<std::streampos> DataIndex;
		typedef std::map<std::string, DataIndex> DataIndexMap;
		typedef std::pair<std::streampos, std::streamoff> BlockIndex;
		typedef std::list<BlockIndex> BlockIndexList;

		struct DataBlockHeader
		{
			std::streamoff blockSize;

			DataBlockHeader() { }
			DataBlockHeader(unsigned sz) { blockSize = sz; }

			DECLARE_BASIC_MEMBER_SERIALIZATION
		};

		struct IndexTables
		{
			DataIndex flatDataIndex;
			DataIndexMap namedDataIndex;
			BlockIndexList availableBlocks;

			/*std::streampos operator[](unsigned i)
			{
				return flatDataIndex[i];
			}*/

			/*!
				Retrieves the data index associated with the name 'name'.
				If there is no such index, it inserts and empty one
				and returns it.
			*/
			DataIndex& operator[](const std::string& name)
			{
				return namedDataIndex[name];
			}

			/*!
				Retrieves a pointer to the data index associated with the 
				name 'name' if there is one, and NULL otherwise.
			*/
			const DataIndex* Find(const std::string& name) const
			{
				DataIndexMap::const_iterator it;

				it = namedDataIndex.find(name);

				if (it == namedDataIndex.end())
					return NULL;

				return &it->second;
			}

			void Clear()
			{
				flatDataIndex.clear();
				namedDataIndex.clear();
				availableBlocks.clear();
			}

			void Serialize(OutputStream& os) const
			{
				::Serialize(os, flatDataIndex);
				::Serialize(os, namedDataIndex);
				::Serialize(os, availableBlocks);
			}

			void Deserialize(InputStream& is)
			{
				::Deserialize(is, flatDataIndex);
				::Deserialize(is, namedDataIndex);
				::Deserialize(is, availableBlocks);
			}
		};

		std::fstream m_stream; //!< The underlying file stream storing the data
		IndexTables m_tables;  //!< Tables of indices to data and blocks
		FileTail* m_pTail;     //!< Heap allocated tail data in the file stream
		bool* m_pIsModified;   //!< Heap allocated 'modified' flag

		std::string m_filename;

	public:
		//! Stores the id and index information of a loaded object
		struct IndexInfo
		{
			const DataIndex* pDataIndex;
			std::string className;
			unsigned objId;

			IndexInfo()
			{
				objId = INVALID_STORAGE_ID;
			}

			void operator=(const IndexInfo& rhs)
			{
				pDataIndex = rhs.pDataIndex;
				className = rhs.className;
				objId = rhs.objId;
			}

			operator bool() const
			{
				return (objId != INVALID_STORAGE_ID);
			}
		};

	protected:
		bool OpenStreamAndIndexMap(const char* szName, OpenMode mode)
		{
			ASSERT(!IsModified());

			// Ensure that we have reading permits because we
			// need to read the tail info
			mode |= std::ios_base::in;

			// Ensure that append is NOT specified because we need 
			// to overwrite the tail info!
			if (mode & std::ios_base::app)
			{
				mode &= ~std::ios_base::app;
				mode |= std::ios_base::out;
			} 

			m_stream.open(szName, mode);

			// If there is an open stream error, simply return false
			if (m_stream.fail())
				return false;

			// Try reading the data index map. It might fail if
			// the file is not empty and the index is corrupted.
			// This is a seriour error, so we report it to the user.
			if (!ReadDataIndexMap())
			{
				StreamError("The data index map in the database " 
					<< szName << " is corrupted.");

				return false;
			}
			
			m_filename = szName;

			return true;
		}

		void CloseStream()
		{
			m_stream.close();

			m_stream.clear(); // ensure that the fail bit is cleared

			m_filename.clear();
		}

		//! Checks that the sterams is closed and clears all error flags
		void ClearStream()
		{
			ASSERT(!m_stream.is_open());

			m_stream.clear();
		}

		void Clear()
		{
			ClearStream();
			
			m_tables.Clear();

			SetModified(false);
		}

		bool ReadDataIndexMap();
		void WriteDataIndexMap();
		bool RebuildDataIndexMap();

		void MoveGetPtrTo(std::streamoff off) 
		{ 
			m_stream.clear(); 
			m_stream.seekg(off, std::ios::beg);	
		}

		void MoveGetPtrToEnd() 
		{ 
			m_stream.clear(); 
			m_stream.seekg(0, std::ios::end);
		}

		void MoveGetPtrToBeg() 
		{ 
			MoveGetPtrTo(0); 
		}

		bool SetModified(bool val) 
		{ 
			return *m_pIsModified = val; 
		}

		//! Returns true iff the file exists and has non-zero size
		bool CheckFileExist(const char* szName)
		{
			// Try opening the stream and the DataIndexMap
			OpenStreamAndIndexMap(szName, std::ios_base::in | 
				std::ios_base::binary | std::ios_base::ate);

			// Note: we ignore the return value of the above function because
			// it could be that the stream was indeed opened
			// but the data index is corrupted and the function returned false.
			bool bFileExist = (m_stream && m_stream.tellg() > 0);

			CloseStream();
			
			return bFileExist;
		}

		void ReadDataBlockHeader(DataBlockHeader* dbh, std::streampos dataPos)
		{
			std::streamoff headerOffset = sizeof(*dbh);

			ASSERT(dataPos >= headerOffset);

			MoveGetPtrTo(dataPos - headerOffset);

			dbh->Deserialize(m_stream);
		}

		/*!
			Saves block header and data in the database and returns the
			position of the data in the file stream.
		*/
		template <typename T> std::streampos Write(const T& x)
		{
			// Get the end-of-file offset from the Tail structure
			const std::streampos headerPos = m_pTail->dataMapOffset;

			// Save the header. By now, simply "guess" a data block size
			DataBlockHeader header(sizeof(x));

			m_stream.seekp(headerPos); // move the "put" pointer
			header.Serialize(m_stream); // save the header

			// Save the data and see if everything works well
			const std::streampos dataPos = m_stream.tellp();

			m_stream.seekp(dataPos); // move the "put" pointer
			::Serialize(m_stream, x); // save the data

			// Update the start of the tables of indices in the file
			m_pTail->dataMapOffset = m_stream.tellp();

			// See if the block size we guessed was correct and update it if not
			std::streamoff blockSize = m_pTail->dataMapOffset - dataPos;

			if (header.blockSize != blockSize)
			{
				header.blockSize = blockSize; // set the correct block size
				m_stream.seekp(headerPos);    // move the "put" pointer back
				header.Serialize(m_stream);   // save the updated header
			}

			return dataPos;
		}

		/*!
			[protected] Retrieves object using data index 'di' 
			and object id 'objId' from the database.
		*/
		template <typename T> bool Read(T& x, const DataIndex& di, unsigned objId) const
		{
			if (objId >= di.size())
				return false;

			// We need to move the get pointer of the stream, but there
			// is no const function to do it, so we have to do some
			// ugly casting to get around this
			std::fstream* pStream = const_cast<std::fstream*>(&m_stream);

			pStream->seekg(di[objId], std::ios::beg);

			::Deserialize(*pStream, x);

			return true;
		}
		
	public:
		SimpleDatabase(FileType type = BINARY, int version = 1)
		{
			m_pTail = new FileTail(type, version);
			m_pIsModified = new bool(false);
		}

		~SimpleDatabase()
		{
			Close();

			delete m_pTail;
			delete m_pIsModified;
		}

		//! File name used to open/create the database
		const std::string& FileName() const
		{
			return m_filename;
		}

		/*! 
			Flushes any unwritten data only. It does not flush the 
			modified indices, which are needed to recover the data.

			@see FlushEverything()
		*/
		void FlushData()
		{
			m_stream.flush();
		}

		//! Flushes any unwritten data and the database indices
		void FlushEverything()
		{
			if (IsModified())
				WriteDataIndexMap(); // flashes data too
			else
				FlushData(); // just in case
		}

		void Close()
		{
			if (IsModified())
				WriteDataIndexMap();

			CloseStream();
		}

		bool IsModified() const 
		{ 
			return *m_pIsModified; 
		}

		/*!
			Creates a file with writing privileges. If 'onlyIfNoExist' 
			is false and the file actually exists, then it is open
			(but not truncated). If 'onlyIfNoExist' is true and the file 
			exists, the file is not created and false is returned.
		*/
		bool Create(const char* szName, bool onlyIfNoExist = false)
		{
			Clear();

			if (!CheckFileExist(szName)) // file doesn't exists
			{
				return OpenStreamAndIndexMap(szName, 
					std::ios_base::out | std::ios_base::binary 
					| std::ios_base::trunc);
			}
			else if (!onlyIfNoExist) // open the existing file
			{
				return OpenStreamAndIndexMap(szName, 
					std::ios_base::out | std::ios_base::binary);
			}

			return false;
		}

		bool OpenRead(const char* szName)
		{
			Clear();

			return OpenStreamAndIndexMap(szName, std::ios_base::in 
				| std::ios_base::binary | std::ios_base::ate);
		}

		/*!
			Returns the number of objects of type/class T.

			It must be called as:

				count = dbobj.ObjectCount<T>();
		*/
		template <typename T> unsigned ObjectCount() const
		{
			const DataIndex* pDI = m_tables.Find(vpl_TypeName(T));

			return (pDI) ? pDI->size() : 0;
		}

		/*!
			Saves objects in the database and returns an ID to retrieve it later.
		*/
		template <typename T> unsigned Save(const T& x)
		{
			// Write the data first
			std::streampos dataPos = Write(x);

			// Save the file position of the object in its class index
			DataIndex& di = m_tables[vpl_TypeName(x)];

			di.push_back(dataPos);

			SetModified(true);

			return di.size() - 1;
		}

		/*!
			Updates the object in the database with ID 'id' with
			a new object of the same class. That is, the class of 'x'
			must be the same than the class of the object being updated.
		*/
		template <typename T> void Update(unsigned id, const T& x, 
			bool assumeSameSize = false)
		{
			DataIndex& di = m_tables[vpl_TypeName(x)];

			ASSERT(id < di.size());

			const std::streampos dataPos = di[id];
			
			DataBlockHeader dbh;

			// Read the block header to find out the block size
			ReadDataBlockHeader(&dbh, dataPos);

			// If requested, we assume that the new data will fit
			// in teh available block (still check that things look normal)
			if (assumeSameSize && dbh.blockSize == sizeof(x))
			{
				m_stream.seekp(dataPos); // move the "put" pointer
				::Serialize(m_stream, x); // save the data
				
				std::streamoff sizeWritten = m_stream.tellp() - dataPos;

				if (sizeWritten > dbh.blockSize)
				{
					StreamError("There is data corruption. Object size in disk is " 
						<< sizeWritten << " but available size was " << dbh.blockSize
						<< ". The data corruption spans offset " << dataPos
						<< " and offset " << sizeWritten + dataPos);
				}
			}
			else
			{
				// Since we don't know how much space will be used the
				// the serialization function of the object,
				// we serialize it to memory first and see whether
				// the data written fits into the existing data block
				// of the object. 
				/*std::stringstream memStream;

				::Serialize(memStream, x); // save the data in memory

				unsigned sz = memStream.tellp(); // the space we need
				*/
				// If the data fits in the existing block, we just move 
				// it there. If it doesn't, we need to create a new 
				// data block and move the new data to it instead. 
				

				// In this case,
				// the old data will remain in the disk with no indices
				// pointing at it. So, it will have to be erases by 
				// a garbage collection procedure.
				
				di[id] = Write(x);
				
				SetModified(true);

				// If it doesn't fit.
				// - get best block from m_availableBlocks or create a new one
				// - remove the old block selected from m_availableBlocks (if there is one)
				// - add the new available block to m_availableBlocks
				// - now that we are done, call make_heap to let m_availableBlocks continue being a heap
			}
		}

		/*!
			Retrieves object with ID 'ii.objId' from the database.

			@param ii input/output. ii.objId must be equal to the
			database id of the object to load. On return, the
			className of 'x' and a const pointer to its associated
			DataIndex are filled in.

			@return false if the object ID is invalid and 
			true if the object was loaded.
		*/
		/*template <typename T> bool Load(T& x, IndexInfo& ii) const
		{
			ii.className = vpl_TypeName(x);
			ii.pDataIndex = m_tables.Find(ii.className);

			if (ii.pDataIndex)
				return Read(x, *ii.pDataIndex, ii.objId);
			else
				return false;
		}*/

		/*!
			Retrieves object with ID 'id' from the database.

			Returns false if the object ID is invalid and 
			true if the object was loaded.
		*/
		template <typename T> bool Load(T& x, unsigned id) const
		{
			const DataIndex* pDataIndex = m_tables.Find(vpl_TypeName(x));

			if (!pDataIndex)
				return false;

			return Read(x, *pDataIndex, id);				
		}
		
		/*!
			Retrieves the first object x in the database whose class
			is vpl_TypeName(x).

			The objId member of IndexInfo is set to the database id of
			teh object if loaded successfully, and to INVALID_STORAGE_ID
			otherwise.

			@param x the object to lead
			
			@param ii output IndexInfo object. Its initial value is
			unimportant. It returns the index information of the object loaded. 
			That is, on return, the	className of 'x' and a const pointer associated
			with its associated DataIndex are filled in. The returned IndexInfo object
			'ii' can be evaluated as a bool to determine whether 'x' was loaded. ie,
			LeadNext(x, ii); if (ii) print("x was loaded correctly");
			
			@return true if 'x' was loaded and false if there are no 
			objects of its class. 
		*/
		template <typename T> bool LoadFirst(T& x, IndexInfo& ii) const
		{
			ii.objId = 0;
			ii.className = vpl_TypeName(x);
			ii.pDataIndex = m_tables.Find(ii.className);

			if (!ii.pDataIndex || !Read(x, *ii.pDataIndex, ii.objId))
			{
				ii.objId = INVALID_STORAGE_ID;
				return false;
			}
			
			return true;
		}

		/*!
			Retrieves the next object x in the database whose class
			is the same than the last object read.

			The objId member of IndexInfo is set to the database id of
			teh object if loaded successfully, and to INVALID_STORAGE_ID
			otherwise.

			A debug exception is thrown if the class of x is different 
			from the class of the last object read.

			@param x the object to lead
			
			@param ii input/output IndexInfo object. It must initially 
			indicate the position of the previous data of the same type
			as 'x' read with either LoadFirst() or LoadNext(). It returns 
			the index information of the object loaded.	It can be
			evaluated as a bool to determine whether 'x' was loaded. ie,
			LeadNext(x, ii); if (ii) print("x was loaded correctly");
			
			@return true if 'x' was loaded and false if there are no more 
			objects of its class. 
		*/
		template <typename T> bool LoadNext(T& x, IndexInfo& ii) const
		{
			ASSERT(vpl_TypeName(x) == ii.className);
			ASSERT(ii.objId != INVALID_STORAGE_ID);
			
			++ii.objId;

			if (!Read(x, *ii.pDataIndex, ii.objId))
			{
				ii.objId = INVALID_STORAGE_ID;
				return false;
			}

			return true;
		}

		/*!
			Finds the data 'x' in the database such that 'x == y'.
			Note that &y must not be a reference to &x or to a
			member variable of x.

			@param x [output] it's the first x in the database
			       for which x == y is true, if there is such an object.
				   Otherwise, its value is undefined.

		    @param y the value or attribute that x must have.

			@return the object 'id' if the data was found and 
			        INVALID_STORAGE_ID otherwise.

		*/
		template <typename T, typename U> unsigned Find(T& x, const U& y) const
		{
			// It's easy to check that x and y aren't the same variable
			ASSERT((void*)&x != (void*)&y);

			IndexInfo ii;

			ii.objId = 0;
			ii.className = vpl_TypeName(x);
			ii.pDataIndex = m_tables.Find(ii.className);

			if (!ii.pDataIndex)
				return INVALID_STORAGE_ID;

			if (!Read(x, *ii.pDataIndex, ii.objId))
				return INVALID_STORAGE_ID;

			while (x != y)
			{
				++ii.objId;

				if (!Read(x, *ii.pDataIndex, ii.objId))
					return INVALID_STORAGE_ID;
			}

			return ii.objId;
		}

		/*!
			Lists all instances of objects with the same type than 'x' 
			in the database.
		*/
		template <typename T> void List(T& x, std::ostream& os) const
		{
			IndexInfo ii;

			ii.objId = 0;
			ii.className = vpl_TypeName(x);
			ii.pDataIndex = m_tables.Find(ii.className);

			if (ii.pDataIndex)
			{
				while (Read(x, *ii.pDataIndex, ii.objId))
				{
					x.Print(os);

					++ii.objId;
				}
			}

			os << std::endl;
		}
	};

} //namespace vpl
