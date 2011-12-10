/************************************************************************

   File:		testPoint.cpp

   Author(s):		Pavel Dimitrov

   Created:		11 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testPoint.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTPOINT_CPP

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

#include <iostream>
#include "Point.h"

using namespace sg;


int main(){

  Point p1(0.5, .3);
  Point p2(0.5, .3);
  Point p3(0.15, .3);

  

  std::cout << (p1 == p2)<< " ";
  std::cout << (p1 == p3) << "\n";

  std::cout << p1 << " == " << p3 << " is " << (p1.isClose(p3,0.10)) << "\n";
  std::cout << EUCSQDIST(p1.x,p1.y, p3.x,p3.y) << "\n";
  return 0;
}
