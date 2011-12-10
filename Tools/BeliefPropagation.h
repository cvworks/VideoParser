/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <bitset>
#include <algorithm>
#include <GraphTheory\AttributedGraph.h>
#include "STLMatrix.h"
#include "STLUtils.h"
#include "MultiLineTable.h"

#define MAX_BIT_SET_SIZE 5u

namespace vpl {

typedef std::vector<unsigned> RandomVariables;
typedef std::bitset<MAX_BIT_SET_SIZE> BinaryInstance;

struct RVAssignment
{
	unsigned varId;
	bool value;

	RVAssignment() { }
	
	RVAssignment(unsigned i, bool v)
	{
		varId = i;
		value = v;
	}

	bool operator<(const RVAssignment& rhs) const
	{
		return varId < rhs.varId;
	}
};

typedef std::list<RVAssignment> RVAssignments;

/*! Simpe UINT pow function that avoids castings. Use if base and exponent 
	are small numbers.
*/
inline unsigned uint_pow(unsigned base, unsigned exponent)
{
	unsigned retval = 1;

	for (; exponent; --exponent)
		retval *= base;

	return retval;
}

inline RVAssignments MakeAssignments(const RandomVariables& vars,
	const BinaryInstance& inst)
{
	RVAssignments config;

	// Find the var names in the set
	for (unsigned i = 0; i < vars.size(); ++i)
		config.push_back(RVAssignment(vars[i], inst[i]));

	return config;
}

inline std::string MakeAssignmentString(const std::vector<std::string>& varNames, 
	RVAssignments config, bool sortVars = false)
{
	std::string str;

	if (sortVars)
		config.sort();

	for (auto it = config.begin(); it != config.end(); ++it)
	{
		if (!str.empty())
			str += ' ';

		if (!it->value)
			str += '~';

		str += varNames[it->varId];
	}

	return str;
}

inline std::string MakeAssignmentString(const std::vector<std::string>& varNames,
	const RandomVariables& vars, const BinaryInstance& inst, bool sortVars = false)
{
	return MakeAssignmentString(varNames, 
		MakeAssignments(vars, inst), sortVars);
}

inline std::string MakeAssignmentString(const std::vector<std::string>& varNames,
	const RandomVariables& vars, unsigned instVal, bool sortVars = false)
{
	return MakeAssignmentString(varNames, vars, 
		BinaryInstance((int)instVal), sortVars);
}

/*!
	Abstract probability function class. It is used to compute
	the conditional probability p(R = r | S = s).
*/
class CliquePotential
{
public:
	/*!
		Computes the conditional probability 

		p(R = r | S = s)
	*/
	virtual double pr(const RandomVariables& R, const BinaryInstance& r, 
		const RandomVariables& S, const BinaryInstance& s) const = 0;
};

/*!
	Conditional probability pr = p(R=r|S=s) processed in a bottom-up
	fashion.
	
	It stores the instantiation of the R part and the terms
	of the immediate descendants used to compute p(R=r|S=s).
*/
struct ProcessedProbability
{
	double pr; //!< Probability p(R=r|S=s)	
	RVAssignments config; //!< List of "definitely seen" RV assignments

	//! Inits pr to p or to product-identity value (ie, pr=1)
	ProcessedProbability(const double& p = 1.0)
	{
		pr = p;
	}

	ProcessedProbability(const double& p, const BinaryInstance& r, 
		const RVAssignments& rva) 
		: pr(p), config(rva)
	{
	}

	void operator=(const ProcessedProbability& rhs)
	{
		pr = rhs.pr;
		config = rhs.config;
	}

	bool operator>(const ProcessedProbability& rhs) const
	{
		return pr > rhs.pr;
	}
};

/*!
	Message from source to parent nodes.
*/
class BottomUpBeliefMessage
{
public:
	typedef std::vector<ProcessedProbability> Candidates;

protected:
	RandomVariables m_reminder; //!< Reminder set of the child
	RandomVariables m_separator; //!< Separator set of the child

	std::vector<Candidates> m_table; //!< Table of candidates for each instance of m_separator

	std::vector<unsigned> m_Rmap; //!< Map R_p values to their corresponding values in S_c

	std::vector<unsigned> m_Smap; //!< Map S_p values to their corresponding values in S_c

