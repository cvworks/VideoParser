/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/BeliefPropagation.h>
#include <Tools/HelperFunctions.h>
#include <GraphTheory/GraphAlgorithms.h>
#include "ShapeParsingModel.h"
//#include <vpdl/vpdl_gaussian.h>

namespace vpl {

class ParsingPotential : public CliquePotential
{
	const ShapeParsingModel* m_pModel;  //!< Shape parsing model
	const ShapeCutGraph& m_scg; //!< Helper reference to the SCG
	const TruthTable<2>& m_table; //!< Truth table with prior probabilities

	struct Params
	{
		double m_minCosAngParallel;

		double m_nonParallelPr;
		double m_maxRelLenghtPr;
		double m_equalOrLongerLenghtPr;
		double singleCutPr;
	};

	Params m_params;

	//! List of the cuts in adjacent (first) and opposite (decond) roles.
	typedef std::pair<std::list<graph::edge>, std::list<graph::edge>> CutRoles;

	//! List of the cut roles wrt to a given cut for each cycle of the cut.
	typedef std::list<CutRoles> CutRolesPerCycle;

public:
	ParsingPotential(const ShapeParsingModel* pModel) 
		: m_pModel(pModel), m_scg(pModel->GetSCG()), m_table(pModel->GetPriors())
	{
		m_params.m_minCosAngParallel = 0.5;
		m_params.m_nonParallelPr = 0.5;
		m_params.m_maxRelLenghtPr = 1;
		m_params.m_equalOrLongerLenghtPr = 0.1;
		m_params.singleCutPr = 1;
	}

	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		CutRolesPerCycle roles;
		graph::edge cutEdge;
		//double relLenPr, parPr, adjPr, pr;
		unsigned varId;
		double setPr = 1;
		TruthTable<2>::States states;

		for (unsigned i = 0; i < R.size(); ++i)
		{
			varId = R[i];

			// Find the adjacent and opposite cuts to the given variable in each cycle
			roles = FindCutRolesPerCycle(varId);

			cutEdge = m_pModel->GetCutVariable(varId).cutEdge;

			states[0] = IsShortestCutInAllCycles(cutEdge, roles);
			states[1] = HasParallelCutInAnyCycle(cutEdge, roles);

			// find the probability of...
			if (r[i]) //...making a cut given states
				setPr *= m_table[states];
			else //...NOT making a cut given states
				setPr *= 1 - m_table[states];
		}

		/*for (unsigned i = 0; i < R.size(); ++i)
			setPr *= (r[i]) ? 0.49 : 0.51;

		return setPr;*/

		/*for (unsigned i = 0; i < R.size(); ++i)
		{
			varId = R[i];

			cutEdge = m_pModel->GetCutVariable(varId).cutEdge;

			roles = FindCutRolesPerCycle(varId);

			relLenPr = RelativeLenghtPr(r[i], m_scg.CutLength(cutEdge), 
				AverageAdjCutLength(cutEdge, roles));

			// See if the opposite node is in the separator variables
			unsigned y = (S.empty()) ? 0 : S.front();

			parPr = m_params.m_nonParallelPr;
			adjPr = 0.5;

			SetConditionalPr(cutEdge, r[i], roles, S, s, S.size(), &adjPr, &parPr);
			SetConditionalPr(cutEdge, r[i], roles, R, r, i, &adjPr, &parPr);

			if (r[i] &&
				relLenPr == m_params.m_equalOrLongerLenghtPr &&
				parPr > m_params.m_nonParallelPr)
			{
				pr = adjPr * parPr;
			}
			else
			{
				pr = relLenPr * adjPr * parPr;
			}

			setPr *= pr;
		}*/

		return setPr;
	}

protected:
	double GaussianFunction(const double& x, const double& mu, const double& s) const
	{
		double a = 1.0 / (s * std::sqrt(2 * M_PI));
		double d = x - mu;

		return a * std::exp(- (d * d) / (2 * s * s));
	}

