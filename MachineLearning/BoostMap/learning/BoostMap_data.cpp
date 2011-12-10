#include "BoostMap_data.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"

#include "basics/definitions.h"

#include "boost_kdd_utils.h"
#include "boost_kdd.h"

#include <Tools/DirWalker.h>

vint8 BoostMap_data::zero()
{
	original_dataset_name = 0;
	training_name = 0;
	candidate_number = 0;
	training_number = 0;
	validation_number = 0;
	is_valid = 0;

	return 1;
}


vint8 BoostMap_data::initialize()
{
	zero();

	return 1;
}


vint8 BoostMap_data::clean_up()
{
	vdelete2(training_name);
	delete_pointer(original_dataset_name);
	delete_pointer(root_data_dir);

	return 1;
}

/*!
	This constructor initializes a BoostMap_data object when
	the named dataset has already been created.
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_training_name)
{
	initialize();
	training_name = vCopyString(in_training_name);
	root_data_dir = vCopyString(root_dir);

	// the dataset is not valid if the header cannot be loaded.
	is_valid = LoadHeader();
}

/*!
	This constructor initializes a BoostMap_data object and
	creates the named dataset on disk.
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_training_name, 
	vMatrix<vint8> in_candidates,
	vMatrix<vint8> in_training, 
	vMatrix<vint8> in_validation)
{
	initialize();
	training_name = vCopyString(in_training_name);
	root_data_dir = vCopyString(root_dir);
	SetIds(in_candidates, in_training, in_validation);
	is_valid = SaveHeader();
}

/*!
	This constructor initializes a BoostMap_data object and
	creates the named dataset on disk. Here we create 
	the matrices for candidates, training and validation,
	by choosing randomly mutually disjoint subsets
	of the set {0, 1, ..., number-1}
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_training_name, 
	const char * in_original_name,
	vint8 number, vint8 in_candidate_number, 
	vint8 in_training_number, vint8 in_validation_number)
{
	initialize();
	training_name = function_copy_string(in_training_name);
	root_data_dir = vCopyString(root_dir);
	original_dataset_name = function_copy_string(in_original_name);
	vint8 start, end;

	// An easy way to create mutually disjoint subjects is to 
	// first randomly permute the entire set, and then to choose
	// different ranges (disjoint), from the permuted set.
	vint8_matrix samples2 = vPermutation(0, number - 1);
	vMatrix<vint8> samples(&samples2);
	start = 0;
	end = start + in_candidate_number - 1;
	vMatrix<vint8> candidatesm(1, in_candidate_number);
	samples.Copy(&candidatesm, 0, 0, start, end, 0, 0);

	start = end+1;
	end = start + in_training_number - 1;
	vMatrix<vint8> trainingm(1, in_training_number);
	samples.Copy(&trainingm, 0, 0, start, end, 0, 0);

	start = end+1;
	end = start + in_validation_number - 1;
	vMatrix<vint8> validationm(1, in_validation_number);
	samples.Copy(&validationm, 0, 0, start, end, 0, 0);

	SetIds(candidatesm, trainingm, validationm);
	is_valid = SaveHeader();
}

/*!
	This constructor initializes a BoostMap_data object,
	creates the named dataset on disk, including all distances
	and pdistances.
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_original_name, 
	const char * in_training_name, 
	vint8 in_candidate_number, 
	vint8 in_training_number, vint8 in_validation_number)
{
	initialize();
	training_name = function_copy_string(in_training_name);
	root_data_dir = vCopyString(root_dir);
	original_dataset_name = function_copy_string(in_original_name);
	vint8 start, end;

	vint8 number = TrainingNumber(original_dataset_name);
	if (number <= 0)
	{
		return;
	}

	// An easy way to create mutually disjoint subjects is to 
	// first randomly permute the entire set, and then to choose
	// different ranges (disjoint), from the permuted set.
	vint8_matrix samples2 = vPermutation(0, number - 1);
	vMatrix<vint8> samples(&samples2);
	start = 0;
	end = start + in_candidate_number - 1;
	vMatrix<vint8> candidatesm(1, in_candidate_number);
	samples.Copy(&candidatesm, 0, 0, start, end, 0, 0);

	start = end+1;
	end = start + in_training_number - 1;
	vMatrix<vint8> trainingm(1, in_training_number);
	samples.Copy(&trainingm, 0, 0, start, end, 0, 0);

	start = end+1;
	end = start + in_validation_number - 1;
	vMatrix<vint8> validationm(1, in_validation_number);
	samples.Copy(&validationm, 0, 0, start, end, 0, 0);

	SetIds(candidatesm, trainingm, validationm);
	is_valid = SaveAll(original_dataset_name);
}

/*!
	This constructor reads distances from the directory where
	an original dataset is stored, and based on those distances
	it creates all the files that are needed for boostmap
	to be run.
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_original_name, 
	const char * in_name, 
	vint8 candidate_start, vint8 candidate_end, 
	vint8 training_start, vint8 training_end,
	vint8 validation_start, vint8 validation_end)
{
	initialize();
	training_name = function_copy_string(in_name);
	root_data_dir = vCopyString(root_dir);
	original_dataset_name = function_copy_string(in_original_name);

	candidate_number = candidate_end - candidate_start + 1;
	training_number = training_end - training_start + 1;
	validation_number = validation_end - validation_start + 1;
	is_valid = 0;

	candidate_ids = function_range_matrix(candidate_start, candidate_end);
	training_ids = function_range_matrix(training_start, training_end);
	validation_ids = function_range_matrix(validation_start, validation_end);

	vint8 error_flag = 0;
	vint8 success = SaveHeader();
	if (success <= 0)
	{
		error_flag = 1;
	}
	success = MakeAllDistances(original_dataset_name);
	if (success <= 0)
	{
		error_flag = 1;
	}

	success = MakeAllPdistances(original_dataset_name);
	if (success <= 0)
	{
		error_flag = 1;
	}

	if (error_flag == 0)
	{
		is_valid = 1;
	}
}

/*!
	This constructor was created explicitly for the protein set,
	where half the training (and validation) objects come from
	the query set, and half come from the database.
*/
BoostMap_data::BoostMap_data(const char * root_dir, const char * in_original_name, const char * in_name, 
	vint8 candidate_start, vint8 candidate_end, 
	vint8 training_start1, vint8 training_end1, vint8 training_start2,
	vint8 validation_start1, vint8 validation_end1, vint8 validation_start2)
{
	initialize();
	training_name = function_copy_string(in_name);
	root_data_dir = vCopyString(root_dir);
	original_dataset_name = function_copy_string(in_original_name);

	candidate_number = candidate_end - candidate_start + 1;
	vint8 training_number2 = training_end1 - training_start1 + 1;
	vint8 validation_number2 = validation_end1 - validation_start1 + 1;
	training_number = training_number2 * 2;
	validation_number = validation_number2 * 2;
	vint8 training_end2 = training_start2 + training_number2 - 1;
	vint8 validation_end2 = validation_start2 + validation_number2 - 1;
	is_valid = 0;
	vint8 i;

	candidate_ids = function_range_matrix(candidate_start, candidate_end);

	vMatrix<vint8> training_ids1 = function_range_matrix(training_start1, training_end1);
	vMatrix<vint8> training_ids2 = function_range_matrix(training_start2, training_end2);
	training_ids = vMatrix<vint8>(1, training_number);
	for (i = 0; i < training_number2; i++)
	{
		training_ids(i) = training_ids1(i);
		training_ids(i+training_number2) = training_ids2(i);
	}

	vMatrix<vint8> validation_ids1 = function_range_matrix(validation_start1, validation_end1);
	vMatrix<vint8> validation_ids2 = function_range_matrix(validation_start2, validation_end2);
	validation_ids = vMatrix<vint8>(1, validation_number);
	for (i = 0; i < validation_number2; i++)
	{
		validation_ids(i) = validation_ids1(i);
		validation_ids(i+validation_number2) = validation_ids2(i);
	}

	vint8 error_flag = 0;
	vint8 success = SaveHeader();
	if (success <= 0)
	{
		error_flag = 1;
	}
	success = MakeAllDistances(original_dataset_name, 0);
	if (success <= 0)
	{
		error_flag = 1;
	}

	success = MakeAllPdistances(original_dataset_name);
	if (success <= 0)
	{
		error_flag = 1;
	}

	if (error_flag == 0)
	{
		is_valid = 1;
	}
}


BoostMap_data::~BoostMap_data()
{
	clean_up();
}

vint8 BoostMap_data::SetCandidates(vMatrix<vint8> in_matrix)
{
	candidate_ids = in_matrix;
	candidate_number = in_matrix.Size();
	return 1;
}


vint8 BoostMap_data::SetTraining(vMatrix<vint8> in_matrix)
{
	training_ids = in_matrix;
	training_number = in_matrix.Size();
	return 1;
}


vint8 BoostMap_data::SetValidation(vMatrix<vint8> in_matrix)
{
	validation_ids = in_matrix;
	validation_number = in_matrix.Size();
	return 1;
}


// Set all of candidate_ids, training_ids, validation_ids.
vint8 BoostMap_data::SetIds(vMatrix<vint8> in_candidates, 
	vMatrix<vint8> in_training,
	vMatrix<vint8> in_validation)
{
	SetCandidates(in_candidates);
	SetTraining(in_training);
	SetValidation(in_validation);
	return 1;
}


// compute all the matrices holding distances that we need to
// train and evaluate BoostMap, and save those matrices to file.
vint8 BoostMap_data::MakeAllDistances(vMatrix<float> distances)
{
	MakeCandCandDistances(distances);
	MakeCandTrainDistances(distances);
	MakeCandValDistances(distances);
	MakeTrainingDistances(distances);
	MakeValidationDistances(distances);
	return 1;
}


// here the original distances of the entire training set to 
// itself are read from a file, as opposed
// to being passed in as an argument.
vint8 BoostMap_data::MakeAllDistances(const char * original_dataset_name, vint8 strict)
{
	MakeCandCandDistances();
	MakeCandTrainDistances();
	MakeCandValDistances();
	MakeTrainingDistances(strict);
	MakeValidationDistances(strict);
	return 1;
}


// in general, in this code, pdistances are distances in parameter
// space (aka state space). MakeAllPdistances computes and saves
// all necessary matrices storing parameter distancse.
vint8 BoostMap_data::MakeAllPdistances(vMatrix<float> pdistances)
{
	MakeTrainingPdistances(pdistances);
	MakeValidationPdistances(pdistances);
	return 1;
}


// here the original distances of the entire training set to 
// itself are read from a file, as opposed
// to being passed in as an argument.
vint8 BoostMap_data::MakeAllPdistances(const char * original_dataset_name)
{
	MakeTrainingPdistances();
	MakeValidationPdistances();
	return 1;
}


vMatrix<float> BoostMap_data::MakeCandCandDistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, candidate_ids, candidate_ids);
	SaveCandCandDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeCandTrainDistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, candidate_ids, training_ids);
	SaveCandTrainDistances(result);
	return result;
}



vMatrix<float> BoostMap_data::MakeCandValDistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, candidate_ids, validation_ids);
	SaveCandValDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeTrainingDistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, training_ids, training_ids);
	SaveTrainingDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeTrainingPdistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, training_ids, training_ids);
	SaveTrainingPdistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeValidationDistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, validation_ids, validation_ids);
	SaveValidationDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeValidationPdistances(vMatrix<float> distances)
{
	vMatrix<float> result = MakeDistances(distances, validation_ids, validation_ids);
	SaveValidationPdistances(result);
	return result;
}


// MakeDistances does a lot of the work for the preceding functions.
// It takes in a big distance matrix (distances), and two
// sets of indices. It computes a matrix storing distances from
// all objects described in set1 to all objects described in 
// set2. Essentially, each pair of objects in set1 x set2 defines
// a reference to an entry in distances, and we copy that entry
// into the result matrix.
vMatrix<float> BoostMap_data::MakeDistances(vMatrix<float> distances, 
	vMatrix<vint8> set1, vMatrix<vint8> set2)
{
	vint8 rows = set1.Size();
	vint8 cols = set2.Size();
	vMatrix<float> result(rows, cols);

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vint8 index1 = set1(row);
		for (col = 0; col < cols; col++)
		{
			vint8 index2 = set2(col);
			float distance = distances(index1, index2);
			result(row, col) = distance;
		}
	}

	return result;
}

////////////////////

vMatrix<float> BoostMap_data::MakeCandCandDistances()
{
	vMatrix<float> result = MakeDistances(candidate_ids, candidate_ids);
	SaveCandCandDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeCandTrainDistances()
{
	vMatrix<float> result = MakeDistances(candidate_ids, training_ids);
	SaveCandTrainDistances(result);
	return result;
}



vMatrix<float> BoostMap_data::MakeCandValDistances()
{
	vMatrix<float> result = MakeDistances(candidate_ids, validation_ids);
	SaveCandValDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeTrainingDistances(vint8 strict)
{
	vMatrix<float> result = MakeDistances(training_ids, training_ids, strict);
	SaveTrainingDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeTrainingPdistances()
{
	vMatrix<float> result = MakePdistances(training_ids, training_ids);
	SaveTrainingPdistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeValidationDistances(vint8 strict)
{
	vMatrix<float> result = MakeDistances(validation_ids, validation_ids, strict);
	SaveValidationDistances(result);
	return result;
}


vMatrix<float> BoostMap_data::MakeValidationPdistances()
{
	vMatrix<float> result = MakePdistances(validation_ids, validation_ids);
	SaveValidationPdistances(result);
	return result;
}


// MakeDistances does a lot of the work for the preceding functions.
// It takes in a big distance matrix (distances), and two
// sets of indices. It computes a matrix storing distances from
// all objects described in set1 to all objects described in 
// set2. Essentially, each pair of objects in set1 x set2 defines
// a reference to an entry in distances, and we copy that entry
// into the result matrix. 
// strict determines if we tolerate errors (like we should for
// the protein dataset, where we don't have the full distance matrix,
// and it's OK) or not (if set to 1, we don't tolerate errors).
vMatrix<float> BoostMap_data::MakeDistances(vMatrix<vint8> set1, 
	vMatrix<vint8> set2, vint8 strict)
{
	char * filename = TrainTrainDistancesPath();

	vint8 rows = set1.Size();
	vint8 cols = set2.Size();
	vMatrix<float> result(rows, cols);

	vint8 row, col;
	vPrint("\n");

	vint8 error_flag = 0;

	for (row = 0; row < rows; row++)
	{
		vPrint("processing object %li\r", (long) row);
		vint8 index1 = set1(row);
		vMatrix<float> distances = ObjectDistances(filename, index1);
		if (distances.valid() <= 0)
		{
			if (strict == 0)
			{
				if (error_flag == 0)
				{
					error_flag = 1;
					vPrint("\nfailed to load distances for object %li\n", (long) index1);
				}
				for (col = 0; col < cols; col++)
				{
					result(row, col) = -10;
				}
			}
			else
			{
				vPrint("\nfailed to load distances for object %li\n", (long) index1);
				vdelete2(filename);
				return vMatrix<float>();
			}
		}
		else
		{
			for (col = 0; col < cols; col++)
			{
				vint8 index2 = set2(col);
				float distance = distances(index2);
				result(row, col) = distance;
			}
		}
	}

	vPrint("\n");
	vdelete2(filename);
	return result;
}

vMatrix<float> BoostMap_data::MakePdistances(vMatrix<vint8> set1, vMatrix<vint8> set2)
{
	vMatrix<float> labelsm = LoadTrainingLabels();

	vint8 rows = set1.Size();
	vint8 cols = set2.Size();
	vMatrix<float> result(rows, cols);

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vint8 index1 = set1(row);
		for (col = 0; col < cols; col++)
		{
			vint8 index2 = set2(col);
			float distance;
			if (labelsm(index1) == labelsm(index2))
			{
				distance = 0;
			}
			else
			{
				distance = 1;
			}
			result(row, col) = distance;
		}
	}

	return result;
}



// returns a concatenation of training ids and validation ids.
vMatrix<vint8> BoostMap_data::TrainingValidationIds()
{
	if (is_valid == 0)
	{
		return vMatrix<vint8>();
	}

	vMatrix<vint8> result(1, training_number + validation_number);
	training_ids.Copy(&result, 0, 0, 0, training_number - 1, 0, 0);
	validation_ids.Copy(&result, 0, 0, 0, validation_number - 1, 
		0, training_number);
	return result;
}


// CheckValidity checks for negative distances, distances in 
// off-diagonal elements that are equal to zero, and distances 
// in diagonal elements that are not zero.
vint8 BoostMap_data::CheckValidity()
{
	vint8 negative = 0, zeros = 0, nonzeros = 0;
	vint8 rows, cols, row, col;
	vMatrix<float> distancesm;
	vArray2(float) distances;
	vint8 error_flag = 0;

	vPrint("\nchecking cand-cand distances:\n");
	distancesm = LoadCandCandDistances();
	rows = distancesm.Rows();
	cols = distancesm.Cols();
	distances = distancesm.Matrix2();
	negative = 0, zeros = 0, nonzeros = 0;

	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			if (distances[row][col] < 0)
			{
				negative++;
				error_flag = 1;
				continue;
			}
			if ((row != col) && (distances[row][col] == 0))
			{
				zeros++;
				error_flag = 1;
				continue;
			}
			if ((row == col) && (distances[row][col] != 0))
			{
				error_flag = 1;
				nonzeros++;
			}
		}
	}
	vPrint("negative = %li, zeros = %li, non-zero = %li\n",
		(long) negative, (long) zeros, (long) nonzeros);

	vPrint("\nchecking train-train distances:\n");
	distancesm = LoadTrainingDistances();
	rows = distancesm.Rows();
	cols = distancesm.Cols();
	distances = distancesm.Matrix2();
	negative = 0, zeros = 0, nonzeros = 0;

	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			if (distances[row][col] < 0)
			{
				negative++;
				error_flag = 1;
				continue;
			}
			if ((row != col) && (distances[row][col] == 0))
			{
				zeros++;
				error_flag = 1;
				continue;
			}
			if ((row == col) && (distances[row][col] != 0))
			{
				error_flag = 1;
				nonzeros++;
			}
		}
	}

	vPrint("negative = %li, zeros = %li, non-zero = %li\n",
		(long) negative, (long) zeros, (long) nonzeros);

	vPrint("\nchecking validation-validation distances:\n");
	distancesm = LoadValidationDistances();
	rows = distancesm.Rows();
	cols = distancesm.Cols();
	distances = distancesm.Matrix2();
	negative = 0, zeros = 0, nonzeros = 0;

	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			if (distances[row][col] < 0)
			{
				negative++;
				error_flag = 1;
				continue;
			}
			if ((row != col) && (distances[row][col] == 0))
			{
				zeros++;
				error_flag = 1;
				continue;
			}
			if ((row == col) && (distances[row][col] != 0))
			{
				error_flag = 1;
				nonzeros++;
			}
		}
	}

	vPrint("negative = %li, zeros = %li, non-zero = %li\n",
		(long) negative, (long) zeros, (long) nonzeros);

	vPrint("\nchecking candidate-training distances:\n");
	distancesm = LoadCandTrainDistances();
	rows = distancesm.Rows();
	cols = distancesm.Cols();
	distances = distancesm.Matrix2();
	negative = 0, zeros = 0, nonzeros = 0;

	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			if (distances[row][col] < 0)
			{
				negative++;
				error_flag = 1;
				continue;
			}
			if ((row != col) && (distances[row][col] == 0))
			{
				zeros++;
				error_flag = 1;
				continue;
			}
		}
	}

	vPrint("negative = (long) %li, zeros = (long) %li\n", negative, zeros);

	vPrint("\nchecking candidate-validation distances:\n");
	distancesm = LoadCandValDistances();
	rows = distancesm.Rows();
	cols = distancesm.Cols();
	distances = distancesm.Matrix2();
	negative = 0, zeros = 0, nonzeros = 0;

	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			if (distances[row][col] < 0)
			{
				negative++;
				error_flag = 1;
				continue;
			}
			if ((row != col) && (distances[row][col] == 0))
			{
				zeros++;
				error_flag = 1;
				continue;
			}
		}
	}

	vPrint("negative = %li, zeros = %li\n", (long) negative, (long) zeros);
	return 1;
}

//! [static] Sets the global data directory
void BoostMap_data::set_global_data_directory(const char* root_dir)
{
	g_data_directory = root_dir;
}

// returns a complete path name for the directory where
// information about the original training dataset is stored.
char * BoostMap_data::original_directory()
{
	return original_directory(root_data_dir, original_dataset_name);
}

//! [static] Makes a complete path name for the directory where
//! information about the original training dataset is stored.
std::string BoostMap_data::make_data_path(const std::string& rootDir, 
	const std::string& datasetName)
{
	return vpl::DirWalker::ConcatPathElements(
		vpl::DirWalker::ConcatPathElements(rootDir, "bm_datasets"),
		datasetName);
}

// a static version of the previous function
char * BoostMap_data::original_directory(const char* rootDir, 
	const char * original_dataset_name)
{
	char * result = vJoinPaths3(rootDir, "bm_datasets", original_dataset_name);
	if (vDirectoryExists(result) <= 0)
	{
		vint8 success = function_make_directory(result);
		if (success <= 0)
		{
			exit_error("Failed to create directory %s\n", result);
			vdelete2(result);
			return 0;
		}
	}
	return result;
}


// converts a simple filename into a pathname, by adding
// the result of original_directory in front of it.
char * BoostMap_data::original_pathname(const char * simple_name)
{
	char * directory = original_directory();
	char * result = vJoinPaths(directory, simple_name);
	vdelete2(directory);
	return result;
}


// returns a complete path name for the directory where
// information about the original training dataset is stored.
char * BoostMap_data::training_directory()
{
	return training_directory(root_data_dir, training_name);
}

// a static version of the previous function
// default shoudl be root_data_dir = g_data_directory
char * BoostMap_data::training_directory(const char * root_data_dir, 
	const char * training_name)
{
	char * result = vJoinPaths4(root_data_dir, "experiments", 
		"bm_training", training_name);

	if (vDirectoryExists(result) <= 0)
	{
		vint8 success = function_make_directory(result);
		if (success <= 0)
		{
			exit_error("Failed to create directory %s\n", result);
			vdelete2(result);
			return 0;
		}
	}
	return result;
}


// converts a simple filename into a pathname, by adding
// the result of training_directory() in front of it.
char * BoostMap_data::training_pathname(const char * simple_name)
{
	char * directory = training_directory();
	char * result = vJoinPaths(directory, simple_name);
	vdelete2(directory);
	return result;
}


// returns a complete path name for the directory where
// we store actual embeddings of database objects and test objects
char * BoostMap_data::embedding_directory(const char * dataset_name)
{
	return training_directory(root_data_dir, dataset_name);
}

// returns a complete path name for the directory where
// we store actual embeddings of database objects and test objects
char * BoostMap_data::embedding_directory()
{
	return training_directory();
}


// a static version of Directory()
char * BoostMap_data::embedding_directory(const char * root_data_dir,
	const char * training_name)
{
	char * result = vJoinPaths5(root_data_dir, "experiments", "embeddings", 
		"BoostMap", training_name);
	if (vDirectoryExists(result) <= 0)
	{
		vint8 success = function_make_directory(result);
		if (success <= 0)
		{
			exit_error("Failed to create directory %s\n", result);
			vdelete2(result);
			return 0;
		}
	}
	return result;
}


// converts a simple filename into a pathname, by adding
// the result of embedding_directory() in front of it.
char * BoostMap_data::embedding_pathname(const char * simple_name)
{
	char * directory = embedding_directory();
	char * result = vJoinPaths(directory, simple_name);
	vdelete2(directory);
	return result;
}


// returns a complete path name for the directory where
// we store actual cascades of database objects and test objects
char * BoostMap_data::cascade_directory()
{
	return training_directory();
}


// a static version of Directory()
char * BoostMap_data::cascade_directory(const char * root_data_dir, 
	const char * training_name)
{
	char * result = vJoinPaths5(root_data_dir, "experiments", "cascades", 
		"BoostMap", training_name);
	if (vDirectoryExists(result) <= 0)
	{
		vint8 success = function_make_directory(result);
		if (success <= 0)
		{
			exit_error("Failed to create directory %s\n", result);
			vdelete2(result);
			return 0;
		}
	}
	return result;
}


// converts a simple filename into a pathname, by adding
// the result of cascade_directory() in front of it.
char * BoostMap_data::cascade_pathname(const char * simple_name)
{
	char * directory = cascade_directory();
	char * result = vJoinPaths(directory, simple_name);
	vdelete2(directory);
	return result;
}


