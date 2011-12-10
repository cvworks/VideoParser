// commented out 01/07/2006 by Vassilis. I think it shouldn't be there.
//#ifndef VASSILIS_UNDEFINE_H
//#define VASSILIS_UNDEFINE_H

#ifdef VASSILIS_DEFINITIONS_H
#undef VASSILIS_DEFINITIONS_H

#undef printf
#undef function_print

#ifdef VASSILIS_WINDOWS_MFC

#undef scanf
#undef cin
#undef cout
#undef endl

#endif // VASSILIS_WINDOWS_MFC

#ifdef VASSILIS_MEMORY_CHECK
#undef new
#endif // VASSILIS_MEMORY_CHECK

#endif // VASSILIS_DEFINITIONS_H

//#endif // VASSILIS_UNDEFINE_H
