
#include <time.h>

#include "base_interpreter.h"
#include "basics/auxiliaries.h"
#include "basics/local_data.h"
#include "algebra.h"
#include "precomputed.h"
#include "simple_algo.h"
#include "io_aux.h"

#include "basics/definitions.h"


const char * platform_face_detector_path = platform_face_detector_path1;
const char * platform_skin_directory = platform_skin_directory1;
const char * platform_non_skin_directory = platform_non_skin_directory1;
const char * crl_skin_file = crl_skin_file1;
const char * crl_non_skin_file = crl_non_skin_file1;
const char * sequences_directory = sequences_directory1;

//char * g_code_directory = vCopyString(g_default_code_directory);
//char * g_data_directory = vCopyString(g_default_data_directory);
//char * nessie = vCopyString(nessie1);
//char * nessied = vCopyString(nessied1);


char * g_code_directory = 0;
const char * g_data_directory = 0;
char * nessie = 0;
char * nessied = 0;


class_interpreter::class_interpreter()
{
  initialize ();
}


class_interpreter::~class_interpreter()
{
  clean_up();
}


vint8 class_interpreter::initialize_directories()
{
  if (g_code_directory == 0)
  {
    g_code_directory = vCopyString(g_default_code_directory);
  }
  if (g_data_directory == 0)
  {
    g_data_directory = vCopyString(g_default_data_directory);
  }
  if (nessie == 0)
  {
    nessie = vCopyString(nessie1);
  }
  if (nessied == 0)
  {
    nessied = vCopyString(nessied1);
  }

  return 1;
}


vint8 class_interpreter::delete_directories()
{
  vdelete2(g_code_directory);
  vdelete2(g_data_directory);
  vdelete2(nessie);
  vdelete2(nessied);
  
  g_code_directory = 0;
  g_data_directory = 0;
  nessie = 0;
  nessied = 0;

  return 1;
}


vint8 class_interpreter::initialize()
{
  initialize_directories();
  input_file_pointer = 0;

//  current_action_mode = vCopyString(graphical_module::mode_pixel_info_response);
//  previous_action_mode = 0;

  exit_request = 0;

  // The goal here is to instantiate some template functions, so
  // that I can call them while debugging.
  vMatrix<uchar> junk1(1,1);
  vMatrix<vint4> junk2(1,1);
  vMatrix<float> junk3(1,1);
  vMatrix<double> junk4(1,1);
  vMatrix<vint8> junk5(1,1);
  vint8 a = 0;
  if (a)
  {
    junk1.Print(0);
    junk1.PrintRange(1,1,1,1,0);
    junk1.PrintInt(0);
    junk1.PrintRangeInt(1,1,1,1,0);
    junk1.PrintRangem(1,1,1,1,0);
    junk1.PrintRangeIntm(1,1,1,1,0);

    junk1.PrintTrans(0);
    junk1.PrintRangeTrans(1,1,1,1,0);
    junk1.PrintIntTrans(0);
    junk1.PrintRangeIntTrans(1,1,1,1,0);
    junk1.PrintRangemTrans(1,1,1,1,0);
    junk1.PrintRangeIntmTrans(1,1,1,1,0);
    junk1.WriteDebug("trash");

    junk2.Print(0);
    junk2.PrintRange(1,1,1,1,0);
    junk2.PrintInt(0);
    junk2.PrintRangeInt(1,1,1,1,0);
    junk2.PrintRangem(1,1,1,1,0);
    junk2.PrintRangeIntm(1,1,1,1,0);

    junk2.PrintTrans(0);
    junk2.PrintRangeTrans(1,1,1,1,0);
    junk2.PrintIntTrans(0);
    junk2.PrintRangeIntTrans(1,1,1,1,0);
    junk2.PrintRangemTrans(1,1,1,1,0);
    junk2.PrintRangeIntmTrans(1,1,1,1,0);
    junk2.WriteDebug("trash");

    junk3.Print(0);
    junk3.PrintRange(1,1,1,1,0);
    junk3.PrintInt(0);
    junk3.PrintRangeInt(1,1,1,1,0);
    junk3.PrintRangem(1,1,1,1,0);
    junk3.PrintRangeIntm(1,1,1,1,0);

    junk3.PrintTrans(0);
    junk3.PrintRangeTrans(1,1,1,1,0);
    junk3.PrintIntTrans(0);
    junk3.PrintRangeIntTrans(1,1,1,1,0);
    junk3.PrintRangemTrans(1,1,1,1,0);
    junk3.PrintRangeIntmTrans(1,1,1,1,0);
    junk3.WriteDebug("trash");

    junk4.Print(0);
    junk4.PrintRange(1,1,1,1,0);
    junk4.PrintInt(0);
    junk4.PrintRangeInt(1,1,1,1,0);
    junk4.PrintRangem(1,1,1,1,0);
    junk4.PrintRangeIntm(1,1,1,1,0);

    junk4.PrintTrans(0);
    junk4.PrintRangeTrans(1,1,1,1,0);
    junk4.PrintIntTrans(0);
    junk4.PrintRangeIntTrans(1,1,1,1,0);
    junk4.PrintRangemTrans(1,1,1,1,0);
    junk4.PrintRangeIntmTrans(1,1,1,1,0);
    junk4.WriteDebug("trash");

    junk5.Print(0);
    junk5.PrintRange(1,1,1,1,0);
    junk5.PrintInt(0);
    junk5.PrintRangeInt(1,1,1,1,0);
    junk5.PrintRangem(1,1,1,1,0);
    junk5.PrintRangeIntm(1,1,1,1,0);

    junk5.PrintTrans(0);
    junk5.PrintRangeTrans(1,1,1,1,0);
    junk5.PrintIntTrans(0);
    junk5.PrintRangeIntTrans(1,1,1,1,0);
    junk5.PrintRangemTrans(1,1,1,1,0);
    junk5.PrintRangeIntmTrans(1,1,1,1,0);
    junk5.WriteDebug("trash");
  }

  input_file_pointer = 0;
  
  return 1;
}


