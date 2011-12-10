/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <FL/Fl.H>
#include "VideoParserUI.h"
#include "threads.h"
#include <Tools/UserArguments.h>
#include <Tools/DirWalker.h>
#include <Tools/SimpleDatabase.h>
#include <Tools/Tuple.h>
#include <vul/vul_file.h>

#include <io.h>

using namespace vpl;

//#define VPL_SUCCESSFUL_RUN  0
//#define VPL_NOTHING_DONE    1

vpl::UserArguments g_userArgs;

#ifdef WIN32
#include <Shlobj.h>

bool CreateProcess(const char* szAppName, const char* szArgs, 
	PROCESS_INFORMATION* pProcInfo);

bool CreateProcessAndWait(const char* szAppName, const char* szArgs);
#else

#endif

void ReadPreferences();
void WritePreferences();

std::string GetDocumentsFolder();
std::list<std::string> ReadFieldProperties(std::string fieldKey);
bool RunExperiments(const char* defaultParamsFilename = NULL);
bool RunProcesses(const char* appName);
bool ReadSavedUserArguments(std::string databaseFilename);
void HandleStandardIORedirection();
void ReadMainUserArguments();
void HandleSavedUserArguments();

/*!
	Command line options.

	-e look for ExperimentsN sections (N = 0, 1, 2, ...)
	-p look for ProcessN sections (N = 0, 1, 2, ...)

	Rules:

	 a) There can be only one OPTION parameter and one filename (in any order).
	 b) If no parameter file is provided, the default one is "params.txt"
	    and is opened from UserDocuments\VideoParser.
	 c) If no parameter file is provided, the default filename is "params.txt";
	 d) If no parameter file is provided, the program first tries to execute the
	    specified processes (from default param file). If there aren't any, it then 
		tries to run the specified experiments. If there are none, it opens the GUI.
	 e) NOGUI_OPT => The current params file is used as the experiments file. ie,
	    the program doesn't read the Experiments {} section. It simply assumes that
		the current file is the experiment to run.
	 f) EXPERIMENTS_OPT => The Experiments {} section is read and the experiments in
	    it are run in batch mode. It exists when done.
	 g) PROCESSES_OPT => The Processes {} section is read and the processes in it are
	    executed. If 'exitWhenDone' is true (default), the program exists after running all 
		the processes. Otherwise, it runs the GUI using the parameters specified in the
		current parms file.
	 h) GUI_OPT => Runs the GUI without reading the Processes or Experiments sections.
*/
int main(int argc, char **argv) 
{
	const int numOptions = 4;
	char* options[numOptions] = {"-gui", "-nogui", "-e", "-p"};
	enum {GUI_OPT, NOGUI_OPT, EXPERIMENTS_OPT, PROCESSES_OPT}; // ordered according to 'options' array
	int optIdx = -1;
	std::string paramFile;
	std::list<const char*> cmdOpt;

#ifdef _DEBUG
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif

	// Look for the first argument that is not one of the
	// valid command options and for the first command option
	for (int i = 1; i < argc && optIdx < 0; i++)
	{
		// There can be only one OPTION parameter
		for (int j = 0; j < numOptions && optIdx < 0; j++)
			if (_stricmp(argv[i], options[j]) == 0)
				optIdx = j;

		// If the OPTION parameter is not set during this iteration, 
		// then the parameter i must be the filename
		if (optIdx < 0)
			paramFile = argv[i];
	}

	// Set a default value for the cmd option
	if (optIdx < 0)
		optIdx = NOGUI_OPT;

	// If no parameter file is provided, use default one
	if (paramFile.empty())
	{
		std::string workDir = 
		DirWalker::ConcatPathElements(GetDocumentsFolder(), "VideoParser");

		paramFile = "params.txt";

		// Change the working directory. If it fails, it means that the
		// default directory does not exist. That might not be a problem
		// if there is an actual params.txt in the current working dir.
		//if (::SetCurrentDirectoryA(workDir.c_str()) == FALSE)
		if (!DirWalker::ChangeCurrentDirectory(workDir.c_str()))
		{
			StreamMsg("The default working folder " << workDir 
				<< " does not exist. Looking for " << paramFile
				<< " in the current working directory.");
		}
	}

	/*if (!DirWalker::CheckFileExist(paramFile.c_str()))
	{
		ShowStatus1("There is no parameter file", paramFile);
	}*/

	if (!g_userArgs.ReadParameters(paramFile.c_str()))
	{
		ShowOpenFileError(paramFile);
		ShowUsage("videoparser [-opt] [parameter_file]");
		return 1;
	}

	// Read the main user arguments: verbose mode, work dir, etc
	ReadMainUserArguments();

	if (argc == 1) // the no-cmd-line-params case
	{
		// Try running processes first!
		if (RunProcesses(argv[0]))
			return 0;
		else if (RunExperiments())
			return 0;

		// otherwise, just create GUI and let user decide what to do
	}
	else if (optIdx == NOGUI_OPT) 
	{
		// Use current params file as default experiments file
		RunExperiments(paramFile.c_str());
		return 0;
	}
	else if (optIdx == EXPERIMENTS_OPT)
	{
		if (!RunExperiments())
			ShowError("Did not find any experiments to run.");

		// Must always exist because 'g_userArgs' is changed
		// when running experiments
		return 0;
	}
	else if (optIdx == PROCESSES_OPT)
	{
		if (!RunProcesses(argv[0]))
			ShowError("Did not find any processes to run.");

		bool exitWhenDone;

		// See if we are required to exit when done
		g_userArgs.ReadBoolArg("Main", "exitWhenDone", 
			"Exit the program after running all processes?", 
			true, &exitWhenDone);

		if (exitWhenDone)
			return 0;
	}
	else
	{
		ASSERT(optIdx == GUI_OPT || optIdx == -1);
	}

	// We need to create a GUI for the user
	VideoParserUI* cvui = new VideoParserUI;
    
    Fl::visual(FL_DOUBLE | FL_INDEX);
	//Fl::visual(FL_RGB);

	Fl::scheme("plastic");

	ReadPreferences();

	// See if the user wants to retrived saved parameters
	HandleSavedUserArguments();

	cvui->show(1, argv); // provide only program name
    
    Fl::lock(); // must do this before creating any threads!

	int retVal = Fl::run();

	cvui->mainWindow->Finalize();

	delete cvui;

	return retVal;
}

