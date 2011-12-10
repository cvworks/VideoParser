#pragma once

#include "boost_map.h"
#include "class_sensitive_classifier.h"
#include "class_embedding.h"
#include "retrieval_statistics.h"

/*!
	Class BoostMap_data stores enough information for 
	BoostMap to run on a particular dataset. Most of this information 
	will be saved on disk, and this class will provide a convenient 
	interface for reading and writing from/to disk relevant information.

	Note that there are two types of functions, static and non-static.
	There are also two types of datasets, training datasets and original
	datasets. Most static functions are used to access the original data set.
	This can be illustrated through an
	example: we want to run BoostMap on the MNIST dataset. From the 
	MNIST dataset, we will create a smaller dataset, that specifies
	candidate objects, training objects and validation objects. That
	is an example of a training dataset, and MNIST is an example
	of an entire dataset. We can make many different training datasets
	based on MNIST, with different sizes (that will affect the 
	memory requirements and training time for BoostMap).
*/
class BoostMap_data
{
protected:
	/*!
	Training_name is the name of the directory where we store all the matrices 
	needed for boostmap training (for this particular dataset). Note that
	for a given dataset (like MNIST) we will have multiple BoostMap_data
	objects, with different names. We play with different datasets because
	some of them are smaller, so training on them can be much faster, and
	because we want to see how much the random choices we make in 
	constructing each dataset affect the result.
	*/
	char * training_name;

	/*! 
	Original_name is the name of the full dataset, where we store the complete
	distance matrices between all training objects and between
	each query object and each training object.
	*/
	const char * original_dataset_name;

	const char * root_data_dir;

	// number of objects used to define candidate 1D embeddings
	vint8 candidate_number;

	// number of objects used to define training triples
	vint8 training_number;

	// number of objects used to define validation triples
	vint8 validation_number;

	vint8 is_valid;

	// the ids matrices should not be confused with the indices matrices
	// used in BoostMap. The ids matrices hold information that maps the
	// objects represented here to the actual objects in the datasets. 
	// The reason is that, for many datasets, there are too many objects,
	// and we only include a sampling of those objects for candidates,
	// training and validation. The ids matrices map the samples to their
	// original indices in the dataset.
	vMatrix<vint8> candidate_ids;
	vMatrix<vint8> training_ids;
	vMatrix<vint8> validation_ids;

	vint8 zero();
	vint8 initialize();
	vint8 clean_up();


public:
	// This constructor initializes a BoostMap_dataset object when
	// the named dataset has already been created.
	BoostMap_data(const char * root_dir, const char * in_training_name);

	// This constructor initializes a BoostMap_data object and
	// creates the named dataset on disk.
	BoostMap_data(const char * root_dir, const char * in_training_name, 
		vMatrix<vint8> in_candidates, vMatrix<vint8> in_training, 
		vMatrix<vint8> in_validation);

	// This constructor initializes a BoostMap_data object and
	// creates the named dataset on disk. Here we create 
	// the matrices for candidates, training and validation,
	// by choosing randomly mutually disjoint subsets
	// of the set {0, 1, ..., number-1}
	BoostMap_data(const char * root_dir, const char * in_training_name, 
		const char * in_original_name,
		vint8 number, vint8 in_candidate_number, 
		vint8 in_training_number, vint8 in_validation_number);

	// This constructor initializes a BoostMap_data object,
	// creates the named dataset on disk, including all distances
	// and class_distances.
	BoostMap_data(const char * root_dir, const char * in_original_name, 
		const char * in_training_name, vint8 in_candidate_number, 
		vint8 in_training_number, vint8 in_validation_number);

	// This constructor reads distances from the directory where
	// an original dataset is stored, and based on those distances
	// it creates all the files that are needed for boostmap
	// to be run.
	BoostMap_data(const char * root_dir, const char * in_original_name, 
		const char * in_training_name, 
		vint8 candidate_start, vint8 candidate_end, 
		vint8 training_start, vint8 training_end,
		vint8 validation_start, vint8 validation_end);

	// this constructor was created explicitly for the protein set,
	// where half the training (and validation) objects come from
	// the query set, and half come from the database.
	BoostMap_data(const char * root_dir, const char * in_original_name, 
		const char * in_training_name, 
		vint8 candidate_start, vint8 candidate_end, 
		vint8 training_start1, vint8 training_end1, vint8 training_start2,
		vint8 validation_start1, vint8 validation_end1, vint8 validation_start2);

	~BoostMap_data();


	// Set candidate or training or validation ids to the given matrix
	vint8 SetCandidates(vMatrix<vint8> in_matrix);
	vint8 SetTraining(vMatrix<vint8> in_matrix);
	vint8 SetValidation(vMatrix<vint8> in_matrix);

	// Set all of candidate_ids, training_ids, validation_ids.
	vint8 SetIds(vMatrix<vint8> in_candidates, vMatrix<vint8> in_training,
		vMatrix<vint8> in_validation);

	// compute all the matrices holding distances that we need to
	// train and evaluate BoostMap, and save those matrices to file.
	vint8 MakeAllDistances(vMatrix<float> distances);

	// here the original distances of the entire training set to 
	// itself are read from a file, as opposed
	// to being passed in as an argument.
	vint8 MakeAllDistances(const char * original_dataset_name, vint8 strict = 1);