	/*! 
		Returns index of the variable in the array if found or 
		MAX_BIT_SET_SIZE if it is not found.

		The size of the array must be equal to or less than
		MAX_BIT_SET_SIZE.
	*/
	unsigned FindVarId(unsigned id, const std::vector<unsigned>& vars) const
	{
		ASSERT(vars.size() <= MAX_BIT_SET_SIZE);

		for (unsigned i = 0; i < vars.size(); ++i)
		{
			if (vars[i] == id)
				return i;
		}

		return MAX_BIT_SET_SIZE;
	}

public:
	BottomUpBeliefMessage()
	{
		// nothing to do. call init() to set up
	}

	//! Number of comuns in the message table
	unsigned NumColumns() const
	{
		return m_table.size();
	}

	//! Number of candidates in the given column
	unsigned NumCandidates(unsigned column) const
	{
		ASSERT(column <  NumColumns());

		return m_table[column].size();
	}

	/*!
		Prepares the maps needed to populate the message.
	*/
	void Init(const RandomVariables& R_c, const RandomVariables& S_c,
		const RandomVariables& R_p, const RandomVariables& S_p)
	{
		ASSERT(R_c.size() <= MAX_BIT_SET_SIZE
			&& S_c.size() <= MAX_BIT_SET_SIZE 
			&& R_p.size() <= MAX_BIT_SET_SIZE 
			&& S_p.size() <= MAX_BIT_SET_SIZE);

		 m_reminder = R_c;
		 m_separator = S_c; 

		 m_Rmap.resize(R_p.size());
		 m_Smap.resize(S_p.size());

		// Init the table of cabdidates to 2^|S_c|
		m_table.resize(uint_pow(2, m_separator.size()));

		// Init the maps
		unsigned i;

		for (i = 0; i < m_table.size(); ++i)
			m_table[i].reserve(m_reminder.size());

		// Create table to map R_p and S_p values to
		// their corresponding values in S_c
		for (i = 0; i < R_p.size(); ++i)
			m_Rmap[i] = FindVarId(R_p[i], S_c);
	
		for (i = 0; i < S_p.size(); ++i)
			m_Smap[i] = FindVarId(S_p[i], S_c);
	}

	/*!
		Maps the r_p in R_parent and s_p in S_parent to their
		corresponding s_child values, and returns the list of
		consistent candidates associated with s_child.

		If s_child is empty, ie, it shares no variables with
		its parent, then the first candidate list is returned.
	*/
	const Candidates* Get(const BinaryInstance& r_p, 
		const BinaryInstance& s_p) const
	{
		BinaryInstance s_c; // it is init to zeros
		unsigned i, pos;

		for (i = 0; i < m_Rmap.size(); ++i)
		{
			pos = m_Rmap[i];

			if (pos < MAX_BIT_SET_SIZE)
				s_c.set(pos, r_p[i]);
		}

		for (i = 0; i < m_Smap.size(); ++i)
		{
			pos = m_Smap[i];

			if (pos < MAX_BIT_SET_SIZE)
				s_c.set(pos, s_p[i]);
		}

		unsigned idx = s_c.to_ulong();
		
		ASSERT(idx <= m_table.size());
		
		return &m_table[idx];
	}

	/*!
		Adds a new conditional probability pr = p(R = r_c | S = s_c)
		to the candidate list of s_c.
	*/
	void Add(const BinaryInstance& r_c, const BinaryInstance& s_c, 
		const ProcessedProbability& pp)
	{
		unsigned idx = s_c.to_ulong();
		
		ASSERT(idx <= m_table.size());

		m_table[idx].push_back(pp);

		append(m_table[idx].back().config, MakeAssignments(m_reminder, r_c));
	}
	
	/*!
		Sorts the candidate list of each possible s_c instantiation
		of m_separator variables. Next, it keeps the K candidates
		with highest probability.
	*/
	void SortAndPrune(unsigned K)
	{
		for (unsigned i = 0; i < m_table.size(); ++i)
		{
			Candidates& c = m_table[i];

			std::sort(c.begin(), c.end(), 
				std::greater<ProcessedProbability>());

			if (c.size() > K)
				c.resize(K);
		}
	}

	const Candidates& GetCandidates(unsigned column) const
	{
		ASSERT(column <  NumColumns());

		return m_table[column];
	}

