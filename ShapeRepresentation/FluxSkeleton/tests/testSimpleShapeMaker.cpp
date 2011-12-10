/************************************************************************

   File:		testSimpleShapeMaker.cpp

   Author(s):		Pavel Dimitrov

   Created:		22 Jun 2002

   Last Revision:	$Date: 2002/07/23 21:02:55 $

   Description:	

   $Revision: 1.6 $

   $Log: testSimpleShapeMaker.cpp,v $
   Revision 1.6  2002/07/23 21:02:55  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DiscreteDivergenceSkeleton is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Revision 1.5  2002/06/26 11:51:38  pdimit
   Implemented the DivergenceMap class. It is supposed to be a base
   class for other implementations. It has very dumb algorithms, but
   which seem to work just fine. Look at testDivergenceMap to see how
   to use it. Also, testSimpleShapeMaker has a much nicer interface --
   exactly the same as for testDivergenceMap...

   Revision 1.4  2002/06/26 07:47:29  pdimit
   Just added the class SimpleShape to the mix.
   It is precisely that, simple. It consist of a
   single header file -- SimpleShape.h.
   However, SimpleShapeMaker does not compute the
   xmin/xmax and ymin/ymax values yet.

   Revision 1.3  2002/06/26 06:34:47  pdimit
   After adding a more successful smoothing.
   A point becomes the average of its previou nbr, next nbr and
   itself. The first and end points (which are the same) do not change.
   This smoothing primitive is applied 5 times to get a nicer result.

   Revision 1.2  2002/06/26 04:30:48  pdimit
   After the failed attempt at getting the spline smoothing to work

   Revision 1.1.1.1  2002/06/23 05:42:09  pdimit
   Initial import


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTSIMPLESHAPEMAKER_CPP

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

#include "SimpleShapeMaker.h"
#include "SimpleShape.h"
#include "DiscreteSegCurve.h"
#include "sg.h"

extern "C"{
#include "read_ppm.h"
};
using namespace sg;

int main(int argc, char *argv[]){
  int i, j, rows=200, cols=200;

  if(argc <= 1){
    std::cerr << "testSimpleShapeMaker $Revision: 1.6 $\n\n";
    std::cerr << " ./testSimpleShapeMaker filename.ppm [intensitymag] [magnification]\n";
    
    std::cerr << "\n Writes to stdout a .ppm file representing\n";
    std::cerr << "the distance transform of filename.ppm.\n";

    exit(1);
  }
  char *a = get_arr(&rows, &cols, argv[1]);  

  sg::SimpleShapeMaker ssm(cols, rows);
  
  for (i=0; i < rows; i++)
    for (j=0; j < cols; j++){
      ssm(j,i) = a[i*cols + j];
    }
  free(a);

  SimpleShape ss = *((SimpleShape*)ssm.getShape());

  double xmin,xmax,ymin,ymax;
  ss.getBounds(&xmin,&xmax, &ymin,&ymax);

  std::cerr << "(xmin,ymin): ("<<xmin<<", "<<ymin<<")\n";
  std::cerr << "(xmax,ymax): ("<<xmax<<", "<<ymax<<")\n";

  DistanceTransform dt(&ss);
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
  std::vector<int> vals;

  for (y=ym, ys=0; y < yM; y+=1.0/magnification, ys++)
    for ( x=xm, xs=0; x < xM; x+=1.0/magnification, xs++)
      {
	int v = (int)(dt.getValue(x, y)*intensmag);

	if (v >= maxval) v = maxval-1;
	if (v <= -maxval) v = -(maxval-1);
   
	vals.push_back(v);
      }

  
  std::vector<int>::iterator I;

  std::cout << "P3\n" << xs << " " << ys << "\n" << maxval << "\n";
  
  for(I=vals.begin(); I!=vals.end(); I++){
    int v = *I;
    
    if(v < 0)
      std::cout << -v << " " << -v <<" 250 ";
    else
      std::cout << " 0 0 " << v << " ";
  }

  return 0;
}