	// in general, in this code, pdistances are distances in parameter
	// space (aka state space). MakeAllPdistances computes and saves
	// all necessary matrices storing parameter distancse.
	vint8 MakeAllPdistances(vMatrix<float> pdistances);

	// here the original distances of the entire training set to 
	// itself are read from a file, as opposed
	// to being passed in as an argument.
	vint8 MakeAllPdistances(const char * original_dataset_name);

	// the next functions compute specific distance matrics and pdistance
	// matrices, and store them to disk.
	vMatrix<float> MakeCandCandDistances(vMatrix<float> distances);
	vMatrix<float> MakeCandTrainDistances(vMatrix<float> distances);
	vMatrix<float> MakeCandValDistances(vMatrix<float> distances);
	vMatrix<float> MakeTrainingDistances(vMatrix<float> distances);
	vMatrix<float> MakeTrainingPdistances(vMatrix<float> distances);
	vMatrix<float> MakeValidationDistances(vMatrix<float> distances);
	vMatrix<float> MakeValidationPdistances(vMatrix<float> distances);

	/*!
		MakeDistances does a lot of the work for the preceding functions.
		It takes in a big distance matrix (distances), and two
		sets of indices. It computes a matrix storing distances from
		all objects described in set1 to all objects described in 
		set2. Essentially, each pair of objects in set1 x set2 defines
		a reference to an entry in distances, and we copy that entry
		into the result matrix.
	*/
	vMatrix<float> MakeDistances(vMatrix<vint8> set1, vMatrix<vint8> set2,
		vint8 strict = 1);

	vMatrix<float> MakePdistances(vMatrix<vint8> set1, vMatrix<vint8> set2);

	vMatrix<float> MakeCandCandDistances();
	vMatrix<float> MakeCandTrainDistances();
	vMatrix<float> MakeCandValDistances();
	vMatrix<float> MakeTrainingDistances(vint8 strict = 1);
	vMatrix<float> MakeTrainingPdistances();
	vMatrix<float> MakeValidationDistances(vint8 strict = 1);
	vMatrix<float> MakeValidationPdistances();

	/*!
	MakeDistances does a lot of the work for the preceding functions.
	It takes in a big distance matrix (distances), and two
	sets of indices. It computes a matrix storing distances from
	all objects described in set1 to all objects described in 
	set2. Essentially, each pair of objects in set1 x set2 defines
	a reference to an entry in distances, and we copy that entry
	into the result matrix.
	*/
	vMatrix<float> MakeDistances(vMatrix<float> distances, 
		vMatrix<vint8> set1, vMatrix<vint8> set2);

	// CheckValidity checks for negative distances, distances in 
	// off-diagonal elements that are equal to zero, and distances 
	// in diagonal elements that are not zero.
	vint8 CheckValidity();

	inline const char * get_original_name()
	{
		return original_dataset_name;
	}

	inline const char * get_training_name()
	{
		return training_name;
	}

	inline vint8 CandidateNumber()
	{
		return candidate_number;
	}

	inline vint8 TrainingNumber()
	{
		return training_number;
	}

	inline vint8 ValidationNumber()
	{
		return validation_number;
	}

	inline vint8_matrix candidate_objects()
	{
		return vint8_matrix(&candidate_ids);
	}

	inline vint8_matrix training_objects()
	{
		return vint8_matrix(&training_ids);
	}

	inline vint8_matrix validation_objects()
	{
		return vint8_matrix(&validation_ids);
	}

	// returns a concatenation of training ids and validation ids.
	vint8_matrix TrainingValidationIds();

	inline vint8 valid()
	{
		return is_valid;
	}

	// returns a complete path name for the directory where
	// information about the original dataset is stored.
	char * original_directory();

	static std::string make_data_path(const std::string& rootDir, 
		const std::string& datasetName);

	// a static version of original_directory()
	static char * original_directory(const char* rootDir, 
		const char * original_dataset_name);

	// converts a simple filename into a pathname, by adding
	// the result of original_directory() in front of it.
	char * original_pathname(const char * simple_name);

	// returns a complete path name for the directory where
	// information about the training dataset is stored.
	char * training_directory();

	// a static version of training_directory()
	static char * training_directory(const char * root_data_dir, 
		const char * training_name);

	// converts a simple filename into a pathname, by adding
	// the result of training_directory() in front of it.
	char * training_pathname(const char * simple_name);

	// returns a complete path name for the directory where
	// we store actual embeddings of database objects and test objects
	char * embedding_directory();

	// returns a complete path name for the directory where
	// we store actual embeddings of database objects and test objects
	// using an alternate dataset (ie, one derived from the original dataset)
	char * embedding_directory(const char * dataset_name);

	// a static version of embedding_directory()
	static char * embedding_directory(const char * root_data_dir, 
		const char * original_dataset_name);

	// converts a simple filename into a pathname, by adding
	// the result of embedding_directory() in front of it.
	char * embedding_pathname(const char * simple_name);

	// returns a complete path name for the directory where
	// we store actual cascades of database objects and test objects
	char * cascade_directory();

	// a static version of cascade_directory()
	static char * cascade_directory(const char * root_data_dir, 
		const char * original_dataset_name);

	// converts a simple filename into a pathname, by adding
	// the result of cascade_directory() in front of it.
	char * cascade_pathname(const char * simple_name);

	// calls SaveHeader, MakeAllDistances, MakeAllPdistances.
	vint8 SaveAll(const char * original_dataset_name);

