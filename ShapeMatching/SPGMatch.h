/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/STLUtils.h>
#include <GraphTheory/GraphMatcher.h>
#include <ShapeParsing/ShapeParseGraph.h>
//#include <ShapeParsing/ShapePartComp.h>

namespace vpl {

struct SPGMatch
{
	enum {SIMILARITY_NOT_SET = -1, COST_NOT_SET = INT_MAX};

	double value; //!< Either a positive cost or a positive similarity value

	unsigned modelViewIdx;
	unsigned modelParseIdx;
	unsigned queryParseIdx;

	NodeMatchMap nodeMap; //!< Query nodes to model nodes correspondences

	static int ValueNotSet()
	{
		return COST_NOT_SET;
	}

	/*! 
		Sets initial value to either COST_NOT_SET or SIMILARITY_NOT_SET.
		The choice depends on teh implementation.
	*/
	SPGMatch()
	{
		value = ValueNotSet();
	}

	SPGMatch(const SPGMatch& rhs) : nodeMap(rhs.nodeMap)
	{
		SetBasicInfo(rhs);
	}

	SPGMatch(SPGMatch&& rhs) : nodeMap(std::move(rhs.nodeMap))
	{
		SetBasicInfo(rhs);
	}

	void operator=(const SPGMatch& rhs)
	{
		SetBasicInfo(rhs);

		nodeMap = rhs.nodeMap;
	}

	void operator=(const SPGMatch&& rhs)
	{
		//std::cout << "=" << std::endl;
		SetBasicInfo(rhs);

		nodeMap = std::move(rhs.nodeMap);
	}

	void SetBasicInfo(double val, unsigned mv_idx, 
		unsigned mp_idx, unsigned qp_idx)
	{
		value = val;
		modelViewIdx = mv_idx;
		modelParseIdx = mp_idx;
		queryParseIdx = qp_idx;
	}

	void SetBasicInfo(const SPGMatch& rhs)
	{
		value = rhs.value;
		modelViewIdx = rhs.modelViewIdx;
		modelParseIdx = rhs.modelParseIdx;
		queryParseIdx = rhs.queryParseIdx;
	}

	void weightValue(double weight)
	{
		value *= 1/weight; // is this a good weighting scheme? probably not...
	}

	//! Used to sort similarity values (ascending)
	bool operator>(const SPGMatch& rhs) const
	{
		ASSERT(ValueNotSet() == SIMILARITY_NOT_SET);

		return (value > rhs.value);
	}

	//! Used to sort cost values (descending)
	bool operator<(const SPGMatch& rhs) const
	{
		//std::cout << "lt" << std::endl;
		ASSERT(ValueNotSet() == COST_NOT_SET);
		
		return value < rhs.value;
	}
};

} // namespace vpl
