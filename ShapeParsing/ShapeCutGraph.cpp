/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ShapeCutGraph.h"
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
#include <Tools/ColorMatrix.h>
#include <queue>

using namespace vpl;

extern UserArguments g_userArgs;

SCG::Params SCG::s_params;

/*!
	[static] Reads the ShapeParseGraph parameters provided by the user
*/
void SCG::ReadParamsFromUserArguments()
{
	// Read the SCG parameters
	g_userArgs.ReadBoolArg("ShapeCutGraph", "drawNodeLabels", 
		"Whether to draw node labels or not", false, &s_params.drawNodeLabels);

	g_userArgs.ReadBoolArg("ShapeCutGraph", "drawEdgeLabels", 
		"Whether to draw edge labels or not", true, &s_params.drawEdgeLabels);

	g_userArgs.ReadBoolArg("ShapeCutGraph", "drawNodes", 
		"Whether to draw nodes or not", false, &s_params.drawNodes);

	g_userArgs.ReadBoolArg("ShapeCutGraph", "drawEdges", 
		"Whether to draw edges or not", true, &s_params.drawEdges);
}

//! [static] Update drawing parameters
void SCG::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("nodes", "draw nodes", 'n', 
		&s_params.drawNodes));

	cmds.push_back(UserCommandInfo("edges", "draw edges", 'e', 
		&s_params.drawEdges));

	cmds.push_back(UserCommandInfo("node labels", "draw node labels", '1', 
		&s_params.drawNodeLabels));

	cmds.push_back(UserCommandInfo("edge labels", "draw edge labels", '2', 
		&s_params.drawEdgeLabels));
}

/*!
	Draw the information associated with the SCG
*/
void SCG::Draw(unsigned parseId) const
{
	SetFont(GetFontFace(0, true), 14);

	if (s_params.drawEdges)
	{
		edge e;

		LineList cutEdges, dummyEdges;

		SetDrawingColor(NamedColor("Black"));
		
		forall_edges(e, *this)
		{
			auto ln = std::make_pair(inf(source(e)).pt, 
				inf(target(e)).pt);

			if (inf(e).isDummy)
			{
				dummyEdges.push_back(ln);
			}
			else
			{
				cutEdges.push_back(ln);

				if (s_params.drawEdgeLabels)
				{
					DrawNumber(index(e), 
						LineSegmentPoint(ln.first, ln.second, 0.5));
				}
			}
		}

		SetDrawingColor(NamedColor("DarkSalmon"));

		DrawLines(cutEdges);

		SetDrawingColor(NamedColor("MediumSlateBlue"));

		DrawLines(dummyEdges);
	}

	if (s_params.drawNodes)
	{
		node v;

		SetDrawingColor(RGBColor(0, 0, 255));

		forall_nodes(v, *this)
		{
			DrawCircle(inf(v).pt, 2);
		}
	}
	
	if (s_params.drawNodeLabels)
	{
		node v;
		
		SetDrawingColor(NamedColor("Black"));

		forall_nodes(v, *this)
		{
			//inf(v).idx
			DrawNumber(index(v), inf(v).pt);
		}
	}

	SetDefaultDrawingColor();
}

std::string SCG::GetOutputText(unsigned parseId) const
{
	std::ostringstream oss;

	oss << "There are " << number_of_nodes() << " nodes and "
		<< number_of_edges() << " edges."; 

	return oss.str();
}

