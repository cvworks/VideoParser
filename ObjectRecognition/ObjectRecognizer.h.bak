/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>
#include <ShapeMatching/SPGMatch.h>

namespace vpl {

class ObjectLearner;
class ShapeParser;
class ShapeMatcher;

/*!
*/
struct QueryRanking
{
	std::vector<SPGPtr> queryParses;
	std::vector<SPGMatch> matches;
	double matchingTime;

	QueryRanking() { }

	QueryRanking(const QueryRanking& rhs)
	{
		operator=(rhs);
	}

	void operator=(const QueryRanking& rhs)
	{
		queryParses = rhs.queryParses;
		matches = rhs.matches;
		matchingTime = rhs.matchingTime;
	}
};

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class ObjectRecognizer : public VisSysComponent
{
	struct Params
	{
		//std::string modelDBPath;
		int matchingAlgorithm;
		bool excludeQueryObjectFromDB;
		bool excludeQueryViewFromDB;
		bool readParsingParamsFromDB;
		bool onlySumModelNodeMatches;
		bool overlayWarpQuery;
	};

	Params m_params;

protected:
	std::shared_ptr<const ObjectLearner> m_pObjectLearner;
	std::shared_ptr<const ShapeParser> m_pShapeParser;
	std::shared_ptr<const ShapeMatcher> m_pShapeMatcher;

	std::vector<QueryRanking> m_rankings;

public:	
	const std::vector<QueryRanking>& GetRankings() const
	{
		return m_rankings;
	}

	int GetMatchingAlgorithm() const
	{
		return m_params.matchingAlgorithm;
	}

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	virtual std::string ClassName() const
	{
		return "ObjectRecognizer";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ObjectLearner");
		deps.push_back("ShapeParser");
		deps.push_back("ShapeMatcher");

		return deps;
	}
	
	virtual void Run();

	virtual void Draw(const DisplayInfoIn& dii) const;
	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const;

	virtual void GetParameterInfo(int i, DoubleArray* pMinVals, 
		DoubleArray* pMaxVals, DoubleArray* pSteps) const;
	
	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Similar models";
		}

		return "error";
	}
};

} // namespace vpl