vint8 BoostMap_data::SaveAll(const char * original_dataset_name)
{
	vint8 error_flag = 0;
	vint8 success = SaveHeader();
	if (success <= 0)
	{
		error_flag = 1;
	}
	success = MakeAllDistances(original_dataset_name);
	if (success <= 0)
	{
		error_flag = 1;
	}

	success = MakeAllPdistances(original_dataset_name);
	if (success <= 0)
	{
		error_flag = 1;
	}

	if (error_flag == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// header refers to the member variables (excluding name).
vint8 BoostMap_data::SaveHeader()
{
	// open the filename
	char * filename = HeaderPath();
	FILE * fp = fopen(filename, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", filename);
		vdelete2(filename);
		return 0;
	}

	vint8 success = 1;
	vint8 temp_success;
	// write all the ids.
	temp_success = integer_matrix(candidate_ids).Write(fp);
	if (temp_success == 0)
	{
		success = 0;
	}

	temp_success = integer_matrix(training_ids).Write(fp);
	if (temp_success == 0)
	{
		success = 0;
	}

	temp_success = integer_matrix(validation_ids).Write(fp);
	if (temp_success == 0)
	{
		success = 0;
	}

	temp_success = save_string_text(fp, original_dataset_name);
	if (temp_success == 0)
	{
		success = 0;
	}

	if (success == 0)
	{
		vPrint("failed to save data to %s\n", filename);
		vdelete2(filename);
		return 0;
	}



	fclose(fp);
	vdelete2(filename);
	return success;
}


vint8 BoostMap_data::LoadHeader()
{
	// open header file
	char * filename = HeaderPath();
	FILE * fp = fopen(filename, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", filename);
		vdelete2(filename);
		return 0;
	}

	// read all the ids.
	vint8 success = 1;
	candidate_ids = vint8_matrix(vMatrix<integer>::read(fp));
	if (candidate_ids.valid() <= 0)
	{
		vPrint("failed to load candidate ids\n");
		success = 0;
	}

	training_ids = vint8_matrix(vMatrix<integer>::read(fp));
	if (training_ids.valid() <= 0)
	{
		vPrint("failed to load training ids\n");
		success = 0;
	}

	validation_ids = vint8_matrix(vMatrix<integer>::read(fp));
	if (validation_ids.valid() <= 0)
	{
		vPrint("failed to load validation ids\n");
		success = 0;
	}

	delete_pointer(original_dataset_name);
	original_dataset_name = load_string_text(fp);
	if (original_dataset_name == 0)
	{
		success = 0;
	}

	if (success == 0)
	{
		vPrint("failed to load data from %s\n", filename);
		vdelete2(filename);
		return 0;
	}

	fclose(fp);
	vdelete2(filename);
	candidate_number = candidate_ids.Size();
	training_number = training_ids.Size();
	validation_number = validation_ids.Size();
	return success;
}


// filename where header is saved.
char * BoostMap_data::HeaderName()
{
	char * simple_name = "header.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::HeaderPath()
{
	char * simple_name = HeaderName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}


// CandCandDistances are distances from i-th candidate to j-th
// candidate (useful for line projections).
vint8 BoostMap_data::SaveCandCandDistances(vMatrix<float> distances)
{
	if ((distances.Rows() != candidate_number) ||
		(distances.Cols() != candidate_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveCandCandDistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = CandCandDistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save cand-cand distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved cand-cand distances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadCandCandDistances()
{
	char * pathname = CandCandDistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load cand-cand distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != candidate_number) ||
		(result.Cols() != candidate_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadCandCandDistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}

	else
	{
		vPrint("Loaded cand-cand distances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::CandCandDistancesName()
{
	char * simple_name = "candcand_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::CandCandDistancesPath()
{
	char * simple_name = CandCandDistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}



// CandTrainDistances are distances from i-th candidate to j-th
// training object.
vint8 BoostMap_data::SaveCandTrainDistances(vMatrix<float> distances)
{
	if ((distances.Rows() != candidate_number) ||
		(distances.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveCandTrainDistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = CandTrainDistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save cand-train distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved cand-train distances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadCandTrainDistances()
{
	char * pathname = CandTrainDistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load cand-train distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != candidate_number) ||
		(result.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadCandTrainDistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}

	else
	{
		vPrint("Loaded cand-train distances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::CandTrainDistancesName()
{
	char * simple_name = "candtrain_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::CandTrainDistancesPath()
{
	char * simple_name = CandTrainDistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}



// CandValDistances are distances from i-th candidate to j-th
// validation object.
vint8 BoostMap_data::SaveCandValDistances(vMatrix<float> distances)
{
	if ((distances.Rows() != candidate_number) ||
		(distances.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveCandValDistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = CandValDistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save cand-val distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved cand-val distances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadCandValDistances()
{
	char * pathname = CandValDistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load cand-val distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != candidate_number) ||
		(result.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadCandValDistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}

	else
	{
		vPrint("Loaded cand-val distances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::CandValDistancesName()
{
	char * simple_name = "candval_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::CandValDistancesPath()
{
	char * simple_name = CandValDistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}








// Number of training objects in the original dataset.
vint8 BoostMap_data::TrainingNumber(const char * original_dataset_name)
{
	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, original_dataset_name);
	if (labels.valid() <= 0)
	{
		return -1;
	}

	vint8 result = labels.Size();
	return result;
}


// Number of test objects in the original dataset.
vint8 BoostMap_data::TestNumber(const char * original_dataset_name)
{
	vMatrix<float> labels = LoadTestLabels(g_data_directory, original_dataset_name);
	if (labels.valid() <= 0)
	{
		return -1;
	}

	vint8 result = labels.Size();
	return result;
}


// training distances refer to distances from training to training.
vint8 BoostMap_data::SaveTrainingDistances(vMatrix<float> distances)
{
	if ((distances.Rows() != training_number) ||
		(distances.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveTrainingDistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = TrainingDistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved training distances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadTrainingDistances()
{
	char * pathname = TrainingDistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != training_number) ||
		(result.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadTrainingDistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}
	else
	{
		vPrint("Loaded training distances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::TrainingDistancesName()
{
	char * simple_name = "training_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::TrainingDistancesPath()
{
	char * simple_name = TrainingDistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}


// validation distances refer to distances from validation to validation
vint8 BoostMap_data::SaveValidationDistances(vMatrix<float> distances)
{
	if ((distances.Rows() != validation_number) ||
		(distances.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveValidationDistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = ValidationDistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved validation distances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadValidationDistances()
{
	char * pathname = ValidationDistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != validation_number) ||
		(result.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadValidationDistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}
	else
	{
		vPrint("Loaded validation distances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::ValidationDistancesName()
{
	char * simple_name = "validation_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::ValidationDistancesPath()
{
	char * simple_name = ValidationDistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}


// Pdistances refer to distances in parameter space.
vint8 BoostMap_data::SaveTrainingPdistances(vMatrix<float> distances)
{
	if ((distances.Rows() != training_number) ||
		(distances.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveTrainingPdistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = TrainingPdistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved training pdistances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadTrainingPdistances()
{
	char * pathname = TrainingPdistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != training_number) ||
		(result.Cols() != training_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadTrainingPdistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}
	else
	{
		vPrint("Loaded training pdistances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::TrainingPdistancesName()
{
	char * simple_name = "training_pdistances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::TrainingPdistancesPath()
{
	char * simple_name = TrainingPdistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}


vint8 BoostMap_data::SaveValidationPdistances(vMatrix<float> distances)
{
	if ((distances.Rows() != validation_number) ||
		(distances.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in SaveValidationPdistances\n",
			(long) distances.Rows(), (long) distances.Cols());
		return 0;
	}

	char * pathname = ValidationPdistancesPath();
	vint8 success = distances.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save distances to %s\n", pathname);
	}
	else
	{
		vPrint("Saved validation pdistances to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadValidationPdistances()
{
	char * pathname = ValidationPdistancesPath();
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load distances to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	if ((result.Rows() != validation_number) ||
		(result.Cols() != validation_number))
	{
		function_warning("Warning: bad matrix %li x %li in LoadValidationPdistances\n",
			(long) result.Rows(), (long) result.Cols());
		vdelete2(pathname);
		return vMatrix<float>();
	}
	else
	{
		vPrint("Loaded validation pdistances from %s\n", pathname);
	}

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::ValidationPdistancesName()
{
	char * simple_name = "validation_pdistances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::ValidationPdistancesPath()
{
	char * simple_name = ValidationPdistancesName();
	char * result = training_pathname(simple_name);
	vdelete2(simple_name);
	return result;
}


// save all the training labels (i.e. class labels for all objects
// of the training set). typically we call this
// function once for an entire dataset (say the MNIST database),
// and not for a specific BoostMap_data. The same is true
// for most of these static functions.
vint8 BoostMap_data::SaveTrainingLabels(const char * rootDir, const char * dataset_name, 
	vMatrix<float> labels)
{
	if (labels.valid() <= 0)
	{
		function_warning("Warning: bad matrix %li x %li in SaveTrainingLabels\n",
			(long) labels.Rows(), (long) labels.Cols());
		return 0;
	}

	char * pathname = TrainingLabelsPath(rootDir, dataset_name);
	vint8 success = labels.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save training labels to %s\n", pathname);
	}
	else
	{
		vPrint("Saved training labels to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadTrainingLabels(const char * rootDir, const char * dataset_name)
{
	char * pathname = TrainingLabelsPath(rootDir, dataset_name);
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load training labels from %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	vPrint("Loaded training labels from %s\n", pathname);

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::TrainingLabelsName()
{
	char * simple_name = "training_labels.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::TrainingLabelsPath(const char * rootDir, const char * dataset_name)
{
	char * target_dir = original_directory(rootDir, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = TrainingLabelsName();
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


vint8 BoostMap_data::SaveTestLabels(const char * rootDir, const char * dataset_name, 
	vMatrix<float> labels)
{
	if (labels.valid() <= 0)
	{
		function_warning("Warning: bad matrix %li x %li in SaveTestLabels\n",
			(long) labels.Rows(), (long) labels.Cols());
		return 0;
	}

	char * pathname = TestLabelsPath(rootDir, dataset_name);
	vint8 success = labels.Write(pathname);
	if (success <= 0)
	{
		vPrint("Failed to save test labels to %s\n", pathname);
	}
	else
	{
		vPrint("Saved test labels to %s\n", pathname);
	}
	vdelete2(pathname);
	return success;
}


vMatrix<float> BoostMap_data::LoadTestLabels(const char * rootDir, const char * dataset_name)
{
	char * pathname = TestLabelsPath(rootDir, dataset_name);
	vMatrix<float> result = vMatrix<float>::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("Failed to load test labels to %s\n", pathname);
		vdelete2(pathname);
		return result;
	}

	vPrint("Loaded test labels from %s\n", pathname);

	vdelete2(pathname);
	return result;
}


char * BoostMap_data::TestLabelsName()
{
	char * simple_name = "test_labels.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::TestLabelsPath(const char * rootDir, const char * dataset_name)
{
	char * target_dir = original_directory(rootDir, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = TestLabelsName();
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


// interfaces for accessing distances from the entire test set
// to the entire training set, or from the entire training
// set to the entire training set.
char * BoostMap_data::TestTrainDistancesName()
{
	char * simple_name = "testtrain_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}

char * BoostMap_data::TestTrainDistancesPath()
{
	return TestTrainDistancesPath(root_data_dir, original_dataset_name);
}

char * BoostMap_data::TestTrainDistancesPath(const char * rootDir, const char * dataset_name)
{
	char * target_dir = original_directory(rootDir, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = TestTrainDistancesName();
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


// file storing distances from test objects to reference objects (useful for
// the protein dataset, where most reference objects are not part of the
// database).
char * BoostMap_data::RefTestDistancesName()
{
	char * simple_name = "reftest_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}


char * BoostMap_data::RefTestDistancesPath(const char * directory)
{
	char * target_dir = original_directory(g_data_directory, directory);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = RefTestDistancesName();
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


vMatrix<float> BoostMap_data::TestTrainDistance(const char * directory, 
	vint8 index)
{
	char * filename = TestTrainDistancesPath(g_data_directory, directory);
	vMatrix<float> result = ObjectDistances(filename, index);
	vdelete2(filename);
	return result;
}


char * BoostMap_data::TrainTrainDistancesName()
{
	char * simple_name = "traintrain_distances.bin";
	char * result = vCopyString(simple_name);
	return result;
}

char * BoostMap_data::TrainTrainDistancesPath()
{
	return TrainTrainDistancesPath(root_data_dir, original_dataset_name);
}

char * BoostMap_data::TrainTrainDistancesPath(const char * rootDir, const char * dataset_name)
{
	char * target_dir = original_directory(rootDir, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = TrainTrainDistancesName();
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


vMatrix<float> BoostMap_data::TrainTrainDistance(const char * rootDir, 
	const char * dataset_name, vint8 index)
{
	char * filename = TrainTrainDistancesPath(rootDir, dataset_name);
	vMatrix<float> result = ObjectDistances(filename, index);
	vdelete2(filename);
	return result;
}


float BoostMap_data::test_train_distance(const char * original_dataset_name, const vint8 first, 
	const vint8 second, vint8 * success_pointer)
{
	*success_pointer = 0;
	vMatrix<float> distances = BoostMap_data::TestTrainDistance(original_dataset_name, first);
	if ((distances.valid() <= 0) || (distances.Size() <= first))
	{
		return 0;
	}

	if (second >= distances.Size())
	{
		return 0;
	}
	float distance = distances(second);
	*success_pointer = 1;
	return distance;
}


float BoostMap_data::train_train_distance(const char * original_dataset_name, const vint8 first, 
	const vint8 second, vint8 * success_pointer)
{
	*success_pointer = 0;
	
	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, 
		original_dataset_name, first);

	if ((distances.valid() <= 0) || (distances.Size() <= first))
	{
		return 0;
	}

	if (second >= distances.Size())
	{
		return 0;
	}
	float distance = distances(second);
	*success_pointer = 1;
	return distance;
}


// opens the appropriate file, and reads the header, so that
// we can then start calling NextObjectDistances(). Not having
// such a function caused a bug that took me a whole day
// (Friday 02/20/2004) to fix, wasting one day of work for KDD
// deadline (02/27/2004).
class_file * BoostMap_data::OpenObjectDistancesFile(const char * filename)
{
	class_file * fp = new class_file(filename, vFOPEN_READ);
	if (fp->file_pointer == 0)
	{
		fclose(fp);
		return 0;
	}

	vector<vint8> header;
	vint8 success = vMatrix<float>::ReadHeader(fp, &header);
	vint8 rows = header[1];
	vint8 cols = header[2];

	if ((success <= 0) || (header[0] != vTypeToVint8(FloatType)) ||
		(rows <= 0) || (cols <= 0))
	{
		fclose(fp);
		return 0;
	}

	return fp;  
}


vMatrix<float> BoostMap_data::ObjectDistances(const char * filename,
	vint8 index)
{
	if (index < 0)
	{
		return vMatrix<float>();
	}

	class_file * fp = new class_file(filename, vFOPEN_READ);
	if (fp->file_pointer == 0)
	{
		fclose(fp);
		return vMatrix<float>();
	}

	vector<vint8> header;
	vint8 success = vMatrix<float>::ReadHeader(fp, &header);
	vint8 rows = header[1];
	vint8 cols = header[2];

	if ((success <= 0) || (header[0] != vTypeToVint8(FloatType)) ||
		(rows <= index) || (cols <= 0))
	{
		fclose(fp);
		return vMatrix<float>();
	}

	// Figure out the position from which we should read
	// the distances. 
	vint8 temp1 = cols;
	vint8 temp2 = sizeof(float);
	vint8 temp3 = index;
	vint8 temp4 = header.size() * sizeof(vint4);
	vint8 offset = temp1 * temp2 * temp3 + temp4;
	function_seek(fp, offset, SEEK_SET);

	// read the distances.
	vMatrix<float> result = NextObjectDistances(fp, cols);
	fclose(fp);
	return result;
}


vMatrix<float> BoostMap_data::ObjectDistances3(class_file * fp,
	vint8 index, vint8 number)
{
	if (index < 0)
	{
		return vMatrix<float>();
	}

	// Figure out the position from which we should read
	// the distances. 
	vint8 temp1 = number;
	vint8 temp2 = sizeof(float);
	vint8 temp3 = index;
	vint8 temp4 = general_image::HeaderSize() * sizeof(vint8);
	vint8 offset = temp1 * temp2 * temp3 + temp4;
	function_seek(fp, offset, SEEK_SET);

	// read the distances.
	vMatrix<float> result = NextObjectDistances(fp, number);
	return result;
}


// returns distances from some object (the next one to be read)
// to the entire database (i.e. the entire training set).
vMatrix<float> BoostMap_data::NextObjectDistances(class_file * fp, vint8 number)
{
	if ((fp->file_pointer == 0) || (number <= 0))
	{
		return vMatrix<float>();
	}

	vMatrix<float> result(1, number);
	vint8 success = result.ReadBandwise(fp);
	if (success <= 0)
	{
		return vMatrix<float>();
	}

	return result;
}


vint8 BoostMap_data::Print()
{
	vPrint("\nbm_data:\n");
	if (is_valid <= 0)
	{
		vPrint("is_valid = %li\n", (long) is_valid);
		return 0;
	}

	PrintCandidates();
	PrintTraining();
	PrintValidation();
	return 1;
}


vint8 BoostMap_data::PrintCandidates()
{
	if (is_valid <= 0)
	{
		vPrint("is_valid = %li\n", (long) is_valid);
		return 0;
	}

	vPrint("candidate_number = %li\n", (long) candidate_number);
	candidate_ids.PrintInt("candidate_ids", "%li\n");
	return 1;
}


vint8 BoostMap_data::PrintTraining()
{
	if (is_valid <= 0)
	{
		vPrint("is_valid = %li\n", (long) is_valid);
		return 0;
	}

	vPrint("training_number = %li\n", (long) training_number);
	training_ids.PrintInt("training_ids", "%li\n");
	return 1;
}


vint8 BoostMap_data::PrintValidation()
{
	if (is_valid <= 0)
	{
		vPrint("is_valid = %li\n", (long) is_valid);
		return 0;
	}

	vPrint("validation_number = %li\n", (long) validation_number);
	validation_ids.PrintInt("validation_ids", "%li\n");
	return 1;
}


vint8 BoostMap_data::PrintDistance(vint8 index1, vint8 index2)
{
	if (is_valid <= 0)
	{
		vPrint("is_valid = %li\n", (long) is_valid);
		return 0;
	}

	vMatrix<float> distances;

	distances = LoadCandCandDistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = candidate_ids(index1);
		vint8 id2 = candidate_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), cand-cand distance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadCandTrainDistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = candidate_ids(index1);
		vint8 id2 = training_ids(index2);
		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), cand-train distance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadCandValDistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = candidate_ids(index1);
		vint8 id2 = validation_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), cand. distance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadTrainingDistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = training_ids(index1);
		vint8 id2 = training_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), train distance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadTrainingPdistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = training_ids(index1);
		vint8 id2 = training_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), train pdistance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadValidationDistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = validation_ids(index1);
		vint8 id2 = validation_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), val. distance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	distances = LoadValidationPdistances();
	if ((distances.valid() > 0) &&
		(distances.check_bounds(index1, index2) > 0))
	{
		vint8 id1 = validation_ids(index1);
		vint8 id2 = validation_ids(index2);

		float distance = distances(index1, index2);
		vPrint("(%li, %li) -> (%li, %li), val. pdistance = %f\n",
			(long) index1, (long) index2, (long) id1, (long) id2, distance);
	}

	return 1;
}


// here we choose triples randomly.
vMatrix<vint8> BoostMap_data::MakeTrainingTriples(vint8 number)
{
	vMatrix<vint8> result = MakeTriples(number, training_number);
	return result;
}


vMatrix<vint8> BoostMap_data::MakeValidationTriples(vint8 number)
{
	vMatrix<vint8> result = MakeTriples(number, validation_number);
	return result;
}


// this function actually chooses the triples. Since we
// choose the triples entirely randomly, we don't need to
// know any distances or pdistances.
vint8_matrix BoostMap_data::MakeTriples(vint8 number, vint8 number_of_objects)
{
	if (number < 1)
	{
		return vint8_matrix();
	}
	// if number_of_objects < 3, then we can never
	// pick (q, a, b) such that they are three different
	// objects, therefore we cannot achieve the goal of this
	// function.
	if (number_of_objects < 3)
	{
		return vint8_matrix();
	}

	vint8_matrix resultm(number, 3);
	vArray2(vint8) result = resultm.Matrix2();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		// choose a random q.
		vint8 q = function_random_vint8(0, number_of_objects - 1);
		vint8 a = q, b = q;
		// choose a random a, different than q.
		while(a == q)
		{
			a = function_random_vint8(0, number_of_objects - 1);
		}
		while(a == q)
		{
			a = function_random_vint8(0, number_of_objects - 1);
		}
		// choose a random b, different than a and q.
		while((b == q) || (b == a))
		{
			b = function_random_vint8(0, number_of_objects - 1);
		}
		result[i][0] = q;
		result[i][1] = a;
		result[i][2] = b;
	}

	return resultm;
}


// here we choose the top k same-class nearest neighbors for an object,
// and the top (classes-1)*k different-class nearest neighbors, and
// make triples based on that. If k = -1, then we choose k
// automatically, based on the number of triples we need to form per
// object.
vint8_matrix BoostMap_data::MakeTrainingTriples2(vint8 number, vint8 classes, 
	vint8 k)
{
	vMatrix<float> distancesm = LoadTrainingDistances();
	vint8 number2 = distancesm.Rows();
	vint8 remainder = number % number2;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		vPrint("remainder = %li\n", (long) remainder);
		return vint8_matrix();
	}
	vMatrix<float> pdistancesm = LoadTrainingPdistances();

	vint8_matrix result = MakeTriples2(number, classes, k, 
		distancesm, pdistancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeValidationTriples2(vint8 number, vint8 classes, 
	vint8 k)
{
	vMatrix<float> distancesm = LoadValidationDistances();
	vint8 number2 = distancesm.Rows();
	vint8 remainder = number % number2;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		vPrint("remainder = %li\n", (long) remainder);
		return vint8_matrix();
	}
	vMatrix<float> pdistancesm = LoadValidationPdistances();

	vint8_matrix result = MakeTriples2(number, classes, k, 
		distancesm, pdistancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeTriples2(vint8 number_of_triples, vint8 classes, 
	vint8 k, vMatrix<float> distancesm, 
	vMatrix<float> pdistancesm)
{
	if ((number_of_triples <= 1) || (classes <= 1))
	{
		exit_error("error: maketriples2, number = %li, classes = %li\n",
			(long) number_of_triples, (long) classes);
	}

	if ((distancesm.valid() <= 0) || (pdistancesm.valid() <= 0))
	{
		exit_error("error: in maketriples2, invalid distances\n");
	}

	vint8 number = distancesm.Rows();
	if ((number != distancesm.Cols()) || (number != pdistancesm.Rows()) ||
		(number != pdistancesm.Cols()))
	{
		exit_error("error: maketriples2, dimensions don't match\n");
	}

	vint8 remainder = number_of_triples % number;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		vPrint("remainder = %li\n", (long) remainder);
		return vint8_matrix();
	}

	// figure out how many triples we will create per object (i.e.,
	// for each possible q, how many triples (q, a, b) will have that q).
	vint8 triples_per_object = number_of_triples / number;

	// if k is negative, set k to a default value.
	if (k == -1)
	{
		k = triples_per_object / (classes - 1);
		vPrint("k = %li\n", (long) k);
		if (k * (classes - 1) < triples_per_object)
		{
			k = k + 1;
		}
		vPrint("k = %li\n", (long) k);
	}

	// get the number of all the possible triples 
	// we can construct by choosing 
	// i, its j-th same-class nearest neighbor (j <= k) and its 
	// j'-th other-class nearest neighbor, where j' is 
	// from (classes-1)*j to (classes-1) * (j+1) - 1.
	vint8 possible_triples = (classes - 1) * k;

	if (triples_per_object > possible_triples)
	{
		exit_error("error: triples_per_object > possible_triples (%li > %li)\n",
			(long) triples_per_object, (long) possible_triples);
	}

	vint8_matrix result(number_of_triples, 3);
	float max_distance = function_image_maximum(&distancesm);
	float distance_limit = vAbs(max_distance) * ((float) 2.0) + (float) 1.0;
	vint8 row = 0;
	vint8 i, j;
	for (i = 0; i < number; i++)
	{
		// get distances from i to all other objects.
		v3dMatrix<float> i_distances = copy_horizontal_line(&distancesm, i);
		// write over distance from i to itself.
		i_distances.Matrix()[i] = distance_limit;
		v3dMatrix<float> i_pdistances = copy_horizontal_line(&pdistancesm, i);

		// get the k same-class nearest neighbors of i.
		vint8_matrix k_same_class = SameClassKnn(i_distances, i_pdistances, k);

		// get the k * (classes-1) other-class nearest neighbors of i.
		vint8_matrix k_other_class = OtherClassKnn(i_distances, i_pdistances, 
			k * (classes - 1));

		// choose randomly a subset of those triples. Here we just pick
		// indices, and the for loop that follows will choose a triple
		// based on each index, in a deterministic way.
		vint8_matrix picks = sample_without_replacement(0, possible_triples-1, 
			triples_per_object);
		for (j = 0; j < triples_per_object; j++)
		{
			// get the index of the j-th pick.
			vint8 index = picks(j);

			// figure out the triple corresponding to that index.
			// First, figure out the current k
			vint8 current_k = index % k;

			// now figure out the rank of the other-class nearest neighbor
			// (i.e. the b of the triple).
			vint8 current_rem = index / k;
			//      vint8 other_index = current_k * (classes - 1) + current_rem;
			vint8 other_index = 0 * (classes - 1) + current_rem;
			vint8 q = i;
			if ((current_k >= k_same_class.Size()) || 
				(other_index >= k_other_class.Size()))
			{
				exit_error("error: maketriples2, impossible\n");
			}

			// get a and b based on the above choices.
			vint8 a = k_same_class(current_k);
			vint8 b = k_other_class(other_index);
			result(row, 0) = q;
			result(row, 1) = a;
			result(row, 2) = b;
			row = row + 1;
		}
	}

	if (row != number_of_triples)
	{
		exit_error("error: maketriples2, row = %li, number_of_triples = %li\n",
			(long) row, (long) number_of_triples);
	}

	return result;
}


// right now there is no difference between these functions and
// the "Triples2" functions.
vint8_matrix BoostMap_data::MakeTrainingTriples3(vint8 number, vint8 k)
{
	vMatrix<float> distancesm = LoadTrainingDistances();
	vint8 number2 = distancesm.Rows();
	vMatrix<float> pdistancesm = LoadTrainingPdistances();

	vint8_matrix result = MakeTriples3(number, k, 
		distancesm, pdistancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeValidationTriples3(vint8 number, vint8 k)
{
	vMatrix<float> distancesm = LoadValidationDistances();
	vint8 number2 = distancesm.Rows();
	vMatrix<float> pdistancesm = LoadValidationPdistances();

	vint8_matrix result = MakeTriples3(number, k, 
		distancesm, pdistancesm);
	return result;
}


// here each triple consists of a query, its w-th same-class nearest
// neighbor, and its w-th other-class nearest neighbor, where w <= k.
vint8_matrix BoostMap_data::MakeTriples3(vint8 number_of_triples, vint8 k, 
	vMatrix<float> distancesm, 
	vMatrix<float> pdistancesm)
{
	if (number_of_triples <= 1)
	{
		exit_error("error: maketriples3, number = %li\n",
			(long) number_of_triples);
	}

	if ((distancesm.valid() <= 0) || (pdistancesm.valid() <= 0))
	{
		exit_error("error: in maketriples3, invalid distances\n");
	}

	vint8 number = distancesm.Rows();
	if ((number != distancesm.Cols()) || (number != pdistancesm.Rows()) ||
		(number != pdistancesm.Cols()))
	{
		exit_error("error: maketriples3, dimensions don't match\n");
	}

	vint8 remainder = number_of_triples % number;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		vPrint("remainder = %li\n", (long) remainder);
		return vint8_matrix();
	}

	// figure out how many triples we will create per object (i.e.,
	// for each possible q, how many triples (q, a, b) will have that q).
	vint8 triples_per_object = number_of_triples / number;

	// if k is negative, set k to a default value.
	if (k == -1)
	{
		k = triples_per_object;
		vPrint("k = %li\n", (long) k);
	}
	vPrint("k = %li\n", (long) k);

	vint8_matrix result(number_of_triples, 3);
	float max_distance = function_image_maximum(&distancesm);
	float distance_limit = vAbs(max_distance) * ((float) 2.0) + (float) 1.0;
	vint8 row = 0;
	vint8 i, j;
	for (i = 0; i < number; i++)
	{
		// get distances from i to all other objects.
		v3dMatrix<float> i_distances = copy_horizontal_line(&distancesm, i);
		// write over distance from i to itself.
		i_distances.Matrix()[i] = distance_limit;
		v3dMatrix<float> i_pdistances = copy_horizontal_line(&pdistancesm, i);

		// get the k same-class nearest neighbors of i.
		vint8_matrix k_same_class = SameClassKnn(i_distances, i_pdistances, k);

		// get the k other-class nearest neighbors of i.
		vint8_matrix k_other_class = OtherClassKnn(i_distances, i_pdistances, k);
		vint8_matrix picks = sample_without_replacement(0, k-1, triples_per_object);

		for (j = 0; j < triples_per_object; j++)
		{
			// get the index of the j-th pick.
			vint8 w = picks(j);

			// get a and b based on the above choices.
			vint8 q = i;
			vint8 a = k_same_class(w);
			vint8 b = k_other_class(w);
			result(row, 0) = q;
			result(row, 1) = a;
			result(row, 2) = b;
			row = row + 1;
		}
	}

	if (row != number_of_triples)
	{
		exit_error("error: maketriples2, row = %li, number_of_triples = %li\n",
			(long) row, (long) number_of_triples);
	}

	return result;
}


// Here we give rank ranges for a and b with respect to q.
vint8_matrix BoostMap_data::MakeTrainingTriples4(vint8 number, vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b)
{
	vMatrix<float> distancesm = LoadTrainingDistances();
	vint8_matrix result = MakeTriples4(number, min_a, max_a, 
		min_b, max_b, distancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeValidationTriples4(vint8 number, vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b)
{
	vMatrix<float> distancesm = LoadValidationDistances();
	vint8_matrix result = MakeTriples4(number, min_a, max_a, 
		min_b, max_b, distancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeTriples4(vint8 number_of_triples, 
	vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b,
	vMatrix<float> distancesm)
{
	if (number_of_triples <= 1)
	{
		exit_error("error: maketriples4, number = %li\n",
			(long) number_of_triples);
	}

	if (distancesm.valid() <= 0)
	{
		exit_error("error: in maketriples4, invalid distances\n");
	}

	vint8 number = distancesm.Rows();
	if (number != distancesm.Cols())
	{
		exit_error("error: maketriples4, dimensions don't match\n");
	}

	if ((min_a < 1) || (min_b < 1) || 
		(max_a < min_a) || (max_b <= min_b) ||
		(max_a > number) || (max_b > number))
	{
		exit_error("error: maketriples4, bad rank ranges\n");
	}

	vint8 remainder = number_of_triples % number;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		number_of_triples = number_of_triples + number - remainder;
	}

	// figure out how many triples we will create per object (i.e.,
	// for each possible q, how many triples (q, a, b) will have that q).
	vint8 triples_per_object = number_of_triples / number;

	vint8_matrix result(number_of_triples, 3);
	float max_distance = function_image_maximum(&distancesm);
	float distance_limit = float(vAbs(max_distance) * 2.0 + 1.0);
	vint8 row = 0;
	vint8 i, j;
	for (i = 0; i < number; i++)
	{
		// get distances from i to all other objects.
		v3dMatrix<float> i_distances_temp = copy_horizontal_line(&distancesm, i);
		vMatrix<float> i_distances(&i_distances_temp);
		// write over distance from i to itself.
		i_distances.Matrix()[i] = distance_limit;

		vint8 k = Max(max_a, max_b);
		// get the k nearest neighbors of i.
		vMatrix<float> knns = FindKnn2(i_distances, k);

		for (j = 0; j < triples_per_object; j++)
		{
			// get the index of the j-th pick.
			vint8 q = i;
			vint8 a_rank = function_random_vint8(min_a-1, max_a-1);
			vint8 b_rank = a_rank;
			while(b_rank == a_rank)
			{
				b_rank = function_random_vint8(min_b-1, max_b-1);
			}

			vint8 a = round_number(knns(a_rank, 0));
			vint8 b = round_number(knns(b_rank, 0));
			result(row, 0) = q;
			result(row, 1) = a;
			result(row, 2) = b;
			row = row + 1;
		}
	}

	if (row != number_of_triples)
	{
		exit_error("error: maketriples4, row = %li, number_of_triples = %li\n",
			(long) row, (long) number_of_triples);
	}

	return result;
}


// These functions were custom made for the protein dataset, where
// triples (q, a, b) are made so that q comes from a query dataset
// and a, b, come from the database. query_idsm are NOT the
// ids of queries in the database, but ids of queries from the
// query set in the set of objects from which a and b are drawn.
// If q does not appear in that set, its query id is -1.
vint8_matrix BoostMap_data::MakeTrainingTriples5(vint8 number, vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b)
{
	vMatrix<float> distancesm = LoadTrainingDistances();
	vint8_matrix result = MakeTriples5(number, min_a, max_a, 
		min_b, max_b, distancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeValidationTriples5(vint8 number, vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b)
{
	vMatrix<float> distancesm = LoadValidationDistances();
	vint8_matrix result = MakeTriples5(number, min_a, max_a, 
		min_b, max_b, distancesm);
	return result;
}


vint8_matrix BoostMap_data::MakeTriples5(vint8 number_of_triples, 
	vint8 min_a, vint8 max_a,
	vint8 min_b, vint8 max_b,
	vMatrix<float> distancesm)
{
	if (number_of_triples <= 1)
	{
		exit_error("error: maketriples4, number = %li\n",
			(long) number_of_triples);
	}

	if (distancesm.valid() <= 0)
	{
		exit_error("error: in maketriples2, invalid distances\n");
	}

	vint8 rows = distancesm.Rows();
	vint8 cols = distancesm.Cols();
	if ((rows != cols) || (rows % 2 != 0))
	{
		exit_error("error: maketriples5, dimensions don't match\n");
	}

	vint8 number = rows / 2;
	if ((min_a < 1) || (min_b < 1) || 
		(max_a <= min_a) || (max_b <= min_b) ||
		(max_a > number) || (max_b > number))
	{
		exit_error("error: maketriples4, bad rank ranges\n");
	}

	vint8 remainder = number_of_triples % number;
	// to simplify things, we will require remainder to be 0, for the 
	// time being.
	if (remainder != 0)
	{
		vPrint("remainder = %li\n", (long) remainder);
		return vint8_matrix();
	}

	// figure out how many triples we will create per object (i.e.,
	// for each possible q, how many triples (q, a, b) will have that q).
	vint8 triples_per_object = number_of_triples / number;

	vint8_matrix result(number_of_triples, 3);
	float max_distance = function_image_maximum(&distancesm);

	float distance_limit = vAbs(max_distance) * ((float) 2.0) + (float) 1.0;
	vint8 row = 0;
	vint8 q, j;
	for (q = 0; q < number; q++)
	{
		// get distances from i to all other objects.
		v3dMatrix<float> q_distances_temp = copy_horizontal_line(&distancesm, q);
		//    vint8 q_id = query_idsm(q);
		//    if (q_id >= 0)
		//    {
		// write over distance from q to itself.
		//      q_distances_temp.Matrix()[q_id] = distance_limit;
		//   }

		// as a sanity check, make sure that the min distance is non-negative
		float min_distance = function_image_minimum(&q_distances_temp);
		if (min_distance < 0)
		{
			exit_error("error: MakeTriples5: min_distance = %f\n", min_distance);
		}


		vMatrix<float> q_distances(1, number);
		for (j = 0; j < number; j++)
		{
			q_distances(j) = q_distances_temp.Matrix()[j + number];
		}

		vint8 k = Max(max_a, max_b);
		// get the k same-class nearest neighbors of i.
		vMatrix<float> knns = FindKnn2(q_distances, k);

		for (j = 0; j < triples_per_object; j++)
		{
			// get the index of the j-th pick.
			vint8 a_rank = function_random_vint8(min_a-1, max_a-1);
			vint8 b_rank = a_rank;
			while(b_rank == a_rank)
			{
				b_rank = function_random_vint8(min_b-1, max_b-1);
			}

			vint8 a = round_number(knns(a_rank, 0)) + number;
			vint8 b = round_number(knns(b_rank, 0)) + number;
			result(row, 0) = q;
			result(row, 1) = a;
			result(row, 2) = b;
			row = row + 1;
		}
	}

	if (row != number_of_triples)
	{
		exit_error("error: maketriples4, row = %li, number_of_triples = %li\n",
			(long) row, (long) number_of_triples);
	}

	return result;
}


// SameClassKnn is given, for some object, its distances and
// pdistances to a set of other objects. We select the k-nearest
// neighbors (based on distances) such that their pdistances are 0.
// result(i) is the index in distancesm and pdistancesm of the
// i-th same-class nearest neighbor.
vint8_matrix BoostMap_data::SameClassKnn(v3dMatrix<float> distancesm, 
	v3dMatrix<float> pdistancesm, vint8 k)
{
	vint8_matrix result = PdistanceClassKnn(distancesm, pdistancesm, 
		k, (float) 0);
	return result;
}

vint8_matrix BoostMap_data::OtherClassKnn(v3dMatrix<float> distancesm, 
	v3dMatrix<float> pdistancesm, vint8 k)
{
	v3dMatrix<float> filtered_pdistances = vSelectGreater(&pdistancesm, 
		(float) 0);
	vint8_matrix result = PdistanceClassKnn(distancesm, filtered_pdistances, 
		k, (float) 1);
	return result;
}

// this is just an auxiliary function that does the work for
// SameClassKnn and OtherClassKnn
vint8_matrix BoostMap_data::PdistanceClassKnn(v3dMatrix<float> distancesm,
	v3dMatrix<float> pdistancesm, 
	vint8 k, float p_distance)
{
	vint8_matrix indices = vGetIndices(&pdistancesm, p_distance);
	if (indices.Size() < k)
	{
		exit_error("error: PdistanceClassKnn, %li indices, k = %li\n",
			(long) indices.Size(), (long) k);
	}

	vector<class_couple> pairs;
	vGetIndicesValues(&distancesm, &indices, &pairs);
	std::sort(pairs.begin(), pairs.end(), couple_less());

	vint8_matrix result(1, k);
	vint8 i;
	for (i = 0; i < k; i++)
	{
		result(i) = (vint8) (long) (pairs[(vector_size) i].object);
	}

	return result;
}


// in these functions, we get the training 
// or validation distances or pdistances
// for the specified triples.
// If the i-th triple is of form (q, a, b), result(i) is
// D(q, b) - D(q, a). 
vMatrix<float> BoostMap_data::TripleTrainingDistances(vint8_matrix triples)
{
	vMatrix<float> distances = LoadTrainingDistances();
	vMatrix<float> result = TripleDistances(triples, distances);
	return result;
}


vMatrix<float> BoostMap_data::TripleTrainingPdistances(vint8_matrix triples)
{
	vMatrix<float> distances = LoadTrainingPdistances();
	vMatrix<float> result = TripleDistances(triples, distances);
	return result;
}


vMatrix<float> BoostMap_data::TripleValidationDistances(vint8_matrix triples)
{
	vMatrix<float> distances = LoadValidationDistances();
	vMatrix<float> result = TripleDistances(triples, distances);
	return result;
}


vMatrix<float> BoostMap_data::TripleValidationPdistances(vint8_matrix triples)
{
	vMatrix<float> distances = LoadValidationPdistances();
	vMatrix<float> result = TripleDistances(triples, distances);
	return result;
}


// TripleDistances does the work for the four preceding functions.
vMatrix<float> BoostMap_data::TripleDistances(vint8_matrix triples_matrix, 
	vMatrix<float> distances_matrix)
{
	vArray2(vint8) triples = triples_matrix.Matrix2();
	vArray2(float) distances = distances_matrix.Matrix2();


	vint8 triple_number = triples_matrix.Rows();
	vint8 triple_cols = triples_matrix.Cols();
	if ((triple_number <= 0) || (triple_cols != 3))
	{
		function_warning("Warning: bad size of triples_matrix: %li x %li\n",
			(long) triple_number, (long) triple_cols);
		return vMatrix<float>();
	}

	vMatrix<float> result(1, triple_number);

	vint8 i;
	for (i = 0; i < triple_number; i++)
	{
		vint8 q = triples[i][0];
		vint8 a = triples[i][1];
		vint8 b = triples[i][2];

		if ((distances_matrix.check_bounds(q, a) <= 0) ||
			(distances_matrix.check_bounds(q, b) <= 0))
		{
			function_warning("Warning: bad (q, a, b): %li %li %li, distances is %li x %li\n",
				(long) q, (long) a, (long) b, (long) distances_matrix.Rows(), (long) distances_matrix.Cols());
		}

		float distance1 = distances[q][a];
		float distance2 = distances[q][b];
		if ((distance1 < 0) || (distance2 < 0))
		{
			exit_error("error: negative distances in TripleDistances\n");
		}
		result(i) = distance2 - distance1;
	}

	return result;
}


// this function chooses pairs of objects to be used for optimizing
// an embedding for stress and distortion. Since we
// choose the pairs entirely randomly, we don't need to
// know any distances or pdistances.
vint8_matrix BoostMap_data::make_couples(vint8 number, vint8 number_of_objects)
{
	if (number < 1)
	{
		return vint8_matrix();
	}
	// if number_of_objects < 2, then we cannot pick any pair
	if (number_of_objects < 2)
	{
		return vint8_matrix();
	}

	vint8_matrix result(number, 2);

	vint8 i;
	for (i = 0; i < number; i++)
	{
		// choose a random q.
		vint8 q = function_random_vint8(0, number_of_objects - 1);
		//    vint8 q = 933;
		vint8 a = q;
		// choose a random a, different than q.
		while(a == q)
		{
			a = function_random_vint8(0, number_of_objects - 1);
		}

		result(i, 0) = q;
		result(i, 1) = a;
	}

	return result;
}


float_matrix BoostMap_data::couple_distances(vint8_matrix couples, float_matrix distances)
{
	vint8 couple_number = couples.Rows();
	if ((couple_number <= 0) || (couples.horizontal() != 2))
	{
		function_warning("Warning: bad size of triples_matrix: %li x %li\n",
			(long) couple_number, (long) couples.horizontal());
		return vMatrix<float>();
	}

	vMatrix<float> result(1, couple_number);

	vint8 i;
	for (i = 0; i < couple_number; i++)
	{
		vint8 q = couples(i, 0);
		vint8 a = couples(i, 1);

		if (distances.check_bounds(q, a) <= 0)
		{
			exit_error("error: bad (q, a): %li %li, distances is %li x %li\n",
				(long) q, (long) a, (long) distances.Rows(), (long) distances.Cols());
		}

		float distance = distances(q, a);
		if (distance < 0)
		{
			exit_error("error: negative distance in couple_distances\n");
		}
		result(i) = distance;
	}

	return result;
}


float_matrix BoostMap_data::training_couple_distances(vint8_matrix couples)
{
	vMatrix<float> distances = LoadTrainingDistances();
	vMatrix<float> result = couple_distances(couples, distances);
	return result;
}


float_matrix BoostMap_data::validation_couple_distances(vint8_matrix couples)
{
	vMatrix<float> distances = LoadValidationDistances();
	vMatrix<float> result = couple_distances(couples, distances);
	return result;
}



// name: the name of a dataset (like "mnist"). Note that here we are
// using the type of name we would use in the static functions like
// ObjectDistances, i.e. a name specifying a directory where we 
// store UNSAMPLED information. We don' use a name like in 
// the bmDataset constructor, since that name specifies a directory
// where we save a particular sampled subset of the training set.
// Index: the index of a training or test object in that dataset.
// test_flag: 0 if index refers to a training object, 1 if 
// it refers to a test object. result(i, 0) is the overall rank of the
// i-class k-nearest neighbor of the object. 
//
// result(i, 1) is the actual
// distance of the i-class k-nearest neighbor of the object to the 
// object.
vMatrix<float> BoostMap_data::WknnRanks4(const char * name, vint8 index, 
	vint8 test_flag, vint8 k)
{
	vMatrix<float> distances;
	if (test_flag == 0)
	{
		distances = TrainTrainDistance(g_data_directory, name, index);
	}
	else
	{
		distances = TestTrainDistance(name, index);
	}

	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, name);
	vMatrix<float> result = WknnRanks3(distances, labels, k);
	return result;
}


// here distances is the distances from an object to all training
// objects, and labels are training labels.
vMatrix<float> BoostMap_data::WknnRanks3(vMatrix<float> distances, 
	vMatrix<float> labels, vint8 k)
{
	if (distances.valid() <= 0)
	{
		vPrint("invalid distances\n");
		return vMatrix<float>();
	}

	if (labels.valid() <= 0)
	{
		vPrint("invalid labels\n");
		return vMatrix<float>();
	}

	if (distances.Size() > labels.Size())
	{
		vPrint("fewer labels than distances: %li, %li\n",
			(long) distances.Size(), (long) labels.Size());
		return vMatrix<float>();
	}

	// get, for each object, its similarity rank with respect
	// to the object described in distances.
	vector<vint8> ranks;
	vector<float> distances_vector;
	vector_from_matrix(&distances, &distances_vector);
	vIndexRanks(&distances_vector, &ranks);

	// initialize a vector that will receive vectors
	// holding distance/rank pairs, for each class.
	vector<vector<class_couple> > splitted_distances;
	vint8 number_of_classes = (vint8) (function_image_maximum(&labels) + 1);
	splitted_distances.reserve((vector_size) number_of_classes);
	vint8 i;
	for (i = 0; i < number_of_classes; i++)
	{
		splitted_distances.push_back(vector<class_couple>());
	}

	SplitDistances(distances, labels, &splitted_distances);

	// sort the class-specific vectors, to find the k-nearest
	// neighbor for each class.
	SortDistances(&splitted_distances);

	vint8 number = splitted_distances.size();
	vMatrix<float> result(number, 2);

	// for each class, find the k-nearest neighbor to the object.
	for (i = 0 ; i < number; i++)
	{
		if (splitted_distances[(vector_size) i].size() <= (ulong) k)
		{
			result(i, 0) = -1;
			result(i, 1) = -1;
		}
		else
		{
			vint8 index = (vint8) (long) (splitted_distances[(vector_size) i][(vector_size) k].object);
			float distance = splitted_distances[(vector_size) i][(vector_size) k].value;
			vint8 rank = ranks[(vector_size) index];
			result(i, 0) = (float) rank;
			result(i, 1) = distance;
		}
	}

	return result;
}

// Here object is the embedding of an object, database is the
// embedding of all database objects, weights specifies the
// weight to be used in each dimension (for the weighted L1
// distance), and labels are the training labels.
vMatrix<float> BoostMap_data::WknnRanks5(vMatrix<float> object, 
	vMatrix<float> database,
	vMatrix<float> weights,
	vMatrix<float> labels,
	vint8 k)
{
	vint8 dimensions = object.Size();
	vint8 number = database.Rows();
	if (dimensions != database.Cols())
	{
		vPrint("Incompatible dimensions: %li %li\n",
			(long) dimensions, (long) database.Cols());
		return vMatrix<float>();
	}

	if ((object.valid() <= 0) || (database.valid() <= 0) || 
		(weights.valid() <= 0) || (labels.valid() <= 0))
	{
		vPrint("Invalid inputs\n");
		return vMatrix<float>();
	}

	// get distances from object to entire database, so that 
	// we can call WknnRanks3.
	vMatrix<float> distances = L1Distances(&object, database, weights);
	vMatrix<float> result = WknnRanks3(distances, labels, k);

	return result;
}


// result(k-1, 0) is the index of the k-nearest neighor of object specified
// by index. result(k-1, 1) is the distance of that neighbor to the 
// specified object.
vMatrix<float> BoostMap_data::FindKnn4(const char * name, vint8 index, 
	vint8 test_flag, vint8 k)
{
	vMatrix<float> distances;
	if (test_flag == 0)
	{
		distances = TrainTrainDistance(g_data_directory, name, index);
	}
	else
	{
		distances = TestTrainDistance(name, index);
	}

	vMatrix<float> result = FindKnn2(distances, k);
	return result;
}


// here instead of passing name, index and test_flag, we just pass the
// distances of the object to all training objects.
vMatrix<float> BoostMap_data::FindKnn2(vMatrix<float> distances, vint8 k)
{
	vector<vint8> ranks;
	vector<float> distances_vector;
	vector_from_matrix(&distances, &distances_vector);
	vSortedRanks(&distances_vector, &ranks);

	if (k > (vint8) ranks.size())
	{
		k = ranks.size();
	}

	vMatrix<float> result(k, 2);
	vint8 i;

	for (i = 0; i < k; i++)
	{
		vint8 rank = ranks[(vector_size) i];
		result(i, 0) = (float) rank;
		result(i, 1) = distances(rank);
	}

	return result;
}



// returns the knn-error in a dataset, using the testtrain distances
// saved for that dataset, for k = 1, ..., max_k. result(0) does
// not have a meaningful value.
// NOTE: this is the error using the original distances, not
// the embeddings (unless testtrain distancse stores
// a distance based on embeddings, which doesn't happen so far).
vMatrix<float> BoostMap_data::KnnError2(const char * name, vint8 max_k)
{
	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> train_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		float label = test_labels(i);
		// get distances to all training objects.
		vMatrix<float> distances = TestTrainDistance(name, i);

		// get, for each k (from 1 to max_k) the class label that 
		// is the result of k-nn classification of the object.
		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError2\n");
		}

		// convert labels into 0,1 values (0 for the correct label,
		// 1 for all other labels).
		vint8 j;
		for (j = 0; j < max_k+1; j++)
		{
			if (labels(j) == label)
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}

		// add the results of this objects into result.
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	// divide by number of objects, to get error rates.
	result = result / (float) test_number;
	return result;
}


// returns the training knn-error in a dataset, 
// using the traintrain distances
// saved for that dataset, for k = 1, ..., max_k.
// overall, very similar to KnnError2, but we 
// use different distances (traintrain), and we
// need to discard the object itself when we 
// find its nearest neighbors. In principle, I should
// have KnnError2 and this function call another function
// to do the work, as opposed to duplicating code, which
// is what I'm doing now.
// NOTE: this is the error using the original distances, not
// the embeddings (unless traintrain distancse stores
// a distance based on embeddings, which doesn't happen so far).
vMatrix<float> BoostMap_data::KnnTrainError2(const char * name, vint8 max_k)
{
	vMatrix<float> train_labels = LoadTrainingLabels(g_data_directory, name);
	if (train_labels.valid() <= 0)
	{
		return vMatrix<float>();
	}

	vint8 training_number = train_labels.Size();
	if (training_number == 0)
	{
		exit_error("Error: training = %li\n", (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < training_number; i++)
	{
		float label = train_labels(i);
		vMatrix<float> distances = TrainTrainDistance(g_data_directory, name, i);

		// since it is training error, we should disregard the object itself.
		float max_distance = vAbs(function_image_maximum(&distances));
		distances(i) = max_distance * ((float) 2.0) + (float) 1.0;

		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError2\n");
		}

		vint8 j;
		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) training_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	result = result / (float) training_number;
	return result;
}



// result(k) is the classification error we get by classifying
// each object of the test set based on its k nearest neighbors
// in the training set, where distances are weighted L1 distances 
// and the weights are specified by argument "weights". training
// flag is set to 1 if the test set is the same as the training
// set, in which case we should exclude each object from the 
// set of its nearest neighbors. Both the test set and the 
// training set hold, in the i-th row, the embedding of
// some object (test object or training object respectively).
// NOTE: here we measure distance based on embeddings, not
// based on original distance measure.
vMatrix<float> BoostMap_data::KnnError7(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vMatrix<float> weights,
	vMatrix<float> test_labels,
	vMatrix<float> train_labels,
	vint8 max_k, vint8 training_flag)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(weights.valid() <= 0) || (test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0) || (max_k <= 0))
	{
		return vMatrix<float>();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		// get the clas label of the test object
		float label = test_labels(i);

		// get the embedding of the test object
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);

		// get the distances of the object's embedding to 
		// the embeddings of all training objects
		vMatrix<float> distances = L1Distances(&object, training_set, weights);

		// if test set == training set, then we should temporarily "delete"
		// the test object from the training set
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// get the k-nn classification results for k = 1, ..., max_k.
		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		// convert labels to errors (0 for correct label, 1 for other labels).
		vint8 j;
		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}

		// add errors to result.
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	// devide 
	result = result / (float) test_number;
	return result;
}


// Same as KnnError7, but here the distance is a weighted L2
// (Euclidean) distance, as opposed to an L1 distance.
vMatrix<float> BoostMap_data::KnnError7L2(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vMatrix<float> weights,
	vMatrix<float> test_labels,
	vMatrix<float> train_labels,
	vint8 max_k, vint8 training_flag)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(weights.valid() <= 0) || (test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0) || (max_k <= 0))
	{
		return vMatrix<float>();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		float label = test_labels(i);
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		vMatrix<float> distances = L2Distances(&object, training_set, weights);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}
		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		vint8 j;
		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	result = result / (float) test_number;
	return result;
}


// distances are the distances from some test object to all
// training objects. label is the class label of the test object.
// labels are the training labels. result(i) is the i-nn label
// of the object (i.e. its labels using i-nn classification).
vMatrix<float> BoostMap_data::KnnLabel4(vMatrix<float> distancesm,
	vMatrix<float> labelsm,
	vint8 label, vint8 max_k)
{
	if ((distancesm.valid() <= 0) || (labelsm.valid() <= 0) ||
		(labelsm.Size() != distancesm.Size()))
	{
		return vMatrix<float>();
	}


	vMatrix<float> result(1, max_k + 1);
	function_enter_value(&result, (float) 0);
	result(0) = 2;
	vArray(float) distances = distancesm.Matrix();
	vArray(float) labels = labelsm.Matrix();
	vint8 training_number = labelsm.Size();

	// counter will count misclassified objects.
	vint8 threshold_index = 0;
	vint8 j, k;
	vint8 classes = round_number(function_image_maximum(&labelsm)) + 1;
	vint8_matrix votes_matrix(1, classes);
	// max_distances[t] will store the highest distance found between
	// i and one of its k nearest neighbors that is of class t.
	vMatrix<float> max_distances_matrix (1, classes);
	vArray(vint8) votes = votes_matrix.Matrix();
	vArray(float) max_distances = max_distances_matrix.Matrix();
	vint8 negative_counter = 0;
	vint8 tie_counter = 0;
	vint8 distance_tie_counter = 0;

	float max_distance_limit = function_image_maximum(&distancesm) * ((float) 2.0) + (float) 1.0;
	float threshold_distance = kth_smallest_cb(max_k, &distancesm,
		&threshold_index);
	if (threshold_distance < 0)
	{
		negative_counter++;
	}

	vector<float> nn_distances;
	vector<vint8> nn_classes;

	// collect the max_k neighbors (and neighbors after that that are
	// tied to the max_k neighbor).
	for (j = 0; j < training_number; j++)
	{
		if (distances[j] <= threshold_distance)
		{
			nn_distances.push_back(distances[j]);
			nn_classes.push_back(round_number(labels[j]));
		}
	}

	vint8 number = nn_distances.size();
	// Now, collect votes, and get the recognition, for each k.
	for (k = 1; k <= max_k; k++)
	{
		function_enter_value(&votes_matrix, (vint8) 0);
		function_enter_value(&max_distances_matrix, (float) -1000000000);
		long junk = 0;
		float k_threshold_distance = kth_smallest_ca((long) k, &nn_distances, &junk);
		vint8 counter = 0;

		// find the k nearest neighbors, and the max distance for each class
		// (i.e. among all objects of that class that are included in 
		// the k nearest neighbors).
		for (j = 0; j < number; j++)
		{
			float distance = nn_distances[(vector_size) j];
			if (distance <= k_threshold_distance)
			{
				counter++;
				vint8 j_class = nn_classes[(vector_size) j];
				votes[j_class] = votes[j_class] + 1;
				if (max_distances[j_class] < distance)
				{
					max_distances[j_class] = distance;
				}
			}
		}

		// check if we had a tie for the k-th nearest neighbor spot.
		if (counter > k)
		{
			distance_tie_counter++;
		}

		// Find the max number of votes received:
		vint8 max_votes = function_image_maximum(&votes_matrix);

		// Find all classes that received the max number of votes.
		vector<vint8> candidate_classes;
		vint8 c;
		for (c = 0; c < classes; c++)
		{
			if (votes[c] == max_votes)
			{
				candidate_classes.push_back(c);
			}
		}

		if (candidate_classes.size() == 0)
		{
			exit_error("Error: impossible, 0 candidate classes\n");
		}

		float min_max_distance = max_distances[candidate_classes[0]];
		vint8 candidates = candidate_classes.size();
		// Find the min max distance among all candidates, to 
		// break ties if there are more than one candidates.
		for (c = 1; c < candidates; c++)
		{
			vint8 candidate_class = candidate_classes[(vector_size) c];
			float current_max = max_distances[candidate_class];
			if (current_max < min_max_distance)
			{
				min_max_distance = current_max;
			}
		}

		// Now go through candidates again, and only keep the one whose
		// max distance is min_max_distance;
		vector<vint8> final_candidates;
		for (c = 0; c < candidates; c++)
		{
			vint8 candidate_class = candidate_classes[(vector_size) c];
			float current_max = max_distances[candidate_class];
			if (current_max == min_max_distance)
			{
				final_candidates.push_back(candidate_class);
			}
		}

		// check if we still have a tie, after using max_distances
		// for tie-breaking.
		vint8 final_size = final_candidates.size();
		if (final_size == 0)
		{
			exit_error("Error: impossible, 0 final candidate classes\n");
		}

		// if we still have a tie, pick randomly, and add one to 
		// the tie counter.
		if (final_size > 1)
		{
			tie_counter = tie_counter + 1;
			vint8 pick = function_random_vint8(0, final_size - 1);
			result(k) = (float) (final_candidates[(vector_size) pick]);
		}
		else if (final_size == 1)
		{
			result(k) = (float) final_candidates[0];
		}
	}

	return result;
}


// object is the embedding (or vector representation) of some
// object. label is the class label of the test object.
// labels are the training labels. result(i) is the i-nn label
// of the object (i.e. its labels using i-nn classification).
vMatrix<float> BoostMap_data::KnnLabel6(vMatrix<float> object,
	vMatrix<float> training_set,
	vMatrix<float> weights,
	vMatrix<float> training_labels,
	vint8 label, vint8 max_k)
{
	vMatrix<float> distances = L1Distances(&object, training_set, weights);
	vMatrix<float> result = KnnLabel4(distances, training_labels, label, max_k);
	return result;
}


// distances(i) is the distance of query object to the i-th element
// of a set of objects (typically the training set). labels(i) is the
// class label of the i-th object. result[w] contains a vector,
// which will receive all distances between the query and objects
// of class w. Remember that class id 0 is not used, so result[0]
// will not get any distances. The distances are stored into
// result[i] as pairs, storing the distance and the index of the 
// object in its set.
vint8 BoostMap_data::SplitDistances(vMatrix<float> distances, 
	vMatrix<float> labels,
	vector<vector<class_couple> > * result)
{
	vint8 number_of_classes = round_number(function_image_maximum(&labels)) + 1;
	if (result->size() < (ulong) number_of_classes)
	{
		exit_error("Error: not enough space in result (SplitDistances)\n");
	}

	if ((distances.valid() <= 0) || (labels.valid() <= 0) ||
		distances.Size() != labels.Size())
	{
		return 0;
	}

	vint8 training_number = distances.Size();
	vint8 i;
	for (i = 0; i < training_number; i++)
	{
		vint8 label = round_number(labels(i));
		float distance = distances(i);
		class_couple pair(distance, (void *) (long) i);
		(*result)[(vector_size) label].push_back(pair);
	}

	return 1;
}


// the input "distances" is of the same form as the argument "result"
// of SplitDistances, AFTER we have called SplitDistances. 
// SortDistances simply sorts all the vectors in increasing order
// of the distances, 
vint8 BoostMap_data::SortDistances(vector<vector<class_couple> > * distances)
{
	vint8 number = distances->size();
	vint8 i;
	for (i = 0; i < number; i++)
	{
		std::sort((*distances)[(vector_size) i].begin(), (*distances)[(vector_size) i].end(), 
			couple_less());
	}

	return 1;
}


// result(i) is the weighted L1 distance between object
// and the i-th object (i-th row) of the database. The weights
// are specified by the argument "weights".
vMatrix<float> BoostMap_data::L1Distances(v3dMatrix<float> * objectm, 
	vMatrix<float> databasem,
	vMatrix<float> weightsm)
{
	if ((objectm == 0) || (objectm->Size() == 0) || 
		(objectm->Size() != databasem.Cols()))
	{
		return vMatrix<float>();
	}

	if ((databasem.valid() <= 0) || (weightsm.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 rows = databasem.Rows();
	vint8 cols = weightsm.Size();

	vMatrix<float> resultm(1, rows);
	vArray(float) result = resultm.Matrix();
	vArray(float) object = objectm->Matrix();
	vArray2(float) database = databasem.Matrix2();
	vArray(float) weights = weightsm.Matrix();

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vArray(float) training_object = database[row];
		float distance = 0;
		for (col = 0; col < cols; col++)
		{
			distance += vAbs(object[col] - training_object[col]) * weights[col];
		}
		result[row] = distance;
	}

	return resultm;
}


// result(i) is the weighted L2 distance between object
// and the i-th object (i-th row) of the database. The weights
// are specified by the argument "weights". Note that we allow
// weights to have fewer entries than the number of cols in 
// database. In that case, only the first dimensions of database
// and object are used.
vMatrix<float> BoostMap_data::L2Distances(v3dMatrix<float> * objectm, 
	vMatrix<float> databasem,
	vMatrix<float> weightsm)
{
	if ((objectm == 0) || (objectm->Size() == 0) || 
		(objectm->Size() != databasem.Cols()))
	{
		return vMatrix<float>();
	}

	if ((databasem.valid() <= 0) || (weightsm.valid() <= 0) ||
		(databasem.Cols() < weightsm.Size()))
	{
		return vMatrix<float>();
	}

	vint8 rows = databasem.Rows();

	// The next line uses weightsm instead of databasem, so that we
	// can pass a weights matrix that is smaller than the number
	// of columns in databasem.
	//vint8 cols = databasem.Cols();
	vint8 cols = weightsm.Size();

	vMatrix<float> resultm(1, rows);
	vArray(float) result = resultm.Matrix();
	vArray(float) object = objectm->Matrix();
	vArray2(float) database = databasem.Matrix2();
	vArray(float) weights = weightsm.Matrix();

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vArray(float) training_object = database[row];
		float distance = 0;
		for (col = 0; col < cols; col++)
		{
			float x = object[col];
			float y = training_object[col];
			float diff = (x - y) * weights[col];
			distance += diff * diff;
		}
		result[row] = sqrt(distance);
	}

	return resultm;
}


// result(i) is the weighted Lp (p defined based on fourth 
// parameter) distance (raised to the p-th power) between object
// and the i-th object (i-th row) of the database. The weights
// are specified by the argument "weights".
vMatrix<float> BoostMap_data::LpDistances(v3dMatrix<float> * objectm, 
	vMatrix<float> databasem,
	vMatrix<float> weightsm, 
	vint8 distance_p)
{
	if (distance_p == 1)
	{
		return L1Distances(objectm, databasem, weightsm);
	}
	else if (distance_p == 2)
	{
		return L2Distances(objectm, databasem, weightsm);
	}

	if ((objectm == 0) || (objectm->Size() == 0) || 
		(objectm->Size() != databasem.Cols()))
	{
		return vMatrix<float>();
	}

	if ((databasem.valid() <= 0) || (weightsm.valid() <= 0) ||
		(databasem.Cols() != weightsm.Size()))
	{
		return vMatrix<float>();
	}

	vint8 rows = databasem.Rows();
	vint8 cols = databasem.Cols();

	vMatrix<float> resultm(1, rows);
	vArray(float) result = resultm.Matrix();
	vArray(float) object = objectm->Matrix();
	vArray2(float) database = databasem.Matrix2();
	vArray(float) weights = weightsm.Matrix();

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vArray(float) training_object = database[row];
		float distance = 0;
		for (col = 0; col < cols; col++)
		{
			distance += pow(vAbs(object[col] - training_object[col]), (float) distance_p) * weights[col];
		}
		result[row] = distance;
	}

	return resultm;
}


// "name" is the name of an entire dataset. bm_name is the 
// name of a file where we saved a summary of the BoostMap 
// training. From that file, we build a d-dimensional embedding,
// according to the argument "dimensions". result(k) is
// is the k-nn test error for the dataset, for k = 1, ..., max_k.
// Note that here we don't handle sensitive embeddings.
vMatrix<float> BoostMap_data::KnnEmbeddingError(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	// read the embedding description
	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	// select the first "dimensions" dimensions
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// get the weights for the L1 distance.
	vMatrix<float> weights = class_BoostMap::ExtractWeights(classifiers);
	weights.Print("weights");

	// load the class labels
	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (training_labels.valid() <= 0))
	{
		vPrint("Invalid labels distances\n");
		return vMatrix<float>();
	}

	vMatrix<float> embedded_database = embed_database(name, classifiers);
	vint8 database_size = embedded_database.Rows();
	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers, database_size);


	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	// compute the errors.
	vMatrix<float> result = KnnError7(embedded_test_set, embedded_database,
		weights, test_labels, training_labels,
		max_k, 0);

	return result;
}


// same as KnnEmbeddingError, but it measures the training error.
vMatrix<float> BoostMap_data::KnnEmbeddingTrainError(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	vdelete2(bm_path);
	vMatrix<float> weights = class_BoostMap::ExtractWeights(classifiers);

	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if (training_labels.valid() <= 0)
	{
		vPrint("Invalid labels distances\n");
		return vMatrix<float>();
	}

	vMatrix<float> embedded_database = embed_database(name, classifiers);
	if (embedded_database.valid() <= 0)
	{
		vPrint("Failed to embed training set\n");
		return vMatrix<float>();
	}

	vMatrix<float> result = KnnError7(embedded_database, embedded_database,
		weights, training_labels, training_labels, 
		max_k, 1);

	return result;
}


// "name" is the name of an entire dataset. "classifiers" holds,
// at each row, the representation of a 1D embedding. The result
// of this function holds, at each row, the embedding of 
// the database defined by the 1D embeddings stored in classifiers.
vMatrix<float> BoostMap_data::embed_database(const char * name,
	vMatrix<float> classifiers)
{
	// get filename of distances from training set to training set (i.e. from
	// database objects to database objects).
	char * pathname = TrainTrainDistancesPath(g_data_directory, name);

	// load class labels
	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, name);
	vint8 number = labels.Size();

	// get pivot distances
	vMatrix<float> pivot_distances = LoadPivotDistances(name, classifiers);
	if (pivot_distances.valid() <= 0)
	{
		delete_pointer(pathname);
		return vMatrix<float>();
	}

	vMatrix<float> result = EmbedSet(pathname, number, number,
		pivot_distances, classifiers);
	vdelete2(pathname);
	return result;
}


vMatrix<float> BoostMap_data::embed_databaseb(const char * name,
	vMatrix<float> classifiers)
{
	// get filename of distances from training set to training set (i.e. from
	// database objects to database objects).
	char * pathname = TrainTrainDistancesPath(g_data_directory, name);

	// load class labels
	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, name);
	vint8 number = labels.Size();

	// get pivot distances
	vMatrix<float> pivot_distances = LoadPivotDistances(name, classifiers);
	vMatrix<float> result = EmbedSetb(pathname, number, number,
		pivot_distances, classifiers);
	vdelete2(pathname);
	return result;
}


// Same as embed_database, but for the test set of the dataset, 
// not for the database (or training set).
vMatrix<float> BoostMap_data::embed_test_set(const char * name,
	vMatrix<float> classifiers,
	vint8 database_size)
{
	char * pathname = TestTrainDistancesPath(g_data_directory, name);
	vMatrix<float> labels = LoadTestLabels(g_data_directory, name);
	vint8 number = labels.Size();
	vMatrix<float> pivot_distances = LoadPivotDistances(name, classifiers);
	vMatrix<float> result = EmbedSet(pathname, number, database_size,
		pivot_distances, classifiers);
	vdelete2(pathname);
	return result;
}


// Same as embed_database, but for the test set of the dataset, 
// not for the database (or training set).
vMatrix<float> BoostMap_data::embed_test_setb(const char * name,
	vMatrix<float> classifiers,
	vint8 database_size)
{
	char * pathname = RefTestDistancesPath(name);
	vMatrix<float> labels = LoadTestLabels(g_data_directory, name);
	vint8 number = labels.Size();
	vMatrix<float> pivot_distances = LoadPivotDistances(name, classifiers);
	vMatrix<float> result = EmbedSetb(pathname, number, database_size,
		pivot_distances, classifiers);
	vdelete2(pathname);
	return result;
}


// This function actually does the work for embed_database and 
// embed_test_set. "pathname" specifies where from to load the distances
// from set of objects to database. 
// "number" specifies the number of objects
// to be loaded. "database_size" specifies the size
// of the database, i.e. the number of distances to be read
// from pathname for each object. pivot_distances(i) is
// the intrapivot distance for the i-th 1D embedding, i.e.
// the i-th row of classifiers, provided that that embedding
// is a "line projection". classifiers holds, in the i-th
// row, the 1D embedding to be used to obtain the i-th
// coordinate of each object.
vMatrix<float> BoostMap_data::EmbedSet(const char * pathname, vint8 number,
	vint8 database_size,
	vMatrix<float> pivot_distances,
	vMatrix<float> classifiers)
{
	if ((pivot_distances.valid() <= 0) || (classifiers.valid() <= 0))
	{
		vPrint("Invalid arguments to EmbedSet\n");
		return vMatrix<float>();
	}

	// get reference objects (i.e. objects such that distances
	// to those objects define the embedding of any object).
	vint8_matrix references = class_BoostMap::ExtractReferenceObjects(classifiers);

	// get types of each 1D embedding.
	vint8_matrix types = class_BoostMap::ExtractTypes(classifiers);
	references.PrintInt("references");
	types.PrintInt("types");

	vint8 distance_counter = 0;
	vint8 type_size = types.Size();
	vint8 i;
	for (i = 0; i < type_size; i++)
	{
		vint8 current_type = types(i);
		if (current_type == 0)
		{
			distance_counter = distance_counter + 1;
		}
		else
		{
			distance_counter = distance_counter + 2;
		}
	}

	vPrint("upper_bound on distances: %li\n", (long) distance_counter);
	if ((references.valid() <= 0) || (types.valid() <= 0))
	{
		vPrint("Invalid references or types extracted\n");
		return vMatrix<float>();
	}

	class_file * fp = OpenObjectDistancesFile(pathname);;
	if (fp->file_pointer == 0)
	{
		vPrint("Failed to open %s\n", pathname);
		return vMatrix<float>();
	}

	vint8 rows = number;
	vint8 cols = types.Size();
	vMatrix<float> result(rows, cols);

	vPrint("\n");
	for (i = 0; i < number; i++)
	{
		// get distances from object i to all database objects.
		vMatrix<float> all_distances = NextObjectDistances(fp, database_size);
		if (all_distances.valid() <= 0)
		{
			vPrint("Failed to read distances for object %li from %s\n",
				(long) i, pathname);
			fclose(fp);
			return vMatrix<float>();
		}

		// extract distances to reference objects.
		vMatrix<float> reference_distances = 
			class_BoostMap::ReferenceDistances(all_distances, references);

		// compute the embedding of current object.
		vMatrix<float> embedding = class_BoostMap::Embedding3(reference_distances,
			pivot_distances, types);
		function_put_row(&embedding, &result, i);
		vPrint("embedded object %li of %li\r", (long) (i+1), (long) number);
	}
	vPrint("\n");

	fclose(fp);
	return result;
}


vMatrix<float> BoostMap_data::EmbedSetb(const char * pathname, vint8 number,
	vint8 database_size,
	vMatrix<float> pivot_distances,
	vMatrix<float> classifiers)
{
	if ((pivot_distances.valid() <= 0) || (classifiers.valid() <= 0))
	{
		vPrint("Invalid arguments to EmbedSet\n");
		return vMatrix<float>();
	}

	// get reference objects (i.e. objects such that distances
	// to those objects define the embedding of any object).
	vint8_matrix references = class_BoostMap::ExtractReferenceObjects(classifiers);
	vint8 reference_number = references.Size();

	// get types of each 1D embedding.
	vint8_matrix types = class_BoostMap::ExtractTypes(classifiers);
	references.PrintInt("references");
	types.PrintInt("types");
	if ((references.valid() <= 0) || (types.valid() <= 0))
	{
		vPrint("Invalid references or types extracted\n");
		return vMatrix<float>();
	}

	vint8 rows = number;
	vint8 cols = types.Size();
	vMatrix<float> result(rows, cols);

	vMatrix<float> reference_distances(number, reference_number);
	vint8 i;
	vPrint("\n");
	for (i = 0; i < reference_number; i++)
	{
		vint8 reference_index = references(i);
		if (reference_index < 0)
		{
			continue;
		}

		// get distances from the reference object to all objects.
		vMatrix<float> all_distances = ObjectDistances(pathname, reference_index);
		if (all_distances.valid() <= 0)
		{
			vPrint("Failed to read distances for reference %li from %s\n",
				(long) reference_index, pathname);
			return vMatrix<float>();
		}

		vint8 j;
		for (j = 0; j < number; j++)
		{
			reference_distances(j, i) = all_distances(j);
		}
		vPrint("read distance for reference object %li of %li\r", 
			(long) (i+1), (long) reference_number);
	}

	vPrint("\n");
	for (i = 0; i < number; i++)
	{  
		// extract distances to reference objects.
		vMatrix<float> distances(1, reference_number);
		vint8 j;
		for (j = 0; j < reference_number; j++)
		{
			distances(j) = reference_distances(i, j);
		}

		// compute the embedding of current object.
		vMatrix<float> embedding = class_BoostMap::Embedding3(distances,
			pivot_distances, types);
		function_put_row(&embedding, &result, (long) i);
		vPrint("embedded object %li of %li\r", (long) (i+1), (long) number);
	}
	vPrint("\n");
	return result;
}


// "name" is the name of an entire dataset. classifiers(0) specifies,
// in the i-th
// row, the 1D embedding to be used to obtain the i-th
// coordinate of each object. result(i) is 
// the intrapivot distance for the i-th 1D embedding, i.e.
// the i-th row of classifiers, 
vMatrix<float> BoostMap_data::LoadPivotDistances(const char * name,
	vMatrix<float> classifiers)
{
	if (classifiers.valid() <= 0)
	{
		return vMatrix<float>();
	}

	// get info about the embedding, based on classifiers.
	vint8_matrix reference_idsm = class_BoostMap::ExtractReferenceObjects(classifiers);
	if (reference_idsm.valid() <= 0)
	{
		return vMatrix<float>();
	}

	vint8_matrix typesm = class_BoostMap::ExtractTypes(classifiers);
	vArray(vint8) reference_ids = reference_idsm.Matrix();
	vArray(vint8) types = typesm.Matrix();

	vint8 number = typesm.Size();
	if (reference_idsm.Size() != 2 * number)
	{
		exit_error("error: in loadpivotdistances, %li references, %li types\n",
			(long) reference_idsm.Size(), (long) number);
	}

	vMatrix<float> resultm(1, number);
	vArray(float) result = resultm.Matrix();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		// we only need to worry about "line projection" embeddings.
		if (types[i] != 1)
		{
			result[i] = (float) -1;
			continue;
		}

		// get the two pivots
		vint8 base = 2*i;
		vint8 pivot1 = reference_ids[base];
		vint8 pivot2 = reference_ids[base+1];

		if ((pivot1 < 0) || (pivot2 < 0))
		{
			exit_error("error: in loadpivotdistances, pivots = %li, %li\n",
				(long) pivot1, (long) pivot2);
		}

		// get distances from pivot1 to entire database.
		vMatrix<float> distances = TrainTrainDistance(g_data_directory, name, pivot1);
		if (distances.valid() <= 0)
		{
			vPrint("Failed to load distances for i = %li, pivot = %li, name = %li\n",
				(long) i, (long) pivot1, (long) name);
			return vMatrix<float>();
		}

		if (distances.Size() <= pivot2)
		{
			exit_error("error: in loadpivotdistances, distances.size = %li, pivot2 = %li\n",
				(long) distances.Size(), (long) pivot2);
		}

		// get distance from pivot1 to pivot2.
		result[i] = distances(pivot2);
	}

	resultm.Print("pivot_distances");

	return resultm;
}


// loads the embedding saved in bm_name, and based on that
// it embeds the training and the test set.
vint8 BoostMap_data::save_embeddings(const char * name, const char * bm_name)
{
	// load embedding
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> detailed_classifiers = vMatrix<float>::ReadText(bm_path);
	if (detailed_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return 0;
	}
	vdelete2(bm_path);

	detailed_classifiers.Print("classifiers");
	float_matrix unique_classifiers = class_BoostMap::select_first_dimensions(detailed_classifiers, -1);

	// embed database
	vMatrix<float> embedded_database = embed_database(name, unique_classifiers);
	if (embedded_database.valid() <= 0)
	{
		vPrint("Failed to embed training set\n");
		return 0;
	}
	vint8 database_size = embedded_database.Rows();

	// save database embedding
	char * output_file = EmbeddingTrainPath(name, bm_name);
	vint8 success = embedded_database.Write(output_file);
	if (success == 0)
	{
		vPrint("Failed to save embedded database to %s\n", output_file);
		vdelete2(output_file);
		return 0;
	}
	else
	{
		function_print("saved embedding of database to %s\n", output_file);
	}

	vdelete2(output_file);

	// clear up memory occupied by the database embedding.
	embedded_database = vMatrix<float>();

	// embed test set
	vMatrix<float> embedded_test_set = embed_test_set(name, unique_classifiers,
		database_size);
	if (embedded_test_set.valid() <= 0)
	{
		vPrint("Failed to embed test set\n");
		return 0;
	}

	// save test set embedding
	// I should take this out
	//  exit_error ("this part has to be rewritten.\n");
	output_file = EmbeddingTestPath(name, bm_name);
	success = embedded_test_set.Write(output_file);
	if (success == 0)
	{
		vPrint("Failed to save embedded test set to %s\n", output_file);
		vdelete2(output_file);
		return 0;
	}
	else
	{
		function_print("saved embedding of database to %s\n", output_file);
	}
	vdelete2(output_file);

	return 1;
}


vMatrix<float> BoostMap_data::load_training_embedding(const char * name, 
	const char * bm_name)
{
	char * pathname = EmbeddingTrainPath(name, bm_name);
	vMatrix<float> embedding = vMatrix<float>::Read(pathname);
	if (embedding.valid() <= 0)
	{
		vPrint("Failed to load training embedding from %s\n", pathname);
	}
	vdelete2(pathname);

	if (embedding.vertical() != BoostMap_data::database_size(name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::database_size(name),
			(long) embedding.vertical());
		return float_matrix();
	}

	return embedding;
}


vMatrix<float> BoostMap_data::load_training_embedding(const char * name, 
	const char * bm_name,
	vint8 dimensions)
{
	exit_error("load_test_embedding: not sure if implemented correctly, after writing retrieval_results\n");
	float_matrix all_dimensions = load_training_embedding(name, bm_name);
	if (all_dimensions.valid() <= 0)
	{
		return all_dimensions;
	}

	if (all_dimensions.vertical() != BoostMap_data::database_size(name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::database_size(name),
			(long) all_dimensions.vertical());
		return float_matrix();
	}

	float_matrix junk;
	float_matrix result = class_BoostMap::choose_first_dimensions(bm_name,all_dimensions,dimensions, &junk);
	return result;
}


vMatrix<float> BoostMap_data::load_test_embedding(const char * name, 
	const char * bm_name)
{
	char * pathname = EmbeddingTestPath(name, bm_name);
	vMatrix<float> embedding = vMatrix<float>::Read(pathname);
	if (embedding.valid() <= 0)
	{
		vPrint("Failed to load test embedding from %s\n", pathname);
	}
	vdelete2(pathname);
	if (embedding.vertical() != BoostMap_data::test_size(name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::test_size(name),
			(long) embedding.vertical());
		return float_matrix();
	}

	return embedding;
}


vMatrix<float> BoostMap_data::load_test_embedding(const char * name, 
	const char * bm_name,
	vint8 dimensions)
{
	exit_error("load_test_embedding: not sure if implemented correctly, after writing retrieval_results\n");
	float_matrix all_dimensions = load_test_embedding(name, bm_name);
	if (all_dimensions.valid() <= 0)
	{
		return all_dimensions;
	}

	if (all_dimensions.vertical() != BoostMap_data::test_size(name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::test_size(name),
			(long) all_dimensions.vertical());
		return float_matrix();
	}

	float_matrix junk;
	float_matrix result = class_BoostMap::choose_first_dimensions(bm_name,all_dimensions,dimensions, &junk);
	return result;
}



// returns the filename that should be used for saving or loading
// the embedding of a test set.
char * BoostMap_data::EmbeddingTestPath(const char * dataset_name,
	const char * embedding_name)
{
	char * target_dir = embedding_directory(g_data_directory, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = vReplaceEnding2(embedding_name, ".test");
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


// returns the filename that should be used for saving or loading
// the embedding of a test set.
char * BoostMap_data::EmbeddingTrainPath(const char * dataset_name,
	const char * embedding_name)
{
	char * target_dir = embedding_directory(g_data_directory, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = vReplaceEnding2(embedding_name, ".train");
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


// in this function we assume that we have already saved
// the embedding of the database as specified by the embedding
// stored in "embedding_name", for the given number of dimensions.
// The embedding of the database is assumed to have been stored
// in a pathname as determined by function 
// EmbeddingTrainPath(dataset, embedding_name, dimensions). This
// function essentially is used to verify that class_BoostMap::L1Distances
// works correctly. We call that function to compute the distances
// from q to a and b, but we also comptue those distances
// explicitly, and we compare the results.
float BoostMap_data::TestTriple(const char * dataset,
	float_matrix all_classifiers,
	float_matrix database_embedding, 
	vint8 q, vint8 a, vint8 b, vint8 distance_p)
{
	vMatrix<float> embedded_database = database_embedding;
	vint8 dimensions = all_classifiers.vertical ();
	if (embedded_database.valid() <= 0)
	{
		vPrint("invalid database embedding passed\n");
		return 0;
	}

	vint8 rows = embedded_database.Rows();
	if ((q < 0) || (a < 0) || (b < 0) || 
		(q >= rows) || (a >= rows) || (b >= rows))
	{
		vPrint("bad inputs: q = %li, a = %li, b = %li, rows = %li\n", 
			(long) q, (long) a, (long) b, (long) rows);
	}

	// get the saved embeddings.
	v3dMatrix<float> q_saved = copy_horizontal_line(&embedded_database, q);
	v3dMatrix<float> a_saved = copy_horizontal_line(&embedded_database, a);
	v3dMatrix<float> b_saved = copy_horizontal_line(&embedded_database, b);

	if ((q_saved.Size() != dimensions) || (a_saved.Size() != dimensions) ||
		(b_saved.Size() != dimensions))
	{
		vPrint("bad dimensions\n");
		return 0;
	}

	// choose the first "dimensions" dimensions

	// get weights for L1 distance.
	vMatrix<float> weights = class_BoostMap::ExtractWeights(all_classifiers);
	weights.Print("weights");

	float margin = 0;
	vint8 i;
	vPrint("\n");
	for (i = 0; i < dimensions; i++)
	{
		// get the i-th coordinates
		float qi = q_saved.Matrix()[i];
		float ai = a_saved.Matrix()[i];
		float bi = b_saved.Matrix()[i];
		float w = weights(i);

		// get contributions made by i-th coordinates to classification
		// output for triple (q, a, b).
		float current = pow(vAbs(qi - bi), (float) distance_p) - pow(vAbs(qi - ai), (float) distance_p);
		float wcurrent = w * current;
		margin = margin + wcurrent;
		vPrint("%5li: %10.5f%10.5f%10.5f%10.5f%10.5f%10.5f\n",
			(long) i, qi, ai, bi, current, wcurrent, margin);
	}
	vPrint("\n");
	vMatrix<float> qa_temp = BoostMap_data::LpDistances(&q_saved, vMatrix<float>(&a_saved),
		weights, distance_p);
	float qa = qa_temp(0);
	vMatrix<float> qb_temp = BoostMap_data::LpDistances(&q_saved, vMatrix<float>(&b_saved),
		weights, distance_p);
	float qb = qb_temp(0);
	vPrint("qa = %f, qb = %f, qb - qa = %f\n", qa, qb, qb - qa);
	vPrint("margin = %f\n", margin);
	vPrint("\n");

	return margin;
}


// arguments are same as before, but there are two differences:
// - here we don't assume the embedding of the database has been
//   saved, we just embed q, a, and b within this function,
//   compute their distances and compare them with the results
//   of the function SL1Distances.
// - EmbedTriple handles sensitive embeddings (i.e. query-sensitive
//   embbeddings), whereas TestTriple only handles global
//   embeddings.
// Essentially, in this function we check whether Sl1Distances does
// the right thing, by comparing its results to the results that
// get computed within this function, and for which we can 
// see the step-by-step intermediate results.
float BoostMap_data::EmbedTriple(const char * dataset, 
	const char * bm_name,
	vint8 dimensions, 
	vint8 q, vint8 a, vint8 b, vint8 distance_p)
{
	// load the embedding specification (minus the query-sensitive part)
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return 0;
	}

	// chose first "dimensions" dimensions
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	if (dimensions == -1)
	{
		dimensions = classifiers.Rows();
	}
	else
	{
		dimensions = Min(dimensions, classifiers.Rows());
	}

	// load the query-sensitive part of the embedding
	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	// get the 1D embeddings in vector form.
	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	vint8_matrix references = class_BoostMap::ExtractReferenceObjects(classifiers);
	vint8_matrix types = class_BoostMap::ExtractTypes(classifiers);

	// prepare some structures useful for embedding objects on the fly.
	// distances from database objects to database objects
	char * pathname = TrainTrainDistancesPath(g_data_directory, dataset);

	// training labels.
	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, dataset);
	vint8 number = labels.Size();

	// intrapivot distances.
	vMatrix<float> pivot_distances = LoadPivotDistances(dataset, classifiers);
	if ((pivot_distances.valid() <= 0) || (classifiers.valid() <= 0))
	{
		vPrint("Invalid pivot distances\n");
		return 0;
	}

	// get distances from q, a, b to entire database
	vMatrix<float> q_distances = ObjectDistances(pathname, q);
	vMatrix<float> a_distances = ObjectDistances(pathname, a);
	vMatrix<float> b_distances = ObjectDistances(pathname, b);
	if ((q_distances.valid() <= 0) || (a_distances.valid() <= 0) ||
		(b_distances.valid() <= 0))
	{
		vPrint("Failed to read distances from %s\n", pathname);
		vdelete2(pathname);
		return 0;
	}

	// extract distances to reference objects, and based on those
	// distances, compute embeddings of q, a, b.
	vdelete2(pathname);
	vMatrix<float> q_references = 
		class_BoostMap::ReferenceDistances(q_distances, references);
	vMatrix<float> q_vector = class_BoostMap::Embedding3(q_references,
		pivot_distances, types);
	vMatrix<float> a_references = 
		class_BoostMap::ReferenceDistances(a_distances, references);
	vMatrix<float> a_vector = class_BoostMap::Embedding3(a_references,
		pivot_distances, types);
	vMatrix<float> b_references = 
		class_BoostMap::ReferenceDistances(b_distances, references);
	vMatrix<float> b_vector = class_BoostMap::Embedding3(b_references,
		pivot_distances, types);

	// get the query-specific weights that define the query-sensitive distance
	// that should be used for q.
	vMatrix<float> weights = class_BoostMap::QueryWeights(&q_vector, &unique_classifiers,
		&sensitive_classifiers);

	float margin = 0;
	vint8 i;
	vPrint("\n");
	for (i = 0; i < dimensions; i++)
	{
		// get i-th coordinate of embeddings
		float qi = q_vector.Matrix()[i];
		float ai = a_vector.Matrix()[i];
		float bi = b_vector.Matrix()[i];

		// compute contribution of i-th coordinate to classification
		// result of embedding on triple (q, a, b).
		float w = weights(i);
		float current = pow(vAbs(qi - bi), (float) distance_p) - pow(vAbs(qi - ai), (float) distance_p);
		float wcurrent = w * current;
		margin = margin + wcurrent;
		vPrint("%5li: %10.5f%10.5f%10.5f%10.5f%10.5f%10.5f\n",
			(long) i, qi, ai, bi, current, wcurrent, margin);
	}
	vPrint("\n");

	// call SL1Distances to check if we get the same results.
	vMatrix<float> qa_temp = BoostMap_data::SLpDistances(&q_vector, vMatrix<float>(&a_vector),
		&unique_classifiers, &sensitive_classifiers, 
		distance_p);
	float qa = qa_temp(0);
	vMatrix<float> qb_temp = BoostMap_data::SLpDistances(&q_vector, vMatrix<float>(&b_vector),
		&unique_classifiers, &sensitive_classifiers,
		distance_p);
	float qb = qb_temp(0);
	vPrint("qa = %f, qb = %f, qb - qa = %f\n", qa, qb, qb - qa);
	vPrint("margin = %f\n", margin);
	vPrint("\n");

	return margin;
}


// re-implementing some functions, so that they can deal with 
// sensitive embeddings.

/*!
	This is equivalent to L1Distances.

	@param objectm embedding parameters of some a query object.
	@param databaseem embedding parameters of each training object.
*/
vMatrix<float> BoostMap_data::SL1Distances(const v3dMatrix<float> * objectm, 
	const vMatrix<float> databasem,
	const vector<class_triple_classifier> * unique_classifiers,
	const vector<class_sensitive_classifier> * sensitive_classifiers)
{
	if ((objectm == 0) || (objectm->Size() == 0) || 
		(objectm->Size() != databasem.Cols()))
	{
		return vMatrix<float>();
	}

	// Get weights that specify query-sensitive distance, specific to objectm.
	// Note: the weights are query-sensitive ONLY IF the vector of
	// sensitive_classifiers is not empty. Otherwise, the weights are
	// query-INsensitive.
	vMatrix<float> weightsm = class_BoostMap::QueryWeights(objectm, 
		unique_classifiers,	sensitive_classifiers);

	if ((databasem.valid() <= 0) || (weightsm.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 rows = databasem.Rows();
	vint8 cols = weightsm.Size();

	// Create a vector (matrix) to hold the distances between each
	// taring object and the query object
	vMatrix<float> resultm(1, rows);

	// View the results matrix as a vector
	vArray(float) result = resultm.Matrix();

	// Embedding params of the query object viewed as
	// a one-dimensional matrix indexed by a single number
	vArray(float) object = objectm->Matrix();

	// View the matrix as a 2-d array from which we an
	// extract entire rows
	vArray2(float) database = databasem.Matrix2();

	// View the weights matrix as a vector
	vArray(float) weights = weightsm.Matrix();

	vint8 row, col;

	for (row = 0; row < rows; row++)
	{
		// Each "row" of the databasem represents the
		// embedding parameters of a training object.
		vArray(float) training_object = database[row];

		// We want to map each training object to a distance
		// between the trainig object a and the given object 
		// computed according to the embedding.

		// Such distance is equal to the weighted sum of absolut 
		// differences betweem object_i and training_object_i. That is,
		// d = Sum_i { weight_i * abs(object_i - training_object_i) }.

		float distance = 0;

		for (col = 0; col < cols; col++)
		{
			// This is a weighted Manhattan distance, or what we can call a
			// San Francisco distance, since "blocks" might be up or down hill
			distance += vAbs(object[col] - training_object[col]) * weights[col];
		}

		result[row] = distance;
	}

	return resultm;
}


// Equivalent of LpDistances.
vMatrix<float> BoostMap_data::SLpDistances(v3dMatrix<float> * objectm, 
	vMatrix<float> databasem,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	vint8 distance_p)
{
	if ((objectm == 0) || (objectm->Size() == 0) || 
		(objectm->Size() != databasem.Cols()))
	{
		return vMatrix<float>();
	}

	// get weights that specify query-sensitive distance, specific to objectm.
	vMatrix<float> weightsm = class_BoostMap::QueryWeights(objectm, unique_classifiers,
		sensitive_classifiers);

	if ((databasem.valid() <= 0) || (weightsm.valid() <= 0) ||
		(databasem.Cols() != weightsm.Size()))
	{
		return vMatrix<float>();
	}

	vint8 rows = databasem.Rows();
	vint8 cols = databasem.Cols();

	vMatrix<float> resultm(1, rows);
	vArray(float) result = resultm.Matrix();
	vArray(float) object = objectm->Matrix();
	vArray2(float) database = databasem.Matrix2();
	vArray(float) weights = weightsm.Matrix();

	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		vArray(float) training_object = database[row];
		float distance = 0;
		for (col = 0; col < cols; col++)
		{
			distance += pow(vAbs(object[col] - training_object[col]), (float) distance_p) * weights[col];
		}
		result[row] = distance;
	}

	return resultm;
}



// Equivalent of KnnEmbeddingError
vMatrix<float> BoostMap_data::KnnSembeddingError(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (training_labels.valid() <= 0))
	{
		vPrint("Invalid labels distances\n");
		return vMatrix<float>();
	}

	// embed training and test sets.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	vint8 database_size = embedded_database.Rows();

	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers, database_size);

	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	vMatrix<float> result = KnnError8(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		test_labels, training_labels,
		max_k, 0);

	return result;
}


// Equivalent of KnnEmbeddingTrainError
vMatrix<float> BoostMap_data::KnnSembeddingTrainError(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if (training_labels.valid() <= 0)
	{
		vPrint("Invalid labels distances\n");
		return vMatrix<float>();
	}

	vMatrix<float> embedded_database = embed_database(name, classifiers);
	if (embedded_database.valid() <= 0)
	{
		vPrint("Failed to embed training set\n");
		return vMatrix<float>();
	}

	vMatrix<float> result = KnnError8(embedded_database, embedded_database,
		&unique_classifiers, &sensitive_classifiers, 
		training_labels, training_labels, 
		max_k, 1);

	return result;
}


// Equivalent of KnnError7.
vMatrix<float> BoostMap_data::KnnError8(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	vMatrix<float> test_labels,
	vMatrix<float> train_labels,
	vint8 max_k, vint8 training_flag)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0) || (max_k <= 0))
	{
		return vMatrix<float>();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		float label = test_labels(i);
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		vMatrix<float> distances = SL1Distances(&object, training_set, 
			unique_classifiers, sensitive_classifiers);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}
		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		vint8 j;
		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	result = result / (float) test_number;
	return result;
}


// we measure the classification error for k = 1,...,max_k that we 
// get by applying the embedding saved in bm_name, with given dimensions,
// keeping the top to_keep matches, and then sorting those matches
// based on the exact distances.
vMatrix<float> BoostMap_data::FilterRefineErrors5(const char * name, 
	const char * bm_name,
	vint8 dimensions, vint8 to_keep,
	vint8 max_k)
{
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (training_labels.valid() <= 0))
	{
		vPrint("Invalid labels distances\n");
		return vMatrix<float>();
	}

	// embed training and test sets.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	vint8 database_size = embedded_database.Rows();

	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers, database_size);

	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	char * distances_file = TestTrainDistancesPath(g_data_directory, name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vMatrix<float>();
	}

	// next lines: hack to use only boostmap training set as database.
	//  vint8 i, j;
	// for (i = 0; i < 5000; i++)
	//  {
	//    for (j = 0; j < embedded_database.Cols(); j++)
	//    {
	//      embedded_database(i,j) = 1000;
	//    }
	//  }

	//  for (i = 10000; i < 20000; i++)
	//  {
	//    for (j = 0; j < embedded_database.Cols(); j++)
	//    {
	//      embedded_database(i,j) = 1000;
	//    }
	//  }


	vMatrix<float> result = FilterRefineErrors10(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		test_labels, training_labels,
		max_k, 0, to_keep, fp);

	fclose(fp);
	vdelete2(distances_file);

	dimensions = classifiers.Rows();
	char * filename = FilterRefineOutputPath(name, bm_name, dimensions, to_keep);
	if (filename != 0)
	{
		vint8 save_success = result.WriteText(filename);
		if (save_success <= 0)
		{
			vPrint("failed to save result to %s\n", filename);
		}
		else
		{
			vPrint("saved result to %s\n", filename);
		}
		vdelete2(filename);
	}

	return result;
}





vMatrix<float> BoostMap_data::FilterRefineErrors10(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	vMatrix<float> test_labels,
	vMatrix<float> train_labels,
	vint8 max_k, vint8 training_flag, 
	vint8 to_keep, class_file * fp)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0) || (max_k <= 0))
	{
		return vMatrix<float>();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	// this is a temporary addition, to keep track of "bad" objects, i.e.
	// objects that get misclassified for k = 1, ..., 5
	vint8 bad_number = Min((vint8) 5, max_k);
	vector<vector<vint8> > bad_objects((vector_size) bad_number + 1);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		float label = test_labels(i);
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		vMatrix<float> distances = SL1Distances(&object, training_set, 
			unique_classifiers, sensitive_classifiers);
		vMatrix<float> original = NextObjectDistances(fp, distances.Size());

		float max_distance1 = vAbs(function_image_maximum(&distances));
		float max_distance2 = vAbs(function_image_maximum(&original));
		float limit = Max(max_distance1, max_distance2) * ((float) 2.0) + (float) 1.0;
		//    if (training_flag == 1)
		{
			distances(i) = limit;
		}

		vint8 junk = 0;
		// we add 1 to to_keep, so that then we check if a distance is 
		// strictly smaller than that. This way, if there are ties we 
		// keep fewer than to_keep, and if there are no ties we
		// keep exactly to_keep objects.
		float threshold_distance = kth_smallest_cb(to_keep+1, &distances,
			&junk);

		vint8 j;
		for (j = 0; j < distances.Size(); j++)
		{ 
			if (distances(j) < threshold_distance)
			{
				distances(j) = original(j);
			}
			else
			{
				distances(j) = distances(j) + limit;
			}
		}

		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}
		result = result + labels;
		for (j = 1; j <= bad_number; j++)
		{
			if (labels(j) == 1)
			{
				bad_objects[(vector_size) j].push_back(i);
			}
		}
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) i+1, (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	result = result / (float) test_number;

	// print bad objects.
	//vint8 j;
	//for (j = 1; j <= bad_number; j++)
	//{
	//  vPrint("k = %li, bad objects:\n", j);
	//  vint8 size = bad_objects[(vector_size) j].size();
	//  vint8 k; 
	//  for (k = 0; k < size; k++)
	//  {
	//    vPrint("%li\n", bad_objects[(vector_size) j][(vector_size) k]);
	//  }
	//}


	return result;
}


// uses L2 distance in embedded space, 
vMatrix<float> BoostMap_data::FilterRefineL2(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vMatrix<float> weights, 
	vMatrix<float> test_labels, 
	vMatrix<float> train_labels,
	vint8 max_k, vint8 training_flag, 
	vint8 to_keep, class_file * fp)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0) || (max_k <= 0))
	{
		return vMatrix<float>();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(1, max_k+1);
	function_enter_value(&result, (float) 0);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		float label = test_labels(i);
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		vMatrix<float> distances = L2Distances(&object, training_set, 
			weights);
		vMatrix<float> original = NextObjectDistances(fp, distances.Size());

		float max_distance1 = vAbs(function_image_maximum(&distances));
		float max_distance2 = vAbs(function_image_maximum(&original));
		float limit = Max(max_distance1, max_distance2) * ((float) 2.0) + (float) 1.0;
		//    if (training_flag == 1)
		{
			distances(i) = limit;
		}

		vint8 junk = 0;
		// we add 1 to to_keep, so that then we check if a distance is 
		// strictly smaller than that. This way, if there are ties we 
		// keep fewer than to_keep, and if there are no ties we
		// keep exactly to_keep objects.
		float threshold_distance = kth_smallest_cb(to_keep+1, &distances,
			&junk);

		vint8 j;
		for (j = 0; j < distances.Size(); j++)
		{ 
			if (distances(j) < threshold_distance)
			{
				distances(j) = original(j);
			}
			else
			{
				distances(j) = distances(j) + limit;
			}
		}

		vMatrix<float> labels = KnnLabel4(distances, train_labels, (vint8) label, max_k);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		for (j = 0; j < max_k+1; j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				labels(j) = (float) 0;
			}
			else
			{
				labels(j) = (float) 1;
			}
		}
		result = result + labels;
		vPrint("processed object %li of %li, 1-nn error = %f\r", 
			(long) (i+1), (long) test_number, result(1) / (float) (i+1));
	}
	vPrint("\n");

	result = result / (float) test_number;
	return result;
}


// index error (as opposed to classification error) is the following:
// given anobject q, and its true nearest neighbor a in the 
// training set, what is the rank of a as a neighbor of q, based
// on the embedding stored in "bm_name" and using the given 
// number of dimensions? Note that here we only look at 
// the true 1-nearest neighbor. 
// "name" is the name of an entire dataset, and "bm_name" stores
// an embedding.
// result(i) is the index error for the i-th test object of the dataset.
// This function does handle query-sensitive embeddings.
vMatrix<float> BoostMap_data::IndexError3(const char * name, const char * bm_name,
	vint8 dimensions)
{
	// The next two lines are just in case there is a bug, to catch it early
	char * trash = IndexErrorOutputPath(name, bm_name, dimensions);
	vdelete2(trash);

	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// load query-sensitive part of embedding.
	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	// get 1D embeddings in vector form.
	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	// embed training and test sets.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	vint8 database_size = embedded_database.Rows();

	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers, database_size);

	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	char * distances_file = TestTrainDistancesPath(g_data_directory, name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vMatrix<float>();
	}

	// get the actual errors.
	vMatrix<float> result = IndexError6(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		fp, 0);
	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = IndexErrorOutputPath(name, bm_name, dimensions);
	vint8 success = result.Write(output_file);
	if (success <= 0)
	{
		vPrint("failed to write result to %s\n", output_file);
	}
	else
	{
		vPrint("saved result to %s\n", output_file);
	}

	vdelete2(output_file);
	return result;
}



// here test_set and training_set are embeddings of two sets of
// some dataset (possibly test set is actually the training set
// of the dataset, if we want to measure training index error).
// unique_classifiers define the embedding, and sensitive_classifiers
// together with unique_classifiers define the query-sensitive distances
// to be used. test_fp specifies an open file from which we can get,
// for each object in test_set, its distances to all database objects
// (i.e. objects in training_set). These distances are needed in order
// to figure out, for each object, its true nearest neighbor.
// training_flag is 1 iff test_set and training_set are identical, so
// that we do not count the object itself as its nearest neighbor.
// result(i, 0) is the index error for object i (row i of test set).
// The other columns of result hold some info I was curious about.
// This function does handle query-sensitive embeddings.
vMatrix<float> BoostMap_data::IndexError6(vMatrix<float> test_set, 
	vMatrix<float> training_set,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	class_file * test_fp, vint8 training_flag)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_fp->file_pointer == 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(test_number, 8);

	// some variables for interim progress report
	float max_value = 0;
	float sum = 0;
	float mean;
	vint8 original_ties = 0;
	vint8 embedding_ties = 0;

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		// get embedding of object.
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);

		// get original distances from objet to database.
		vMatrix<float> original_distances = NextObjectDistances(test_fp, training_number);
		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) i);
		}

		// get query-sensitive distances of object to database.
		vMatrix<float> distances = SL1Distances(&object, training_set, 
			unique_classifiers, sensitive_classifiers);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// Find the nearest neighbor using original distance measure.
		vint8 junk = 0, nn_index = 0;
		vector<vint8> nn_indices;
		float min_distance = function_image_minimum3(&original_distances, &junk, &nn_index);
		//    float min_distance = kth_smallest_cb(50, &original_distances, &nn_index);

		// Find ties for nearest neighbor
		vint8 j;
		for (j = 0; j < training_number; j++)
		{
			if (original_distances(j) <= min_distance)
			{
				nn_indices.push_back(j);
			}
		}
		if (nn_indices.size() == 0)
		{
			exit_error("\nerror: indexerror7, impossible\n");
		}
		else if (nn_indices.size() > 1)
		{
			original_ties++;
		}

		// find objects closer than nearest neighbor in embedding space.
		float embedded_distance = distances(nn_index);
		float counter = (float) 0.0;
		float tie_counter = (float) 0.0;
		for (j = 0; j < training_number; j++)
		{
			if (j == nn_index)
			{
				continue;
			}
			if (distances(j) < embedded_distance)
			{
				counter = counter + (float) 1;
			}
			else if (distances(j) == embedded_distance)
			{
				tie_counter = tie_counter + (float) 1;
			}
		}

		vint8 embedding_nn = 0;
		float min_embedded = function_image_minimum3(&distances, &junk, &embedding_nn);

		// we add one to the rank, so that 1 is the best rank.
		float rank = counter + tie_counter / (((float) 2.0) + (float) 1.0);
		result(i, 0) = i + (float) 1.0;
		result(i, 1) = rank;
		result(i, 2) = (float) nn_index;
		result(i, 3) = embedded_distance;
		result(i, 4) = min_embedded;
		result(i, 5) = (float) embedding_nn;
		result(i, 6) = (float) nn_indices.size();
		result(i, 7) = tie_counter;

		max_value = Max<float>(max_value, rank);    
		sum = sum + rank;
		mean = sum / (float) (i+1);

		vPrint("object %li of %li, max_value = %8li, mean = %f\r", 
			(long) (i+1), (long) test_number, (long) round_number(max_value), mean);
	}
	vPrint("\n");

	return result;
}


// mostly similar to IndexError3, but
// here we return a test_number x max_k matrix, telling us, for each test
// object, the worst rank among the top k neighbors, for k = 1, ..., max_k.
// The worst rank is the following: we find the nearest k neighbors
// using original distance measure. We then find the rank of each
// of those neighbors using embedding (using query-sensitive 
// distance measure). The worst rank is the worst among those ranks.
vMatrix<float> BoostMap_data::IndexErrors4(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	// The next two lines are just in case there is a bug, to catch it early
	char * trash = IndexErrorOutputPath2(name, bm_name, dimensions);
	vdelete2(trash);

	// load embedding. First, load query-insensitive part.
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	// choose first "dimensions" dimensions of query-insensitive part.
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);
	dimensions = classifiers.Rows();

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// load query-sensitive part of the embedding.
	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	// get 1D embeddings in vector form.
	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	// embed database and test set.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	if (embedded_database.valid() <= 0)
	{
		return vMatrix<float>();
	}

	vint8 database_size = embedded_database.Rows();
	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers,
		database_size);
	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	// prepare file holding distances from test objects
	// to database.
	char * distances_file = TestTrainDistancesPath(g_data_directory, name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vMatrix<float>();
	}

	// compute the result.
	vMatrix<float> result = IndexErrors7(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		fp, 0, max_k);
	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = IndexErrorOutputPath2(name, bm_name, dimensions);
	if (output_file != 0)
	{
		vint8_matrix converted(& result);
		vint8 success = converted.Write(output_file);
		if (success <= 0)
		{
			vPrint("failed to write result to %s\n", output_file);
		}
		else
		{
			vPrint("saved result to %s\n", output_file);
		}
	}

	vdelete2(output_file);
	return result;
}


vMatrix<float> BoostMap_data::index_errors_distribution(const char * name, const char * bm_name,
	vint8 dimensions, vint8 max_k,
	vint8 label, vint8 distribution)
{
	float_matrix test_labels = LoadTestLabels(g_data_directory, name);
	if (test_labels.valid() == 0)
	{
		return vMatrix<float>();
	}

	// The next two lines are just in case there is a bug, to catch it early
	char * trash = IndexErrorOutputPath2(name, bm_name, dimensions);
	vdelete2(trash);

	// load embedding. First, load query-insensitive part.
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	// choose first "dimensions" dimensions of query-sensitive part.
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);
	dimensions = classifiers.Rows();

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// load query-sensitive part of the embedding.
	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	// get 1D embeddings in vector form.
	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	// embed database and test set.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	vint8 database_size = embedded_database.Rows();
	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers,
		database_size);
	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	// prepare file holding distances from test objects
	// to database.
	char * distances_file = TestTrainDistancesPath(g_data_directory, name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vMatrix<float>();
	}

	// compute the result.
	vMatrix<float> result = IndexErrors7_distribution(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		fp, 0, max_k, test_labels, label, distribution);
	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = IndexErrorOutputPath2(name, bm_name, dimensions);
	if (output_file != 0)
	{
		vint8_matrix converted(& result);
		vint8 success = converted.Write(output_file);
		if (success <= 0)
		{
			vPrint("failed to write result to %s\n", output_file);
		}
		else
		{
			vPrint("saved result to %s\n", output_file);
		}
	}

	vdelete2(output_file);
	return result;
}


// made for the protein dataset
vMatrix<float> BoostMap_data::IndexErrors4b(const char * name, 
	const char * bm_name,
	vint8 dimensions,
	vint8 max_k)
{
	// The next two lines are just in case there is a bug, to catch it early
	char * trash = IndexErrorOutputPath2(name, bm_name, dimensions);
	vdelete2(trash);

	// load embedding. First, load query-insensitive part.
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vMatrix<float>();
	}

	// choose first "dimensions" dimensions of query-sensitive part.
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);
	dimensions = classifiers.Cols();

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// load query-insensitive part of the embedding.
	bm_path = class_BoostMap::sensitive_pathname(bm_name);
	vMatrix<float> s_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load sensitive classifiers from %s\n", bm_path);
		vdelete2(bm_path);
	}

	else
	{
		s_classifiers.Print("sensitive classifiers");
	}
	vdelete2(bm_path);

	// get 1D embeddings in vector form.
	vector<class_triple_classifier> unique_classifiers;
	vector<class_sensitive_classifier> sensitive_classifiers;
	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(s_classifiers, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	// embed database and test set.
	vMatrix<float> embedded_database = embed_databaseb(name, classifiers);
	vint8 database_size = embedded_database.Rows();
	vMatrix<float> embedded_test_set = embed_test_setb(name, classifiers,
		database_size);
	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vMatrix<float>();
	}

	// prepare file holding distances from test objects
	// to database.
	char * distances_file = TestTrainDistancesPath(g_data_directory, name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vMatrix<float>();
	}

	// compute the result.
	vMatrix<float> result = IndexErrors7(embedded_test_set, embedded_database,
		&unique_classifiers, &sensitive_classifiers,
		fp, 0, max_k);
	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = IndexErrorOutputPath2(name, bm_name, dimensions);
	vint8 success = result.Write(output_file);
	if (success <= 0)
	{
		vPrint("failed to write result to %s\n", output_file);
	}
	else
	{
		vPrint("saved result to %s\n", output_file);
	}

	vdelete2(output_file);
	return result;
}


// mostly similar to IndexError6, but here we return
// a test_number x max_k matrix, telling us, for each test
// object, the worst rank among the top k neighbors, for k = 1, ..., max_k.
vMatrix<float> BoostMap_data::IndexErrors7(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	class_file * test_fp, vint8 training_flag, vint8 max_k)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_fp->file_pointer == 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(test_number, max_k+1);
	vint8 i;
	float_matrix rank_results(max_k,2);
	for (i = 0; i < max_k; i++)
	{
		rank_results(i, 0) = (float) (i+1);
		rank_results(i, 1) = 0.0f;
	}

	double global_rank_counter = 0.0f;
	double global_counter = 0.0f;

	// some variables for interim progress report
	float max_value1 = 0;
	float max_valuek = 0;
	float sum1 = 0;
	float sumk = 0;
	float mean1;
	float meank;
	vint8 original_ties = 0;
	vint8 embedding_ties = 0;

	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		// load distances from object to database, based on original distance
		// measure.
		vMatrix<float> original_distances = NextObjectDistances(test_fp, training_number);
		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) i);
		}

		// get embedding-based distances from object to database.
		vMatrix<float> distances = SL1Distances(&object, training_set, 
			unique_classifiers, 
			sensitive_classifiers);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// Find the max_k nearest neighbors (based on original 
		// distance measure).
		vint8 junk = 0, nn_index = 0;
		vector<vint8> nn_indices((vector_size) max_k, -1);
		vector<float> original_vector;
		vector<vint8> index_ranks;
		vector_from_matrix(&original_distances, &original_vector);
		vIndexRanks(&original_vector, &index_ranks);

		vint8 j;
		for (j = 0; j < training_number; j++)
		{
			if (index_ranks[(vector_size) j] < max_k)
			{
				vint8 rank = index_ranks[(vector_size) j];
				nn_indices[(vector_size) rank] = j;
			}
		}

		// count the rank (in the embedding space) of the k-nearest neighbor
		// of the query.
		vector<float> rank_counters((vector_size) max_k);

		// count filter parameter that is needed to give correct result 
		// for each k. Given a query and k, the correct filter parameter is the
		// maximum rank (in the embedding target space) among ALL k-nearest
		// neighbors of the query from the original space.
		vector<float> max_counters((vector_size) max_k);

		// distance, in embedding, between query and its k-th nearest neighbor
		vector<float> embedded_distances((vector_size) max_k);

		// max distance, in embedding, between query and any of its k-nearest neighbors
		vector<float> max_distances((vector_size) max_k);

		// get embedding-based distances for max_k nearest neighbors.
		for (j = 0; j < max_k; j++)
		{
			vint8 index = nn_indices[(vector_size) j];
			if (index == -1)
			{
				exit_error("error: index = -1\n");
			}
			float distance = distances(index);
			embedded_distances[(vector_size) j] = distance;
			rank_counters[(vector_size) j] = 0;
			max_counters[(vector_size) j] = 0;
		}

		// for each k in 1, ..., max_k, find the 
		// largest embedding-based distance attained
		// by one of the k nearest neighbors.
		max_distances[0] = embedded_distances[0];
		for (j = 1; j < max_k; j++)
		{
			float previous = max_distances[(vector_size) j-1];
			float current = embedded_distances[(vector_size) j];
			max_distances[(vector_size) j] = Max(previous, current);
		}

		// now, using the embedding distances, find, for each k,
		// the worst rank among the k nearest neighbor of the object,
		// when ranks are computed based on the embedding.
		for (j = 0; j < training_number; j++)
		{
			float distance = distances(j);
			vint8 k;
			for (k = 0; k < max_k; k++)
			{
				if (distance < max_distances[(vector_size) k])
				{
					max_counters[(vector_size) k] = max_counters[(vector_size) k] + 1;
				}
			}

			if (index_ranks[(vector_size) j] < max_k)
			{
				continue;
			}

			for (k = 0; k < max_k; k++)
			{
				global_counter = global_counter + 1.0f;
				if (distance < embedded_distances[(vector_size) k])
				{
					rank_counters[(vector_size) k] = rank_counters[(vector_size) k] + 1;
					global_rank_counter = global_rank_counter + 1.0f;
				}
			}
		}

		// print some temporary information
		max_value1 = Max<float>(max_value1, max_counters[0]);    
		sum1 = sum1 + max_counters[0];
		mean1 = sum1 / (float) (i+1);

		max_valuek = Max<float>(max_valuek, max_counters[(vector_size) max_k - 1]);    
		sumk = sumk + max_counters[(vector_size) max_k - 1];
		meank = sumk / (float) (i+1);

		result(i, 0) = (float) i;
		for (j = 0; j < max_k; j++)
		{
			result(i, j+1) = max_counters[(vector_size) j];
			rank_results(j, 1) = rank_results(j, 1) + rank_counters[(vector_size) j];
		}

		vPrint("%li of %li, max:%8li-%8li, mean:%10.2f-%10.2f, now:%8li-%8li\r", 
			(long) (i+1), (long) test_number, (long) round_number(max_value1), (long) round_number(max_valuek), 
			mean1, meank, (long) round_number(max_counters[0]), 
			(long) round_number(max_counters[(vector_size) max_k - 1]));
	}
	vPrint("\n");

	for (i = 0; i < max_k; i++)
	{
		rank_results(i, 1) = rank_results(i, 1) / (float) training_number;
	}

	global_rank_counter = global_rank_counter / global_counter;

	rank_results.print("rank_results", "%10.1f ");
	function_print("average rank for k = 1, ..., %li: %lf\n", (long) max_k, global_rank_counter);

	return result;
}


vMatrix<float> BoostMap_data::IndexErrors7_distribution(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers,
	class_file * test_fp, vint8 training_flag, vint8 max_k,
	float_matrix test_labels, vint8 label, vint8 distribution)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_fp->file_pointer == 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vint8 label_counter = 0;
	vint8 counter;
	for (counter = 0; counter < test_number; counter++)
	{
		vint8 current_label = round_number(test_labels(counter));
		if (current_label == label)
		{
			label_counter++;
		}
	}

	vMatrix<float> result(test_number + label_counter * (distribution - 1), max_k+1);
	vint8 i;
	float_matrix rank_results(max_k,2);
	for (i = 0; i < max_k; i++)
	{
		rank_results(i, 0) = (float) (i+1);
		rank_results(i, 1) = 0.0f;
	}

	double global_rank_counter = 0.0f;
	double global_counter = 0.0f;

	// some variables for interim progress report
	float max_value1 = 0;
	float max_valuek = 0;
	float sum1 = 0;
	float sumk = 0;
	float mean1;
	float meank;
	vint8 original_ties = 0;
	vint8 embedding_ties = 0;

	vPrint("\n");
	vint8 result_index = 0;
	for (i = 0; i < test_number; i++)
	{
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		// load distances from object to database, based on original distance
		// measure.
		vMatrix<float> original_distances = NextObjectDistances(test_fp, training_number);
		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) i);
		}

		// get embedding-based distances from object to database.
		vMatrix<float> distances = SL1Distances(&object, training_set, 
			unique_classifiers, 
			sensitive_classifiers);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// Find the max_k nearest neighbors (based on original 
		// distance measure).
		vint8 junk = 0, nn_index = 0;
		vector<vint8> nn_indices((vector_size) max_k, -1);
		vector<float> original_vector;
		vector<vint8> index_ranks;
		vector_from_matrix(&original_distances, &original_vector);
		vIndexRanks(&original_vector, &index_ranks);

		vint8 j;
		for (j = 0; j < training_number; j++)
		{
			if (index_ranks[(vector_size) j] < max_k)
			{
				vint8 rank = index_ranks[(vector_size) j];
				nn_indices[(vector_size) rank] = j;
			}
		}

		// count the rank (in the embedding space) of the k-nearest neighbor
		// of the query.
		vector<float> rank_counters((vector_size) max_k);

		// count filter parameter that is needed to give correct result 
		// for each k. Given a query and k, the correct filter parameter is the
		// maximum rank (in the embedding target space) among ALL k-nearest
		// neighbors of the query from the original space.
		vector<float> max_counters((vector_size) max_k);

		// distance, in embedding, between query and its k-th nearest neighbor
		vector<float> embedded_distances((vector_size) max_k);

		// max distance, in embedding, between query and any of its k-nearest neighbors
		vector<float> max_distances((vector_size) max_k);

		// get embedding-based distances for max_k nearest neighbors.
		for (j = 0; j < max_k; j++)
		{
			vint8 index = nn_indices[(vector_size) j];
			if (index == -1)
			{
				exit_error("error: index = -1\n");
			}
			float distance = distances(index);
			embedded_distances[(vector_size) j] = distance;
			rank_counters[(vector_size) j] = 0;
			max_counters[(vector_size) j] = 0;
		}

		// for each k in 1, ..., max_k, find the 
		// largest embedding-based distance attained
		// by one of the k nearest neighbors.
		max_distances[0] = embedded_distances[0];
		for (j = 1; j < max_k; j++)
		{
			float previous = max_distances[(vector_size) j-1];
			float current = embedded_distances[(vector_size) j];
			max_distances[(vector_size) j] = Max(previous, current);
		}

		// now, using the embedding distances, find, for each k,
		// the worst rank among the k nearest neighbor of the object,
		// when ranks are computed based on the embedding.
		for (j = 0; j < training_number; j++)
		{
			float distance = distances(j);
			vint8 k;
			for (k = 0; k < max_k; k++)
			{
				if (distance < max_distances[(vector_size) k])
				{
					max_counters[(vector_size) k] = max_counters[(vector_size) k] + 1;
				}
			}

			if (index_ranks[(vector_size) j] < max_k)
			{
				continue;
			}

			for (k = 0; k < max_k; k++)
			{
				global_counter = global_counter + 1.0f;
				if (distance < embedded_distances[(vector_size) k])
				{
					rank_counters[(vector_size) k] = rank_counters[(vector_size) k] + 1;
					global_rank_counter = global_rank_counter + 1.0f;
				}
			}
		}

		vint8 repetitions = 1;
		if (test_labels(i) == label)
		{
			repetitions = distribution;
		}

		vint8 repetition;
		for (repetition = 0; repetition < repetitions; repetition++)
		{
			result(result_index, 0) = (float) i;
			for (j = 0; j < max_k; j++)
			{
				result(result_index, j+1) = max_counters[(vector_size) j];
				rank_results(j, 1) = rank_results(j, 1) + rank_counters[(vector_size) j];
			}
			max_value1 = Max<float>(max_value1, max_counters[0]);    
			sum1 = sum1 + max_counters[0];
			mean1 = sum1 / (float) (result_index+1);

			max_valuek = Max<float>(max_valuek, max_counters[(vector_size) max_k - 1]);    
			sumk = sumk + max_counters[(vector_size) max_k - 1];
			meank = sumk / (float) (result_index+1);

			result_index++;
		}

		// print some temporary information
		vPrint("%li of %li, max:%8li-%8li, mean:%10.2f-%10.2f, now:%8li-%8li\r", 
			(long) (i+1), (long) test_number, (long) round_number(max_value1), (long) round_number(max_valuek), 
			mean1, meank, (long) round_number(max_counters[0]), 
			(long) round_number(max_counters[(vector_size) max_k - 1]));
	}
	vPrint("\n");

	for (i = 0; i < max_k; i++)
	{
		rank_results(i, 1) = rank_results(i, 1) / (float) (training_number * test_number / result_index);
	}

	global_rank_counter = global_rank_counter / global_counter;

	rank_results.print("rank_results", "%10.1f ");
	function_print("average rank for k = 1, ..., %li: %lf\n", (long) max_k, global_rank_counter);

	return result;
}


vint8 BoostMap_data::distance_bound(const char * bm_name, vint8 dimensions)
{
	// load embedding. First, load query-insensitive part.
	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return -1;
	}

	vdelete2(bm_path);

	// choose first "dimensions" dimensions of query-sensitive part.
	vMatrix<float> classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);
	dimensions = classifiers.Rows();

	// get types of each 1D embedding.
	vint8_matrix types = class_BoostMap::ExtractTypes(classifiers);
	types.PrintInt("types");

	vint8 distance_counter = 0;
	vint8 type_size = types.Size();
	vint8 i;
	for (i = 0; i < type_size; i++)
	{
		vint8 current_type = types(i);
		if (current_type == 0)
		{
			distance_counter = distance_counter + 1;
		}
		else
		{
			distance_counter = distance_counter + 2;
		}
	}

	vPrint("upper_bound on distances: %li\n", (long) distance_counter);
	return distance_counter;
}

// this function is useful if the embeddings have already been computed
// (for example for FastMap and MetricMap embeddings), and we do not
// use a query-sensitive distance measure. Otherwise, result is
// similar to that of IndexErrors7.
vMatrix<float> BoostMap_data::IndexErrors6(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vMatrix<float> weights,
	class_file * test_fp, vint8 training_flag, vint8 max_k)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_fp->file_pointer == 0) || (weights.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(test_number, max_k+1);

	// some variables for interim progress report
	float max_value = 0;
	float sum = 0;
	float mean;
	vint8 original_ties = 0;
	vint8 embedding_ties = 0;

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		// get object embedding
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);

		// get original distances from object to database
		vMatrix<float> original_distances = NextObjectDistances(test_fp, training_number);
		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) i);
		}

		// get embedding-based distances from object to database.
		vMatrix<float> distances = L1Distances(&object, training_set, weights);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// Find the max-k nearest neighbors
		vint8 junk = 0, nn_index = 0;
		vector<vint8> nn_indices((vector_size) max_k, -1);
		vector<float> original_vector;
		vector<vint8> index_ranks;
		vector_from_matrix(&original_distances, &original_vector);
		vIndexRanks(&original_vector, &index_ranks);

		vint8 j;
		for (j = 0; j < training_number; j++)
		{
			if (index_ranks[(vector_size) j] < max_k)
			{
				vint8 rank = index_ranks[(vector_size) j];
				nn_indices[(vector_size) rank] = j;
			}
		}

		vector<float> counters((vector_size) max_k);
		vector<float> embedded_distances((vector_size) max_k);
		vector<float> max_distances((vector_size) max_k);

		for (j = 0; j < max_k; j++)
		{
			vint8 index = nn_indices[(vector_size) j];
			if (index == -1)
			{
				exit_error("error: index = -1\n");
			}
			float distance = distances(index);
			embedded_distances[(vector_size) j] = distance;
			counters[(vector_size) j] = 0;
		}

		max_distances[0] = embedded_distances[0];
		for (j = 1; j < max_k; j++)
		{
			float previous = max_distances[(vector_size) j-1];
			float current = embedded_distances[(vector_size) j];
			max_distances[(vector_size) j] = Max(previous, current);
		}

		// get worst ranks for each k.
		for (j = 0; j < training_number; j++)
		{
			float distance = distances(j);
			vint8 k;
			for (k = 0; k < max_k; k++)
			{
				if (distance < max_distances[(vector_size) k])
				{
					counters[(vector_size) k] = counters[(vector_size) k] + 1;
				}
			}
		}

		max_value = Max<float>(max_value, counters[(vector_size) max_k - 1]);    
		sum = sum + counters[(vector_size) max_k - 1];
		mean = sum / (float) (i+1);

		result(i, 0) = (float) i;
		for (j = 0; j < max_k; j++)
		{
			result(i, j+1) = counters[(vector_size) j];
		}

		vPrint("object %li of %li, max_value = %8li, mean = %f\r", 
			(long) (i+1), (long) test_number, (long) round_number(max_value), mean);
	}
	vPrint("\n");


	return result;
}


// this function is useful if the embeddings have already been computed
// (for example for FastMap and MetricMap embeddings) and we don't
// use a query-sensitive distance measure. Here distances are 
// weighted Euclidean distances.
vMatrix<float> BoostMap_data::IndexErrors6L2(vMatrix<float> test_set,
	vMatrix<float> training_set,
	vMatrix<float> weights,
	class_file * test_fp, vint8 training_flag, vint8 max_k)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_fp == 0) || (weights.valid() <= 0))
	{
		return vMatrix<float>();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vMatrix<float> result(test_number, max_k+1);

	// some variables for interim progress report
	float max_value = 0;
	float sum = 0;
	float mean;
	vint8 original_ties = 0;
	vint8 embedding_ties = 0;

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);
		vMatrix<float> original_distances = NextObjectDistances(test_fp, training_number);
		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) i);
		}

		vMatrix<float> distances = L2Distances(&object, training_set, weights);
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// Find the nearest neighbor
		vint8 junk = 0, nn_index = 0;
		vector<vint8> nn_indices((vector_size) max_k, -1);
		vector<float> original_vector;
		vector<vint8> index_ranks;
		vector_from_matrix(&original_distances, &original_vector);
		vIndexRanks(&original_vector, &index_ranks);

		vint8 j;
		for (j = 0; j < training_number; j++)
		{
			if (index_ranks[(vector_size) j] < max_k)
			{
				vint8 rank = index_ranks[(vector_size) j];
				nn_indices[(vector_size) rank] = j;
			}
		}

		vector<float> counters((vector_size) max_k);
		vector<float> embedded_distances((vector_size) max_k);
		vector<float> max_distances((vector_size) max_k);

		for (j = 0; j < max_k; j++)
		{
			vint8 index = nn_indices[(vector_size) j];
			if (index == -1)
			{
				exit_error("error: index = -1\n");
			}
			float distance = distances(index);
			embedded_distances[(vector_size) j] = distance;
			counters[(vector_size) j] = 0;
		}

		max_distances[0] = embedded_distances[0];
		for (j = 1; j < max_k; j++)
		{
			float previous = max_distances[(vector_size) j-1];
			float current = embedded_distances[(vector_size) j];
			max_distances[(vector_size) j] = Max(previous, current);
		}

		for (j = 0; j < training_number; j++)
		{
			float distance = distances(j);
			vint8 k;
			for (k = 0; k < max_k; k++)
			{
				if (distance < max_distances[(vector_size) k])
				{
					counters[(vector_size) k] = counters[(vector_size) k] + 1;
				}
			}
		}

		max_value = Max<float>(max_value, counters[(vector_size) max_k - 1]);    
		sum = sum + counters[(vector_size) max_k - 1];
		mean = sum / (float) (i+1);

		result(i, 0) = (float) i;
		for (j = 0; j < max_k; j++)
		{
			result(i, j+1) = counters[(vector_size) j];
		}

		vPrint("object %li of %li, max_value = %8li, mean = %f\r", 
			(long) (i+1), (long) test_number, (long) round_number(max_value), mean);
	}
	vPrint("\n");


	return result;
}