/*!
	Creates the shape cut graph of the shape.
*/
void SCG::Create(ShapeInfoPtr ptrShapeInfo)
{
	typedef std::map<int, std::list<graph::node> > Point2NodeMap;

	const sg::DDSGraph* pSkelG = ptrShapeInfo->GetDDSGraph();
	const BoundaryCutSet& cuts = ptrShapeInfo->m_bndryCuts.GetCuts();
	const sg::ShapeBoundary& bndry = *ptrShapeInfo->m_pBoundary;

	// Sort and store all cut endpoints
	m_sortedEndpts.clear();
	m_sortedEndpts.reserve(cuts.size() * 2);

	for (auto it = cuts.begin(); it != cuts.end(); ++it)
	{
		m_sortedEndpts.push_back(it->From());
		m_sortedEndpts.push_back(it->To());
	}

	// Sort endpoints
	std::sort(m_sortedEndpts.begin(), m_sortedEndpts.end());

	// Remove duplicates
	auto newEndIt = std::unique(m_sortedEndpts.begin(), m_sortedEndpts.end());

	m_sortedEndpts.resize(newEndIt - m_sortedEndpts.begin()); 

	// Create a node for each shape cut endpoint
	SCGNodeAtt nodeAtt;

	for (auto it = m_sortedEndpts.begin(); it != m_sortedEndpts.end(); ++it)
	{
		nodeAtt.idx = *it;
		bndry.getPoint(*it, &nodeAtt.pt);
		
		m_endpt2node[*it] = new_node(nodeAtt);
	}

	// Create an edge for each shape cut
	SCGEdgeAtt edgeAtt;
	
	edgeAtt.isDummy = false;
	edgeAtt.isIgnored = false;

	for (auto it = cuts.begin(); it != cuts.end(); ++it)
	{
		new_edge(m_endpt2node[it->From()], m_endpt2node[it->To()], edgeAtt);
	}

	// Store the partial count of edges, which corresponds to
	// the number of edges that map to shape cuts
	m_numShapeCuts = number_of_edges();

	// Create the dummy edges
	edgeAtt.isDummy = true;
	edgeAtt.isIgnored = true;

	node u, v;

	if (m_sortedEndpts.size() >= 2)
	{
		for (std::vector<int>::iterator it1 = m_sortedEndpts.begin(), it0 = it1++; 
			it1 != m_sortedEndpts.end(); ++it0, ++it1)
		{
			u = m_endpt2node[*it0];
			v = m_endpt2node[*it1];

			if (!has_edge(u, v))
				new_edge(u, v, edgeAtt);
		}
	}

	// Add the back-to-first endpoint edge if there are 3 or more endpoints
	if (m_sortedEndpts.size() >= 3)
	{
		u = m_endpt2node[m_sortedEndpts.back()];
		v = m_endpt2node[m_sortedEndpts.front()];

		if (!has_edge(u, v))
			new_edge(u, v, edgeAtt);
	}

	// Set the indices of nodes and edges
	set_indices();

	// Find and store the relevant cycles in the graph
	FindMaxLenCycles(4);

	// Set the isIgnored attribute to false for all dummy edges in a cycle
	for (auto it = m_cycles.begin(); it != m_cycles.end(); ++it)
	{
		if (it->dummyEdge != nil)
			attribute(it->dummyEdge).isIgnored = false;
	}
}

/*!
	It stores all cycles of length equal to or less than K into a list
	of cycles. Every cycle	starts at the node with the lowest index. 
	Every cycle is added twice to the list of cycles, one for each
	possible direction in which its node can be visited starting from 
	the selected first node.
*/
void SCG::FindMaxLenCycles(const unsigned K)
{
	std::queue<ShapeCutPath> paths;
	ShapeCutPath p;
	node u, v;
	edge e;

	// Init list of cycles to empty list
	m_cycles.clear();
	
	// Init queue of paths by creating a two-node path 
	// with each edge in the graph
	forall_edges(e, *this)
	{
		u = source(e);
		v = target(e);

		if (index(u) > index(v))
			std::swap(u, v);
		
		paths.push(ShapeCutPath(u, v, e));
	}
	
	// Process the queue until it's empty
	edge cycleEdge;
	node a;
	
	while (!paths.empty())
	{
		p = paths.front();
		paths.pop();

		a = p.nodes.front();
		u = p.nodes.back();
		cycleEdge = nil;
		
		// Add the interior edges of the path (except for
		// the current "cycle" edge, which has to be
		// added only to subsequent the super-cycles.
		// Note that dummy edges cannot be interior edges.
		forall_adj_edges(e, u)
		{
			if (e == p.edges.back())
				continue;

			v = opposite(u, e);

			if (v == a)
				cycleEdge = e;
			else if (!inf(e).isDummy && p.ContainsProper(v))
				p.interiorEdges.push_back(e);
		}
		
		// Find cycle and candidate super-cycles
		bool isDummy;
		
		forall_adj_edges(e, u)
		{
			if (e == p.edges.back())
				continue;

			isDummy = inf(e).isDummy;

			// If edge is dummy and there is already a dummy
			// edge in the path, then we skip this edge
			if (isDummy && p.dummyEdge != nil)
				continue;

			v = opposite(u, e);
			
			if (v == a)
			{
				m_cycles.push_back(p);

				if (isDummy)
					m_cycles.back().dummyEdge = e;
				else
					m_cycles.back().edges.push_back(e);
			}
			else if (p.Size() <= K - 1 && 
				index(v) > index(a) && !p.Contains(v))
			{
				// We have a candidate super-cycle of length > k
				paths.push(p);

				paths.back().nodes.push_back(v);
				
				if (isDummy) 
					paths.back().dummyEdge = e;
				else
				{
					paths.back().edges.push_back(e);

					// A "cycle edge" that is not dummy is also
					// an interior edge of each subsequent super-cycle
					if (cycleEdge != nil)
						paths.back().interiorEdges.push_back(cycleEdge);
				}
			}
		}
	}
}