// An interesting thing with the CleanUp functions of each interpreter
// (i.e. interpreter1, 2, 3), is that each of them gets called by
// the destructor of that class. I don't understand why that happens.
// I would expect that, since the CleanUp functions are all virtual,
// only the CleanUp function of the lowest subclass (interpreter3
// currently) should be called. I don't know if the way things work 
// now is according to the C++ standard or if it is a Virtual Studio bug.
vint8 class_interpreter::clean_up()
{
  vEraseCharpVector(&module_names);
  vDeletePrecomputedValues();

  if (input_file_pointer != 0)
  {
    fclose(input_file_pointer);
  }

  delete_directories();  
  return 1;
}


vint8 class_interpreter::vScan(const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result;
  if (input_file_pointer == 0)
  {
    vConsoleToForeground ();
    result = vVScan(format, arguments);
  }
  else
  {
    result = vFscan(input_file_pointer, format, arguments);
  }
  va_end(arguments);
  return result;
}


// speech-recognition-friendly version of vprint
vint8 class_interpreter::print (const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result = vVprint(format, arguments);
  va_end(arguments);
  return result;
}


// speech-recognition-friendly version of vScan
vint8 class_interpreter::question (const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result;
  if (input_file_pointer == 0)
  {
    vConsoleToForeground ();
    result = vVScan(format, arguments);
  }
  else
  {
    result = vFscan(input_file_pointer, format, arguments);
  }
  va_end(arguments);
  return result;
}

//vint8 class_interpreter::default_current_action_mode()
//{
//	set_current_action_mode(graphical_module::mode_pixel_info_response);
//	return 1;
//}
//
//vint8 class_interpreter::set_current_action_mode(const char* mode)
//{
//	vdelete2(current_action_mode);
//	current_action_mode = vCopyString(mode);
//	return 1;
//}
//
//vint8 class_interpreter::set_previous_action_mode(const char* mode)
//{
//	vdelete2(previous_action_mode);
//	previous_action_mode = vCopyString(mode);
//	return 1;
//}
//
//vint8 class_interpreter::is_current_action_mode(const char* mode)
//{
//	if (function_compare_strings( current_action_mode, mode ) == 0)
//	{ return 1; }
//	else 
//	{ return 0;}
//}

ushort class_interpreter::process_key(char command)
{
  vint8 number = modules.size();
  vint8 success = 0;
  vint8 i;
  vint8 success_all = 0;
//  vint8 ret = 0;
  vint8 success_temp = 0;

  for (i = 0; i < number; i++)
  {
    success_temp = modules[(vector_size) i]->process_key(command);
    if (success_temp >= 1)
    {
      success_all += 1;
	 // if ()
    }
  }

  if (success_all > 1)
  {
	  function_warning("command %c found in multiple modules", command);
  }

  // we only get here if success is 0.
 // vPrint("??? %c with action mode %s is not a recognized command\n", command, current_action_mode);
  
  return 0;

}