// "name" is the name of an entire dataset, "bm_name" is the filename
// storing an embedding, and dimensions specifies the dimensionality of
// the embedding that we use. The result is a filename where we should store
// the result of IndexError3.
char * BoostMap_data::IndexErrorOutputPath(const char * name, const char * bm_name, 
	vint8 dimensions)
{
	char * dimension_string = string_from_number(dimensions);
	char * filename = vMergeStrings5(name, "b_", bm_name, "_", dimension_string);
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "index_errors");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		exit_error("error: indexerroroutputpath, no directory %s found.\n",
			directory);
	}

	vdelete2(dimension_string);
	vdelete2(filename);
	vdelete2(directory);
	return pathname;
}


char * BoostMap_data::FilterRefineOutputPath(const char * name, const char * bm_name, 
	vint8 dimensions, vint8 to_keep)
{
	char * dimension_string = string_from_number(dimensions);
	char * to_keep_string = string_from_number(to_keep);
	char * filename2 = vMergeStrings5(name, "fr", bm_name, "_", dimension_string);
	char * filename = vMergeStrings4(filename2, "_", to_keep_string, ".txt");
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "fr_errors");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		function_print("error: cannot save results to file, need to create directory %s.\n",
			directory);
		delete_pointer(pathname);
		pathname = 0;
	}

	vdelete2(dimension_string);
	vdelete2(to_keep_string);
	vdelete2(filename);
	vdelete2(filename2);
	vdelete2(directory);
	return pathname;
}


