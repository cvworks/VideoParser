/************************************************************************

   File:		testDivergenceSkeletonMaker.cpp

   Author(s):		Pavel Dimitrov

   Created:		27 Jun 2002

   Last Revision:	$Date: 2003/06/05 04:44:54 $

   Description:	

   $Revision: 1.8 $

   $Log: testDivergenceSkeletonMaker.cpp,v $
   Revision 1.8  2003/06/05 04:44:54  pdimit
   Before making the switch to the new ordering.
   The new ordering will be
   DivPt p1,p2;

   p1 <= p2    <==>    (p1.dist < p2.dist AND abs(p1.dist - p2.dist) < sigma)
                    OR (p1.div < p2.div)

   where sigma is the lattice spacing.

   Revision 1.7  2002/11/20 22:03:09  pdimit
   A LOT has changed since last time.

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
#include "FastDistanceTransform.h"

extern "C"{
#include "read_ppm.h"
};
using namespace sg;

////////////////////////////////////////////////////////
void create_ps_skeleton(DDSkeleton *sk, int freq=0, bool shift=true);
void create_ps_gradient_map(DDSkeleton *sk, int frequency=7, bool shift=true);
void create_ps_ligature_skeleton(DDSkeleton *sk, double ligthr=0.5);
void ps_write_header(char *title, 
		     double xmin, double xmax, 
		     double ymin, double ymax,
		     double sigma); 
void ps_draw_contour(DDSkeleton *sk);
void ps_scale(double sx, double sy);
void ps_translate(double sx, double sy);

double shift_FluxPoints(FluxPointList &fpl, DDSkeleton *sk, 
			bool update_flux=true);
double compute_flux(Point p, DistanceTransform *dt, double eps, int np);

////////////////////////////////////////////////////////
int main(int argc, char *argv[]){
  int i, j, rows=200, cols=200;

  if(argc <= 2){
    std::cerr << "testDivergenceSkeletonMaker $Revision: 1.8 $\n\n";
    std::cerr << " ./testDivergenceSkeletonMaker filename.ppm command\n";
    std::cerr << "\n  the commands are:\n";
    std::cerr << "\t -d [threshold] [magnification]  DivArr as a ppm file to stdout \n";

    std::cerr << "\t -s [threshold] [magnification] [shift=0|1] [frequency]   a ps file of\n";
    std::cerr << "\t\tthe skeleton to stdout (print reconstructed lines\n";
    std::cerr << "\t\tat every 'frequency' points)\n";

    std::cerr << "\t -g [threshold] [magnification] [frequency] a ps file of the gradient map at every 'frequency' pixels on the skeleton to stdout\n";

    std::cerr << "\t -l [threshold] [magnification] [ligthr] a ps file of the shape and skeleton with ligature removed to stdout\n";
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
  //  FastDistanceTransform dt(&ss);

  // and a DivergenceMap
  DivergenceMap dm(dt);

  // to supply to the SkeletonMaker
  DivergenceSkeletonMaker dsm(dm);

  double thresh=-2.0, mag=1.0;
  if(argc >= 4)
    thresh = atof(argv[3]);

  if(argc >= 5)
    mag = atof(argv[4]);
  
  if(mag <= 0){
    std::cerr << "Must choose mag > 0\n";
    exit(1);
  }

  double sigma = 1/mag;

  double area_complexity = (xmax - xmin) * (ymax - ymin) * mag * mag;
  
  std::cerr << "Complexity: ";
  std::cerr << (double)dsc.get_num_segs() * area_complexity << "\n";


  char title[256] = "\n";

  if(argv[2][1] == 's'){
    // from the SkeletonMaker, we can get the skeleton
    DDSkeleton *skeleton = dsm.getDiscreteDivergenceSkeleton(1.0/mag,
							     thresh); 

    int freq = 0; bool shift=true;
    if(argc >= 6)
      shift = atoi(argv[5]);
    if(argc >= 7)
      freq = atoi(argv[6]);


     
    sprintf(title, "file: %s mag: %.1f thresh: %.2f freq: %d", 
	    argv[1], mag, thresh, freq);
    
    std::cerr << "drawing skeleton ...\n";
    
    ps_write_header(title, xmin, xmax, ymin, ymax, sigma);
    ps_scale(1,-1);
    ps_translate(0, -ymin-ymax);
    
    ps_draw_contour(skeleton);
    create_ps_skeleton(skeleton, freq, shift);
  }
  
  if(argv[2][1] == 'g'){
    // from the SkeletonMaker, we can get the skeleton
    DDSkeleton *skeleton = dsm.getDiscreteDivergenceSkeleton(1.0/mag,
							     thresh); 
    std::cerr << "drawing skeleton ...\n";

    ps_write_header(title, xmin, xmax, ymin, ymax, sigma);

    ps_scale(1,-1);
    ps_translate(0, -ymin-ymax);

    ps_draw_contour(skeleton);

    int freq = 7;
    if(argc >= 6)
      freq = atoi(argv[5]);


    create_ps_gradient_map(skeleton, freq);
  }
  
  if(argv[2][1] == 'l'){
    // from the SkeletonMaker, we can get the skeleton
    DDSkeleton *skeleton = dsm.getDiscreteDivergenceSkeleton(1.0/mag,
							     thresh); 
    std::cerr << "drawing skeleton ...\n";


    double ligthr = 0.5;
    if(argc >= 6)
      ligthr = atof(argv[5]);


    sprintf(title, "file: %s mag: %.1f thresh: %.2f ligthr: %.2f", 
	    argv[1], mag, thresh, ligthr);
    ps_write_header(title, xmin, xmax, ymin, ymax, sigma);

    ps_scale(1,-1);
    ps_translate(0, -ymin-ymax);

    ps_draw_contour(skeleton);

    create_ps_ligature_skeleton(skeleton, ligthr);
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


void ps_scale(double sx, double sy){
  std::cout << sx << " " << sy << " scale\n";
}
void ps_translate(double sx, double sy){
  std::cout << sx << " " << sy << " translate\n";
}
void ps_moveto(Point p){
  std::cout << p.x << " " << p.y << " moveto\n";
}
void ps_lineto(Point p){
  std::cout << p.x << " " << p.y << " lineto\n";
}

void ps_digipt(Point p){
  std::cout << p.x << " " << p.y << " dp\n";
}
void ps_setlinewidth(double w){
  std::cout << w <<" setlinewidth\n";
}
void ps_stroke(){
  std::cout <<"stroke\n";
}
void ps_fill(){
  std::cout <<"fill\n";
}
void ps_gsave(){
  std::cout <<"gsave\n";
}
void ps_grestore(){
  std::cout <<"grestore\n";
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

void ps_draw_circle(Point centre, double r, bool fill=false){
    std::cout << centre.x << " " << centre.y << " moveto\n"; 
    std::cout << centre.x << " " << centre.y << " "; 

    if (fill)
      std::cout << r << " 0 360 arc \nfill\n";
    else
      std::cout << r << " 0 360 arc \nstroke\n";

}

void ps_define_disc(){
  std::cout << "/disc {1 0 360 arc fill} def\n";
}

void ps_draw_disc(Point centre){
  //    std::cout << centre.x << " " << centre.y << " moveto\n"; 
  std::cout << centre.x << " " << centre.y << " disc\n"; 
}

void ps_setrgbcolor(double cr, double cg, double cb){
  std::cout << cr << " " << cg << " " << cb << " setrgbcolor\n"; 
}

// pb-->pe: l is length as is tip size
void ps_arrow_line(Point pb, Point pe, double as)
{

  Vector v(pe.x-pb.x, pe.y-pb.y);

  v.normalize();

  std::cout <<pb.x<<" "<<pb.y<<" "<<pe.x<<" "<<pe.y<< " line\n"; 
  //
  
  if(as <= 0.0){
    ps_stroke();
    return; 
  }

  //  Point p1(p.x+v.x, p.y+v.y);
  Point p1(pe.x, pe.y);
  
  v.normalize();

  double a = 3.1415 / 9.0;
  v.scale(-as/cos(a));

  v.rotate(a);
  Point p2(pe.x+v.x, pe.y+v.y);
  v.rotate(-2*a);
  Point p3(pe.x+v.x, pe.y+v.y);

  std::cout <<p1.x<<" "<<p1.y<<" "<<p2.x<<" "<<p2.y<<" "<<p3.x<<" "<<p3.y;
  std::cout <<" triangle\n";

  //  ps_stroke();

}


void ps_write_header(char *title, 
		     double xmin, double xmax,
		     double ymin, double ymax,
		     double sigma)
{
  std::cout << "%!PS-Adobe-2.0 EPSF-1.2\n";

  std::cout << "%%Title: " << title << "\n";


  std::cout << "%%Pages: 1\n";
  std::cout << "%%BoundingBox: " << xmin-1.0 << " " << ymin-1.0 << " " << xmax+1.0 << " " << ymax+1.0 <<" \n";


  //std::cout << "7 scalefont\nsetfont\n";
  //std::cout << ".5 setlinewidth\n";

  ps_define_disc();
  std::cout << "/dp { "<< sigma/2.0 <<" 0 360 arc fill} def\n"; // digital point represented as a disc
  std::cout << "/rl {rlineto} bind def\n";
  std::cout << "/rm {rmoveto} bind def\n";
  std::cout << "/m {moveto} bind def\n";
  std::cout << "/l {lineto} bind def\n";

  // xorigin yorigin dx dy tiplength arrow

  // x1 y1 x2 y2
  std::cout << "/line { moveto lineto stroke} def\n";

  // x1 y1 x2 y2 x3 y3
  std::cout << "/triangle {newpath m l l closepath stroke} def\n";

  std::cout << "\n";

}

///////// compute_curvature():
//
//
//
/////////
double compute_curvature(Point p1, Point p2, Point p3){

  const double comp_prec = 0.0000001;

  double ma = (p2.y - p1.y) / (p2.x - p1.y);
  double mb = (p3.y - p2.y) / (p3.x - p2.y);

  if (fabs(ma - mb) < comp_prec) 
    return 0.0;

  double x = ma*mb*(p1.y-p3.y) + mb*(p1.x+p2.x) - ma*(p2.x+p3.x) / (2*(mb-ma));
  double y;

  if(fabs(ma) > comp_prec)
    y = - 1/ma * (x - (p1.x+p2.x)/2.0) + (p1.y+p2.y)/2.0;
  else
    y = - 1/mb * (x - (p3.x+p2.x)/2.0) + (p3.y+p2.y)/2.0;

  double r = p1.distanceToPt(Point(x,y));
  if (r < comp_prec)
    return 1 / comp_prec;
  else
    return 1/r;
}

///////// compute_dalpha_ds():
//
//
//
/////////
double compute_dalpha_ds(DDSkeleton *sk, FluxPoint fp)
{
  DistanceTransform dt(sk->getShape());
  double Pi = 3.141592654;
  double alpha;

  // do not consider "bad points"
  if (fabs(fp.val) > 4.0)
    return 0;

  if (fabs(fp.val) > 4.0)
    alpha = Pi / 2.0;
  else
    alpha = asin(fabs(fp.val) / 4.0);  

  
  double eps = 0.001;
  const int n = 30;
  const double Pi_n = 2.0 * 3.141592654 / (double) n;
  double xx,yy;
  xx = fp.p.x + eps*cos(0.0);
  yy = fp.p.y + eps*sin(0.0);

  Vector vprev = dt.getGradValue(xx, yy, 0.001*eps);

  double val = 0.0;
  for(int i = 1; i < n; i++){
   
    xx = fp.p.x + eps*cos(((double)i)*Pi_n);
    yy = fp.p.y + eps*sin(((double)i)*Pi_n);

    Vector v = dt.getGradValue(xx, yy, 0.001*eps);

    val += acos(v.dot(vprev));
    vprev = v;
  }


  return val; // (2.0 * 3.141592654 * eps);

}

///////// compute_alpha_p():
//
//
//
/////////
double compute_alpha_p(FluxPoint fpm2, 
		       FluxPoint fpm1, 
		       FluxPoint fpm0, 
		       FluxPoint fpp1, 
		       FluxPoint fpp2)
  
{
  
  // do not consider "bad points"
  if (fabs(fpm2.val) > 4.0 || 
      fabs(fpm1.val) > 4.0 || 
      fabs(fpm0.val) > 4.0 || 
      fabs(fpp1.val) > 4.0 || 
      fabs(fpp2.val) > 4.0)
    return 0.0;
  
  const int i = 2;
  double a[5]= { asin(fabs(fpm2.val) / 4.0),  // alpha at i-2
		 asin(fabs(fpm1.val) / 4.0),  // alpha at i-1
		 asin(fabs(fpm0.val) / 4.0),  // alpha at i
		 asin(fabs(fpp1.val) / 4.0),  // alpha at i+1
		 asin(fabs(fpp2.val) / 4.0),  // alpha at i+2
  };
  double h = fpm2.p.distanceToPt(fpp2.p) / 4.0;

  double alpha_p = (-a[i+2] + 8*a[i+1] - 8*a[i-1] + a[i-2]) / (12*h);
  //double alpha_p = (a[i+1] - a[i-1]) / (2*h)

  double tmpap = sin(a[i]) / fabs(fpm0.dist);
  if(alpha_p > tmpap)
    alpha_p = tmpap;

  return alpha_p;
}



///////// compute_bar():
//
// Computes the boundary-to-axis ratio. The formula is as follows:
// dB / ds  = r(s) (alpha'(s) +- kappa(s)) - sin(alpha)
// 
// but we return the combined ratio for both sides, i.e.
// 2 r(s) alpha'(s) - 2 sin(alpha)
//
// NOTE: Must have computed alpha_p on the FluxPoints !
//
/////////
double compute_bar(DDSkeleton *sk, 
		   FluxPoint fp)
// 		   FluxPoint fpm2, 
// 		   FluxPoint fpm1, 
// 		   FluxPoint fpm0, 
// 		   FluxPoint fpp1, 
// 		   FluxPoint fpp2)
{
  double bar = 2.0;
  double r = fabs(fp.dist);
  
  if (fabs(fp.val) > 4.0)
    return bar;

  double alpha = asin(fabs(fp.val) / 4.0);
  double alpha_p = fp.alpha_p;

//   double Pi = 3.141592654;

//   // do not consider "bad points"
//   if (fabs(fpm2.val) > 4.0 || 
//       fabs(fpm1.val) > 4.0 || 
//       fabs(fpm0.val) > 4.0 || 
//       fabs(fpp1.val) > 4.0 || 
//       fabs(fpp2.val) > 4.0)
//     return bar;
  
//   const int i = 3;
//   double a[5]= { asin(fabs(fpm2.val) / 4.0),  // alpha at i-2
// 		 asin(fabs(fpm1.val) / 4.0),  // alpha at i-1
// 		 asin(fabs(fpm0.val) / 4.0),  // alpha at i
// 		 asin(fabs(fpp1.val) / 4.0),  // alpha at i+1
// 		 asin(fabs(fpp2.val) / 4.0),  // alpha at i+2
//   };
//   double h = fpm2.p.distanceToPt(fpp2.p) / 3.0;

//   double alpha_p = (-a[i+2] + 8*a[i+1] - 8*a[i-1] + a[i-2]) / (12*h);
//   double alpha = a[i];

  // alpha'(s) = d(alpha(s))/ds = d(sin(alpha))/ds / cos(alpha) 
  // but it must be in the direction of decreasing radius
  // is the direction of the tangent good ?

//   double sign = 1.0;
//   if (fabs(fp.dist) > fabs(fp_prev.dist)){
//     sign = -1.0;
//   }

//   double alpha_prev;
//   if (fabs(fp_prev.val) >= 4.0)
//     return bar;

//   alpha_prev = asin(fabs(fp_prev.val) / 4.0);  
 
//   if (fabs(fp_next.val) >= 4.0)
//     return bar;
//   double alpha_next = asin(fabs(fp_next.val) / 4.0);  

  //double dsina_ds = sign*(fabs(fp.val)/4.0 - fabs(fp_prev.val) / 4.0);
  //dsina_ds = dsina_ds / fp_prev.p.distanceToPt(fp.p);
  //double alpha_p = dsina_ds / cos(alpha);
  

  //double alpha_p = sign*(alpha_next - alpha_prev) / fp_prev.p.distanceToPt(fp_next.p);
  //double alpha_p = compute_dalpha_ds(sk, fp);
  

  //  bar = 2 * fabs(r * fabs(alpha_p) - sin(alpha));
  bar =  fabs(r * 2 * fabs(alpha_p) - 2*sin(alpha));

//   double k = compute_curvature(fpm1.p, fpm0.p, fpp1.p);
  
//   bar = fabs(fabs(fpm0.dist) * (fabs(alpha_p) + k) - sin(alpha)) +
//         fabs(fabs(fpm0.dist) * (fabs(alpha_p) - k) - sin(alpha));

  return bar;
}

///////// compute_branch_bar():
//
//
//
/////////
int compute_branch_bar(DDSkeleton *sk, FluxPointList &fpl, int bn =1)
{
  FluxPointList::iterator I=fpl.begin();

  // first smooth the flux values
  for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
    (*I).val = (0.3*(*I).val + 0.4*(*(I-1)).val + 0.3*(*(I+1)).val);

  }

  // initialize da/ds to 0 for all FluxPoint's
  for(I = fpl.begin(); I < fpl.end(); I++){
    (*I).alpha_p = 0.0;
  }


  // calculate initial da/ds
  //std::cerr << "b("<<bn<<")=[\n";
  int pti = 0;
  for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
    double alpha_p = compute_alpha_p(*(I-2), *(I-1), *I, *(I+1), *(I+2));
    (*I).alpha_p = fabs(alpha_p);

    // output for matlab: [x, y, val, dist, alpha_p]
    // where val = -4sin(alpha)
    FluxPoint fp = *I;

    pti++;
    std::cerr << "b("<<bn<<").fp("<<pti<<").x="<< fp.p.x << ";\n"; 
    std::cerr << "b("<<bn<<").fp("<<pti<<").y="<< fp.p.y << ";\n"; 
    std::cerr << "b("<<bn<<").fp("<<pti<<").val="<< fp.val << ";\n"; 
    std::cerr << "b("<<bn<<").fp("<<pti<<").dist="<< fp.dist << ";\n"; 
    std::cerr << "b("<<bn<<").fp("<<pti<<").alpha_p="<< fp.alpha_p << ";\n"; 

//     std::cerr << "[" << fp.p.x << ","<<fp.p.y<<",";
//     std::cerr << fp.val<<","<<fp.dist<<","<<fp.alpha_p << "]\n";
  }
   
  //  std::cerr << "]\n";


  // now smooth the da/ds values
//   for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
//     (*I).alpha_p = ((*I).alpha_p + (*(I-1)).alpha_p + (*(I+1)).alpha_p) / 3.0;

//   }


  // compute initial guess
  for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
    double bar = compute_bar(sk, *I); // *(I-2), *(I-1), *I, *(I+1), *(I+2));
    (*I).bar = bar;
  }

  // now smooth the values
//   for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
//     (*I).bar = ((*I).bar + (*(I-1)).bar + (*(I+1)).bar) / 3.0;

//   }

//   // now smooth the values
//   for(I = fpl.begin()+2; I+1 < fpl.end(); I++){
//     (*I).bar = ((*I).bar + (*(I-1)).bar + (*(I+1)).bar) / 3.0;

//   }

 return 0;
}



///////// create_ps_ligature_skeleton(DDSkeleton *, double)
//
//
//
/////////
void create_ps_ligature_skeleton(DDSkeleton *sk, double ligthr)
{
  DDSEdgeVect edges = sk->getEdges();
  DDSEdgeVect::iterator I;
  //Curve *c = sk->getShape()->getCurves()->front();
  DistanceTransform dt(sk->getShape());

  // draw the edges
  ps_setlinewidth(0.5);
  int branch_n = 1;
  for(I = edges.begin(); I != edges.end(); I++, branch_n++){
    DDSEdge *e = *I;
    FluxPointList fpl = e->getFluxPoints();
    FluxPointList::iterator II=fpl.begin();
    
    std::cerr << "branch " << branch_n << "/" << edges.size()<<": [";
    //    double step = 
    shift_FluxPoints(fpl, sk, false);
    std::cerr << "]\n";

    // compute boundary-to-axis ratio
    compute_branch_bar(sk, fpl, branch_n);

    ps_moveto((*II).p);
    ps_setrgbcolor(0,0,0);
    for(II = fpl.begin()+2; II+1 < fpl.end(); II++){

      
      //std::cerr << "bar: " << (*II).bar << "\n";
      if (fabs((*II).bar) < ligthr){
//  	ps_gsave();
//  	ps_draw_disc((*II).p);
//  	ps_grestore();
	ps_moveto((*II).p);
      }
      else
	ps_lineto((*II).p);

    } 
    ps_stroke();
  }  

 return;
}


//#define PI 3.1415
void ps_reconstruct_contour(DistanceTransform *dt,
			    FluxPoint fp0, FluxPoint fp1, FluxPoint fp2,
			    bool draw_lines=false)
{
  // the vector tangent to the skeleton
  Vector t(fp2.p.x - fp0.p.x, fp2.p.y - fp0.p.y);
  t.normalize();

  double Pi = 3.141592654;
  double alpha;

  if (fabs(fp1.val) > 4.1)
    return;

  if (fabs(fp1.val) > 4.0)
    alpha = Pi / 2.0;
  else
    alpha = asin(fabs(fp1.val) / 4.0);  

  // do not reconstruct sensitive points
  if(fabs(fp1.val) < 0.50) 
    return;

  // is the direction of the tangent good ?
  if (fabs(fp2.dist) > fabs(fp0.dist)){
    t.x = -t.x;
    t.y = -t.y;
//      alpha = PI - alpha;
  }
  
  // rotate the t by alpha
  t.normalize();
  t.rotate(alpha);
  
  // the reconstructed contour pt
  Point cp = fp1.p;
  cp.x += fabs(fp1.dist) * t.x;
  cp.y += fabs(fp1.dist) * t.y;
  
  if(!isnan(cp.x) && !isnan(cp.y)) 
    if(fabs(dt->getValue(cp)) < 1)
      {
	
	if(draw_lines){
	  ps_gsave();
	  ps_setlinewidth(0.1);
	  ps_draw_line(fp1.p, cp, 0,0,0);
	  ps_grestore();
	}
	ps_draw_disc(cp);
     }
    else
      {
	ps_gsave();
	ps_setrgbcolor(0.5, 0, 0);
	ps_draw_disc(cp);
	ps_draw_line(fp1.p, cp, 0,0,0);
	ps_grestore();
       }
  
  // the other side of the skeletal branch now
  t.rotate(-2*alpha); 
  cp = fp1.p;
  cp.x += fabs(fp1.dist) * t.x;
  cp.y += fabs(fp1.dist) * t.y;
  
  if(!isnan(cp.x) && !isnan(cp.y)) 
    if(fabs(dt->getValue(cp)) < 1.0)
      {
	if(draw_lines){
	  ps_gsave();
	  ps_setlinewidth(0.1);
	  ps_draw_line(fp1.p, cp, 0,0,0);
	  ps_grestore();
	}
	ps_draw_disc(cp);
      }
    else
      {
	ps_gsave();
	ps_setrgbcolor(0.5, 0, 0);
	ps_draw_disc(cp);
	ps_draw_line(fp1.p, cp, 0,0,0);
	ps_grestore();
      }

}

void create_ps_skeleton(DDSkeleton *sk, int freq, bool shift){
  DDSEdgeVect edges = sk->getEdges();
  DDSEdgeVect::iterator I;



  // draw the edges
  ps_setlinewidth(0.5);
  int branch_n = 1;
  for(I = edges.begin(); I != edges.end(); I++, branch_n++){
    DDSEdge *e = *I;
    FluxPointList fpl = e->getFluxPoints();
    FluxPointList::iterator II=fpl.begin();
    
    if(shift){
      std::cerr << "branch " << branch_n << "/" << edges.size()<<": [";
      shift_FluxPoints(fpl, sk);
      std::cerr << "]\n";
    }
    ps_moveto((*II).p);
    ps_setrgbcolor(0,0,0);
    for(II = fpl.begin()+1; II != fpl.end(); II++){
      if(shift)
	ps_lineto((*II).p);
      else
	ps_digipt((*II).p);
      //double ang = 180.0 / 3.141592654 * asin(fabs((*II).val) / 4.0);
      //std::cerr << (*II).val << " " << PI << " " << ang << "\n";
    } 
    ps_stroke();

    int pc = 0;
    for(II = fpl.begin()+2; II < (fpl.end()-2); II++, pc++){
      bool draw_line = false;
      
      if(freq > 0)
	if((pc % freq) == 0)
	  draw_line = true;

      if(freq != 0)
	ps_reconstruct_contour(sk->getDivergenceMap()->getDT(), 
			       *(II-1), *II, *(II+1), draw_line);
    }
  }
}

void ps_draw_contour(DDSkeleton *sk)
{
  // now draw the contour
  Curve *c = sk->getShape()->getCurves()->front();
  const double step = 1;
  double t=0;


  std::cout << "/contour {";
  ps_moveto(c->atT(0));
  for(t = 0; t < c->getLength()-2*step; t += step){
    ps_lineto(c->atT(t));
    //ps_draw_line(c->atT(t), c->atT(t+step));
  }
  ps_lineto(c->atT(0));
  std::cout << "} def\n";

  ps_setlinewidth(0.1);
  std::cout << "contour \n";
  ps_stroke();

}

void create_ps_gradient_map(DDSkeleton *sk, int frequency, bool shift)
{
  DDSEdgeVect edges = sk->getEdges();
  DDSEdgeVect::iterator I;
  Curve *c = sk->getShape()->getCurves()->front();
  DistanceTransform dt(sk->getShape());

  // draw the edges
  ps_setlinewidth(0.5);
  int branch_n = 1;
  for(I = edges.begin(); I != edges.end(); I++, branch_n++){
    DDSEdge *e = *I;
    FluxPointList fpl = e->getFluxPoints();
    FluxPointList::iterator II=fpl.begin();

    //if(shift){
    std::cerr << "branch " << branch_n << "/" << edges.size()<<": [";
    double step = shift_FluxPoints(fpl, sk, false);
    std::cerr << "]\n";
      //}
    ps_moveto((*II).p);
    ps_setrgbcolor(0,0,0);
    for(II = fpl.begin()+1; II != fpl.end(); II++){
      ps_lineto((*II).p);

      //double ang = 180.0 / 3.141592654 * asin(fabs((*II).val) / 4.0);
      //std::cerr << (*II).val << " " << PI << " " << ang << "\n";
    } 
    ps_stroke();

    int pc = 0;
    for(II = fpl.begin()+2; II < (fpl.end()-2); II++, pc++){
      
      if((pc % frequency) == 0){
	
	Point p = (*II).p;
	Distance d = c->computeDistance(p);

	ps_gsave();
	ps_setlinewidth(0.1);
	ps_arrow_line(d.p, p, 0);

	Vector v = dt.getGradValue(p, 0.00001);
	v.scale(step);
	d = c->computeDistance(Point(p.x+v.x, p.y+v.y));

	ps_arrow_line(d.p, p, 0);
	ps_grestore();
      }

    }
  }

}

void shift_FluxPoints_old(FluxPointList &fpl, DDSkeleton *sk)
{
  double alpha_thresh = cos(PI/40.0);

  if(fpl.size() < 5)
    return;
  const DivergenceMap *dm = sk->getDivergenceMap();
  DistanceTransform *dt = dm->getDT();

  
  FluxPointList::iterator I = fpl.begin();
  
  double step;
  FluxPoint fp1 = *I;     //fpl.begin();
  FluxPoint fp2 = *(I+1); //fpl.begin()+1;

  // compute step by taking two neighbouring points
  // NOTE: consecutive points in flp must be 8-nbrs
  if(fp1.p.x != fp2.p.x){
    step = fabs(fp1.p.x - fp2.p.x);
  }
  else
    step = fabs(fp1.p.y - fp2.p.y);


  // go through all points in the vector
  std::cerr << "branch: [";
  for (I = fpl.begin(); I < (fpl.end()); I++){
    FluxPoint fp = *I;
    
    // we must shift along the gradient line through fp
    Vector t = dt->getGradValue(fp.p, step / 10000000.0);
    t.normalize();
    double seg_half_length = 1*step;

    Point pl; // the "left" point
    pl.x = fp.p.x - seg_half_length*t.x;
    pl.y = fp.p.y - seg_half_length*t.y;
    
    Point pr; // the "left" point
    pr.x = fp.p.x + seg_half_length*t.x;
    pr.y = fp.p.y + seg_half_length*t.y;

    Vector gpl = dt->getGradValue(pl, step / 10000000.0);
    gpl.normalize();

    Vector gpr = dt->getGradValue(pr, step / 10000000.0);
    gpr.normalize();

    bool err = false;
    if ((t.dot(gpl)) > alpha_thresh){ // t || gpl
      pl = pl;
      
      //std::cerr << "t . gpr = " << t.dot(gpr) << "\n";
      if((t.dot(gpr)) > alpha_thresh){
	std::cerr << "SOMETHING is WRONG\n";
	std::cerr << "(t . gpr) = " << (t.dot(gpr)) << "\n";
	std::cerr << "(t . gpl) = " << (t.dot(gpl)) << "\n\n";
	
	err = true;
      }
      
    }
    else{ // switch to make t || gpl
      if((t.dot(gpr)) < alpha_thresh){
	std::cerr << "SOMETHING is WRONG\n";
	std::cerr << "(t . gpr) = " << (t.dot(gpr)) << "\n";
	std::cerr << "(t . gpl) = " << (t.dot(gpl)) << "\n\n";
	
	err = true;
      }


      Point tmpp = pr;
      pr = pl;
      pl = tmpp;
    }
    // hence, pl's gradient value is the same as t

    
    Point mp;

    int num_iter = 30;
    for (int i=0; i < num_iter && !err; i++){
      mp.x = (pl.x+pr.x)/2.0;
      mp.y = (pl.y+pr.y)/2.0; // the midpoint
      
      Vector gmp = dt->getGradValue(mp, step / 100000.0);
      gmp.normalize();
      
      if (isnan(gmp.x) || isnan(gmp.y))
	{
	  std::cerr << step << " found gmp to be NaN\n";
	  err = true;
	  continue;
	}
//        gpl = dt->getGradValue(pl, step / 10000000.0);
//        gpl.normalize();
      
      gpr = dt->getGradValue(pr, step / 100000.0);
      gpr.normalize();
      
      if((t.dot(gpr)) > alpha_thresh){
	std::cerr << "Inside ERROROROROR "<< gpl.dot(gpr)<<"\n";
	err = true;
	
      }
      
      if((gmp.dot(t)) > alpha_thresh){ // i.e. gmp and t are ||
	pl = mp;
      }
      else //if((gmp.dot(gpr)) < alpha_thresh)
	pr = mp;
      
      //std::cerr << "pl-to-pr: " << pl.distanceToPt(pr) <<"\n";
      
    } // done bisecting
    
    if(err == false){
      (*I).p = mp;
      (*I).val = compute_flux((*I).p, dt, 
			      step, //3*pl.distanceToPt(pr), 
			      50);
    }
    
    std::cerr << ".";
  }

  std::cerr << "\n";


  delete dt;
}



double compute_flux_old(Point p, DistanceTransform *dt, double eps, int np){
  double f = 0.0;

  double x = p.x, y = p.y;
  
  double Pi_n = 2.0 * 3.141592654 / (double) np;
  for(int i = 0; i < np; i++){
    
    double xx,yy;
    
    xx = cos(((double)i)*Pi_n);
    yy = sin(((double)i)*Pi_n);

    Vector N;
    N.x = xx; N.y = yy; //(xx, yy);
    
    xx = x + eps*xx;
    yy = y + eps*yy;

    Vector v = dt->getGradValue(xx, yy, 0.00001*eps);

    f += N.dot(v);
  }

  
  return Pi_n * f;
}

// returns the grid step
double shift_FluxPoints(FluxPointList &fpl, DDSkeleton *sk, 
			bool update_flux)
{
  double alpha_thresh = cos(PI/40.0);

  const DivergenceMap *dm = sk->getDivergenceMap();
  DistanceTransform *dt = dm->getDT();

  
  FluxPointList::iterator I = fpl.begin();
  
  double step;
  FluxPoint fp1 = *I;     //fpl.begin();
  FluxPoint fp2 = *(I+1); //fpl.begin()+1;

  // compute step by taking two neighbouring points
  // NOTE: consecutive points in flp must be 8-nbrs
  if(fp1.p.x != fp2.p.x){
    step = fabs(fp1.p.x - fp2.p.x);
  }
  else
    step = fabs(fp1.p.y - fp2.p.y);

  if(fpl.size() < 5)
    return step;

  // go through all points in the vector
  for (I = fpl.begin(); I < (fpl.end()); I++){
    FluxPoint fp = *I;
    
    // we must shift along the gradient line through fp
    Vector t = dt->getGradValue(fp.p, step / 10000000.0);
    t.normalize();
    double seg_half_length = 5*step;

    Point pl; // the "left" point
    pl.x = fp.p.x;// - seg_half_length*t.x;
    pl.y = fp.p.y;// - seg_half_length*t.y;
    
    Point pr; // the "right" point
    pr.x = fp.p.x + seg_half_length*t.x;
    pr.y = fp.p.y + seg_half_length*t.y;

    Vector gpl = dt->getGradValue(pl, step / 10000000.0);
    gpl.normalize();

    Vector gpr = dt->getGradValue(pr, step / 10000000.0);
    gpr.normalize();

    bool err = false;

    if(isnan(gpr.x) || isnan(gpr.y) ||
       isnan(gpl.x) || isnan(gpl.y))
      err = true;

    if ((t.dot(gpl)) > alpha_thresh){ // t || gpl
      pl = pl;
      
      //std::cerr << "t . gpr = " << t.dot(gpr) << "\n";

      if((t.dot(gpr)) > alpha_thresh){
	std::cerr << "SOMETHING is WRONG\n";
	std::cerr << "(t . gpr) = " << (t.dot(gpr)) << "\n";
	std::cerr << "(t . gpl) = " << (t.dot(gpl)) << "\n\n";
	
	err = true;
      }

    }
    else{ // switch to make t || gpl
      if((t.dot(gpr)) < alpha_thresh){
	std::cerr << "SOMETHING is WRONG\n";
	std::cerr << "(t . gpr) = " << (t.dot(gpr)) << "\n";
	std::cerr << "(t . gpl) = " << (t.dot(gpl)) << "\n\n";
	
	err = true;
      }


      Point tmpp = pr;
      pr = pl;
      pl = tmpp;
    }
    // hence, pl's gradient value is the same as t

    
    Point mp;

    int num_iter = 20;
    for (int i=0; i < num_iter && !err; i++){
      mp.x = (pl.x+pr.x)/2.0;
      mp.y = (pl.y+pr.y)/2.0; // the midpoint
      
      Vector gmp = dt->getGradValue(mp, step / 10000000.0);
      gmp.normalize();
      
//        gpl = dt->getGradValue(pl, step / 10000000.0);
//        gpl.normalize();
      
      gpr = dt->getGradValue(pr, step / 10000000.0);
      gpr.normalize();
      
      if((t.dot(gpr)) > alpha_thresh){
	std::cerr << "Inside ERROROROROR: gpl.gpr="<< gpl.dot(gpr)<<"\n";
	err = true;
	
      }
      
      if((gmp.dot(t)) > alpha_thresh){ // i.e. gmp and t are ||
	pl = mp;
      }
      else //if((gmp.dot(gpr)) < alpha_thresh)
	pr = mp;
      
      //std::cerr << "pl-to-pr: " << pl.distanceToPt(pr) <<"\n";
      
    } // done bisecting
    
    
    
    if(err == false){
      (*I).p = mp;
      (*I).dist = dt->getValue(mp);
      if(update_flux)
	(*I).val = compute_flux((*I).p, dt, 
				step/10000.0, //1000*pl.distanceToPt(pr), 
				50);
    }
    std::cerr <<".";
  }


  delete dt;

  return step;
}




double compute_flux(Point p, DistanceTransform *dt, double eps, int np){
  double f = 0.0;

  double x = p.x, y = p.y;
  
  double Pi_n = 2.0 * 3.141592654 / (double) np;
  for(int i = 0; i < np; i++){
    
    double xx,yy;
    
    xx = cos(((double)i)*Pi_n);
    yy = sin(((double)i)*Pi_n);

    Vector N;
    N.x = xx; N.y = yy; //(xx, yy);
    
    xx = x + eps*xx;
    yy = y + eps*yy;

    Vector v = dt->getGradValue(xx, yy, 0.001*eps);

    f += N.dot(v);
  }

  
  return Pi_n * f;
}

