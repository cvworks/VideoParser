/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "DDSGraph.h"
#include "DivergenceSkeletonMaker.h"
#include "DDSGraphUtils.h"
#include "BoundaryPointMapping.h"
#include <Tools/UserArguments.h>

using namespace sg;

extern vpl::UserArguments g_userArgs;

int DDSGraph::s_div_map_resolution = 8;
double DDSGraph::s_div_array_step = 1;
double DDSGraph::s_thinning_thresh = 2.5;

bool DDSGraph::s_compute_boundary_mapping = 1;
bool DDSGraph::s_regularize_skeleton = 0;
bool DDSGraph::s_smooth_branches = 0;

/*!
	[static] 
*/
void DDSGraph::ReadParamsFromUserArguments()
{
	// Read skeleton computation parameters
	g_userArgs.ReadArg("FluxSkeleton", "resolution", 
		"Number of rays emanating from each pixel", 8, 
		&s_div_map_resolution);

	g_userArgs.ReadArg("FluxSkeleton", "step", 
		"Step size for sampling shape points", 1.0, &s_div_array_step);

	g_userArgs.ReadArg("FluxSkeleton", "tau", 
		"Branch threshold for minimum angle", -2.5, &s_thinning_thresh);

	// Check that the values are valid
	int minRes = 4;
	double minStep = 0.01, maxStep = 1, maxTau = -1;

	g_userArgs.CheckMinValues("FluxSkeleton", "resolution", minRes);
	g_userArgs.CheckValues("FluxSkeleton", "step", minStep, maxStep);
	g_userArgs.CheckMaxValues("FluxSkeleton", "tau", maxTau);

	// Read the type of shape representation to use when make shape is called
	ShapeMaker::ReadUserArguments();

	// Read skeleton post-processing options
	g_userArgs.ReadBoolArg("FluxSkeleton", "mapBoundary", 
		"Compute skeleton->boundary mapping?", true, &s_compute_boundary_mapping);

	g_userArgs.ReadBoolArg("FluxSkeleton", "regularize", 
		"Enforce skeletal properties?", true, &s_regularize_skeleton);

	g_userArgs.ReadBoolArg("FluxSkeleton", "smooth", 
		"Smooth skeletal branches?", true, &s_smooth_branches);
}

/*!
	[static] Normal values for params and recommended values:

	resolution >= 4 [16]; 0 < step <= 1 [1]; fluxTau <= -1 [-2.5];
*/
DDSGraph* DDSGraph::createDDSGraph(ShapeBoundary* pShape)
{
	DDSGraph* pDDSGraph = new DDSGraph(pShape); // owns the shape

	// Create a DistanceTransform object (false => don't own shape)
	DistanceTransform* pDT = new DistanceTransform(pShape, false);

	// Create a DivergenceMap (owns the DT)
	DivergenceMap dm(pDT, s_div_map_resolution);

	computeDDSkeleton(pDDSGraph, dm, s_div_array_step, s_thinning_thresh);

	if (s_regularize_skeleton)
		pDDSGraph->regularize();

	if (s_compute_boundary_mapping)
	{
		pDDSGraph->setBoundaryPoints();

		pDDSGraph->m_pShape->setSkeletalPoints(pDDSGraph);
	}

	// Finally, now that the graph is built, it is convenient
	// to assign consecutive indices to nodes and edges, so that
	// they can be used as ids when analysing the junctions and
	// branches of the skeleton
	pDDSGraph->SetNodeAndEdgeIndices();

	return pDDSGraph;
}

/*!
	@brief Assigns the boundary point information of each point in the 
	skeleton graph data structure. It follows a nearest neighbour search 
	approach. This is the only function that modifies the input skeleton graph. 
	
	Essentially, this function fills the information in the BoundaryInfoArray 
	of each	edge in the skeleton graph. In fact, the BoundaryInfoArray member 
	variable was added to the DDSGraph class so that we could assing the needed 
	info in this function.
*/
void DDSGraph::setBoundaryPoints()
{
	BoundaryPointMapping finder(m_pShape->getKDTree());

	finder.AssignBoundaryPoints(this);
}