char * BoostMap_data::CascadeInfoOutputPath(const char * name, const char * bm_name, 
	vint8 dimensions, vint8 to_keep)
{
	char * dimension_string = string_from_number(dimensions);
	char * to_keep_string = string_from_number(to_keep);
	char * filename2 = vMergeStrings5(name, "_cascade_", bm_name, "_", dimension_string);
	char * filename = vMergeStrings4(filename2, "_", to_keep_string, ".bin");
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "cascade_info");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		exit_error("error: indexerroroutputpath, no directory %s found.\n",
			directory);
	}

	vdelete2(dimension_string);
	vdelete2(to_keep_string);
	vdelete2(filename);
	vdelete2(filename2);
	vdelete2(directory);
	return pathname;
}


char * BoostMap_data::CascadeKnnOutputPath(const char * name, const char * bm_name, 
	vint8 dimensions, vint8 to_keep)
{
	char * dimension_string = string_from_number(dimensions);
	char * to_keep_string = string_from_number(to_keep);
	char * filename2 = vMergeStrings5(name, "_knn_", bm_name, "_", dimension_string);
	char * filename = vMergeStrings4(filename2, "_", to_keep_string, ".bin");
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "cascade_info");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		exit_error("error: indexerroroutputpath, no directory %s found.\n",
			directory);
	}

	vdelete2(dimension_string);
	vdelete2(to_keep_string);
	vdelete2(filename);
	vdelete2(filename2);
	vdelete2(directory);
	return pathname;
}



