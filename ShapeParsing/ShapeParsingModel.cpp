/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include <Tools/MathUtils.h>
#include "ShapeParsingModel.h"
#include <Tools/LinearAlgebra.h>
#include <Tools/UserArguments.h>
#include <Tools/NamedColor.h>
#include <Tools/ColorMatrix.h>
#include <Tools/UserArguments.h>
#include <GraphTheory/GraphAlgorithms.h>
#include <Tools/BeliefPropagation.h>
#include <Tools/STLMatrix.h>
#include <Tools/LinearAlgebra.h>
#include <Tools/Num2StrConverter.h>
#include <GraphTheory/EdgeMap.h>
#include <VideoParserGUI/DrawingUtils.h>

#include <queue>

#include "ParsingProbabilities.h"
#include "ASIAModelAndProbabilities.h"

//! Proportion 'P' of the horizontal intra-node separation = (node_with / P)
#define NODE_HSEP_PROP 3.0 

//! Proportion 'P' of the vertical intra-node separation = (node_height / P)
#define NODE_VSEP_PROP 1.0 

using namespace vpl;

extern UserArguments g_userArgs;

ShapeParsingModel::Params ShapeParsingModel::s_params;

/*!
	[static] Reads the ShapeParsingModel parameters provided by the user
*/
void ShapeParsingModel::ReadParamsFromUserArguments()
{
	// Read the parameters for the ShapeParseGraph class because
	// the SPM is the class in charge of creating them!
	ShapeParseGraph::ReadParamsFromUserArguments();

	// We don't need this as a user argument really
	s_params.showLabels = false;

	g_userArgs.ReadArg("ShapeParsingModel", "priors", 
		"Sparse truth table for 2 boolean variables. Provide full "
		"state arrays and probabilities of making a shape cut: "
		"eg, {1,0,0.3;0,0,0.4}. Unspecified states (1,1) and (0,1) "
		"have zero probability. The meaning of each column is: "
		"IsShortestCutInAllCycles, HasParallelCutInAnyCycle", 
		s_params.priors, &s_params.priors);

	// State that we want to serialize all these fields
	g_userArgs.AddSerializableFieldKey("ShapeParsingModel");
}

//! [static] Update drawing parameters
void ShapeParsingModel::GetSwitchCommands(std::list<UserCommandInfo>& cmds)
{
	cmds.push_back(UserCommandInfo("labels", "show labels", 
		'l', &s_params.showLabels));
}

/*!
	Draws a rooted (sub) tree.
*/
void ShapeParsingModel::DrawTree(node root, const Point& treeCenter, 
	const double& N_w, const double& N_h) const
{
	Point p0(treeCenter.x - N_w / 2.0, treeCenter.y);
	Point p1(treeCenter.x + N_w / 2.0, treeCenter.y + N_h);

	DrawRectangle(p0, p1);

	void* font = GetFont(HELVETICA_18_FONT);

	DrawNumbersInBox(inf(root).variables, p0, p1, font, 
		CENTER_ALIGNMENT, MIDDLE_ALIGNMENT);

	int K = outdeg(root);

	// Calc size occupied by children
	double C_n = K * N_w + (K - 1) * N_w / NODE_HSEP_PROP;

	// Center children horizontally
	p0.x = treeCenter.x - C_n / 2.0;

	// Set hight of child level
	p0.y = p1.y + N_h / NODE_VSEP_PROP;

	// Calc the center of the first child node
	p0.x += N_w / 2.0;
	
	Point nodeBottomCenter(treeCenter.x, treeCenter.y + N_h);
	Point edgeTL, edgeBR;
	edge e;

	// Draw all children
	forall_adj_edges(e, root)
	{
		DrawLine(nodeBottomCenter, p0);

		edgeTL.Set(p0.x + 5, p0.y - N_h);
		edgeBR.Set(p0.x + N_w, p0.y);

		DrawNumbersInBox(ToIntVector(inf(e).sharedVars), 
			edgeTL, edgeBR, font, LEFT_ALIGNMENT, MIDDLE_ALIGNMENT);

		DrawTree(target(e), p0, N_w, N_h);

		p0.x += N_w + NODE_HSEP_PROP;
	}
}

