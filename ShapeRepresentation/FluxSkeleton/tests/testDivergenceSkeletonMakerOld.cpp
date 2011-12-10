/************************************************************************

   File:		testDivergenceSkeletonMaker.cpp

   Author(s):		Pavel Dimitrov

   Created:		27 Jun 2002

   Last Revision:	$Date: 2002/07/25 20:50:50 $

   Description:	

   $Revision: 1.6 $

   $Log: testDivergenceSkeletonMaker.cpp,v $
   Revision 1.6  2002/07/25 20:50:50  pdimit
   Making release 0.1

   Revision 1.5  2002/07/23 21:02:55  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DiscreteDivergenceSkeleton is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Revision 1.4  2002/07/01 07:03:30  pdimit
   Now the thinned discrete divergence array is disected properly
   into branches; that is, the algorithm seems to be sound.

   Revision 1.3  2002/06/30 11:13:40  pdimit
   There are now skeleton creation functions that are able to compute
   the list of branches. Seems to be working fine, but more testing may be
   needed.

   Revision 1.2  2002/06/30 05:58:49  pdimit
   Just implemented the colouring of the thinned DivergenceMap.
   Also, fixed a bug in the way of counting the number of intersection
   in the 8-nbhd of a pt.

   Revision 1.1  2002/06/30 01:22:33  pdimit
   Added DivergenceSkeletonMaker and a test for it. Incomplete functionality
   and the testing is mostly in DivergenceSkeletonMaker.cpp. Only the
   thinning of the DivergenceMap is implemented.


	
   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 ***********************************************************************/

#define TESTDIVERGENCESKELETONMAKER_CPP

#include "sg.h"

extern "C"{
#include "read_ppm.h"
};
using namespace sg;

void create_ps_skeleton(DDSkeleton *sk);

int main(int argc, char *argv[]){
  int i, j, rows=200, cols=200;

  if(argc <= 2){
    std::cerr << "testDivergenceSkeletonMaker $Revision: 1.6 $\n\n";
    std::cerr << " ./testDivergenceSkeletonMaker filename.ppm command\n";
    std::cerr << "\n  the commands are:\n";
    std::cerr << "\t -da [threshold] [magnification]  DivArr as a ppm file to stdout \n";
    std::cerr << "\t -sk [threshold] [magnification] a ps file of the skeleton to stdout\n";
    
    exit(1);
  }

  char *a = get_arr(&rows, &cols, argv[1]);  

  // first, we must make a Shape
  sg::SimpleShapeMaker ssm(cols, rows);

  // update the SimpleShape
  for (i=0; i < rows; i++)
    for (j=0; j < cols; j++){
      ssm(j,i) = a[i*cols + j];
    }

  // free the input array 
  free(a);

  // the shape is being made here
  SimpleShape ss = *((SimpleShape*)ssm.getShape());

  double xmin,xmax,ymin,ymax;
  ss.getBounds(&xmin,&xmax, &ymin,&ymax);

  std::cerr << "(xmin,ymin): ("<<xmin<<", "<<ymin<<")\n";
  std::cerr << "(xmax,ymax): ("<<xmax<<", "<<ymax<<")\n";

  // we can get the contour of a SimpleShape as is consists of a
  // single Curve (actually a DiscreteSegCurve) 
  DiscreteSegCurve dsc = *(DiscreteSegCurve *)ssm.getContour();
  std::cerr << dsc << "\n";

  // now, we can create a DistanceTransform object
  DistanceTransform dt(&ss);

  // and a DivergenceMap
  DivergenceMap dm(dt);

  // to supply to the SkeletonMaker
  DivergenceSkeletonMaker dsm(dm);

  double thresh=-2.0, mag=1.0;
  if(argc >= 4)
    thresh = atof(argv[3]);

  if(argc >= 5)
    mag = atof(argv[4]);
  

  if(argv[2][1] == 's'){
    // from the SkeletonMaker, we can get the skeleton
    DDSkeleton *skeleton = dsm.getDiscreteDivergenceSkeleton(1.0/mag,
							     thresh); 
    std::cerr << "drawing skeleton ...\n";
    create_ps_skeleton(skeleton);
  }
  
  
  if(argv[2][1] == 'd'){
    // or the thinned divergence array
    DivArr &da = dsm.getThinDivArr(1.0/mag, thresh);
    
    if(1)
      {
	double scale = 50.0;
	int maxval = 255;
	std::cout << "P3\n" << da.getXSize() << " " << da.getYSize() << "\n";
	std::cout << maxval << "\n";
	
	for(int y=0; y<da.getYSize(); y++)
	  for(int x=0; x<da.getXSize(); x++){
	    int v = (int)(da(x,y).val*scale);
	    
	    if(da(x,y).visited)
	      switch(da(x,y).col){
		
	      case END_POINT_COL:
		std::cout << " 0 250 0 ";
		break;
		
	      case SK_POINT_COL:
		std::cout << -v <<" "<< -v<<" 0 ";
		break;
		
	      case BRANCHING_POINT_COL:
		std::cout << " 250 0 0 ";
		break;
		
	      default:
		//	    std::cout << " 100 100 100 ";
		if(da(x,y).dist <= 0)
		  std::cout << " 100 100 100 ";
		else
		  std::cout << " 250 250 250 ";
		break;
	      }
	    else
	      if(da(x,y).dist <= 0)
		std::cout << " 100 100 100 ";
	      else
		std::cout << " 250 250 250 ";
	    
	  }
      }
  }

  return 0;
}

void ps_draw_line(Point p1, Point p2, 
		  double cr = 0,
		  double cg = 0,
		  double cb = 0){

  std::cout << cr << " " << cg << " " << cb << " setrgbcolor\n"; 
  std::cout << p1.x << " " << p1.y << " moveto\n"; 
  std::cout << p2.x << " " << p2.y << " lineto\n"; 
  std::cout << "stroke\n";
}

void create_ps_skeleton(DDSkeleton *sk){
  DDSEdgeVect edges = sk->getEdges();
  DDSEdgeVect::iterator I;

  std::cout << "%!PS-Adobe-2.0\n/Arial-Normal findfont\n";
  std::cout << "7 scalefont\nsetfont\n";
  std::cout << "75 setlinewidth\n";
  
  // draw the edges
  for(I = edges.begin(); I != edges.end(); I++){
    DDSEdge *e = *I;
    FluxPointList fpl = e->getFluxPoints();
    FluxPointList::iterator II;
    for(II = fpl.begin(); II != (fpl.end() - 1); II++){
      Point p1 = (*II).p;
      Point p2 = (*(II+1)).p;
      ps_draw_line(p1, p2, 
		   fabs((*II).val) / 7.0, 
		   fabs((*II).val) / 7.0);
    }
  }
  
  // now draw the contour
  Curve *c = sk->getShape()->getCurves()->front();
  const double step = 0.5;
  double t=0;
  for(t = 0; t < c->getLength()-2*step; t += step){
    ps_draw_line(c->atT(t), c->atT(t+step));
  }
  ps_draw_line(c->atT(t), c->atT(0));

  // finally, finish the PS page
  std::cout << "\nshowpage\n";
}