// "name" is the name of an entire dataset, "bm_name" is the filename
// storing an embedding, and dimensions specifies the dimensionality of
// the embedding that we use. The result is a filename where we should store
// the result of IndexErrors4.
char * BoostMap_data::IndexErrorOutputPath2(const char * name, const char * bm_name, 
	vint8 dimensions)
{
	char * dimension_string = string_from_number(dimensions);
	char * filename = vMergeStrings5(name, "k_", bm_name, "_", dimension_string);
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "index_errors");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		function_print("error: cannot save results to file, need to create directory %s.\n",
			directory);
		delete_pointer(pathname);
		pathname = 0;
	}

	vdelete2(dimension_string);
	vdelete2(filename);
	vdelete2(directory);
	return pathname;
}


// PermuteTraining permutes the order of the database objects. This is
// useful when the database objects are not in a random order to begin with.
// By putting them in a random order, we can then choose consecutive
// ranges for candidates, training and validation objects, and know that
// all three sets are essentially random subsets of the database. If 
// we did not permute, then in some datasets, where objects appear in
// groups of similar objects, the training would be different from the 
// validation set (and the test set), and there would be overfitting. Plus,
// the candidates would not cover the whole database, which would lead
// to worse choices for 1D embeddings.
vint8 BoostMap_data::PermuteTraining(const char * name, const char * new_name)
{
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if (training_labels.valid() <= 0)
	{
		vPrint("failed to load training labels\n");
		return 0;
	}

	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	if (test_labels.valid() <= 0)
	{
		vPrint("failed to load test labels\n");
		return 0;
	}

	vint8 test_size = test_labels.Size();
	vint8 training_size = training_labels.Size();

	// if permutation(i) = j, it means that i in name maps to j
	// in new_name.
	vint8_matrix permutation = vPermutation(0, training_size - 1);

	// test labels do not need to be permuted, they just need to
	// be saved to the directory of the permuted dataset.
	vint8 success = SaveTestLabels(g_data_directory, new_name, test_labels);
	if (success <= 0)
	{
		return 0;
	}

	// training labels do need to be permuted.
	vMatrix<float> permuted_training_labels(1, training_size);
	vint8 i;
	for (i = 0; i < training_size; i++)
	{
		vint8 index1 = i;
		vint8 index2 = permutation(i);
		permuted_training_labels(index2) = training_labels(index1);
	}
	success = SaveTrainingLabels(g_data_directory, new_name, permuted_training_labels);
	if (success <= 0)
	{
		return 0;
	}

	// Save the permutation.
	char * directory = original_directory(g_data_directory, new_name);
	char * permutation_file = vJoinPaths(directory, "permutation.bin");
	success = permutation.Write(permutation_file);
	vdelete2(directory);
	if (success <= 0)
	{
		vPrint("failed to save permutation to %s\n", permutation_file);
		vdelete2(permutation_file);
		return 0;
	}
	vdelete2(permutation_file);

	// compute the inverse permutation 
	vint8_matrix inverse_permutation(1, training_size);
	for (i = 0; i < training_size; i++)
	{
		vint8 j = permutation(i);
		inverse_permutation(j) = i;
	}

	// now we need to permute the test-to-database distances.
	char * testtrain_path = TestTrainDistancesPath(g_data_directory, new_name);
	FILE * fp = fopen(testtrain_path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", testtrain_path);
		vdelete2(testtrain_path);
		return 0;
	}
	vdelete2(testtrain_path);

	success = vWriteHeader(fp, FloatType, test_size, training_size, 1);
	if (success == 0)
	{
		fclose(fp);
		vPrint("could not write header to test-train file\n");
		return 0;
	}

	// Saving new testtrain distances.
	vPrint("saving testtrain:\n");
	vMatrix<float> permuted_distances(1, training_size);
	for (i = 0; i < test_size; i++)
	{
		vMatrix<float> distances = TestTrainDistance(name, i);
		if (distances.valid() <= 0)
		{
			exit_error("error: invalid distances\n");
		}
		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 index1 = j;
			vint8 index2 = permutation(j);
			permuted_distances(index2) = distances(index1);
		}
		success = permuted_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write distances for test object %li\n", i);
		}
		vPrint("processed test object %li of %li\r", (long) (i+1), (long) test_size);
	}
	fclose(fp);
	vPrint("\n");

	// now we need to permute the database-to-database distances.
	char * traintrain_path = TrainTrainDistancesPath(g_data_directory, new_name);
	fp = fopen(traintrain_path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", traintrain_path);
		vdelete2(traintrain_path);
		return 0;
	}
	vdelete2(traintrain_path);

	success = vWriteHeader(fp, FloatType, training_size, training_size, 1);
	if (success == 0)
	{
		fclose(fp);
		vPrint("could not write header to train-train file\n");
		return 0;
	}

	// Saving new traintrain distances.
	vPrint("saving traintrain:\n");
	for (i = 0; i < training_size; i++)
	{
		vint8 object1 = inverse_permutation(i);
		vMatrix<float> distances = TrainTrainDistance(g_data_directory, name, object1);
		if (distances.valid() <= 0)
		{
			exit_error("error: invalid distances\n");
		}
		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 index1 = j;
			vint8 index2 = permutation(j);
			permuted_distances(index2) = distances(index1);
		}
		success = permuted_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write distances for training object %li\n", i);
		}
		vPrint("processed training object %li of %li\r", (long) (i+1), (long) training_size);
	}
	vPrint("\n");
	fclose(fp);

	vPrint("finisned permuting dataset %s into dataset %s\n", name, new_name);
	return 1;
}