ushort class_interpreter::process_mouse(vint8 button_down, vint8 row, vint8 col)
{
  vint8 number = modules.size();
  vint8 success = 0;
  vint8 i;
  vint8 success_all = 0;

  for (i = 0; i < number; i++)
  {
    vint8 success_temp = modules[(vector_size) i]->process_mouse( button_down, row, col);
    if (success_temp >= 1)
    {
      success_all += 1;
    }
  }

  if (success_all > 1)
  {
	  function_warning("mouse click(%li, %li, %li)  found in multiple modules", (long)button_down, (long)row, (long)col);
  }

  // we only get here if success is 0.
 // vPrint("??? mouse click(%li, %li, %li) with mode %s is not recognized", (long)button_down, (long)row, (long)col, current_action_mode);
  
  return 1;

}

vint8 class_interpreter::process_command(const char * command)
{
  if (command == 0) return 0;

  if (command[0] == 0) return 0;
  
  if (strcmp(command, "file") == 0)
  {
    process_command_file();
    return 1;
  }

  if (strcmp(command, "system") == 0)
  {
    char system_command[1000];
    vGetNonEmptyLine(system_command, 990);
    system(system_command);
    return 1;
  }

  if (strcmp(command, "x") == 0)
  {
    // 2 signifies we should exit.
    vMainWindowToForeground();
    return 2;
  }

  vint8 number = modules.size();
  vint8 success = 0;
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 success = modules[(vector_size) i]->process_command(command);
    if (success == 1)
    {
      return 1;
    }
  }

  // we only get here if success is 0.
  vPrint("??? %s is not a recognized command\n", command);
  
  return 1;
}


// This command allows us to process text commands that are
// stored in a file.
vint8 class_interpreter::process_command_file()
{
  char buffer[500];
  vPrint("Input name of command file: ");
  vScan("%s", buffer);
  return process_command_file1(buffer);
}


vint8 class_interpreter::process_command_file1(const char * simple_file_name)
{
  // We require that the file is in the directory g_code_directory/commands 
  // and we add the ".txt" extension to it.
  char * temp = vJoinPaths(g_code_directory, "commands");
  char * temp2;
  if (vStringEndsIn(simple_file_name, ".txt"))
  {
    temp2 = vCopyString(simple_file_name);
  }
  else
  {
    temp2 = vMergeStrings2(simple_file_name, ".txt");
  }
  char * file = vJoinPaths(temp, temp2);
  vdelete2(temp);
  vdelete2(temp2);

  vint8 result = process_command_file2(file);
  delete_pointer(file);
  return result;
}


vint8 class_interpreter::process_command_file2(const char * pathname)
{
  // Possibly we are already executing commands from another
  // file and one of those asks us to execute this file. Then,
  // we must save the previous FILE * object, process the 
  // new file, and then restore the previous file.
  FILE * previous_input_file_pointer = 0;
  // If we are processing another command file, save the status
  // of that file
  if (input_file_pointer != 0) 
  {
    previous_input_file_pointer = input_file_pointer;
  }
  // Open the requested file, return failure if we can't open it.
  input_file_pointer = fopen(pathname, vFOPEN_READ);
  if (input_file_pointer == 0)
  {
    vPrint("Can't find input file %s\n", pathname);
    input_file_pointer = previous_input_file_pointer;
    return 0;
  }

  // Process commands from the file until we read all the contents
  // of the file. If there is nothing more to read, we are done
  while(1)
  {
    char command[100];
    vint8 items = vScan("%s", command);
    // If we reached the end of file (or a read error has occured)
    // we are done.
    if (items != 1) break;
    process_command(command);
  }
  fclose(input_file_pointer);
  // Restore the previous file (if any)
  input_file_pointer = previous_input_file_pointer;
  return 1;
}


vint8 class_interpreter::enter_module(class_module * module, char * name)
{
  modules.push_back (module);
  module_names.push_back (function_copy_string (name));
  return 1;
}


class_module * class_interpreter::get_module(char * name)
{
  class_module * result = 0;
  vint8 number = module_names.size ();
  vint8 counter;

  for (counter = 0; counter < number; counter ++)
  {
    if (strcmp (name, module_names[(vector_size) counter]) == 0)
    {
      result = modules[(vector_size) counter];
      break;
    }
  }

  return result;
}