	bool IsShortestCutInAllCycles(graph::edge cutEdge,
		const CutRolesPerCycle& roles) const
	{
		// Look for adjacent edges on each of the two
		// endpoints of e0, which are added to an array.
		graph::node nv[2] = {source(cutEdge), target(cutEdge)};

		bool isShortestCut = true;
		double cutLen = m_scg.CutLength(cutEdge);

		// Iterate over the roles of cuts in each cycle to which
		// cutEdge belogs.
		for (auto it = roles.begin(); it != roles.end() && isShortestCut; ++it)
		{
			// Iterate over the adjacent (first) edges to cutEdge
			for (auto edgeIt = it->first.begin(); 
				edgeIt != it->first.end(); ++edgeIt)
			{
				if (cutLen * 2 >= m_scg.CutLength(*edgeIt))
				{
					isShortestCut = false;
					break;
				}
			}
		}
		
		return isShortestCut;
	}

	bool HasParallelCutInAnyCycle(graph::edge cutEdge,
		const CutRolesPerCycle& roles) const
	{
		return false;
	}

	void SetConditionalPr(graph::edge cutEdge, bool cutState,
		                  const CutRolesPerCycle& roles,
		                  const RandomVariables& X, const BinaryInstance& x, 
		                  unsigned N, double* pAdjPr, double* pParPr) const
	{
		CutRolesPerCycle::const_iterator roleIt;

		for (unsigned j = 0; j < N; ++j)
		{
			roleIt = FindOppositeCut(X[j], roles);

			if (roleIt != roles.end()) // S[j] is opposite
			{
				double pr = ParallelPr(cutEdge, *roleIt);

				if (pr > m_params.m_nonParallelPr)
				{
					if ((cutState && x[j]) || (!cutState && !x[j]))
						*pParPr = pr;
					else
						*pParPr = 1 - pr;
				}
			}	
			else // S[j] is adjacent
			{
				if (cutState && x[j])
					*pAdjPr = 0;
			}
		}
	}

	/*!
		Checks if the varId has an opposite cut role in any of the list
		of roles per cycles given.
	*/
	CutRolesPerCycle::const_iterator FindOppositeCut(unsigned varId, 
		const CutRolesPerCycle& roles) const
	{
		for (auto it = roles.begin(); it != roles.end(); ++it)
		{
			if (!it->second.empty() && m_scg.index(it->second.front()) == varId)
				return it;
		}

		return roles.end();
	}

	/*!
		Finds the adjacent and opposite cuts to the given variable in each cycle.
	*/
	CutRolesPerCycle FindCutRolesPerCycle(unsigned varId) const
	{
		CutRolesPerCycle roles;

		const CutVariable& cv1 = m_pModel->GetCutVariable(varId);

		//DBG_PRINT1(cv1.nodes.size())

		// Each node the var belongs to represents a cycle 
		// of length 4 or less and with an optional interior edge.
		for (auto nodeIt = cv1.nodes.begin(); nodeIt != cv1.nodes.end(); ++nodeIt)
		{
			const ShapeCutPath& cy = m_pModel->GetCycle(*nodeIt);

			// Store the (adjacent,opposite) roles in each cycle
			roles.push_back(CutRoles());

			for (auto edgeIt = cy.edges.begin(); edgeIt != cy.edges.end(); ++edgeIt)
			{
				if (*edgeIt != cv1.cutEdge)
				{
					if (SharedNode(*edgeIt, cv1.cutEdge) == nil)
						roles.back().second.push_back(*edgeIt);
					else
						roles.back().first.push_back(*edgeIt);
				}
			}

			// Don't forget to consider the dummy edge
			if (cy.dummyEdge != nil)
			{
				if (SharedNode(cy.dummyEdge, cv1.cutEdge) == nil)
					roles.back().second.push_back(cy.dummyEdge);
				else
					roles.back().first.push_back(cy.dummyEdge);
			}
		}

		return roles;
	}