// produces a "merged_distances.bin" and a "merged_labels.bin", which
// is a file storing a square file, with distances from all training
// and test objects to all training and test objects (training objects
// appear first).
vint8 BoostMap_data::MergeTestTrain(const char * name)
{
	vint8 error_flag = 0;

	// check if input files exist.
	char * directory = original_directory(g_data_directory, name);
	char * testtrain_file = TestTrainDistancesPath(g_data_directory, name);
	if (vFileExists(testtrain_file) <= 0)
	{
		vPrint("could not open %s\n", testtrain_file);
		error_flag = 1;
	}
	char * traintrain_file = TrainTrainDistancesPath(g_data_directory, name);
	if (vFileExists(traintrain_file) <= 0)
	{
		vPrint("could not open %s\n", traintrain_file);
		error_flag = 1;
	}
	char * traintest_file = vJoinPaths(directory, "traintest_distances.bin");
	if (vFileExists(traintest_file) <= 0)
	{
		vPrint("could not open %s\n", traintest_file);
		error_flag = 1;
	}
	char * testtest_file = vJoinPaths(directory, "testtest_distances.bin");;
	if (vFileExists(testtest_file) <= 0)
	{
		vPrint("could not open %s\n", testtest_file);
		error_flag = 1;
	}

	if (error_flag == 1)
	{
		vdelete2(directory);
		vdelete2(testtrain_file);
		vdelete2(traintrain_file);
		vdelete2(testtest_file);
		vdelete2(traintest_file);
		return 0;
	}

	// write merged labels
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if (training_labels.valid() <= 0)
	{
		exit_error("error: could not load training labels\n");
	}
	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	if (test_labels.valid() <= 0)
	{
		exit_error("error: could not load testing labels\n");
	}

	vint8 training_size = training_labels.Size();
	vint8 test_size = test_labels.Size();
	vint8 merged_size = training_size + test_size;
	vMatrix<float> merged_labels(1, merged_size);
	vint8 i;
	for (i = 0; i < training_size; i++)
	{
		merged_labels(i) = training_labels(i);
	}
	for (i = 0; i < test_size; i++)
	{
		merged_labels(i + training_size) = test_labels(i);
	}
	char * label_path = vJoinPaths(directory, "merged_labels.bin");
	vint8 success = merged_labels.Write(label_path);
	if (success <= 0)
	{
		vPrint("failed to write to %s\n", label_path);
		error_flag = 1;
	}
	else
	{
		vPrint("saved merged labels to %s\n", label_path);
	}
	vdelete2(label_path);

	// write merged distances.
	char * output_file = vJoinPaths(directory, "merged_distances.bin");
	FILE * fp = fopen(output_file, vFOPEN_WRITE);
	if (fp == 0)
	{
		exit_error("error: failed to open %s\n", output_file);
	}

	// write header for merged distances.
	success = vWriteHeader(fp, FloatType, merged_size, merged_size, 1);
	if (success <= 0)
	{
		exit_error("failed to write header to %s\n", output_file);
	}

	// write train-to-train and train-to-test distances.
	vPrint("\n");
	for (i = 0; i < training_size; i++)
	{
		vMatrix<float> train_distances = ObjectDistances(traintrain_file, i);
		if ((train_distances.valid() <= 0) || (train_distances.Size() != training_size))
		{
			exit_error("\nfailed to load train distances for object %li\n", (long) i);
		}
		vMatrix<float> test_distances = ObjectDistances(traintest_file, i);
		if ((test_distances.valid() <= 0) || (test_distances.Size() != test_size))
		{
			exit_error("\nfailed to load test distances for object %li\n", (long) i);
		}

		success = train_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write training distances for object %li\n", (long) i);
		}
		success = test_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write test distances for object %li\n", (long) i);
		}

		vPrint("saved training distances for object %li of %li\r", (i+1), (long) training_size);
	}
	vPrint("\n");

	// write test-to-train and test-to-test distances.
	vPrint("\n");
	for (i = 0; i < test_size; i++)
	{
		vMatrix<float> train_distances = ObjectDistances(testtrain_file, i);
		if ((train_distances.valid() <= 0) || (train_distances.Size() != training_size))
		{
			exit_error("\nfailed to load testtrain distances for object %li\n", (long) i);
		}
		vMatrix<float> test_distances = ObjectDistances(testtest_file, i);
		if ((test_distances.valid() <= 0) || (test_distances.Size() != test_size))
		{
			exit_error("\nfailed to load testtest distances for object %li\n", (long) i);
		}

		success = train_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write testtraining distances for object %li\n", (long) i);
		}
		success = test_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("\nfailed to write testtest distances for object %li\n", (long) i);
		}

		vPrint("saved test distances for object %li of %li\r", (long) (i+1), (long) test_size);
	}
	vPrint("\n");

	fclose(fp);
	vdelete2(directory);
	vdelete2(testtrain_file);
	vdelete2(traintrain_file);
	vdelete2(testtest_file);
	vdelete2(traintest_file);
	return 0;

	if (error_flag == 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// Split assumes there are "merged_distances.bin" and "merged_labels.bin"
// files in dataset name1. Based on those, it splits the entire dataset
// into a training and a test set, and saves the corresponding distances
// and labels.
vint8 BoostMap_data::Split(const char * name1, const char * name2,
	vint8 test_size)
{
	if (test_size <= 0)
	{
		vPrint("test side should be greater than zero\n");
		return 0;
	}

	char * directory = original_directory(g_data_directory, name1);
	char * merged_path = vJoinPaths(directory, "merged_distances.bin");
	char * merged_labels_path = vJoinPaths(directory, "merged_labels.bin");

	vint8 error_flag = 0;
	if (vFileExists(merged_path) <= 0)
	{
		vPrint("file %s does not exist\n", merged_path);
		error_flag = 1;
	}
	if (vFileExists(merged_labels_path) <= 0)
	{
		vPrint("file %s does not exist\n", merged_labels_path);
		error_flag = 1;
	}
	if (error_flag != 0)
	{
		vdelete2(directory);
		vdelete2(merged_path);
		vdelete2(merged_labels_path);
		return 0;
	}

	vMatrix<float> merged_labels = vMatrix<float>::Read(merged_labels_path);
	if (merged_labels.valid() <= 0)
	{
		exit_error("error: could not load from %s\n", merged_labels_path);
	}
	vint8 merged_size = merged_labels.Size();
	if (test_size >= merged_size)
	{
		vdelete2(directory);
		vdelete2(merged_path);
		vdelete2(merged_labels_path);
		vPrint("test size should be less than %li\n", (long) merged_size);
		return 0;
	}

	vint8 training_size = merged_size - test_size;

	// if permutation(i) = j, it means that object i in original dataset
	// (name1) goes to object j in new dataset.
	vint8_matrix permutation = vPermutation(0, merged_size - 1);

	vint8 i;
	// compute the inverse permutation 
	vint8_matrix inverse_permutation(1, merged_size);
	for (i = 0; i < merged_size; i++)
	{
		vint8 j = permutation(i);
		inverse_permutation(j) = i;
	}

	// Save permutation.
	char * directory2 = original_directory(g_data_directory, name2);
	char * permute_path = vJoinPaths(directory2, "split_permutation.bin");
	vint8 success = permutation.Write(permute_path);
	if (success <= 0)
	{
		exit_error("error: failed to write permutation into %s\n",
			permute_path);
	}

	// save test labels.
	vMatrix<float> test_labels(1, test_size);
	for (i = 0; i < test_size; i++)
	{
		vint8 index2 = training_size + i;
		vint8 index1 = inverse_permutation(index2);
		test_labels(i) = merged_labels(index1);
	}

	success = SaveTestLabels(g_data_directory, name2, test_labels);
	if (success <= 0)
	{
		exit_error("error: failed to write test labels");
	}

	// save training labels.
	vMatrix<float> training_labels(1, training_size);
	for (i = 0; i < training_size; i++)
	{
		vint8 index1 = inverse_permutation(i);
		vint8 index2 = i;
		training_labels(index2) = merged_labels(index1);
	}

	success = SaveTrainingLabels(g_data_directory, name2, training_labels);
	if (success <= 0)
	{
		exit_error("error: failed to write training labels");
	}

	FILE * fp;
	// save testtrain distances
	char * testtrain_path = TestTrainDistancesPath(g_data_directory, name2);
	fp = fopen(testtrain_path, vFOPEN_WRITE);
	if (fp == 0)
	{
		exit_error("error: could not open %s", testtrain_path);
	}
	success = vWriteHeader(fp, FloatType, test_size, training_size, 1);
	if (success <= 0)
	{
		exit_error("error: could not write header to %s", testtrain_path);
	}

	vPrint("\n");
	for (i = 0; i < test_size; i++)
	{
		vint8 index2 = i + training_size;
		vint8 index1 = inverse_permutation(index2);
		vMatrix<float> all_distances = ObjectDistances(merged_path, index1);
		vMatrix<float> testtrain_distances(1, training_size);

		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 index2 = j;
			vint8 index1 = inverse_permutation(index2);
			float distance = all_distances(index1);
			testtrain_distances(index2) = distance;
		}
		success = testtrain_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("error: could not write testtrain distances for object %li\n", (long) i);
		}

		vPrint("processed test object %li of %li\r", (long) (i+1), (long) test_size);
	}
	fclose(fp);

	// save traintrain distances
	char * traintrain_path = TrainTrainDistancesPath(g_data_directory, name2);
	fp = fopen(traintrain_path, vFOPEN_WRITE);
	if (fp == 0)
	{
		exit_error("error: could not open %s", traintrain_path);
	}
	success = vWriteHeader(fp, FloatType, training_size, training_size, 1);
	if (success <= 0)
	{
		exit_error("error: could not write header to %s", traintrain_path);
	}

	vPrint("\n");
	for (i = 0; i < training_size; i++)
	{
		vint8 index2 = i;
		vint8 index1 = inverse_permutation(index2);
		vMatrix<float> all_distances = ObjectDistances(merged_path, index1);
		vMatrix<float> traintrain_distances(1, training_size);

		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 index2 = j;
			vint8 index1 = inverse_permutation(index2);
			float distance = all_distances(index1);
			traintrain_distances(index2) = distance;
		}
		success = traintrain_distances.WriteBandwise(fp);
		if (success <= 0)
		{
			exit_error("error: could not write traintrain distances for object %li\r", (long) i);
		}

		vPrint("processed training object %li of %li\r", (long) (i+1), (long) training_size);
	}
	vPrint("\n");
	fclose(fp);
	vdelete2(directory);
	vdelete2(merged_path);
	vdelete2(merged_labels_path);
	vdelete2(directory2);
	vdelete2(permute_path);
	vdelete2(testtrain_path);
	vdelete2(traintrain_path);

	return 1;
}


