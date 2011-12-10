/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#ifndef _SHAPE_PARSING_GRAPH_H_
#define _SHAPE_PARSING_GRAPH_H_

#include <ShapeRepresentation/ShapeInformation.h>
#include <GraphTheory/Graph.h>
#include <Tools/Point.h>
#include <Tools/STLUtils.h>
#include "ShapeParseGraph.h"

namespace vpl {

/*!
	Attributes of a node in a Boundary Cut Graph
*/
struct BCGNodeAtt
{
	BoundaryCut bc; //!< Boundary cut represented by the graph node
	Point pt;       //!< Middle point along the boundary cut
	std::vector<bool> active; //!< Specifies whether the boundary cut is active in the i'th parse
	std::vector<IntList> boundaries; //!< The boundary of the part in the i'th parse

	void operator=(const BCGNodeAtt& rhs)
	{
		bc = rhs.bc;
		pt = rhs.pt;
		active = rhs.active;
	}
};

struct BoundaryCutEndpoint
{
	int idx;
	bool firstIdx;
	int partnerIdx;
	graph::node v;

	BoundaryCutEndpoint() { }
	BoundaryCutEndpoint(graph::node n) : v(n) { }

	void Set(int i_endpt, bool isFromPt, int i_partner, graph::node n)
	{
		idx = i_endpt;
		firstIdx = isFromPt;
		partnerIdx = i_partner;
		v = n;
	}

	bool operator<(const BoundaryCutEndpoint& rhs) const;
};

/*struct BCGNodeAtt
{
	const sg::SkelJoint* pJoint;
	Point pt;
};*/

/*!
	Attributes of an edge in a Boundary Cut Graph
*/
struct BCGEdgeAtt
{
	bool isDummy;

	//const sg::SkelBranch* pBranch;
	//Point srcPt;
	//Point tgtPt;

	BCGEdgeAtt()
	{
		isDummy = false;
	}
};

typedef AttributedGraph<BCGNodeAtt, BCGEdgeAtt> BCGBaseClass;

/*!
	Graph of boundary cuts with edges representinc the
	probabilistic dependencies in the activation of cuts.
*/
class BoundaryCutGraph : public BCGBaseClass
{
	typedef std::vector<BoundaryCutEndpoint> EndpointArray;

	ShapeInfoPtr m_ptrShapeInfo;  //!< Shape representation associated with the BCG

	std::vector<EndpointArray> m_activeEndpts; //!< Active endpoints associated with each parse
	std::vector<ShapeParseGraph> m_parses; //!< Array of all shape parses

	struct Params
	{
		bool drawLabels;
		bool drawNodes;
		bool drawEdges;
	};
	
	static Params s_params;

public:
	BoundaryCutGraph(ShapeInfoPtr ptrShapeInfo) 
		: m_ptrShapeInfo(ptrShapeInfo)
	{
		
	}

	void ComputeShapeParses(unsigned k);

	void SortActiveEndpoints(unsigned parseId);

	unsigned NumberOfParses() const
	{
		return m_parses.size();
	}

	const ShapeParseGraph& GetShapeParse(unsigned parseId) const
	{
		return m_parses[parseId];
	}

	unsigned ShapeBoundarySize() const
	{
		return m_ptrShapeInfo->BoundarySize();
	}

	void Draw(unsigned parseId) const;

	std::string GetOutputText(unsigned parseId) const;

	static void ReadParamsFromUserArguments();

	static void GetSwitchCommands(std::list<UserCommandInfo>& cmds);
};

//! Handy alias for the class BoundaryCutGraph
typedef BoundaryCutGraph BCG;

//! Shared pointer to a BoundaryCutGraph
typedef std::shared_ptr<BoundaryCutGraph> BCGPtr;

} // namespace vpl

#endif //_SHAPE_PARSING_GRAPH_H_