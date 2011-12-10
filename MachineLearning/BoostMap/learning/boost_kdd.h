#ifndef VASSILIS_BOOST_KDD_H
#define VASSILIS_BOOST_KDD_H

#include "BoostMap_data.h"

// This is essentially a cleaned up
// re-implementation of BoostMap. Compared to the implementation
// in boost_map.h and boost_map.cpp, the main differences here
// are:
// 
// - it is cleaned up, we don't have subclasses and virtual functions.
//   A lot of code from boost_map.cpp is repeated here.
//
// - I tried to make it easier to apply BoostMap to any dataset, so
//   I constructed the BoostMap_data class to define an interface between
//   BoostMap and different datasets. BoostMap_data is a specification (one 
//   of many possible and useful specifications that I may define later)
//   of how a dataset should be prepared in order to apply BoostMap to it.
//   
// - I include query-sensitive embeddings.
// 
// - I include parameter sensitive embeddings.
// 
// - I include a better way to choose triples (q, a, b), so that 
//   a and b are close to q.

// this function should become a template. Takes a matrix and
// returns another matrix where each entry in the first matrix
// is squared.
vMatrix<float> vSquareEntries(vMatrix<float> input);


/*!
	Class_BoostMap is a new implementation, that includes all the 
	functionality of V_BoostMap3, but also includes additional
	things.
	In particular, to this point, it has these main differences 
	from V_BoostMap3:

	- it is somewhat more memory-efficient, because we do not store in 
	memory some distances, like from all training objects to all
	validation objects.

	- it includes implementation of query-sensitive embeddings.

	- it includes implementation of parameter-space embeddings.

	- it includes a more selective way to choose training triples.
*/
class class_BoostMap
{
protected:
	// class_name is used to identify subtypes of boost_map.
	char * class_name;
	BoostMap_data * data;
	char * triples_file;
	vint8 is_valid;

	// number of objects from which we will pick candidates. 
	// Note that this is different from number_of_picked_candidates.
	vint8 candidate_number;

	// number of training objects used to form training triples,
	// or at least the a and b of training triples (q, a, b)
	vint8 training_number;

	// number of objects used to draw q from (for training triples 
	// of the form (q, a, b).
	vint8 qtraining_number;

	// number of validation objects used to form validation triples,
	// or at least the a and b of validation triples (q, a, b)
	vint8 validation_number;

	// number of objects used to draw q from (for validation triples 
	// of the form (q, a, b).
	vint8 qvalidation_number;

	// number of training and validation triples.
	vint8 large_triple_number;
	vint8 small_triple_number;
	vint8 training_triple_number;
	vint8 validation_triple_number;

	// At each step 
	// we pick a set of random candidates from the values specified
	// by candidate_range. The number of candidates we pick is 
	// specified by number_of_picked_candidates, which can be different
	// than candidate_number (which stores the number of candidates
	// currently chosen).
	vint8 number_of_picked_candidates;

	// defines what type of Lp distance we should use to compute
	// distances in the target space.
	vint8 distance_p;

	// candidate_ids_matrix stores the ids of the candidate objects, i.e.
	// their indices in the actual training set of the entire dataset.
	// This is useful for debugging, but also so that we know, when
	// we load an embedding from file, what actual objects to use
	// for each 1D embedding.
	vint8_matrix candidate_ids_matrix;

	// candidates_matrix specifies the current set of candidate
	// objects. These are picked among all candidates, and the
	// size of candidates_matrix is number_of_picked_candidates.
	vint8_matrix candidates_matrix;

	// distances from all candidate objects to all other candidate
	// objects. These are needed for line projections, to compute
	// intra-pivot distances.
	vMatrix<float> candcand_distances_matrix;

	// distances from all candidate objects to all training or
	// validation objects. These are needed in order to classify
	// the triples.
	vMatrix<float> candtrain_distances_matrix;
	vMatrix<float> candval_distances_matrix;
	vMatrix<float> candqtrain_distances_matrix;
	vMatrix<float> candqval_distances_matrix;

	// the current weights of training triples, updated as specified
	// by the AdaBoost algorithm.
	vMatrix<double> training_factors_matrix;

	// for debugging, the ids of the training objects in the 
	// entire dataset.
	vint8_matrix training_ids_matrix;
	vint8_matrix qtraining_ids_matrix;

	// this is the training set used in AdaBoost. Each row is a triple
	// (q, a, b) of training objects.
	vint8_matrix training_triple_matrix;

	// training_distances[i] is distance(q_i, b_i) - distance(q_i, a_i).
	// similarly for parameter-space distances.
	vMatrix<float> training_distances;
	vMatrix<float> training_pdistances;

	// training_margins and training_pmargins are useful for storing
	// the result of the strong classifier assembled so far for each
	// training triple, multiplied by the class label for that triple
	// (the class label is the only thing that can cause differences
	// between training_margins and training_pmargins). By using
	// these matrices and updating them every time we choose a new weak 
	// classifier, we can easily obtain the current training error
	// of the strong classifier, by thresholding the margin at 0.
	vMatrix<float> training_margins_matrix;
	vMatrix<float> training_pmargins_matrix;
	// ground_truth[i] will equal training_distances[i] if
	// use_pdistances is zero, and ground_truth[i] will equal
	// training_pdistances[i] if use_pdistances is 1.
	vMatrix<float> ground_truth;

	double_matrix large_training_factors;
	vint8_matrix large_training_triples;
	float_matrix large_training_margins;
	float_matrix large_training_pmargins;
	float_matrix large_training_distances;
	float_matrix large_training_pdistances;
	float_matrix large_ground_truth;

	// The "validation" variables are equivalent to the training variables,
	// and they are useful for checking how much we are overfitting the
	// training set, and how well the classifier generalizes outside
	// the training set.
	vint8_matrix validation_ids_matrix;
	vint8_matrix qvalidation_ids_matrix;
	vint8_matrix validation_triple_matrix;
	vMatrix<float> validation_distances;
	vMatrix<float> validation_pdistances;
	vMatrix<float> validation_margins_matrix;
	vMatrix<float> validation_pmargins_matrix;

	// For a lot of the matrices defined above, here we store
	// the corresponding arrays, for convenience.
	vArray(vint8) candidate_ids;
	vArray(vint8) candidates;
	vArray2(float) candcand_distances;
	vArray2(float) candtrain_distances;
	vArray2(float) candqtrain_distances;
	vArray2(float) candval_distances;
	vArray2(float) candqval_distances;

	vArray(vint8) training_ids;
	vArray(vint8) qtraining_ids;
	vArray(double) training_factors;
	vArray2(vint8) training_triples;
	//  vArray(float) training_distances;
	//  vArray(float) training_pdistances;
	//  vArray(float) ground_truth;