/*!
	Draws the SPM.
*/
void ShapeParsingModel::Draw(const UIntArray& colorIndices, unsigned partId) const
{
	std::list<TreeAttributes> forest = compute_forest_info(*this, m_roots);

	const double M = 5;
	unsigned N = forest.size();

	DoubleVector a_w(N), a_h(N);
	unsigned I_w, I_h;
	unsigned i = 0;

	for (auto it = forest.begin(); it != forest.end(); ++it, ++i)
	{
		a_w(i) = it->width + (it->width - 1) / NODE_HSEP_PROP;
		a_h(i) = it->height + (it->height - 1) / NODE_VSEP_PROP;
	}

	m_ptrShapeInfo->GetImgDimensions(&I_w, &I_h);

	double N_w = (I_w - 2 * M) / (a_w.sum() + N - 1);
	double N_h = (I_h - 2 * M) / a_h.max_value();

	N_w = MIN(N_w, I_w * 0.2);
	N_h = MIN(N_h, I_h * 0.2);

	Point tree_top_left, tree_center;

	tree_top_left.y = M;

	tree_top_left.x = (I_w - a_w.sum() * N_w - (N - 1) * N_w) / 2.0;

	//DBG_PRINT5(tree_top_left.x, I_w, a_w.sum(), N, N_w)

	SetDrawingColor(NamedColor("DimGray"));

	tree_center.y = M;

	i = 0;

	for (auto it = forest.begin(); it != forest.end(); ++it, ++i)
	{
		tree_center.x = tree_top_left.x + a_w(i) * N_w / 2.0;
		
		DrawTree(it->root, tree_center, N_w, N_h);
		
		tree_top_left.x += a_w(i) * N_w + N_w;
	}	
}

std::string ShapeParsingModel::GetOutputText() const
{
	std::ostringstream oss;

	oss << "There are " << m_roots.size() << " roots, " 
		<< number_of_nodes() << " nodes and "
		<< number_of_edges() << " edges."; 

	return oss.str();
}

