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

#ifndef DISCRETE_DIVERGENCE_SKELETON_NODE_H
#define DISCRETE_DIVERGENCE_SKELETON_NODE_H

#include "FluxPoint.h"
#include "defines.h"

namespace sg {
	
class DDSGraph;
class DDSEdge;

typedef std::vector<DDSEdge*> DDSEdgeVect;

/*!
	Node of a DDSGraph graph 
*/
class DDSNode 
{
public: // TODO: should be protected but AFMMSkeleton.h must be corrected first
	DDSGraph *dds;

public:
	FluxPoint fp; //!< Flux point information
	DDSEdgeVect edges; //!< Array of incident graph edges (skeletal branches)

	unsigned int nodeIndex; //!< Node's index on the node vector of the containter graph

public:
	DDSNode(const FluxPoint& p)
	{
		fp = p;
	}

	void addEdge(DDSEdge* e)
	{
		edges.push_back(e);
	}

	DDSEdgeVect& getEdges() 
	{ 
		return edges; 
	}

	void setEdge(unsigned int i, DDSEdge* e) 
	{ 
		edges[i] = e; 
	}
	
	const DDSEdge* getEdge(unsigned int i) const
	{
		return edges[i];
	}

	DDSEdge* getEdge(unsigned int i)
	{
		return edges[i];
	}

	void setSkeleton(DDSGraph* sk) { dds = sk; }
	DDSGraph* getSkeleton()        { return dds; }

	double getFlux() { return fp.val;  }
	double getDist() { return fp.dist; }
	char   getCol()  { return fp.col;  }
	Point  getPoint(){ return fp.p;    }

	unsigned int degree() const { return edges.size(); }
};

} //namespace sg

#endif // DISCRETE_DIVERGENCE_SKELETON_NODE_H