	/*!
		Prints the message's table.
	*/
	void Print(std::ostream& os, const std::vector<std::string>& varNames) const
    {
		unsigned numCandidates = 0;

		for (unsigned i = 0; i < m_table.size(); ++i)
			if (numCandidates < m_table[i].size())
				numCandidates = m_table[i].size();

		MultiLineTable tbl(numCandidates, m_table.size());

		for (unsigned j = 0; j < m_table.size(); ++j)
		{
			const Candidates& c = m_table[j];

			tbl.AddColHeader(j, MakeAssignmentString(
				varNames, m_separator, j));

			for (unsigned i = 0; i < c.size(); ++i)
			{
				tbl.AddValue(i, j, c[i].pr);

				tbl.AddText(i, j, MakeAssignmentString(
					varNames, c[i].config, true));
			}
		}

		tbl.Print(os);

		os << std::endl;
    }
};

/*!
	Conditional probability tables on a chain of sets of nodes.

	Variables are assumed to be binary. So, the table has size
	2^|R| x 2^|S|, where R is the set of remainder variables and
	S is is the set of separator variables.
*/
class ConditionalProbabilityTable
{
protected:
	typedef std::vector<const BottomUpBeliefMessage::Candidates*> 
		ConsistentCandidates;

	RandomVariables m_R; //!< Set of remainder random variables
	RandomVariables m_S; //!< Set of separator random variables

	STLMatrix<double> m_table; //!< 2^|R| x 2^|S| table

	std::list<const BottomUpBeliefMessage*> m_childMsgs; //!< Msgs from children
	BottomUpBeliefMessage m_parentMsg; //!< Msg to parent

	/*! 
		Product of N probability terms obtained from a list of candidate 
		lists for each term in the product.
		
		term[i]=j encodes that the i'th term in the product uses 
		the j'th candidate for that term.
	*/
	struct Product
	{
		double val;
		std::vector<unsigned> terms;

		void operator=(const Product& rhs)
		{
			val = rhs.val;
			terms = rhs.terms;
		}

		bool operator>(const Product& rhs) const
		{
			return val > rhs.val;
		}
	};

protected:
	/*!
	*/
	std::list<Product> ComputeMaxProducts(const double& init_pr, 
		const ConsistentCandidates& cc, const unsigned K) const
	{	
		// Init the list with the absolute best product
		Product bestProd;
		
		bestProd.val = init_pr;
		bestProd.terms.resize(cc.size());

		for (unsigned i = 0; i < cc.size(); ++i)
		{
			const BottomUpBeliefMessage::Candidates& cand = *cc[i];

			bestProd.terms[i] = 0;
			bestProd.val *= cand[0].pr;
		}

		std::list<Product> prodsList(1, bestProd);
		
		// Recursive add derived max products (if needed)
		if (K - 1 > 0)
		{
			AddMaxProducts(bestProd, K - 1, cc, prodsList);
			
			// Sort and prune list
			prodsList.sort(std::greater<Product>());
			
			// Keep at most K elements
			prodsList.erase(iterator_at(prodsList, K), prodsList.end());
		}

		return prodsList;
	}

	/*!
		Recursive adds derived max products.
	*/
	void AddMaxProducts(const Product& baseProd, unsigned k,
		const ConsistentCandidates& cc, std::list<Product>& prodsList) const
	{
		std::vector<Product> maxprods;
		unsigned j;
		
		maxprods.reserve(cc.size());

		// For each child list of candidates, change one term to next best
		for (unsigned i = 0; i < cc.size(); ++i)
		{
			// Get the i'th list of candidates
			const BottomUpBeliefMessage::Candidates& cand = *cc[i];

			// See which is the current best term used
			j = baseProd.terms[i];

			// See if there is one more candidate to consider
			if (j + 1 < cand.size())
			{
				// Create a new product by replacing the i'th term with
				// its next best in the i'th list
				Product prod(baseProd);

				// Move to the next candidate
				prod.terms[i] = j + 1;

				// Remove j'th term from product and apply j+1'th term
				prod.val *= cand[j + 1].pr / cand[j].pr;

				if (prod.val > 0)
					maxprods.push_back(prod);
			}
		}
		
		std::sort(maxprods.begin(), maxprods.end(), std::greater<Product>());

		if (maxprods.size() > k)
			maxprods.resize(k);

		// Recursively add new derived products. But, first, update k.
		// We now need at most k - 1 derived max products.
		ASSERT(k > 0);

		--k;

		for (auto it = maxprods.begin(); it != maxprods.end(); ++it)
		{
			prodsList.push_back(*it);

			if (k > 0)
				AddMaxProducts(*it, k, cc, prodsList);
		}
	}

public:
	/*!
		Constructs a 2^|R| x 2^|S| table to store all possible
		instantiations of p(R=r | S=s).
	*/
	ConditionalProbabilityTable(const RandomVariables& R, 
		const RandomVariables& S) : m_R(R), m_S(S)
	{
		// nothing else to do
	}