/* 
	@brief Uses the info computed by AssignBoundaryInfo() to the set 
	bndry pts of a NEW branch.
*/ 
void DDSGraph::assignBoundaryPoints(const FluxPointArray& fpl, BoundaryInfoArray& bil,
									const BoundaryInfo& bi0, const BoundaryInfo& biN)
{
	BoundaryPointMapping finder(m_pShape->getKDTree());

	finder.AssignBoundaryPoints(fpl, bil, bi0, biN);
}

/*! 
	@brief Uses the info computed by AssignBoundaryInfo() to set the 
	radius values of a NEW branch
*/
void DDSGraph::assignRadiusValues(const BoundaryInfoArray& bil, 
									   FluxPointArray& fpl)
{
	BoundaryPointMapping finder(m_pShape->getKDTree());

	finder.AssignRadiusValues(bil, fpl);
}

/*!
	@brief Ensures that all invariant properties of a skeletal graph are
	satisfied
*/
bool DDSGraph::regularize()
{
	SkelJoint* pJoint;
	SkelBranch* pBranch;

	std::vector< std::pair<SkelJoint*, SkelJoint*> > nodePairs;

	forall_joints(pJoint, getNodes())
	{
		// Search for special joints that are redundant and delete them
		if (pJoint->degree() < 3)
		{	
			WARNING1(true, "Joint has less than 3 branches", pJoint->fp);

			SkelBranch* pBranch;
			SkelJoint* pTgtJoint = NULL;
			double minDist = 3;

			forall_branches(pBranch, pJoint->edges)
			{
				if (pBranch->n1 != pJoint &&
					pJoint->fp.p.dist(pBranch->n1->fp.p) < minDist)
				{
					pTgtJoint = pBranch->n1;
					minDist = pJoint->fp.p.dist(pBranch->n1->fp.p);
				}

				if (pBranch->n2 != pJoint &&
					pJoint->fp.p.dist(pBranch->n2->fp.p) < minDist)
				{
					pTgtJoint = pBranch->n2;
					minDist = pJoint->fp.p.dist(pBranch->n2->fp.p);
				}
			}

			if (minDist > 2)
			{
				ShowError("Invalid input. Skeleton is disconnected.");
				return false;
			}

			nodePairs.push_back(std::make_pair(pJoint, pTgtJoint));
		}
	}

	// Find the most common case of adjacent joints, which shoud in fact be a single joint
	forall_branches(pBranch, getEdges())
	{
		if (pBranch->size() == 2 && pBranch->n1->degree() == 3 && pBranch->n2->degree() == 3)
		{
			if (pBranch->n1->fp.radius() > pBranch->n2->fp.radius())
			{
				nodePairs.push_back(std::make_pair(pBranch->n2, pBranch->n1));
			}
			else
			{
				nodePairs.push_back(std::make_pair(pBranch->n1, pBranch->n2));
			}
		}
	}

	// Remove all empty DDS edges (ie branches) found
	for (unsigned int i = 0; i < nodePairs.size(); i++)
		eraseEmptyEdge(nodePairs[i].first, nodePairs[i].second);

	// Smooth skeletal branch points
	forall_branches(pBranch, getEdges())
	{
		FluxPointArray& fpl = pBranch->getFluxPoints();
		int zeroRadiusCount = 0;

		// Some terminal points may have zero radius due to subpixel accuracy
		for (unsigned int i = 0; i < fpl.size(); i++)
		{
			if (fpl[i].radius() == 0)
			{
				fpl[i].dist = 0.5; // half a pixel
				zeroRadiusCount++;
			}
		}

		WARNING1(zeroRadiusCount > 1, 
			"There are too many points with zero radius", zeroRadiusCount);

		if (s_smooth_branches)
			smoothBranchPoints(pBranch->getFluxPoints());
	}

	return true;
}

//! Here we average the points. Diego: Bug fixed. it was "eating" one point.
void DDSGraph::smoothBranchPoints(sg::FluxPointArray& pts)
{
	if (pts.size() <= 2)
		return;

	// We have at least 3 points
	std::vector<sg::Point> spts(pts.size());

	unsigned int sz = pts.size() - 1;
	unsigned int i;
	
	for (i = 1; i < sz; i++)
	{
		spts[i].x = (pts[i - 1].p.x + pts[i].p.x + pts[i + 1].p.x) / 3.0;
		spts[i].y = (pts[i - 1].p.y + pts[i].p.y + pts[i + 1].p.y) / 3.0;
	}

	for (i = 1; i < sz; i++)
	{
		pts[i].p = spts[i];
	}
}

