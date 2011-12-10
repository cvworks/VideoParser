/**************************************************************************

   File:                Drawable.h

   Author(s):           Pavel Dimitrov

   Created:             11 Jun 2002

   Last Revision:       $Date: 2002/06/27 14:14:40 $

   Description: This is an abstract class that prvides an interface to a
                canvas-like object. For example, if a ShapeBoundary is to be
		drawn, one gives it a Drawable. (ContourCurve's know how to
		draw themselves on a Drawable)

   $Revision: 1.2 $

   $Log: Drawable.h,v $
   Revision 1.2  2002/06/27 14:14:40  pdimit
   Just updated the descriptions in the header files.

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef DRAWABLE_H
#  define DRAWABLE_H

#include "Point.h"

namespace sg {
  
  // abstract class
  class Drawable {
    
  public:
    virtual ~Drawable() { }
    virtual void setColour(int c) {} // if a Drawable implements this ...

    virtual void clear() = 0;
    virtual void drawPoint(double x, double y, int col=0) = 0;
    virtual void drawPoint(Point p, int col=0) { drawPoint(p.x, p.y); }
    virtual void drawLine(double x1, double y1, 
			  double x2, double y2,
			  int col=0) = 0;
    virtual void drawLine(Point p1, Point p2, int col=0)
    {
      drawLine(p1.x, p1.y, p2.x, p2.y);
    }
  };

}

#endif  /* DRAWABLE_H */
