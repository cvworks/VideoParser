/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <ShapeRepresentation/ShapeInformation.h>
#include <ShapeRepresentation/ShapeDescriptors.h>
#include <GraphTheory/AttributedGraph.h>
#include <GraphTheory/NodeMatch.h>
#include <Tools/Point.h>
#include <Tools/STLUtils.h>
#include <Tools/Serialization.h>

#include <ShapeRepresentation/FluxSkeleton/include/ContourCurve.h>

namespace vpl {

typedef std::shared_ptr<ShapeDescriptor> ShapeDescriptorPtr;

struct ShapePart
{
	IntList boundarySegments;
	Point center;
	double nilMatchCost;

	SHAPE_DESCRIPTOR_TYPE descriptorType;
	ShapeDescriptorPtr ptrDescriptor;
	
	ShapePart(SHAPE_DESCRIPTOR_TYPE shapeDescType = VOID_DESCRIPTOR)
	{
		descriptorType = shapeDescType;
	}

	void operator=(const ShapePart& rhs)
	{
		boundarySegments = rhs.boundarySegments;
		center           = rhs.center;
		nilMatchCost     = rhs.nilMatchCost;

		descriptorType   = rhs.descriptorType;
		ptrDescriptor    = rhs.ptrDescriptor;
	}

	void Serialize(OutputStream& os) const;
	void Deserialize(InputStream& is);
};

struct ShapePartAttachment
{
	int i;

	ShapePartAttachment()
	{
		i = -42;
	}

	void Serialize(OutputStream& os) const
	{
		
	}

	void Deserialize(InputStream& is) 
	{
		
	}
};

typedef AttributedGraph<ShapePart, ShapePartAttachment> SPGBaseClass;

/*!
	parsing of a shape into parts and part attachments
*/
class ShapeParseGraph : public SPGBaseClass
{
	enum SAMPLING_TYPE  {NO_SAMPLING, ABSOLUTE_SAMPLING, PERCENTAGE_SAMPLING, CORNER_COUNT_SAMPLING};
	enum SAMPLING_SCOPE {WHOLE_SHAPE, SHAPE_PART};
	enum NIL_MATCH_SCHEME {NIl_MATCH_VALUE, NIL_MATCH_PART_COMP};

	ShapeInfoPtr m_ptrShapeInfo;  //!< Shape representation associated with the parse

public:
	struct Params
	{
		double nodeRadius;
		bool showLabels;
		SHAPE_DESCRIPTOR_TYPE descriptorType;
		unsigned boundarySubsamplingValue;
		SAMPLING_TYPE boundarySubsamplingType;
		SAMPLING_SCOPE boundarySubsamplingScope;
		bool sampleShapeCutPoints;
		bool showDescriptorInfo;
		bool showShapeInfo;
		NIL_MATCH_SCHEME nodeNilMatchScheme;
		double nodeNilMatchValue;
		unsigned corner_alpha;
	};

protected:
	static Params s_params;

public:
	static const Params& GetParams() 
	{
		return s_params;
	}

	static void SetParams(const Params& params) 
	{
		s_params = params;
	}

public: // static functions to query about the current params used
	static SHAPE_DESCRIPTOR_TYPE GetShapeDescriptorType()
	{
		return s_params.descriptorType;
	}

public:
	ShapeParseGraph() : SPGBaseClass(graph::UNDIRECTED)
	{
	}

	ShapeParseGraph(ShapeInfoPtr ptrShapeInfo) 
		: SPGBaseClass(graph::UNDIRECTED), 
		  m_ptrShapeInfo(ptrShapeInfo)
	{
	}

	/*!
		Finalizes the construction of a graph by computing
		the necessary derived values from its current data. 

		This function must be called right after the graph is 
		contructed
	*/
	void FinalizeConstruction()
	{
		// Now the the parse graph is created, we can set the indices of nodes
		// and edges, which will make some later operations easier
		set_indices();

		ComputeShapeDescriptors();
	}

	unsigned int getNumberOfBoundaryPoints() const;

	void Serialize(OutputStream& os) const
	{
		SPGBaseClass::Serialize(os);
	}

	void Deserialize(InputStream& is) 
	{
		SPGBaseClass::Deserialize(is);

		// @todo fix this!
		// The AttributedGraph base class doesn't serialize the node indices, 
		// so we recomputed after loading the graph
		set_indices();
	}

	void ComputeShapeDescriptors();

	void SetShapeInfo(ShapeInfoPtr ptrShapeInfo)
	{
		m_ptrShapeInfo = ptrShapeInfo;
	}

	void Draw(unsigned partId = 0) const
	{
		Draw(NodeMatchMap(), partId);
	}

	void Draw(const NodeMatchMap& nodeMap, unsigned partId = 0) const;

	std::string GetOutputText() const;

	const ShapePart& GetShapePart(node v) const
	{
		return dynamic_cast<const ShapePart&>(inf(v));
	}

	////////////////////////////////////////////////////////////////////////////
	// Static functions

	static void ComputeNodeSimilarityMatrix(const ShapeParseGraph& g1, 
		const ShapeParseGraph& g2, DoubleMatrix& simWeights);

	static void ReadParamsFromUserArguments();

	static void GetSwitchCommands(std::list<UserCommandInfo>& cmds);
};

//! Handy alias for the class ShapeParseGraph
typedef ShapeParseGraph SPG;

//! Shared pointer to a ShapeParseGraph
typedef std::shared_ptr<ShapeParseGraph> SPGPtr;

} // namespace vpl
