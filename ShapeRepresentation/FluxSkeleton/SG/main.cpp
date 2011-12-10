#include <sg.h>
#include <LEDA/graphwin.h>
#include <LEDA/graph.h> 
#include <LEDA/node_array.h>
#include <stdio.h>
#include <vector>
#include <iostream>

#include <DAG.h>
#include <SGNode.h>
#include <ShockGraph.h>
#include <VisualDAG.h>

extern "C"
{
      #include <ppm.h>
}

using namespace sg;

// test code from McGill
char* get_arr(int *rows, int *cols, char shapefile[])
{
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
};

/*********************************MAIN**********************************/

int main(int argc, char** argv)
{
	using namespace sg;

	// do the skeleton work, Alex's code
	cout << endl << "Creating shock points..." << endl;

	int i, j, rows=200, cols=200;

	if(argc <= 1)
	{
		std::cerr << "\nError: What's the main ppmfile?\n";
		exit(1);
	}

	char *a = get_arr(&rows, &cols, argv[1]);  

	// first, we must make a Shape
	sg::SimpleShapeMaker ssm(cols, rows);

	// update the SimpleShape
	for (i=0; i < rows; i++)
		for (j=0; j < cols; j++)
			ssm(j,i) = a[i*cols + j];

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

/*************************************NEW CODE********************************/

	// analyse the points, create the LEDA graph (SG)
	cout << endl << "Generating the shock graph..." << endl;

	// stores the shock points
	std::vector<DiscreteDivergenceSkeletonNode*> nodes;
	std::vector<DiscreteDivergenceSkeletonEdge*> edges;

	nodes = skeleton->getNodes();	// get all nodes
	edges = skeleton->getEdges();

	DiscreteDivergenceSkeletonEdge* SKparent;

       	leda_d_array<long, leda_node> map;
	SGNode* pNode, *SGparent;

	SGparent = pNode;
	SKparent = edges[0];

	ShockGraph* g = new ShockGraph;

	// make some LEDA nodes
	for ( int i=0; i<edges.size(); i++ )
	  {
	    long pointerID = (long)(edges[i]);
	    pNode = new SGNode("UNKNOWN",0);
	    FluxPointList fpl = (edges[i])->getFluxPoints();
	    int num_pts = fpl.size();

	    pNode->m_shocks.Resize((num_pts>0) ? num_pts:0);
	    for ( int j=0; j<num_pts; j++ )
	      {
		FluxPoint pt = fpl[j];
		ShockInfo& si = pNode->m_shocks[j];
		si.xcoord = pt.p.x;
		si.ycoord = pt.p.x;
		si.radius = pt.dist; // guessing here
		si.speed  = pt.val;  // guessing here too
	      }

	    pNode->ComputeDerivedValues();
	    cout << "Node #" << i << " has " << pNode->GetShockCount() << " points and average radius " << pNode->m_shocks.AvgRadius() << endl;

	    // check if it's the new parent
	    if ( pNode->m_shocks.AvgRadius() < SGparent->m_shocks.AvgRadius() )
	      {
		SGparent = pNode;
		SKparent = edges[i];
	      }

	    map[pointerID] = g->NewNode(pNode);
     	  }

	DiscreteDivergenceSkeletonNode* node;
	std::vector<DiscreteDivergenceSkeletonEdge*> l_edges;

	// this needs to be recursive so that edges can be generated below the first level.
	node = SKparent->getN1();
	if ( node->getEdges().size() > 1 )
	  {
	    l_edges = node->getEdges();
	    for ( int j=0; j<l_edges.size(); j++ )
	      {
		if ( SKparent != l_edges[j] )
		  g->NewEdge( map[(long)SKparent], map[(long)l_edges[j]], 1.0);
	      }
	  }
	node = SKparent->getN2();
	if ( node->getEdges().size() > 1 )
	  {
	    l_edges = node->getEdges();
	    for ( int j=0; j<l_edges.size(); j++ )
	      {
		if ( SKparent != l_edges[j] )
		  g->NewEdge( map[(long)SKparent], map[(long)l_edges[j]], 1.0);
	      }
	  }

	// use ShowGraph
	//	ShowGraph(g);
	
      	// display the graph, modified from Alex's code
	cout << endl << "Displaying the shock graph..." << endl;
	int nWinWidth = 300;
	GraphWin gw(*g,"Test Graph");
	gw.win_init(0, nWinWidth, 0);

	gw.display();
	gw.edit();
	
	delete g;

	return 0;
}
