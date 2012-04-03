/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>
#include <ShapeMatching/SPGMatch.h>
#include <fstream>
#include <iostream>

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

struct WeightKey
{
	int model, parse, part;
	
	WeightKey(int a, int b, int c) : model(a), parse(b), part(c) {}
	
	bool operator < (const WeightKey &rhs) const
	{
		return model < rhs.model || (model == rhs.model && parse < rhs.parse) || (model == rhs.model && parse == rhs.parse && part < rhs.part);
	}
};

typedef std::map<WeightKey, int> Lookup_Table;

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

private:
	
	void findMaxClique(int query_model_id, int query_parse_id, int query_shape_part, 
						std::vector<int> model_id, std::vector<int> model_parse_id,std::vector<int> model_part_id, 
						std::vector<double> matching_distance, 
						int &clique_size, double &score);

	bool nodeInGraph(AttributedGraph<std::pair<std::string, std::vector<int> >, double> &g,
					std::vector<graph::node> nodes, std::string query_string);

	graph::node getNodeByString(AttributedGraph<std::pair<std::string, std::vector<int> >, double> &g,
					std::vector<graph::node> nodes, std::string query_string);

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