	//!	Constructs a copy of the rhs table.
	ConditionalProbabilityTable(const ConditionalProbabilityTable& rhs) 
		: m_R(rhs.m_R), m_S(rhs.m_S), m_table(rhs.m_table),
		  m_childMsgs(rhs.m_childMsgs), m_parentMsg(rhs.m_parentMsg)
	{
	}

	const RandomVariables& R() const
	{
		return m_R;
	}

	const RandomVariables& S() const
	{
		return m_S;
	}

	void InitParentMsg(const RandomVariables& R_p, const RandomVariables& S_p)
	{
		m_parentMsg.Init(m_R, m_S, R_p, S_p);
	}

	/*!
		Init the probabilities using a given conditional probability
		function.

		It constructs a 2^|R| x 2^|S| table to store all possible
		instantiations of p(R=r | S=s).
	*/
	void Init(const CliquePotential* pFun)
	{
		// Calc the number of rows and columns needed: 2^|R| and 2^|S|
		unsigned ni = uint_pow(2, m_R.size());
		unsigned nj = uint_pow(2, m_S.size());

		m_table.resize(ni, nj);

		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			BinaryInstance r((int) i);

			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				BinaryInstance s((int) j);

				m_table(i, j) = pFun->pr(m_R, r, m_S, s);
			}
		}
	}

	/*!
		Updates the CPT with a message from a child node. When
		all child nodes are processed, the operator(i,j) can
		be used to retrive the (i,j) instance and create
		a new message for the parent node.
	*/
	void ReceiveChildMessage(const BottomUpBeliefMessage* pMsg)
	{
		m_childMsgs.push_back(pMsg);
	}

	//! Number of messages from child nodes received so far
	unsigned NumMsgReceived() const
	{
		return m_childMsgs.size();
	}

	/*!
		Once all messages from the child nodes are received, a new 
		bottom-up message for the parent node can be created.
	*/
	const BottomUpBeliefMessage* CreateParentMessage(const unsigned K)
	{
		ConsistentCandidates cc(m_childMsgs.size());
		
		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			BinaryInstance r((int) i);

			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				BinaryInstance s((int) j);

				const double& pr = m_table(i, j);

				unsigned childIdx = 0;
				auto it = m_childMsgs.begin();
				
				// Get one consistent candidate per child node
				for (; it != m_childMsgs.end(); ++it, ++childIdx)
				{
					cc[childIdx] = (*it)->Get(r, s);
				}
				
				std::list<Product> mp = ComputeMaxProducts(pr, cc, K);
				
				for (auto it = mp.begin(); it != mp.end(); ++it)
				{
					// Find the RV assignments from the consistent 
					// candidates and their terms used in the product
					ProcessedProbability pp(it->val);

					for (unsigned t = 0; t < cc.size(); ++t)
						append(pp.config, cc[t]->at(it->terms[t]).config);

					m_parentMsg.Add(r, s, pp);
				}
			}
		}
		
		// Sorts across the S values and keeps the best K candidate instances
		m_parentMsg.SortAndPrune(K);

		return &m_parentMsg;
	}

	const BottomUpBeliefMessage* ComputeRootMaxProduct(const unsigned K)
	{
		// Message to parent should not be already initialized
		ASSERT(m_parentMsg.NumColumns() == 0);

		// Create a "dummy" message
		m_parentMsg.Init(m_R, m_S, RandomVariables(), RandomVariables());

		return CreateParentMessage(K);
	}

	double& operator()(unsigned i, unsigned j)
	{
		return m_table(i, j);
	}

	const double& operator()(unsigned i, unsigned j) const
	{
		return m_table(i, j);
	}

	/*!
		Prints the CPT table.
	*/
	void Print(std::ostream& os, const std::vector<std::string>& varNames) const
    {
		MultiLineTable tbl(m_table.ni(), m_table.nj());

		// Add column headers
		for (unsigned j = 0; j < m_table.nj(); ++j)
		{
			tbl.AddColHeader(j, MakeAssignmentString(
				varNames, m_S, j));
		}

		// Add cell data
		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			// Add row headers
			tbl.AddRowHeader(i, MakeAssignmentString(
				varNames, m_R, i));

			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				tbl.AddValue(i, j, m_table(i, j));
			}
		}

		os << "CPT:\n";

		tbl.Print(os);

		os << "\nBottom-up Message:\n";

		m_parentMsg.Print(os, varNames);

		os << std::endl;
    }
};

