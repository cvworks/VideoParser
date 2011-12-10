/**************************************************************************

   File:                DDSGraph.h

   Author(s):           Pavel Dimitrov

   Created:             22 Jul 2002

   Last Revision:       $Date: 2002/07/25 20:50:47 $

   Description: 

   $Revision: 1.2 $

   $Log: DDSGraph.h,v $
   Revision 1.2  2002/07/25 20:50:47  pdimit
   Making release 0.1

   Revision 1.1  2002/07/23 21:02:51  pdimit
   The branch segmentation of the thinned DivMap is improved --
   the two pixel branches are now included. Still, the degenerate case
   of the square of junction-points is not taken care of.

   Also, a DDSGraph is created which still does not know
   of left and right for the branches, i.e. the contour has not been cut.

   Copyright (c) 2002 by Pavel Dimitrov, Centre for Intelligent Machines,
   McGill University, Montreal, QC.  Please see the copyright notice
   included in this distribution for full details.

 **************************************************************************/

#ifndef DISCRETE_DIVERGENCE_SKELETON_H
#define DISCRETE_DIVERGENCE_SKELETON_H

#include <stdlib.h>
#include "defines.h"
#include "DivergenceMap.h"
#include "DDSEdge.h"
#include "ShapeBoundary.h"

namespace sg {

typedef std::vector<DDSNode*> DDSNodeVect;

/*!
	Graph of skeletal points in which the m_nodes represent
	the terminal point of skeletal branches, and the m_edges
	represent all the points in each branch (including
	the terminal points).
*/
class DDSGraph
{
protected:
	ShapeBoundary* m_pShape;
	DDSEdgeVect m_edges;
	DDSNodeVect m_nodes;

public:
	DDSGraph()
	{
		m_pShape = NULL;
	}

	DDSGraph(ShapeBoundary* pShape) 
	{
		m_pShape = pShape;
	}

	virtual ~DDSGraph()
	{
		DDSEdgeVect::iterator edgeIt;
		DDSNodeVect::iterator nodeIt;

		for(edgeIt = m_edges.begin(); edgeIt != m_edges.end(); edgeIt++)
			delete (*edgeIt);

		for(nodeIt = m_nodes.begin(); nodeIt != m_nodes.end(); nodeIt++)
			delete (*nodeIt);

		delete m_pShape;
	}

	void getBounds(double *xmin, double *xmax,
		double *ymin, double *ymax) const
	{
		if (m_pShape != NULL)
		{
			m_pShape->getBounds(xmin, xmax, ymin, ymax);
		}
		else
		{
			*xmin = *xmax = *ymin = *ymax = 0;
		}
	}

	/*!
		Release the shape's boundary so that it can be used
		after the skeleton is destroyed.
	*/
	ShapeBoundary* releaseShape()
	{
		ShapeBoundary* p = m_pShape;
		m_pShape = NULL;
		return p;
	}

	void addNode(DDSNode *n)
	{
		m_nodes.push_back(n);
		n->setSkeleton(this);
	}

	void addEdge(DDSEdge *e)
	{
		m_edges.push_back(e);
		e->setSkeleton(this);
	}

	/*!
		Assigne to every node and edge their corresponding
		indices along the node and edge vectors, respectively.
	*/
	void SetNodeAndEdgeIndices()
	{
		// Assign consecutive indices to each node
		unsigned int i;

		for (i = 0; i < m_nodes.size(); i++)
			m_nodes[i]->nodeIndex = i;

		for (i = 0; i < m_edges.size(); i++)
			m_edges[i]->edgeIndex = i;
	}

	void eraseEmptyEdge(DDSNode *src, DDSNode *tgt);

	const DDSEdgeVect& getEdges() const { return m_edges; }
	DDSEdgeVect& getEdges()             { return m_edges; }

	const DDSNodeVect& getNodes() const { return m_nodes; }
	DDSNodeVect& getNodes()             { return m_nodes; }

	const ShapeBoundary* getShape() const { return m_pShape; }
	ShapeBoundary* getShape()             { return m_pShape; }

	unsigned int edgeCount() const      { return m_edges.size(); }

	unsigned int nodeCount() const      { return m_nodes.size(); }

	bool empty() const { return (m_edges.empty() && m_nodes.empty()); }

	void setBoundaryPoints();

	void assignBoundaryPoints(const FluxPointArray& fpl, BoundaryInfoArray& bil, 
		const BoundaryInfo& bi0, const BoundaryInfo& biN);

	void assignRadiusValues(const BoundaryInfoArray& bil, FluxPointArray& fpl);

	bool regularize();

	void Deserialize(InputStream& is);
	void Serialize(OutputStream& os) const;

public: // static members and functions
	static int s_div_map_resolution;
	static double s_div_array_step;
	static double s_thinning_thresh;
	static bool s_compute_boundary_mapping;
	static bool s_regularize_skeleton;
	static bool s_smooth_branches;

	static void ReadParamsFromUserArguments();
	static DDSGraph* createDDSGraph(ShapeBoundary* pShape);
	static void smoothBranchPoints(sg::FluxPointArray& pts);
};

} // namespace sg

#endif // DISCRETE_DIVERGENCE_SKELETON_H
