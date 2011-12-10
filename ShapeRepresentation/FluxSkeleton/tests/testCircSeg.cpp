/************************************************************************

   File:		testCircSeg.cpp

   Author(s):		Pavel Dimitrov

   Created:		22 Jun 2002

   Last Revision:	$Date: 2002/06/23 05:42:09 $

   Description:	Spits a ppm format image to stdout and information on stderr

   $Revision: 1.1.1.1 $

   $Log: testCircSeg.cpp,v $
   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTCIRCSEG_CPP

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

#include "CircSeg.h"

using namespace sg;

int main(){

  Point p1(0,.9);
  Point p0(1,0.9);
  Point centre(0.5,0);


  //  CircSeg cs(p0, p1, centre, CCW);
  CircSeg cs(Point(1,1), Point(.75,.9), Point(1.75/2, 1.9/2));
  std::cerr << cs << '\n';




  CircSeg cs1(p0, p1, centre, CW);
  std::cerr << cs1 << '\n';


  double l = cs.getLength();

  int my=200, mx=my;
  
  bool *arr = new bool[my*mx];

  for(int i=0; i<mx*my; i++)
    arr[i] = false;

  for(double t=0; t<=l; t+=0.01){
   
    Point p = cs.atT(t);
    p.x = 50*p.x + 100;
    p.y = 50*(-p.y) + 100;

    arr[(int)p.y*mx + (int)p.x] = true;


  }

  std::cout << "P3\n" << mx << " " << my << "\n256\n";

   for(int i=0; i<mx*my; i++)
     if(arr[i])
       std::cout << " 255 0 0 ";
     else
       std::cout << " 0 0 255 ";


  return 0;
}
