/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include "ShapeParseGraph.h"
#include <ShapeRepresentation/ShapeDescriptorComp.h>

namespace vpl {

/*!
	A function object representing the comparison
	of two shape parts encoded by the attributes of
	a query node and a model node.

	The comparison function used is determined from the
	SHAPE_DESCRIPTOR_TYPE of the parts that are compared.

	The advantage of this functor is that it provides acccess 
	to the container query/model graphs without requiring the 
	user to pass them as arguments every time a pair of nodes 
	are compared. That is, in general, the query and model 
	graphs are specified once, right before all their nodes 
	are compared.
*/
class ShapePartComp
{
	const ShapeParseGraph* m_pQueryGraph;
	const ShapeParseGraph* m_pModelGraph;

	ShapeDescriptorComp* m_pSDC;
	SHAPE_DESCRIPTOR_TYPE m_shapeDescriptorType;

protected:
	void InitSDC(const ShapePart& sp_q, const ShapePart& sp_m)
	{
		 ASSERT(sp_q.descriptorType == sp_m.descriptorType);

		if (!m_pSDC || m_shapeDescriptorType != sp_q.descriptorType)
			LoadShapeSimilarityMeasurer(sp_q.descriptorType);
	}

public:
	/*! 
		Constructs the functor using the shape similarity 
		function specified by the user arguments.
	*/
	ShapePartComp()
	{
		m_pSDC = NULL;
		m_shapeDescriptorType = VOID_DESCRIPTOR;
	}

	~ShapePartComp()
	{
		delete m_pSDC;
	}

	void LoadShapeSimilarityMeasurer(SHAPE_DESCRIPTOR_TYPE type);

	//! Specifies the query and model graphs
	void SetGraphs(const ShapeParseGraph& query, 
		const ShapeParseGraph& model)
	{
		m_pQueryGraph = &query;
		m_pModelGraph = &model;
	}

	/*!
		For some descriptors, this function can be used to retrieve 
		transformation parameters. It matches the descriptors
		and the retrieves its transformation parameters.
	*/
	void GetTransformationParams(graph::node u, graph::node v,
		PointTransform* pPT)
	{
		if (u == nil || v == nil)
		{
			pPT->Clear();
		}
		else
		{
			const ShapePart& sp_q = m_pQueryGraph->inf(u);
			const ShapePart& sp_m = m_pModelGraph->inf(v);

			InitSDC(sp_q, sp_m);

			m_pSDC->GetTransformationParams(*sp_q.ptrDescriptor, 
				*sp_m.ptrDescriptor, pPT);
		}
	}

	/*!
		Compares a query node 'u' against a model node 'v'
		using the shape cost or similarity function selected
		by the user. One of the nodes can be nil, in which
		case the cost of leaving the non-nil node unmatched
		is returned.
	*/
	double operator()(graph::node u, graph::node v)
	{
		const double UNMATCHED_IMPORTANCE_THRESHOLD = 0.2;
		const double SIZE_DIFFERENCE_THRESHOLD = 0.5;
		if (u == nil)
		{
			ASSERT(v != nil);
			ASSERT(m_pModelGraph->inf(v).nilMatchCost >= 0);

			unsigned int shape_part_size = m_pModelGraph->inf(v).ptrDescriptor->GetBoundaryLength();
			unsigned int shape_size = m_pModelGraph->getNumberOfBoundaryPoints();
			double relative_part_size = (double)shape_part_size / (double)shape_size;

			// if the part is large, we want to discourage it from being
			// unmatched.  If it's small, perhaps it is unimportant.
			if (relative_part_size < UNMATCHED_IMPORTANCE_THRESHOLD)
			{
				return m_pModelGraph->inf(v).nilMatchCost;
			}
			else
			{
				return m_pModelGraph->inf(v).nilMatchCost * (1 + relative_part_size);
			}
		}
		else if (v == nil)
		{
			ASSERT(m_pQueryGraph->inf(u).nilMatchCost >= 0);
			unsigned int shape_part_size = m_pQueryGraph->inf(u).ptrDescriptor->GetBoundaryLength();
			unsigned int shape_size = m_pQueryGraph->getNumberOfBoundaryPoints();
			double relative_part_size = (double)shape_part_size / (double)shape_size;

			// if the part is large, we want to discourage it from being
			// unmatched.  If it's small, perhaps it is unimportant.
			if (relative_part_size < UNMATCHED_IMPORTANCE_THRESHOLD)
			{
				return m_pModelGraph->inf(u).nilMatchCost;
			}
			else
			{
				return m_pModelGraph->inf(u).nilMatchCost * (1 + relative_part_size);
			}
		}
		else
		{
			const ShapePart& sp_q = m_pQueryGraph->inf(u);
			const ShapePart& sp_m = m_pModelGraph->inf(v);

			InitSDC(sp_q, sp_m);

			double dist = m_pSDC->Match(*sp_q.ptrDescriptor, *sp_m.ptrDescriptor);

			// relative model size.
			unsigned int shape_part_size = m_pModelGraph->inf(v).ptrDescriptor->GetBoundaryLength();
			unsigned int shape_size = m_pModelGraph->getNumberOfBoundaryPoints();
			double relative_model_part_size = (double)shape_part_size / (double)shape_size;

			// relative query size.
			shape_part_size = m_pQueryGraph->inf(u).ptrDescriptor->GetBoundaryLength();
			shape_size = m_pQueryGraph->getNumberOfBoundaryPoints();
			double relative_query_part_size = (double)shape_part_size / (double)shape_size;

			// if the difference in relative size between the two parts is significant,
			// we add that weight onto the distance to discourage such matchings.
			double bigger_part_size = std::max(relative_query_part_size, relative_model_part_size);
			double smaller_part_size = std::min(relative_query_part_size, relative_model_part_size);

			if (bigger_part_size - smaller_part_size > SIZE_DIFFERENCE_THRESHOLD)
			{
				dist = dist * (1 + (bigger_part_size - smaller_part_size));
			}


			ASSERT(dist >= 0);

			return dist;
		}
	}
};

} // namespace vpl