	// header refers to the member variables (excluding name).
	vint8 SaveHeader();
	vint8 LoadHeader();
	char * HeaderName();
	char * HeaderPath();

	// CandCandDistances are distances from i-th candidate to j-th
	// candidate (useful for line projections).
	vint8 SaveCandCandDistances(vMatrix<float> distances);
	vMatrix<float> LoadCandCandDistances();
	char * CandCandDistancesName();
	char * CandCandDistancesPath();

	// CandTrainDistances are distances from i-th candidate to j-th
	// training object.
	vint8 SaveCandTrainDistances(vMatrix<float> distances);
	vMatrix<float> LoadCandTrainDistances();
	char * CandTrainDistancesName();
	char * CandTrainDistancesPath();

	// CandValDistances are distances from i-th candidate to j-th
	// validation object.
	vint8 SaveCandValDistances(vMatrix<float> distances);
	vMatrix<float> LoadCandValDistances();
	char * CandValDistancesName();
	char * CandValDistancesPath();

	// training distances refer to distances from training to training.
	vint8 SaveTrainingDistances(vMatrix<float> distances);
	vMatrix<float> LoadTrainingDistances();
	char * TrainingDistancesName();
	char * TrainingDistancesPath();

	// validation distances refer to distances from validation to validation.
	vint8 SaveValidationDistances(vMatrix<float> distances);
	vMatrix<float> LoadValidationDistances();
	char * ValidationDistancesName();
	char * ValidationDistancesPath();

	// Pdistances refer to distances in parameter space.
	vint8 SaveTrainingPdistances(vMatrix<float> Pdistances);
	vMatrix<float> LoadTrainingPdistances();
	char * TrainingPdistancesName();
	char * TrainingPdistancesPath();

	vint8 SaveValidationPdistances(vMatrix<float> Pdistances);
	vMatrix<float> LoadValidationPdistances();
	char * ValidationPdistancesName();
	char * ValidationPdistancesPath();

	// Number of training objects in the original dataset.
	static vint8 TrainingNumber(const char * original_dataset_name);

	// Number of test objects in the original dataset.
	static vint8 TestNumber(const char * original_dataset_name);

	// save all the training labels (i.e. class labels for all objects
	// of the training set). typically we call this
	// function once for an entire dataset (say the MNIST database),
	// and not for a specific BoostMap_data. The same is true
	// for most of these static functions.
	static vint8 SaveTrainingLabels(const char * rootDir, const char * dataset_name,
		vMatrix<float> labels);

	vMatrix<float> LoadTrainingLabels() const
	{
		return LoadTrainingLabels(root_data_dir, original_dataset_name);
	}

	static vMatrix<float> LoadTrainingLabels(const char * rootDir, const char * dataset_name);

	char * TrainingLabelsPath() const
	{
		TrainingLabelsPath(root_data_dir, original_dataset_name);
	}

	static char * TrainingLabelsName();
	static char * TrainingLabelsPath(const char * rootDir, const char * dataset_name);

	vint8 SaveTestLabels(vMatrix<float> labels) const
	{
		return SaveTestLabels(root_data_dir, original_dataset_name, labels);
	}

	// save the class labels of all objects of the test set
	static vint8 SaveTestLabels(const char * rootDir, const char * dataset_name,
		vMatrix<float> labels);
	
	vMatrix<float> LoadTestLabels() const
	{
		return LoadTestLabels(root_data_dir, original_dataset_name);
	}

	static vMatrix<float> LoadTestLabels(const char * rootDir, const char * dataset_name);

	static char * TestLabelsName();

	char * TestLabelsPath() const
	{
		return TestLabelsPath(root_data_dir, original_dataset_name);
	}

	static char * TestLabelsPath(const char * rootDir, const char * dataset_name);

	// interfaces for accessing distances from the entire test set
	// to the entire training set, or from the entire training
	// set to the entire training set.
	char * TestTrainDistancesPath();

	static char * TestTrainDistancesName();
	static char * TestTrainDistancesPath(const char * rootDir, const char * dataset_name);

	// for the protein dataset
	static char * RefTestDistancesName();
	static char * RefTestDistancesPath(const char * directory);
	static vMatrix<float> TestTrainDistance(const char * directory, 
		vint8 index);

	char * TrainTrainDistancesPath();

	static char * TrainTrainDistancesName();
	static char * TrainTrainDistancesPath(const char * rootDir, const char * dataset_name);
	static vMatrix<float> TrainTrainDistance(const char * rootDir, const char * dataset_name, 
		vint8 index);

	static float test_train_distance(const char * original_dataset_name, const vint8 first, 
		const vint8 second, vint8 * success_pointer);
	static float train_train_distance(const char * original_dataset_name, const vint8 first, 
		const vint8 second, vint8 * success_pointer);


	// opens the appropriate file, and reads the header, so that
	// we can then start calling NextObjectDistances(). Not having
	// such a function caused a bug that took me a whole day
	// (Friday 02/20/2004) to fix, wasting one day of work for KDD
	// deadline (02/27/2004).
	static class_file * OpenObjectDistancesFile(const char * filename);
	static vMatrix<float> ObjectDistances(const char * filename,
		vint8 index);
	static vMatrix<float> ObjectDistances3(class_file * fp,
		vint8 index, vint8 number);

	// returns distances from some object (the next one to be read)
	// to the entire database (i.e. the entire training set).
	static vMatrix<float> NextObjectDistances(class_file * fp, vint8 number);