/*!

*/
class BeliefPropagationGraph : AttributedGraph<ConditionalProbabilityTable, int>
{
public:
	class IdentityPotential : public CliquePotential
	{
	public:
		virtual double pr(const RandomVariables& R, const BinaryInstance& r, 
			const RandomVariables& S, const BinaryInstance& s) const
		{
			return 1;
		}
	};

protected:
	void ProcessTree(node r, const unsigned K)
	{
		node v;

		ConditionalProbabilityTable& cpt = attribute(r);

		ASSERT(cpt.NumMsgReceived() == 0);

		// Collect messages from all child nodes
		forall_adj_nodes(v, r)
		{
			ProcessTree(v, K);

			cpt.ReceiveChildMessage(attribute(v).CreateParentMessage(K));
		}

		ASSERT(cpt.NumMsgReceived() == outdeg(r));
	}

public:
	/*!
		New super-node in a forest. Each node is a cluster of
		random variables.

		@param pPot pointer to a clique potential representing
				    the conditional probability p(R = r | S = s).
	*/
	node NewNode(const RandomVariables& R, const RandomVariables& S, 
		const CliquePotential* pPot)
	{
		node v = new_node(ConditionalProbabilityTable(R, S));

		attribute(v).Init(pPot);

		return v;
	}

	/*!	
		Directed edge from u to v.
	*/
	void NewEdge(node u, node v)
	{
		new_edge(u, v, 0);

		const ConditionalProbabilityTable& cpt = inf(u);

		attribute(v).InitParentMsg(cpt.R(), cpt.S());
	}

	//! Finds the roots in the forest
	std::list<node> FindRoots()
	{
		std::list<node> roots;

		node v;

		forall_nodes(v, *this)
		{
			if (indeg(v) == 0) // it's a root node
				roots.push_back(v);
		}

		return roots;
	}

	/*!
		Finds the K most probable joint instantiation of 
		all the random variables in the tree rooted at 'root'.
	*/
	const BottomUpBeliefMessage* FindMostProbableConfigurations(
		const unsigned K, node root)
	{
		ProcessTree(root, K);
				
		return attribute(root).ComputeRootMaxProduct(K);
	}

	/*!
		Finds the K most probable joint instantiation of 
		all the trees rooted at the given 'roots'.
	*/
	const BottomUpBeliefMessage* FindMostProbableConfigurations(
		const unsigned K, const std::list<node>& roots)
	{
		if (roots.size() == 1)
		{
			return FindMostProbableConfigurations(K, roots.front());
		}
		else
		{
			// We add a new "dummy" root and make the root nodes
			// be children of it.
			node dummyRoot = NewNode(RandomVariables(), 
				RandomVariables(), &IdentityPotential());

			for (auto it = roots.begin(); it != roots.end(); ++it)
			{
				ASSERT(indeg(*it) == 0);

				NewEdge(dummyRoot, *it);
			}

			return FindMostProbableConfigurations(K, dummyRoot);
		}
	}

	const BottomUpBeliefMessage* FindMostProbableConfigurations(
		const unsigned K)
	{
		return FindMostProbableConfigurations(K, FindRoots());
	}

	//! Print subtree
	void PrintTree(node r, std::ostream& os, 
		const std::vector<std::string>& varNames) const
	{
		node v;

		// Print children first
		forall_adj_nodes(v, r)
		{
			PrintTree(v, os, varNames);
		}

		inf(r).Print(os, varNames);
	}

	//! Print forest
	void Print(std::ostream& os, 
		const std::vector<std::string>& varNames) const
	{
		node v;

		forall_nodes(v, *this)
		{
			if (indeg(v) == 0) // it's a root node
				PrintTree(v, os, varNames);
		}
	}
};

} // namespace vpl
