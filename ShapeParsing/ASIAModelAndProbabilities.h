/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/BeliefPropagation.h>

namespace vpl {

typedef STLInitVector<double> CPTRow;
typedef STLInitVector<CPTRow> CPT;

struct PotentialASIA1 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		CPT cpt(CPTRow(0),     // ~a ~t
			    CPTRow(0.6),   //  a ~t
			    CPTRow(0),     // ~a  t
			    CPTRow(0.4));  //  a  t

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

struct PotentialASIA2 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		//             ~t      t
		CPT cpt(CPTRow(0.2722,      0),   // ~l ~e
			    CPTRow(0,           0),   //  l ~e
			    CPTRow(0,      0.9442),   // ~l  e
			    CPTRow(0.7278, 0.0558));  //  l  e

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

struct PotentialASIA3 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		//             ~l ~e   l ~e    ~l e    l e
		CPT cpt(CPTRow(0.1523, 0.0963, 0.5279, 0.3987),   // ~b
			    CPTRow(0.8477, 0.9037, 0.4721, 0.6013));  // b

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

struct PotentialASIA4 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		//             ~l ~b   l ~b    ~l b    l b
		CPT cpt(CPTRow(0.6581, 0.1489, 0.3548, 0.0476),   // ~s 
			    CPTRow(0.3419, 0.8511, 0.6452, 0.9524));  // s

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

struct PotentialASIA5 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		//          ~b~e b~e ~be be
		CPT cpt(CPTRow(0, 0,  0,  0),  // ~d
			    CPTRow(1, 1,  1,  1));  //  d

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

struct PotentialASIA6 : public CliquePotential
{
	double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const
	{
		//            ~e  e
		CPT cpt(CPTRow(0, 0), // ~x
			    CPTRow(1, 1));    //  x

		//DBG_PRINT4(cpt.size(), cpt.front().size(), r.to_ulong(), s.to_ulong())

		ASSERT(r.to_ulong() < cpt.size() && 
			s.to_ulong() < cpt.front().size());

		return cpt[r.to_ulong()][s.to_ulong()];
	}
};

void TestASIAModel(unsigned K)
{
	typedef STLInitVector<unsigned> IVec;

	BeliefPropagationGraph bpg;

	// Create ASIA model	            
	STLInitVector<std::string> vars("a", "b", "d", "e", "l", "s", "t", "x");
	                              //'0', '1', '2', '3', '4', '5', '6', '7'

	graph::node cl1 = bpg.NewNode(IVec(0, 6), IVec(), &PotentialASIA1());
	graph::node cl2 = bpg.NewNode(IVec(4, 3), IVec(6), &PotentialASIA2());
	graph::node cl3 = bpg.NewNode(IVec(1), IVec(4, 3), &PotentialASIA3());
	graph::node cl4 = bpg.NewNode(IVec(5), IVec(4, 1), &PotentialASIA4());
	graph::node cl5 = bpg.NewNode(IVec(2), IVec(1, 3), &PotentialASIA5());
	graph::node cl6 = bpg.NewNode(IVec(7), IVec(3), &PotentialASIA6());
	
	bpg.NewEdge(cl1, cl2);
	bpg.NewEdge(cl2, cl3);
	bpg.NewEdge(cl3, cl4);
	bpg.NewEdge(cl3, cl5);
	bpg.NewEdge(cl2, cl6);

	// Test it
	bpg.FindMostProbableConfigurations(K);

	// Print it
	bpg.Print(std::cout, vars);
}

} // namespace vpl
