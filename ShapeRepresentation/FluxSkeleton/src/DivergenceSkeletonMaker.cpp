/**************************************************************************
File:                DivergenceSkeletonMaker.cpp

Author(s):           Pavel Dimitrov

Created:             27 Jun 2002

Last Revision:       Date: 2009/10/14 by Diego Macrini
**************************************************************************/
#include "defines.h"
#include "DivergenceSkeletonMaker.h"
#include "DDSGraph.h"
#include <vector>
#include <algorithm>

#include "DDSMakerUtils.h"

using namespace sg;

void create_branch(int x, int y, int nextx, int nexty,
				   std::vector<DivPtVector*> &branches, 
				   DivArr &da);

/*!
	Creates the discrete array that will be used for the thinning
*/
DivArr* sg::create_shape_DivArr(DivergenceMap& dm, const double& step)
{
	DistanceTransform* dt = dm.getMemberDT();

	double xmin, xmax, ymin, ymax;
	const double bdry = 0;//2;

	dm.getShapeBounds(&xmin, &xmax, &ymin, &ymax);

	ymin -= bdry;
	xmin -= bdry;
	ymax += bdry;
	xmax += bdry;

	int xs, ys; // exact number of pts computed
	Point pt;

	// Find out how many pts will be computed
	for (ys = 0, pt.y = ymin; pt.y < ymax; pt.y += step, ys++);

	for (xs = 0, pt.x = xmin; pt.x < xmax; pt.x += step, xs++);

	// Reserve the space for all the pts
	DivArr* da = new DivArr(xs, ys);

	DivArr::iterator it = da->begin();

	double dist;
	int xIdx, yIdx;

	// Compute the distances for all the points
	for (yIdx = 0, pt.y = ymin; pt.y < ymax; pt.y += step, yIdx++)
	{
		for (xIdx = 0, pt.x = xmin; pt.x < xmax; pt.x += step, xIdx++, it++)
		{
			dist = dt->getValue(pt);

			if (dist > 0) // blank all points outside the shape
			{
				it->Set(pt, DIV_MAP_MAX_VAL, dist, BACKGROUND_COL);
			}
			else // only points inside the shape are considered
			{ 
				it->Set(pt, dm.getValue(pt, step), dist, FOREGROUND_COL);
			}

			it->SetOptInfo(xIdx, yIdx, false);
		}
	}

	return da;
}