// This function creates a new dataset for training BoostMap, that
// allows BoostMap to be run "on top" of another method. For example,
// if the training embedding is a SparseMap or FastMap embedding,
// we can then run the BoostMap algorithm (with distance_p = 2,
// allow_negative = 0, allow_projections = 0), so that it simply
// computes weights to be used in measuring the Euclidean distance
// in the target space. We keep the same training_distances and
// validation_distances, but we change candtrain and candval. 
// candcand is useless, so we just write zeros to it. We keep the 
// header (except for candidates, where we write -1 to signify
// that we don't use candidate objects here).
vint8 BoostMap_data::OnTopDataset(const char * new_name, 
	vMatrix<float> training_embeddingm)
{
	if (is_valid <= 0)
	{
		return 0;
	}

	vint8 new_candidate_number = training_embeddingm.Cols();
	vMatrix<vint8> new_candidates(1, new_candidate_number);
	function_enter_value(&new_candidates, (vint8) -1);

	// create new dataset, save header.
	BoostMap_data new_set(root_data_dir, new_name, new_candidates, 
		training_ids, validation_ids);

	// save new candcand distances (set to 0)
	vMatrix<float> new_candcand(new_candidate_number, new_candidate_number);
	function_enter_value(&new_candcand, (float) 0);
	new_set.SaveCandCandDistances(new_candcand);

	// save training/validation distances and pdistances (which remain the same)
	vMatrix<float> distances_matrix = LoadTrainingDistances();
	new_set.SaveTrainingDistances(distances_matrix);

	distances_matrix = LoadValidationDistances();
	new_set.SaveValidationDistances(distances_matrix);

	distances_matrix = LoadTrainingPdistances();
	new_set.SaveTrainingPdistances(distances_matrix);

	distances_matrix = LoadValidationPdistances();
	new_set.SaveValidationPdistances(distances_matrix);

	// create candtrain distances.
	vMatrix<float> new_candtrain(new_candidate_number, training_number);
	vint8 row, col;
	for (row = 0; row < new_candidate_number; row++)
	{
		for (col = 0; col < training_number; col++)
		{
			vint8 index = training_ids(col);
			new_candtrain(row, col) = training_embeddingm(index, row);
		}
	}
	new_set.SaveCandTrainDistances(new_candtrain);

	// create candval distances.
	vMatrix<float> new_candval(new_candidate_number, validation_number);
	for (row = 0; row < new_candidate_number; row++)
	{
		for (col = 0; col < validation_number; col++)
		{
			vint8 index = validation_ids(col);
			new_candval(row, col) = training_embeddingm(index, row);
		}
	}
	new_set.SaveCandValDistances(new_candval);
	return 1;
}


// this function takes in exact distances and approximate distances
// for a given object, and reports, in result(k-1),
// the approximate rank of the k-th nearest neibhbor.
vMatrix<float> BoostMap_data::IndexErrors3(vMatrix<float> approximate_distances,
	vMatrix<float> exact_distances, 
	vint8 max_k)
{
	if ((approximate_distances.valid() <= 0) || 
		(exact_distances.valid() <= 0) ||
		(max_k <= 0))
	{
		return vMatrix<float>();
	}

	vint8 training_number = approximate_distances.Size();
	if (approximate_distances.Size() != exact_distances.Size())
	{
		exit_error("Error: inconsistent number of distances: %li %li\n",
			(long) approximate_distances.Size(), (long) exact_distances.Size());
	}

	vMatrix<float> result(1, max_k+1);
	vMatrix<float> original_distances = exact_distances;

	// get embedding-based distances from object to database.
	vMatrix<float> distances = approximate_distances;

	// Find the max-k nearest neighbors
	vint8 junk = 0, nn_index = 0;
	vector<vint8> nn_indices((vector_size) max_k, -1);
	vector<float> original_vector;
	vector<vint8> index_ranks;
	vector_from_matrix(&original_distances, &original_vector);
	vIndexRanks(&original_vector, &index_ranks);

	vint8 j;
	for (j = 0; j < training_number; j++)
	{
		if (index_ranks[(vector_size) j] < max_k)
		{
			vint8 rank = index_ranks[(vector_size) j];
			nn_indices[(vector_size) rank] = j;
		}
	}

	vector<float> counters((vector_size) max_k);
	vector<float> embedded_distances((vector_size) max_k);
	vector<float> max_distances((vector_size) max_k);

	for (j = 0; j < max_k; j++)
	{
		vint8 index = nn_indices[(vector_size) j];
		if (index == -1)
		{
			exit_error("error: index = -1\n");
		}
		float distance = distances(index);
		embedded_distances[(vector_size) j] = distance;
		counters[(vector_size) j] = 0;
	}

	max_distances[0] = embedded_distances[0];
	for (j = 1; j < max_k; j++)
	{
		float previous = max_distances[(vector_size) j-1];
		float current = embedded_distances[(vector_size) j];
		max_distances[(vector_size) j] = Max(previous, current);
	}

	// get worst ranks for each k.
	for (j = 0; j < training_number; j++)
	{
		float distance = distances(j);
		vint8 k;
		for (k = 0; k < max_k; k++)
		{
			if (distance < max_distances[(vector_size) k])
			{
				counters[(vector_size) k] = counters[(vector_size) k] + 1;
			}
		}
	}

	result(0, 0) = (float) 0;
	for (j = 0; j < max_k; j++)
	{
		result(0, j+1) = counters[(vector_size) j];
	}

	return result;
}


// this function takes in the distances from an object to all training
// objects. It computes, for each k, whether that object has
// all its k nearest neighbors bevint8ing to a single class, and whether 
// that single class is the correct class or not. results(k) is
// incremented by 1 if all k nearest neighbors bevint8 to the same class,
// and error_results(k) is incremented by 1 if that class is the wrong
// class.
vint8 BoostMap_data::CascadeStats5(vMatrix<float> distancesm, vMatrix<float> labelsm,
	vint8 bad_index, vint8 test_label, 
	vint8_matrix results)
{
	if ((distancesm.valid() <= 0) || (labelsm.valid() <= 0) ||
		(labelsm.Size() != distancesm.Size()))
	{
		return 0;
	}

	vint8 training_number = labelsm.Size();
	vArray(float) distances = distancesm.Matrix();
	vArray(float) labels = labelsm.Matrix();

	vint8 j, k;
	vint8 max_k = results.Cols();

	vint8 threshold_index = 0;
	float max_distance_limit = function_image_maximum(&distancesm) * ((float) 2.0) + (float) 1.0;
	float threshold_distance = kth_smallest_cb(max_k, &distancesm,
		&threshold_index);
	vector<sResultPair> result_pairs;

	// collect the max_k neighbors (and neighbors after that that are
	// tied to the max_k neighbor).
	for (j = 0; j < training_number; j++)
	{
		if (distances[j] <= threshold_distance)
		{
			sResultPair pair(distances[j], round_number(labels[j]));
			result_pairs.push_back(pair);
		}
	}
	std::sort(result_pairs.begin(), result_pairs.end(), sResultPairLess());

	vint8 number = result_pairs.size();
	if (number < max_k)
	{
		exit_error("error: there must be a bug in CascadeStats\n");
	}

	vint8 first_label = result_pairs[0].label;
	results(0, 1)++;
	if (test_label != first_label)
	{
		results(1, 1)++;
		if (bad_index != 0)
		{
			results(2, 1)++;
		}
	}
	else if (bad_index != 0)
	{
		results(3, 1)++;
	}

	for (k = 2; k < max_k; k++)
	{
		vint8 current_label = result_pairs[(vector_size) k-1].label;
		if (current_label != first_label)
		{
			break;
		}
		results(0, k)++;
		if (test_label != first_label)
		{
			results(1, k)++;
			if (bad_index != 0)
			{
				results(2, k)++;
			}
		}
		else if (bad_index != 0)
		{
			results(3, k)++;
		}
	}

	return 1;
}




// "name" is the name of an entire dataset. bm_name is the 
// name of a file where we saved a summary of the BoostMap 
// training. From that file, we build a d-dimensional embedding,
// according to the argument "dimensions". The matrices 
// results and error_results are passed to CascadeStats5,
// which computes the cascade stats.
// there is a subtle point here, in that we read 
vint8 BoostMap_data::CascadeStats4(const char * name, const char * bm_name,
	vint8 dimensions, vint8_matrix results)
{
	vint8_matrix bad_indices = ReadBadIndices(name);
	if (bad_indices.valid() <= 0)
	{
		return 0;
	}

	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	// read the embedding description
	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(bm_path);
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return 0;
	}

	// select the first "dimensions" dimensions
	vMatrix<float> classifiers2 = 
		class_BoostMap::select_first_dimensions(all_classifiers, -1);
	vMatrix<float> classifiers;
	if ((dimensions == -1) || dimensions > classifiers2.Rows())
	{
		classifiers = classifiers2;
	}
	else
	{
		classifiers = vMatrix<float>(dimensions, classifiers2.Cols());
		vCopyRectangle2(&classifiers2, &classifiers, 0, 0, (long) dimensions, 
			(long) classifiers2.Cols(), 0, 0);
	}

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// get the weights for the L1 distance.
	vMatrix<float> weights = class_BoostMap::ExtractWeights(classifiers);
	weights.Print("weights");

	// load the class labels
	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (training_labels.valid() <= 0))
	{
		vPrint("Invalid labels distances\n");
		return 0;
	}

	// embed training and test sets.
	vMatrix<float> embedded_database = load_training_embedding(name, bm_name);
	if (embedded_database.valid() <= 0)
	{
		return 0;
	}
	vint8 database_size = embedded_database.Rows();

	vMatrix<float> embedded_test_set = load_test_embedding(name, bm_name);
	if (embedded_test_set.valid() <= 0)
	{
		return 0;
	}

	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return 0;
	}

	function_enter_value(&results, (vint8) 0);

	// compute the errors.
	vint8 result = CascadeStats8(embedded_test_set, embedded_database,
		weights, test_labels, training_labels,
		bad_indices, results, 0);

	return result;
}


vint8 BoostMap_data::CascadeStats8(vMatrix<float> test_set, vMatrix<float> training_set,
	vMatrix<float> weights, vMatrix<float> test_labels,
	vMatrix<float> train_labels, vint8_matrix bad_indices,
	vint8_matrix results, vint8 training_flag)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(weights.valid() <= 0) || (test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0))
	{
		return 0;
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return 0;
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		// get the clas label of the test object
		vint8 label = round_number(test_labels(i));

		// get the embedding of the test object
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);

		// get the distances of the object's embedding to 
		// the embeddings of all training objects
		vMatrix<float> distances = L1Distances(&object, training_set, weights);

		// if test set == training set, then we should temporarily "delete"
		// the test object from the training set
		if (training_flag == 1)
		{
			float max_distance = vAbs(function_image_maximum(&distances));
			distances(i) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		CascadeStats5(distances, train_labels, bad_indices(i), label, results);
		vPrint("processed object %li of %li\r", (long) (i+1), (long) test_number);
	}
	vPrint("\n");

	return 1;
}


vint8_matrix BoostMap_data::ReadBadIndices(const char * name)
{
	char * directory = cascade_directory(g_data_directory, name);
	char * pathname = vJoinPaths(directory, "bad_indices");
	vdelete2(directory);
	vint8_matrix result = vint8_matrix::Read(pathname);
	if (result.valid() <= 0)
	{
		vPrint("failed to read bad_indices from %s\n", pathname);
	}
	vdelete2(pathname);
	return result;
}


vint8_matrix  BoostMap_data::read_database_test_indices(const char * name)
{
	char * directory = original_directory(g_data_directory, name);
	char * pathname = vJoinPaths(directory, "database_test_indices.bin");
	vdelete2(directory);
	vint8_matrix result = vint8_matrix::Read(pathname);
	if (result.valid() > 0)
	{
		vPrint("successfully read database test indices from %s\n", pathname);
	}
	vdelete2(pathname);
	return result;
}


// cascade stats for filter and refine. result contains two rows. 
// result(0, i) stores number of single_class neighbors for object i,
// i.e. the max number k such that all k neighbors of i bevint8 to the
// same class. result(1,i) is 0 if the class of those k neighbors is
// the same class as the class of i, and 1 otherwise.
// in output argument results we store:
// results(0,k): # of objects with k same-class nearest neighbors
// results(1,k): # of misclassified objects with k same-class nearest neighbors 
// results(2,k): # of BAD misclassified objects with k same-class nearest neighbors 
// results(2,k): # of BAD correctly classified objects with k same-class nearest neighbors 
vint8_matrix BoostMap_data::CascadeStatsFr(const char * name, const char * bm_name,
	vint8 dimensions, vint8 to_keep, vint8_matrix results)
{
	vint8_matrix database_test_indices = read_database_test_indices(name);

	char * bm_path = class_BoostMap::Pathname(g_data_directory, bm_name);

	// read the embedding description
	vMatrix<float> all_classifiers = float_matrix(vMatrix<float>::read_text(bm_path));
	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", bm_path);
		vdelete2(bm_path);
		return vint8_matrix();
	}

	// select the first "dimensions" dimensions
	vMatrix<float> classifiers2 = 
		class_BoostMap::select_first_dimensions(all_classifiers, -1);
	vMatrix<float> classifiers;
	if ((dimensions == -1) || dimensions > classifiers2.Rows())
	{
		classifiers = classifiers2;
	}
	else
	{
		classifiers = vMatrix<float>(dimensions, classifiers2.Cols());
		vCopyRectangle2(&classifiers2, &classifiers, 0, 0, (long) dimensions, 
			(long) classifiers2.Cols(), 0, 0);
	}

	classifiers.Print("classifiers");
	vdelete2(bm_path);

	// get the weights for the L1 distance.
	vMatrix<float> weights = class_BoostMap::ExtractWeights(classifiers);
	weights.Print("weights");

	// load the class labels
	vMatrix<float> test_labels = LoadTestLabels(g_data_directory, name);
	vMatrix<float> training_labels = LoadTrainingLabels(g_data_directory, name);
	if ((test_labels.valid() <= 0) || (training_labels.valid() <= 0))
	{
		vPrint("Invalid labels distances\n");
		return vint8_matrix();
	}

	// embed training and test sets.
	vMatrix<float> embedded_database = embed_database(name, classifiers);
	if (embedded_database.valid() <= 0)
	{
		return vint8_matrix();
	}
	vint8 database_size = embedded_database.Rows();

	vMatrix<float> embedded_test_set = embed_test_set(name, classifiers,
		database_size);
	if (embedded_test_set.valid() <= 0)
	{
		return vint8_matrix();
	}

	if ((embedded_database.valid() <= 0) || 
		(embedded_test_set.valid() <= 0))
	{
		vPrint("Failed to embed test and training sets\n");
		return vint8_matrix();
	}

	function_enter_value(&results, (vint8) 0);

	char * pathname = TestTrainDistancesPath(g_data_directory, name);
	class_file * distances_fp = OpenObjectDistancesFile(pathname);
	if (distances_fp->file_pointer == 0)
	{
		vPrint("failed to open %s\n", pathname);
		vdelete2(pathname);
		return vint8_matrix();
	}
	vdelete2(pathname);

	vint8 bad_index_flag = 0;
	vint8_matrix bad_indices = ReadBadIndices(name);
	if (bad_indices.valid() <= 0)
	{
		bad_index_flag = 1;
		bad_indices = vint8_matrix(1, embedded_test_set.vertical());
		function_enter_value(&bad_indices, (vint8) 0);
	}
	vint8_matrix knn_results(test_labels.Size(), 50);

	// compute the errors.
	vint8_matrix result = CascadeStatsFr2(embedded_test_set, embedded_database,
		weights, test_labels, training_labels,
		bad_indices, results, 0, to_keep, distances_fp,
		knn_results, database_test_indices);
	fclose(distances_fp);

	pathname = CascadeInfoOutputPath(name, bm_name, dimensions, to_keep);
	vint8 success = result.Write(pathname);
	if (success <= 0)
	{
		vPrint("failed to save cascade info to %s\n", pathname);
	}
	else
	{
		vPrint("saved cascade info to %s\n", pathname);
	}
	vdelete2(pathname);

	pathname = CascadeKnnOutputPath(name, bm_name, dimensions, to_keep);
	success = knn_results.Write(pathname);
	if (success <= 0)
	{
		vPrint("failed to save knn results to %s\n", pathname);
	}
	else
	{
		vPrint("saved knn results to %s\n", pathname);
	}
	vdelete2(pathname);

	if (bad_index_flag == 1)
	{
		function_print("bad indices not loaded\n");
	}
	return result;
}



vint8_matrix BoostMap_data::CascadeStatsFr2(vMatrix<float> test_set, vMatrix<float> training_set,
	vMatrix<float> weights, vMatrix<float> test_labels,
	vMatrix<float> train_labels, vint8_matrix bad_indices,
	vint8_matrix results, vint8 training_flag, vint8 to_keep,
	class_file * fp, vint8_matrix knn_results, vint8_matrix database_test_indices)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(weights.valid() <= 0) || (test_labels.valid() <= 0) || 
		(train_labels.valid() <= 0))
	{
		return vint8_matrix();
	}

	if ((test_labels.valid() <= 0) || (train_labels.valid() <= 0))
	{
		return vint8_matrix();
	}

	vint8 test_number = test_labels.Size();
	vint8 training_number = train_labels.Size();
	if ((test_number == 0) || (training_number == 0) ||
		(test_number != test_set.Rows()) || 
		(training_number != training_set.Rows()) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	vint8_matrix test_info(2, test_number);

	vint8 i;
	vPrint("\n");
	for (i = 0; i < test_number; i++)
	{
		// get the class label of the test object
		vint8 label = round_number(test_labels(i));

		// get the embedding of the test object
		v3dMatrix<float> object = copy_horizontal_line(&test_set, i);

		// get the distances of the object's embedding to 
		// the embeddings of all training objects
		vMatrix<float> distances = L1Distances(&object, training_set, weights);
		vMatrix<float> original = NextObjectDistances(fp, distances.Size());

		float max_distance1 = vAbs(function_image_maximum(&distances));
		float max_distance2 = vAbs(function_image_maximum(&original));
		float limit = Max(max_distance1, max_distance2) * ((float) 2.0) + (float) 1.0;

		if (database_test_indices.valid() > 0)
		{
			vint8 index = database_test_indices(i);
			distances(index) = limit;
		}


		if (training_flag == 1)
		{
			distances(i) = limit;
		}

		vint8 junk = 0;
		// we add 1 to to_keep, so that then we check if a distance is 
		// strictly smaller than that. This way, if there are ties we 
		// keep fewer than to_keep, and if there are no ties we
		// keep exactly to_keep objects.
		float threshold_distance = kth_smallest_cb(to_keep+1, &distances,
			&junk);

		vint8 j;
		for (j = 0; j < distances.Size(); j++)
		{ 
			if (distances(j) < threshold_distance)
			{
				distances(j) = original(j);
			}
			else
			{
				distances(j) = distances(j) + limit;
			}
		}

		vMatrix<float> labels = KnnLabel4(distances, train_labels, label, knn_results.Cols() + 3);
		if (labels.valid() <= 0)
		{
			exit_error("Error: invalid labels in KnnError\n");
		}

		for (j = 0; j <  knn_results.Cols(); j++)
		{
			if (round_number(labels(j)) == round_number(label))
			{
				knn_results(i, j) = (vint8) 0;
			}
			else
			{
				knn_results(i, j) = (vint8) 1;
			}
		}

		vint8 same_class_k = 0;
		vint8 error = CascadeStatsFr3(distances, train_labels, bad_indices(i), 
			label, results, &same_class_k);
		test_info(0, i) = same_class_k;
		test_info(1, i) = error;

		vPrint("processed object %li of %li\r", (long) (i+1), (long) test_number);
	}
	vPrint("\n");

	return test_info;
}

// returns whether the object was misclassified or not (1 for misclassified).
// Stores, in same_class_kp, the number of same-class neighbors.
vint8 BoostMap_data::CascadeStatsFr3(vMatrix<float> distancesm,
	vMatrix<float> labelsm,
	vint8 bad_index, vint8 test_label, 
	vint8_matrix results, vint8 * same_class_kp)
{
	if ((distancesm.valid() <= 0) || (labelsm.valid() <= 0) ||
		(labelsm.Size() != distancesm.Size()))
	{
		return 0;
	}

	vint8 training_number = labelsm.Size();
	vArray(float) distances = distancesm.Matrix();
	vArray(float) labels = labelsm.Matrix();

	vint8 j, k;
	vint8 max_k = results.Cols();

	vint8 threshold_index = 0;
	float max_distance_limit = function_image_maximum(&distancesm) * ((float) 2.0) + (float) 1.0;
	float threshold_distance = kth_smallest_cb(max_k, &distancesm,
		&threshold_index);
	vector<sResultPair> result_pairs;

	// collect the max_k neighbors (and neighbors after that that are
	// tied to the max_k neighbor).
	for (j = 0; j < training_number; j++)
	{
		if (distances[j] <= threshold_distance)
		{
			sResultPair pair(distances[j], round_number(labels[j]));
			result_pairs.push_back(pair);
		}
	}
	std::sort(result_pairs.begin(), result_pairs.end(), sResultPairLess());

	vint8 number = result_pairs.size();
	if (number < max_k)
	{
		exit_error("error: there must be a bug in CascadeStats\n");
	}

	vint8 first_label = result_pairs[0].label;
	results(0, 1)++;
	if (test_label != first_label)
	{
		results(1, 1)++;
		if (bad_index != 0)
		{
			results(2, 1)++;
		}
	}
	else if (bad_index != 0)
	{
		results(3, 1)++;
	}

	*same_class_kp = max_k;
	for (k = 2; k < max_k; k++)
	{
		vint8 current_label = result_pairs[(vector_size) k].label;
		if (current_label != first_label)
		{
			*same_class_kp = k - 1;
			break;
		}
		results(0, k)++;
		if (test_label != first_label)
		{
			results(1, k)++;
			if (bad_index != 0)
			{
				results(2, k)++;
			}
		}
		else if (bad_index != 0)
		{
			results(3, k)++;
		}
	}

	if (test_label != first_label)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// this function is useful when we already have an original dataset and 
// we want to create a smaller original dataset (maybe because we want
// to test out something quickly on the smaller dataset).
vint8 BoostMap_data::OriginalSubdataset(const char * original, const char * subset,
	vint8 test_start, vint8 test_size, 
	vint8 training_start, vint8 training_size)
{
	if ((test_start < 0) || (training_start < 0))
	{
		vPrint("bad arguments: test_start = %li, training_start = %li\n",
			(long) test_start, (long) training_start);
		return 0;
	}

	vint8 test_end = test_start + test_size - 1;
	vint8 training_end = training_start + training_size - 1;

	vint8 old_test_size = BoostMap_data::TestNumber(original);
	vint8 old_training_size = BoostMap_data::TrainingNumber(original);

	if ((old_test_size < test_end) || (old_training_size < training_end))
	{
		vPrint("old_test_size = %li, old_training_size = %li\n", 
			(long) old_test_size, (long) old_training_size);
		return 0;
	}

	// create the directory
	char * directory = BoostMap_data::original_directory(g_data_directory, subset);
	vint8 success = function_make_directory(directory);
	if (success <= 0)
	{
		vPrint("can't create directory %s\n", directory);
		vdelete2(directory);
		return 0;
	}

	vdelete2(directory);

	// create the test labels
	vMatrix<float> old_test_labels = BoostMap_data::LoadTestLabels(g_data_directory, original);
	if (old_test_labels.valid() <= 0)
	{
		vPrint("failed to load test labels\n");
		return 0;
	}

	vMatrix<float> test_labels(1, test_size);
	vint8 i;
	for (i = 0; i < test_size; i++)
	{
		test_labels(i) = old_test_labels(i + test_start);
	}
	success = BoostMap_data::SaveTestLabels(g_data_directory, subset, test_labels);
	if (success <= 0)
	{
		vPrint("failed to save test labels\n");
		return 0;
	}
	else
	{
		vPrint("saved test labels\n");
	}

	// create the training labels
	vMatrix<float> old_training_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, original);
	if (old_training_labels.valid() <= 0)
	{
		vPrint("failed to load training labels\n");
		return 0;
	}

	vMatrix<float> training_labels(1, training_size);
	for (i = 0; i < training_size; i++)
	{
		training_labels(i) = old_training_labels(i + training_start);
	}
	success = BoostMap_data::SaveTrainingLabels(g_data_directory, subset, training_labels);
	if (success <= 0)
	{
		vPrint("failed to save training labels\n");
		return 0;
	}
	else
	{
		vPrint("saved training labels\n");
	}


	// create the test-to-training distances.

	char * path = BoostMap_data::TestTrainDistancesPath(g_data_directory, subset);
	FILE * fp = fopen(path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", path);
		vdelete2(path);
		return 0;
	}
	vdelete2(path);

	success = vWriteHeader(fp, FloatType, test_size, training_size, 1);
	if (success <= 0)
	{
		vPrint("failed to write header\n");
		fclose(fp);
		return 0;
	}

	vPrint("\n");
	for (i = 0; i < test_size; i++)
	{
		vint8 index = i + test_start;
		vMatrix<float> old_distances = BoostMap_data::TestTrainDistance(original, index);
		vMatrix<float> distances(1, training_size);
		vint8 j;
		for (j = training_start; j <= training_end; j++)
		{
			distances(j - training_start) = old_distances(j);
		}
		success = distances.WriteBandwise(fp);
		if (success <= 0)
		{
			vPrint("\nfailed to save distances for object %li\n", (long) index);
			fclose(fp);
			return 0;
		}
		vPrint("saved distances for object %li of %li\r", (long) (i+1), (long) test_size);
	}
	vPrint("\n");
	fclose(fp);

	// create the training-to-training distances.

	path = BoostMap_data::TrainTrainDistancesPath(g_data_directory, subset);
	fp = fopen(path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", path);
		vdelete2(path);
		return 0;
	}
	vdelete2(path);

	success = vWriteHeader(fp, FloatType, training_size, training_size, 1);
	if (success <= 0)
	{
		vPrint("failed to write header\n");
		fclose(fp);
		return 0;
	}

	vPrint("\n");
	for (i = 0; i < training_size; i++)
	{
		vint8 index = i + training_start;
		vMatrix<float> old_distances = BoostMap_data::TrainTrainDistance(g_data_directory, original, index);
		vMatrix<float> distances(1, training_size);
		vint8 j;
		for (j = training_start; j <= training_end; j++)
		{
			distances(j - training_start) = old_distances(j);
		}
		success = distances.WriteBandwise(fp);
		if (success <= 0)
		{
			vPrint("\nfailed to save distances for object %li\n", (long) index);
			fclose(fp);
			return 0;
		}
		vPrint("saved distances for object %li of %li\r", (i+1), (long) training_size);
	}
	vPrint("\n");
	fclose(fp);

	return 1;
}


