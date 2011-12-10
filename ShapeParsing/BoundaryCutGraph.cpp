/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BoundaryCutGraph.h"
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
#include <Tools/ColorMatrix.h>
#include <map>

using namespace vpl;

extern UserArguments g_userArgs;

BCG::Params BCG::s_params;

/*!
	
*/
bool BoundaryCutEndpoint::operator<(const BoundaryCutEndpoint& rhs) const
{
	if (idx < rhs.idx)
		return true;
	else if (idx > rhs.idx)
		return false;

	// We need to know the number of boundary points in order to define an interval
	const unsigned N = static_cast<const BoundaryCutGraph*>
		(graph_of(v))->ShapeBoundarySize();

	sg::BoundaryInterval bi;

	if (idx < partnerIdx)
		bi.Set(N, partnerIdx, idx);
	else
		bi.Set(N, idx, partnerIdx);

	return (bi.Includes(rhs.partnerIdx)) ? false : true;
}

/*!
	[static] Reads the ShapeParseGraph parameters provided by the user
*/
void BCG::ReadParamsFromUserArguments()
{
	// Read the parameters for the ShapeParseGraph class because
	// the BCG is the class in charge of creating them!
	ShapeParseGraph::ReadParamsFromUserArguments();

	// Read the BCG parameters
	g_userArgs.ReadBoolArg("BoundaryCutGraph", "drawLabels", 
		"Whether to draw node labels or not", true, &s_params.drawLabels);

	g_userArgs.ReadBoolArg("BoundaryCutGraph", "drawNodes", 
		"Whether to draw nodes or not", true, &s_params.drawNodes);

	g_userArgs.ReadBoolArg("BoundaryCutGraph", "drawEdges", 
		"Whether to draw edges or not", true, &s_params.drawEdges);
}

/*!
*/
void BCG::SortActiveEndpoints(unsigned parseId)
{
	const unsigned len = ShapeBoundarySize();
	BoundaryCutEndpoint bce;
	node v;

	// Get a reference to the active endpts for parse 'parseId'
	EndpointArray& epts = m_activeEndpts[parseId];

	epts.clear();

	// The maximum num of endpts is the num of nodes times 2
	epts.reserve(number_of_nodes() * 2);

	forall_nodes(v, *this)
	{
		const BCGNodeAtt& a = inf(v);

		if (a.active[parseId])
		{
			bce.Set(a.bc.From(), true, a.bc.To(), v);
			epts.push_back(bce);

			bce.Set(a.bc.To(), false, a.bc.From(), v);
			epts.push_back(bce);
		}
	}

	std::sort(epts.begin(), epts.end());
}

