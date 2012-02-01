/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "ResultsAnalyzer.h"
#include "VSCGraph.h"
#include <ObjectRecognition/ObjectRecognizer.h>
#include <MachineLearning/ObjectLearner.h>
#include <ShapeParsing/ShapeParser.h>
#include <Tools/UserArguments.h>
#include <VideoParserGUI/DrawingUtils.h>
#include <Tools/ParamFile.h>
#include <Tools/DirWalker.h>

using namespace vpl;

extern UserArguments g_userArgs;

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void ResultsAnalyzer::ReadParamsFromUserArguments()
{
	VisSysComponent::ReadParamsFromUserArguments();

	g_userArgs.ReadBoolArg("ResultsAnalyzer", "saveResults", 
		"Whether to save video recognition results or not", true, &m_params.saveResults);

	g_userArgs.ReadArg("ResultsAnalyzer", "resultsFileName", 
		"Name of the file where the results are saved", 
		std::string("recognition_results.txt"), &m_params.strResultsFilename);

	g_userArgs.ReadArg("ResultsAnalyzer", "cutoffRank", 
		"Maximum K used to compute precision and recall plots", 
		20u, &m_params.cutoffRank);

	g_userArgs.ReadArg("ResultsAnalyzer", "experimentTitle", 
		"A title for the current experiment being performed", 
		std::string("none"), &m_params.strExperimentTitle);
}

void ResultsAnalyzer::Initialize(graph::node v)
{
	VisSysComponent::Initialize(v);

	m_pObjectLearner = FindParentComponent(ObjectLearner);
	m_pObjectRecognizer = FindParentComponent(ObjectRecognizer);

	m_hasOpenedFile = false;
	m_hasOpenedDataBlock = false;
	m_maxExperimentNumber = -1;
}

/*!
	Analyzes the current content of the results file
	in order to determine the new experiment ID.
*/
void ResultsAnalyzer::OpenResultsFile()
{
	ASSERT(!m_hasOpenedFile);

	// The results file is opened as a temporary "ParamFile" object
	ParamFile datafile;

	// Read the results file
	if (datafile.ReadParameters(m_params.strResultsFilename.c_str()))
	{
		int minNum, maxNum;

		if (datafile.GetFieldPrefixMinMax(std::string("Experiment"), 
			&minNum, &maxNum))
		{
			m_maxExperimentNumber = maxNum;
		}
	}
		
	// Open the results file for writing. Note: we want to
	// extend the file AND delete temporary closing brackets, 
	// so we open it with std::ios_base::ate (not std::ios::app)
	m_resultsFile.open(m_params.strResultsFilename.c_str(), 
		std::ios::out | std::ios_base::app);

	//std::ios::out | std::ios_base::ate | ~std::ios_base::trunc | std::ios::app

	if (m_resultsFile.fail())
	{
		ShowCreateFileError(m_params.strResultsFilename);
	}
	else
	{
		m_hasOpenedFile = true;
	}
}

void ResultsAnalyzer::Run()
{
	if (!m_pObjectLearner)
	{
		ShowMissingDependencyError(ObjectLearner);
		return;
	}

	if (!m_pObjectRecognizer)
	{
		ShowMissingDependencyError(ObjectRecognizer);
		return;
	}

	// Analize results only if we are performing the recognition task
	if (TaskName() != "Recognition")
		return;

	ShowStatus("Analyzing results...");

	// See if we have to open the results file
	if (m_params.saveResults && !m_hasOpenedFile)
	{
		// Try to open the file
		OpenResultsFile();

		// Report an error if the operation failed
		if (!m_hasOpenedFile)
		{
			ShowError("Results are not being saved");
			return;
		}
	}

	if (m_hasOpenedFile && !m_hasOpenedDataBlock)
	{
		m_hasOpenedDataBlock = true;
		
		ASSERT(m_maxExperimentNumber >= -1 && m_maxExperimentNumber < 100);

		m_resultsFile << "\nExperiment" << (++m_maxExperimentNumber) << " {";

		// Get the current images from the container graph
		const VSCGraph* pG = static_cast<const VSCGraph*>(ContainerGraph());

		m_resultsFile << "\nexperiment_title = " 
			<< m_params.strExperimentTitle;

		m_resultsFile << "\nvideo_filename = " 
			<< DirWalker::GetName(pG->GetVideoFilename().c_str());

		m_resultsFile << "\nmodel_database = " 
			<< DirWalker::GetName(m_pObjectLearner->ModelDBPath().c_str());

		m_resultsFile << "\nmax_num_parses = " << ShapeParser::MaxNumParses();

		m_resultsFile << "\nshape_descriptor = " 
			<< ShapeParseGraph::GetShapeDescriptorType();

		m_resultsFile << "\nmatching_algorithm = " 
			<< m_pObjectRecognizer->GetMatchingAlgorithm();
	}

	// It might be that we are not required to save the results but
	// just show them in the display. Right now, we don't consider that
	// case

	if (m_hasOpenedFile)
	{
		SaveRecognitionResults();
	}

	m_resultsFile.flush();
}