// this function is useful when we already have an original dataset and 
// we want to create a smaller original dataset. This is similar to 
// OriginalSubdataset, but here we keep the entire test set,
// and a random sample of the training set.
vint8 BoostMap_data::random_subdataset(const char * original, const char * subset,
	vint8 training_size)
{
	if (training_size < 0)
	{
		vPrint("bad arguments: training_size = %li\n",
			(long) training_size);
		return 0;
	}

	vint8 old_test_size = BoostMap_data::TestNumber(original);
	vint8 old_training_size = BoostMap_data::TrainingNumber(original);

	if (old_training_size < training_size)
	{
		vPrint("old_test_size = %li, old_training_size = %li\n", 
			(long) old_test_size, (long) old_training_size);
		return 0;
	}

	// create the directory
	char * directory = BoostMap_data::original_directory(g_data_directory, subset);
	vint8 success = function_make_directory(directory);
	if (success <= 0)
	{
		vPrint("can't create directory %s\n", directory);
		vdelete2(directory);
		return 0;
	}

	vdelete2(directory);

	// create the test labels
	vMatrix<float> old_test_labels = BoostMap_data::LoadTestLabels(g_data_directory, original);
	if (old_test_labels.valid() <= 0)
	{
		vPrint("failed to load test labels\n");
		return 0;
	}

	vMatrix<float> test_labels = old_test_labels;
	success = BoostMap_data::SaveTestLabels(g_data_directory, subset, test_labels);
	if (success <= 0)
	{
		vPrint("failed to save test labels\n");
		return 0;
	}
	else
	{
		vPrint("saved test labels\n");
	}

	// choose the training object
	vint8_matrix training_indices = sample_without_replacement(0, old_training_size - 1, training_size);

	// create the training labels
	vMatrix<float> old_training_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, original);
	if (old_training_labels.valid() <= 0)
	{
		vPrint("failed to load training labels\n");
		return 0;
	}

	vMatrix<float> training_labels(1, training_size);
	vint8 i;
	for (i = 0; i < training_size; i++)
	{
		training_labels(i) = old_training_labels(training_indices(i));
	}
	success = BoostMap_data::SaveTrainingLabels(g_data_directory, subset, training_labels);
	if (success <= 0)
	{
		vPrint("failed to save training labels\n");
		return 0;
	}
	else
	{
		vPrint("saved training labels\n");
	}


	// create the test-to-training distances.

	char * path = BoostMap_data::TestTrainDistancesPath(g_data_directory, subset);
	FILE * fp = fopen(path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", path);
		vdelete2(path);
		return 0;
	}
	vdelete2(path);

	success = vWriteHeader(fp, FloatType, old_test_size, training_size, 1);
	if (success <= 0)
	{
		vPrint("failed to write header\n");
		fclose(fp);
		return 0;
	}

	vPrint("\n");
	for (i = 0; i < old_test_size; i++)
	{
		vint8 index = i;
		vMatrix<float> old_distances = BoostMap_data::TestTrainDistance(original, index);
		vMatrix<float> distances(1, training_size);
		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 training_index = training_indices(j);
			distances(j) = old_distances(training_index);
		}
		success = distances.WriteBandwise(fp);
		if (success <= 0)
		{
			vPrint("\nfailed to save distances for object %li\n", (long) index);
			fclose(fp);
			return 0;
		}
		vPrint("saved distances for object %li of %li\r", (long) (i+1), (long) old_test_size);
	}
	vPrint("\n");
	fclose(fp);

	// create the training-to-training distances.

	path = BoostMap_data::TrainTrainDistancesPath(g_data_directory, subset);
	fp = fopen(path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", path);
		vdelete2(path);
		return 0;
	}
	vdelete2(path);

	success = vWriteHeader(fp, FloatType, training_size, training_size, 1);
	if (success <= 0)
	{
		vPrint("failed to write header\n");
		fclose(fp);
		return 0;
	}

	vPrint("\n");
	for (i = 0; i < training_size; i++)
	{
		vint8 index = training_indices(i);
		vMatrix<float> old_distances = BoostMap_data::TrainTrainDistance(g_data_directory, original, index);
		vMatrix<float> distances(1, training_size);
		vint8 j;
		for (j = 0; j < training_size; j++)
		{
			vint8 second_index = training_indices(j); 
			distances(j) = old_distances(second_index);
		}
		success = distances.WriteBandwise(fp);
		if (success <= 0)
		{
			vPrint("\nfailed to save distances for object %li\n", (long) index);
			fclose(fp);
			return 0;
		}
		vPrint("saved distances for object %li of %li\r", (long) (i+1), (long) training_size);
	}
	vPrint("\n");
	fclose(fp);

	return 1;
}



// labels for the training objects of the training set
// (not the training objects of the original set, which
// are obtained using LoadTrainingLabels());
vint8_matrix BoostMap_data::training_training_labels()
{
	float_matrix training_labels = LoadTrainingLabels(g_data_directory, original_dataset_name);
	vint8 number = training_ids.length();

	vint8_matrix result(1, number);
	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		vint8 index = training_ids(counter);
		vint8 label = round_number(training_labels(index));
		result(counter) = label;
	}

	return result;
}


// labels for the validation objects of the training set
vint8_matrix BoostMap_data::training_validation_labels()
{
	float_matrix training_labels = LoadTrainingLabels(g_data_directory, original_dataset_name);
	vint8 number = validation_ids.length();

	vint8_matrix result(1, number);
	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		vint8 index = validation_ids(counter);
		vint8 label = round_number(training_labels(index));
		result(counter) = label;
	}

	return result;
}

// start of BoostMap_data functions written for a possible SIGMOD 2007 paper.

/*!
	This function actually does the work for embed_database and 
	embed_test_set. "pathname" specifies where from to load the distances
	from set of objects to database. 
	"number" specifies the number of objects that we are going to embed. 
	"database_size" specifies the size
	of the database, i.e. the number of distances to be read
	from pathname for each object. pivot_distances(i) is
	the intrapivot distance for the i-th 1D embedding, i.e.
	the i-th row of classifiers, provided that that embedding
	is a "line projection". classifiers holds, in the i-th
	row, the 1D embedding to be used to obtain the i-th
	coordinate of each object.
*/
float_matrix BoostMap_data::embed_set(const char * pathname, 
	vint8 set_size, const class_embedding * embedding)
{
	vint8 distance_counter = embedding->upper_bound_distances();
	vPrint("upper_bound on distances: %li\n", (long) distance_counter);

	class_file * fp = OpenObjectDistancesFile(pathname);;
	if (fp->file_pointer == 0)
	{
		vPrint("Failed to open %s\n", pathname);
		return vMatrix<float>();
	}

	vMatrix<float> result(set_size, embedding->dimensionality());

	vPrint("\n");
	vint8 counter;
	for (counter = 0; counter < set_size; counter++)
	{
		// compute the embedding of current object.
		vMatrix<float> object_embedding = embedding->embed(fp);
		if (object_embedding.valid() <= 0)
		{
			return float_matrix();
		}

		function_put_row(&object_embedding, &result, counter);
		vPrint("embedded object %li of %li\r", (long) (counter+1), (long) set_size);
	}
	vPrint("\n");

	fclose(fp);
	return result;
}

/*!
	"name" is the name of an entire dataset. The result
	of this function holds, at each row, the embedding of a database object.
	reimplementation of a deprecated previous embed_database, that took as an argument
	float matrix representing the embedding.
*/
float_matrix BoostMap_data::embed_database(const class_embedding * embedding)
{
	// get filename of distances from training set to training set (i.e. from
	// database objects to database objects).
	char * pathname = TrainTrainDistancesPath(g_data_directory, embedding->original_dataset_name);
	vMatrix<float> result = embed_set(pathname, embedding->database_size, embedding);
	vdelete2(pathname);
	return result;
}


// Same as embed_database, but for the test set of the dataset, 
// not for the database (or training set).
// reimplementation of a deprecated previous embed_test_set, that took as an argument
// float matrix representing the embedding.
float_matrix BoostMap_data::embed_test_set(const class_embedding * embedding)
{
	// get filename of distances from training set to training set (i.e. from
	// database objects to database objects).
	char * pathname = TestTrainDistancesPath(g_data_directory, embedding->original_dataset_name);
	vMatrix<float> result = embed_set(pathname, embedding->test_size, embedding);
	vdelete2(pathname);
	return result;
}


// the next functions are useful for computing distances from 
// test or training set to training set according to
// the embedding, and save those distances to a file

char * BoostMap_data::embedding_test_train_file(const char * dataset_name, const char * embedding_name)
{
	char * target_dir = embedding_directory(g_data_directory, dataset_name);

	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = vReplaceEnding2(embedding_name, ".testtrain");
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


char * BoostMap_data::embedding_train_train_file(const char * dataset_name, const char * embedding_name)
{
	char * target_dir = embedding_directory(g_data_directory, dataset_name);
	if (target_dir == 0)
	{
		return 0;
	}

	char * simple_name = vReplaceEnding2(embedding_name, ".traintrain");
	char * result = vJoinPaths(target_dir, simple_name);
	vdelete2(simple_name);
	vdelete2(target_dir);

	return result;
}


vint8 BoostMap_data::embedding_test_train(const char * dataset_name, const char * embedding_name)
{
	// load embedding for the queries and the database
	float_matrix test_embedding = load_test_embedding(dataset_name, embedding_name);
	float_matrix database_embedding = load_training_embedding(dataset_name, embedding_name);

	// get the file where the distances will be saved
	char * output_file = embedding_test_train_file(dataset_name, embedding_name);

	// do the work
	vint8 result = embedding_distances(dataset_name, embedding_name, output_file,
		test_embedding, database_embedding);

	return 1;
}


vint8 BoostMap_data::embedding_train_train(const char * dataset_name, const char * embedding_name)
{
	// load embedding for the database
	float_matrix database_embedding = load_training_embedding(dataset_name, embedding_name);

	// get the file where the distances will be saved
	char * output_file = embedding_train_train_file(dataset_name, embedding_name);

	// do the work
	vint8 result = embedding_distances(dataset_name, embedding_name, output_file,
		database_embedding, database_embedding);

	return 1;
}


// this function does the actual work for embedding_test_train and embedding_train_train.
// the way I anticipate this function to be used, first_embedding 
// is the embedding of either the test subjects or the training (database) objects,
// and second_embedding is the embedding of the database objects.
vint8 BoostMap_data::embedding_distances(const char * dataset_name, const char * embedding_name,
	const char * output_file,
	float_matrix first_embedding, float_matrix second_embedding)
{
	if (first_embedding.valid() <= 0)
	{
		function_print("\ninvalid first embedding\n");
		return 0;
	}

	if (second_embedding.valid() <= 0)
	{
		function_print("\ninvalid second embedding\n");
		return 0;
	}

	class_embedding embedding(dataset_name, embedding_name, 0);
	if (embedding.valid <= 0)
	{
		function_print("\nfailed to load embedding definition from %s\n", embedding_name);
		return 0;
	}

	// try to create output file, return if there is a problem
	file_handle * file_pointer = open_save_file(output_file);
	if (file_pointer == 0)
	{
		function_print("\nfailed to open output file %s\n", output_file);
		return 0;
	}
	function_print("\ncreated output file %s\n", output_file);

	// write the header information into the output file. The header
	// corresponds to matrix with one row per test object
	// and one column per training object.
	vint8 first_size = first_embedding.vertical();
	vint8 second_size = second_embedding.vertical();

	vector<vint8> header(4);
	header[0] = function_type_vint8(FloatType);
	header[1] = first_size;
	header[2] = second_size;
	header[3] = 1;

	float_matrix::store_header(file_pointer, &header);

	// now, for each object in the first embedding, compute and save the 
	// embedding-based distances to all objects in second embedding

	function_print("\n");
	vint8 first_counter;
	for (first_counter = 0; first_counter < first_size; first_counter++)
	{
		function_print("processing object %li of %li\r", (long) (first_counter + 1), (long) first_size);
		float_image current_embedding = copy_horizontal_line(&first_embedding, first_counter);
		float_matrix distances = sensitive_distances(&current_embedding, second_embedding,
			&(embedding.unique_classifiers), 
			&(embedding.sensitive_classifiers));

		vint8 success = distances.store_bandwise(file_pointer);
		if (success <= 0)
		{
			function_print("\nfailed to save distances for first set's object %li\n", (long) first_counter);
			close_file(file_pointer);
			return 0;
		}
	}

	function_print("\n");
	close_file(file_pointer);
	function_print("\nsaved embedding-based distances successfully\n");
	return 1;
}


// cleaned up version of indexerrors, for test set
vint8_matrix BoostMap_data::retrieval_results_test(
	const char * dataset_name, const char * embedding_name,
	vint8 dimensions, vint8 max_neighbors)
{
	// Load and print classifiers.
	// The classifier params stored in the file contain the necessary
	// information to embed points.
	class_embedding embedding(dataset_name, embedding_name, dimensions);

	if (embedding.valid <= 0)
	{
		function_print("\nfailed to load embedding from %s\n", embedding_name);
		return vint8_matrix();
	}

	// Embed database using the 'embedding' parameters
	// Note: the 'embedding' knows it's associated database, which is
	// 'dataset_name'.
	vMatrix<float> embedded_database = embed_database(&embedding);

	if (embedded_database.valid() <= 0)
	{
		vPrint("Failed to embed training set\n");
		return vint8_matrix();
	}

	// Embed test set using the 'embedding' parameters
	// Note: the 'embedding' knows it's associated database, which is
	// 'dataset_name'.
	vMatrix<float> embedded_test_set = embed_test_set(&embedding);
	if (embedded_test_set.valid() <= 0)
	{
		vPrint("Failed to embed test set\n");
		return vint8_matrix();
	}

	// prepare file holding distances from test objects
	// to database.
	char * distances_file = TestTrainDistancesPath(g_data_directory, dataset_name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vint8_matrix();
	}

	// compute the result.
	vint8_matrix result = retrieval_results(embedded_test_set, embedded_database,
		&embedding, fp, 0, max_neighbors);

	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = test_retrieval_pathname(embedding);
	save_retrieval_results(result, output_file);
	vdelete2(output_file);

	return result;
}


vint8_matrix BoostMap_data::retrieval_results_training(const char * original_dataset_name, 
	const char * embedding_name,
	vint8 dimensions, vint8 max_neighbors)
{
	// load and print classifiers
	class_embedding embedding(original_dataset_name, embedding_name, dimensions);
	if (embedding.valid <= 0)
	{
		function_print("\nfailed to load embedding from %s\n", embedding_name);
		return vint8_matrix();
	}

	// embed database.
	vMatrix<float> embedded_database = embed_database(&embedding);
	if (embedded_database.valid() <= 0)
	{
		vPrint("Failed to embed training set\n");
		return vint8_matrix();
	}

	// prepare file holding distances from test objects
	// to database.
	char * distances_file = TrainTrainDistancesPath(g_data_directory, original_dataset_name);
	class_file * fp = OpenObjectDistancesFile(distances_file);
	if (fp->file_pointer == 0)
	{
		vPrint("failed to open distances file %s\n", distances_file);
		vdelete2(distances_file);
		return vint8_matrix();
	}

	// compute the result.
	vint8_matrix result = retrieval_results(embedded_database, embedded_database,
		&embedding, fp, 1, max_neighbors);

	fclose(fp);
	vdelete2(distances_file);

	// save the result.
	char * output_file = training_retrieval_pathname(embedding);
	save_retrieval_results(result, output_file);
	vdelete2(output_file);

	return result;
}

/*!
	Computes retrieval results.

	@param test_set is the embedding of the test set

	@param trainign_flag it is set to 0 wheh this function is called
	       by retrieval_results_test and to 1 when is called by 
	       retrieval_results_training.
*/
vint8_matrix BoostMap_data::retrieval_results(const vMatrix<float> test_set, 
	const vMatrix<float> training_set,
	const class_embedding * embedding, class_file * test_fp, 
	const vint8 training_flag, const vint8 max_neighbors)
{
	// Perform sanity checking on the test and training embeddings
	vint8 validity = check_embedding_validity(test_set, training_set, test_fp);

	if (validity <= 0)
	{
		return vint8_matrix();
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();

	// Create a structure to store all stats
	retrieval_statistics statistics(max_neighbors, test_number, training_number);

	vint8 counter;
	vPrint("\n");

	// For each training object
	for (counter = 0; counter < test_number; counter++)
	{
		// Get the embedding parameters associated with the object
		v3dMatrix<float> object = copy_horizontal_line(&test_set, counter);

		// load the entire row of distances from the file test_fp. 
		// They are the object-to-database distances, which are 
		// based on original distance measure.
		vMatrix<float> original_distances = NextObjectDistances(test_fp, 
			training_number);

		if (original_distances.valid() <= 0)
		{
			vPrint("\nerror: indexerror7, failed to read object distances for %li\n",
				(long) counter);
		}

		// get embedding-based distances from object to database.
		vMatrix<float> approximate_distances = SL1Distances(&object, training_set, 
			&(embedding->unique_classifiers), 
			&(embedding->sensitive_classifiers));

		if (training_flag == 1)
		{
			float max_distance;
			max_distance = vAbs(function_image_maximum(&approximate_distances));
			approximate_distances(counter) = max_distance * ((float) 2.0) + (float) 1.0;

			max_distance = vAbs(function_image_maximum(&original_distances));
			original_distances(counter) = max_distance * ((float) 2.0) + (float) 1.0;
		}

		// get retrieval results for the object
		object_retrieval_result(original_distances, approximate_distances, max_neighbors, & statistics);
		statistics.process_object_result();

		// print some temporary information
		print_retrieval_statistics(statistics);
	}
	vPrint("\n");

	// compute and print some average retrieval statistics.
	statistics.error_averages();

	statistics.rank_errors.print("rank_results", "%10.1f ");
	function_print("triple error rate for k = 1, ..., %li: %f\n", 
		(long) max_neighbors, statistics.global_error);

	return statistics.result;
}

/*!
	Gets retrieval results for ONE object.

	@param original_distances *Row* of the full distance matrix corresponding
	to ONE object.

	@param embedding_distances distances in the embeding between the query 
	object and all other objects.
*/
vint8 BoostMap_data::object_retrieval_result(const float_matrix & original_distances,
	const float_matrix & embedding_distances,
	const vint8 max_neighbors, 
	retrieval_statistics * statistics)
{
	if ((original_distances.valid() <= 0) || 
		(embedding_distances.valid() <= 0) || 
		(original_distances.length() != embedding_distances.length()))
	{
		vPrint("\nerror: object_retrieval_result, invalid inputs\n");
	}

	vint8 & global_counter = statistics->global_counter;
	vint8 & global_error_counter = statistics->global_error_counter;
	class_pointer(vint8) rank_counters = statistics->rank_error_counters.flat();
	class_pointer(vint8) max_counters = statistics->max_counters.flat();

	// Find the max_neighbors nearest neighbors (based on original 
	// distance measure).
	vint8 junk = 0, nn_index = 0;
	vector<vint8> nn_indices((long) max_neighbors, -1);
	vector<float> original_vector;
	vector<vint8> index_ranks;

	// See the matrix of distances as a vector
	vector_from_matrix(&original_distances, &original_vector);

	// Find the position of each elements in 'original_vector' that results from
	// sorting 'original_vector'. ie, index_ranks[i] = j means that elements 
	// 'i' is in position 'j' when 'original_vector' is sorted.
	vIndexRanks(&original_vector, &index_ranks);

	// Note that 'index_ranks' is the result of sorting ALL distances
	// for the query object

	vint8 training_number = original_distances.length();
	vint8 j;

	// For ALL pairweise distances...
	for (j = 0; j < training_number; j++)
	{
		// See if the "assignment" is within the
		// NN threshold, and if it is, fill in the
		// 'nn_indices' at the appropriate ranking position
		// with the "id" of the element (the index over the *flaten*
		// distance matrix)
		if (index_ranks[(vector_size) j] < max_neighbors)
		{
			vint8 rank = index_ranks[(vector_size) j];
			nn_indices[(vector_size) rank] = j;
		}
	}

	// Since ALL true distances to the object were processed, 
	// the 'nn_indices' array will be complete

	// Now we need the distances in the embedding between query 
	// and each of its TRUE k-th nearest neighbors
	vector<float> embedded_distances((vector_size) max_neighbors);

	// And the max distance in the embedding between query and 
	// any of its TRUE k-nearest neighbors
	vector<float> max_distances((vector_size) max_neighbors);

	// get embedding-based distances for max_neighbors nearest neighbors.
	for (j = 0; j < max_neighbors; j++)
	{
		// get the j'th TRUE nearest neighbor
		vint8 index = nn_indices[(vector_size) j];

		if (index == -1)
		{
			exit_error("error: index = -1\n");
		}

		// Find the distance in the embedding between
		// the current object and the true j'th narest neighbour
		float distance = embedding_distances(index);

		// Save the embeding distance retrieved
		embedded_distances[(vector_size) j] = distance;

		// this is an array in retrieval_statistics and
		// don't know how it is initialized
		max_counters[(vector_size) j] = 0;
	}

	// for each k in 1, ..., max_neighbors, find the 
	// largest embedding-based distance attained
	// by one of the k nearest neighbors.

	// The 0-th max distance is the embedding distance 
	// of the first true nearest neighbor
	max_distances[0] = embedded_distances[0];

	for (j = 1; j < max_neighbors; j++)
	{
		float previous = max_distances[(vector_size) j - 1];
		float current = embedded_distances[(vector_size) j];
		
		// The j-h max distance is the maximum embedding distance 
		// of any true NN between 0 and j.
		max_distances[(vector_size) j] = Max(previous, current);
	}

	// now, using the embedding distances, find, for each k,
	// the worst rank among the k nearest neighbor of the object,
	// when ranks are computed based on the embedding.

	// For ALL trainign objects
	for (j = 0; j < training_number; j++)
	{
		float distance = embedding_distances(j);
		vint8 k;

		for (k = 0; k < max_neighbors; k++)
		{
			// Compare embedding distances for each training object
			// and each TRUE nearest neighbors. 
			
			// Increment the counter of a true NN if, according to
			// the embedding, there is a training object that comes before
			// the true NN
			if (distance < max_distances[(vector_size) k])
			{
				//max_counters[(vector_size) k] = max_counters[(vector_size) k] + 1; // Diego
				max_counters[(vector_size) k]++;
			}
		}

		// See if the j'th trainign object is actually a nearest neighbor
		if (index_ranks[(vector_size) j] < max_neighbors)
		{
			continue; // j is an NN
		}

		for (k = 0; k < max_neighbors; k++)
		{
			//global_counter = global_counter + 1; // Diego
			global_counter++;

			// j is not an NN, so if its distance in th eembedding is
			// closer than a true NN, we increase the rank counter of
			// the k-th NN that showed us this, and also increase the 
			// global error counter
			if (distance < embedded_distances[(vector_size) k])
			{
				//rank_counters[k] = rank_counters[(vector_size) k] + 1; // Diego
				rank_counters[k]++;

				//global_error_counter = global_error_counter + 1; // Diego
				global_error_counter++;
			}
		}
	}

	return 1;
}


vint8 BoostMap_data::print_retrieval_statistics(const retrieval_statistics & statistics)
{
	vPrint("%li of %li, max:%8li-%8li, mean:%10.2f-%10.2f, now:%8li-%8li\n", 
		(long) statistics.object_counter, (long) statistics.test_number, 
		(long) statistics.worst_result_first, (long) statistics.worst_result_last, 
		statistics.ranks_mean_first, statistics.ranks_mean_last, 
		(long) statistics.max_counters(0), (long) statistics.max_counters(statistics.max_neighbors - 1));

	return 1;
}


// an auxiliary function used to perform some sanity checking on
// the test and training embeddings
vint8 BoostMap_data::check_embedding_validity(float_matrix test_set, float_matrix training_set,
	class_file * test_file_handle)
{
	if ((test_set.valid() <= 0) || (training_set.valid() <= 0) ||
		(test_file_handle->file_pointer == 0))
	{
		return 0;
	}

	vint8 test_number = test_set.Rows();
	vint8 training_number = training_set.Rows();
	if ((test_number == 0) || (training_number == 0) ||
		(test_set.Cols() != training_set.Cols()))
	{
		exit_error("Error: test_number = %li, training = %li\n",
			(long) test_number, (long) training_number);
	}

	return 1;
}


// filenames where we should store retrieval results for the test set and the training set
// replacing IndexErrorOutputPath2
char * BoostMap_data::test_retrieval_pathname(const class_embedding & embedding)
{
	char * dimension_string = string_from_number(embedding.dimensionality());
	char * filename = vMergeStrings5(embedding.original_dataset_name, "_retrieval_", 
		embedding.embedding_name, "_", dimension_string);
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "retrieval");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		function_print("error: cannot save results to file, need to create directory %s.\n",
			directory);
		delete_pointer(pathname);
		pathname = 0;
	}

	vdelete2(dimension_string);
	vdelete2(filename);
	vdelete2(directory);
	return pathname;
}


char * BoostMap_data::training_retrieval_pathname(const class_embedding & embedding)
{
	char * dimension_string = string_from_number(embedding.dimensionality());
	char * filename = vMergeStrings5(embedding.original_dataset_name, "_training_retrieval_", 
		embedding.embedding_name, "_", dimension_string);
	char * directory = vJoinPaths4(g_data_directory, "experiments", 
		"embedding_statistics", "retrieval");
	char * pathname = vJoinPaths(directory, filename);

	if (vDirectoryExists(directory) <= 0)
	{
		function_print("error: cannot save results to file, need to create directory %s.\n",
			directory);
		delete_pointer(pathname);
		pathname = 0;
	}

	vdelete2(dimension_string);
	vdelete2(filename);
	vdelete2(directory);
	return pathname;
}

vint8 BoostMap_data::save_retrieval_results(const vint8_matrix result, const char * output_file)
{
	if (output_file == 0)
	{
		return 0;
	}

	integer_matrix converted(& result);
	vint8 success = converted.Write(output_file);
	if (success <= 0)
	{
		vPrint("failed to write result to %s\n", output_file);
	}
	else
	{
		vPrint("saved result to %s\n", output_file);
	}

	return success;
}


// vint8 nearest neighbor retrieval results for the test set.
// if the results do not contain enough information for at least
// retrieval of max_neighbors, we return an empty matrix.
// however, if max_neighbors is less than or equal to zero
// then we ignore that argument
vint8_matrix BoostMap_data::load_results_test(const class_embedding & embedding, vint8 max_neighbors)
{
	char * pathname = test_retrieval_pathname(embedding);
	vint8_matrix result = load_retrieval_results(pathname, max_neighbors);
	delete_pointer(pathname);

	if (result.vertical() != BoostMap_data::test_size(embedding.original_dataset_name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::test_size(embedding.original_dataset_name),
			(long) result.vertical());
		return vint8_matrix();
	}

	return result;
}


// vint8 nearest neighbor retrieval results for the training set.
// if the results do not contain enough information for at least
// retrieval of max_neighbors, we return an empty matrix.
// however, if max_neighbors is less than or equal to zero
// then we ignore that argument
vint8_matrix BoostMap_data::load_results_training(const class_embedding & embedding, const vint8 max_neighbors)
{
	char * pathname = training_retrieval_pathname(embedding);
	vint8_matrix result = load_retrieval_results(pathname, max_neighbors);
	delete_pointer(pathname);

	if (result.vertical() != BoostMap_data::database_size(embedding.original_dataset_name))
	{
		function_print("expected %li results, read %li results\n",
			(long) BoostMap_data::database_size(embedding.original_dataset_name),
			(long) result.vertical());
		return vint8_matrix();
	}

	return result;
}


vint8_matrix BoostMap_data::load_retrieval_results(const char * pathname, const vint8 max_neighbors)
{
	integer_matrix integer_result = integer_matrix::read(pathname);
	if (integer_result.valid() <= 0)
	{
		function_print("failed to read retrieval results from %s\n", pathname);
		return vint8_matrix();
	}

	if ((max_neighbors > 0) && (integer_result.vertical()  <= max_neighbors))
	{
		function_print("results only contain information for %li neighbors",
			(long) integer_result.vertical());
		return vint8_matrix();
	}

	vint8_matrix result(&integer_result);
	return result;
}


vint8 BoostMap_data::database_size(const char * original_dataset_name)
{
	vMatrix<float> labels = LoadTrainingLabels(g_data_directory, original_dataset_name);
	vint8 number = labels.Size();

	return number;
}


vint8 BoostMap_data::test_size(const char * original_dataset_name)
{
	vMatrix<float> labels = LoadTestLabels(g_data_directory, original_dataset_name);
	vint8 number = labels.Size();

	return number;
}

// end of BoostMap_data functions written for a possible SIGMOD 2007 paper.
