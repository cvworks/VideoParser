#include <sg.h>
#include <LEDA/graphwin.h>
#include <LEDA/graph.h> 
#include <LEDA/node_array.h>
#include <stdio.h>


extern "C" {
	#include <ppm.h>
}

char* get_arr(int *rows, int *cols, char shapefile[]){

  char *arr;
  int k, i, j;
  
  pixval maxval;
  pixel **pixels;

  FILE* ppmshape = fopen(shapefile, "r");  

  pixels = ppm_readppm(ppmshape, cols, rows, &maxval);

  arr = (char*)malloc((*cols)*(*rows));


  for(i = 0, k = 0; i < *rows; i++)
    for(j = 0; j < *cols; j++, k++)
      {
	char r, g, b;
	
	r = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	g = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	b = (0xff * PPM_GETR(pixels[i][j])) / maxval;
	
	if(r==0 && b==0 && g==0)
	  arr[k] = 0;
	else
	  arr[k] = 0xff;
      }

  ppm_freearray(pixels, *rows);

  
  return arr;
}

//char* get_arr(int *rows, int *cols, char shapefile[]);

int main(int argc, char** argv) {

  using namespace sg;

  int i, j, rows=200, cols=200;

  if(argc <= 1){
    std::cerr << "\nError: What's the main ppmfile?\n";
    
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
  DDSkeleton *skeleton = dsm.getDiscreteDivergenceSkeleton(1.0/mag, thresh); 

  /*std::vector<DiscreteDivergenceSkeletonEdge *> edges;
  std::vector<DiscreteDivergenceSkeletonNode *> nodes;

  edges = skeleton->getEdges();
  nodes = skeleton->getNodes();*/

  cerr << endl << "DONE!!! Ready to display graph" << endl;

  leda_graph g;
  leda_node n1,n2;


  n1 = g.new_node();
  n2 = g.new_node();
  g.new_edge(n1,n2);
  
  int nWinWidth = 300;

  GraphWin gw(g,"Test Graph");
  gw.win_init(0, nWinWidth, 0);
  
  //gw.get_window();
  //gw.place_into_win();
  gw.display();
  gw.edit();

  //while (gw.get_window().read_event());

  return 0;
}