	vArray(vint8) validation_ids;
	vArray(vint8) qvalidation_ids;
	vArray2(vint8) validation_triples;
	//  vArray(float) validation_distances;
	//  vArray(float) validation_pdistances;

	// classifiers[i] holds the weak classifier chosen
	// at the i-th training step (starting with the zero-th step).
	vector<class_triple_classifier> classifiers;

	// unique_classifiers represents the same strong classifier, 
	// i.e. the same weighted combination of weak classifiers, as 
	// the "classifiers" vector, but here no weak classifier is repeated
	// twice, so we get a more compact representation.
	vector<class_triple_classifier> unique_classifiers;

	// these vectors hold the error of the strong classifier after the
	// i-th training step. perror stands for "parameter-space error".
	vector<float> training_errors;
	vector<float> validation_errors;
	vector<float> training_perrors;
	vector<float> validation_perrors;

	// number of projections to try at each step
	vint8 projection_candidate_number;

	// allow_projections is non-zero iff we are allowed at the current
	// training step to consider "projections" onto "lines" defined by
	// pairs of pivot objects.
	vint8 allow_projections;

	// allow_lipschitz is non-zero iff we are allowed at the current
	// training step to consider distances to reference objects.
	vint8 allow_lipschitz;

	// use_pdistances is 1 if we should try to optimize 
	// for preservation of parameter-space structure, and 
	// use_pdistances is 0 if we should try to optimize
	// feature-space structure.
	vint8 use_pdistances;

	// specifies, at each step, for how many classifiers we should
	// evaluate the best z.
	vint8 use_best_k;

	// training error and margin of weak classifier that we picked last, 
	// and some other statistics for that classifier.
	float last_error; 
	float last_et;
	float last_correlation;
	float last_scaled_et;
	float last_scaled_correlation;

	// more statistics for the last weak classifier we picked, but
	// these ones are with respect to the parameter space distance.
	float last_perror; 
	float last_pet;
	float last_pcorrelation;
	float last_scaled_pet;
	float last_scaled_pcorrelation;

	// last_new_z is the z of the last new classifier that we added 
	// (i.e. a classifier that was not just a removal or an adjustment
	// of an already chosen classifier). This value is useful to 
	// determine at what level of z we should accept weight modifications.
	float last_new_z;

	// training and validation error and perror (and also training margin,
	// which I'm not sure if it's of any significance) 
	// of the strong classifier learned so far.
	float training_error;
	float training_margin;
	float validation_error;

	float training_perror;
	float validation_perror;

	// some variables that collect information useful for future
	// optimizations:
	// counts the total number of iterations spent on binary search
	// for minimizing z.
	vas_int16 iterations;
	// The smallest x passed to the pow(e, x) function
	float min_exp;
	// The largest x passed to the pow(e, x) function
	float max_exp;

	// 0 if negative weights (alphas) should not be allowed, non-zero
	// otherwise.
	vint8 allow_negative;

	// 0 if removing existing classifiers, or reducing their weight, should not 
	// be tried as part of the training. 1 if those things should be tried.
	vint8 allow_removals;

	// 0 if we should use sensitive embeddings, 1 otherwise.
	vint8 allow_sensitive;

	// if allow_sensitive is 1, new_sensitive = 1 allows use of classifiers
	// that are not in unique_classifiers.
	vint8 new_sensitive;

	// here we hold sensitive classifiers, which I also call
	// query-sensitive classifiers, and which are used to define
	// query-sensitive distances.
	vector<class_sensitive_classifier> sensitive_classifiers;

	// will keep track of the max output value corresponding to weak classifiers
	// whose results have been read from file. This will be used mainly to
	// make sure that, when we normalize so that all outputs are in the 
	// [-1, 1] range, we always divide by the same number. 
	float distance_max_entry;

	vMatrix<float> m_training_results;
	vMatrix<float> m_validation_results;
	vint8_matrix m_classifier_ids;

protected:
	// set all training weights to initial values (equal to each other,
	// and summing to 1).
	vint8 InitialWeights();

	// add the specified weak classifier to unique_classifiers. If 
	// the classifier already appears in unique_classifiers, we 
	// just modify the weight there, otherwise we append this
	// classifier to the end of unique_classifiers.
	vint8 clean_up_classifier(class_triple_classifier classifier);

	// initialize training and validation margins and pmargins, and
	// initialize weights.
	vint8 InitializeMatrices();

	vint8 SetAllArrays();
	vint8 Initialize();

	// to be used with the constructor that reads training triples from disk.
	vint8 Initialize2();

public:
	class_BoostMap();