vint8 class_interpreter::check_command_duplicates()
{
  vint8 module_number = modules.size ();
  vector <char*> commands;
  
  vint8 counter;
  for (counter = 0; counter <module_number; counter ++)
  {
    modules[(vector_size) counter]->get_commands(& commands);
  }

  sort (commands.begin (), commands.end (), CharpLess());
  vint8 number = commands.size ();
  vPrint ("%li commands found.\n", number);

  vint8 flag = 0;
  for (counter = 0; counter < number-1; counter ++)
  {
    if (function_compare_strings(commands[(vector_size) counter], commands [(vector_size) (counter + 1)]) == 0)
    {
      vPrint("duplicate command: %s\n", commands[(vector_size) counter]);
      flag = 1;
    }
  }

  if (flag == 0)
  {
    vPrint ("no duplicates were found\n");
  }

  return 1;
}

  
vint8 class_interpreter::list_commands()
{
  vint8 module_number = modules.size ();
  vector <char*> commands;
  
  vint8 counter;
  for (counter = 0; counter <module_number; counter ++)
  {
    modules[(vector_size) counter]->get_commands(& commands);
  }

  sort (commands.begin (), commands.end (), CharpLess());
  vint8 number = commands.size ();
  vPrint ("%li commands found.\n", number);

  for (counter = 0; counter < number-1; counter ++)
  {
    vPrint ("%s\n", commands[(vector_size) counter]);
  }

  return 1;
}

  
vint8 class_interpreter::main_loop(vint8 argc, char ** argv)
{
  // If there was a command-line argument, we need to just process a script file.
  if (argc > 1)
  {
    if (argc != 2)
    {
      exit_error("usage: %s [script_file]\n", argv[0]);
    }

    const char * script_file = argv[1];
    process_command_file1(script_file);
    return 0;
  }

  // If no arguments were passed, then we just start the interactive
  // loop.
  char command[1000];
  while(1)
  {
    vPrint(">> ");
    vScan("%s", command);
    vint8 return_code = process_command(command);
    
    // if the user decided to exit...
    if (return_code == 2) 
    {
      break;
    }
  }

  return 0;
}


class_module::class_module(class_interpreter * interpreter)
{
  initialize();
  base_interpreter = interpreter;
}


class_module::~class_module()
{
  clean_up();
  initialize();
}

vint8 class_module::initialize()
{
  base_interpreter = 0;
  
  return 1;
}

vint8 class_module::clean_up()
{
  vEraseCharpVector(&commands);
  
  return 1;
}


vint8 class_module::get_commands (std::vector<char*> * strings)
{
  strings->insert (strings->end (), commands.begin (), commands.end ());
  return 1;
}


vint8 class_module::command_index(const char * command)
{
  vint8 result = -1;
  vint8 number = commands.size();
  
  vint8 i;
  for (i = 0; i < number; i++)
  {
    if (strcmp(command, commands[(vector_size) i]) == 0)
    {
      result = i;
      break;
    }
  }
  
  return result;
}


vint8 class_module::vScan(const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result;
  if (base_interpreter->input_file_pointer == 0)
  {
    vConsoleToForeground ();
    result = vVScan(format, arguments);
  }
  else
  {
    result = vFscan(base_interpreter->input_file_pointer, format, arguments);
  }
  va_end(arguments);
  return result;
}


// speech-recognition-friendly version of vprint
vint8 class_module::print (const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result = vVprint(format, arguments);
  va_end(arguments);
  return result;
}


// speech-recognition-friendly version of vScan
vint8 class_module::question (const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  vint8 result;
  if (base_interpreter->input_file_pointer == 0)
  {
    vConsoleToForeground ();
    result = vVScan(format, arguments);
  }
  else
  {
    result = vFscan(base_interpreter->input_file_pointer, format, arguments);
  }
  va_end(arguments);
  return result;
}



//////////////////////////////////////////////////////////
//                                                      //
// start of obligatory member functions for any module  //
//                                                      //
//////////////////////////////////////////////////////////

class_base_module::class_base_module(class_interpreter * interpreter) :
  class_module(interpreter)
{
  initialize();
  sort_commands ();
}


class_base_module::~class_base_module()
{
  class_module::clean_up();
  clean_up();
}

vint8 class_base_module::initialize()
{
  enter_command("c_code_dir", &class_base_module::c_code_dir);
  enter_command("c_data_dir", &class_base_module::c_data_dir);

  enter_command("d_code_dir", &class_base_module::d_code_dir);
  enter_command("d_data_dir", &class_base_module::d_data_dir);

  enter_command("l_code_dir", &class_base_module::l_code_dir);
  enter_command("l_data_dir", &class_base_module::l_data_dir);
  enter_command("list_commands", &class_base_module::list_all_commands);

  enter_command("randomize", &class_base_module::randomize);
  enter_command("r_code_dir", &class_base_module::r_code_dir);
  enter_command("r_data_dir", &class_base_module::r_data_dir);

  enter_command("s_data_dir", &class_base_module::s_data_dir);
  enter_command("set_code_dir", &class_base_module::set_code_dir);
  enter_command("set_data_dir", &class_base_module::set_data_dir);
  enter_command("set_nessie", &class_base_module::set_nessie);
  enter_command("system", &class_base_module::system_function);
  

  return 1;
}

