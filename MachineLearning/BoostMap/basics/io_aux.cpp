#include <stdio.h>
#include <assert.h>
#include <vector>

#include "vplatform.h"
#include "basics/io_aux.h"

#include "basics/definitions.h"

using namespace std;

#ifndef VASSILIS_USE_vArray

//*************************************************************
//
// The following definitions are used when we want vArray to mean
// simply pointer.
//
//*************************************************************

#define vArray(type) type *
#define vArray2(type) type **
#define vArray3(type) type ***
#define vArray4(type) type ****
#define vArray5(type) type *****

#define class_pointer(type) type *
#define matrix_pointer(type) type **

#define vnew(type, size) (new type[size])
#define vData(array) (array)
#define actual_pointer(array) (array)
#define vToArray(size, data) (data)

#define vdelete2(array) delete [] (array);
#define delete_pointer(array) delete [] (array);
#define vZero(type) ((type *) 0)
#define vZero(type) ((type *) 0)
#define function_zero(type) ((type *) 0)

#endif // VASSILIS_USE_vArray


#ifdef VASSILIS_WINDOWS_MFC

vConsoleStream * pc_console_stream;
HWND application_window;
long application_window_initialized = 0;

const long max_input_length = 1000;

static HWND S_GetConsoleWindow();


void specify_application_window(void * pointer)
{
  if (pointer == 0)
  {
    return;
  }
  application_window = * (HWND*) pointer;
  application_window_initialized = 1;
}


void vInitializeConsole()
{
  static ushort already_called = 0;
  if (already_called != 0) return;
  // For a multithreaded application, this should be a critical section;
  already_called = 1;
  vConsoleStream::Initialize();
}


void vConsoleStream::Initialize()
{
  // We can't call new, because maybe our overloaded new is trying
  // to initialize the console, and that would create an infinite loop
  pc_console_stream = (vConsoleStream *) malloc(sizeof(vConsoleStream));
  AllocConsole();
  pc_console_stream->max_length = max_input_length + 3;
  pc_console_stream->buffer = (char *) malloc(pc_console_stream->max_length);
  pc_console_stream->buffer[0] = 0;
  pc_console_stream->index = 0;
  pc_console_stream->current_length = 0;
  pc_console_stream->stdin_handle = GetStdHandle(STD_INPUT_HANDLE);

  pc_console_stream->console_window = S_GetConsoleWindow();
}

vConsoleStream::vConsoleStream()
{
  assert(0);
}


vConsoleStream::~vConsoleStream()
{
  free(buffer);
  CloseHandle(stdin_handle);
}


long vConsoleStream::ConsoleToForeground()
{
  if (GetForegroundWindow() != console_window)
  {
    main_window = GetForegroundWindow();
    SetForegroundWindow(console_window);
  }

  return 1;
}


long vConsoleStream::MainWindowToForeground()
{
  if (application_window_initialized != 0)
  {
    SetForegroundWindow(application_window);
    return 1;
  }

  SetForegroundWindow(main_window);
  return 1;
}

  
void vConsoleStream::getline(char * result, vint8 length)
{
  if (index == current_length) GetLine();
  if (result == 0) return;
  vint8 actual_length;
  if (length < (vint8) (current_length - index))
  {
    actual_length = length;
  }
  else
  {
    actual_length = current_length - index;
  }
  memcpy(result, &(buffer[index]), (vector_size)actual_length);
  index += (long)actual_length;
  result[actual_length] = 0;
}


void vConsoleStream::GetLine()
{
  do
  {
    BOOL success = ReadConsole(stdin_handle, buffer, max_length, &current_length, 0);
    assert(success != 0);
    if (current_length == max_length)
    {
      vprint("Input line can't exceed %li characters, please renter.\n",
              max_length - 3);
      Flush();
    }
  }
  while(current_length == max_length);

  buffer[current_length] = 0;
  index = 0;
}


long vConsoleStream::LineLength()
{
  return max_length;
}


// This function flushes out the remainder of the current input line
void vConsoleStream::Flush()
{
  FlushConsoleInputBuffer(stdin_handle);
  current_length = 0;
  index = 0;
}


// This function is provided for compatibility with istream.flush
void vConsoleStream::flush()
{
  Flush();
}


int vConsoleStream::TryToReadItem(char * format, void * item)
{
  int bytes_read;
  int items_matched;
  assert(strlen(format) < 20);
  static char new_format[25];
  sprintf(new_format, "%s%%n", format);

  if (current_length == 0) GetLine();
  items_matched = sscanf(&(buffer[index]), new_format, item, &bytes_read);
  if (items_matched > 0)
  {
   index = index + bytes_read;
   assert(index <= (long) current_length);
  }
  return items_matched;
}