/*!
	
*/
bool ShapeCutEndpoint::operator<(const ShapeCutEndpoint& rhs) const
{
	if (idx < rhs.idx)
		return true;
	else if (idx > rhs.idx)
		return false;

	// We need to know the number of boundary points in order to define an interval
	const unsigned N = static_cast<const SCG*>
		(graph_of(source(arc)))->ShapeBoundarySize();

	sg::BoundaryInterval bi;

	if (idx < partnerIdx)
		bi.Set(N, partnerIdx, idx);
	else
		bi.Set(N, idx, partnerIdx);

	return (bi.Includes(rhs.partnerIdx)) ? false : true;
}

/*!
	Sorts the active endpts of the given shape parse configuration.
*/
SCG::EndpointArray SCG::GetSortedActiveEndpoints(
	const EdgeMap<bool>& activeCuts) const
{
	EndpointArray epts;

	// The maximum num of endpts is the num of nodes times 2
	epts.reserve(number_of_nodes() * 2);

	ShapeCutEndpoint sce;
	edge e;

	forall_edges(e, *this)
	{
		if (activeCuts[e])
		{
			sce.Set(inf(source(e)).idx, true, inf(target(e)).idx, e);
			epts.push_back(sce);

			sce.Set(inf(target(e)).idx, false, inf(source(e)).idx, e);
			epts.push_back(sce);
		}
	}

	std::sort(epts.begin(), epts.end());

	return epts;
}

