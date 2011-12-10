/************************************************************************

   File:		testDivergenceMap.cpp

   Author(s):		Pavel Dimitrov

   Created:		26 Jun 2002

   Last Revision:	$Date: 2002/07/23 21:02:54 $

   Description:	

   $Revision: 1.2 $

   $Log: testDivergenceMap.cpp,v $
   Revision 1.2  2002/07/23 21:02:54  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DiscreteDivergenceSkeleton is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Revision 1.1  2002/06/26 11:51:38  pdimit
   Implemented the DivergenceMap class. It is supposed to be a base
   class for other implementations. It has very dumb algorithms, but
   which seem to work just fine. Look at testDivergenceMap to see how
   to use it. Also, testSimpleShapeMaker has a much nicer interface --
   exactly the same as for testDivergenceMap...


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTDIVERGENCEMAP_CPP

#include "DivergenceMap.h"
#include "SimpleShapeMaker.h"
#include "SimpleShape.h"
#include "DiscreteSegCurve.h"
#include "sg.h"

#include <cmath>

extern "C"{
#include "read_ppm.h"
};
using namespace sg;

struct DivPt{
  double div;
  int v;

  DivPt(double d, int i) {div=d; v = i;}
};

int main(int argc, char *argv[]){
  int i, j, rows=200, cols=200;

  if(argc <= 1){
    std::cerr << "testDivergenceMap $Revision: 1.2 $\n\n";
    std::cerr << " ./testDivergenceMap filename.ppm [intensitymag] [magnification]\n";
    
    std::cerr << "\n Writes to stdout a .ppm file\n";

    exit(1);
  }

  char *a = get_arr(&rows, &cols, argv[1]);  

  sg::SimpleShapeMaker ssm(cols, rows);

  // update the SimpleShape
  for (i=0; i < rows; i++)
    for (j=0; j < cols; j++){
      ssm(j,i) = a[i*cols + j];
    }

  // free the input array 
  free(a);

  
  SimpleShape ss = *((SimpleShape*)ssm.getShape());

  double xmin,xmax,ymin,ymax;
  ss.getBounds(&xmin,&xmax, &ymin,&ymax);

  std::cerr << "(xmin,ymin): ("<<xmin<<", "<<ymin<<")\n";
  std::cerr << "(xmax,ymax): ("<<xmax<<", "<<ymax<<")\n";

  DiscreteSegCurve dsc = *(DiscreteSegCurve *)ssm.getContour();
  std::cerr << dsc << "\n";

  DistanceTransform dt(&ss);

  DivergenceMap dm(dt);

  int maxval = 255;
  double magnification = 1.0;
  double intensmag = 100;

  if(argc >= 3)
    intensmag = atof(argv[2]);

  if(argc >= 4)
    magnification = atof(argv[3]);


  std::cerr << "(x"<<magnification<< ") and intensity aug of: "<<intensmag;
  std::cerr << "\n";
  
  double bdry = 10;
  double xm = floor(xmin-bdry), xM = ceil(xmax+bdry);
  double ym = floor(ymin-bdry), yM = ceil(ymax+bdry);

  int xs=0, ys=0;
  double x, y;
  std::vector<DivPt> vals;

  double dmax = -100000, dmin = 100000;

  double step = 1.0/magnification;
  for (y=ym, ys=0; y < yM; y+=step, ys++)
    for ( x=xm, xs=0; x < xM; x+=step, xs++)
      {

	double div = dm.getValue(x, y, step*1.0);
	if(div < dmin) dmin = div;
	if(div > dmax) dmax = div;

	int v = (int)(div*intensmag);

	if (v >= maxval) v = maxval-1;
	if (v <= -maxval) v = -(maxval-1);
   
	vals.push_back(DivPt(div,v));
      }

  std::cerr << "dmin: "<<dmin<<"\tdmax: " <<dmax<<"\n";

  
  std::vector<DivPt>::iterator I;

  std::cout << "P3\n" << xs << " " << ys << "\n" << maxval << "\n";
  
  for(I=vals.begin(); I!=vals.end(); I++){
    DivPt dp = *I;

    int v = dp.v;

    if(v < 0){
      if(fabs(dp.div + 4.0) < 0.1){
	std::cerr << dp.div << "\n";
	std::cout << -dp.v << " 0 0 ";
      }
      else
	std::cout << -v << " " << -v <<" 0 ";
      
    }
    else
      std::cout << " 0 0 " << v << " ";
  }

  return 0;
}