void ShapeParsingModel::Create(ShapeInfoPtr ptrShapeInfo, 
	SCGPtr ptrShapeCutGraph, unsigned maxNumParses)
{
	ASSERT(Empty());

	m_ptrShapeInfo = ptrShapeInfo;
	m_ptrShapeCutGraph = ptrShapeCutGraph;
	m_maxNumParses = maxNumParses;

	// Create an alias for the graph of shape cuts
	const ShapeCutGraph& scg = *m_ptrShapeCutGraph;

	// Create the cut variables
	m_variables.resize(m_ptrShapeCutGraph->NumberOfShapeCuts());
	
	// Init all cut variables from non-dummy edges in the SCG
	// The edge index of each shape cut becomes the ID of its
	// corresponidng cut variable
	{
		edge e;

		forall_edges(e, scg)
		{
			// The edges that map to shape cuts have
			// lower indices than the dummy edges
			if (scg.inf(e).isDummy)
				break;
			
			m_variables[scg.index(e)].cutEdge = e;
		}
	}
	
	// Create one "temporary" clique for each cycle
	auto cycles = scg.Cycles();
	std::vector<CVClique> cliques(cycles.size());
	unsigned cliqueId = 0;
	
	// Note that the i'th clique should correspond to the i'th cycle
	for (auto cycleIt = cycles.begin(); cycleIt != cycles.end(); 
		++cycleIt, ++cliqueId)
	{
		CVClique& cvc = cliques[cliqueId];

		// First, add all exterior cut variable IDs to the clique
		for (auto edgeIt = cycleIt->edges.begin(); 
			edgeIt != cycleIt->edges.end(); ++edgeIt)
		{
			if (scg.inf(*edgeIt).isDummy)
				continue;

			// The variable ID is the index of its corresponding 
			// edge in the shape cut graph
			unsigned varId = scg.index(*edgeIt);

			//DBG_PRINT3(varId, scg.index(source(*edgeIt)), scg.index(target(*edgeIt)))
			
			// Add the variable to the clique
			cvc.variables.push_back(varId);

			// Let the variable know that it belongs to this clique
			m_variables[varId].cliques.push_back(cliqueId);
		}
		
		// Next, add all interior cut variable IDs to the clique
		for (auto edgeIt = cycleIt->interiorEdges.begin(); 
			edgeIt != cycleIt->interiorEdges.end(); ++edgeIt)
		{
			// Dummy edges cannot be in the interior of a cycle
			ASSERT(!scg.inf(*edgeIt).isDummy);

			// The variable ID is the index of its corresponding 
			// edge in the shape cut graph
			unsigned varId = scg.index(*edgeIt);

			// Add the variable to the clique
			cvc.variables.push_back(varId);

			// Let the variable know that it belongs to this clique
			m_variables[varId].cliques.push_back(cliqueId);
		}
	}
	
	// Now that we have maximal and non-maximal cliques, we have to 
	// determine which are maximal and add the edges between them,
	// based on the number of shared variables by each pair of cliques
	STLMatrix<std::list<unsigned>> adj(cycles.size(), cycles.size());
	
	// Collect the common variables for each possible pair of cliques
	for (unsigned varId = 0; varId < m_variables.size(); ++varId)
	{
		const CutVariable& var = m_variables[varId];

		for (auto n0 = var.cliques.begin(); n0 != var.cliques.end(); ++n0)
		{
			auto n1 = n0;

			for (++n1; n1 != var.cliques.end(); ++n1)
			{
				adj(*n0, *n1).push_back(varId);
				adj(*n1, *n0).push_back(varId);
			}
		}
	}
	
	// Next, detect non-maximal cliques. ie, a clique that shares ALL its
	// variables with another clique. Start by assuming that all cliques
	// are maximal, and then check their maximality.
	std::vector<bool> maximal(cliques.size(), true);
	unsigned n_i, n_shared;

	// Since all cliques are in the adj matrix, we evaluate variables
	// in the direction i -> j only.
	for (unsigned i = 0; i < cliques.size(); ++i)
	{
		n_i = cliques[i].Size(); // num vars in clique i

		// If clique i shares n_i variables with any other 
		// clique, then clique i is not maximal
		for (unsigned j = 0; j < cliques.size(); ++j)
		{
			if (i == j)
				continue;
			
			n_shared = adj(i, j).size(); // num vars shared with clique j

			// See if ALL variables in clique i are shared with clique j
			if (n_shared >= n_i)
			{
				// n_shared can be AT MOST n_i
				ASSERT(n_shared == n_i);

				// If the number of vars in clique j is also n_i, both 
				// cliques are the same and only one is non-maximal
				if (cliques[j].Size() == n_i && i < j)
					continue; // ie, do nothing with clique i in this case
					
				// All vars are shared, so clique i is not maximal
				maximal[i] = false;

				// We are done checking the maximality of cllique i
				break;
			}
		}
	}

	// Helper structure to store info about the shared variables 
	// between pairs of cliques in order to build a junction forest
	struct VarInfo
	{
		unsigned numShared;
		node child;
		node parent;

		void set(unsigned n, node u, node v)
		{
			numShared = n;
			child = u;
			parent = v;
		}

		bool operator<(const VarInfo& rhs) const
		{
			return numShared < rhs.numShared;
		}
	};

	// Create "clique" nodes for each maximal clique and
	// init the priority queue with all nodes as potential roots
	//std::vector<node> nodes(cliques.size(), nil);
	std::priority_queue<VarInfo> pq;
	VarInfo vi;
	unsigned i;

	for (i = 0; i < cliques.size(); ++i)
	{
		if (maximal[i])
		{
			CVClique& cvc = cliques[i];

			cvc.cycleId = i; // cycle and cliques id's are the same
			
			vi.set(0, new_node(cvc), nil);

			pq.push(vi);

			// Let all variable in the node know that the belong to the node
			for (auto it = cvc.variables.begin(); it != cvc.variables.end(); ++it)
				m_variables[*it].nodes.push_back(vi.child);
		}
	}

	// Create the needed edges between all maximal clique
	// that share cut variables
	std::vector<bool> visited(cliques.size(), false);
	node u, v;
	edge e;

	while (!pq.empty())
	{
		vi = pq.top();
		pq.pop();

		i = inf(vi.child).cycleId;

		if (visited[i])
			continue;

		visited[i] = true;

		if (vi.numShared == 0)
		{
			m_roots.push_back(vi.child);
		}
		else
		{
			e = new_edge(vi.parent, vi.child, CVCliqueDependency());

			attribute(e).sharedVars = adj(inf(vi.parent).cycleId, i);
		}

		// Save the current node so that we can reuse the vi variable
		v = vi.child;

		// Add the children to the queue
		forall_nodes(u, *this)
		{
			if (visited[inf(u).cycleId])
				continue;

			std::list<unsigned>& shared = adj(i, inf(u).cycleId);

			if (!shared.empty())
			{
				vi.set(shared.size(), u, v);
				pq.push(vi);
			}
		}
	}

	// Add a node for each variable that does not belong to any clique
	for (unsigned varId = 0; varId < m_variables.size(); ++varId)
	{
		const CutVariable& var = m_variables[varId];

		if (var.cliques.empty())
		{
			v = new_node(CVClique());

			attribute(v).variables.push_back(varId);

			m_roots.push_back(v);
		}
	}

	// Set the reminder and separator sets of each clique
	forall_nodes(v, *this)
	{
		CVClique& cl = attribute(v);

		if (indeg(v) == 0) // has no separator set
		{
			cl.reminder = cl.variables;
		}
		else
		{
			auto shrd = inf(first_in_edge(v)).sharedVars;

			for (auto it = cl.variables.begin(); it != cl.variables.end(); ++it)
			{
				if (std::find(shrd.begin(), shrd.end(), *it) == shrd.end())
					cl.reminder.push_back(*it);
				else
					cl.separator.push_back(*it);
			}
		}
	}

	// Set the indices of the nodes and the edges
	set_indices();
}