/*!
	 Thin the discrete sampling of the plane (divergence array) to
	 obtain an approximation of the skeleton. The process begins by
	 putting all contour points (that is, contour in the array) in a
	 heap where the key is the divergence value. It is assumed that the
	 lower (more negative) the value the more likely it is for the point
	 to be on the skeleton. Hence, we start removing (i.e. blanking)
	 points on the divergence array with highest divergence value; that
	 it, on the top of the heap. It may be that the point cannot be
	 removed. This occurs only if the point satisfies the criterion for
	 being an end-point or if removig the point would disconnect the
	 skeleton. An end point is defined as one that has divergence value
	 below the threshold thresh and has eihter a single neighbour
	 (non-blank, that is) or it has exactly two neighbours and its
	 8-neighbourhood has exactly two connected regions -- blank and
	 non-blank. Removing a point disconnects the skeleton if the graph
	 of non-blank 8-neighbours of it do not form a tree.

	 If a point is "removable," we push its removable neighbours into
	 the heap. Thus, the algorithm goes around the shape peeling layers
	 according to the divergence values. Its running time is
	 less than O(n log n) where n is the number of pixels in the
	 shape. (In fact, it is O(n log m) where m is the maximal size of
	 the heap, which, of course, is m<n.)
*/
#ifdef USE_NEW_THINNING
void sg::thin_div_arr(DivArr& da, double thresh)
{
	// Reserve space equal to the whole image
	PriorityQueueDP the_heap(da.getXSize() * da.getYSize());

	DivArr::iterator it;

	// Start by pushing all contour points
	for (it = da.begin(); it != da.end(); ++it)
		if (is_on_contour(*it, da))
			the_heap.push_back(it);

	// Make the heap using the current points
	the_heap.make();

	DivPt* p;

	while(!the_heap.empty())
	{
		// get point from heap
		p = the_heap.top();
		the_heap.pop();

		// decide wheather to blank it or not
		if (is_end_pt(p->x, p->y, da, thresh)) // an end_point anchors the skeleton
		{
			p->col = SKELETON_COL;
		}
		else if (p->col != BACKGROUND_COL && is_removable(*p, da))
		{
			// Ensure that the current point won't be 
			// added to the heap again
			p->col = BACKGROUND_COL;

			// Add 8-con neighbours to the heap if their color is
			// FOREGROUND_COL and they are "removable".
			// 8-connected order:
			// 8 1 2
			// 7 P 3
			// 6 5 4
			push_nbr_if(p->x, p->y+1,   da, the_heap);  // 1
			push_nbr_if(p->x+1, p->y+1, da, the_heap);  // 2
			push_nbr_if(p->x+1, p->y,   da, the_heap);  // 3
			push_nbr_if(p->x+1, p->y-1, da, the_heap);  // 4
			push_nbr_if(p->x, p->y-1,   da, the_heap);  // 5
			push_nbr_if(p->x-1, p->y-1, da, the_heap);  // 6
			push_nbr_if(p->x-1, p->y,   da, the_heap);  // 7
			push_nbr_if(p->x-1, p->y+1, da, the_heap);  // 8
		}
	}	
}
#else // USE_NEW_THINNING
void sg::thin_div_arr(DivArr& da, double thresh)
{
	// Reserve space equal to the whole image
	PriorityQueueDP the_heap(da.getXSize() * da.getYSize());

	DivArr::iterator it;

	// Start by pushing all contour points
	for (it = da.begin(); it != da.end(); ++it)
		if (is_on_contour(*it, da))
			the_heap.push_back(*it);

	// Make the heap using the current points
	the_heap.make();

	DivPt dp;

	while(!the_heap.empty())
	{
		// get point from heap
		dp = the_heap.top();
		the_heap.pop();

		// decide wheather to blank it or not
		if (is_end_pt(dp.x, dp.y, da, thresh)) // an end_point anchors the skeleton
		{
			da(dp.x, dp.y).col = SKELETON_COL;
		}
		else if (is_removable(dp, da))
		{
			// Manage only not previously erased points
			if (da(dp.x, dp.y).col != BACKGROUND_COL)
			{
				// Ensure that the current point won't be 
				// added to the heap again
				da(dp.x, dp.y).col = BACKGROUND_COL;

				// Add 8-con neighbours to the heap if their color is
				// FOREGROUND_COL and they are "removable".
				// 8-connected order:
				// 8 1 2
				// 7 P 3
				// 6 5 4
				push_nbr_if(dp.x, dp.y+1,   da, the_heap);  // 1
				push_nbr_if(dp.x+1, dp.y+1, da, the_heap);  // 2
				push_nbr_if(dp.x+1, dp.y,   da, the_heap);  // 3
				push_nbr_if(dp.x+1, dp.y-1, da, the_heap);  // 4
				push_nbr_if(dp.x, dp.y-1,   da, the_heap);  // 5
				push_nbr_if(dp.x-1, dp.y-1, da, the_heap);  // 6
				push_nbr_if(dp.x-1, dp.y,   da, the_heap);  // 7
				push_nbr_if(dp.x-1, dp.y+1, da, the_heap);  // 8
			}
		}
	}
}
#endif // USE_NEW_THINNING