	// we specify a training dataset, number of training triples,
	// and whether we should optimize for feature-space distances
	// or parameter-space distances.
	class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples,
		vint8 in_use_pdistances);

	// here we pick triples that are focused, and more appropriate
	// for nearest neighbor optimization, or for nearest neighbor
	// classification.
	class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples,
		vint8 in_use_pdistances, vint8 classes, vint8 max_k);

	// here we pick triples that are focused, and more appropriate
	// for nearest neighbor optimization, or for nearest neighbor
	// classification. Flag is used as an argument just to differentiate
	// this constructor from the previous one. In the previous one 
	// we call MakeTriples2, here we call MakeTriples3.
	class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples,
		vint8 in_use_pdistances, vint8 max_k);

	// here we pick triples in which we specify how close a and b should
	// be to q (min_a, max_a. min_b, max_b specify rank ranges), 
	// but we don't care about class labels.
	class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples, 
		vint8 min_a, vint8 max_a, vint8 min_b, vint8 max_b);

	// here we pick triples in which we specify how close a and b should
	// be to q (min_a, max_a. min_b, max_b specify rank ranges), 
	// but we don't care about class labels. This constructor is a modification
	// of the previous one, so that this one works with the protein dataset.
	// junk is used as an argument just to make this constructor different
	// than the previous one.
	class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples, 
		vint8 min_a, vint8 max_a, vint8 min_b, vint8 max_b, vint8 junk);

	class_BoostMap(const char * root_dir, const char * in_dataset_name, const char * in_saved_name, 
		vint8 in_training_triples, vint8 in_validation_triples,
		vint8 in_p_distances);

	~class_BoostMap();

	// assign default values to member variables.
	vint8 Zero();
	const char * get_class_name();
	inline vint8 valid()
	{
		return is_valid;
	}

	// pick the next weak classifier.
	float NextStep();

	// An alternative way to pick the next weak classifier. 
	// Here we first compute just the training errors for each candidate, 
	// and then we only minimize z for the use_best_k of those candidates.
	// this function simply set a small number of training triples,
	// calls fast_next_step_actual, and then sets back the original large 
	// number of training triples.
	float fast_next_step();

	// this function actually does the work of fast_next_step
	float fast_next_step_actual();

	// This function just calls NextStep or FastNextStep. Right now
	// it is hardcoded, I should probably have a member variable
	// based on which we should decide.
	float NextSteps(vint8 i);

	// check how many training rounds we have performed (excluding
	// query-sensitive training).
	vint8 StepsDone();

	// find the best z and alpha (weight) for the specified
	// line projection embedding.
	float PivotPairZ(vint8 pivot1, vint8 pivot2, float * alpha);

	// get the results of the classifier specified by pivot1 and
	// pivot2 on training or validation triples (passed in as
	// triple_matrix). results is assumed to have enough space
	// to store the results. The result for each triple is 
	// the output of the classifier times the class label for the
	// triple. candset_distances_matrix should be
	// either candtrain_distances_matrix
	// or candval_distances_matrix, depending on triple_matrix.
	vint8 PivotPairResults(vint8 pivot1, vint8 pivot2, vector<float> * results,
		vint8_matrix triple_matrix, 
		vMatrix<float> triple_distances_matrix,
		vMatrix<float> candset_distances_matrix);

	// same as PivotPairResults, but for a reference-object type of
	// classifier, using the index-th candidate as the reference object.
	vint8 LipschitzResults(vint8 index, vector<float> * results,
		vint8_matrix triple_matrix, 
		vMatrix<float> triple_distances_matrix,
		vMatrix<float> candset_distances_matrix);

	// get the best z and alpha for the given weak classifier.
	float ClassifierZ(class_triple_classifier * classifier, float * alpha);

	// similar to PivotPairResults and LipschitzResults, but here
	// we can handle both types of classifiers (line projections
	// and reference-object-based).
	vint8 ClassifierResults(class_triple_classifier * classifier, 
		vector<float> * results,
		vint8_matrix triple_matrix, 
		vMatrix<float> triple_distances_matrix,
		vMatrix<float> candset_distances_matrix);


	// It finds the lipschitz classifier that 
	// by removing it we attain the best accuracy
	class_triple_classifier LipschitzRemoval(vint8 * success);
	// It finds the best weight modification for already
	// selected lipschitz classifiers
	class_triple_classifier LipschitzChange(vint8 * success);
	// It finds the best Lipschitz weak classifier for the
	// current training step
	class_triple_classifier NextLipschitz();

	// It finds the projection classifier that 
	// by removing it we attain the best accuracy
	class_triple_classifier ProjectionRemoval(vint8 * success);
	// It finds the best weight modification for already
	// selected projection classifiers
	class_triple_classifier ProjectionChange(vint8 * success);
	// It finds the best projection weak classifier for the
	// current training step
	class_triple_classifier NextProjection();

	// this function is used to pass to outside functions the
	// definition of the strong classifier stored in classifiers.
	// Not implemented yet.
	vint8 Classifier(vector<vint8> * out_indices, vector<float> * out_weights);

	// Here we identify repetitions of classifiers and we combine them. 
	// Essentially we pass the contents of unique_classifiers.
	// Not implemented yet.
	vint8 CleanClassifier(vector<vint8> * out_indices, vector<float> * out_weights);

	// returns a number x 2 matrix, where every row is a pair of indices
	// occuring in candidate_indices. candidate_indices specifies the
	// indices of objects that we should use to define 1D embeddings.
	static vint8_matrix RandomPivots(vint8_matrix candidate_indices,
		vint8 number);

	vint8 SetCandidates(vint8_matrix in_candidates)
	{
		candidates_matrix = in_candidates;
		candidates = candidates_matrix.Matrix();
		return 1;
	}

	// result(i) = i. A trivial way to generate a candidates_matrix
	// that includes all candidates.
	vint8_matrix AllCandidates(vint8 in_number);

	// find the best z and alpha for the specified reference-object-based
	// classifier.
	float ClassifierZ(vint8 index, float * alpha);

	// find the best alpha and the corresponding minimum z for
	// the weak classifier that gives these results.
	vint8 MinimizeZ(vector<float> * results, float * z, float * a);

	// This function is not used right now, but it should probably be 
	// used when we evaluate changing weights of classifiers.
	// Same as MinimizeZ, but with the additional constraint that
	// *a should be in the range [alpha_low, alpha_hi].
	vint8 MinimizeZ5(vector<float> * results, float * z, float * a,
		float alpha_low, float alpha_hi);

	// compute z for a classifier with the given weight (a) and
	// that gives the given results 
	double Z(double a, vector<float> * results);

	// compute the derivative of z with respect to a
	// for a classifier with the given weight (a) and
	// that gives the given results 
	double Z_Prime(double a, vector<float> * results);

	// assuming that we just chose the weak classifier that gives these
	// results, and that achieves the given z with the given alpha,
	// update the training weights as specified by AdaBoost.
	float UpdateWeights(vector<float> * results, float z, float alpha);

	// Computes errors and margins for the last chosen weak classifier,
	// which was assigned the given alpha (weight) and achieves
	// the given results (multiplied by class label, as usual). One
	// version for feature-space distances, one for parameter-space distances.
	float ComputeLastError(float alpha, vector<float> * results);
	float ComputeLastPerror(float alpha, vector<float> * results);

	// The next functions compute errors and margins 
	// for the strong classifier assembled
	// so far, possibly including the sensitive classifiers.
	float ComputeTrainingError(vector<float> * results, vint8 use_sensitive = 0);
	float ComputeValidationError(vector<float> * results, vint8 use_sensitive = 0);

	float ComputeTrainingPerror(vector<float> * results, 
		vint8 use_sensitive = 0);
	float ComputeValidationPerror(vector<float> * results,
		vint8 use_sensitive = 0);

	// UpdateMargins does all the work for the preceding four functions,
	// i.e. for computing training or validation errors or perrors.
	vint8 UpdateMargins(vint8_matrix triple_matrix,
		vector<float> * results,
		vMatrix<float> margins_matrix,
		float * errorp, float * marginp,
		vint8 use_sensitive = 0);

	// This should be called to load from a file: translates a
	// simple filename into a complete pathname, and adds the 
	// extension ".txt".
	static char * Pathname(const char * root_dir, const char * filename);

	// This should be called to save unique classifiers to a file.
	// Translates simple filename into complete pathname, and
	// adds an extension that depends on the number of training
	// rounds (including query-sensitive training).
	char * Pathname2(const char * filename) const;

	// This should be called to save detailed classifiers to a file.
	// Translates simple filename into complete pathname, and
	// adds an extension that depends on the number of training
	// rounds (including query-sensitive training).
	char * Pathname3(const char * filename) const;

	float TrainingError();
	float TrainingMargin();
	float ValidationError();
	float LastError();
	float LastEt();
	float LastCorrelation();  
	float StepZ(vint8 step);
	float StepTrainingError(vint8 step);
	float StepValidationError(vint8 step);

	// given distances from an object to all training objects,
	// and given the ids of the reference objects, compute
	// the distances to all reference objects.
	static vMatrix<float> ReferenceDistances(vMatrix<float> all_distancesm,
		vint8_matrix reference_idsm);


	// this function only handles reference-object-type embeddings. 
	// "references" includes all the reference objects. "index" is
	// the index of the object (with respect to the training or
	// validation set). candset_distances_matrix should be either 
	// candtrain_distances_matrix or candval_distances_matrix,
	// depending on whether index refers to a training or validation 
	// object.
	vMatrix<float> Embedding3a(vint8 index, vector<vint8> * references,
		vMatrix<float> candset_distances_matrix);


	// "index" and "candset_distances_matrix" are as in Embedding3a. However,
	// this function (contrary to Embedding3a) handles also line-projection 
	// embeddings. It produces the embedding of object specified by index
	// based on the classifiers stored in unique_classifiers.
	vMatrix<float> Embedding2(vint8 index, 
		vMatrix<float> candset_distances_matrix);

	// query_distances is a matrix with dimensions x 2 
	// entries, of distances
	// from the query to each of the two objects corresponding
	// to each dimension. If a dimension corresponds to a lipschitz
	// embedding, the second col is ignored for that dimension.
	// pivot_distances has, at position i, the distance between the i-th
	// pivot points (if the dimension corresponds to a projection).
	// If the number of rows of query_distances is less than the
	// number of unique classifiers, we use only the first classifiers.
	// types_matrix(i) is 0 if the i-th dimension is a lipschitz embedding,
	// and 1 if the i-th dimension is a pseudo-projection.
	static vMatrix<float> Embedding3(vMatrix<float> query_distances_matrix,
		vMatrix<float> pivot_distances_matrix,
		vint8_matrix types_matrix);


	// it returns a row matrix, which at positions 2*i and 2*i+1 stores
	// the indices of the objects that define the i-th classifier 
	// stored in unique_classifiers (i-th
	// coordinate of the embedding). Of course, if the i-th classifier
	// is a reference-object type of classifier, then position 2*i+1
	// does not matter and it can hold anything (most likely -1, to
	// indicate that no object is needed there).
	static vint8_matrix ExtractReferenceObjects(vMatrix<float> classifiers);
	static vint8_matrix extract_references_from_unique_classifiers(vMatrix<float> classifiers);

	// It returns a row matrix, whose i-th entry is the weight of the
	// i-th unique classifier.
	static vMatrix<float> ExtractWeights(vMatrix<float> classifiers);

	// it returns a row matrix whose i-th entry is the type of
	// the i-th unique classifier.
	static vint8_matrix ExtractTypes(vMatrix<float> classifiers);

	// assuming that v1 and v2 are embeddings of two objects, as 
	// specified by the embedding defined in unique_classifiers,
	// here we compute the L1 distance between those vectors.
	// This function cannot handle query-sensitive embeddings.
	float L1_Distance(vMatrix<float> v1, vMatrix<float> v2);

	// it seems to me right now that this function should be static.
	// v1 and v2 are embeddings of two objects, as specified by
	// the embedding defined by the classifiers in "weights". 
	// Since the objects have already been embedded, the only useful
	// information in "weights" is the weights of those classifiers.
	float L1_Distance3(vMatrix<float> v1, vMatrix<float> v2,
		vector<class_triple_classifier> * weights);

	// this function should also probably be static. 
	// Similar to L1_Distance3, but here instead of passing
	// a vector of classifiers, we pass in a vector of weights,
	// since we don't need any other information from the classifiers
	// anyway.
	float L1_Distance3a(vMatrix<float> v1, vMatrix<float> v2,
		vector<float> * weights);

	float Lp_Distance(vMatrix<float> v1, vMatrix<float> v2);

	// it seems to me right now that this function should be static.
	// v1 and v2 are embeddings of two objects, as specified by
	// the embedding defined by the classifiers in "weights". 
	// Since the objects have already been embedded, the only useful
	// information in "weights" is the weights of those classifiers.
	float Lp_Distance3(vMatrix<float> v1, vMatrix<float> v2,
		vector<class_triple_classifier> * weights);

	// this function should also probably be static. 
	// Similar to L1_Distance3, but here instead of passing
	// a vector of classifiers, we pass in a vector of weights,
	// since we don't need any other information from the classifiers
	// anyway.
	float Lp_Distance3a(vMatrix<float> v1, vMatrix<float> v2,
		vector<float> * weights);

	// For debugging: weights should sum up to 1.
	float SumWeights();
	float SumClassifierWeights();
	virtual vint8 PrintSummary();
	vint8 PrintClassifier();
	vint8 PrintAll();

	// Pick random reference points, and compute error and margin for them.
	vint8 PickRandom(vint8 number);

	// PickRandom3 does the work for PickRandom
	vint8 PickRandom3(vint8 number, float * training_errorp,
		float * validation_errorp);

	// ComputeError is used in conjunction with PickRandom, to 
	// compute the statistics of the embedding (errors, margins).
	// It does not handle parameter space errors, at least not right
	// now.
	float ComputeError(vector<vint8> * references, vector<float> * weights,
		vint8_matrix triples_matrix, vMatrix<float> ground,
		vMatrix<float> candset_distances_matrix);

	// chooses, from all candidate objects, a random subset which will
	// be used to define the candidate weak classifiers for the next
	// training step.
	vint8 ChooseCandidates(vint8 in_pick_number);

	// it seems this function is not implemented and is not needed.
	vint8 SetPickCandidatesFlag(vint8 value);

	// This function is useful when we want to fully specify the classifier,
	// for example if we want to load an existing classifier, so we want
	// to use the weight that has already been computed.
	virtual vint8 AddClassifier(class_triple_classifier classifier);

	// Adds, in the order in which they are given, all the classifiers
	// stored in the rows of the matrix.
	vint8 AddClassifierMatrix(vMatrix<float> classifier_matrix);

	// Adds, in the order in which they are given, all the classifiers
	// stored in the rows of the matrix. It stops when either it has
	// gone through all the rows of the matrix, or the next row of the
	// matrix will increase the number of unique classifiers to dimensions+1.
	vint8 AddClassifierMatrix2(vMatrix<float> classifier_matrix, 
		vint8 dimensions);

	// set whether we allow negative weights for a weak classifier in
	// unique_classifiers. We always allow a negative weight for a 
	// classifier in classifiers, as vint8 as the corresponding weight
	// in unique_classifiers (after we sum up weights for all other
	// occurrences of the same classifier in classifiers) is non-negative.
	vint8 SetAllowNegative(vint8 in_value);

	// set whether we allow, at each training step, removal of 
	// previously chosen weak classifiers, if that removal is 
	// deemed to improve classification accuracy.
	vint8 SetAllowRemovals(vint8 in_value);

	// Return zero if no removal was successful, one otherwise.
	// Only tries removals of lipschitz types. If successful,
	// it stores the z attained by removing the classifier, the
	// weight that will set the global weight of that classifier to 0,
	// and the index of the reference object in the set of candidates.
	vint8 TryRemovals(vint8 * z_min_indexp, float * z_minp, float * alphap);


	// It finds the best weight modification for already
	// selected lipschitz classifiers. It returns the same things as 
	// TryRemovals.
	vint8 TryWeightChange(vint8 * z_min_indexp, float * z_minp, float * alphap);

	// returns the location of the directory where we save and load 
	// embeddings.
	char * Directory() const;

	// returns the location of the directory where we save and load 
	// embeddings.
	static char * Directory(const char* root_dir);

