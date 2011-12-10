/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <fstream> // for FileSize()
#include <vul/vul_file.h> // currently, it isn't used, but might at some point
#include "DirWalker.h"
#include "BasicUtils.h"
#include "FileAttributes.h"

#ifndef WIN32
#include <sys/stat.h>
#define INVALID_HANDLE_VALUE	0
#else
// Either undef UNICODE and _UNICODE or, in Visual Studio, go to 
// "Configuration Properties, General, Character Set" and 
// select "Not Set".
#undef _UNICODE
#undef UNICODE
# include <windows.h>
#endif

using namespace vpl;

DirWalker::DirWalker()
{ 
#ifdef WIN32
	m_pFindData = new WIN32_FIND_DATAA;
#endif

	hFile = INVALID_HANDLE_VALUE; 
	*szDirPath = '\0';
	nDirPathLen = 0;
}

DirWalker::~DirWalker() 
{ 
	CloseDir();

#ifdef WIN32
	delete m_pFindData;
#endif
}

/*!
	[static] Returns true if the path is a valid system's path.
*/ 
bool DirWalker::CheckFilePath(const char* szFullFileName)
{
	int nLen = strlen(szFullFileName);
#ifdef WIN32
	char cWrongFileSep = '/';
#else
	char cWrongFileSep = '\\';
#endif
	for (int i = 0; i < nLen; i++)
	{
		if (szFullFileName[i] == cWrongFileSep)
		{
			std::cerr << "\nError: path '" << szFullFileName 
				<< "' contains one or more " << cWrongFileSep
				<< " characters.\nUse " << FILE_SEP 
				<< " as the file-path separator character." << std::endl;
			return false;
		}
	}

	return true;
}

bool DirWalker::OpenDir(const char* szDirName)
{ 
	CloseDir();

	if (!CheckFilePath(szDirName))
		return false;

	nDirPathLen = strlen(szDirName);

#ifdef WIN32
	int nLastFileSepPos;

	if (HasWildcards(szDirName, &nLastFileSepPos))
	{
		hFile = FindFirstFile(szDirName, m_pFindData);
		nDirPathLen = nLastFileSepPos + 1;
	}
	else
	{
		char szDirNameWC[MAX_PATH_SIZE]; // needs the wildcard '*' to see all files
		strcpy(szDirNameWC, szDirName);

		if (szDirPath[nDirPathLen - 1] != FILE_SEP)
			strcat(szDirNameWC, "\\*");
		else
			strcat(szDirNameWC, "*");

		hFile = FindFirstFile(szDirNameWC, m_pFindData); // perhaps szDirName + "*.*"
	}
#else
	hFile = opendir(szDirName); 
#endif

	if (hFile != INVALID_HANDLE_VALUE)
	{
		if (nDirPathLen > 0)
		{
			memcpy(szDirPath, szDirName, nDirPathLen);
		}
		else
		{
			szDirPath[0] = '.';
			nDirPathLen = 1;
		}

		if (szDirPath[nDirPathLen - 1] != FILE_SEP)
			szDirPath[nDirPathLen++] = FILE_SEP;

		szDirPath[nDirPathLen] = '\0';

		return true;
	}
	
	return false;
}

void DirWalker::CloseDir()
{ 
	if (hFile != INVALID_HANDLE_VALUE)
	{ 
#ifdef WIN32
		FindClose(hFile);
#else
		closedir(hFile); 
#endif
		hFile = INVALID_HANDLE_VALUE;
		*szDirPath = '\0';
		nDirPathLen = 0;
	} 
}

/*! 
	\brief Returns the next file or subdirectory in the opened directory. The
	subdirectory may be '.' or '..'.

	@param szFileName pointer to a buffer where the full file name is to be written.

	@buffSize size of the buffer. The buffer has to be large enough to hold the path 
	characters, otherwise false is returned.
*/
bool DirWalker::GetNextFileOrDir(char* szFileName, size_t buffSize)
{
	const char* sz;
	size_t len;

	// Clear out szFileName before hand in case something goes wrong.
	*szFileName = '\0';

	// Read next file
#ifdef WIN32
	// We should already have a filename...
	if (*m_pFindData->cFileName == '\0')
		return false;

	sz = m_pFindData->cFileName;
	// below, and just before returning, read the next file...
#else
	struct dirent* dp;

	if ((dp = readdir(hFile)) == 0) 
		return false;

	sz = dp->d_name;
#endif

	len = strlen(sz);

	if (nDirPathLen + len > buffSize - 1)
		return false;

	if (sz[0] == '.' && (len == 1 || (len == 2 && sz[1] == '.')))
	{
		memcpy(szFileName, sz, len + 1);
	}
	else
	{
		memcpy(szFileName, szDirPath, nDirPathLen);
		memcpy(szFileName + nDirPathLen, sz, len + 1);
	}

#ifdef WIN32
	// Read the next file, so that we have it for the next call
	if (FindNextFile(hFile, m_pFindData) == 0) 
		*m_pFindData->cFileName = '\0'; // make sure filename is empty for next call
#endif

	return true;
}