/*!
	colour all points into one of end_pt|branching_pt|skeleton_pt
*/
void sg::colour_skeleton_array(DivArr &da)
{
	int x, y, nints;

	for (y = 0; y < da.getYSize(); y++)
	{
		for(x = 0; x < da.getXSize(); x++) 
		{
			if (da(x,y).col != BACKGROUND_COL) 
			{
				nints = num_of_intersections(x, y, da);

				if(nints == 1)
					da(x,y).col = END_POINT_COL;
				else if(nints == 2)
					da(x,y).col = SK_POINT_COL;
				else
					da(x,y).col = BRANCHING_POINT_COL;
			}
		}
	}

	// testing
#ifdef DEBUG_DIVERGENCE_SKELETON_MAKER
	if(0)
	{
		double scale = 20;
		int maxval = 255;
		std::cout << "P3\n" << da.getXSize() << " " << da.getYSize() << "\n";
		std::cout << maxval << "\n";

		for(y=0; y<da.getYSize(); y++)
		{
			for(x=0; x<da.getXSize(); x++)
			{
				int v = (int)(da(x,y).val*scale);

				switch(da(x,y).col)
				{
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
						std::cout << " 100 100 100 ";
						break;
				}
			}
		}
	}
#endif
}

/*!
	Makes the skeleton.
*/
void sg::buildDDSGraph(DivArr &da, DDSGraph* skeleton)
{
	int i = 0, x = 0, y = 0;

	const int xs = da.getXSize();
	const int ys = da.getYSize();
	const int sz = xs * ys;

	// find the initial branch-end-point
	while (i < sz && !(da(x,y).col == END_POINT_COL))
	{
		x = i % xs;
		y = i / xs;
		i++;
	}

	// now, create a preliminary branch list
	std::vector<DivPtVector*> branches; // <- here

	da(x,y).visited = true;
	//std::cerr << "got " << branches.size() << " branches\n";

	// find the next point
	int vx, vy;

	for (i = 0; i < 8; i++)
	{
		vx = x + nbrs[i][0];
		vy = y + nbrs[i][1];

		if (da.checkLimits(vx, vy) && da(vx,vy).col != BACKGROUND_COL)
			break;
	}

	// make the branches
	create_branch(x, y, vx, vy, branches, da);

	// now, make the DDSGraph
	//std::cerr << "making DDSGraph\n";
	//  DBG_MSG1("Number of branches in the graph: ",branches.size());

	std::vector<DDSNode*> nodes;
	std::vector<DDSNode*>::iterator NodeI;
	DDSNode* n;
	
	// begin by creating the nodes
	std::vector<DivPtVector*>::iterator I;
	DivPtVector* dpv;
	DivPt dp;
	bool dp_is_new;

	for(I = branches.begin(); I != branches.end(); ++I)
	{
		dpv = *I;

		// try the first branch-end-point
		dp = dpv->front();
		dp_is_new = true;

		if(nodes.size() > 0)
			for(NodeI = nodes.begin(); NodeI != nodes.end(); NodeI++)
			{
				n = *NodeI;

				if(n != 0)
					if(dp.p == n->getPoint())
					{
						dp_is_new = false;   

						if(dpv->n1 == 0)
							dpv->n1 = n;
						else if(dpv->n2 == 0)
							dpv->n2 = n;
						else
							ShowError("More than two nodes per branch");

						break;
					}
			}

			if(dp_is_new)
			{
				FluxPoint fp(dp.p, dp.val, dp.dist, dp.col);
				n = new DDSNode(fp);
				nodes.push_back(n);

				if(dpv->n1 == 0)
					dpv->n1 = n;
				else if(dpv->n2 == 0)
					dpv->n2 = n;
				else
					ShowError("More than two nodes per branch");
			}

			// and the other branch-end-point
			dp = dpv->back();
			dp_is_new = true;

			if (nodes.size() > 0)

				for(NodeI = nodes.begin(); NodeI!=nodes.end(); NodeI++)
				{
					n = *NodeI;

					if(n != 0)
					{
						if(dp.p == n->getPoint())
						{
							dp_is_new = false;     

							if(dpv->n1 == 0)
								dpv->n1 = n;
							else if(dpv->n2 == 0)
								dpv->n2 = n;
							else
								ShowError("More than two nodes per branch");

							break;
						}
					}
				}

				if(dp_is_new)
				{
					FluxPoint fp(dp.p, dp.val, dp.dist, dp.col);
					n = new DDSNode(fp);
					nodes.push_back(n);

					if(dpv->n1 == 0)
						dpv->n1 = n;
					else if(dpv->n2 == 0)
						dpv->n2 = n;
					else
						ShowError("More than two nodes per branch");
				}

	}

	ASSERT(skeleton->empty());

	for(NodeI = nodes.begin(); NodeI!=nodes.end(); ++NodeI)
	{
		if(*NodeI != 0)
			skeleton->addNode(*NodeI);
	}

	std::vector<DivPt>::const_iterator dpIt;
	DDSEdge* e;

	// now make and insert the edges
	for(I = branches.begin(); I != branches.end(); ++I)
	{
		dpv = *I;

		if(dpv->n1 == 0 || dpv->n2 == 0)
			ShowError("Branch does not have two branch-end-points");
		
		e = new DDSEdge(skeleton,
			dpv->n1, dpv->n2);

		FluxPointArray& fpl = e->flux_points;

		fpl.reserve(dpv->size());

		for(dpIt = dpv->begin(); dpIt != dpv->end(); ++dpIt)
		{
			fpl.push_back(FluxPoint(dpIt->p, dpIt->val, dpIt->dist, dpIt->col));
		}
		
		skeleton->addEdge(e);
	}

	// finally, destroy the preliminary list
	destroy_preliminary_branch_list(branches);
}