	vint8 Print();
	vint8 PrintCandidates();
	vint8 PrintTraining();
	vint8 PrintValidation();
	vint8 PrintDistance(vint8 index1, vint8 index2);

	// here we choose triples randomly.
	vint8_matrix MakeTrainingTriples(vint8 number);
	vint8_matrix MakeValidationTriples(vint8 number);

	// this function actually chooses the triples. Since we
	// choose the triples entirely randomly, we don't need to
	// know any distances or pdistances.
	vint8_matrix MakeTriples(vint8 number, vint8 number_of_objects);

	// here we choose the top k same-class nearest neighbors for an object,
	// and the top (classes-1)*k different-class nearest neighbors, and
	// make triples based on that. If k = -1, then we choose k
	// automatically, based on the number of triples we need to form per
	// object.
	vint8_matrix MakeTrainingTriples2(vint8 number, vint8 classes, vint8 k);
	vint8_matrix MakeValidationTriples2(vint8 number, vint8 classes, vint8 k);
	vint8_matrix MakeTriples2(vint8 number, vint8 classes, vint8 k,
		vMatrix<float> distancesm, 
		vMatrix<float> pdistancesm);


	// right now there is no difference between these functions and
	// the "Triples2" functions.
	vint8_matrix MakeTrainingTriples3(vint8 number, vint8 k);
	vint8_matrix MakeValidationTriples3(vint8 number, vint8 k);
	vint8_matrix MakeTriples3(vint8 number, vint8 k,
		vMatrix<float> distancesm, 
		vMatrix<float> pdistancesm);

	// Here we give rank ranges for a and b with respect to q.
	vint8_matrix MakeTrainingTriples4(vint8 number, vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b);
	vint8_matrix MakeValidationTriples4(vint8 number, vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b);
	vint8_matrix MakeTriples4(vint8 number,
		vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b,
		vMatrix<float> distancesm);

	// These functions were custom made for the protein dataset, where
	// triples (q, a, b) are made so that q comes from a query dataset
	// and a, b, come from the database.
	vint8_matrix MakeTrainingTriples5(vint8 number, vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b);
	vint8_matrix MakeValidationTriples5(vint8 number, vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b);
	vint8_matrix MakeTriples5(vint8 number_of_triples, 
		vint8 min_a, vint8 max_a,
		vint8 min_b, vint8 max_b,
		vMatrix<float> distancesm);

	// this function chooses pairs of objects to be used for optimizing
	// an embedding for stress and distortion. Since we
	// choose the pairs entirely randomly, we don't need to
	// know any distances or pdistances.
	vint8_matrix make_couples(vint8 number, vint8 number_of_objects);
	float_matrix couple_distances(vint8_matrix couples, float_matrix distances);
	float_matrix training_couple_distances(vint8_matrix couples);
	float_matrix validation_couple_distances(vint8_matrix couples);


	// SameClassKnn is given, for some object, its distances and
	// pdistances to a set of other objects. We select the k-nearest
	// neighbors (based on distances) such that their pdistances are 0.
	static vint8_matrix SameClassKnn(v3dMatrix<float> distancesm, 
		v3dMatrix<float> pdistancesm, vint8 k);
	static vint8_matrix OtherClassKnn(v3dMatrix<float> distancesm, 
		v3dMatrix<float> pdistancesm, vint8 k);

	// this is just an auxiliary function that does the work for
	// SameClassKnn and OtherClassKnn
	static vint8_matrix PdistanceClassKnn(v3dMatrix<float> distancesm,
		v3dMatrix<float> pdistancesm, vint8 k,
		float p_distance);

	// we get the training or validation distances or pdistances
	// for the specified triples.
	// If the i-th triple is of form (q, a, b), result(i) is
	// D(q, b) - D(q, a). 
	vMatrix<float> TripleTrainingDistances(vint8_matrix triples);
	vMatrix<float> TripleTrainingPdistances(vint8_matrix triples);
	vMatrix<float> TripleValidationDistances(vint8_matrix triples);
	vMatrix<float> TripleValidationPdistances(vint8_matrix triples);

	// TripleDistances does the work for the four preceding functions.
	vMatrix<float> TripleDistances(vint8_matrix triples, 
		vMatrix<float> distances);

	// name: the name of a dataset (like "mnist"). Note that here we are
	// using the type of name we would use in the static functions like
	// ObjectDistances, i.e. a name specifying a directory where we 
	// store UNSAMPLED information. We don' use a name like in 
	// the BoostMap_data constructor, since that name specifies a directory
	// where we save a particular sampled subset of the training set.
	// Index: the index of a training or test object in that dataset.
	// test_flag: 0 if index refers to a training object, 1 if 
	// it refers to a test object. result(i, 0) is the overall rank of the
	// i-class k-nearest neighbor of the object. 
	//
	// result(i, 1) is the actual
	// distance of the i-class k-nearest neighbor of the object to the 
	// object.
	static vMatrix<float> WknnRanks4(const char * name, vint8 index, 
		vint8 test_flag, vint8 k);

	// here distances is the distances from an object to all training
	// objects, and labels are training labels.
	static vMatrix<float> WknnRanks3(vMatrix<float> distances, 
		vMatrix<float> labels, vint8 k);

