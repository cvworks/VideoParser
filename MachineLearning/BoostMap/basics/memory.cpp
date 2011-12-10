
#include "vplatform.h"


#include "memory.h"

#include "memory_classes.h"


#include <assert.h>

#include <vector>
#include <list>


// Set WatchID and WatchUse to -1 so that they never check for valid objects.
static const vint8 WatchID = -1;
static const vint8 WatchUse = 3;
static const void * WatchPointer = (void *) 0;


#ifdef VASSILIS_WINDOWS_MFC
#include "io_aux.h"
#define cin vInitializeConsole(); (*pc_console_stream)
#define cout vInitializeConsole(); (*pc_console_stream) 
#define vPrint vprint
#define vScan vscan
extern vConsoleStream * pc_console_stream;
//#else
//#define vPrint printf
//#endif
#else
#include <stdio.h>
#include <cstdarg>
#define vPrint vPrintAndFlush
extern int vScanAndVerify(const char * format, ...);
#define vScan vScanAndVerify
static int vPrintAndFlush(const char * format, ...)
{
  va_list arguments;
  va_start(arguments, format);
  int result = vprintf(format, arguments);
  va_end(arguments);
  fflush(stdout);
 
  return result;
}
#endif // VASSILIS_WINDOWS_MFC

static void vWaitForEnter()
{
  cout << "Press <ENTER> to continue: ";
#ifdef VASSILIS_WINDOWS_MFC
  cin.flush();
  char buffer[20];
  cin.getline(buffer, 10);
#endif
  getchar();
  getchar();
}


#ifdef VASSILIS_MEMORY_CHECK

using namespace std;

void S_CheckMemoryManager();
static S_MemoryManager * manager;
static S_ManagerDeleter manager_deleter;
ushort S_MemoryManager::show_statistics = 1;

static const vint8 CREATED_BY_MY_CODE = 1;
static const vint8 CREATED_BY_OTHER_CODE = 2;
static const vint8 CREATED_BY_MEMORY_CPP = 3;
static const vint8 CREATED_AFTER_MANAGER_DESTRUCTION = 4;

static const ushort SIZE_INDEX = 0;
static const ushort CREATION_INDEX = sizeof(vint8) / sizeof(vint8);
static const ushort ID_INDEX = CREATION_INDEX + 1;
static const ushort EXTRA_WORDS = 2 + sizeof(vint8) / sizeof(vint8);
static const ushort EXTRA_BYTES = EXTRA_WORDS * sizeof(vint8);

static const vint8 CALL_FROM_MEMORY_CPP = 1;
static const vint8 CALL_AFTER_MANAGER_DESTRUCTION = 2;
static const vint8 NORMAL_CALL = 3;

// The following variables are only used so that at debug time I can
// check their values (the debugger won't show const values). They
// never get used in the actual code.
static vint8 call_from_memory_cpp = CALL_FROM_MEMORY_CPP;
static vint8 call_after_manager_destruction = CALL_AFTER_MANAGER_DESTRUCTION;
static vint8 normal_call = NORMAL_CALL;

static vint8 POINTER_ID_LIMIT = 2000000000; // 4 billion.
static vint8 CALL_INFORMATION = NORMAL_CALL;
static vint8 errors_counter;
static vas_int16 total_memory_allocated;
static vas_int16 total_memory_deleted;
static vas_int16 new_calls_counter;
static vas_int16 delete_calls_counter;
static ushort out_of_memory_flag;



void vDeclareAsMyCode(vint8 * pointer)
{
  if (pointer == 0)
  {
    printf("\nerror: 0 passed to vDeclareAsMyCode\n");
    exit(0);
  }

  if (CALL_INFORMATION == CALL_AFTER_MANAGER_DESTRUCTION) return;
  CALL_INFORMATION = CALL_FROM_MEMORY_CPP;
  vint8 * base = pointer - EXTRA_WORDS + 1;
  ushort success_flag = 0;
  while(success_flag == 0)
  {
    base--;
    vint8 pointer_id = base[ID_INDEX];

    if (manager->id_to_pointer.size() <= pointer_id) success_flag = 0;
    else if (manager->id_to_pointer[(vector_size) pointer_id] != base) success_flag = 0;
    else success_flag = 1;
  }

  if ((pointer != base + EXTRA_WORDS) &&
      (pointer != base + EXTRA_WORDS + 1))
  {
    // This should never happen.
    vPrint("Error, something is really wrong.\n");
  }
  base[CREATION_INDEX] = CREATED_BY_MY_CODE;
  CALL_INFORMATION = NORMAL_CALL;
}


