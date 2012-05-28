/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

#include <fstream>
#include <Tools/VisSysComponent.h>

namespace vpl {

class ObjectLearner;
class ObjectRecognizer;

/*!
	@brief Wrapper for a generic object recognition algorithm
*/
class ResultsAnalyzer : public VisSysComponent
{
	struct Params
	{
		bool saveResults;
		std::string strResultsFilename;
		unsigned cutoffRank;
		std::string strExperimentTitle;
	};

	Params m_params;

protected:
	std::shared_ptr<const ObjectLearner> m_pObjectLearner;
	std::shared_ptr<const ObjectRecognizer> m_pObjectRecognizer;

	std::fstream m_resultsFile;
	std::fstream m_shapeContextFile;

	bool m_hasOpenedFile;
	bool m_hasOpenedDataBlock;
	int m_maxExperimentNumber;

	void OpenResultsFile();
	void SaveRecognitionResults();
	void SaveShapeContextRecognitionResults();

public:	
	virtual void Clear()
	{
		PostProcessSequence();

		VisSysComponent::Clear();
	}

	virtual void ReadParamsFromUserArguments();

	virtual void Initialize(graph::node v);

	//! This is called right after a whole video is processed.
	virtual void PostProcessSequence()
	{
		if (m_hasOpenedDataBlock)
		{
			m_resultsFile << "\n}" << std::endl;
			m_hasOpenedDataBlock = false;
		}
	}

	virtual std::string ClassName() const
	{
		return "ResultsAnalyzer";
	}

	virtual StrArray Dependencies() const
	{
		StrArray deps;

		deps.push_back("ObjectLearner");
		deps.push_back("ObjectRecognizer");
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

		//if (!m_rankings.empty())
		//	pMaxVals->at(0) = m_rankings.size() - 1;

		//if (m_modelHierarchy.ModelViewCount() > 0)
		//	pMaxVals->at(1) = m_modelHierarchy.ModelViewCount() - 1;
	}
	
	virtual int NumOutputImages() const 
	{ 
		return 1; 
	}

	virtual std::string GetOutputImageLabel(int i) const
	{
		switch (i)
		{
			case 0: return "Precision and Recall";
		}

		return "error";
	}
};

} // namespace vpl