/*!
	 Recursive. 

	 Traverses the thinned divergence array and creates the
	 skeleton-braches. Pass an empty vector, to indicate the beginning
	 of the traversal. The array is assumed to be properly initialized;
	 that is, at the beginning all skeletal points are not da().visited.

	 This assumes that each strictly skeletal point (i.e. not a branching
	 point and not an end-point) has at most one neighbour which is a
	 branching point or an end-point. Also, there must be more than a
	 single skeletal point for this function to work.

	 Here is the algorithm. The function creates branches
	 recursively. It needs an anchor point and a direction point. It
	 starts creating the branch from the anchor point toward the
	 direction point. If it reaches an end point, it finishes the
	 current branch and returns. If it reaches a branching point, it
	 finishes the current branch, looks at branching-point's
	 neioghbourhood and finds all unvisited nbrs. It marks them as
	 visited and calls itself on each pair (branch-pt, nbr). The
	 blanking of the nbrs is important since it is possible that some
	 nbrs of a point are also nbrs among them; hence, marking those as
	 visited prevents a branch from flowing into another branch.

	 This algorithm does not care about the one degenerate case of the
	 divergence thinning -- the creation of a square of branching
	 points. In such a case, it would simply enter the square from one
	 of the branching points creating branches from it to the other
	 three. After that, the branch creation flows away from the square
	 departing from each of the other three branching points. If so desired,
	 one could post-process the output of this function to collapse such
	 squares into a single branching point...
*/
void create_branch(int x, int y, int nextx, int nexty,
				   std::vector<DivPtVector*> &branches, 
				   DivArr &da)
{
	DivPtVector* dpv = new DivPtVector;

	// minimal error-check
	if(da(x,y).col == BACKGROUND_COL){

		std::cerr << "error: create_branch() in DivergenceSkeletonMaker\n";
		std::cerr << "error: if(da(x,y).col == BACKGROUND_COL)\n";

		return;   // something is very wrong if this happens
	}

	// insert the first point into the branch
	dpv->push_back(da(x,y));

	bool found_nbr = true;

	if(x != nextx || y != nexty)
	{
		da(nextx, nexty).visited = true;
		dpv->push_back(da(nextx,nexty));

		if(da(nextx,nexty).col == END_POINT_COL)
		{
			found_nbr = false;
			branches.push_back(dpv); // insert new branch
		}

		if(da(nextx,nexty).col == BRANCHING_POINT_COL) 
		{
			found_nbr = false; // a short branch -- don't look for a new nbr
			branches.push_back(dpv); // insert new branch

			int nx = nextx, ny = nexty;
			int i;

			//! BEGIN: Matthijs 14-04-2006
			//we should not claim a branch as ours, if one of our
			//nbrs is a branching point and the distance
			//from that branching point to the point we want
			//to claim is smaller
			//so lets first check whether we have a branching
			//point as nbr
			bool bBrPntAsNbr = false;
			int nPosX = 0, nPosY = 0;
			for(i = 0; i < 8; i++)
			{
				int vx = nx + nbrs[i][0];
				int vy = ny + nbrs[i][1];
				if(da(vx,vy).col == BRANCHING_POINT_COL)
				{
					if((abs(nx-vx) + abs(ny-vy)) == 1)
					{
						nPosX = vx;
						nPosY = vy;
						bBrPntAsNbr = true;
					}
				}
			}
			//! END: Matthijs 14-04-2006

			// first, find the appropriate neighbours and claim them
			// for different branches by marking them visited
			int num_nbrs = 0;  // number of nbrs in nextpts
			int nextpts[8][2]; // there are never 8 or more

			for(i=0; i<8; i++)
			{	
				int vx = nx + nbrs[i][0];
				int vy = ny + nbrs[i][1];

				if((da(vx,vy).visited == false && // only go to new places
					da(vx,vy).col != BACKGROUND_COL)){ // foreground
						// point 

						//! BEGIN: Matthijs 14-04-2006
						//skip a skelpointnbr if we have a branching point as nbr
						//which is closer to this skelpointnbr
						if(bBrPntAsNbr && 
							( (abs(nPosX-vx) + abs(nPosY-vy)) < 
							(abs(nx-vx) + abs(ny-vy)) ) &&
							da(vx,vy).col == SK_POINT_COL)
						{
							continue;
						}
						//! END: Matthijs 14-04-2006

						da(vx,vy).visited = true; // so that a nearby branch
						// does not get confused 

						nextpts[num_nbrs][0] = vx;
						nextpts[num_nbrs][1] = vy;
						num_nbrs++;		
				}
			}

			// then, recursively build the branches supplying as next
			// points the neighbours found above
			for(i=0; i<num_nbrs; i++){
				create_branch(nx, ny, 
					nextpts[i][0], // next x
					nextpts[i][1], // next y
					branches, da); 
			}
		}
	}  //endif(x!=nextx || y!=nexty)

	int xprev = x, yprev = y;
	x = nextx; y = nexty;

	while(found_nbr)
	{
		//! BEGIN: Matthijs 14-04-2006
		bool bSPECIALCASE = false;
		//! END: Matthijs 14-04-2006

		found_nbr = false;

		da(x,y).visited = true;

		int nx = -1;
		int ny = -1;
		int i;

		//! BEGIN: Matthijs 14-04-2006
		bool bBranchNbr = false;
		bool bTwoBranchNbr = false;
		bool bEndNbr = false;
		int nEndPos = -1, nBranch1Pos = -1, nBranch2Pos = -1;
		for(i = 0; i < 8; i++)
		{
			int vx = x + nbrs[i][0];
			int vy = y + nbrs[i][1];
			if(da(vx,vy).col == END_POINT_COL &&
				!(xprev == vx && yprev == vy))
			{
				bEndNbr = true;
				nEndPos = i;
			}
			if(da(vx,vy).col == BRANCHING_POINT_COL &&
				!(xprev == vx && yprev == vy))
			{
				//if first branch point
				if(!bBranchNbr)
				{
					bBranchNbr = true;
					nBranch1Pos = i;
				}
				if(bBranchNbr && ( abs(nbrs[nBranch1Pos][0]) +
					abs(nbrs[nBranch1Pos][1]) ) > 
					( abs(nbrs[i][0]) +
					abs(nbrs[i][1]) ) )
				{
					bTwoBranchNbr = true;
					nBranch2Pos = nBranch1Pos;
					nBranch1Pos = i; 
				}
			}

			if(bTwoBranchNbr && (nBranch1Pos > nBranch2Pos))
			{
				specialNbrs.SetSwap(nBranch1Pos, nBranch2Pos);
				bSPECIALCASE = true;
			}
			else if(bBranchNbr && bEndNbr && (nBranch1Pos > nEndPos))
			{
				specialNbrs.SetSwap(nBranch1Pos, nEndPos);
				bSPECIALCASE = true;
			}
			else if(bTwoBranchNbr && bEndNbr)
			{
				DBG_PRINT1("\nLAST SPECIAL CASE!!! FIX IT!!\n")
			}
		}
		//! END: Matthijs 14-04-2006*/

		for(i=0; i < 8 && (found_nbr==false); i++){ // go around the nbhd

			//! BEGIN: Matthijs 14-04-2006
			int tx = 0, ty = 0;
			if(!bSPECIALCASE)
			{
				//! END: Matthijs 14-04-2006
				tx = x + nbrs[i][0];
				ty = y + nbrs[i][1];
				//! BEGIN: Matthijs 14-04-2006
			}
			else
			{
				tx = x + specialNbrs[i][0];
				ty = y + specialNbrs[i][1];
			}
			//! END: Matthijs 14-04-2006  

			found_nbr = false;

			// if destination is a foreground point not the previous one
			if (da(tx,ty).col != BACKGROUND_COL &&
				!(xprev==tx && yprev==ty)){// must not be coming from destination


					if(da(tx,ty).visited == false)
					{
						if(da(tx,ty).col == SK_POINT_COL){
							// prefer the,closer, non-diagonal points -- this is
							// actually necessary 
							nx = tx;
							ny = ty;

						}

						if(da(tx,ty).col == END_POINT_COL){
							nx = tx;
							ny = ty;
							found_nbr = true;
						}
					} // endif (tx,ty) not visited

					// We prefer a branching-point to a mere skeletal pt when coming
					// from a skeletal point. This is to prevent from flowing into a
					// neighbouring branch at a branching point.

					if(!(tx==x && ty==y))
						if(da(tx,ty).col == BRANCHING_POINT_COL)
						{
							if(da(xprev, yprev).col != BRANCHING_POINT_COL ||
								SQR(tx-xprev) > 1 || SQR(ty-yprev) > 1) // we do not want to go
								// to a branching point which is next to our starting
								// branching point. if we do, then the branch will not be
								// traced completely
							{
								nx = tx;
								ny = ty;
								found_nbr = true;
							}
						}
			} //endif foreground dest not same as previous
		} // end of for loop -- nbrs traversal

		if(nx<0)
		{
			found_nbr = false;

			std::cerr << "did not find a nbr at: ["
				<< x << ", " << y << "], size = " 
				<< dpv->size() << std::endl;

			/*DivPt dp;

			dp = da(x-1,y+1); std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x,y+1);   std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x+1,y+1); std::cerr << dp.col <<"("<<dp.visited<<")\n";

			dp = da(x-1,y); std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x,y);   std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x+1,y); std::cerr << dp.col <<"("<<dp.visited<<")\n";

			dp = da(x-1,y-1); std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x,y-1);   std::cerr << dp.col <<"("<<dp.visited<<")  ";
			dp = da(x+1,y-1); std::cerr << dp.col <<"("<<dp.visited<<")\n";*/
		}
		else
		{
			bool bBrPntAsNbr2 = false;
			int nPosX2 = 0, nPosY2 = 0;

			switch(da(nx,ny).col)
			{
			case END_POINT_COL:
				if(da(nx,ny).visited == false)
				{ 
					// since we may start at an end-point
					da(nx,ny).visited = true;  // update visited status
					dpv->push_back(da(nx,ny));
					branches.push_back(dpv);   // insert new branch
					return;                    // nothing else left to do
				} 
				found_nbr = false;
				break;

			case BRANCHING_POINT_COL:

				//! BEGIN: Matthijs 14-04-2006
				//we should not claim a branch as ours, if one of our
				//nbrs is a branching point and the distance
				//from that branching point to the point we want
				//to claim is smaller
				//so lets first check whether we have a branching
				//point as nbr
				for(i = 0; i < 8; i++)
				{
					int vx = nx + nbrs[i][0];
					int vy = ny + nbrs[i][1];
					if(da(vx,vy).col == BRANCHING_POINT_COL && da(nx,ny).col == BRANCHING_POINT_COL)
					{
						if((abs(nx-vx) + abs(ny-vy)) == 1)
						{
							nPosX2 = vx;
							nPosY2 = vy;
							bBrPntAsNbr2 = true;
						}
					}
				}
				//! END: Matthijs 14-04-2006 

				if(xprev != nx || yprev != ny){ // we're not coming from
					// this BRANCHING_POINT

					da(nx,ny).visited = true;  // update visited status
					dpv->push_back(da(nx,ny));
					branches.push_back(dpv); // insert new branch

					// first, find the appropriate neighbours and claim them
					// for different branches by marking them visited
					int num_nbrs = 0;  // number of nbrs in nextpts
					int nextpts[8][2]; // there are never 8 or more

					for(i=0; i<8; i++){

						int vx = nx + nbrs[i][0];
						int vy = ny + nbrs[i][1];


						if((da(vx,vy).visited == false && // only go to new places
							da(vx,vy).col != BACKGROUND_COL)){ // foreground
								// point 

								//! BEGIN: Matthijs 14-04-2006
								if(bBrPntAsNbr2 && 
									( (abs(nPosX2-vx) + abs(nPosY2-vy)) < 
									(abs(nx-vx) + abs(ny-vy)) ) &&
									da(vx,vy).col == SK_POINT_COL)
								{
									continue;
								}
								//! END: Matthijs 14-04-2006 

								da(vx,vy).visited = true; // so that a nearby branch
								// does not get confused 
								nextpts[num_nbrs][0] = vx;
								nextpts[num_nbrs][1] = vy;
								num_nbrs++;		
						}
					}

					// then, recursively build the branches supplying as next
					// points the neighbours found above
					for(i=0; i<num_nbrs; i++){
						create_branch(nx, ny, 
							nextpts[i][0], // next x
							nextpts[i][1], // next y
							branches, da); 
					}

					return; // do no more after the branching is over
				}

				std::cerr << "\ngot to a false branching point\n";
				found_nbr = false;

				break;

			case SK_POINT_COL: 

				if(da(nx,ny).visited == false){
					dpv->push_back(da(nx,ny)); // insert in branch
					da(nx,ny).visited = true;  // update visited status
					xprev = x; yprev = y;
					x = nx; y = ny;            // go to the next point
					found_nbr = true;          // continue
				}
				break;

			default:
				found_nbr = false;
				break;
			} // end of switch
		}
	} // end of while loop

	if(dpv->size() <= 1) // we did not create a real branch only
		delete dpv;        // the first point was put in and there
	//  __cannot__ be 1-point branches 
}