/*! 
	\brief Returns the next file in the opened directory.

	@param szFileName pointer to a buffer where the full file name is to be written.

	@buffSize size of th ebuffer poninted by szFileName.

	@szFileExt pointer to a buffer in which the file extention is to be stored.
	The buffer has to be large enough to hold 3 characters. If szFileExt is NULL,
	no file extention is returned.
*/
bool DirWalker::GetNextFile(char* szFileName, size_t buffSize, char* szFileExt /*=NULL*/)
{
	bool bSuccess;

	do {
		bSuccess = GetNextFileOrDir(szFileName, buffSize);

	} while(bSuccess && IsDirectory(szFileName));

	if (szFileExt && bSuccess)
	{
		*szFileExt = '\0'; // Init the string
		size_t len = strlen(szFileName + nDirPathLen);
		char* pEnd = szFileName + nDirPathLen + len - 1;

		for (int i = 0; i < 4; i++, pEnd--)
		{
			if (*pEnd == '.')
			{
				strcpy(szFileExt, pEnd + 1);
				break;
			}
		}
	}

	return bSuccess;
}

bool DirWalker::GetNextDir(char* szFileName, size_t buffSize)
{
	bool bSuccess, bDotDirectory;

	do {
		bSuccess = GetNextFileOrDir(szFileName, buffSize);

		// Ignore dot and dot-dot directory
		if (!strcmp(szFileName, ".") || !strcmp(szFileName, ".."))
			bDotDirectory = true;
		else
			bDotDirectory = false;

	} while(bSuccess && (bDotDirectory || !IsDirectory(szFileName)));

	return bSuccess;
}

/*!
	[static] Decomposes the given file name into (path, name, extension) parts.

	These components are stored in the member variables path, name, ext.
*/
bool DirWalker::ParseFilePath(const std::string& str, std::string& path, 
							  std::string& name, std::string& ext)
{
	using namespace std;
	
	string::size_type namePos = str.rfind(FILE_SEP);
	string::size_type extPos = str.rfind('.');
	
	if (namePos == string::npos)
		namePos = 0;
	else
		namePos++;

	// Make sure that the dot comes after the las SEP. Else, 
	// it's not an extension separator. Note .Name is valid
	if (extPos <= namePos) // its equal for .Name
	{
		extPos = string::npos; // there is no extension
	}
	
	// If there is no extension, we have a special case
	if (extPos == string::npos)
	{
		ext.clear();
		name = str.substr(namePos);
	}
	else
	{
		ext = str.substr(extPos + 1, str.size() - extPos + 1);
		name = str.substr(namePos, extPos - namePos);
	}
	
	if (namePos > 0)
		path = str.substr(0, namePos);
	else
		path.clear();

	//cerr << "[" << path << "]" << "[" << name << "]" << "[" << ext << "]" << endl;
	
	return true;
}

