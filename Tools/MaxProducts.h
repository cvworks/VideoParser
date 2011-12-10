/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <bitset>
#include <algorithm>
#include "STLUtils.h"
#include "STLMatrix.h"

namespace vpl {

class MaxProducts
{
	typedef std::vector<const BottomUpBeliefMessage::Candidates*> 
		ConsistentCandidates;

	unsigned m_K;        //!< Maximum number of most probable config needed

	struct Product
	{
		double val;
		std::vector<unsigned> terms;
		const ConsistentCandidates* pCandidates;

		Product(const ConsistentCandidates* cc) : pCandidates(cc)
		{
			val = 0; // this is assumed in ComputeMaxProducts
			pCandidates = NULL;
		}

		void operator=(const Product& rhs)
		{
			val = rhs.val;
			terms = rhs.terms;
			pCandidates = rhs.pCandidates;
		}

		bool operator>(const Product& rhs)
		{
			return val > rhs.val;
		}
	};

protected:
	MaxProducts(const ConditionalProbability& cp, 
		const ConsistentCandidates& cc)
	{
	}

	/*!
	*/
	void ComputeMaxProducts()
	{
		// Init the list with the absolute best product
		Product bestProd(&cc);
		
		bestProd.val = cp.pr;
		bestProd.terms.resize(cc.size());

		for (unsigned i = 0; i < cc.size(); ++i)
		{
			const BottomUpBeliefMessage::Candidates& cand = *cc[i];

			bestProd.terms[i] = 0;
			bestProd.val *= cand[0].pr;
		}

		std::list<Product> prodsList(1, bestProd);

		// Recursive add derived max products
		AddMaxProducts(bestProd, m_K - 1, prodsList);

		// Sort and prune list
		prodsList.sort(std::greater<Product>());

		// Keep at most m_K elements
		prodsList.erase(iterator_at(prodsList, m_K), prodsList.end());
	}

	/*!
		Recursive adds derived max products.
	*/
	void AddMaxProducts(const Product& baseProd, unsigned k,
		std::list<Product>& prodsList)
	{
		std::vector<Product> maxprods;
		const ConsistentCandidates& cc = *baseProd.pCandidates;
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

				maxprods.push_back(prod);
			}
		}

		std::sort(maxprods.begin(), maxprods.end(), std::greater<Product>());

		maxprods.resize(k);

		// Recursively add new derived products. But, first, update k.
		// We now need at most k - 1 derived max products.
		--k;

		for (auto it = maxprods.begin(); it != maxprods.end(); ++it)
		{
			prodsList.push_back(*it);

			if (k > 0)
				AddMaxProducts(*it, k, prodsList);
		}
	}

public:
	/*!
		Constructs a 2^|R| x 2^|S| table to store all possible
		instantiations of p(R=r | S=s).
	*/
	ConditionalProbabilityTable(unsigned K,
		const RandomVariables& R, const RandomVariables& S, 
		const RandomVariables& R_p, const RandomVariables& S_p) 
		: m_K(K), m_R(R), m_S(S), m_parentMsg(R, S, R_p, S_p)
	{
		unsigned i;

		// Calc the number of rows and columns needed: 2^|R| and 2^|S|
		unsigned ni = uint_pow(2, m_R.size());
		unsigned nj = uint_pow(2, m_S.size());

		m_table.resize(ni, nj);
	}

	/*!
		Init the probabilities using a given conditional probability
		function.
	*/
	void Init(ConditionalProbabilityFunction* pFun)
	{
		for (unsigned long long i = 0; i < m_table.ni(); ++i)
		{
			BinaryInstance r(i);

			for (unsigned long long j = 0; j < m_table.nj(); ++j)
			{
				BinaryInstance s(j);

				ConditionalProbability& sp = m_table(i, j);

				sp.reminder = r;
				sp.separator = s;

				sp.pr = pFun->pr(m_R, sp.reminder, m_S, sp.separator);
			}
		}
	}

	/*!
		Updates the CTP with a message from a child node. When
		all child nodes are processed, the operator(i,j) can
		be used to retrive the (i,j) instance and create
		a new message for the parent node.
	*/
	void ReceiveChildMessage(const BottomUpBeliefMessage* pMsg)
	{
		m_childMsgs.push_back(pMsg);
	}

	/*!
		Once all messages from the child nodes are received, a new 
		bottom-up message for the parent node can be created.
	*/
	const BottomUpBeliefMessage* CreateParentMessage()
	{
		ConsistentCandidates cc(m_childMsgs.size());

		for (unsigned i = 0; i < m_table.ni(); ++i)
		{
			for (unsigned j = 0; j < m_table.nj(); ++j)
			{
				ConditionalProbability& sp = m_table(i, j);

				unsigned childIdx = 0;
				auto it = m_childMsgs.begin();

				// Get one consistent candidate per child node
				for (; it != m_childMsgs.end(); ++it, ++childIdx)
				{
					cc[childIdx] = (*it)->Get(sp.reminder, sp.separator);
				}

				m_parentMsg.Add(ComputeMaxProducts(sp, cc), 
					sp.reminder, sp.separator);				
			}
		}


		// Sorts across the S values and keeps the best K candidate instances
		m_parentMsg.SortAndPrune(m_K);

		return &m_parentMsg;
	}

	ConditionalProbability& operator()(unsigned i, unsigned j)
	{
		return m_table(i, j);
	}

	const ConditionalProbability& operator()(unsigned i, unsigned j) const
	{
		return m_table(i, j);
	}
};

} // namespace vpl