/*! 
	@brief It eliminates the empty branch formed by two connected nodes src and tgt.
	It copies the non-empty branches from src to tgt and deletes src.

	By D. Macrini
*/
void DDSGraph::eraseEmptyEdge(DDSNode* src, DDSNode* tgt)
{
	DDSEdgeVect::iterator edgeIt;
	DDSEdge *pEdge, *pEmptyEdge = NULL;

	// Look for the actual edge with given endpoints
	for (edgeIt = src->edges.begin(); edgeIt != src->edges.end(); edgeIt++)
	{
		pEdge = *edgeIt;

		if ((pEdge->n1 == src && pEdge->n2 == tgt) ||
			(pEdge->n2 == src && pEdge->n1 == tgt))
		{
			pEmptyEdge = pEdge;
			break;
		}
	}

	if (!pEmptyEdge || pEmptyEdge->size() > 2)
	{
		std::cerr << "Error in eraseEmptyEdge(): no empty edge" << std::endl;
		return;
	}

	// Update the info of the tgt node and the flux endpoins of each branch in src
	for (edgeIt = src->edges.begin(); edgeIt != src->edges.end(); edgeIt++)
	{
		pEdge = *edgeIt;

		if (pEdge != pEmptyEdge)
		{
			// Update the endpoins of the branch
			if (pEdge->n1 == src)
				pEdge->n1 = tgt;

			if (pEdge->n2 == src)
				pEdge->n2 = tgt;

			// Update the flux points of the branch
			FluxPointArray&    fpl = pEdge->getFluxPoints();
			BoundaryInfoArray& bil = pEdge->getBoundaryInfoArray();

			if (fpl.front().p == src->fp.p)
			{
				if (bil.size() == fpl.size())
					bil.insert(bil.begin(), bil.front()); // just a duplicate

				fpl.insert(fpl.begin(), tgt->fp);
			}

			if (fpl.back().p == src->fp.p)
			{
				if (bil.size() == fpl.size())
					bil.push_back(bil.back()); // just a duplicate

				fpl.push_back(tgt->fp);
			}

			tgt->edges.push_back(pEdge);
		}
	}

	// Remove empty edge from the edge list of tgt
	for (edgeIt = tgt->edges.begin(); edgeIt != tgt->edges.end(); edgeIt++)
	{
		if (*edgeIt == pEmptyEdge)
		{
			tgt->edges.erase(edgeIt);
			break;
		}
	}

	// Remove empty edge from the edge list of the graph
	for (edgeIt = m_edges.begin(); edgeIt != m_edges.end(); edgeIt++)
	{
		if (*edgeIt == pEmptyEdge)
		{
			m_edges.erase(edgeIt);
			break;
		}
	}

	// Remove the src node from the node list of the graph
	DDSNodeVect::iterator nodeIt;

	for (nodeIt = m_nodes.begin(); nodeIt != m_nodes.end(); nodeIt++)
	{
		if (*nodeIt == src)
		{
			m_nodes.erase(nodeIt);
			break;
		}
	}

	// Delete the empty edge
	delete pEmptyEdge;

	// Finally, release the memory of the src node
	delete src;
}