/*!
	[static] Parses a file name of the form prefixNumer0Param1Number1Param2Number2....

	The numbers must be positive integers!
	
	The results are left in the 'params' map, such that the first key in the map is prefix
	and its value is Number0. The remaining param-number pairs are added in order. The map
	can then be accessed sequentially or by key.
	
	@param includeMalformedPair If false, then a name that doesn't end with a number, 
	will either result in an empty 'params' or it will be missing all the characters
	after the last number found. If true, a default value equal to -1 is used to
	complete the malformed key-value pair.

	Note, if a file name starts with a number, the first key is assumed to be the 
	empty string.

	@return the 'params' maps is filled with the string-value pairs in the file name. 
*/
bool DirWalker::ParseFileNameParams(const std::string& name, KeyValueList& params,
									bool includeMalformedPair)
{	
	params.clear();

	if (name.empty())
		return false;

	std::string prefix;
	std::string number;
	bool bLastChar, bIsNum, bReadingText = true;
	const unsigned lastIdx = name.size() - 1; // name isn't empty, so this is ok

	for (unsigned int s = 0, e = 1; e <= lastIdx; e++)
	{
		bLastChar = (e == lastIdx);
		bIsNum = (isdigit(name[e]) != 0);

		if (bReadingText && (bIsNum || bLastChar))
		{
			prefix = name.substr(s, (bIsNum) ? e-s:e-s+1);
			s = e;
			bReadingText = false;
			
			// Handle special case: single last digit.
			if (bIsNum && bLastChar) e--; // e is incremeted back in for loop
		}
		else if (!bReadingText && (!bIsNum || bLastChar))
		{
			number = name.substr(s, (!bIsNum) ? e-s:e-s+1);
			params.push_back(std::make_pair(prefix, atoi(number.c_str())));
			s = e;
			bReadingText = true;
		}
	}

	// If the name doesn't end with a number, it means that
	// there is a malformed KeyValue pair. ie, a key without a value.
	if (includeMalformedPair && isdigit(name[lastIdx]) == 0)
	{
		params.push_back(std::make_pair(prefix, -1));
	}

	return params.empty();
}

/*!
	[static] Parses all the file names such into key-value pairs. The 
	path-minus-filename part is added as the first key-value pair of
	each path (with a -1 value). The file extension is added as the last
	key-value pair of each path (with a -1 value).

	All parameters are positive integers except for those of the first
	and last key-value pairs.

	For example, let PathAndParams& pap = papl.front(). ie, pap is the 
	parse of the first file path. Then, the following must be true

	(a) pap.first == paths.front()
	(b) pap.second.front().first == "paths.front() minus the file name"
	(c) pap.second.front().second == -1
	(d) pap.second.back().first == "file extension in paths.front()"
	(e) pap.second.back().second == -1

	Having the path as the first key-value pair is important for sorting. If
	this behavious is not desired, the function RemovePathAndExtensionFromParamsList()
	can be called either before or after calling SortFileNamesNumerically(). 
	Also, note that the paths can be sorted usuing any sort function BEFORE 
	they are parsed, since their order is not modified.

	@param papl Output list of pairs holding the original paths and their 
	corresponding list of key-value pairs. This list can be used to sort 
	the paths numerically. The list maintains the path ordering in the input 
	list 'paths'.

	@see SortFileNamesNumerically(), RemovePathAndExtensionFromParamsList()
*/
void DirWalker::ParseFileNameParams(const StrList& paths, PathAndParamsList& papl,
									bool includeMalformedPair)
{
	std::string path, name, ext;
	KeyValueList params;

	papl.clear();

	std_forall(it, paths)
	{
		ParseFilePath(*it, path, name, ext);
		ParseFileNameParams(name, params, includeMalformedPair);

		// Add the path as the first key with value -1
		params.push_front(std::make_pair(path, -1));

		// Add the extension as the last key with value -1
		params.push_back(std::make_pair(ext, -1));

		// Add the new list of key-value pairs
		papl.push_back(std::make_pair(*it, params));
	}
}

/*!
	[static] Removes the path-minus-filename and extesnion key-value pairs added by 
	ParseFileNameParams(). There keys are useful for sorting, but may not
	be desired after, so this functions takes care of eliminating them.

	@see ParseFileNameParams()
*/
void DirWalker::RemovePathAndExtensionFromParamsList(PathAndParamsList& papl)
{
	// Remove the file path from the list of arguments
	std_forall(it, papl)
	{
		ASSERT(it->second.size() >= 2);
		ASSERT(it->second.front().second == -1);
		ASSERT(it->second.back().second == -1);

		it->second.pop_front();
		it->second.pop_back();
	}
}