S_ManagerDeleter::S_ManagerDeleter()
{
}


S_ManagerDeleter::~S_ManagerDeleter()
{
  S_MemoryManager * temp_copy = manager;
  manager = 0;
  CALL_INFORMATION = CALL_FROM_MEMORY_CPP;
  delete temp_copy;
  // The following line just forces the console to remain open,
  // so that I have time to look at all messages related 
  // to memory leaks and stuff like that.
  CALL_INFORMATION = CALL_AFTER_MANAGER_DESTRUCTION;
  vWaitForEnter();
}


S_MemoryManager::S_MemoryManager()
{
  vPrint("Creating memory manager\n");
  assert(sizeof(long) >= 4);
  assert(sizeof(void *) >= sizeof(long));
  leaks_flag = 0;
  wrong_delete_operator_flag = 0;
  out_of_memory_flag = 0;
  id_overflow_flag = 0;
  id_counter_overflow_flag = 0;
  bad_delete_argument_flag = 0;

  id_counter = 0;
  errors_counter = 0;
}


S_MemoryManager::~S_MemoryManager()
{
  vint8 number_of_objects = (vint8) object_list.size();
  if (number_of_objects != 0) leaks_flag = 1;
  list<vint8>::iterator iter = object_list.end();
  vas_int16 total_external_leaks, total_internal_leaks;

  while(iter != object_list.begin())
  {
    iter--;
    vint8 id = *iter;
    vint8 * pointer = (vint8 *) (id_to_pointer[(vector_size) id]);
    vint8 creation_flag = pointer[CREATION_INDEX];
    vint8 size;
    memcpy(&size, &(pointer[SIZE_INDEX]), sizeof(vint8));
    if (creation_flag == CREATED_BY_OTHER_CODE)
    {
      vPrint("External memory leak: id = %lu, id use = %lu, size = %lu\n",
             id, id_use_counter[(vector_size) id], size);
      errors_counter++;
      total_external_leaks = total_external_leaks + vas_int16((vint4) size);
    }
    else
    {
      assert(creation_flag == CREATED_BY_MY_CODE);
    }
  }
  
  iter = object_list.end();
  while(iter != object_list.begin())
  {
    iter--;
    vint8 id = *iter;
    vint8 * pointer = (vint8 *) (id_to_pointer[(vector_size) id]);
    vint8 creation_flag = pointer[CREATION_INDEX];
    vint8 size;
    memcpy(&size, &(pointer[SIZE_INDEX]), sizeof(vint8));
    if (creation_flag == CREATED_BY_MY_CODE)
    {
      vPrint("My memory leak: id = %i, id use = %i, size = %i\n",
             (vint4) id, (vint4) id_use_counter[(vector_size) id], (vint4) size);
      errors_counter++;
      total_internal_leaks = total_internal_leaks + vas_int16((vint4) size);
    }
  }
 
  
  if (leaks_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("Memory leaks have been detected\n");
    vPrint("External leaks: ");
    total_external_leaks.Print();
    vPrint("\n");
    vPrint("Internal leaks: ");
    total_internal_leaks.Print();
    vPrint("\n\n");
  }
  if (wrong_delete_operator_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("The wrong delete operator has been used at least once\n");
  }
  if (out_of_memory_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("Operator new has returned NULL at least once\n");
  }
  if (id_overflow_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("More than %lu pointer IDs have been allocated\n",
           POINTER_ID_LIMIT);
  }
  if (id_counter_overflow_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("At least one ID use counter has had an overflow\n");
  }
  if (bad_delete_argument_flag != 0)
  {
    vPrint("***Memory warning***:\n");
    vPrint("At least once an invalid pointer was passed to delete.\n");
  }
  if (errors_counter != 0)
  {
    vPrint("\nTotal number of memory errors: %li\n", errors_counter);
  }
  else
  {
    vPrint("\n:) No memory errors were detected\n");
  }

  if (show_statistics != 0)
  {
    vPrint("\nMemory allocated: ");
    total_memory_allocated.Print();
    vPrint("\nMemory deleted:   ");
    total_memory_deleted.Print();
    vPrint("\nDifference:       ");
    (total_memory_allocated - total_memory_deleted).Print();
    vPrint("\n\nNew was called    ");
    new_calls_counter.Print();
    vPrint("times");
    vPrint("\nDelete was called ");
    delete_calls_counter.Print();
    vPrint("times\n");
  }
}