/*!
*/
void BCG::ComputeShapeParses(unsigned k)
{
	struct CutNodeInfo {
		unsigned first; //!< Position of first endpoint in sorter list
		unsigned second; //!< Position of second endpoint in sorter list
		graph::node partNode; //!< A part node created using this cut
	};

	struct PartNodeInfo
	{
		graph::node partNode; //!< The part node
		graph::NodeList cutNodes; //!< Cuts along the boundary of the part

		PartNodeInfo(graph::node v) { partNode = v; }
		
		void operator=(const PartNodeInfo& rhs)
		{
			partNode = rhs.partNode;
			cutNodes = rhs.cutNodes;
		}
	};

	std::map<graph::node, CutNodeInfo> cut2idxMap;
	std::list<PartNodeInfo> part2cutsMap;
	graph::node partNode;

	m_activeEndpts.resize(k);

	m_parses.clear();

	// Create k shape parse graphs
	m_parses.resize(k);

	node v;

	// Let all nodes in the BCG have a k-size bool array
	// that specifies whether the node is active in the k'th parse
	forall_nodes(v, *this)
	{
		operator[](v).active.resize(k, true);
	}

	// For each parse, use the active nodes (boundary cuts) to create
	// a parse graph
	for (unsigned parseId = 0; parseId < k; ++parseId)
	{
		SortActiveEndpoints(parseId);

		// Let the Shape Parse know its source shape
		m_parses[parseId].SetShapeInfo(m_ptrShapeInfo);

		// Get a const reference to the active endpts for parse 'parseId'
		const EndpointArray& epts = m_activeEndpts[parseId];

		cut2idxMap.clear();
		part2cutsMap.clear();

		// First, build a map so that we can find the corresponding partner 
		// endpoint of each endpoint in the NEWLY SORTED array of endpoints
		for (unsigned i = 0; i < epts.size(); ++i)
		{
			if (epts[i].firstIdx)
				cut2idxMap[epts[i].v].first = i;
			else
				cut2idxMap[epts[i].v].second = i;
		}

		sg::BoundaryInterval bi;
		
		bi.SetBoundarySize(ShapeBoundarySize());

		for (unsigned i = 0; i < epts.size(); ++i)
		{
			const BoundaryCutEndpoint& bce_i = epts[i];

			// Only consider one endpoint per boundary cut, unless it is
			// the first boundary cut, in which case, we process it twice.
			// That is, epts[0].v is the only cut that creates TWO parts.
			if (bce_i.idx >= bce_i.partnerIdx && bce_i.v != epts[0].v)
				continue;

			// Add a new node (shape part) in the shape parse graph
			partNode = m_parses[parseId].new_node(ShapePart());

			// Associate the new node with the cut node by storing it in the 
			// auxiliar info of the cut. NOTE: when bce_i.v == epts[0].v,
			// the cut2idxMap[bce_i.v].partNode will be set twice, overwriting
			// its previous value. That is okay because we only want to associate
			// one single part node with the cut, since that's enough to add
			// an edge between the acual two parts nodes created using epts[0].v
			cut2idxMap[bce_i.v].partNode = partNode;

			// Get a reference to the boundary of the new shape part
			IntList& partBndry = m_parses[parseId][partNode].boundarySegments;

			// Add to the list the first boundary point of the cut 
			// that defines the part
			partBndry.push_back(bce_i.idx);

			// Add all pairs of boundary segment endpoints found
			// between the first and last point of the cut that defines the part
			// j is the index of the next cut's endpoint
			//DBG_PRINT3(i, bce_i.idx, bce_i.partnerIdx)

			part2cutsMap.push_back(PartNodeInfo(partNode));
			graph::NodeList& cutsInPart = part2cutsMap.back().cutNodes;
			cutsInPart.push_back(bce_i.v);

			bi.SetLast(bce_i.partnerIdx);

			for (unsigned j = i + 1; ; ++j)
			{
				// The list is circular
				if (j == epts.size())
					j = 0;

				const BoundaryCutEndpoint& bce_j = epts[j];

				// When we find the partner endpoint of the reference cut, we are done
				if (bce_j.idx == bce_i.partnerIdx)
					break;

				//DBG_PRINT4("\t", j, bce_j.idx, bce_j.partnerIdx)

				// The interval between the first endpoint of the current boundary cut
				// and the last endpoint of the reference cut must include the last
				// endpoint of the current boundary cut in order to be valid
				bi.SetFirst(bce_j.idx);

				if (!bi.Includes(bce_j.partnerIdx))
					continue;

				// The cut is valid...

				// Add the bndary cut to the list of cuts in the part
				cutsInPart.push_back(bce_j.v);

				// Add both endpoints of the current cut
				partBndry.push_back(bce_j.idx);
				partBndry.push_back(bce_j.partnerIdx);

				// If the partner endpoint of the current cut is equal to 
				// that of the reference cut, we are done
				if (bce_j.partnerIdx == bce_i.partnerIdx)
					break;

				// Move to the partner endpoint by finding its index
				if (bce_j.firstIdx)
					j = cut2idxMap[bce_j.v].second;
				else
					j = cut2idxMap[bce_j.v].first;
			}

			// Add to the last boundary point of the cut that defines the part
			partBndry.push_back(bce_i.partnerIdx);
		}

		// Add the edges of the graph
		std::list<PartNodeInfo>::const_iterator partIt;
		graph::NodeList::const_iterator it;

		for (partIt = part2cutsMap.begin(); partIt != part2cutsMap.end(); ++partIt)
		{
			for (it = partIt->cutNodes.begin(); it != partIt->cutNodes.end(); ++it)
			{
				const CutNodeInfo& cni = cut2idxMap[*it];

				if (partIt->partNode != cni.partNode)
				{
					m_parses[parseId].new_edge(partIt->partNode, cni.partNode,
						ShapePartAttachment());
				}
			}
		}

		// Finalize the contruction of the graph...
		m_parses[parseId].FinalizeConstruction();
	}
}

//! [static] Update drawing parameters
void BCG::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("nodes", "draw nodes", 'n', &s_params.drawNodes));
	cmds.push_back(UserCommandInfo("edges", "draw edges", 'e', &s_params.drawEdges));
	cmds.push_back(UserCommandInfo("labels", "draw labels", 'l', &s_params.drawLabels));
}

/*!
	Draw the information associated with the BCG
*/
void BCG::Draw(unsigned parseId) const
{
	if (s_params.drawEdges)
	{
		edge e;

		LineList arcs;

		forall_edges(e, *this)
		{
			arcs.push_back(std::make_pair(
				inf(source(e)).pt, 
				inf(target(e)).pt));
		}

		SetDrawingColor(NamedColor("MediumSlateBlue"));

		DrawLines(arcs);
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

	if (s_params.drawLabels)
	{
		NodeList nl;
		node v;

		dfs_node_list(nl);

		int idx = 0;

		forall_node_items(v, nl)
		{
			if (degree(v) == 1)
				SetDrawingColor(NamedColor("Black"));
			else
				SetDrawingColor(NamedColor("Blue"));

			DrawNumber(idx++, inf(v).pt);
		}
	}

	SetDefaultDrawingColor();
}

std::string BCG::GetOutputText(unsigned parseId) const
{
	std::ostringstream oss;

	oss << "There are " << number_of_nodes() << " nodes and "
		<< number_of_edges() << " edges."; 

	return oss.str();
}