/*!
	[static] Compares path and parameters numerically. Returns
	true of the second field of pap1 is smaller than
	the second field of pap2.

	@see ParseFileNameParams
*/
bool DirWalker::ComparePathAndParamsNumerically(
	const PathAndParams& pap1, const PathAndParams& pap2)
{
	const KeyValueList& kvl1 = pap1.second;
	const KeyValueList& kvl2 = pap2.second;

	KeyValueList::const_iterator it1 = kvl1.begin();
	KeyValueList::const_iterator it2 = kvl2.begin();

	for (bool isEnd1, isEnd2; ; ++it1, ++it2)
	{
		isEnd1 = (it1 == kvl1.end());
		isEnd2 = (it2 == kvl2.end());

		if (isEnd1 && isEnd2)
			return false; // lists are equal (reached end simultaneously)
		else if (isEnd1 && !isEnd2)
			return true; // 1 reached end before 2
		else if (!isEnd1 && isEnd2)
			return false; // 2 reached end before 1
		else if (it1->first < it2->first)
			return true; // key1 is smaller than key2
		else if (it1->first > it2->first)
			return false; // key2 is smaller than key1
		else if (it1->second < it2->second)
			return true; // value1 is smaller than value2
		else if (it1->second > it2->second)
			return false; // value2 is smaller than value1

		// else => neither list reached the end and the 
		//         current keys and values are equal
	}

	// We should never be here
	ASSERT(false);

	return true;
}

/*!
	[static] Sorts the file names numerically using its parameters.

	@see ParseFileNameParams
*/
void DirWalker::SortFileNamesNumerically(PathAndParamsList& papl)
{
	papl.sort(ComparePathAndParamsNumerically);
}

/*!
	[static] Sorts the file names numerically.
*/
void DirWalker::SortFileNamesNumerically(StrList& paths)
{
	PathAndParamsList papl;

	// Get a list of key-value pairs form each filename. The first 
	// key-value pair is (path-minus-filename,-1) and the last one
	// is (file-extension,-1).
	ParseFileNameParams(paths, papl, true);

	// Sort the files names by interpreting numeric characters as numbers
	SortFileNamesNumerically(papl);

	paths.clear();

	std_forall(it, papl)
	{
		paths.push_back(it->first);
	}

	/*
	// Example of how to list the info in the papl
	PathAndParamsList::iterator it0;
	KeyValueList::iterator it1;

	// Remove the file paths and extensions from the list of arguments
	RemovePathAndExtensionFromParamsList(papl);

	std_forall(it0, papl)
	{
		std::cout << it0->first << ":  ";

		std_forall(it1, it0->second)
			std::cout << it1->first << " = " << it1->second << ", ";

		std::cout << std::endl;
	}*/
}

/*!
	Concatenates 'subPath' at the end of the 'path' member variable
*/
void DirWalker::AddToPath(const char* szSubPath)
{
	unsigned int n = m_path.size();

	if (n > 0 && m_path[n - 1] != FILE_SEP)
		m_path.append(1, FILE_SEP);

	m_path.append(szSubPath);
}

/*! 
	@brief Finds wildcards {'*', '?'} in the path. Returns false if there are no
	wildcards and true otherwise.
*/
/*static*/
bool DirWalker::HasWildcards(const char* szPath)
{
	for (; *szPath != '\0'; szPath++)
	{
		if (*szPath == '*' || *szPath == '?')
			return true;
	}

	return false;
}

/*!
	@brief Checks whether the path contains wildcards.

	@return pLastFileSepPos is set to th eposition of the last FILE_SEP
	        character or -1 if there is no such character.
*/
/*static*/
bool DirWalker::HasWildcards(const char* szPath, int* pLastFileSepPos)
{
	*pLastFileSepPos = -1;

	for (int i = 0; *szPath != '\0'; szPath++, i++)
	{
		if (*szPath == '*' || *szPath == '?')
			return true;

		if (*szPath == FILE_SEP)
			*pLastFileSepPos = i;
	}

	return false;
}

/*static*/ 
bool DirWalker::ReadFileAttributes(const char* szFileName, FileAttributes* pAtts)
{
   ASSERT(pAtts);

   if (stat(szFileName, pAtts) != 0) 
   {  
	   StreamError("Cannot read file attributes for " << szFileName 
		   << " due to " << strerror(errno));

      return false;
   }

	return true;
}

/* static */
time_t DirWalker::ReadCreationTime(const std::string& filename)
{
	FileAttributes fa;
	
	DirWalker::ReadFileAttributes(filename.c_str(), &fa);
	
	return fa.st_ctime;
}

/*static*/ 
bool DirWalker::IsDirectory(const char* szFileName)
{
#ifdef WIN32
	DWORD info = GetFileAttributes(szFileName);
	return (info & FILE_ATTRIBUTE_DIRECTORY) != 0;
#else
	struct stat info;

	if (stat(szFileName, &info) != 0)
		return false;
	else
		return (info.st_mode & S_IFDIR) != 0;
#endif
}

