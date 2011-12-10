/************************************************************************

   File:		testDiscreteSegCurve.cpp

   Author(s):		Pavel Dimitrov

   Created:		12 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testDiscreteSegCurve.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTDISCRETESEGCURVE_CPP

#include "DiscreteSegCurve.h"
#include <iostream>

#include "LineSeg.h"
#include "PSDrawable.h"

using namespace sg;

int main()
{
  std::vector<int> v;

  v.push_back(34);
  v.push_back(234);

  std::cout << v.size() << '\n';

  std::vector<int>::iterator vI = v.begin();

  for(;vI != v.end(); vI++)
    std::cout << *vI << ' ';

  ////////////////////

  //  LineSeg ls1(Point(0,0), Point(3,0));
  LineSeg ls1(Point(3,0), Point(0,0));
  LineSeg ls2(Point(3,0), Point(3,1));
  
  std::vector<CurveSeg *> segs;

  segs.push_back(&ls1);
  segs.push_back(&ls2);

  std::vector<CurveSeg *>::iterator I = segs.begin();

  for(;I != segs.end(); I++)
    {
      LineSeg *ls = (LineSeg*)*I;
      std::cout << *ls << ' ';
    }

  std::cout << '\n';

  DiscreteSegCurve dsc(segs, false);
  std::cout << dsc << '\n';
  
  PSDrawable psd;

  dsc.drawCurve(psd);
  //psd.print();

  std::cout << "dist: " << dsc.distToPt(Point(0.5,1)) << std::endl;
  Point pp = dsc.atT(dsc.closestToPt(Point(0.5,0.8)));
  std::cout << "closest: " << dsc.closestToPt(Point(0.5,0.8)) << " " << pp << std::endl;

  double t = 3.41421;//2.75;
  Point p = dsc.atT(t);
  Vector vv = dsc.normal(t);
  std::cout << dsc.getLength() << '\n';
  std::cout << "at(" << t << ") = " << p << " " << vv <<'\n'; 

  DiscreteSegCurve *dsc0 = (DiscreteSegCurve *)dsc.clone();

  //  std::cout <
  return 0;
}