void ResultsAnalyzer::SaveRecognitionResults()
{
	const std::vector<QueryRanking>& rankings = m_pObjectRecognizer->GetRankings();
	const ModelHierarchy& modelHierarchy = m_pObjectLearner->GetModelHierarchy();

	// Get the current images from the container graph
	const VSCGraph* pG = static_cast<const VSCGraph*>(ContainerGraph());

	const InputImageInfo& iii = pG->GetInputImageInfo();

	m_resultsFile.precision(3);

	// See whether we can compute precion and recall results
	//std::vector<std::pair<double, double>> prVals;
	std::vector<std::vector<unsigned>> prVals(rankings.size());
	TrainingObjectData tod;
	bool bComPR = (m_pObjectLearner && m_pObjectLearner->GetTrainingObjectData(&tod));
	unsigned numMatches, numRelevant;

	// Iterate over all query shapes in the current frame
	for (unsigned q_idx = 0; q_idx < rankings.size(); ++q_idx)
	{
		if (bComPR)
		{
			m_resultsFile << "\nframe" << iii.frameNumber << "query" 
				<< q_idx << "_info = " << tod.ToString();
		}

		m_resultsFile << "\nframe" << iii.frameNumber << "query" 
			<< q_idx << "_matchingTime = " << rankings[q_idx].matchingTime;

		m_resultsFile << "\nframe" << iii.frameNumber << "query" 
			<< q_idx << "_matches = ";

		numMatches = rankings[q_idx].matches.size();
		// Save only top 20 matches
		if (numMatches > 20)
			numMatches = 20;

		if (bComPR) 
		{
			numRelevant = 0;
			prVals[q_idx].assign(m_params.cutoffRank, 0);
		}

		// Iterate over all the sorted model matches for the current query
		for (unsigned m_idx = 0; m_idx < numMatches; ++m_idx)
		{
			const SPGMatch& match = rankings[q_idx].matches[m_idx];
			
			m_resultsFile << "(" << match.queryParseIdx << ","
				<< match.modelViewIdx << "," << match.modelParseIdx << ","
				<< match.value << ") ";

			// Compute precision and recall data. Here we assume that 
			// there is only one query per frame and that we know what
			// object it is via the provided training data 'tod'
			if (bComPR && m_idx < m_params.cutoffRank) 
			{
				const ModelHierarchy::ModelView& mv = 
					modelHierarchy.GetModelView(match.modelViewIdx);

				// See if the known query class name (for the current frame)
				// is equal to the model class retrieved
				if (modelHierarchy.HasAncestor(mv, tod.className))
				{
					numRelevant++;
					m_resultsFile << " and it's a match! ";

					//if (tod.viewId2 == mv.viewId2 && tod.viewId3 == mv.viewId3)
					//	numRelevantView++;
				}
				else
				{
					m_resultsFile << " it's not a match.. ";
				}

				// Store the number of relevant "documents" seen up to
				// the current model index
				prVals[q_idx][m_idx] = numRelevant;
			}
		}

		// prVals[i][j] stores the number of correct (relevant) matches for the i'th
		// query seen among the top j matching candidates retrieved.
		if (bComPR) 
		{
			double numRelevantInDB = modelHierarchy.ModelObjectCount(tod.className);

			// For each query in the current frame
			for (unsigned i = 0; i < prVals.size(); ++i)
			{
				m_resultsFile << "\nframe" << iii.frameNumber << "query" << i 
					<< "_precision_recall = ";

				// For each model in the sorted match list of the i'th query
				for (unsigned j = 0; j < prVals[i].size(); ++j)
				{
					// save as (recall, precision) values
					m_resultsFile << "(" << prVals[i][j] / numRelevantInDB << ","
						<< prVals[i][j] / double(j + 1) << ") ";
				}
			}

			//m_resultsFile << "\n";
		}
	}
}

/*!
	Draws the output of the component. This function is called from
	a "drawing" thread, which is different from the "processing" thread
	that calls Run(). In order to protect the shared resources between the
	threads, a mutex is used.
*/
void ResultsAnalyzer::Draw(const DisplayInfoIn& dii) const
{
	unsigned q_idx = (unsigned)dii.params.at(0);
	unsigned m_idx = (unsigned)dii.params.at(1);
}

/*!	
	Returns the basic information specifying the output of this component.
	It must provide an image, its type, and a text message. All of this parameters 
	are optional. For example, if there is no output image, the image type
	can be set to VOID_IMAGE. 
*/
void ResultsAnalyzer::GetDisplayInfo(const DisplayInfoIn& dii, DisplayInfoOut& dio) const
{
	dio.imageType = VOID_IMAGE;
	dio.syncDisplayViews = true;

	unsigned q_idx = (unsigned)dii.params.at(0);
	unsigned m_idx = (unsigned)dii.params.at(1);
}