	double ParallelPr(graph::edge cutEdge, const CutRoles& role) const
	{
		if (role.first.size() != 2 || role.second.size() != 1)
		{
			DBG_PRINT3(m_scg.index(cutEdge), role.first.size(), role.second.size())
			return 0;
		}

		// Ensure that there are two adjacent and one opposite cuts
		ASSERT(role.first.size() == 2 && role.second.size() == 1);

		graph::node a1 = SharedNode(cutEdge, role.first.front());
		graph::node b1 = SharedNode(role.second.front(), role.first.front());

		ASSERT(a1 != nil && b1 != nil);

		graph::node a2 = opposite(a1, cutEdge);
		graph::node b2 = opposite(b1, role.second.front());

		// Compute the angle between the segments
		Point A = m_scg.inf(a2).pt - m_scg.inf(a1).pt;
		double normA = A.norm();

		Point B = m_scg.inf(b2).pt - m_scg.inf(b1).pt;
		double normB = B.norm();

		double cosAng = VectorCosine(A.x, A.y, B.x, B.y, normA, normB);

		double relLen = (normA < normB) ? normA / normB : normB / normA;

		double pr;

		if (cosAng <= m_params.m_minCosAngParallel)
		{
			pr = m_params.m_nonParallelPr;
		}
		else
		{
			double m = (1 - m_params.m_nonParallelPr) / (1 - m_params.m_minCosAngParallel);

			pr = 0.3 * (m * cosAng - m + 1) + 0.7 * relLen;

			if (pr < m_params.m_nonParallelPr)
				pr = m_params.m_nonParallelPr;
		}

		//DBG_PRINT7(m_scg.index(cutEdge), m_scg.index(role.second.front()), 
		//	pr, cosAng, relLen, normA, normB)

		return pr;
	}

	/*double ParallelPr(graph::edge cutEdge, const CutRolesPerCycle& roles) const
	{
		double maxPr = 0;

		for (auto it = roles.begin(); it != roles.end(); ++it)
		{
			double pr = ParallelPr(cutEdge, *it);
				
			if (pr > maxPr) 
				maxPr = pr;
		}

		return (maxPr < 0) ? 0 : maxPr;
	}*/ 

	/*!
		The relative length prob is a linear function on the length ratio
		r = MIN(L1, L2) / MAX(L1, L2), and is defined such that
		the probability of two equal line segments is always 0.5.

		pr = maxRelLenghtPr if L1 / L2 = 0,
		pr = (1 - maxRelLenghtPr) if L2 / L1 = 0,
		pr = 0.5 if r = 1.
	*/
	double RelativeLenghtPr(bool cutState, double len1, double len2) const
	{
		double pr;

		// Assume true state
		if (len2 == 0)
		{
			pr = m_params.singleCutPr;
		}
		else if (len1 < len2)
		{
			double ratio = len1 / len2;

			pr = (m_params.m_equalOrLongerLenghtPr - m_params.m_maxRelLenghtPr) 
				* ratio + m_params.m_maxRelLenghtPr;
		}
		else
		{
			


			pr = m_params.m_equalOrLongerLenghtPr;
		}

		if (!cutState)
			pr = 1 - pr;

		return pr;
	}

	double AverageAdjCutLength(graph::edge cutEdge, 
		const CutRolesPerCycle& roles) const
	{
		unsigned n = 0;
		double len = 0;
		
		// We look for adjacebt edges on each of the two
		// endpoints of e0, which are added to an array.
		graph::node nv[2] = {source(cutEdge), target(cutEdge)};

		for (auto it = roles.begin(); it != roles.end(); ++it)
		{
			for (auto edgeIt = it->first.begin(); edgeIt != it->first.end(); ++edgeIt)
			{
				n++;
				len += m_scg.CutLength(*edgeIt);
			}
		}
		
		return (n > 0) ? len / n : 0;
	}
};

} // namespace vpl
