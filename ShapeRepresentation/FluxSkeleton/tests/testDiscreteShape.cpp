/************************************************************************

   File:		testDiscreteShape.cpp

   Author(s):		Pavel Dimitrov

   Created:		17 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testDiscreteShape.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTDISCRETESHAPE_CPP


#include "DiscreteShape.h"
#include "LineSeg.h"
#include "DiscreteSegCurve.h"

using namespace sg;

int main(){

  LineSeg ls1(Point(0,0), Point(1,0));
  LineSeg ls2(Point(1,0), Point(1,1));
  LineSeg ls3(Point(1,1), Point(0,0));
  
  std::vector<CurveSeg *> segs;

  segs.push_back(&ls1);
  segs.push_back(&ls2);
  segs.push_back(&ls3);

  std::vector<CurveSeg *>::iterator I = segs.begin();

  for(;I != segs.end(); I++)
    {
      LineSeg *ls = (LineSeg*)*I;
      //std::cout << *ls << ' ';
    }

  //std::cout << '\n';

  DiscreteSegCurve dsc(segs, false);

  
  DiscreteShape ds(-0.5,-0.5, 100,100, 0.02,0.02, &dsc);

  //std::cout << ds.isInside(1,1) << '\n';

  //ds.spit();

  std::cout << "P3\n210 210\n256\n";

  for (int y=-10; y < 200; y++)
    for (int x=-10; x < 200; x++)
      {
	if (ds.isInside((double)x*0.02 , 
			(double)y*0.02 ))
	  std::cout << "255 0 0 ";
	else
	  std::cout << "0 0 255 ";
      }

  return 0;
}