private:
	// save_itinerary does the actual work for the save function,
	// and is called once for each of the two copies that we save.
	vint8 save_auxiliary(const char * filename);

public:
	// save the classifier to disk. We actually create two copies of
	// three different 
	// files: one file for the unique classifiers, one file for classifiers 
	// (useful if we want to load them in the same order in which
	// they were picked, for example to see the step-by-step errors,
	// or to stop at a given number of dimensions), and one file for
	// sensitive_classifiers. Also, in the second copy of those files,
	// we add a round-robin extension that 
	// depends on the total number of classifiers we have so far.
	// Adding that extension is useful because, at training, 
	// instead of writing over the same file over and over again,
	// we actually have ten different files, so we can easily
	// compare the last ten steps, and so that if writing the 
	// latest file breaks down we still have the previous one intact.
	vint8 save_classifier(const char * filename);

	char * round_robin_filename(const char * filename);

	// responsible for loading the insensitive part of the classifier
	// from a file ending in *d.txt.
	static float_matrix load_insensitive_classifier(const char* filename);


	// We load a classifier from disk. The filename should be 
	// a filename where we store classifiers (the ".txt" extension
	// is not necessary, but the stuff that Pathname2 and Pathname3 
	// adds to the name should be included here). Note that,
	// if the file was generated with SaveClassifier, and 
	// sensitive classifiers were also stored, then we can 
	// use either the filename that was generated from Pathname2
	// or the filename generated from Pathname3. sensitive_pathname
	// is smart enough to figure out the corresponding name
	// for the file where the sensitive classifiers get saved.
	vint8 load_classifier(const char * filename);

	// Here we load the most classifiers we can load without
	// getting over the specified number of dimensions.
	// Note that the filename passed in here should not be the 
	// same as the filename passed to SaveClassifier, because
	// SaveClassifier adds a number to it. For example, we
	// may call SaveClassifier("name"), and LoadClassifier("name_4").
	vint8 load_classifier_b(const char * filename, vint8 dimensions);

	// LoadQsClassifier is useful for loading classifiers/embeddings
	// trained by using (from the beginning of the training)
	// the bmallow_sensitive 1 1 command, i.e.
	// with allow_sensitive = 1, and new_sensitive = 1. In those
	// cases, we want to load the first dimensions of the detailed
	// classifiers, and the first dimensions of the sensitive classifiers.
	vint8 load_sensitive_classifier(const char * filename, vint8 dimensions);


	// the next function is useful when passed in a detailed classifier,
	// by giving us only the first dimensions dimensions learned in the process
	// of learning that classifier.
	static vMatrix<float> select_first_dimensions(vMatrix<float> classifier_matrix,
		vint8 dimensions);

	// The cutoff under which z must be in order to accept weight modifications.
	float ChangeCutoff();

	// object_number, reference_indices and reference_weights are input
	// arguments, cleaned_indices and cleaned_factors are output arguments.
	// object_number is simply used to decide how much "scratch" memory
	// to use, in which we keep a look-up table to check if we have
	// seen a classifier before. The output arguments received a cleaned-up
	// version of the classifier stored in reference indices and reference
	// weights. Obviously, this version only handles classifiers defined
	// using reference-object embeddings.
	// NOT IMPLEMENTED YET.
	static vint8 CleanUp(vint8 object_number, vector<vint8> * reference_indices,
		vector<float> * reference_weights,
		vector<vint8> * cleaned_indices,
		vector<float> * cleaned_factors);

	// We assume that reference_images(i, 3) is the index
	// into the distances matrix corresponding to the i-th 
	// reference image. 
	// NOT IMPLEMENTED YET.
	static v3dMatrix<float> * CleanUp(vint8 object_number, 
		v3dMatrix<float> * reference_images);

	// returns number of lipschitz-embedding-type classifiers 
	// in cleaned_classifiers.
	vint8 LipschitzSteps();
	// returns number of line-projection-type classifiers 
	// in cleaned_classifiers.
	vint8 ProjectionSteps();

	// Returns the unique classifiers in a matrix form, that is easier to 
	// save and load.
	vMatrix<float> ClassifierMatrix();

	// Returns the classifiers (not from unique_classifiers, but from
	// classifiers) in a matrix form, that is easier to  save and load.
	// This matrix contains more information than the matrix returned
	// in ClassifierMatrix. Here we have a row for each training step
	// that was performed. We also have additional columns, for example
	// for training and validation errors and perrors.
	vMatrix<float> DetailedClassifierMatrix();

	// the input "distance" is the distance from q to b
	// minus the distance from q to a for a triple of objects (q, a, b).
	// Label(distance) returns the corresponding class label for that
	// triple. Note that, for numerical reasons, we give a 0 label
	// if the absolute value of the distance is non-zero but still
	// very small.
	inline float Label(float distance)
	{
		float label;
		if (vAbs(distance) < 0.000001)
		{
			label = 0;
		}
		else if (distance < 0)
		{
			label = (float) -1;
		}
		else
		{
			label = (float) 1;
		}
		return label;
	}

	inline vint8 SetAllowLipschitz(vint8 in_allow_lipschitz)
	{
		allow_lipschitz = in_allow_lipschitz;
		return 1;
	}

	inline vint8 SetAllowProjections(vint8 in_allow_projections)
	{
		allow_projections = in_allow_projections;
		return 1;
	}

	inline vint8 SetAllowSensitive(vint8 in_value1, vint8 in_value2)
	{
		allow_sensitive = in_value1;
		new_sensitive = in_value2;
		return 1;
	}

	inline vint8 SetPickedCandidates(vint8 number)
	{
		number_of_picked_candidates = number;
		projection_candidate_number = number_of_picked_candidates;
		return 1;
	}

	inline vint8 GetAllowSensitive()
	{
		return allow_sensitive;
	}

	// For debugging.
	// TestTriple prints out info about a triple, prints the current
	// margin for that triple, and verifies that margin by computing
	// the distance from scratch. We print that info for the index-th
	// training triple and the index-th validation triple.
	vint8 TestTriple(vint8 index);

	// print a range of triples, whose indices are in {start, ..., end}
	vint8 PrintTrainingTriples(vint8 start, vint8 end);
	vint8 PrintValidationTriples(vint8 start, vint8 end);

	// TripleEmbeddings prints out the embedding for each object in 
	// a triple, the distances qa and qb, and qa-qb.
	vint8 TripleEmbeddings(vint8 index);

	// TripleResults simulates the process by which training_margins[index]
	// got its current value, and prints, for each classifier, the contribution
	// that this classifier makes to this margin.
	// It does not handle query-sensitive embeddings.
	vint8 TripleResults(vint8 index);

	// Start of functions implementing sensitive embeddings 
	// (query-sensitive embeddings)

	// pick the next sensitive classifier and add it to 
	// sensitive_classifiers
	vint8 NextSensitiveStep();

	// identify the next sensitive classifier
	class_sensitive_classifier NextSensitiveClassifier();

	// choose candidate sensitive classifiers. They are built
	// based on unique_classifiers (at least in the current, restricted
	// implementation). Roughly half of them use the same 1D embedding
	// both as splitter and as classifier, and half of them use
	// a different splitter. For each combination of splitter/classifier
	// we try lots of different ranges for the splitter.
	vint8 GetSensitiveCandidates(vector<class_sensitive_classifier> * classifiers);

	// Number of query-sensitive classifiers chosen so far.
	vint8 SensitiveStepsDone();

	// measure z and alpha (weight) for the given query-sensitive classifier.
	float SensitiveZ(class_sensitive_classifier * classifier, float * alpha);

	// get the result (classification result times class label)
	// corresponding to each triple in triples_matrix, for the
	// given classifier. results is an output argument.
	// splitter_one_dim and classifier_one_dim are the one-D 
	// embeddings of all (training or validation) objects
	// based on the splitter and the classifier embedding.
	// triples_matrix should be either training triples
	// or validation triples, and triple_distances_matrix
	// should be the matrix that corresponds to triples_matrix.
	vint8 SensitiveResults(class_sensitive_classifier * classifier, 
		vector<float> * splitter_one_dim,
		vector<float> * classifier_one_dim,
		vint8_matrix triples_matrix,
		vMatrix<float> triple_distances_matrix,
		vector<float> * results);

	// Add the sensitive classifier to sensitive_classifiers,
	// and do the necessary updates (compute errors, update training
	// weights).
	virtual vint8 AddSensitiveClassifier(class_sensitive_classifier classifier);

	// we return the one-dimensional embeddings of all training objects
	// based on this classifier. We assume that results already has
	// enough allocated space to store the embeddings.
	vint8 TrainingOneDim(class_triple_classifier classifier,
		vector<float> * results);
	float_matrix compute_training_embeddings(class_triple_classifier classifier);
	float_matrix compute_embeddings(class_triple_classifier classifier,
		float_matrix distances);

	// we return the one-dimensional embeddings of all validation objects
	// based on this classifier. We assume that results already has
	// enough allocated space to store the embeddings.
	vint8 ValidationOneDim(class_triple_classifier classifier,
		vector<float> * results);

	// This should be called to load the sensitive classifiers from a file.
	static char * sensitive_pathname(const char * filename);

	// This should be called to save sensitive classifiers to a file.
	char * sensitive_pathname2(const char * filename);

	// These functions are called from SaveClassifier and LoadClassifier
	// to read the filenames storing the query-sensitive (sensitive)
	// classifiers.
	vint8 SaveSensitiveClassifier(const char * filename);
	vint8 LoadSensitiveClassifier(const char * filename);

	// loads the first "number" classifiers.
	vint8 LoadSensitiveClassifier2(const char * filename,
		vint8 number);

	// converts the classifiers in sensitive_classifiers into 
	// a convenient (for some things) matrix form.
	vMatrix<float> SensitiveClassifierMatrix();

	// we add the query-sensitive classifiers contained in 
	// sensitive_matrix into sensitive_classifiers, and we
	// treat them like weak classifiers picked during training
	// (so we compute the corresponding errors, and we update
	// the training weights).
	vint8 AddSensitiveMatrix(vMatrix<float> sensitive_matrix);

	// convert a query-sensitive classifier into a row matrix.
	vMatrix<float> SensitiveToMatrix(class_sensitive_classifier classifier);

	// convert the row-th row of matrix into a sensitive classifier.
	static class_sensitive_classifier MatrixToSensitive(vMatrix<float> matrix, vint8 row);

	// returns the number of columns that a matrix of sensitive classifiers
	// should contain. This changes when I decide to add to or take things out
	// from that matrix, and when we read from file we use this function
	// as a sanity check to make sure we are reading something that is
	// not obviously an obsolete format. A better way to do this would
	// be to store a format version in each matrix.
	vint8 SensitiveMatrixCols();

	// the next few functions are for exploring whether we can come up
	// with heuristics that would speed up search time. In ClassifierStatistics
	// we do almost what we would do for NextStep, but we also collect some 
	// statistics, and we may skip optimizing z, since that is costly.
	vint8 ClassifierStatistics(vint8 type, vint8 object1, vint8 object2);

	// AppendClassifiers loads some classifiers from file, but it skips
	// as many of them as the number of steps performed so far. After
	// it skips those, it adds as many dimensions as specified. This is 
	// useful if we want to add classifiers a few at a time, and check
	// certain things after we add some of them.
	vint8 AppendClassifiers(const char * filename, vint8 dimensions);

	// AppendClassifiers loads some classifiers from file, but it skips
	// as many of them as the number of steps performed so far. After
	// it skips those, it adds as many dimensions as specified.
	vint8 AppendSensitiveClassifiers(const char * filename, 
		vint8 dimensions);

	// computes the error of specified classifier, on weighted
	// training set.
	float ClassifierWeightedError(vint8 type, vint8 object1, vint8 object2);

	// computes the error of specified classifier, on weighted
	// training set.
	float ClassifierWeightedError(class_triple_classifier classifier);

	// computes the error of classifier with given results, on weighted
	// training set.
	float ClassifierWeightedError(vector<float> * results);

	// computes the margin of specified classifier, on weighted
	// training set.
	float ClassifierWeightedMargin(vint8 type, vint8 object1, vint8 object2);

	// computes the margin of specified classifier, on weighted
	// training set.
	float ClassifierWeightedMargin(class_triple_classifier classifier);

	// computes the margin of classifier with given results, on weighted
	// training set.
	float ClassifierWeightedMargin(vector<float> * results);

	// returns sensitive classifiers that are limited to 
	// using classifiers from unique_classifiers. For each
	// splitter/classifier pair we consider, we find the 
	// range that leads to the lowest training error using
	// BestClassifier.
	vint8 RestrictedSensitiveCandidates(vector<class_sensitive_classifier> * classifiers);

	// create some random sensitive classifiers (not limiting ourselves
	// to using classifiers already in unique_classifiers). For each
	// splitter/classifier pair we consider, we find the 
	// range that leads to the lowest training error using
	// BestClassifier.
	// To keep the overall complexity of embedding the query low, we
	// always have, in this case, splitter = base. Note that
	// in the RestrictedSensitiveCandidates splitter = base only half
	// the times.

	vint8 RandomSensitiveCandidates(vector<class_sensitive_classifier> * classifiers);

	// create some random candidate weak classifiers, that will compute
	// for the next classifier to be chosen. Right now this function
	// is only used in FastNextStep(), in principle I should also
	// use it in NextStep().
	vint8 RandomCandidates(vector<class_triple_classifier> * classifiers);

	// Given some constraint on the range, we evaluate lots of possible 
	// sensitive classifiers built using splitter and classifier, and
	// we return the one with the lowest training error.
	class_sensitive_classifier BestClassifier(class_triple_classifier splitter,
		class_triple_classifier classifier);

	// here we assume that, in classifiers, classifier.splitter.weight
	// holds the weighted training error rate for that classifier.
	// We put into selected the "number" classifiers with the lowest
	// weighted training error rate.
	vint8 SelectBest(vector<class_sensitive_classifier> * classifiers,
		vector<class_sensitive_classifier> * selected,
		vint8 number);


	// similar to the previous SelectBest, but here, instead
	// of looking for classifier A at A.splitter.weight
	// for the training error, we look that up in the 
	// corresponding entry in errors. Note that we have to
	// do that, because here we handle non-sensitive classifiers,
	// whereas the previous SelectBest handles query-sensitive
	// classifiers.
	vint8 SelectBest(vector<class_triple_classifier> * classifiers,
		vector<class_triple_classifier> * selected,
		vector<float> * errors,
		vint8 number);

	// similar to the previous SelectBest functions, but this    
	// one uses ClassifierWeightedError to figure out the
	// training error of each classifier in classifiers.
	vint8 SelectBest(vector<class_triple_classifier> * classifiers,
		vector<class_triple_classifier> * selected,
		vint8 number);

	// A fast (relatively speaking) method of choosing the next weak
	// query-sensitive classifier. For each splitter/classifier pair,
	// we choose the range that gives the best training error,
	// and then among the candidates we choose the use_best_k ones,
	// and then finally among those we pick the one that yields
	// the smallest Z.
	// this function simply set a small number of training triples,
	// calls fast_next_sensitive_actual, and then sets back the original large 
	// number of training triples.
	vint8 fast_next_sensitive_step();
	vint8 fast_next_sensitive_actual();

	inline vint8 SetBestK(vint8 in_k)
	{
		use_best_k = in_k;
		return 1;
	}

	// looks in unique_classifiers to find a classifier that 
	// has the same type and reference objects as the argument.
	// returns -1 if no classifier is found.
	vint8 FindClassifierIndex(class_triple_classifier classifier,
		vint8 strictness_flag = 1);

	// a static version of the previous function, useful when
	// we don't have a BoostMap object, but we do have
	// the unique_classifiers vector.
	static vint8 FindClassifierIndex(class_triple_classifier classifier,
		vector<class_triple_classifier> * unique_classifiers,
		vint8 strictness_flag = 1);

	// To make sure there are no bugs, we go through all sensitive 
	// classifiers, and we verify that their splitter_index and
	// classifier_index parameters are correct. If they are not correct
	// we print a warning. Result is > 0 if there was no problem.
	vint8 VerifySensitiveIndices();

	// a static version of the previous function, useful when we have the
	// vectors but we don't have a BoostMap object.
	static vint8 VerifySensitiveIndices(const vector<class_triple_classifier> * unique_classifiers,
		const vector<class_sensitive_classifier> * sensitive_classifiers);

	// Go through all sensitive classifiers and set their 
	// splitter_index and classifier_index parameters.
	vint8 SetSensitiveIndices();

	// a static version of the previous function.
	static vint8 SetSensitiveIndices(vector<class_triple_classifier> * unique_classifiers,
		vector<class_sensitive_classifier> * sensitive_classifiers);

	// we return the weights that correspond to each dimension of
	// the query, by taking into account which sensitive classifiers
	// are applicable to the query. query_matrix is the embedding
	// of the query. 
	vMatrix<float> QueryWeights(const v3dMatrix<float> * query_matrix);

	// a static version of the previous function.
	static vMatrix<float> QueryWeights(const v3dMatrix<float> * query_matrix,
		const vector<class_triple_classifier> * unique_classifiers,
		const vector<class_sensitive_classifier> * sensitive_classifiers);

	// we check if the two classifiers correspond to the same dimension
	// of the embedding. This means that we don't care if the weights
	// and z's are equal, but the type and objects must be equal.
	static vint8 check_classifiers(class_triple_classifier c1, 
		class_triple_classifier c2);

	// create triple classifiers out of the entries of a matrix.
	static vint8 MatrixToTclassifiers(vMatrix<float> c_matrix, 
		vector<class_triple_classifier> * unique_classifiers);

	// create a triple classifier out of the specified row of the matrix
	static class_triple_classifier classifier_from_matrix(vMatrix<float> c_matrix, 
		vint8 vertical);

	// create sensitive classifiers out of the entries of a matrix.
	static vint8 MatrixToSclassifiers(vMatrix<float> c_matrix, 
		vector<class_sensitive_classifier> * sensitive_classifiers);

	// adds classifier to unique_classifiers, in a way that avoids repetitions.
	// if classifier does not occur in unique classifiers, it gets added
	// to them. Otherwise, we find an occurrence of classifier in classifiers,
	// and we adjust the weight of that occurrence by adding to it the
	// weight of classifier.
	static vint8 clean_up_classifier(class_triple_classifier classifier,
		vector<class_triple_classifier> * unique_classifiers);

	vint8 ComputeAlphaLimits(vMatrix<float> results, 
		float * alpha_min, float * alpha_max);

	// Exponentiates the number according to distance_p
	inline float Power(float number)
	{
		switch(distance_p)
		{
		case 1:
			return number;
		case 2:
			return number * number;
		case 3:
			return number * number * number;
		default:
			return pow((float) number, (float) distance_p);
		}
	}

	inline vint8 SetDistanceP(vint8 in_distance_p)
	{
		if (in_distance_p <= 0)
		{
			return 0;
		}
		distance_p = in_distance_p;
		return 1;
	}

	inline vint8 DistanceP()
	{
		return distance_p;
	}

	// adds the specified reference object, with given weight
	vint8 AddReferenceObject(vint8 index, float weight);

	// adds the specified range of reference objects, with
	// given weight.
	vint8 AddRange(vint8 start_index, vint8 end_index, float weight);

	// Here we don't pass a weight as an argument. Instead, the
	// optimal weight is computed.
	vint8 AddWeightedReference(vint8 index);
	vint8 AddWeightedRange(vint8 start_index, vint8 end_index);

	// adds random references (useful for testing
	// the accuracy of choosing reference objects randomly
	// and with the same weight).
	vint8 RandomReferences2(vint8 number, float weight);

	// adds random unweighted references (useful for testing
	// the accuracy of choosing reference objects randomly
	// and with the same weight).
	vint8 RandomProjections2(vint8 number, float weight);

	// These functions are useful if we want to use arbitrary weak
	// classifiers, as opposed to using weak classifiers that
	// correspond to singleton-object Lipschitz embeddings or 
	// line projections. training_raw_results and validation_raw_results
	// simply store, at each row, the output of a weak classifier
	// on all training and validation triples (so it is the responsibility
	// of the caller to figure out what that output is). classifier_ids
	// contains, at position i, the ID of classifier whose results are 
	// stored in the i-th row of training_raw_results and 
	// validation_raw_results. This id is useful, because sometimes
	// we may not be able to fit into training_raw_results and 
	// validation_raw_results all weak classifiers. This way, we can load
	// some of them, and store their IDs in classifier_ids. steps simply
	// specifies the number of training steps we want to perform.
	vint8 SetClassifiers(vMatrix<float> in_training_raw_results, 
		vMatrix<float> in_validation_raw_results,
		vint8_matrix in_classifier_ids,
		float max_entry);

	vint8 NextSteps2(vint8 steps);

	float NextStep2();

	vint8 ClearClassifierResults();

	// converts raw results to classification accuracy results. raw
	// results are simply the outputs of the weak classifier. 
	// classification accuracy results are positive when the raw results 
	// are correct (same sign as true results), and negative otherwise. 
	// The input argument itself
	// is modified, so that we avoid creating a new matrix that can 
	// be potentially very large.
	vint8 ConvertRawResults(vMatrix<float> raw_results, 
		vMatrix<float> true_results);

	class_triple_classifier ClassifierRemoval(vMatrix<float> training_results,
		vint8_matrix classifier_ids,
		vint8 * success);

	class_triple_classifier ClassifierChange(vMatrix<float> training_results,
		vint8_matrix classifier_ids,
		vint8 * success);

	class_triple_classifier NextClassifier(vMatrix<float> training_results,
		vint8_matrix classifier_ids);

	float AddClassifier4(class_triple_classifier classifier, 
		vMatrix<float> training_results,
		vMatrix<float> validation_results,
		vint8_matrix classifier_ids);

	// this function can be called from the interpreter, when we want to
	// add some classifiers manually.
	float AddClassifier4b(vint8 id, float weight);

	vint8 ClassifierResults3(vint8 row, vMatrix<float> all_results,
		vector<float> * results);

	vint8 ClassifierResults4(vint8 classifier_id, vMatrix<float> all_results,
		vector<float> * results, vint8_matrix classifier_ids);

	vint8 FindClassifierRow(vint8 classifier_id, vint8_matrix classifier_ids);

	// This function modifies results, so that they don't reflect classification
	// accuracy with respect to proximity, but with respect to class labels. 
	// The conversion is simple: if distances and pdistances have a different 
	// sign, then we just negate the results.
	vint8 ResultsToPresults(vector<float> * results, vMatrix<float> distances,
		vMatrix<float> pdistances);

	// directory where SaveData saves the data.
	char * DataDirectory();
	static char * DataDirectory1(const char * root_data_dir, const char * dataset);

	// adds DataDirectory to name.
	char * DataPathname(const char * name);

	static char * DataPathname2(const char * dataset, const char * name);

	// Saves data (i.e. triples and distances) that we can later load, 
	// so that we can initialize
	// a BoostMap object.
	vint8 SaveData(const char * name);

	static char * MaxEntryPath(const char * training_dataset, 
		const char * triples_file,
		vint8 first, vint8 last);

	static vint8 RecordMaxEntry(const char * training_dataset, 
		const char * triples_file,
		vint8 first, vint8 last);

	static float ReadMaxEntry(const char * training_dataset, 
		const char * triples_file,
		vint8 first, vint8 last);

	const char * DatasetName();
	const char * TriplesFile();
	float DistanceMaxEntry()
	{
		return distance_max_entry;
	}

	vint8_matrix ReadTrainingTriples();
	vint8_matrix ReadValidationTriples();
	vint8 TrainingTripleNumber();
	vint8 ValidationTripleNumber();

	vint8 TripleResult(vint8 triple, vint8 classifier);
	vint8_matrix TrainingTriples()
	{
		return vint8_matrix(&training_triple_matrix, 0);
	}

	vint8_matrix ValidationTriples()
	{
		return vint8_matrix(&validation_triple_matrix, 0);
	}

	inline vint8_matrix TrainingIds()
	{
		return vint8_matrix(&training_ids_matrix);
	}

	inline vint8_matrix ValidationIds()
	{
		return vint8_matrix(&validation_ids_matrix);
	}

	vint8 load_classifier_c(const char * filename, vint8 dimensions);

	vint8 AddClassifierMatrix3(vMatrix<float> classifier_matrix, 
		vint8 dimensions);

	static float_matrix get_unique_classifiers (float_matrix classifiers);

	static float_matrix choose_first_dimensions (const char * classifier_name, float_matrix embedding, 
		vint8 dimensions, float_matrix * selected_classifiers_pointer);

	// for each classifier described by a row of selected_classifiers,
	// find its index in all_classifiers.  We assume that only one row
	// is relevant, otherwise the program exits with an error message.
	static vint8 find_classifier_indices(float_matrix selected_classifiers, float_matrix all_classifiers, 
		vector <vint8> * classifier_indices);

	// this function samples from the large set of training triples,
	// so that we can efficiently choose the next weak classifier.
	vint8 use_small_training();

	// this function sets as training triples the large set of training triples,
	// so that we can compute the weight of the next weak classifier.
	vint8 use_large_training();

	static vint8_matrix classified_triples(float_matrix margins);
	static vint8_matrix misclassified_triples(float_matrix margins);
	static double index_total(double_matrix values, vint8_matrix indices);

	// prints the sum of weights for the classified objects 
	// and the misclassified objects
	vint8 print_factor_totals();

	vint8 set_small_triple_number(vint8 new_number);

	// for query-aware experiment.
	// set cumulative sum of weights to factor for the specified label,
	vint8 set_distribution(vint8 class_label, float factor);
	vint8 print_class_total(vint8 class_label);

	static vint8 print_retrieval_results(vint8_matrix result);
};

// end of definition of class_BoostMap



// path names of files where we save output of weak classifiers on
// triples saved in triples_file.
char * vTrainingClassifiersPath(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last);


char * vValidationClassifiersPath(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last);








#endif // VASSILIS_BOOST_KDD_H
