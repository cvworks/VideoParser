/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <ShapeRepresentation/ShapeInformation.h>
#include <GraphTheory/AttributedGraph.h>
#include <GraphTheory/EdgeMap.h>
#include <Tools/Point.h>
#include <Tools/STLUtils.h>
#include "ShapeCutPath.h"
#include "ShapeParseGraph.h"

namespace vpl {

/*!
	Attributes of a node in a Shape Cut Graph
*/
struct SCGNodeAtt
{
	int idx;
	Point pt;
};

/*!
	Attributes of an edge in a Shape Cut Graph
*/
struct SCGEdgeAtt
{
	bool isDummy;
	bool isIgnored;
};

/*!
	Endpoints of a shape cut used during the construction of the
	shape parse graphs.
*/
struct ShapeCutEndpoint
{
	int idx;
	bool firstIdx;
	int partnerIdx;
	graph::edge arc;

	ShapeCutEndpoint() { }
	ShapeCutEndpoint(graph::edge e) : arc(e) { }

	void Set(int i_endpt, bool isFromPt, int i_partner, graph::edge e)
	{
		idx = i_endpt;
		firstIdx = isFromPt;
		partnerIdx = i_partner;
		arc = e;
	}

	bool operator<(const ShapeCutEndpoint& rhs) const;
};

typedef AttributedGraph<SCGNodeAtt, SCGEdgeAtt> SCGBaseClass;

/*!
	Graph of shape cuts and dummy contour intervals.
	
	The edges that map to shape cuts in the SCG have
	lower indices than the dummy edges. This fact is used
	to index the cut variables in the shape parsing model.

	The edges that corresponds to shape cuts are indexed from
	0 to NumberOfShapeCuts() - 1.

	The dummy edges are indexed from NumberOfShapeCuts() to 
	number_of_edges() - 1.

	When forall_edges(e,g) reaches a dummy edge, it means that 
	all	non-dummy edges and no dummy edges have been visited.

	@see ShapeParsingModel
*/
class ShapeCutGraph : public SCGBaseClass
{
	typedef std::vector<ShapeCutEndpoint> EndpointArray;

	ShapeInfoPtr m_ptrShapeInfo; //!< Basic shape information. eg, its boundary, corners, etc

	std::vector<int> m_sortedEndpts;
	std::map<int,node> m_endpt2node;
	std::list<ShapeCutPath> m_cycles;

	unsigned m_numShapeCuts; //! Number of edges that map to shape cuts

	struct Params
	{
		bool drawNodeLabels;
		bool drawEdgeLabels;
		bool drawNodes;
		bool drawEdges;
	};
	
	static Params s_params;

	void Create(ShapeInfoPtr ptrShapeInfo);
	void FindMaxLenCycles(const unsigned K);

protected:
	EndpointArray GetSortedActiveEndpoints(
		const EdgeMap<bool>& activeCuts) const;

public:
	ShapeCutGraph(ShapeInfoPtr ptrShapeInfo) 
		: SCGBaseClass(graph::UNDIRECTED), m_ptrShapeInfo(ptrShapeInfo)
	{
		Create(ptrShapeInfo);	
	}

	const std::list<ShapeCutPath>& Cycles() const
	{
		return m_cycles;
	}

	const ShapeCutPath& Cycle(unsigned i) const
	{
		return element_at(m_cycles, i);
	}

	unsigned NumberOfShapeCuts() const
	{
		return m_numShapeCuts;
	}

	double CutLength(edge e) const
	{
		return inf(source(e)).pt.dist(inf(target(e)).pt);
	}

	unsigned ShapeBoundarySize() const
	{
		return m_ptrShapeInfo->BoundarySize();
	}

	SPGPtr CreateShapeParseGraph(const EdgeMap<bool>& activeCuts) const;
	SPGPtr CreateSinglePartSPG() const;

	void Draw(unsigned parseId) const;

	std::string GetOutputText(unsigned parseId) const;

	static void ReadParamsFromUserArguments();

	static void GetSwitchCommands(std::list<UserCommandInfo>& cmds);
};

//! Handy alias for the class ShapeCutGraph
typedef ShapeCutGraph SCG;

//! Shared pointer to a ShapeCutGraph
typedef std::shared_ptr<ShapeCutGraph> SCGPtr;

} // namespace vpl