void vConsoleStream::ReadItem(char * format, void * item)
{
  int bytes_read;
  int items_matched;
  assert(strlen(format) < 20);
  static char new_format[25];
  sprintf(new_format, "%s%%n", format);
  do
  {
     items_matched = sscanf(&(buffer[index]), new_format, item, &bytes_read);
     if (items_matched <= 0) GetLine();
     else
     {
       index = index + bytes_read;
       assert(index <= (long) current_length);
     }
  }
  while(items_matched != 1);
}


// Input operator definitions
vConsoleStream & vConsoleStream::operator >> (char & number)
{
  ReadItem("%c", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (short & number)
{
  ReadItem("%hi", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (int & number)
{
  ReadItem("%i", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (long & number)
{
  ReadItem("%li", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (float & number)
{
  ReadItem("%f", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (double & number)
{
  ReadItem("%lf", &number);
  return *this;
}



vConsoleStream & vConsoleStream::operator >> (uchar & number)
{
  ReadItem("%c", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (ushort & number)
{
  ReadItem("%hu", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (uint & number)
{
  ReadItem("%u", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (ulong & number)
{
  ReadItem("%lu", &number);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (char * str)
{
  ReadItem("%s", str);
  return *this;
}


vConsoleStream & vConsoleStream::operator >> (string & str)
{
  char input_array[max_input_length+3];
  ReadItem("%s", input_array);
  str = input_array;

  return *this;
}


// Output operator definitions
vConsoleStream & vConsoleStream::operator << (char number)
{
  vprint("%c", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (short number)
{
  vprint("%hi", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (int number)
{
  vprint("%i", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (long number)
{
  vprint("%li", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (float number)
{
  vprint("%f", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (double number)
{
  vprint("%lf", number);
  return *this;
}



vConsoleStream & vConsoleStream::operator << (uchar number)
{
  vprint("%c", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (ushort number)
{
  vprint("%hu", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (uint number)
{
  vprint("%u", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (ulong number)
{
  vprint("%lu", number);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (char * str)
{
  vprint("%s", str);
  return *this;
}


vConsoleStream & vConsoleStream::operator << (string & str)
{
  if (str.size() == 0) return *this;
  vprint("%s", str.c_str());
  return *this;
}


vConsoleStream & vConsoleStream::operator << (void * pointer)
{
  vprint("%lx", pointer);
  return *this;
}


int vprint(const char * format, ...)
{
  vInitializeConsole();
  static const long buffer_length = max_input_length;
  long max_length = 1000000;
  static char buffer[buffer_length];
  va_list arguments;
  va_start(arguments, format);
  int result = _vsnprintf(buffer, buffer_length, format, arguments);
  va_end(arguments);
  char * working_buffer = buffer;

  if (result < 0)
  {
    class_pointer (char) new_buffer = vnew(char, max_length);
    va_start(arguments, format);
    result = _vsnprintf(actual_pointer (new_buffer), max_length, format, arguments);
    va_end(arguments);
    working_buffer = actual_pointer(new_buffer);
  }
    
  int length;
  if (result >= 0) length = result;
  else length = max_length - 1;
  static HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  assert(out_handle != INVALID_HANDLE_VALUE);
  unsigned long junk;
  WriteConsole(out_handle, working_buffer, length, &junk, 0);
  if (result < 0)
  {
    static char * error_message = "\nvprint error: output string was truncated.\n";
    const long error_length = (long) strlen(error_message);
    assert(error_length < max_length - 1);
    vprint(error_message);
  }
  if (working_buffer != buffer)
  {
    delete_pointer (working_buffer);
  }
  return length;
}


int vvprint(const char * format, va_list arguments)
{
  vInitializeConsole();
  static const long buffer_length = max_input_length;
  long max_length = 1000000;
  static char buffer[buffer_length];
  int result = _vsnprintf(buffer, buffer_length, format, arguments);
  char * working_buffer = buffer;

  if (result < 0)
  {
    class_pointer (char) new_buffer = vnew(char, max_length);
    va_start(arguments, format);
    result = _vsnprintf(actual_pointer (new_buffer), max_length, format, arguments);
    va_end(arguments);
    working_buffer = actual_pointer (new_buffer);
  }
    
  int length;
  if (result >= 0) length = result;
  else length = max_length - 1;
  static HANDLE out_handle = GetStdHandle(STD_OUTPUT_HANDLE);
  assert(out_handle != INVALID_HANDLE_VALUE);
  unsigned long junk;
  WriteConsole(out_handle, working_buffer, length, &junk, 0);
  if (result < 0)
  {
    static char * error_message = "\nvprint error: output string was truncated.\n";
    const long error_length = (long) strlen(error_message);
    assert(error_length < max_length - 1);
    vprint(error_message);
  }
  if (working_buffer != buffer)
  {
    delete [] working_buffer;
  }
  return length;
}


// This version is fine for standard consoles (UNIX and DOS applications)
/*
int vscan(char * format, ...)
{
  long length = strlen(format);
  long fields = 0;
  long i;
  vector<long> field_indices;

  // Count how many arguments we expect, and where each of them is.
  for (i = 0; i < length; i++)
  {
    if (format[i] == '%') 
    {
      fields++;
      field_indices.push_back(i);
    }
  }
  field_indices.push_back(length);

  char * temp_format = new char[length+1];
  i = 0;

  va_list arguments;
  va_start(arguments, format);
  
  while (i < fields)
  {
    int items_matched;
    long start = field_indices[i];
    long end = field_indices[i+1];
    long temp_length = end - start;
    memcpy(temp_format, &(format[start]), temp_length);
    temp_format[temp_length] = 0;
    void * argument = va_arg(arguments, void*);
    do
    {
      items_matched = scanf(temp_format, argument);
      if (items_matched != 1) 
      {
        vprint("%s: ", &format[start]);;
        fflush(stdin);
      }
    }
    while(items_matched != 1);

    do
    {
    //**  items_matched = console.TryToReadItem(temp_format, argument);
      if (items_matched != 1) 
      {
        vprint("%s: ", &format[start]);
    //    console.GetLine();
      }
    }
    while(items_matched != 1);

    i++;
  }

  va_end(arguments);
  return fields;
}
*/


// This version works with MFC applications that call AllocConsole
int vscan(const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  int result = vvscan(format, arguments);
  va_end(arguments);
  return result;
}


int vvscan(const char * format, va_list arguments)
{
  vInitializeConsole();
  static vConsoleStream & console = *pc_console_stream;
  long length = (long) strlen(format);
  long fields = 0;
  long i;
  vector<long> field_indices;

  // Count how many arguments we expect, and where each of them is.
  for (i = 0; i < length; i++)
  {
    if (format[i] == '%') 
    {
      fields++;
      field_indices.push_back(i);
    }
  }
  field_indices.push_back(length);

  class_pointer (char) temp_format = vnew(char, length+1);
  i = 0;

  while (i < fields)
  {
    int items_matched;
    long start = field_indices[i];
    long end = field_indices[i+1];
    long temp_length = end - start;
    memcpy(actual_pointer (temp_format), &(format[start]), temp_length);
    temp_format[temp_length] = 0;
    void * argument = va_arg(arguments, void*);
    do
    {
      items_matched = console.TryToReadItem(actual_pointer (temp_format), argument);
      if (items_matched != 1) 
      {
        vprint("%s: ", &format[start]);
        console.GetLine();
      }
    }
    while(items_matched != 1);
    i++;
  }
  delete_pointer(temp_format);
  return fields;
}


vint8 vMFCGetLine(char * buffer, vint8 max_length)
{
  vint8 result;
  pc_console_stream->getline(buffer, max_length);
  result =  strlen(buffer);
  return result;
}



HWND S_GetConsoleWindow(void) 
{ 
  const long MY_BUFSIZE = 1024;  // buffer size for console window titles
  HWND hwndFound;                // this is what is returned to the caller
  char pszNewWindowTitle[MY_BUFSIZE]; // contains fabricated WindowTitle
  char pszOldWindowTitle[MY_BUFSIZE]; // contains original WindowTitle

  // fetch current window title
  GetConsoleTitle(pszOldWindowTitle, MY_BUFSIZE);

  // format a "unique" NewWindowTitle

  wsprintf(pszNewWindowTitle,"%d/%d",
              GetTickCount(),
              GetCurrentProcessId());

  // change current window title

  SetConsoleTitle(pszNewWindowTitle);

  // ensure window title has been updated

  Sleep(40);

  // look for NewWindowTitle

  hwndFound=FindWindow(NULL, pszNewWindowTitle);

  // restore original window title

  SetConsoleTitle(pszOldWindowTitle);

  return(hwndFound);
} 


long vConsoleToForeground()
{
  vInitializeConsole();
  pc_console_stream->ConsoleToForeground();
  return 1;
}


long vMainWindowToForeground()
{
  vInitializeConsole();
  pc_console_stream->MainWindowToForeground();
  return 1;
}



#else // if not defined VASSILIS_WINDOWS_MFC

long vConsoleToForeground()
{
  return 1;
}


long vMainWindowToForeground()
{
  return 1;
}

#endif // VASSILIS_WINDOWS_MFC