/*static*/ 
const char* DirWalker::FindFileExtension(const char* szFileName)
{
	size_t len = strlen(szFileName);
	const char* pEnd = szFileName + len - 1;

	for (; pEnd > szFileName && *pEnd != FILE_SEP; pEnd--)
		if (*pEnd == '.')
			return pEnd + 1;

	return szFileName + len;
}

/*static*/ 
bool DirWalker::CompareFileExtension(const char* szFileName, const char* szFileExt)
{
	return strcmp(szFileExt, FindFileExtension(szFileName)) == 0;
}

/*!
	@brief Changes the extension component of the given file name by replacing
	it with the given extension, or removing it if szExt is NULL.
*/
/*static*/ 
void DirWalker::ChangeFileExt(const char* szFileName, const char* szExt, char* szNewName)
{
	const char* szOldExt = FindFileExtension(szFileName);
	int nBytes = szOldExt - szFileName;

	memcpy(szNewName, szFileName, nBytes);

	if (szExt)
	{
		if (szNewName[nBytes - 1] != '.')
			szNewName[nBytes++] = '.';

		strcpy(szNewName + nBytes, szExt);
	}
	/* szExt == NULL*/
	else if (szNewName[nBytes - 1] == '.')
		szNewName[nBytes - 1] = '\0';
	else
		szNewName[nBytes] = '\0';
}

/*!
	@brief Returs the last-modification time difference between szFileName1 and szFileName2
	returns:
		-1 First file time is earlier than second file time. 
		0 First file time is equal to second file time. 
		1 First file time is later than second file time. 
*/
/*static*/
int DirWalker::CompareLastModifDate(const char* szFileName1, const char* szFileName2)
{
#ifndef WIN32
	struct stat info1, info2;

	info1.st_mtime = info2.st_mtime = 0;

	stat(szFileName1, &info1);
	stat(szFileName2, &info2);

	return info1.st_mtime - info2.st_mtime;
#else
	WIN32_FIND_DATA findData1, findData2;
	HANDLE hFile;
	
	if ((hFile = FindFirstFile(szFileName1, &findData1)) == INVALID_HANDLE_VALUE)
		return 1;
	
	FindClose(hFile);

	if ((hFile = FindFirstFile(szFileName2, &findData2)) == INVALID_HANDLE_VALUE)
		return 1;

	FindClose(hFile);

	return CompareFileTime(&findData1.ftLastWriteTime, &findData2.ftLastWriteTime);
#endif
}

/*!
	[static] Makes the given name unique by adding a unique ID as a subfix
	of the file name if necessary. The ID is choose as one plus
	the maximum ID dound in the given path.

	@return the chosen ID or -1 if no ID was added.
*/
int DirWalker::MakeNameUnique(std::string* pStrFileName)
{
	char szNewFileName[MAX_PATH_SIZE];
	DirWalker dw;
	
	ASSERT(pStrFileName);

	dw.ParseFilePath(pStrFileName->c_str());

	int id = CreateUniqueFileName(dw.Path().c_str(), dw.Name().c_str(), 
		dw.Ext().c_str(), szNewFileName);

	if (id >= 0)
		*pStrFileName = szNewFileName;

	return id;
}

/*!
	@brief Finds the maximum subfix of any file that has
	the given name prefix and extension

	@return -1 if the given directory is invalid
			0 if there is no file that matches the name
			n in [1, MAXINT] s.t. n is the number of matching subfix found
*/
int DirWalker::CountNumberedFiles(const char* szDirName, const char* szPrefix, 
								  const char* szFileExt, 
								  int* pMaxIdx, int* pMinIdx)
{
	// If we are given an empty path, set it to the currrent path
	if (*szDirName == '\0')
		szDirName = "."; // ie, just copy ptr adress no str content

	// Init the max and min indices if given
	if (pMaxIdx)
		*pMaxIdx = -1;

	if (pMinIdx)
		*pMinIdx = -1;

	// Open the given directory
	if (!IsDirectory(szDirName) || !OpenDir(szDirName))
		return -1;

	char szFileName[MAX_PATH_SIZE];
	int nPrefixLen = strlen(szPrefix);
	int nPathLen   = GetDirPathLen();
	char szExt[10];
	int n, nCount = 0;

	while (GetNextFile(szFileName, MAX_PATH_SIZE, szExt))
	{
		// Find the maximum subfix of any file that has
		// the given name prefix and extension
		if (!strncmp(szPrefix, szFileName + nPathLen, nPrefixLen) && 
			!strcmp(szFileExt, szExt))
		{
			nCount++;

			// make use of the fact that atoi ignores characters
			n = atoi(szFileName + nPathLen + nPrefixLen);

			if (pMaxIdx && n > *pMaxIdx)
				*pMaxIdx = n;

			if (pMinIdx && (*pMinIdx < 0 || n < *pMinIdx))
				*pMinIdx = n;
		}
	}

	return nCount;
}