void S_MemoryManager::RegisterMemory(vint8 * pointer)
{
  S_ObjectList::iterator * position_copy;
  const vint8 iterator_size = sizeof(S_ObjectList::iterator);

  // Illegal initialization (iterator is not contructed)
  //position_copy = (S_ObjectList::iterator *) malloc(iterator_size);

  // Fix by Diego Macrini (without calling new for iterator)
  // we init the memory using a copy of a valid NULL iterator
  S_ObjectList::iterator auxIt;
  position_copy = (S_ObjectList::iterator *) malloc(iterator_size);
  memcpy((void*)position_copy, (void*)&auxIt, iterator_size);

  // For debugging
  vint8 id_to_pointer_size = (vint8) id_to_pointer.size();
  vint8 id_use_counter_size = (vint8) id_use_counter.size();
  vint8 object_list_positions_size = (vint8) object_list_positions.size();
  vint8 available_ids_size = (vint8) available_ids.size();
  vint8 pointer_id;

  if (available_ids.size() == 0)
  {
    pointer_id = id_counter++;
    assert(id_to_pointer_size == pointer_id);
    assert(id_use_counter_size == pointer_id);
    assert(object_list_positions_size == pointer_id);
    id_to_pointer.push_back(pointer);
    id_use_counter.push_back(0);
    if ((vint8) pointer_id > POINTER_ID_LIMIT)
    {
      vPrint("***WARNING*** Overflow in pointer IDs detected.\n");
      vPrint("\nenter a string to continue:\n");
      char buffer[1000];
      vScan("%s", buffer);

      id_overflow_flag = 1;
      errors_counter++;
    }
    object_list.push_front(pointer_id);
    *position_copy = object_list.begin();
    object_list_positions.push_back(position_copy);
  }
  else 
  {
    pointer_id = available_ids.back();
    assert(id_to_pointer_size > pointer_id);
    assert(id_use_counter_size > pointer_id);
    assert(object_list_positions_size > pointer_id);
    available_ids.pop_back();
    id_to_pointer[(vector_size) pointer_id] = pointer;
    id_use_counter[(vector_size) pointer_id]++;
    if (id_use_counter[(vector_size) pointer_id] > POINTER_ID_LIMIT)
    {
      vPrint("***WARNING*** Ovefflow in id use counter detected.\n");
      vPrint("\nenter a string to continue:\n");
      char buffer[1000];
      vScan("%s", buffer);

      id_counter_overflow_flag = 1;
      errors_counter++;
    }
  
    object_list.push_front(pointer_id);
    *position_copy = object_list.begin();
    object_list_positions[(vector_size) pointer_id] = position_copy;
  }

  if ((WatchID == pointer_id) && 
      ((WatchUse == id_use_counter[(vector_size) pointer_id]) || (WatchUse == -1)))
  {
    vPrint("This is the problematic object\n");
  }
  
  pointer[ID_INDEX] = pointer_id;
  return;
}


