#ifndef VASSILIS_MEMORY_H
#define VASSILIS_MEMORY_H

#include "vplatform.h"
#include "v_types.h"



#ifdef VASSILIS_MEMORY_CHECK


void vDeclareAsMyCode(vint8 * argument);

class vNewController
{
public:
  vNewController() {}

  // This operator is just used so that we can redefine new (see 
  // following lines) and make sure that this code gets executed
  // right after new returns the new memory piece.
  template<class type>
  friend type * operator << (vNewController controller, type * argument);
};


template<class type>
type * operator << (vNewController controller, type * argument)
{
  vDeclareAsMyCode((vint8 *) argument);
  return argument;
}


template <class type>
void vDeleteAndSetToZero(type *& pointer)
{
  ::delete pointer;
  pointer = 0;
}


#endif // VASSILIS_MEMORY_CHECK

#endif // VASSILIS_MEMORY_H

