#ifndef VASSILIS_BASE_INTERPRETER_H
#define VASSILIS_BASE_INTERPRETER_H

#include <stdio.h>
#include <vector>

#include "auxiliaries.h"

class class_module;


class class_mode{
public:
   vint8 is_valid;
   virtual ushort process_key(char command) = 0;
   virtual ushort process_mouse(vint8 button_down, vint8 row, vint8 col) = 0;  

   inline vint8 valid(){
	   return is_valid;
   }
};

//class base_mode{
//public:
//   vint8 is_valid;
//   virtual ushort process_key(char command) = 0;
//   virtual ushort process_mouse(vint8 button_down, vint8 row, vint8 col) = 0;  
//
//   inline vint8 valid(){
//	   return is_valid;
//   }
//};


class class_interpreter
{
private:
  std::vector<char *> module_names;
  std::vector<class_module *> modules;

public:
  FILE * input_file_pointer;
  
  // this will allow the key-pressed and mouse-click events
  // to be interpreted differently depending on the 
  // active action_mode.
  //char* current_action_mode;
  //char* previous_action_mode;

  vint8 exit_request;


public:
  class_interpreter();
  ~class_interpreter();

  vint8 initialize();
  vint8 clean_up();
  
  static vint8 initialize_directories();
  static vint8 delete_directories();


  //vint8 set_current_action_mode(const char* mode);
  //vint8 default_current_action_mode();
  //vint8 set_previous_action_mode(const char* mode);
  //// return non-zero if current_action_mode == mode
  //// and zero if different.
  //vint8 is_current_action_mode(const char* mode);

  ushort process_key(char command);
  ushort process_mouse(vint8 button_down, vint8 row, vint8 col);

  vint8 process_command(const char * command);
  vint8 process_command_file();
  vint8 process_command_file1(const char * filename); // simple filename
  vint8 process_command_file2(const char * filename); // complete pathname
  vint8 vScan(const char * format, ...);
  vint8 print (const char * format, ...);
  vint8 question (const char * format, ...);

  vint8 enter_module(class_module * module, char * name);
  class_module * get_module(char * name);
  vint8 check_command_duplicates();
  vint8 list_commands();
  vint8 main_loop(vint8 argc, char ** argv);
};


class class_module
{
protected:
  class_interpreter * base_interpreter;
  std::vector<char *> commands;
  typedef vint8 (class_module::*type_procedure) ();

protected:

  // the virtual functions should be set to 0, so that class_module is abstract.
  // however, I wanted to provide prototypes for these functions, that I can
  // copy, paste and modify when I define a subclass.
  virtual vint8 initialize();
  virtual vint8 clean_up();
  
public:
  class_module(class_interpreter * interpreter);
  virtual ~class_module();

  // alex
  virtual ushort process_key(char command) = 0;
  virtual ushort process_mouse(vint8 button_down, vint8 row, vint8 col) = 0;


  vint8 enter_command(char * command, type_procedure procedure);
  vint8 get_commands (std::vector<char*> * strings);
  virtual vint8 process_command(const char * command) = 0;

  vint8 command_index(const char * command);
  vint8 vScan(const char * format, ...);
  vint8 print (const char * format, ...);
  vint8 question (const char * format, ...);
};


class class_base_module : public class_module
{
protected:
  // the next two lines are mandatory for any module
  typedef vint8 (class_base_module::*type_procedure) ();
  std::vector<type_procedure> procedures;

public:

  //////////////////////////////////////////////////////////
  //                                                      //
  // start of obligatory member functions for any module  //
  //                                                      //
  //////////////////////////////////////////////////////////

  class_base_module(class_interpreter * interpreter);
  virtual ~class_base_module();
  
  virtual vint8 initialize();
  virtual vint8 clean_up();

  virtual ushort process_key(char command)  {  
	  return 0;  } // it is imp to return 0, to show that this module does not process the action
  virtual ushort process_mouse(vint8 button_down, vint8 row, vint8 col) {
	  return 0;} // it is imp to return 0, to show that this module does not process the action

  vint8 enter_command(char * command, type_procedure procedure);
  virtual vint8 process_command(const char * command);
  vint8 sort_commands ();

  //////////////////////////////////////////////////////////
  //                                                      //
  // end of obligatory member functions for any module  //
  //                                                      //
  //////////////////////////////////////////////////////////

  vint8 randomize();
  vint8 c_code_dir();
  vint8 d_code_dir();
  vint8 l_code_dir();
  vint8 r_code_dir();

  vint8 c_data_dir();
  vint8 d_data_dir();
  vint8 l_data_dir();
  vint8 r_data_dir();
  vint8 s_data_dir();

  vint8 set_code_dir();
  vint8 set_data_dir();
  vint8 set_nessie();
  vint8 system_function();
  vint8 list_all_commands();
};


#endif // VASSILIS_BASE_INTERPRETER_H