	// Here object is the embedding of an object, database is the
	// embedding of all database objects, weights specifies the
	// weight to be used in each dimension (for the weighted L1
	// distance), and labels are the training labels.
	static vMatrix<float> WknnRanks5(vMatrix<float> object, 
		vMatrix<float> database,
		vMatrix<float> weights,
		vMatrix<float> labels, vint8 k);

	// result(k-1, 0) is the index of the k-nearest neighor of object specified
	// by index. result(k-1, 1) is the distance of that neighbor to the 
	// specified object.
	static vMatrix<float> FindKnn4(const char * name, vint8 index, 
		vint8 test_flag, vint8 k);

	// here instead of passing name, index and test_flag, we just pass the
	// distances of the object to all training objects.
	static vMatrix<float> FindKnn2(vMatrix<float> distances, vint8 k);


	// returns the knn-error in a dataset, using the testtrain distances
	// saved for that dataset, for k = 1, ..., max_k. result(0) does
	// not have a meaningful value.
	// NOTE: this is the error using the original distances, not
	// the embeddings (unless testtrain distancse stores
	// a distance based on embeddings, which doesn't happen so far).
	static vMatrix<float> KnnError2(const char * name, vint8 max_k);

	// returns the training knn-error in a dataset, 
	// using the traintrain distances
	// saved for that dataset, for k = 1, ..., max_k.
	// NOTE: this is the error using the original distances, not
	// the embeddings (unless traintrain distancse stores
	// a distance based on embeddings, which doesn't happen so far).
	static vMatrix<float> KnnTrainError2(const char * name, vint8 max_k);

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
	// NOTE: here we measure distance based on embeddings, not
	// based on original distance measure.
	static vMatrix<float> KnnError7(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vMatrix<float> weights,
		vMatrix<float> test_labels,
		vMatrix<float> training_labels,
		vint8 max_k, vint8 training_flag);

	// Same as KnnError7, but here the distance is a weighted L2
	// (Euclidean) distance, as opposed to an L1 distance.
	static vMatrix<float> KnnError7L2(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vMatrix<float> weights,
		vMatrix<float> test_labels,
		vMatrix<float> training_labels,
		vint8 max_k, vint8 training_flag);

	// distances are the distances from some test object to all
	// training objects. label is the class label of the test object.
	// labels are the training labels. result(i) is the i-nn label
	// of the object (i.e. its labels using i-nn classification).
	static vMatrix<float> KnnLabel4(vMatrix<float> distances,
		vMatrix<float> labels, vint8 label,
		vint8 max_k);

	// object is the embedding (or vector representation) of some
	// object. label is the class label of the test object.
	// labels are the training labels. result(i) is the i-nn label
	// of the object (i.e. its labels using i-nn classification).
	static vMatrix<float> KnnLabel6(vMatrix<float> object,
		vMatrix<float> training_set,
		vMatrix<float> weights,
		vMatrix<float> training_labels,
		vint8 label, vint8 max_k);


	// distances(i) is the distance of query object to the i-th element
	// of a set of objects (typically the training set). labels(i) is the
	// class label of the i-th object. result[w] contains a vector,
	// which will receive all distances between the query and objects
	// of class w. Remember that class id 0 is not used, so result[0]
	// will not get any distances. The distances are stored into
	// result[i] as pairs, storing the distance and the index of the 
	// object in its set.
	static vint8 SplitDistances(vMatrix<float> distances, vMatrix<float> labels,
		vector<vector<class_couple> > * result);

	// the input "distances" is of the same form as the argument "result"
	// of SplitDistances, AFTER we have called SplitDistances. 
	// SortDistances simply sorts all the vectors in increasing order
	// of the distances, 
	static vint8 SortDistances(vector<vector<class_couple> > * distances);

	// result(i) is the weighted L1 distance between object
	// and the i-th object (i-th row) of the database. The weights
	// are specified by the argument "weights".
	static vMatrix<float> L1Distances(v3dMatrix<float> * object, 
		vMatrix<float> database,
		vMatrix<float> weights);


	// result(i) is the weighted L2 distance between object
	// and the i-th object (i-th row) of the database. The weights
	// are specified by the argument "weights".
	static vMatrix<float> L2Distances(v3dMatrix<float> * object, 
		vMatrix<float> database,
		vMatrix<float> weights);

	// result(i) is the weighted Lp (p defined based on fourth 
	// parameter) distance (raised to the p-th power) between object
	// and the i-th object (i-th row) of the database. The weights
	// are specified by the argument "weights".
	static vMatrix<float> LpDistances(v3dMatrix<float> * object, 
		vMatrix<float> database,
		vMatrix<float> weights,
		vint8 distance_p);