/*!
	[static] Creates a unique file name with the given prefix and extension of the 
	form 'szDirNameprefix.ext' or 'szDirNameprefixID.ext'. Here ID is added to make
	the file name unique in the given directory.

	ID is the next number corresponding to the gratest ID of file names 
	in the given directory with the same name form.

	\param szDirName directory name.
	\param szPrefix prefix for the file
	\param szFileExt extension of the file
	\param szFileName buffer where the created file name is stored. 
		    Should be of size MAX_PATH_SIZE.
	\return the ID added to the file name, or -1 if no ID was added.
*/
int DirWalker::CreateUniqueFileName(const char* szDirName, const char* szPrefix, 
									const char* szFileExt, char szFileName[])
{
	DirWalker dw;

	int nMax, nCount;
	
	nCount = dw.CountNumberedFiles(szDirName, szPrefix, szFileExt, &nMax);

	if (nCount == -1)
		*szFileName = '\0';	// init return variable to a valid string
	else if (nCount == 0)
		sprintf(szFileName, "%s%s.%s", dw.GetDirPath(), szPrefix, szFileExt);
	else
		sprintf(szFileName, "%s%s%d.%s", dw.GetDirPath(), szPrefix, ++nMax, szFileExt);

	return nMax;
}

/*!
	[static] Appends szSubfix to the file name. If szNewFileExtension is not NULL,
	it is used as the new extension of the file.
*/
std::string DirWalker::AppendToFileName(const char* szFileName, const char* szSubfix,
	const char* szNewFileExtension /*=NULL*/)
{
	DirWalker dw;

	dw.ParseFilePath(szFileName);

	std::string newFileName = dw.Path() + dw.Name() + szSubfix;

	if (szNewFileExtension != NULL)
	{
		newFileName += ".";
		newFileName += szNewFileExtension;
	}
	else if (dw.Ext().length() > 0)
	{
		newFileName += ".";
		newFileName += dw.Ext();
	}

	return newFileName;
}

/*!
	[static] Appends a number to the file name extension.
*/
std::string DirWalker::AppendToExtension(const char* szFileName, int num)
{
	char szNewFileName[MAX_PATH_SIZE];

	sprintf(szNewFileName, "%s%d", szFileName, num);

	return szNewFileName;
}

/*!
	[static] Assumes that the given text is of the form PrefixNumber, and splits 
	it accordingly.
	
	It is assume that the buffer pointed by szPrefix is big enough to store
	the prefix.
*/
void DirWalker::SplitPrefixAndNumber(const char* szText, char* szPrefix, int* pNumber)
{
	const int len = strlen(szText);

	*szPrefix = '\0';
	*pNumber = -1;

	for (int i = 0; i < len; i++)
	{
		if (isdigit(szText[i]))
		{
			*pNumber = atoi(szText + i);

			if (i > 0)
			{
				strncpy(szPrefix, szText, i);
				szPrefix[i] = '\0';
			}

			break;
		}
	}
}

/*!
	[static] Returs the size of the file of -1 if the file does not exist
*/
int DirWalker::FileSize(const char* szFileName)
{
	std::fstream auxFile;
	
	// Set 'ate' (at end) mode so that tellg > 0 if file is not empty
	auxFile.open(szFileName, std::ios_base::in | std::ios_base::binary 
		| std::ios_base::ate);

	return (auxFile.fail()) ? -1 : auxFile.tellg();
}

/*!
	[static] 
*/
std::string DirWalker::ConcatPathElements(const StrList& elems)
{
	ASSERT(!elems.empty());

	StrList::const_iterator it = elems.begin();
	std::string fullStr = *it;
	
	for (++it; it != elems.end(); ++it)
	{
		if (last_char(fullStr) != FILE_SEP)
			fullStr += FILE_SEP;

		fullStr += *it;
	}

	return fullStr;
}