void ShapeParsingModel::ComputeShapeParses()
{
	NodeMap<node> n2n(*this);
	BeliefPropagationGraph bpg;
	ParsingPotential* pPot;

	// Choose a potential based on the user arguments
	pPot = new ParsingPotential(this);

	node v;

	forall_nodes(v, *this)
	{
		const CVClique& c = inf(v);

		n2n[v] = bpg.NewNode(c.reminder, c.separator, pPot);
	}

	edge e;

	forall_edges(e, *this)
	{
		bpg.NewEdge(n2n[source(e)], n2n[target(e)]);
	}

	auto pMsg = bpg.FindMostProbableConfigurations(m_maxNumParses);

	/*std::vector<std::string> varNames(m_variables.size());
	Num2StrConverter sc(NUM_TO_STRING_BUFFER_SIZE);

	for (unsigned i = 0; i  < m_variables.size(); ++i)
	{
		varNames[i] = sc.toString(i);
	}

	bpg.Print(std::cout, varNames);*/

	// The potential is not needed any longer
	delete pPot;

	// Create the shape parse graphs
	ASSERT(pMsg->NumColumns() == 1);

	auto candidates = pMsg->GetCandidates(0);

	//std::vector<EdgeMap<bool>> configs(candidates.size());

	m_parses.resize(candidates.size());

	// Init the cut variables of each parse configuration
	for (unsigned i = 0; i < candidates.size(); ++i)
	{
		EdgeMap<bool> emap(GetSCG(), false);

		auto cand = candidates[i];
		
		for (auto it = cand.config.begin(); it != cand.config.end(); ++it)
		{
			if (it->value)
				emap[m_variables[it->varId].cutEdge] = true;
		}

		m_parses[i] = m_ptrShapeCutGraph->CreateShapeParseGraph(emap);
	}
}