/*!
	See whether the user wants to be in verbose mode.

	This function is called by main() and also by 
	RunExperiments(), becuase they might read 
	different user parameter files.
*/
void ReadMainUserArguments()
{
	// Get the verbose mode parameter from the file
	g_userArgs.ReadArg("Main", "verboseMode", Tokenize("silent minimal medium all"),
		"Level of verbosity", 0, &g_verboseMode);

	std::string workDir;

	g_userArgs.ReadArg("Main", "workingDirectory", 
		"Sets the working directory", workDir, &workDir);

	if (!workDir.empty())
	{
		// @todo there seems be a bug with change_directory() and, at least in Win32,
		// it returns false when the operationm succeeds and false otherwise
		if (!vul_file::change_directory(workDir))
			ShowStatus1("Changing working directory to", workDir);
		else
			ShowError1("Cannot open working directory", workDir);
	}

	// See if the user wants to redirect the standard I/O streams
	HandleStandardIORedirection();
}

/*!
	See whether the user wants to redirect the standard I/O C handles and C++ streams.

	See Documentation/UsefulWepages/ioredirection.html
*/
void HandleStandardIORedirection()
{
#ifdef WIN32
	std::string stdoutFile;

	g_userArgs.ReadArg("Main", "alternateStdOut", 
		"[Experiments mode] Filename for redirecting all stdout output", 
		std::string(), &stdoutFile);

	if (!stdoutFile.empty())
	{
		HANDLE hFile = CreateFileA(stdoutFile.c_str(),
			GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, 
			CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			int hHandle = _open_osfhandle((long)hFile, 0);
			FILE* fp = _fdopen(hHandle, "w");
			*stdout = *fp;
			std::ios::sync_with_stdio();

			std::cout << "Std is redirected" << std::endl;
		}
		else
		{
			ShowOpenFileError(stdoutFile);
		}
	}
#endif
}

void HandleSavedUserArguments()
{
	bool readParamsFromDatabase;
	std::string databaseFilename;

	// See if we are required to load previously saved parameters
	g_userArgs.ReadBoolArg("Main", "readParamsFromDatabase", 
		"[Experiments mode] Read parameters from a database"
		" and replace current ones?", false, &readParamsFromDatabase);

	if (readParamsFromDatabase)
	{
		// Read the name of the database where the params were saved
		g_userArgs.ReadArg("Main", "databaseFilename", 
			"Filename of the database in which the"
			" user params are stored", std::string(), &databaseFilename);

		if (databaseFilename.empty())
		{
			vpl::Tuple<std::string, 2, '(', ')'> ref("ObjectLearner", "modelDBPath");

			g_userArgs.ReadArg("Main", "databaseReference", "Reference to the"
				" field-property that has the database filename in which the"
				" user params are stored", ref, &ref);

			if (!ref[0].empty() && !ref[1].empty())
			{
				g_userArgs.ReadArg(ref[0], ref[1], NULL, 
					std::string(), &databaseFilename);
			}
		}

		if (databaseFilename.empty())
		{
			ShowError("There is no database filename specified"
				" for retrieving saved parameters");
		}
		else if (!ReadSavedUserArguments(databaseFilename))
		{
			ShowError1("Cannot retrieved the parameters stored in",
				databaseFilename);
		}
	}
}

