/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#include <RootHeader.h>
#include "BoostMapEmbedder.h"
#include "learning/boost_kdd.h"
#include <Tools/UserArguments.h>
#include <Tools/DirWalker.h>
#include <ShapeMatching/ShapeMatcher.h>
#include <MachineLearning/ObjectLearner.h>
#include <vul/vul_file.h>

using namespace vpl;

extern UserArguments g_userArgs;


BoostMapEmbedder::BoostMapEmbedder() 
{
	m_pBoostMap = NULL;
}

BoostMapEmbedder::~BoostMapEmbedder()
{
	delete m_pBoostMap;
}

void BoostMapEmbedder::Clear()
{
	delete m_pBoostMap;
	m_pBoostMap = NULL;
}

/*!
	Reads the parameters provided by the user. 

	It is called by VisSysComponent::Initialize(), which
	is in turn calledwhen the vis component is created.
	
	It is NOT an static function because the user is allowed
	to changed the parameters of a vision system component
	several times during the execution of a program. 
*/
void BoostMapEmbedder::ReadParamsFromUserArguments()
{
	FeatureEmbedder::ReadParamsFromUserArguments();

	g_userArgs.ReadArg(Name(), "root_data_directory", 
		"Root directory with all the necessary data", 
		DirWalker::CurrentDirectory(), 
		&m_params.root_data_directory);

	g_userArgs.ReadArg(Name(), "complete_dataset", 
		"Name of the dataset", std::string("feature_dataset"), 
		&m_params.complete_dataset);

	g_userArgs.ReadArg(Name(), "training_dataset", 
		"Name of the dataset", std::string("feature_dataset1"), 
		&m_params.training_dataset);

	g_userArgs.ReadArg(Name(), "number_of_triples", 
		"Specifies the number of training triplets", 100000u, 
		&m_params.number_of_triples);

	g_userArgs.ReadBoolArg(Name(), "use_pdistances", 
		"Whether to use class labels or not", false, 
		&m_params.use_pdistances);

	g_userArgs.ReadArg(Name(), "output_filename", 
		"Results are saved to this file", std::string("boostmap_out"), 
		&m_params.output_filename);

	g_userArgs.ReadArg(Name(), "steps", 
		"Number of iterations used by the training algorithm", 100l, 
		&m_params.steps);

	g_userArgs.ReadArg(Name(), "retrieval_dimensions", 
		"Number of dimensions used for retrieval", 50l, 
		&m_params.retrieval_dimensions);

	g_userArgs.ReadArg(Name(), "retrieval_max_k", 
		"maximum number of k nearest neighbors evaluated during retrieval", 10l, 
		&m_params.retrieval_max_k);

	g_userArgs.ReadBoolArg(Name(), "reuse_training_data", 
		"Whether to use the previously saved training date or not", false, 
		&m_params.reuse_training_data);

	g_userArgs.ReadBoolArg(Name(), "randomize_data", 
		"Whether to randomize the order of the data or not", true, 
		&m_params.randomize_data);

	g_userArgs.ReadArg(Name(), "random_data_seed", 
		"Seed used to randomize data (0 == no seed used)", 1u, 
		&m_params.random_data_seed);

	// The function initialize_directories() takes care of all dirs
	// in local_data.h the g_default_code_directory and g_default_data_directory
	// set the default dirs

	// The class_interpreter * base_interpreter; in the class_module
	// sets the directories via initialize_directories()
	// However, we are not using an interpreter, so the directoires are not
	// initialized to the dafault ones
}

void BoostMapEmbedder::Run()
{
	if (!m_pObjectLearner)
	{
		ShowMissingDependencyError(ObjectLearner);
		return;
	}

	if (!m_pShapeMatcher)
	{
		ShowMissingDependencyError(ShapeMatcher);
		return;
	}

	if (TaskName() == "Offline_learning")
	{
		if (m_params.reuse_training_data || PrepareTrainingData())
			LearnEmbedding();		
	}
	else if (TaskName() == "Offline_testing")
	{
		TestRetrievalAccuracy();
	}
	else
	{
		ShowStatus("Nothing to do: only work in offline learning mode");
		return;
	}

}

