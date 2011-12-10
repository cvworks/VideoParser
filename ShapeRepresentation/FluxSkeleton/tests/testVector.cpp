/************************************************************************

   File:		testVector.cpp

   Author(s):		Pavel Dimitrov

   Created:		11 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	

   $Revision: 1.1.1.1 $

   $Log: testVector.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTVECTOR_CPP

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
#include "Vector.h"

using namespace sg;


int main(){

  Vector v1(0.5, .3);
  Vector v2(0.5, .3);
  Vector v3(0.15, .3);

  

  std::cout << (v1 == v2)<< " ";
  std::cout << (v1 == v3) << "\n";

  std::cout << v1 << " == " << v3 << " is " << (v1.isClose(v3,0.10)) << "\n";
  std::cout << EUCSQDIST(v1.x,v1.y, v3.x,v3.y) << "\n";

  std::cout <<"dot: " <<v1.dot(v1) << "\n";
  std::cout << "sqrt(dot): " <<sqrt(v1.dot(v1)) << "\n";
  std::cout << "norm: " <<v1.norm() << "\n";
  return 0;
}