/*!
	Reads all the fields and properties serialied to
	the given data base. Such fields correspond to those
	that were declared as serializable using the funtion:

	g_userArgs.AddSerializableFieldKey("field_key");

	returs true iff the operation was performed successfuly.
*/
bool ReadSavedUserArguments(std::string databaseFilename)
{
	if (databaseFilename.empty())
	{
		ShowError("The database filename is empty");
		return false;
	}

	vpl::SimpleDatabase db;

	if (!DirWalker::CheckFileExist(databaseFilename.c_str()))
	{
		StreamError("Database file '" << databaseFilename << "' does not exist");
		return false;
	}

	if (!db.Create(databaseFilename.c_str(), false))
	{
		ShowOpenFileError(databaseFilename);
		return false;
	}

	vpl::SimpleDatabase::IndexInfo ii;

	if (!db.LoadFirst(g_userArgs, ii))
	{
		ShowError1("There are no parameters to load from the database", 
			databaseFilename);
		return false;
	}
		
	ShowStatus("Updating user's parameters with saved ones");

	return true;
}

std::string GetDocumentsFolder()
{
	char szPath[MAX_PATH];

	SHGetSpecialFolderPathA(0, szPath, CSIDL_PERSONAL, FALSE);

	return szPath;
}

std::list<std::string> ReadFieldProperties(std::string fieldKey)
{
	int minIdx, maxIdx;
	std::list<std::string> fnames;
	char propName[100];

	if (g_userArgs.GetPropertyPrefixMinMax(fieldKey, 
			std::string("paramsFilename"), &minIdx, &maxIdx))
	{
		for (int idx = minIdx; idx <= maxIdx; idx++)
		{
			sprintf(propName, "paramsFilename%d", idx);

			// The min and max don't guarantee that all vals in
			// between exist, so catch any problems
			try {
				fnames.push_back(g_userArgs.GetStrValue(
					fieldKey, std::string(propName)));
			}
			catch(BasicException e) 
			{
				ShowError1("Didn't find filename property", propName);
			}
		}
	}

	return fnames;
}

/*!
	Looks for experiments fields, and process 
	the set of commands specified by each 'paramsFilenameN'
	property of the field.

	Note: if there are no experients fields, the 'defaultParamsFilename'
	is used as the set of commands to run (if it's not null). 
	Otherwise, 'defaultParamsFilename' is ignored.
*/
bool RunExperiments(const char* defaultParamsFilename)
{
	std::list<std::string> fnames = ReadFieldProperties("Experiments");
		
	// If there are no experiments file, use default file (if provided)
	if (fnames.empty())
	{
		if (!defaultParamsFilename)
			return false;
		else
			fnames.push_back(defaultParamsFilename);
	}

	for (auto it = fnames.begin(); it != fnames.end(); ++it)
	{
		ShowStatus1("\n\nProcessing experiment param file:", *it);

		vpl::VideoProcessor vp;

		if (g_userArgs.ReadParameters(it->c_str()))
		{
			// Read the main user arguments for the new params file
			ReadMainUserArguments();

			// See if the user wants to retrieve saved parameters
			HandleSavedUserArguments();

			// Let the component read the user arguments
			vp.Initialize();

			// Process the video
			vp.ProcessVideo();
		}
		else
		{
			ShowOpenFileError(it->c_str());
		}
	}

	return true;
}

/*!

*/
bool RunProcesses(const char* appName)
{
	std::list<std::string> fnames = ReadFieldProperties("Processes");

	if (fnames.empty())
		return false;

	const int numProc = fnames.size();
	PROCESS_INFORMATION* procInfo = new PROCESS_INFORMATION[numProc];
	HANDLE* handles = new HANDLE[fnames.size()];
	int procIdx = 0;

	for (auto it = fnames.begin(); it != fnames.end(); ++it, ++procIdx)
	{
		StreamMsg("Running process with param file: " << *it);

		it->append(" -e");

		CreateProcess(appName, it->c_str(), &procInfo[procIdx]);

		handles[procIdx] = procInfo[procIdx].hProcess;
	}

	char progress[4] = {'/', '-', '\\', '|'};
	
	std::cout << "\n";

	for (int progressIdx = 0; ; progressIdx++)
	{
		if (progressIdx >= 4)
			progressIdx = 0;

		std::cout << progress[progressIdx] << "\r" << std::flush;

		DWORD state = WaitForMultipleObjects(numProc, handles, TRUE, 2000); //INFINITE

		if (state != WAIT_TIMEOUT)
			break;
	}

	// Close process and thread handles
	for (procIdx = 0; procIdx < numProc; ++procIdx)
	{
		CloseHandle(procInfo[procIdx].hProcess);
		CloseHandle(procInfo[procIdx].hThread);
	}

	delete[] handles;
	delete[] procInfo;

	return true;
}