	// "name" is the name of an entire dataset. bm_name is the 
	// name of a file where we saved a summary of the BoostMap 
	// training. From that file, we build a d-dimensional embedding,
	// according to the argument "dimensions". result(k) is
	// is the k-nn test error for the dataset, for k = 1, ..., max_k.
	// Note that here we don't handle sensitive embeddings.
	static vMatrix<float> KnnEmbeddingError(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	// same as KnnEmbeddingError, but it measures the training error.
	static vMatrix<float> KnnEmbeddingTrainError(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	// "name" is the name of an entire dataset. "classifiers" holds,
	// at each row, the representation of a 1D embedding. The result
	// of this function holds, at each row, the embedding of 
	// the database defined by the 1D embeddings stored in classifiers.
	static vMatrix<float> embed_database(const char * name,
		vMatrix<float> classifiers);

	// made for the protein datset
	static vMatrix<float> embed_databaseb(const char * name,
		vMatrix<float> classifiers);

	// Same as embed_database, but for the test set of the dataset, 
	// not for the database (or training set).
	static vMatrix<float> embed_test_set(const char * name,
		vMatrix<float> classifiers,
		vint8 database_size);

	// made for the protein datset
	static vMatrix<float> embed_test_setb(const char * name,
		vMatrix<float> classifiers,
		vint8 database_size);

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
	static vMatrix<float> EmbedSet(const char * pathname, vint8 number,
		vint8 database_size,
		vMatrix<float> pivot_distances,
		vMatrix<float> classifiers);

	// made for the protein dataset
	static vMatrix<float> EmbedSetb(const char * pathname, vint8 number,
		vint8 database_size,
		vMatrix<float> pivot_distances,
		vMatrix<float> classifiers);

	// "name" is the name of an entire dataset. classifiers(0) specifies,
	// in the i-th
	// row, the 1D embedding to be used to obtain the i-th
	// coordinate of each object. result(i) is 
	// the intrapivot distance for the i-th 1D embedding, i.e.
	// the i-th row of classifiers, 
	static vMatrix<float> LoadPivotDistances(const char * name,
		vMatrix<float> classifiers);

	// loads the embedding saved in bm_name, and based on that
	// it embeds the training and the test set.
	static vint8 save_embeddings(const char * name, const char * bm_name);

	static vMatrix<float> load_training_embedding(const char * name, 
		const char * bm_name);

	static vMatrix<float> load_training_embedding(const char * name, 
		const char * bm_name,
		vint8 dimensions);

	static vMatrix<float> load_test_embedding(const char * name, 
		const char * bm_name);

	static vMatrix<float> load_test_embedding(const char * name, 
		const char * bm_name,
		vint8 dimensions);

	// returns the filename that should be used for saving or loading
	// the embedding of a test set.
	static char * EmbeddingTestPath(const char * dataset_name,
		const char * embedding_name);
	// returns the filename that should be used for saving or loading2
	// the embedding of a test set.
	static char * EmbeddingTrainPath(const char * dataset_name,
		const char * embedding_name);

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
	static float TestTriple(const char * dataset, 
		float_matrix classifiers,
		float_matrix database_embedding, 
		vint8 q, vint8 a, vint8 b, vint8 distance_p);

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
	static float EmbedTriple(const char * dataset, 
		const char * embedding_name,
		vint8 dimensions, 
		vint8 q, vint8 a, vint8 b, vint8 distance_p);


	// re-implementing some functions, so that they can deal with 
	// sensitive embeddings.

	// Equivalent of L1Distances.
	static vMatrix<float> SL1Distances(const v3dMatrix<float> * object, 
		const vMatrix<float> database,
		const vector<class_triple_classifier> * unique_classifiers,
		const vector<class_sensitive_classifier> * sensitive_classifiers);

	static vMatrix<float> sensitive_distances(const v3dMatrix<float> * object, 
		const vMatrix<float> database,
		const vector<class_triple_classifier> * unique_classifiers,
		const vector<class_sensitive_classifier> * sensitive_classifiers)
	{
		return SL1Distances(object, database, unique_classifiers, sensitive_classifiers);
	}

	// Equivalent of LpDistances.
	static vMatrix<float> SLpDistances(v3dMatrix<float> * object, 
		vMatrix<float> database,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers, 
		vint8 distance_p);

	// Equivalent of KnnEmbeddingError
	static vMatrix<float> KnnSembeddingError(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	// Equivalent of KnnEmbeddingTrainError
	static vMatrix<float> KnnSembeddingTrainError(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	// Equivalent of KnnError7.
	static vMatrix<float> KnnError8(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers,
		vMatrix<float> test_labels,
		vMatrix<float> training_labels,
		vint8 max_k, vint8 training_flag);

	// we measure the classification error for k = 1,...,max_k that we 
	// get by applying the embedding saved in bm_name, with given dimensions,
	// keeping the top to_keep matches, and then sorting those matches
	// based on the exact distances.
	static vMatrix<float> FilterRefineErrors5(const char * name, 
		const char * bm_name,
		vint8 dimensions, vint8 to_keep,
		vint8 max_k);

	static vMatrix<float> 
		FilterRefineErrors10(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers,
		vMatrix<float> test_labels,
		vMatrix<float> train_labels,
		vint8 max_k, vint8 training_flag, 
		vint8 to_keep, class_file * fp);

	// uses L2 distance in embedded space, 
	static vMatrix<float> 
		FilterRefineL2(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vMatrix<float> weights, 
		vMatrix<float> test_labels, 
		vMatrix<float> train_labels,
		vint8 max_k, vint8 training_flag, 
		vint8 to_keep, class_file * fp);



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
	static vMatrix<float> IndexError3(const char * name, 
		const char * bm_name,
		vint8 dimensions);

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
	static vMatrix<float> IndexError6(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers,
		class_file * test_fp, vint8 training_flag);

	// mostly similar to IndexError3, but
	// here we return a test_number x max_k matrix, telling us, for each test
	// object, the worst rank among the top k neighbors, for k = 1, ..., max_k.
	// deprecated
	static vMatrix<float> IndexErrors4(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	static vMatrix<float> index_errors_distribution(const char * name, const char * bm_name,
		vint8 dimensions, vint8 max_k,
		vint8 label, vint8 distribution);

	// made for the protein dataset
	static vMatrix<float> IndexErrors4b(const char * name, 
		const char * bm_name,
		vint8 dimensions,
		vint8 max_k);

	// mostly similar to IndexError6, but here we return
	// a test_number x max_k matrix, telling us, for each test
	// object, the worst rank among the top k neighbors, for k = 1, ..., max_k.
	// deprecated
	static vMatrix<float> IndexErrors7(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers,
		class_file * test_fp, vint8 training_flag, vint8 max_k);

	static vMatrix<float> IndexErrors7_distribution(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers,
		class_file * test_fp, vint8 training_flag, vint8 max_k,
		float_matrix test_labels, vint8 label, vint8 distribution);

	// computes upper bound of distances needed for embedding step
	static vint8 distance_bound(const char * embedding, vint8 dimensions);

	// this function is useful if the embeddings have already been computed
	// (for example for FastMap and MetricMap embeddings), and we do not
	// use a query-sensitive distance measure.  Otherwise, result is
	// similar to that of IndexErrors7.
	static vMatrix<float> IndexErrors6(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vMatrix<float> weights,
		class_file * test_fp, vint8 training_flag, vint8 max_k);

	// this function is useful if the embeddings have already been computed
	// (for example for FastMap and MetricMap embeddings) and we don't
	// use a query-sensitive distance measure. Here distances are 
	// weighted Euclidean distances.
	static vMatrix<float> IndexErrors6L2(vMatrix<float> test_set,
		vMatrix<float> training_set,
		vMatrix<float> weights,
		class_file * test_fp, vint8 training_flag, vint8 max_k);

	// "name" is the name of an entire dataset, "bm_name" is the filename
	// storing an embedding, and dimensions specifies the dimensionality of
	// the embedding that we use. The result is a filename where we should store
	// the result of IndexError3.
	static char * IndexErrorOutputPath(const char * name, const char * bm_name, 
		vint8 dimensions);

	static char * FilterRefineOutputPath(const char * name, const char * bm_name, 
		vint8 dimensions, vint8 to_keep);


	static char * CascadeInfoOutputPath(const char * name, const char * bm_name, 
		vint8 dimensions, vint8 to_keep);

	static char * CascadeKnnOutputPath(const char * name, const char * bm_name, 
		vint8 dimensions, vint8 to_keep);


	// "name" is the name of an entire dataset, "bm_name" is the filename
	// storing an embedding, and dimensions specifies the dimensionality of
	// the embedding that we use. The result is a filename where we should store
	// the result of IndexErrors4.
	static char * IndexErrorOutputPath2(const char * name, const char * bm_name, 
		vint8 dimensions);

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
	static vint8 PermuteTraining(const char * name, const char * new_name);

	// produces a "merged_distances.bin" and a "merged_labels.bin", which
	// is a file storing a square file, with distances from all training
	// and test objects to all training and test objects (training objects
	// appear first).
	static vint8 MergeTestTrain(const char * name);

	// Split assumes there are "merged_distances.bin" and "merged_labels.bin"
	// files in dataset name1. Based on those, it splits the entire dataset
	// into a training and a test set, and saves the corresponding distances
	// and labels.
	static vint8 Split(const char * name1, const char * name2,
		vint8 test_size);

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
	vint8 OnTopDataset(const char * new_name, 
		vMatrix<float> training_embeddingm);

	const char* RootDataDirectory() const
	{
		return root_data_dir;
	}

	static void set_global_data_directory(const char* root_dir);

	// this function takes in exact distances and approximate distances
	// for a given object, and reports, in result(k-1),
	// the approximate rank of the k-th nearest neibhbor.
	static vMatrix<float> IndexErrors3(vMatrix<float> approximate_distances,
		vMatrix<float> exact_distances, 
		vint8 max_k);

	// this function takes in the distances from an object to all training
	// objects. It computes, for each k, whether that object has
	// all its k nearest neighbors bevint8ing to a single class, and whether 
	// that single class is the correct class or not. results(0,k) is
	// incremented by 1 if all k nearest neighbors bevint8 to the same class,
	// and results(1,k) is incremented by 1 if that class is the wrong
	// class. results(2,k) is incremented by 1 if that object bevint8s to
	// the "bad" indices for this dataset.
	static vint8 CascadeStats5(vMatrix<float> distances, vMatrix<float> labels,
		vint8 bad_index, vint8 test_label, 
		vint8_matrix results);

	static vint8 CascadeStats4(const char * name, const char * bm_name,
		vint8 dimensions, vint8_matrix results);

	static vint8 CascadeStats8(vMatrix<float> test_set, vMatrix<float> training_set,
		vMatrix<float> weights, vMatrix<float> test_labels,
		vMatrix<float> train_labels, vint8_matrix bad_indices,
		vint8_matrix results, vint8 training_flag);

	static vint8_matrix ReadBadIndices(const char * name);
	static vint8_matrix read_database_test_indices(const char * name);

	// cascade stats for filter and refine. result contains two rows. 
	// result(0, i) stores number of single_class neighbors for object i,
	// i.e. the max number k such that all k neighbors of i bevint8 to the
	// same class. result(1,i) is 0 if the class of those k neighbors is
	// the same class as the class of i, and 1 otherwise.
	static vint8_matrix CascadeStatsFr(const char * name, const char * bm_name,
		vint8 dimensions, vint8 to_keep, vint8_matrix results);

	static vint8_matrix CascadeStatsFr2(vMatrix<float> test_set, vMatrix<float> training_set,
		vMatrix<float> weights, vMatrix<float> test_labels,
		vMatrix<float> train_labels, vint8_matrix bad_indices,
		vint8_matrix results, vint8 training_flag, vint8 to_keep,
		class_file * fp, vint8_matrix knn_results,
		vint8_matrix database_test_indices);

	// returns whether the object was misclassified or not (1 for misclassified).
	// Stores, in same_class_kp, the number of same-class neighbors.
	static vint8 CascadeStatsFr3(vMatrix<float> distances,
		vMatrix<float> training_labels,
		vint8 bad_index, vint8 test_label, 
		vint8_matrix results, vint8 * same_class_kp);

	// this function is useful when we already have an original dataset and 
	// we want to create a smaller original dataset (maybe because we want
	// to test out something quickly on the smaller dataset).
	static vint8 OriginalSubdataset(const char * original, const char * subset,
		vint8 test_start, vint8 test_size, 
		vint8 training_start, vint8 training_size);

	// this function is useful when we already have an original dataset and 
	// we want to create a smaller original dataset. This is similar to 
	// OriginalSubdataset, but here we keep the entire test set,
	// and a random sample of the training set.
	static vint8 random_subdataset(const char * original, const char * subset,
		vint8 training_size);

	// labels for the training objects of the training set
	// (not the training objects of the original set, which
	// are obtained using LoadTrainingLabels());
	vint8_matrix training_training_labels();

	// labels for the validation objects of the training set
	vint8_matrix training_validation_labels();

	// start of functions written for a possible SIGMOD 2007 paper.


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
	static float_matrix embed_set(const char * pathname, vint8 set_size, const class_embedding * embedding);

	// "name" is the name of an entire dataset. The result
	// of this function holds, at each row, the embedding of a database object.
	// reimplementation of a deprecated previous embed_database, that took as an argument
	// float matrix representing the embedding.
	static float_matrix embed_database(const class_embedding * embedding);

	// Same as embed_database, but for the test set of the dataset, 
	// not for the database (or training set).
	// reimplementation of a deprecated previous embed_test_set, that took as an argument
	// float matrix representing the embedding.
	static float_matrix embed_test_set(const class_embedding * embedding);

	// the next functions are useful for computing distances from 
	// test or training set to training set according to
	// the embedding, and save those distances to a file
	static char * embedding_test_train_file(const char * dataset_name, const char * embedding_name);
	static char * embedding_train_train_file(const char * dataset_name, const char * embedding_name);
	static vint8 embedding_test_train(const char * dataset_name, const char * embedding_name);
	static vint8 embedding_train_train(const char * dataset_name, const char * embedding_name);

	// this function does the actual work for embedding_test_train and embedding_train_train.
	// the way I anticipate this function to be used, first_embedding 
	// is the embedding of either the test subjects or the training (database) objects,
	// and second_embedding is the embedding of the database objects.
	static vint8 embedding_distances(const char * dataset_name, const char * embedding_name,
		const char * output_name,
		float_matrix first_embedding, float_matrix second_embedding);

	// cleaned up version of indexerrors, for test set
	static vint8_matrix retrieval_results_test( 
		const char * dataset_name, const char * embedding_name,
		vint8 dimensions, vint8 max_neighbors);

	static vint8_matrix retrieval_results_training(const char * dataset_name, const char * embedding_name,
		vint8 dimensions, vint8 max_neighbors);

	// A (hopefully better) reimplementation of IndexErrors7
	static vint8_matrix retrieval_results(const float_matrix test_set, const float_matrix training_set,
		const class_embedding * embedding, class_file * test_fp, 
		const vint8 training_flag, const vint8 max_neighbors);

	// an auxiliary function used to perform some sanity checking on
	// the test and training embeddings
	static vint8 check_embedding_validity(float_matrix test_set, float_matrix training_set,
		class_file * test_file_handle);

	// auxiliary function, used to get retrieval accuracy statistics for a particular object,
	// given its exact distances to all database objects and the embedding-based distances
	// to all database objects.
	static vint8 object_retrieval_result(const float_matrix & original_distances,
		const float_matrix & embedding_distances,
		const vint8 max_neighbors, 
		retrieval_statistics * statistics);

	static vint8 print_retrieval_statistics(const retrieval_statistics & statistics);

	// filenames where we should store retrieval results for the test set and the training set
	static char * test_retrieval_pathname(const class_embedding & embedding);
	static char * training_retrieval_pathname(const class_embedding & embedding);

	static vint8 save_retrieval_results(const vint8_matrix result, const char * output_file);

	// vint8 nearest neighbor retrieval results for the test set.
	static vint8_matrix load_results_test(const class_embedding & embedding, const vint8 max_neighbors);

	static vint8_matrix load_retrieval_results(const char * pathname, const vint8 max_neighbors);

	// vint8 nearest neighbor retrieval results for the training set.
	static vint8_matrix load_results_training(const class_embedding & embedding, const vint8 max_neighbors);

	static vint8 database_size(const char * original_dataset_name);
	static vint8 test_size(const char * original_dataset_name);

	// end of functions written for a possible SIGMOD 2007 paper.
};