ushort S_MemoryManager::UnregisterMemory(vint8 * pointer)
{
  ushort identified_flag;

  vint8 pointer_id = pointer[ID_INDEX];

  if (id_to_pointer.size() <= pointer_id) identified_flag = 0;
  else if (id_to_pointer[(vector_size) pointer_id] != pointer) identified_flag = 0;
  else identified_flag = 1;

  if (identified_flag == 0)
  {
    vPrint("??? Unidentified pointer passed to delete: %lu, %lu\n",
           pointer, pointer_id);
    bad_delete_argument_flag = 1;
    errors_counter++;
  }

  else 
  {
    id_to_pointer[(vector_size) pointer_id] = 0;
    available_ids.push_back(pointer_id);
    S_ObjectList::iterator * position;
    position = (S_ObjectList::iterator *) object_list_positions[(vector_size) pointer_id];
    object_list.erase(*position);
    free(position);
    object_list_positions[(vector_size) pointer_id] = 0;
  }

  return identified_flag;
}


void S_MemoryManager::IncrementErrors()
{
  errors_counter++;
}


void S_CheckMemoryManager()
{
  if (manager == 0) 
  {
    manager = new S_MemoryManager();
  }
}


void * operator new(size_t requested_size)
{
  vint8 size = (vint8) (requested_size + EXTRA_BYTES);
  ++new_calls_counter;
  total_memory_allocated = total_memory_allocated + vas_int16((vint4) size);

  void * pointer = (void *) malloc((size_t) size);
  if (pointer == 0)
  {
    vPrint("***Error*** No memory left.\n");
    out_of_memory_flag = 1;
    errors_counter++;
    return 0;
  }

  vint8 * long_pointer = (vint8 *) pointer;
  void * result = (void *) (long_pointer + EXTRA_WORDS);

  memcpy(&(long_pointer[SIZE_INDEX]), &size, sizeof(vint8));

  switch(CALL_INFORMATION)
  {
  case NORMAL_CALL:
    CALL_INFORMATION = CALL_FROM_MEMORY_CPP;
    S_CheckMemoryManager();
    manager->RegisterMemory(long_pointer);  
    CALL_INFORMATION = NORMAL_CALL;
    // If the call to new was from my code, this line will
    // be overwritten 
    long_pointer[CREATION_INDEX] = CREATED_BY_OTHER_CODE;
    break;

  case CALL_FROM_MEMORY_CPP:
    long_pointer[CREATION_INDEX] = CREATED_BY_MEMORY_CPP;
    break;

  case CALL_AFTER_MANAGER_DESTRUCTION:
    vPrint("***Warning: Operator new called after memory manager destruction\n");
    long_pointer[CREATION_INDEX] = CREATED_AFTER_MANAGER_DESTRUCTION;
    break;

  default:
    assert(0);
    break;
  }

  if (result == WatchPointer)
  {
    vPrint("This is the problematic pointer\n");
  }

  return result;
}

void * operator new[](size_t nSize)
{
  return operator new(nSize);
}

void operator delete(void * object_pointer)
{
  //V_Print("Calling delete\n");  
  if (object_pointer == 0) return;

  vint8 * long_pointer = (vint8 *) object_pointer;
  long_pointer = long_pointer - EXTRA_WORDS;
  vint8 creation_flag = long_pointer[CREATION_INDEX];
  vint8 size;
  memcpy(&size, &(long_pointer[SIZE_INDEX]), sizeof(vint8));
  total_memory_deleted = total_memory_deleted + vas_int16((vint4) size);
  ++delete_calls_counter;

  switch(CALL_INFORMATION)
  {
  case NORMAL_CALL:
    CALL_INFORMATION = CALL_FROM_MEMORY_CPP;
    S_CheckMemoryManager();
    manager->UnregisterMemory(long_pointer);
    assert((creation_flag == CREATED_BY_MY_CODE) ||
           (creation_flag == CREATED_BY_OTHER_CODE));
    CALL_INFORMATION = NORMAL_CALL;
    break;

  case CALL_FROM_MEMORY_CPP:
    assert(creation_flag == CREATED_BY_MEMORY_CPP);
    break;

  case CALL_AFTER_MANAGER_DESTRUCTION:
    vPrint("***Warning: Delete called after memory manager destruction.\n");
    return;
  }

  free((void *) long_pointer);
}

void operator delete[](void* p)
{
	::operator delete(p);
}

#endif // VASSILIS_MEMORY_CHECK
