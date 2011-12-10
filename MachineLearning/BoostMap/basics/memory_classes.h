#ifndef VASSILIS_MEMORY_CLASSES_H
#define VASSILIS_MEMORY_CLASSES_H

#include "vplatform.h"
#include "v_types.h"
#include <list>
#include <vector>

using namespace std;

class S_ObjectList : public std::list<vint8>
{
public:
  S_ObjectList()
  {}
  ~S_ObjectList()
  {}
};


class S_MemoryManager
{
public:
  // These vectors store information that lets new and delete keep
  // track of which pointers have been allocated so far.
  vector<void *> id_to_pointer;
  vector<vint8> available_ids;
  vector<vint8> id_use_counter;
  vector<void *> object_list_positions;
  S_ObjectList object_list;
  vector<voidp> last_new_pointers;

  vuint4 id_counter;

  // These flags are set to 0 when the program starts, and each of
  // them is set to 1 the first time an error condition has occurred.
  // At the end of the program we print out information about all
  // the error conditions.
  ushort leaks_flag;
  ushort wrong_delete_operator_flag;
  ushort id_overflow_flag;
  ushort id_counter_overflow_flag;
  ushort bad_delete_argument_flag;

  list<void*> vdelete_stack;
  list<void*> vdelete2_stack;
public:
  S_MemoryManager();
  ~S_MemoryManager();
  void RegisterMemory(vint8 * pointer);
  ushort UnregisterMemory(vint8 * pointer);
  void IncrementErrors();

  static ushort show_statistics;
};


// The sole reason this class exists is so that its destructor deletes
// the memory manager object. This way, by creating a static global variable
// of that type in this file, we make sure that, at the end of the program,
// as the variable gets deleted, it will call the destructor of 
// the memory manager, which will detect memory leaks.
class S_ManagerDeleter
{
public:
  S_ManagerDeleter();
  ~S_ManagerDeleter();
};





#endif // VASSILIS_MEMORY_CLASSES_H
