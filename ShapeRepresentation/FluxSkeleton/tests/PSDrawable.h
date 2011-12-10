/**************************************************************************

   File:                PSDrawable.h

   Author(s):           Pavel Dimitrov

   Created:             12 Jun 2002

   Last Revision:       $Date: 2002/06/26 04:30:48 $

   Description: 

   $Revision: 1.1 $

   $Log: PSDrawable.h,v $
   Revision 1.1  2002/06/26 04:30:48  pdimit
   After the failed attempt at getting the spline smoothing to work

   Revision 1.1.1.1  2002/06/23 05:42:08  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef PSDRAWABLE_H
#  define PSDRAWABLE_H

#include "Drawable.h"
#include <iostream>
#include <sstream>

namespace sg{
  
  class PSDrawable : public Drawable{
    
    //std::vector<string> &str;
    std::stringstream *str; //(std::stringstream::in | std::stringstream::out);

      //(std::stringstream::out);
    //std::ostream
  public:
    
    PSDrawable(double linewidth=0.2) {

      //str=std::cout;
      str = new std::stringstream(std::stringstream::in | std::stringstream::out);

      *str << "%!PS-Adobe-2.0\n/Arial-Normal findfont\n";
      *str << "7 scalefont\nsetfont\n";
      *str << linewidth << " setlinewidth\n";
    }

    virtual ~PSDrawable() {}

    void clear() {}
    void drawPoint(Point p, int col=0) { drawPoint(p.x, p.y); }
    void drawPoint(double x, double y, int col=0){
      drawLine(x,y, x,y);
    }
    void drawLine(Point p1, Point p2, int col=0){
      drawLine(p1.x, p1.y, p2.x, p2.y);
    }
    void drawLine(double x1, double y1, double x2, double y2, int col=0){
      *str << x1 << " " << y1 << " moveto\n";
      *str << x2 << " " << y2 << " lineto\n";
      *str << "stroke\n";
    }
    
    void print(){
      std::string s;
      s = str->str();
      std::cout << s << "\nshowpage\n";
    }
  };
  
}

#endif  /* PSDRAWABLE_H */
