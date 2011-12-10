#pragma once

#include "../FeatureEmbedder.h"
#include <Tools/LinearAlgebra.h>

class class_BoostMap;

namespace vpl {

/*!

*/
class BoostMapEmbedder : public FeatureEmbedder
{
public:
	struct Params
	{
		std::string root_data_directory;
		std::string complete_dataset;
		std::string training_dataset;
		unsigned number_of_triples;
		bool use_pdistances;

		std::string output_filename;
		long steps;
		bool reuse_training_data;

		bool randomize_data;
		unsigned random_data_seed;

		long retrieval_dimensions;
		long retrieval_max_k;
	};

protected:
	class_BoostMap* m_pBoostMap;
	std::vector<unsigned> m_indexMap; //!< Map from matrix indices to database indices

	Params m_params;

protected:
	void LearnEmbedding();
	void TestRetrievalAccuracy();
	void TestClassificationAccuracy();
	bool SaveMatrix(const Matrix& m, const std::string& rootDir,
		std::string filename) const;
	bool PrepareTrainingData();

public:

	BoostMapEmbedder();

	virtual ~BoostMapEmbedder();

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v)
	{
		VisSysComponent::Initialize(v);
		
		// This component only runs in offline mode
		SetRunningMode(OFFLINE_RUNNING_MODE);
	}

	virtual void Clear();

	virtual std::string ClassName() const
	{
		return "BoostMapEmbedder";
	}

	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		return "Learned embedding";
	}

	virtual void GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
	{
		dio.imageType = VOID_IMAGE;
		return;
	}

	virtual void Run();
};

} // namespace vpl
