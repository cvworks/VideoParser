/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>
#include <GraphTheory/GraphMatcher.h>
#include <ShapeParsing/ShapeParsingModel.h>
#include <ShapeParsing/ShapePartComp.h>
//#include "ModelHierarchy.h"

namespace vpl {

class ObjectLearner;
class ShapeParser;

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class ShapeMatcher : public VisSysComponent
{
	struct Params
	{
		int matchingAlgorithm;
	};

	Params m_params;

protected:
	std::shared_ptr<const ObjectLearner> m_pObjectLearner;
	std::shared_ptr<const ShapeParser> m_pShapeParser;

	GraphMatcher* m_pGraphMatcher;

	/*!
		Prepare an object to measure the similarity of nodes
		This reads user arguments to decide which similarity
		function to use, and allocates all needed resources.
	*/
	ShapePartComp m_spsm;

	//Matrix m_distances;

public:	
	ShapeMatcher()
	{
		m_pGraphMatcher = NULL;
	}

	/*const Matrix& Distances() const
	{
		return m_distances;
	}*/

	int GetMatchingAlgorithm() const
	{
		return m_params.matchingAlgorithm;
	}

	std::vector<unsigned> ComputeDatabaseDistances(Matrix& distances, 
		bool randomizeData, unsigned seed = 0) const;

	double Match(const ShapeParseGraph& g1, const ShapeParseGraph& g2) const;

	double GetGraphDistanceS2F() const;

	void GetF2SNodeMap(NodeMatchMap& nodeMap) const;

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "ShapeMatcher";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ObjectLearner");
		deps.push_back("ShapeParser");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;
	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const
	{
		InitArrays(2, pMinVals, pMaxVals, pSteps, 0, 0, 1);
	}
	
	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Distance matrix";
		}

		return "error";
	}
};

} // namespace vpl