/*!
	Write the header and data to a file as required by the BoostMap format.
*/
bool BoostMapEmbedder::SaveMatrix(const Matrix& m, const std::string& rootDir,
	std::string filename) const
{
	// Complete the file name
	filename = DirWalker::ConcatPathElements(rootDir, filename);

	fstream fs(filename, std::ios::trunc | std::ios::out | std::ios::binary);

	if (!fs)
	{
		ShowOpenFileError(filename);
		return false;
	}

	// Write the header as required by the BoostMap format
	vint4 a(4), b(m.rows()), c(m.cols()), d(1);

	fs.write((char*)&a, sizeof(a));
	fs.write((char*)&b, sizeof(b));
	fs.write((char*)&c, sizeof(c));
	fs.write((char*)&d, sizeof(d));

	float val;

	// Write the data as required by the BoostMap format
	for (unsigned i = 0; i < m.rows(); i++)
	{
		for (unsigned j = 0; j < m.cols(); j++)
		{
			val = float(m(i, j));
			fs.write((char*)&val, sizeof(val));
		}
	}

	return true;
}

/*!
	Prepare the trainign data to learn the embedding.

	It sets the index map m_indexMap so that we can map
	matrix indices to database IDs.
*/
bool BoostMapEmbedder::PrepareTrainingData()
{
	ShowStatus("Preparing the training data...");

	Matrix d;

	// Compute the database distance matrix
	m_indexMap = m_pShapeMatcher->ComputeDatabaseDistances(d, 
		m_params.randomize_data, m_params.random_data_seed);

	// Make the path to the place where we must save the matrix
	std::string rootDir = BoostMap_data::make_data_path(
		m_params.root_data_directory, m_params.complete_dataset);

	// Make sure that the directory exists
	if (!vul_file::make_directory_path(rootDir))
		return false;

	// Save the matrix
	if (!SaveMatrix(d, rootDir, "traintrain_distances.bin"))
		return false;

	// Store the number of database objects that we have
	const unsigned N = d.rows();

	// Save a matrix of class labels
	if (!SaveMatrix(Matrix(1, N, 0.0), rootDir, "training_labels.bin"))
		return false;

	vint8 portion = (vint8) std::floor(N / 3.0);
	//DBG_PRINT1(portion)

	vint8 candidate_start = 0;
	vint8 candidate_end = portion;
	vint8 training_start = candidate_end + 1;
	vint8 training_end = training_start + portion;
	vint8 validation_start = training_end + 1;
	vint8 validation_end = N - 1;

	//DBG_PRINT6(candidate_start, candidate_end, training_start, training_end,
	//	validation_start, validation_end)

	// Save the validation data as the test data
	Matrix t = d.Sub((unsigned) validation_start, 
		             (unsigned) validation_end, 
		             0, N);

	if (!SaveMatrix(t, rootDir, "testtrain_distances.bin"))
		return false;

	if (!SaveMatrix(Matrix(1, t.rows(), 0.0), rootDir, "test_labels.bin"))
		return false;

	// Let the boostmap code prepare the data the way it likes it
	ShowStatus("Creating BoostMap data files...");

	BoostMap_data bm_data(m_params.root_data_directory.c_str(), 
		m_params.complete_dataset.c_str(), m_params.training_dataset.c_str(),
		candidate_start, candidate_end, training_start, training_end,                       
		validation_start, validation_end);

	ShowStatus("\nThe training data is ready!!!\n");

	return true;
}

void BoostMapEmbedder::LearnEmbedding()
{
	ShowStatus("Learning feature embedding");

	// make sure that there is no boost map
	delete m_pBoostMap;

	m_pBoostMap = new class_BoostMap(
		m_params.root_data_directory.c_str(), 
		m_params.training_dataset.c_str(), 
		m_params.number_of_triples, 
		m_params.use_pdistances);
		
	if (m_pBoostMap->valid() > 0)
	{
		float min_validation_error = 100000;
		float training_margin, validation_error;

		for (long i = 0; i < m_params.steps; i++)
		{
			training_margin = m_pBoostMap->fast_next_step();

			m_pBoostMap->save_classifier(m_params.output_filename.c_str());

			m_pBoostMap->PrintSummary();

			validation_error = m_pBoostMap->ValidationError();

			if (validation_error < min_validation_error)
				min_validation_error = validation_error;
		}
	}
	else
	{
		ShowError("Cannot create boost map");
	}

	ShowStatus("\nThe embedding has been learnt!!!\n");
}

void BoostMapEmbedder::TestRetrievalAccuracy()
{
	const char* root_dir = string_copy(m_params.root_data_directory);
	BoostMap_data::set_global_data_directory(root_dir);

	std::string embedding = m_params.output_filename + "d";

	vint8_matrix result = BoostMap_data::retrieval_results_test(
		m_params.complete_dataset.c_str(), embedding.c_str(), 
		m_params.retrieval_dimensions, m_params.retrieval_max_k);

	class_BoostMap::print_retrieval_results(result);

	BoostMap_data::set_global_data_directory(NULL);
	delete[] root_dir;
}

void BoostMapEmbedder::TestClassificationAccuracy()
{
}
