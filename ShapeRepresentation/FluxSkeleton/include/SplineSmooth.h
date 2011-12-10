/**************************************************************************

   File:                SplineSmooth.h

   Author(s):           Pavel Dimitrov

   Created:             25 Jun 2002

   Last Revision:       $Date: 2002/06/26 04:32:07 $

   Description: 

   $Revision: 1.1 $

   $Log: SplineSmooth.h,v $
   Revision 1.1  2002/06/26 04:32:07  pdimit
   After the failed attempt at getting the spline smoothing to work


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef SPLINESMOOTH_H
#  define SPLINESMOOTH_H

#include <vector>

namespace sg{
  void spline_smooth_pts(std::vector<Point> &pts, float weight);
}
#endif  /* SPLINESMOOTH_H */