DECLARE_BASIC_SERIALIZATION(FluxPoint)
DECLARE_BASIC_SERIALIZATION(BoundaryInfo)

/*!
	Writes the member variables of a DDSGraph and then calls
	WriteNodesAndEdges() in order to write the attributes of
	all nodes and edges of the graph.

	It also writes the m_lineSegments and m_bezierSegments
	members of the DDSGraph.
*/
void DDSGraph::Serialize(OutputStream& os) const
{
	// Serialize nodes
	{
		const DDSNodeVect& nodes = getNodes();

		DDSNode *pNode;
		
		FluxPoint fp;
		BoundaryInfo bi;

		// Write the size of the graph (ie, the number of nodes)
		::Serialize(os, nodes.size());
		
		// Assign an ID to each node and write it along with
		// the FluxPoint attribute of the node. The ID of the
		// node is defined by the pointer to it stored in the node array.
		for(DDSNodeVect::const_iterator I = nodes.begin(); I != nodes.end(); ++I)
		{
			pNode = *I; // get the pointer to the node and use it as its ID
			::Serialize(os, (void*)pNode);
			::Serialize(os, pNode->fp);
		}
	}
	
	// Serialize nodes
	{
		DDSEdge *e;

		const DDSEdgeVect& edges = getEdges();

		// Write the number of edges in the graph
		::Serialize(os, edges.size());

		for(DDSEdgeVect::const_iterator I = edges.begin(); I != edges.end(); I++)
		{
			e = *I;

			::Serialize(os, (void*)e->getN1());
			::Serialize(os, (void*)e->getN2());

			::Serialize(os, e->getFluxPoints());
			::Serialize(os, e->getBoundaryInfoArray());
		}
	}

	// Read Contour
	bool hasShapeBoundary = (m_pShape != NULL);

	::Serialize(os, hasShapeBoundary);

	if (hasShapeBoundary)
		m_pShape->Serialize(os);
}

/*!
	Reads the member variables of a DDSGraph and then calls
	ReadNodesAndEdges() in order to read the attributes of
	all nodes and edges of the graph.

	It also reads the m_lineSegments and m_bezierSegments
	members of the DDSGraph.
*/
void DDSGraph::Deserialize(InputStream& is)
{
	ASSERT(empty());
	
	std::map<long, DDSNode*> nodeMap;

	// Deserialize nodes
	{
		DDSNode* pNode;	
		unsigned numNodes;
		FluxPoint fp;
		void *key;
		
		// Read the size of the graph (ie, the number of nodes)
		::Deserialize(is, numNodes);
		
		// Read the ID and FluxPoint attribute of each node
		for (unsigned i = 0; i < numNodes; i++) 
		{
			::Deserialize(is, key); // get the node ID
			::Deserialize(is, fp);   // get the FluxPoint attribute

			pNode = new DDSNode(fp);  // create a new node

			nodeMap[(long)key] = pNode; // map the ID the node pointer
			addNode(pNode);         // add the node pointer to the graph attributes
		}
	}

	// Deserialize edges
	{
		DDSEdge *e;
		unsigned numEdges;
		void *keyN1, *keyN2;

		// Read the number of edges in the graph
		::Deserialize(is, numEdges);

		FluxPointArray fpl;
		BoundaryInfoArray bil;

		for (unsigned i = 0; i < numEdges; i++) 
		{
			::Deserialize(is, keyN1);
			::Deserialize(is, keyN2);

			::Deserialize(is, fpl);
			::Deserialize(is, bil);

			// Create a new edge object with pointers to its pair of terminal nodes
			e = new DDSEdge(this, fpl, nodeMap[(long)keyN1], nodeMap[(long)keyN2]);

			e->setBoundaryInfoArray(bil);

			addEdge(e);
		}
	}

	//Read Contour
	ASSERT(!m_pShape);

	bool hasShapeBoundary;

	::Deserialize(is, hasShapeBoundary);

	if (hasShapeBoundary)
	{
		m_pShape = new ShapeBoundary();
		
		m_pShape->Deserialize(is);

		// Recompute the skeletal info that is not serializied along with the shape
		m_pShape->setSkeletalPoints(this);
	}
}
