/************************************************************************

   File:		testDistanceTransform.cpp

   Author(s):		Pavel Dimitrov

   Created:		19 Jun 2002

   Last Revision:	$Date: 2002/06/26 04:30:48 $

   Description:	

   $Revision: 1.2 $

   $Log: testDistanceTransform.cpp,v $
   Revision 1.2  2002/06/26 04:30:48  pdimit
   After the failed attempt at getting the spline smoothing to work

   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTDISTANCETRANSFORM_CPP

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

#include "DistanceTransform.h"
#include "sg.h"

using namespace sg;

int main(){

  LineSeg ls1(Point(0,0), Point(1,0));
  //LineSeg ls2(Point(1,1), Point(1,0));
  LineSeg ls2(Point(1,0), Point(1,1));
  LineSeg ls3(Point(1,1), Point(.75,.9));
  LineSeg ls4(Point(0.75,0.9), Point(-.3,0.3));
//    LineSeg ls4(Point(1,0), Point(0,0));
//    LineSeg ls3(Point(1,1), Point(1,0));
//    LineSeg ls2(Point(.75,.9), Point(1,1));
//    LineSeg ls1(Point(0,0), Point(0.75,0.9));

  CircSeg cs1(Point(1,1), Point(.75,.9), Point(1.75/2, 1.9/2));
  std::cerr << cs1 << '\n';

  CircSeg cs2(Point(-0.3,0.3), Point(0,0), Point(-0.15, 0.15));

  Point pt = cs2.atT(cs2.getLength());
  std::cerr << cs2 << "\n" << pt << "\n";
  
  std::vector<CurveSeg *> segs;

  segs.push_back(&ls1);
  segs.push_back(&ls2);
  segs.push_back(&cs1);
  //segs.push_back(&ls3);
  segs.push_back(&ls4);
  segs.push_back(&cs2);

//    std::vector<CurveSeg *>::iterator I = segs.begin();

//    for(;I != segs.end(); I++)
//      {
//        LineSeg *ls = (LineSeg*)*I;
//        //std::cout << *ls << ' ';
//      }


  DiscreteSegCurve dsc(segs, false);

//    std::cerr << "here" << '\n';
  
  Curve *curve1 = dsc.clone();

  DiscreteShape ds(0,0, 500,500, 0.002,0.002, &dsc);
//    std::cerr << "there" << '\n';

  //  DistanceTransform dt(&ds);


  int xs = 500, ys=xs;
  int maxval = 1000;
  double scale = 0.005;
  std::cout << "P3\n"<< xs << " " << ys << "\n" << maxval <<"\n";

  for (int y=-100; y < ys-100; y++)
    for (int x=-100; x < xs-100; x++)
      {
	//int v = (int)(dt.getValue((double)x*0.005, (double)y*0.005)*1500);
	int v = (int)(curve1->distToPt(Point((double)x*scale, 
					 (double)y*scale))*85500);
	//Vector ve = (dt.getGradValue((double)x*0.005, (double)y*0.005));
	//ve.x ++;

	if (v >= maxval) v = maxval-1;
	if (v <= -maxval) v = -(maxval-1);

	if(v < 0)
	  std::cout << -v << " 0 0 ";
	else
	  std::cout << " 0 0 " << v << " ";


//    	if (ds.isInside((double)x*0.02 , 
//    			(double)y*0.02 ))
//    	  std::cout << "255 0 0 ";
//    	else
//  	  std::cout << v << " " << v << " " << v << '\n';
	//  	  std::cout << "0 0 255 ";
      }

  std::cerr << "THE END\n";
  return 0;

}