/*!
	@brief Adds the boundary gap points to the list of connected boundary points
	within a gap. It does not add 'pt0' and only adds pt1 if 'bIncludeLast' is true. 

	Use addGapPoint(pt) to add the first point before calling addGapPoints(...).

	If the given points are not connected, the points in between them are interpolated 
	by a line. The result is always the addition of a list of connected gap points.

	By D. Macrini
*/
void BoundarySegment::addGapPoints(const Point& pt0, const Point& pt1, bool bIncludeLast)
{
	ASSERT_VALID_POINT(pt0);
	ASSERT_VALID_POINT(pt1);

	// If the points are already connected, only add last point if requested
	if (fabs(pt1.x - pt0.x) <= 1 && fabs(pt1.y - pt0.y) <= 1)
	{
		if (bIncludeLast)
			gapPts.push_back(pt1);

		return;
	}

	// Interpolate the given points with a rasterized line, so that the result 
	// is a connected set of points

	// Rasterize a line using Bresenham's line algorithm
	int x0 = ROUND_NUM(pt0.x);
	int y0 = ROUND_NUM(pt0.y);
	const int x1 = ROUND_NUM(pt1.x);
	const int y1 = ROUND_NUM(pt1.y);

	int dy = y1 - y0;
	int dx = x1 - x0;
	int stepx, stepy;

	if (dy < 0) { dy = -dy;  stepy = -1; } else { stepy = 1; }
	if (dx < 0) { dx = -dx;  stepx = -1; } else { stepx = 1; }
	dy <<= 1;                                                  // dy is now 2*dy
	dx <<= 1;                                                  // dx is now 2*dx

	// do not add the first pt, because it's already there. 
	// ie, no gapPts.push_back(...) here

	if (dx > dy) {
		int fraction = dy - (dx >> 1);                         // same as 2*dy - dx
		while (x0 != x1) {
			if (fraction >= 0) {
				y0 += stepy;
				fraction -= dx;                                // same as fraction -= 2*dx
			}
			x0 += stepx;
			fraction += dy;                                    // same as fraction -= 2*dy

			if (bIncludeLast || (x0 != x1 || y0 != y1))        // add last pt only if requested
				gapPts.push_back(Point(x0, y0));
		}
	} else {
		int fraction = dx - (dy >> 1);
		while (y0 != y1) {
			if (fraction >= 0) {
				x0 += stepx;
				fraction -= dy;
			}
			y0 += stepy;
			fraction += dx;

			if (bIncludeLast || (x0 != x1 || y0 != y1))
				gapPts.push_back(Point(x0, y0));
		}
	}

	if (bIncludeLast && (gapPts.back().x != x1 || gapPts.back().y != y1))
		gapPts.push_back(Point(x1, y1));
}
