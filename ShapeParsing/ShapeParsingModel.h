/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeCutGraph.h"
#include <Tools/TruthTable.h>
#include <set>
#include <algorithm>

namespace vpl {

//! Random binary cut variable
struct CutVariable
{
	graph::edge cutEdge; //!< Edge in the shape cut graph

	std::list<unsigned> cliques; //!< Cliques that containt the variable

	std::list<graph::node> nodes; //!< Nodes that contain the variable
};

//! Clique of random binary cut variables
struct CVClique
{
	unsigned cycleId; //!< Id of the in SCG inducing this clique

	std::vector<unsigned> variables; //!< Indices of the random variables in the clique

	std::vector<unsigned> reminder;  //!< Indices of the variables in remainder set
	std::vector<unsigned> separator; //!< Indices of the variables in separator set

	CVClique() 
	{
	
	}

	void operator=(const CVClique& rhs)
	{
		cycleId   = rhs.cycleId;
		variables = rhs.variables;
		reminder  = rhs.reminder;
		separator = rhs.separator;
	}

	/*! 
		Finds the index of the variable. If there is no such variable,
		the size of the clique is returned.
	*/
	unsigned Find(unsigned varId) const
	{
		for (unsigned i = 0; i < variables.size(); ++i)
			if (variables[i] == varId)
				return i;

		return variables.size();
	}

	unsigned int Size() const
	{
		return variables.size();
	}

	void Serialize(OutputStream& os) const
	{
		::Serialize(os, variables);
	}

	void Deserialize(InputStream& is)
	{
		::Deserialize(is, variables);
	}
};

//! Dependency between a pair of cliques of cut variables
struct CVCliqueDependency
{
	std::list<unsigned> sharedVars;

	CVCliqueDependency()
	{

	}

	void operator=(const CVCliqueDependency& rhs)
	{
		sharedVars = rhs.sharedVars;
	}

	void Serialize(OutputStream& os) const
	{
		
	}

	void Deserialize(InputStream& is) 
	{
		
	}
};

typedef AttributedGraph<CVClique, CVCliqueDependency> SPM_BASE_CLASS;

/*!
	Parsing model of a shape into parts and part attachments. It represents
	a forest of maximal cliques with a unique path or no path between any pair
	of maximal cliques.
*/
class ShapeParsingModel : public SPM_BASE_CLASS
{
public:
	struct Params
	{
		TruthTable<2> priors;
		bool showLabels;
	};

protected:
	ShapeInfoPtr m_ptrShapeInfo; //!< Basic shape information. eg, its boundary, corners, etc
	SCGPtr m_ptrShapeCutGraph;  //!< Shape cut graph associated with the model
	std::vector<CutVariable> m_variables; //!< Binary random variables representing the state of each shape cut

	std::list<node> m_roots; //!< Roots of all trees in the junction forest
	unsigned m_maxNumParses; //!< Maximum number of parses computed

	static Params s_params;

protected:
	std::vector<SPGPtr> m_parses; //!< Array of K most probable shape parses

public:
	ShapeParsingModel() : SPM_BASE_CLASS(graph::DIRECTED)
	{
		m_maxNumParses = 0;
	}

	ShapeParsingModel(const ShapeParsingModel& rhs) 
		: SPM_BASE_CLASS(rhs)
	{
		//ASSERT(rhs.empty());
		//m_maxNumParses = 0;
		operator=(rhs);
	}

	const TruthTable<2>& GetPriors() const
	{
		return s_params.priors;
	}

	unsigned MaxNumParses() const
	{
		return m_maxNumParses;
	}

	bool Empty() const
	{
		return (SPM_BASE_CLASS::empty() && m_variables.empty()
			&& m_maxNumParses == 0);
	}

	void Create(ShapeInfoPtr ptrShapeInfo, unsigned maxNumParses)
	{
		//ShowStatus("Computing shape cut graph...");

		// Create a shape cut graph with the shape information
		SCGPtr ptrSCG(new ShapeCutGraph(ptrShapeInfo));

		Create(ptrShapeInfo, ptrSCG, maxNumParses);
	}

	void Create(ShapeInfoPtr ptrShapeInfo, SCGPtr ptrShapeCutGraph,
		unsigned maxNumParses);

	void clear()
	{
		SPM_BASE_CLASS::clear();

		m_variables.clear();

		m_maxNumParses = 0;
	}

	void operator=(const ShapeParsingModel& rhs)
	{
		ASSERT(rhs.empty() && m_variables.empty());

		m_ptrShapeInfo = rhs.m_ptrShapeInfo;
		m_ptrShapeCutGraph = rhs.m_ptrShapeCutGraph;
		m_maxNumParses = rhs.m_maxNumParses;
	}

	void ComputeShapeParses();

	//! Gets the shape information
	const ShapeInformation& GetShapeInfo() const 
	{ 
		return *m_ptrShapeInfo; 
	}

	//! Gets the shape cut graph
	const SCG& GetSCG() const 
	{
		return *m_ptrShapeCutGraph;
	}

	const ShapeCutPath& GetCycle(node v) const
	{
		return m_ptrShapeCutGraph->Cycle(inf(v).cycleId);
	}

	const std::vector<CutVariable>& GetCutVariables() const
	{
		return m_variables;
	}

	const CutVariable& GetCutVariable(unsigned varId) const
	{
		ASSERT(varId < m_variables.size());

		return m_variables[varId];
	}

	void Serialize(OutputStream& os) const
	{
		//SPM_BASE_CLASS::Serialize(os);

		::Serialize(os, m_maxNumParses);

		m_ptrShapeInfo->Serialize(os);
	}

	void Deserialize(InputStream& is) 
	{
		//SPM_BASE_CLASS::Deserialize(is);
		unsigned K;

		::Deserialize(is, K);

		m_ptrShapeInfo->Deserialize(is);

		Create(m_ptrShapeInfo, K);
	}

	unsigned NumberOfParses() const
	{
		return m_parses.size();
	}

	const std::vector<SPGPtr>& GetShapeParses() const
	{
		return m_parses;
	}

	const ShapeParseGraph& GetShapeParse(unsigned parseId) const
	{
		return *m_parses[parseId];
	}

	/*void SetShapeInfo(ShapeInfoPtr ptrShapeInfo)
	{
		m_ptrShapeInfo = ptrShapeInfo;
	}*/

	void DrawTree(node root, const Point& treeCenter, 
		const double& N_w, const double& N_h) const;

	void Draw(unsigned partId = 0) const
	{
		Draw(UIntArray(), partId);
	}

	void Draw(const UIntArray& colorIndices, unsigned partId = 0) const;

	std::string GetOutputText() const;

	////////////////////////////////////////////////////////////////////////////
	// Static functions

	static void ReadParamsFromUserArguments();

	static void GetSwitchCommands(std::list<UserCommandInfo>& cmds);
};

//! Handy alias for the class ShapeParsingModel
typedef ShapeParsingModel SPM;

//! Shared pointer to a ShapeParsingModel
typedef std::shared_ptr<ShapeParsingModel> SPMPtr;

} // namespace vpl
