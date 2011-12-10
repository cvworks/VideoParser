#ifndef VASSILIS_IO_AUX_H
#define VASSILIS_IO_AUX_H
#include <cstdarg>

#include <string>

#include "v_types.h"
#include "vplatform.h"


class vConsoleStream
{
private:
  long max_length;
  char * buffer;
  long index;
  unsigned long current_length;
#ifdef VASSILIS_WINDOWS_MFC
  HANDLE stdin_handle;
  HWND console_window;
  HWND main_window;
#endif // VASSILIS_WINDOWS_MFC

public:
  vConsoleStream();
  ~vConsoleStream();
  
  long ConsoleToForeground();
  long MainWindowToForeground();

  void GetLine();
  void getline(char * buffer, vint8 length);
  long LineLength();
  void Flush();
  void flush();
  void ReadItem(char * format, void * item);
  int TryToReadItem(char * format, void * item);

  // Input operator declarations
  vConsoleStream & operator >> (char & number);
  vConsoleStream & operator >> (short & number);
  vConsoleStream & operator >> (int & number);
  vConsoleStream & operator >> (long & number);
  vConsoleStream & operator >> (float & number);
  vConsoleStream & operator >> (double & number);

  vConsoleStream & operator >> (uchar & number);
  vConsoleStream & operator >> (ushort & number);
  vConsoleStream & operator >> (uint & number);
  vConsoleStream & operator >> (ulong & number);
  
  vConsoleStream & operator >> (char * input);
  vConsoleStream & operator >> (std::string &nput);

  // Output operator declarations
  vConsoleStream & operator << (char number);
  vConsoleStream & operator << (short number);
  vConsoleStream & operator << (int number);
  vConsoleStream & operator << (long number);
  vConsoleStream & operator << (float number);
  vConsoleStream & operator << (double number);

  vConsoleStream & operator << (uchar number);
  vConsoleStream & operator << (ushort number);
  vConsoleStream & operator << (uint number);
  vConsoleStream & operator << (ulong number);
  
  vConsoleStream & operator << (char * str);
  vConsoleStream & operator << (std::string & str);
  vConsoleStream & operator << (void * pointer);

  static void Initialize();
};


int vprint(const char * format, ...);
int vvprint(const char * format, va_list ap);
int vscan(const char * format, ...);
int vvscan(const char * format, va_list ap);
void vInitializeConsole();
vint8 vMFCGetLine(char * buffer, vint8 max_length); //alex & v change
long vConsoleToForeground();
long vMainWindowToForeground();
void specify_application_window(void * pointer);


#endif // VASSILIS_IO_AUX_H