vint8 class_base_module::clean_up()
{
  vEraseCharpVector(&commands);
  
  return 1;
}


vint8 class_base_module::enter_command(char * command, type_procedure procedure)
{
  commands.push_back(function_copy_string (command));
  procedures.push_back(procedure);
  return 1;
}


vint8 class_base_module::process_command(const char * command)
{
  vint8 index = command_index (command);
  if (index < 0)
  {
    return 0;
  }
  type_procedure procedure = procedures[(vector_size) index];
  (this->*procedure)();

  return 1;
}


vint8 class_base_module::sort_commands()
{
  vector<vint8> indices;
  function_string_ranks (&commands, & indices);
  
  vector <char*> temporary;
  temporary.insert(temporary.end(), commands.begin (), commands.end ());
  vector<type_procedure> temporary_procedures;
  temporary_procedures.insert(temporary_procedures.end(), procedures.begin (), procedures.end ());

  vint8 number = commands.size ();
  vint8 counter;
  for (counter = 0; counter < number; counter ++)
  {
    vint8 index = indices[(vector_size) counter];
    commands[(vector_size) counter] = temporary[(vector_size) index];
    procedures[(vector_size) counter] = temporary_procedures[(vector_size) index];
  }

  return 1;
}


//////////////////////////////////////////////////////////
//                                                      //
//  end of obligatory member functions for any module   //
//                                                      //
//////////////////////////////////////////////////////////

vint8 class_base_module::randomize()
{
  srand((unsigned)time(NULL));
  return 1;
}



vint8 class_base_module::c_code_dir()
{
  vdelete2(g_code_directory);
  g_code_directory = vCopyString(TEST_DIR_C);
  return 1;
}


vint8 class_base_module::d_code_dir()
{
  vdelete2(g_code_directory);
  g_code_directory = vCopyString(TEST_DIR_D);
  return 1;
}


vint8 class_base_module::r_code_dir()
{
  vdelete2(g_code_directory);
  g_code_directory = vCopyString(TEST_DIR_R);
  return 1;
}


vint8 class_base_module::l_code_dir()
{
  vdelete2(g_code_directory);
  g_code_directory = vCopyString(TEST_DIR_L);
  return 1;
}


vint8 class_base_module::c_data_dir()
{
  vdelete2(g_data_directory);
  g_data_directory = vCopyString(DATA_DIR_C);
  return 1;
}


vint8 class_base_module::d_data_dir()
{
  vdelete2(g_data_directory);
  g_data_directory = vCopyString(DATA_DIR_D);
  return 1;
}

vint8 class_base_module::r_data_dir()
{
  vdelete2(g_data_directory);
  g_data_directory = vCopyString(DATA_DIR_R);
  return 1;
}


vint8 class_base_module::s_data_dir()
{
  vdelete2(g_data_directory);
  g_data_directory = vCopyString(DATA_DIR_S);
  return 1;
}


vint8 class_base_module::l_data_dir()
{
  vdelete2(g_data_directory);
  g_data_directory = vCopyString(DATA_DIR_L);
  return 1;
}


vint8 class_base_module::set_code_dir()
{
  char buffer[1000];
  vPrint("\nenter new directory:\n");
  vScan("%s", buffer);
  vdelete2(g_code_directory);
  g_code_directory = vCopyString(buffer);
  return 1;
}


vint8 class_base_module::set_data_dir()
{
	// Added by Diego
	if (!g_data_directory)
		std::cout << "There is no current data directory" << std::endl;
	else
		std::cout << "The current data directory is:\n\t" 
			<< g_data_directory << std::endl;

  char buffer[1000];
  vPrint("\nenter new directory:\n");
  vScan("%s", buffer);

  // Added by Diego
  if (strlen(buffer) <= 1)
  {
	  std::cout << "The data directory was not updated" << std::endl;
	  return 1;
  }
  else
  {
	  std::cout << "The data directory was changed to:\n\t" 
		<< g_data_directory << std::endl;
  }

  vdelete2(g_data_directory);
  g_data_directory = vCopyString(buffer);
  return 1;
}


vint8 class_base_module::set_nessie()
{
  char buffer[1000];
  vPrint("\nenter new directory:\n");
  vScan("%s", buffer);
  vdelete2(nessie);
  nessie = vCopyString(buffer);
  return 1;
}


vint8 class_base_module::system_function ()
{
  char system_command[1000];
  vGetNonEmptyLine(system_command, 990);
  system(system_command);

  return 1;
}


vint8 class_base_module::list_all_commands()
{
  base_interpreter->list_commands();
  return 1;
}
