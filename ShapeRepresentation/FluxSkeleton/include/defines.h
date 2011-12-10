/**************************************************************************

   File:                defines.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/06/27 14:14:40 $

   Description: Common constants and defines.

   $Revision: 1.3 $

   $Log: defines.h,v $
   Revision 1.3  2002/06/27 14:14:40  pdimit
   Just updated the descriptions in the header files.

   Revision 1.2  2002/06/26 04:30:41  pdimit
   After the failed attempt at getting the spline smoothing to work

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef DEFINES_H
#define DEFINES_H

#include <vector>
#include "Tools/BasicUtils.h"
#include "Tools/Serialization.h"

#define SQR(x) ((x)*(x))
#define EUCSQDIST(x1,y1,x2,y2) (SQR(x1-x2) + SQR(y1-y2)) 

#define PI 3.14159265
#define CW  -1    // clockwise
#define CCW  1    // counter-clockwise

#endif  /* DEFINES_H */