#ifdef WIN32

/*!
	Caller must call "delete[] szCmds" once process is done.
*/
bool CreateProcess(const char* szAppName, const char* szArgs, 
	PROCESS_INFORMATION* pProcInfo)
{
	STARTUPINFOA si;

	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);

	ZeroMemory(pProcInfo, sizeof(PROCESS_INFORMATION));

	// We need a non-const string with the arguments and also
	// to add the app name as the first parameter surrounded by quotes
	// The number of extra chars to add is 4: 2 quotes, one space, one null char
	char* szCmds = new char[strlen(szAppName) + strlen(szArgs) + 4];

	sprintf(szCmds, "\"%s\" %s", szAppName, szArgs);

	//std::cerr << "Creating process with params: " << szCmds << std::endl;

	BOOL hasProc = CreateProcessA(szAppName, szCmds, 
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		FALSE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		pProcInfo       // Pointer to PROCESS_INFORMATION structure
	);

	// Delete the temporary command line args
	delete[] szCmds;

	return (hasProc == TRUE);
}


/*!
	Creates a new process and waits for it to finish. 

	Note: the arguments 'szArgs' should not include the app name
	as its first argument. It will be added by this function.

	Unicode vs ASCII issues. 
	See: msdn.microsoft.com/en-us/library/cc500362.aspx
	
	const char* c = "Hello";
	LPCWSTR str = TEXT("Hello");
	LPCWSTR str1 = L"Hello";
	LPCWSTR str2 = _T("Hello");
	WCHAR    str3[6];
	MultiByteToWideChar( 0,0, c, 5, str3, 6);
	LPCWSTR cstr4 = str3;
*/
bool CreateProcessAndWait(const char* szAppName, const char* szArgs)
{
	PROCESS_INFORMATION pi;

	if (!CreateProcess(szAppName, szArgs, &pi))
		return false;

	// Wait until child process exits
	WaitForSingleObject(pi.hProcess, INFINITE);

	DWORD dwExitCode;

	// Not really used right now
	GetExitCodeProcess(pi.hProcess, &dwExitCode);

	// Close process and thread handles 
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	return true;
}
#else

#endif

void ReadPreferences()
{
	Fl_Preferences appPref(Fl_Preferences::USER, 
		"dmac@cs.toronto.edu", "VideoParser");

	int x, y;

	appPref.get( "x_wnd_pos", x, 20);
	appPref.get( "y_wnd_pos", y, 30);

	//DBG_PRINT2(x, y)
}

void WritePreferences()
{
	Fl_Preferences appPref(Fl_Preferences::USER, 
		"dmac@cs.toronto.edu", "VideoParser");

	double x = 0, y = 0;

	appPref.set( "x_wnd_pos", x, 20);
	appPref.set( "y_wnd_pos", y, 30);

	//DBG_PRINT2(x, y)
}

/*
#include "icon.xbm"

fl_open_display(); // needed if display has not been previously opened

Pixmap p = XCreateBitmapFromData(fl_display, DefaultRootWindow(fl_display),
                                 icon_bits, icon_width, icon_height);

window->icon((char *)p);

*/

/*
// Before exiting program, deal with VXL video bug
#ifdef HAS_VXL_VIDEO	
	int retVal = Fl::run();

	// BEGIN BUG HANDLING
	// Deal with bug in VXL (the destruction of a static codec fails)

	// Get a pointer to the static five-element array of smart ptrs to codecs
	// declared in core\vidl\vidl_codec.cxx
	vidl_codec_sptr* codecs = vidl_codec::all_codecs();

	// There are five codecs, but only the mpegcodec in position 1 causes problems
	for (int i = 1; i < 2; i++) 
	{
		codecs[i]->close();

		// change the protected state of the pointer to unprotected
		// by changing the value of its first member variable
		*((bool*)&codecs[i]) = false;
	}
	// END BUG HANDLING
#endif // HAS_VXL_VIDEO*/