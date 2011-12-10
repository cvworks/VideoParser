/************************************************************************

   File:		testLineSeg.cpp

   Author(s):		Pavel Dimitrov

   Created:		11 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testLineSeg.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTLINESEG_CPP

/**********************
     Include Files
 **********************/

/**********************
    Private Defines
 **********************/

/**********************
     Private Types
 **********************/

/**********************
    Private Variables
 **********************/

/**********************
    Private Functions
 **********************/

#include "LineSeg.h"

using namespace sg;

int main()
{
  LineSeg *ls0 = new LineSeg(Point(0,0), Point(1,0.1));
  
  LineSeg &ls = *ls0;

  std::cout << ls << '\n';
  
  Point p(0.25,0);

  std::cout << ls << ".toPt " << p << " = " << ls.distToPt(p) << '\n';
  std::cout << ls << ".ClPttoPt " << p << " = " << ls.closestToPt(p) << '\n';
  //  std::cout << "atT(.ClPttoPt) " << " = " << (ls.atT(ls.closestToPt(p))) << '\n';
  
  Point pt = ls.atT(ls.closestToPt(p));

  std::cout << pt << '\n';
  return 0;
}