SPGPtr SCG::CreateSinglePartSPG() const
{
	SPGPtr ptrSPG(new ShapeParseGraph(m_ptrShapeInfo));

	graph::node part_node = ptrSPG->new_node(ShapePart());
	IntList &part_boundary = ptrSPG->attribute(part_node).boundarySegments;

	part_boundary.push_back(0);
	part_boundary.push_back(ShapeBoundarySize() - 1);

	ptrSPG->FinalizeConstruction();
	return ptrSPG;
}
/*!
	Creates an SPG given the active cuts associated with the parse.
*/
SPGPtr SCG::CreateShapeParseGraph(const EdgeMap<bool>& activeCuts) const
{
	struct CutNodeInfo {
		unsigned first; //!< Position of first endpoint in sorter list
		unsigned second; //!< Position of second endpoint in sorter list
		graph::node partNode; //!< A part node created using this cut
	};

	struct PartNodeInfo
	{
		graph::node partNode; //!< The part node
		graph::EdgeList cutEdges; //!< Cuts along the boundary of the part

		PartNodeInfo(graph::node v) { partNode = v; }
		
		void operator=(const PartNodeInfo& rhs)
		{
			partNode = rhs.partNode;
			cutEdges = rhs.cutEdges;
		}
	};

	// Create a shape parse graphs and let it know its source shape
	SPGPtr ptrSPG(new ShapeParseGraph(m_ptrShapeInfo));

	// Get the active endpts of the parse
	EndpointArray epts = GetSortedActiveEndpoints(activeCuts);

	// If there are no active endpoints it means that it is
	// a one-part parse, and is treated differently.
	if (epts.empty() && ShapeBoundarySize() > 0)
	{
		// Add a new node (shape part) in the shape parse graph
		graph::node partNode = ptrSPG->new_node(ShapePart());

		// Get a reference to the boundary of the new shape part
		IntList& partBndry = ptrSPG->attribute(partNode).boundarySegments;

		// Add to the list the first boundary point of the cut 
		// that defines the part
		partBndry.push_back(0);
		partBndry.push_back(ShapeBoundarySize() - 1);
	}
	else
	{
		std::map<graph::edge, CutNodeInfo> cut2idxMap;
		std::list<PartNodeInfo> part2cutsMap;
		graph::node partNode;

		// Build a map so that we can find the corresponding partner 
		// endpoint of each endpoint in the NEWLY SORTED array of endpoints
		for (unsigned i = 0; i < epts.size(); ++i)
		{
			if (epts[i].firstIdx)
				cut2idxMap[epts[i].arc].first = i;
			else
				cut2idxMap[epts[i].arc].second = i;
		}

		sg::BoundaryInterval bi;
		
		bi.SetBoundarySize(ShapeBoundarySize());

		for (unsigned i = 0; i < epts.size(); ++i)
		{
			const ShapeCutEndpoint& sce_i = epts[i];

			// Only consider one endpoint per boundary cut, unless it is
			// the first boundary cut, in which case, we process it twice.
			// That is, epts[0].v is the only cut that creates TWO parts.
			if (sce_i.idx >= sce_i.partnerIdx && sce_i.arc != epts[0].arc)
				continue;

			// Add a new node (shape part) in the shape parse graph
			partNode = ptrSPG->new_node(ShapePart());

			// Associate the new node with the cut node by storing it in the 
			// auxiliar info of the cut. NOTE: when sce_i.v == epts[0].v,
			// the cut2idxMap[sce_i.v].partNode will be set twice, overwriting
			// its previous value. That is okay because we only want to associate
			// one single part node with the cut, since that's enough to add
			// an edge between the acual two parts nodes created using epts[0].v
			cut2idxMap[sce_i.arc].partNode = partNode;

			// Get a reference to the boundary of the new shape part
			IntList& partBndry = ptrSPG->attribute(partNode).boundarySegments;

			// Add to the list the first boundary point of the cut 
			// that defines the part
			partBndry.push_back(sce_i.idx);

			// Add all pairs of boundary segment endpoints found
			// between the first and last point of the cut that defines the part
			// j is the index of the next cut's endpoint
			//DBG_PRINT3(i, sce_i.idx, sce_i.partnerIdx)

			part2cutsMap.push_back(PartNodeInfo(partNode));
			graph::EdgeList& cutsInPart = part2cutsMap.back().cutEdges;
			cutsInPart.push_back(sce_i.arc);

			bi.SetLast(sce_i.partnerIdx);

			for (unsigned j = i + 1; ; ++j)
			{
				// The list is circular
				if (j == epts.size())
					j = 0;

				const ShapeCutEndpoint& sce_j = epts[j];

				// When we find the partner endpoint of the reference cut, we are done
				if (sce_j.idx == sce_i.partnerIdx)
					break;

				//DBG_PRINT4("\t", j, sce_j.idx, sce_j.partnerIdx)

				// The interval between the first endpoint of the current boundary cut
				// and the last endpoint of the reference cut must include the last
				// endpoint of the current boundary cut in order to be valid
				bi.SetFirst(sce_j.idx);

				if (!bi.Includes(sce_j.partnerIdx))
					continue;

				// The cut is valid...

				// Add the bndary cut to the list of cuts in the part
				cutsInPart.push_back(sce_j.arc);

				// Add both endpoints of the current cut
				partBndry.push_back(sce_j.idx);
				partBndry.push_back(sce_j.partnerIdx);

				// If the partner endpoint of the current cut is equal to 
				// that of the reference cut, we are done
				if (sce_j.partnerIdx == sce_i.partnerIdx)
					break;

				// Move to the partner endpoint by finding its index
				if (sce_j.firstIdx)
					j = cut2idxMap[sce_j.arc].second;
				else
					j = cut2idxMap[sce_j.arc].first;
			}

			// Add to the last boundary point of the cut that defines the part
			partBndry.push_back(sce_i.partnerIdx);

			// Add the gap formed by the first cut
			partBndry.push_back(sce_i.idx);
		}

		// Add the edges of the graph
		std::list<PartNodeInfo>::const_iterator partIt;
		graph::EdgeList::const_iterator it;

		for (partIt = part2cutsMap.begin(); partIt != part2cutsMap.end(); ++partIt)
		{
			for (it = partIt->cutEdges.begin(); it != partIt->cutEdges.end(); ++it)
			{
				const CutNodeInfo& cni = cut2idxMap[*it];

				if (partIt->partNode != cni.partNode)
				{
					ptrSPG->new_edge(partIt->partNode, cni.partNode,
						ShapePartAttachment());
				}
			}
		}
	}

	// Finalize the contruction of the graph by 
	// seting node/edge indices and computing shape descriptors
	ptrSPG->FinalizeConstruction();

	return ptrSPG;
}

