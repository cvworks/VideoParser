/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini, Chris Whiten
 *-----------------------------------------------------------------------*/
#pragma once

#include <Tools/VisSysComponent.h>
#include <Tools/STLUtils.h>
#include <ShapeMatching/SPGMatch.h>
#include <MachineLearning/ObjectLearner.h>
#include <fstream>
#include <iostream>
#include <boost/tuple/tuple_comparison.hpp>
#include <lbfgs.h>
#include "ObjectRecognition/GAPhenotype.h"

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
		bool test_against_shape_contexts;
		bool use_importance_weights;
		bool learn_importance_weights;
		bool learn_parsing_model;
		bool use_learned_parsing_model;
	};

	Params m_params;

private:
	double evaluate(GAPhenotype &pheno);
	void getClassToIndexMapping(std::map<std::string, unsigned int> &class_to_index, std::vector<std::string> classes);
	void getAllClassesInDatabase(std::vector<std::string> &classes, const ModelHierarchy &modelHierarchy);
	void getAllModelIndicesOfGivenClass(std::vector<unsigned int> &models, std::string target_class, const ModelHierarchy &model_hierarchy);
	void getModelToClassMapping(std::map<unsigned int, std::string> &model_to_class, const ModelHierarchy &model_hierarchy);
	int getQueryId(std::string class_name, int obj_id, const ModelHierarchy &model_hierarchy);

	void learnWeights();
	void learnParsingModel();
	void learnJointParsingModel();
	void loadWeights(Lookup_Table &lt);
	void loadParsingModels(std::map<std::string, int> &parsing_models);

	std::vector<std::string> all_classes;
	std::map<unsigned int, std::string> model_to_class;
	std::map<std::string, unsigned int> class_to_index;

	void testShapeContext(SPGMatch &gmatch, const ModelHierarchy &modelHierarchy);
	void findMaxClique(int query_model_id, int query_parse_id, int query_shape_part, 
						std::vector<int> model_id, std::vector<int> model_parse_id,std::vector<int> model_part_id, 
						std::vector<double> matching_distance, 
						int &clique_size, double &score);

	bool nodeInGraph(AttributedGraph<std::pair<std::string, std::vector<int> >, double> &g,
					std::vector<graph::node> nodes, std::string query_string);

	graph::node getNodeByString(AttributedGraph<std::pair<std::string, std::vector<int> >, double> &g,
					std::vector<graph::node> nodes, std::string query_string);

	// this is for evaluation purposes against shape context.
	SPGPtr CreateSinglePartSPG(const ShapeInfoPtr &sip);

protected:
	std::shared_ptr<const ObjectLearner> m_pObjectLearner;
	std::shared_ptr<const ShapeParser> m_pShapeParser;
	std::shared_ptr<const ShapeMatcher> m_pShapeMatcher;

	std::vector<QueryRanking> m_rankings;

public:	
	boost::tuple<int, int, double, int> shape_context_MAP_match;
	std::vector<boost::tuple<int, int, double, int> > shape_context_MAP_matches;
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

