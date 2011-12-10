/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/STLUtils.h>

#ifdef WIN32
typedef void* HANDLE;
struct _WIN32_FIND_DATAA;
#else
# include <sys/types.h>
# include <dirent.h>
typedef DIR* HANDLE;
#endif

namespace vpl {

struct FileAttributes;

/*!
	DirWalker::iterator it = dw.params.begin();

	for (it = dw.params.begin(); it != dw.params.end(); ++it)
	{
		it->first // param name (std::string)
		it->second // param value (int)
	}
*/
class DirWalker
{
	HANDLE hFile;
	char szDirPath[MAX_PATH_SIZE];
	size_t nDirPathLen;

#ifdef WIN32
	_WIN32_FIND_DATAA* m_pFindData;
#endif

public:	
	typedef std::pair<std::string, int> KeyValuePair;
	typedef std::list<KeyValuePair> KeyValueList;

	typedef KeyValueList::const_iterator const_iterator;
	typedef KeyValueList::iterator iterator;

	typedef std::pair<std::string, KeyValueList> PathAndParams;
	typedef std::list<PathAndParams> PathAndParamsList;

protected:
	std::string m_path, m_name, m_ext;
	KeyValueList m_params;

public:
	DirWalker();
	~DirWalker();

	bool OpenDir(const char* szDirName);
	void CloseDir();

	///////////////////////////////////////////////////////////////////////////
	// Parse functions
	bool ParseFilePath(const std::string& fileName)
	{
		return ParseFilePath(fileName, m_path, m_name, m_ext);
	}

	bool ParseFilePath(const char* szFileName)
	{
		return ParseFilePath(std::string(szFileName));
	}

	bool ParseFileNameParams(bool includeMalformedPair)
	{
		return ParseFileNameParams(m_name, m_params, includeMalformedPair);
	}

	static bool ParseFilePath(const std::string& fileName, std::string& path, 
							  std::string& name, std::string& ext);

	static bool ParseFileNameParams(const std::string& name, KeyValueList& params,
		bool includeMalformedPair);

	static void ParseFileNameParams(const StrList& paths, PathAndParamsList& papl,
		bool includeMalformedPair);

	///////////////////////////////////////////////////////////////////////////
	// Sort functions
	static void SortFileNamesNumerically(PathAndParamsList& paps);

	static void SortFileNamesNumerically(StrList& paths);

	///////////////////////////////////////////////////////////////////////////
	// Comparison functions
	static bool ComparePathAndParamsNumerically(const PathAndParams& pap1, 
		const PathAndParams& pap2);

	static bool CompareFileExtension(const char* szFileName, 
		const char* szFileExt);

	static int CompareLastModifDate(const char* szFileName1, 
		const char* szFileName2);

	///////////////////////////////////////////////////////////////////////////
	// Concatenation functions
	static std::string ConcatPathElements(const StrList& elems);
	static std::string ConcatPathElements(const std::string& str1, const std::string& str2);

	static std::string ConcatPathElements(const char* str1, const char* str2)
	{
		return ConcatPathElements(std::string(str1), std::string(str2));
	}

	static std::string ConcatPathElements(const char* str1, const std::string& str2)
	{
		return ConcatPathElements(std::string(str1), str2);
	}

	static std::string ConcatPathElements(const std::string& str1, const char* str2)
	{
		return ConcatPathElements(str1, std::string(str2));
	}

	///////////////////////////////////////////////////////////////////////////
	// Check functions
	static void ChangeFileExt(const char* szFileName, 
		const char* szExt, char* szNewName);

	static bool CheckFilePath(const char* szFullFileName);

	//! Returns true iff the file exists and has non-zero size
	static bool CheckFileExist(const char* szFileName)
	{
		return (FileSize(szFileName) > 0);
	}

	///////////////////////////////////////////////////////////////////////////
	// Replace and Remove functions
	/*!
		Uses 'path in place of the path of "dest" in order to 
		create the return filename.
	*/
	static std::string ReplacePath(const char* dest, const char* path)
	{
		DirWalker dw;

		dw.ParseFilePath(dest);

		std::string fname = dw.Name();

		if (!dw.Ext().empty())
			 fname += '.' + dw.Ext();

		return ConcatPathElements(path, fname);
	}

	static void RemoveFileExt(const char* szFileName, char* szNewName)
	{
		ChangeFileExt(szFileName, NULL, szNewName);
	}

	static std::string RemoveFileExt(const char* szFileName)
	{
		DirWalker dw;
		dw.ParseFilePath(szFileName);
		return dw.Path() + dw.Name();
	}

	static void RemovePathAndExtensionFromParamsList(PathAndParamsList& papl);

	///////////////////////////////////////////////////////////////////////////
	// Get and Property functions
	const std::string& Path() const { return m_path; }
	const std::string& Name() const { return m_name; }
	const std::string& Ext() const  { return m_ext; }
	const KeyValueList& Params() const { return m_params; }

	const char* GetDirPath() { return szDirPath; }
	int GetDirPathLen() { return nDirPathLen; }

	bool GetNextFileOrDir(char* szFileName, size_t buffSize);
	bool GetNextFile(char* szFileName, size_t buffSize, char* szFileExt = NULL);
	bool GetNextDir(char* szFileName, size_t buffSize);

	static bool HasWildcards(const char* szPath);
	static bool HasWildcards(const char* szPath, int* pLastFileSepPos);
	static bool IsDirectory(const char* szFileName);

	static int FileSize(const char* szFileName);

	static std::string GetName(const char* szFileName)
	{
		DirWalker dw;
		dw.ParseFilePath(szFileName);
		return dw.Name();
	}

	static std::string GetPath(const char* szFileName)
	{
		DirWalker dw;
		dw.ParseFilePath(szFileName);
		return dw.Path();
	}

	static std::string GetExtension(const char* szFileName)
	{
		DirWalker dw;
		dw.ParseFilePath(szFileName);
		return dw.Ext();
	}

	///////////////////////////////////////////////////////////////////////////
	// Append functions
	static std::string AppendToFileName(const char* szFileName, 
		const char* szSubfix, const char* szNewFileExtension = NULL);

	static std::string AppendToExtension(const char* szFileName, int num);

	///////////////////////////////////////////////////////////////////////////
	// Other functions
	void AddToPath(const char* subPath);

	static const char* FindFileExtension(const char* szFileName);
	static void SplitPrefixAndNumber(const char* szText, char* szPrefix, int* pNumber);
	static void CollectFileNames(const std::string& rootDir, const std::string& glob,
		unsigned maxLevel, StrList& paths, unsigned curLevel = 0);

	int CountNumberedFiles(const char* szDirName, const char* szPrefix, 
		const char* szFileExt, int* pMaxIdx = NULL, int* pMinIdx = NULL);

	static int CreateUniqueFileName(const char* szDirName, const char* szPrefix, 
		const char* szFileExt, char szFileName[]);

	static int MakeNameUnique(std::string* pStrFileName);

	static std::string CurrentDirectory();

	static bool ChangeCurrentDirectory(const char* workdir);

	static std::string RelativePath(const char* absolutePath, const char* relativePath);
	static std::string AbsolutePath(const char* path);

	static bool ReadFileAttributes(const char* szFileName, FileAttributes* pAtts);

	static time_t ReadCreationTime(const std::string& filename);
};

} // namespace vpl