/*!
	[static] 
*/
std::string DirWalker::ConcatPathElements(const std::string& str1, 
										  const std::string& str2)
{
	std::string fullStr = str1;

	WARNING(str1.empty(), "Cancatenating empty string in param 1");
	WARNING(str2.empty(), "Cancatenating empty string in param 2");

	if (fullStr.empty() || last_char(fullStr) != FILE_SEP)
		fullStr += FILE_SEP;

	return fullStr += str2;
}

/*!
	[static] Appends to 'paths' all files paths under the root directory
	whose file name match the glob expression (eg, *.bmp). 
	
	The subdirectories are read recursively up to 'maxLevels'.

	Note: the glob does not affect the selection of subdirectories.
*/
void DirWalker::CollectFileNames(const std::string& rootDir, const std::string& glob,
								 unsigned maxLevel, StrList& paths, 
								 unsigned curLevel /*= 0*/)
{
	char szFileName[MAX_PATH_SIZE];
	DirWalker dw;

	if (curLevel < maxLevel)
	{
		if (dw.OpenDir(rootDir.c_str()))
		{
			while (dw.GetNextDir(szFileName, MAX_PATH_SIZE))
			{
				CollectFileNames(szFileName, glob, maxLevel, 
					paths, curLevel + 1);
			}
		}
	}

	if (dw.OpenDir(ConcatPathElements(rootDir, glob).c_str()))
	{
		while (dw.GetNextFile(szFileName, MAX_PATH_SIZE))
		{
			paths.push_back(szFileName);
		}
	}
}
/*!
	[static] Takes an absolute path and turns it into a relative path.
*/
std::string DirWalker::RelativePath(const char* absolutePath, const char* relativePath)
{
	/*string[] absoluteDirectories = absolutePath.Split(PATH_SEP);
    string[] relativeDirectories = relativeTo.Split(PATH_SEP);

    //Get the shortest of the two paths
    int length = absoluteDirectories.Length < relativeDirectories.Length ? absoluteDirectories.Length : relativeDirectories.Length;

    //Use to determine where in the loop we exited
    int lastCommonRoot = -1;
    int index;

    //Find common root
    for (index = 0; index < length; index++)
	{
        if (absoluteDirectories[index] == relativeDirectories[index])
            lastCommonRoot = index;
        else
            break;
	}

    //If we didn't find a common prefix then throw
    if (lastCommonRoot == -1)
	{
        ShowError("Paths do not have a common base");
		return std::string();
	}

    //Build up the relative path
    StringBuilder relativePath = new StringBuilder();

    //Add on the ..
    for (index = lastCommonRoot + 1; index < absoluteDirectories.Length; index++)
        if (absoluteDirectories[index].Length > 0)
            relativePath.Append("..\\");

    //Add on the folders
    for (index = lastCommonRoot + 1; index < relativeDirectories.Length - 1; index++)
        relativePath.Append(relativeDirectories[index] + "\\");

    relativePath.Append(relativeDirectories[relativeDirectories.Length - 1]);

    return relativePath.ToString();*/
	return std::string();
}

/*!
	[static] Takes a (possibly) relative path and turns it into an absolute path. 
	If the path is already absolute, it does nothing to it.
*/
std::string DirWalker::AbsolutePath(const char* path)
{
	return std::string();
}

/*!
	[static] 
*/
std::string DirWalker::CurrentDirectory()
{
	char path[MAX_PATH_SIZE];

	if (::GetCurrentDirectory(MAX_PATH_SIZE, path) > 0)
		return path;
	else
		return std::string();
}

/*!
	[static] 
*/
bool DirWalker::ChangeCurrentDirectory(const char* workdir)
{
	return (::SetCurrentDirectoryA(workdir) == TRUE);
}

/*!The following sample code will search the current  directory
     for the entry name:*/
/*int ReadDir(char* name)
{
	DIR* dirp = opendir(".");
	struct dirent* dp;
	int errno;

	while (dirp) 
	{
		errno = 0;

		if ((dp = readdir(dirp)) != NULL) 
		{
			DBG_MSG(dp->d_name)
			if (strcmp(dp->d_name, name) == 0) 
			{
				closedir(dirp);
				return FOUND;
			}
		} 
		else 
		{
			if (errno == 0) 
			{
				closedir(dirp);
				return NOT_FOUND;
			}

			closedir(dirp);

			return READ_ERROR;
		}
	}

	return OPEN_ERROR;
}*/
