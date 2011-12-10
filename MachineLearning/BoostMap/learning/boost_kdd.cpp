
#include "boost_kdd.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"

#include "basics/definitions.h"

#include "boost_kdd_utils.h"

class_BoostMap::class_BoostMap()
{
	Zero();
	class_name = vCopyString("class_BoostMap");
}


// we specify a training dataset, number of training triples,
// and whether we should optimize for feature-space distances
// or parameter-space distances.
class_BoostMap::class_BoostMap(const char * root_dir, const char * dataset_name, 
	vint8 in_training_triples, vint8 in_use_pdistances)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	data = new BoostMap_data(root_dir, dataset_name);
	if (data->valid() == 0)
	{
		return;
	}

	training_triple_number = in_training_triples;
	validation_triple_number = small_triple_number;
	use_pdistances = in_use_pdistances;

	large_training_triples = data->MakeTrainingTriples(training_triple_number);
	validation_triple_matrix = data->MakeValidationTriples(validation_triple_number);

	Initialize();
}


// here we pick triples that are focused, and more appropriate
// for nearest neighbor optimization, or for nearest neighbor
// classification.
class_BoostMap::class_BoostMap(const char * root_dir, const char * dataset_name, 
	vint8 in_training_triples, vint8 in_use_pdistances, vint8 classes, vint8 max_k)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	data = new BoostMap_data(root_dir, dataset_name);
	if (data->valid() == 0)
	{
		return;
	}

	use_pdistances = in_use_pdistances;
	training_triple_number = in_training_triples;
	validation_triple_number = small_triple_number;
	large_training_triples = data->MakeTrainingTriples2(training_triple_number,
		classes, max_k);
	validation_triple_matrix = data->MakeValidationTriples2(validation_triple_number,
		classes, max_k);

	Initialize();
}


// here we pick triples that are focused, and more appropriate
// for nearest neighbor optimization, or for nearest neighbor
// classification. Flag is used as an argument just to differentiate
// this constructor from the previous one. In the previous one 
// we call MakeTriples2, here we call MakeTriples3.
class_BoostMap::class_BoostMap(const char * root_dir, const char * dataset_name, 
	vint8 in_training_triples, vint8 in_use_pdistances, vint8 max_k)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	data = new BoostMap_data(root_dir, dataset_name);
	if (data->valid() == 0)
	{
		return;
	}

	use_pdistances = in_use_pdistances;
	training_triple_number = in_training_triples;
	validation_triple_number = small_triple_number;
	large_training_triples = data->MakeTrainingTriples3(training_triple_number,
		max_k);
	validation_triple_matrix = data->MakeValidationTriples3(validation_triple_number,
		max_k);

	Initialize();
}


// here we pick triples in which we specify how close a and b should
// be to q (min_a, max_a. min_b, max_b specify rank ranges), 
// but we don't care about class labels.
class_BoostMap::class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples, 
	vint8 min_a, vint8 max_a, vint8 min_b, vint8 max_b)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	data = new BoostMap_data(root_dir, dataset_name);
	if (data->valid() == 0)
	{
		return;
	}

	use_pdistances = 0;
	training_triple_number = in_training_triples;
	validation_triple_number = small_triple_number;

	large_training_triples = data->MakeTrainingTriples4(training_triple_number,
		min_a, max_a, min_b, max_b);
	validation_triple_matrix = data->MakeValidationTriples4(validation_triple_number,
		min_a, max_a, min_b, max_b);

	training_triple_number = large_training_triples.vertical();
	validation_triple_number = validation_triple_matrix.vertical();

	Initialize();
}


// here we pick triples in which we specify how close a and b should
// be to q (min_a, max_a. min_b, max_b specify rank ranges), 
// but we don't care about class labels. This constructor is a modification
// of the previous one, so that this one works with the protein dataset.
// junk is used as an argument just to make this constructor different
// than the previous one.
class_BoostMap::class_BoostMap(const char * root_dir, const char * dataset_name, vint8 in_training_triples, 
	vint8 min_a, vint8 max_a, vint8 min_b, vint8 max_b, vint8 junk)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	data = new BoostMap_data(root_dir, dataset_name);
	if (data->valid() == 0)
	{
		return;
	}

	use_pdistances = 0;
	training_triple_number = in_training_triples;
	validation_triple_number = small_triple_number;
	large_training_triples = data->MakeTrainingTriples5(training_triple_number,
		min_a, max_a, min_b, max_b);
	validation_triple_matrix = data->MakeValidationTriples5(validation_triple_number,
		min_a, max_a, min_b, max_b);

	Initialize();
}


class_BoostMap::class_BoostMap(const char * root_dir, const char * in_dataset_name, const char * in_saved_name, 
	vint8 in_training_triples, vint8 in_validation_triples,
	vint8 in_p_distances)
{
	Zero();
	class_name = vCopyString("class_BoostMap");
	triples_file = vCopyString(in_saved_name);
	data = new BoostMap_data(root_dir, in_dataset_name);
	if (data->valid() == 0)
	{
		return;
	}
	use_pdistances = in_p_distances;

	vint8 result = 1;
	char * pathname = DataPathname(in_saved_name);
	FILE * fp = fopen(pathname, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", pathname);
		vdelete2(pathname);
		return;
	}

	vdelete2(pathname);

	vint8_matrix temp;
	vMatrix<float> tempf;

	// read training triples
	temp = vint8_matrix::Read(fp);
	if (temp.valid() <= 0)
	{
		vPrint("failed to load training triples\n");
		result = 0;
	}
	if (temp.Rows() < in_training_triples)
	{
		vPrint("only %li triples read\n", (long) temp.Rows());
		result = 0;
		in_training_triples = temp.Rows();
	}
	training_triple_number = in_training_triples;
	large_training_triples = vint8_matrix(training_triple_number, 3);
	vCopyRectangle2(&temp, &large_training_triples, 0, 0, 
		(long) training_triple_number, 3, 0, 0);

	// read training distances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load training distances\n");
		result = 0;
	}
	if (tempf.Cols() < training_triple_number)
	{
		vPrint("only %li triples read\n", (long) tempf.Cols());
		result = 0;
	}
	large_training_distances = vMatrix<float>(1, training_triple_number);
	vCopyRectangle2(&tempf, &large_training_distances, 0, 0, 
		1, (long) training_triple_number, 0, 0);

	// read training pdistances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load training pdistances\n");
		result = 0;
	}
	if (tempf.Cols() < training_triple_number)
	{
		vPrint("only %li triples read\n", (long) tempf.Cols());
		result = 0;
	}
	large_training_pdistances = vMatrix<float>(1, training_triple_number);
	vCopyRectangle2(&tempf, &large_training_pdistances, 0, 0, 
		1, (long) training_triple_number, 0, 0);

	// read validation triples
	temp = vint8_matrix::Read(fp);
	if (temp.valid() <= 0)
	{
		vPrint("failed to load validation triples\n");
		result = 0;
	}
	if (temp.Rows() < in_validation_triples)
	{
		vPrint("only %li triples read\n", (long) temp.Rows());
		result = 0;
		in_validation_triples = temp.Rows();
	}
	validation_triple_number = in_validation_triples;
	validation_triple_matrix = vint8_matrix(validation_triple_number, 3);
	vCopyRectangle2(&temp, &validation_triple_matrix, 0, 0, 
		(long) validation_triple_number, 3, 0, 0);

	// read validation distances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load validation distances\n");
		result = 0;
	}
	if (tempf.Cols() < validation_triple_number)
	{
		vPrint("only %li triples read\n", (long) tempf.Cols());
		result = 0;
	}
	validation_distances = vMatrix<float>(1, validation_triple_number);
	vCopyRectangle2(&tempf, &validation_distances, 0, 0, 
		1, (long) validation_triple_number, 0, 0);

	// read validation pdistances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load validation pdistances\n");
		result = 0;
	}
	if (tempf.Cols() < validation_triple_number)
	{
		vPrint("only %li triples read\n", (long) tempf.Cols());
		result = 0;
	}
	validation_pdistances = vMatrix<float>(1, validation_triple_number);
	vCopyRectangle2(&tempf, &validation_pdistances, 0, 0, 
		1, (long) validation_triple_number, 0, 0);


	fclose(fp);
	if (result > 0)
	{
		Initialize2();
	}
}

class_BoostMap::~class_BoostMap()
{
	vdelete2(class_name);
	vdelete2(triples_file);
	vdelete(data);
	Zero();
}


vint8 class_BoostMap::Initialize()
{
	large_triple_number = training_triple_number;
	use_large_training();
	if ((training_triple_number < 0) || (validation_triple_number < 0))
	{
		return 0;
	}
	if ((training_triple_matrix.valid() <= 0) || 
		(validation_triple_matrix.valid() <= 0))
	{
		return 0;
	}

	// get ids of different sets from the dataset object.
	candidate_ids_matrix = data->candidate_objects();
	training_ids_matrix = data->training_objects();
	validation_ids_matrix = data->validation_objects();
	candidate_number = candidate_ids_matrix.Size();
	training_number = training_ids_matrix.Size();

	validation_number = validation_ids_matrix.Size();

	// load the required distance matrices.
	candcand_distances_matrix = data->LoadCandCandDistances();
	candtrain_distances_matrix = data->LoadCandTrainDistances();
	candval_distances_matrix = data->LoadCandValDistances();

	// normalize entries, so that max in candtrain entry is 1.
	distance_max_entry = function_image_maximum(&candtrain_distances_matrix);
	//  distance_max_entry = 1.0f;
	vPrint("max_entry = %f\n", distance_max_entry);
	vint8 i, size;
	size = candcand_distances_matrix.Size();

	large_training_distances = data->TripleTrainingDistances(training_triple_matrix);
	if (large_training_distances.valid() == 0)
	{
		return 0;
	}
	validation_distances = data->TripleValidationDistances(validation_triple_matrix);
	for (i = 0; i < size; i++)
	{
		candcand_distances_matrix(i) /= distance_max_entry;
	}
	size = candtrain_distances_matrix.Size();
	for (i = 0; i < size; i++)
	{
		candtrain_distances_matrix(i) /= distance_max_entry;
	}
	size = candval_distances_matrix.Size();
	for (i = 0; i < size; i++)
	{
		candval_distances_matrix(i) /= distance_max_entry;
	}
	size = large_training_distances.Size();
	for (i = 0; i < size; i++)
	{
		large_training_distances(i) /= distance_max_entry;
	}
	size = validation_distances.Size();
	for (i = 0; i < size; i++)
	{
		validation_distances(i) /= distance_max_entry;
	}

	// get parameter-space distances for training and validation set.
	large_training_pdistances = data->TripleTrainingPdistances(training_triple_matrix);
	validation_pdistances = data->TripleValidationPdistances(validation_triple_matrix);

	// mark whether we want to use feature-space or parameter-space distance.
	if (use_pdistances == 1)
	{
		if (large_training_pdistances.valid() <= 0)
		{
			return 0;
		}
		large_ground_truth = large_training_pdistances;
	}
	else if (use_pdistances == 0)
	{
		if (large_training_distances.valid() <= 0)
		{
			return 0;
		}
		large_ground_truth = large_training_distances;
	}
	else
	{
		return 0;
	}

	number_of_picked_candidates = Min<vint8>(candidate_number, 1000);
	projection_candidate_number = number_of_picked_candidates;

	InitializeMatrices();
	SetAllArrays();

	is_valid = 1;

	return is_valid;
}


// to be used with the constructor that reads training triples from disk.
vint8 class_BoostMap::Initialize2()
{
	large_triple_number = training_triple_number;
	use_large_training();
	if ((training_triple_number < 0) || (validation_triple_number < 0))
	{
		return 0;
	}
	if ((training_triple_matrix.valid() <= 0) || 
		(validation_triple_matrix.valid() <= 0))
	{
		return 0;
	}

	// get ids of different sets from the dataset object.
	training_ids_matrix = data->training_objects();
	validation_ids_matrix = data->validation_objects();
	candidate_number = 0;
	training_number = training_ids_matrix.Size();
	validation_number = validation_ids_matrix.Size();

	// mark whether we want to use feature-space or parameter-space distance.
	if (use_pdistances == 1)
	{
		if (large_training_pdistances.valid() <= 0)
		{
			return 0;
		}
		large_ground_truth = large_training_pdistances;
	}
	else if (use_pdistances == 0)
	{
		if (large_training_distances.valid() <= 0)
		{
			return 0;
		}
		large_ground_truth = large_training_distances;
	}
	else
	{
		return 0;
	}

	number_of_picked_candidates = 1000;
	projection_candidate_number = 1000;

	InitializeMatrices();
	SetAllArrays();

	is_valid = 1;
	allow_lipschitz = 0;
	allow_projections = 0;
	allow_sensitive = 0;
	new_sensitive = 0;

	return is_valid;
}


// assign default values to member variables.
vint8 class_BoostMap::Zero()
{
	class_name = 0;
	data = 0;
	triples_file = 0;
	is_valid = 0;

	candidate_number = 0;
	training_number = 0;
	validation_number = 0;

	training_triple_number = 0;
	small_triple_number = 100000;
	validation_triple_number = 0;

	number_of_picked_candidates = 0;
	projection_candidate_number = 0;
	allow_projections = 1;
	allow_lipschitz = 1;
	allow_negative = 0;
	allow_removals = 1;
	allow_sensitive = 0;
	new_sensitive = 0;
	use_pdistances = 0;
	use_best_k = 200;

	// By default we produce an L1 distance measure.
	distance_p = 1;

	min_exp = (float) 1000000000; // one billion
	max_exp = (float) -1000000000;

	last_error = -1;
	last_et = -1;
	last_correlation = -1;
	last_scaled_et = -1;
	last_scaled_correlation = -1;

	last_perror = -1;
	last_pet = -1;
	last_pcorrelation = -1;
	last_scaled_pet = -1;
	last_scaled_pcorrelation = -1;

	last_new_z = 0;

	training_error = -1;
	training_margin = -1;
	validation_error = -1;
	training_perror = -1;
	validation_perror = -1;

	distance_max_entry = (float)-1.0;

	SetAllArrays();
	return 1;
}


// set all training weights to initial values (equal to each other,
// and summing to 1).
vint8 class_BoostMap::InitialWeights()
{
	if (training_triple_number == 0)
	{
		return 1;
	}
	large_training_factors = vMatrix<double>(1, training_triple_number);
	training_factors = large_training_factors.Matrix();

	float weight = ((float) 1) / (float) training_triple_number;
	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		training_factors[i] = weight;
	}

	return 1;
}


// initialize training and validation margins and pmargins, and
// initialize weights.
vint8 class_BoostMap::InitializeMatrices()
{
	InitialWeights();

	large_training_margins = vMatrix<float>(1, training_triple_number);
	function_enter_value(&large_training_margins, (float) 0);
	validation_margins_matrix = vMatrix<float>(1, validation_triple_number);
	function_enter_value(&validation_margins_matrix, (float) 0);

	large_training_pmargins = vMatrix<float>(1, training_triple_number);
	function_enter_value(&large_training_pmargins, (float) 0);
	validation_pmargins_matrix = vMatrix<float>(1, validation_triple_number);
	function_enter_value(&validation_pmargins_matrix, (float) 0);

	use_large_training();
	return 1;
}


vint8 class_BoostMap::SetAllArrays()
{
	candidate_ids = candidate_ids_matrix.flat();
	candidates = candidates_matrix.flat();
	candcand_distances = candcand_distances_matrix.planar_safe();
	candtrain_distances = candtrain_distances_matrix.planar_safe();
	candval_distances = candval_distances_matrix.planar_safe();

	training_ids = training_ids_matrix.flat();
	training_factors = training_factors_matrix.flat();
	training_triples = training_triple_matrix.planar_safe();

	validation_ids = validation_ids_matrix.flat();
	validation_triples = validation_triple_matrix.planar_safe();

	return 1;
}


const char * class_BoostMap::get_class_name()
{
	return class_name;
}

// we assume that the input matrix classifiers is a matrix
// describing the classifiers chosen in each training round.
// here we want to get all the unique classifiers appearing
// in that matrix, even if, at the end, some of them may have a weight of zero.
// The reason is that we want to use these unique classifiers
// to create and store to disk an embedding from which we can recover
// all corresponding lower dimensional embeddings.  A unique classifier
// having a total zero weight for the final dimensionality
// may have a nonzero weight for one of the lower dimensionalities.
float_matrix class_BoostMap::get_unique_classifiers(float_matrix classifiers)
{
	class_BoostMap BoostMap;
	vint8 number = classifiers.vertical();

	vint8 counter;
	for (counter = 0; counter <number; counter ++)
	{
		class_triple_classifier classifier = class_BoostMap::classifier_from_matrix(classifiers, counter);

		// we set the weight to one to make sure that the classifier will not be removed.
		classifier.weight = 1;
		BoostMap.clean_up_classifier(classifier);
	}

	float_matrix result = BoostMap.ClassifierMatrix ();
	return result;
}

// chooses, from all candidate objects, a random subset which will
// be used to define the candidate weak classifiers for the next
// training step.
vint8 class_BoostMap::ChooseCandidates(vint8 in_pick_number)
{
	// Is this a bug? Should I check whether 
	// in_pick_number > candidate_number?
	if (in_pick_number > candidate_number)
	{
		in_pick_number = candidate_number;
	}
	vint8_matrix temp_candidates_matrix = sample_without_replacement(0, candidate_number - 1, 
		in_pick_number);
	candidates_matrix = vint8_matrix(&temp_candidates_matrix);
	candidates = candidates_matrix.Matrix();
	return 1;
}


// pick the next weak classifier.
float class_BoostMap::NextStep()
{
	if ((is_valid <= 0) || (allow_sensitive > 0))
	{
		return 0;
	}

	class_triple_classifier new_classifier;
	vint8 success = 0, success1 = 0, success2 = 0;

	// first, check if it is beneficial to remove an already chosen
	// classifier.
	if (allow_removals != 0)
	{
		// get "best" classifiers to remove, for each classifier type.
		class_triple_classifier lipschitz_remove = LipschitzRemoval(&success1);
		class_triple_classifier projection_remove = ProjectionRemoval(&success2);

		// if no good classifier to remove was found, we don't remove
		// any.
		if ((success1 <= 0) && (success2 <= 0)) 
		{
		}
		// otherwise we find the best classifier to remove
		else
		{
			// figure out which of the two classifiers is more beneficial
			// to remove.
			success = 1;
			if ((success1 > 0) && (success2 > 0))
			{
				if (lipschitz_remove.z < projection_remove.z)
				{
					new_classifier = lipschitz_remove;
				}
				else
				{
					new_classifier = projection_remove;
				}
			}
			else if (success1 > 0)
			{
				new_classifier = lipschitz_remove;
			}
			else if (success2 > 0)
			{
				new_classifier = projection_remove;
			}
			else
			{
				exit_error("Error: we should never get here\n");
			}
		}
		if (success > 0)
		{
			vPrint("removing: ");
			new_classifier.Print();
		}
		// if no classifier is getting removed, check whether we 
		// can find a classifier for which it would be beneficial
		// to update the weight.
		else
		{
			// get most beneficial weight changes for each type.
			class_triple_classifier lipschitz_changed = LipschitzChange(&success1);
			class_triple_classifier projection_changed = ProjectionChange(&success2);

			// check if any weight change is beneficial, and if so, which
			// one (of the two possible candidates) is the most beneficial.
			if ((success1 <= 0) && (success2 <= 0)) 
			{
			}
			else
			{
				success = 1;
				if ((success1 > 0) && (success2 > 0))
				{
					if (lipschitz_changed.z < projection_changed.z)
					{
						new_classifier = lipschitz_changed;
					}
					else
					{
						new_classifier = projection_changed;
					}
				}
				else if (success1 > 0)
				{
					new_classifier = lipschitz_changed;
				}
				else if (success2 > 0)
				{
					new_classifier = projection_changed;
				}
				else
				{
					exit_error("Error: we should never get here\n");
				}
			}

			if (success > 0)
			{
				vPrint("modifying: last_new_z = %f, cutoff = %f\n", 
					last_new_z, ChangeCutoff());
				new_classifier.Print();
			}
		}
	}

	// if we neither remove classifiers nor modify weights, then
	// we will look to find the best weak classifier to add to 
	// existing classifiers.
	if (success == 0)
	{
		// find the best classifier for each type.
		class_triple_classifier lipschitz = NextLipschitz();

		// choose the best among the two candidates.
		class_triple_classifier projection = NextProjection();
		if (lipschitz.z <= projection.z)
		{
			new_classifier = lipschitz;
		}
		else
		{
			new_classifier = projection;
		}
		last_new_z = new_classifier.z;
	}

	// Add the new weak classifier, which, depending on
	// what happened earlier in the function, could lead
	// to removal of a classifier, weight update, or 
	// adding in a new classifier.
	float margin = (float) AddClassifier(new_classifier);
	return margin;
}


// An alternative way to pick the next weak classifier. 
// Here we first compute just the training errors for each candidate, 
// and then we only minimize z for the use_best_k of those candidates.
// this function simply set a small number of training triples,
// calls fast_next_step_actual, and then sets back the original large 
// number of training triples.
float class_BoostMap::fast_next_step()
{
	use_small_training();
	float result = fast_next_step_actual();
	use_large_training();

	return result;
}


// this function actually does the work of fast_next_step
float class_BoostMap::fast_next_step_actual()
{
	use_small_training();
	if ((is_valid <= 0) || (allow_sensitive > 0))
	{
		return 0;
	}

	class_triple_classifier new_classifier;
	vint8 success = 0, success1 = 0, success2 = 0;

	// for removals and weight updates, it is the same as in NextStep,.
	if (allow_removals != 0)
	{
		class_triple_classifier lipschitz_remove = LipschitzRemoval(&success1);
		class_triple_classifier projection_remove = ProjectionRemoval(&success2);
		if ((success1 <= 0) && (success2 <= 0)) 
		{
		}
		else
		{
			success = 1;
			if ((success1 > 0) && (success2 > 0))
			{
				if (lipschitz_remove.z < projection_remove.z)
				{
					new_classifier = lipschitz_remove;
				}
				else
				{
					new_classifier = projection_remove;
				}
			}
			else if (success1 > 0)
			{
				new_classifier = lipschitz_remove;
			}
			else if (success2 > 0)
			{
				new_classifier = projection_remove;
			}
			else
			{
				exit_error("Error: we should never get here\n");
			}
		}
		if (success > 0)
		{
			vPrint("removing: ");
			new_classifier.Print();
		}
		else
		{
			class_triple_classifier lipschitz_changed = LipschitzChange(&success1);
			class_triple_classifier projection_changed = ProjectionChange(&success2);
			if ((success1 <= 0) && (success2 <= 0)) 
			{
			}
			else
			{
				success = 1;
				if ((success1 > 0) && (success2 > 0))
				{
					if (lipschitz_changed.z < projection_changed.z)
					{
						new_classifier = lipschitz_changed;
					}
					else
					{
						new_classifier = projection_changed;
					}
				}
				else if (success1 > 0)
				{
					new_classifier = lipschitz_changed;
				}
				else if (success2 > 0)
				{
					new_classifier = projection_changed;
				}
				else
				{
					exit_error("Error: we should never get here\n");
				}
			}

			if (success > 0)
			{
				vPrint("modifying: last_new_z = %f, cutoff = %f\n", 
					last_new_z, ChangeCutoff());
				new_classifier.Print();
			}
		}
	}

	// here is where we differ from NextStep, i.e. in the case
	// where we pick a new classifier.
	if (success == 0)
	{
		vector<class_triple_classifier> candidates;
		// pick random weak classifiers of both types, to evaluate.
		RandomCandidates(&candidates);
		vector<class_triple_classifier> selected;

		// select the use_best_k classifiers that give the 
		// lowest weighted training error.
		SelectBest(&candidates, &selected, use_best_k);
		if (selected.size() == 0) 
		{
			return 0;
		}

		// now go through the selected classifiers, and find the best
		// one among them, based on their Z.
		vint8 best_index = 0;
		float best_alpha = 0, alpha = 0;
		float min_z = ClassifierZ(&(selected[0]), &best_alpha);
		vint8 i;
		vPrint("\n");
		for (i = 1; i < (vint8) selected.size(); i++)
		{
			float z = ClassifierZ(&(selected[(vector_size) i]), &alpha);
			if (z < min_z)
			{
				best_index = i;
				min_z = z;
				best_alpha = alpha;
			}
			vPrint("measured z for classifier %li of %li\r", (long) (i+1), (long) selected.size());
		}
		vPrint("\n");
		new_classifier = selected[(long) best_index];
		new_classifier.weight = best_alpha;
		new_classifier.z = min_z;
		last_new_z = new_classifier.z;
	}

	use_large_training();
	float margin = (float) AddClassifier(new_classifier);
	return margin;
}


// This function just calls NextStep or FastNextStep. Right now
// it is hardcoded, I should probably have a member variable
// based on which we should decide.
float class_BoostMap::NextSteps(vint8 steps)
{
	vint8 i;
	float error;
	for (i = 0; i < steps; i++)
	{
		error = fast_next_step();
	}

	return error;
}


// check how many training rounds we have performed (excluding
// query-sensitive training).
vint8 class_BoostMap::StepsDone()
{
	vint8 result = classifiers.size();
	return result;
}


// find the best z and alpha (weight) for the specified
// line projection embedding.
float class_BoostMap::PivotPairZ(vint8 pivot1, vint8 pivot2, float * alpha)
{
	float distance = candcand_distances[pivot1][pivot2];
	if (distance < 0.00001) 
	{
		*alpha = 0;
		return 1000000;
	}

	vector<float> results((vector_size) training_triple_number);
	PivotPairResults(pivot1, pivot2, &results, 
		training_triple_matrix, ground_truth,
		candtrain_distances_matrix);
	float a = 0, z = 0;
	iterations = iterations + vas_int16((long) MinimizeZ(&results, &z, &a));
	*alpha = a;
	return z;
}


// get the results of the classifier specified by pivot1 and
// pivot2 on training or validation triples (passed in as
// triple_matrix). results is assumed to have enough space
// to store the results. The result for each triple is 
// the output of the classifier times the class label for the
// triple. candset_distances_matrix should be
// either candtrain_distances_matrix
// or candval_distances_matrix, depending on triple_matrix.
vint8 class_BoostMap::PivotPairResults(vint8 pivot1, vint8 pivot2, vector<float> * results,
	vint8_matrix triple_matrix, 
	vMatrix<float> triple_distances_matrix,
	vMatrix<float> candset_distances_matrix)
{
	vint8 number = triple_matrix.Rows();

	vint8 i;
	// get the distance between the two pivots.
	float pivot_distance = candcand_distances[pivot1][pivot2];
	vArray2(vint8) triples = triple_matrix.Matrix2();
	vArray(float) triple_distances = triple_distances_matrix.Matrix();
	vArray2(float) candset_distances = candset_distances_matrix.Matrix2();
	for (i = 0; i < number; i++)
	{
		vint8 q = triples[i][0];
		vint8 a = triples[i][1];
		vint8 b = triples[i][2];

		// get the class label for the i-th triple.
		float label = Label(triple_distances[i]);

		// get distances of q, a, b, from two pivots.
		float qp1 = candset_distances[pivot1][q];
		float ap1 = candset_distances[pivot1][a];
		float bp1 = candset_distances[pivot1][b];
		float qp2 = candset_distances[pivot2][q];
		float ap2 = candset_distances[pivot2][a];
		float bp2 = candset_distances[pivot2][b];

		// embed q, a, b.
		float qr = V_FastMap::LineProjection3(qp1, qp2, pivot_distance);
		float ar = V_FastMap::LineProjection3(ap1, ap2, pivot_distance);
		float br = V_FastMap::LineProjection3(bp1, bp2, pivot_distance);

		// compute the classification result for the i-th triple (notice
		// that we multiply by label).
		(*results)[(vector_size) i] = (Power(vAbs(qr - br)) - Power(vAbs(qr - ar))) * label;
	}

	return 1;
}


// same as PivotPairResults, but for a reference-object type of
// classifier, using the index-th candidate as the reference object.
vint8 class_BoostMap::LipschitzResults(vint8 index, vector<float> * results,
	vint8_matrix triple_matrix, 
	vMatrix<float> triple_distances_matrix,
	vMatrix<float> candset_distances_matrix)
{
	vint8 number = triple_matrix.Rows();

	vint8 i;
	vArray2(vint8) triples = triple_matrix.Matrix2();
	vArray(float) triple_distances = triple_distances_matrix.Matrix();
	vArray2(float) candset_distances = candset_distances_matrix.Matrix2();
	vint8 set_size = candset_distances_matrix.Cols();
	for (i = 0; i < number; i++)
	{
		vint8 q = triples[i][0];
		vint8 a = triples[i][1];
		vint8 b = triples[i][2];

		if ((q < 0) || (a < 0) || (b < 0) ||
			(q >= set_size) || (a >= set_size) || (b >= set_size))
		{
			exit_error("Error: (q,a,b) = (%li ,%li ,%li), set_size = %li\n",
				(long) q, (long) a, (long) b, (long) set_size);
		}

		float label = Label(triple_distances[i]);

		float qr = candset_distances[index][q];
		float ar = candset_distances[index][a];
		float br = candset_distances[index][b];

		(*results)[(vector_size) i] = (Power(vAbs(qr - br)) - Power(vAbs(qr - ar))) * label;
	}

	return 1;
}


// get the best z and alpha for the given weak classifier.
float class_BoostMap::ClassifierZ(class_triple_classifier * classifier, float * alpha)
{
	switch(classifier->type)
	{
	case 0:
		return ClassifierZ(classifier->object1, alpha);
		break;

	case 1:
		return PivotPairZ(classifier->object1, classifier->object2, alpha);
		break;
	}

	exit_error("Error in ClassifierZ: We should never get here\n");
	return 0;
}


// similar to PivotPairResults and LipschitzResults, but here
// we can handle both types of classifiers (line projections
// and reference-object-based).
vint8 class_BoostMap::ClassifierResults(class_triple_classifier * classifier, 
	vector<float> * results,
	vint8_matrix triple_matrix, 
	vMatrix<float> triple_distances_matrix,
	vMatrix<float> candset_distances_matrix)
{
	switch(classifier->type)
	{
	case 0:
		return LipschitzResults(classifier->object1, results,
			triple_matrix, triple_distances_matrix,
			candset_distances_matrix);
		break;

	case 1:
		return PivotPairResults(classifier->object1, classifier->object2, results,
			triple_matrix, triple_distances_matrix,
			candset_distances_matrix);
		break;
	}

	exit_error("Error in ClassifierResults: We should never get here\n");
	return 0;
}


// It finds the lipschitz classifier that 
// by removing it we attain the best accuracy
class_triple_classifier class_BoostMap::LipschitzRemoval(vint8 * success)
{
	vint8 z_min_index = 0;
	float z_min = 0, best_alpha = 0;
	*success = TryRemovals(&z_min_index, &z_min, &best_alpha);
	class_triple_classifier result((long) z_min_index, best_alpha, z_min);
	return result;
}


// Return zero if no removal was successful, one otherwise.
// Only tries removals of lipschitz types. If successful,
// it stores the z attained by removing the classifier, the
// weight that will set the global weight of that classifier to 0,
// and the index of the reference object in the set of candidates.
vint8 class_BoostMap::TryRemovals(vint8 * z_min_indexp, float * z_minp, float * alphap)
{
	vint8 steps = unique_classifiers.size();
	if (steps == 0) return 0;

	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_index = 0;
	vector<float> results((vector_size) training_triple_number);
	vint8 i;
	for (i = 0; i < steps; i++)
	{
		// in this function we only consider Lipschitz-type classifiers.
		if (unique_classifiers[(vector_size) i].type != 0)
		{
			continue;
		}

		// get the Z corresponding to removing this classifier.
		vint8 index = unique_classifiers[(vector_size) i].object1;
		float weight = unique_classifiers[(vector_size) i].weight;
		LipschitzResults(index, &results, training_triple_matrix, 
			ground_truth, candtrain_distances_matrix);
		float z = (float) Z(-weight, &results);

		// check if this is the best Z seen so far (among candidate
		// removals).
		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = -weight;
			min_z_index = index;
		}
	}

	*z_min_indexp = min_z_index;
	*z_minp = min_z;
	*alphap = min_z_alpha;

	// if min_z is less than 1, then removing the corresponding classifier
	// is beneficial, and should improve the training error.
	if (min_z < 1)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// It finds the best weight modification for already
// selected lipschitz classifiers
class_triple_classifier class_BoostMap::LipschitzChange(vint8 * success)
{
	vint8 z_min_index = 0;
	float z_min = 0, best_alpha = 0;
	TryWeightChange(&z_min_index, &z_min, &best_alpha);
	float z_cutoff = ChangeCutoff();
	if (z_min <= z_cutoff) 
	{
		*success = 1;
	}
	else
	{
		*success = 0;
	}

	class_triple_classifier result((long) z_min_index, best_alpha, z_min);
	return result;
}


// It finds the best weight modification for already
// selected lipschitz classifiers. It returns the same things as 
// TryRemovals.
vint8 class_BoostMap::TryWeightChange(vint8 * z_min_indexp, 
	float * z_minp, float * alphap)
{
	vPrint("Trying weight change for reference objects:\n");
	*z_minp = 2;
	vint8 steps = unique_classifiers.size();
	if (steps == 0) return 0;

	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_index = 0;
	vector<float> results((vector_size) training_triple_number);
	vint8 i;
	vPrint("\n");
	for (i = 0; i < steps; i++)
	{
		// this function only looks at Lipschitz-type embeddings.
		if (unique_classifiers[(vector_size) i].type != 0)
		{
			continue;
		}

		// get the best weight for this classifier.
		vint8 index = unique_classifiers[(vector_size) i].object1;
		float weight = unique_classifiers[(vector_size) i].weight;
		LipschitzResults(index, &results, training_triple_matrix,
			ground_truth, candtrain_distances_matrix);
		float z = 0, alpha = 0;
		MinimizeZ(&results, &z, &alpha);

		// verify that, if we use this weight, the total sum
		// will be non-negative.
		if (((weight + alpha) < 0) && (allow_negative == 0))
		{
			continue;
		}

		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = alpha;
			min_z_index = index;
		}
		vPrint("evaluated lipschitz classifier %li of %li\r",
			(long) (i+1), (long) steps);
	}
	vPrint("\n");

	*z_min_indexp = min_z_index;
	*z_minp = min_z;
	*alphap = min_z_alpha;

	// although, in theory, we should check if min_z is less than 1, we 
	// want to avoid weight updates that are numerically insignificant,
	// so I  use .9999 as an empirically determined arbitrary threshold.
	if (min_z < .9999)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


// It finds the best Lipschitz weak classifier for the
// current training step
class_triple_classifier class_BoostMap::NextLipschitz()
{
	if (is_valid <= 0) 
	{
		return class_triple_classifier();
	}

	float z_min = (float) 10;
	vint8 z_min_index = -1;
	float best_alpha = (float) 0;
	if (number_of_picked_candidates == 0) 
	{
		vPrint("no candidate classifiers are available\n");
		return class_triple_classifier();
	}

	// choose candidate reference objects.
	ChooseCandidates(number_of_picked_candidates);

	// initialize alpha and z_min.
	float alpha = 0;
	z_min = ClassifierZ(candidates[0], &best_alpha);

	z_min_index = candidates[0];
	vint8 i;
	vPrint("\n");

	// go through all candidates.
	for (i = 1; i < candidates_matrix.Size(); i++)
	{
		// get the z for current candidate, check if 
		// it is the best (smallest) seen so far.
		float z = ClassifierZ(candidates[i], &alpha);
		if (z < z_min)
		{
			z_min = z;
			z_min_index = candidates[i];
			best_alpha = alpha;
		}
		vPrint("evaluated lipschitz classifier %li of %li\r",
			(long) (i+1), (long) candidates_matrix.Size());
	}
	vPrint("\n");
	class_triple_classifier result((long) z_min_index, best_alpha, z_min);
	return result;
}


// It finds the projection classifier such that 
// by removing it we attain the best accuracy
class_triple_classifier class_BoostMap::ProjectionRemoval(vint8 * success)
{
	*success = 0;
	vint8 steps = unique_classifiers.size();
	if (steps == 0) return class_triple_classifier();

	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_pivot1 = -1, min_z_pivot2 = -1;
	vector<float> results((vector_size) training_triple_number);
	vint8 i;

	// go through all already chosen classifiers.
	for (i = 0; i < steps; i++)
	{
		// this function only looks at "projection"-type classifiers.
		class_triple_classifier & classifier = unique_classifiers[(vector_size) i];
		if (classifier.type != 1)
		{
			continue;
		}

		float weight = classifier.weight;
		PivotPairResults(classifier.object1, classifier.object2, &results,
			training_triple_matrix, ground_truth,
			candtrain_distances_matrix);

		// evaluate how good removing this classifier is.
		float z = (float) Z(-weight, &results);
		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = -weight;
			min_z_pivot1 = classifier.object1;
			min_z_pivot2 = classifier.object2;
		}
	}

	class_triple_classifier result((long) min_z_pivot1, (long) min_z_pivot2, min_z_alpha, min_z);
	if (min_z < 1)
	{
		*success = 1;
	}
	else
	{
		*success = 0;
	}

	return result;
}


// It finds the best weight modification for already
// selected projection classifiers
class_triple_classifier class_BoostMap::ProjectionChange(vint8 * success)
{
	vPrint("Trying weight change for projections:\n");
	*success = 0;
	vint8 steps = unique_classifiers.size();
	if (steps == 0) return class_triple_classifier();

	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_pivot1 = 0, min_z_pivot2 = 0;
	vector<float> results((vector_size) training_triple_number);
	vint8 i;
	vPrint("\n");
	for (i = 0; i < steps; i++)
	{
		class_triple_classifier classifier = unique_classifiers[(vector_size) i];
		if (classifier.type != 1) 
		{
			continue;
		}

		// find the best z and alpha for the current classifier.
		float weight = classifier.weight;
		PivotPairResults(classifier.object1, classifier.object2, &results,
			training_triple_matrix, ground_truth,
			candtrain_distances_matrix);
		float z = 0, alpha = 0;
		MinimizeZ(&results, &z, &alpha);

		// check if updating the weight will cause the total
		// weight to be negative. In that case, we don't do it
		// if negative weights are not allowed. Note that, in principle,
		// this is not the best thing to check for. Instead, we should
		// have found the allowed range for weight, and we should
		// have found the weight that minimizes z, from that range. The way
		// we do it now, there may be an alternative weight that achieves
		// a very good z, and that does not lead to a total negative weight,
		// but we never consider that.
		if (((weight + alpha) < 0) && (allow_negative == 0))
		{
			continue;
		}

		// check if this is the best z seen so far.
		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = alpha;
			min_z_pivot1 = classifier.object1;
			min_z_pivot2 = classifier.object2;
		}
		vPrint("evaluated projection classifier %li of %li\r",
			(long) (i+1), (long) steps);
	}
	vPrint("\n");

	class_triple_classifier result((long) min_z_pivot1, (long) min_z_pivot2, min_z_alpha, min_z);
	float z_cutoff = ChangeCutoff();
	if (min_z <= z_cutoff) 
	{
		*success = 1;
	}
	else
	{
		*success = 0;
	}

	return result;
}


// It finds the best projection weak classifier for the
// current training step
class_triple_classifier class_BoostMap::NextProjection()
{
	if (is_valid <= 0) return class_triple_classifier();
	if (candidate_number == 0) 
	{
		vPrint("no candidate classifiers are available\n");
		return class_triple_classifier();
	}

	// pivots are chosen randomly from all candidates.
	vint8_matrix temp_candidates1 = sample_without_replacement(0, candidate_number - 1, 
		candidate_number);
	vint8_matrix temp_candidates(&temp_candidates1);

	// choose the candidate projection embeddings (i.e. choose random
	// pairs of pivots).
	vint8_matrix candidate_pivots_matrix = RandomPivots(temp_candidates, 
		projection_candidate_number);
	vArray2(vint8) candidate_pivots = candidate_pivots_matrix.Matrix2();

	float z_min = (float) 10;
	vint8 z_min_pivot1 = -1, z_min_pivot2 = -1;
	float best_alpha = (float) 0;

	float alpha = 0;
	z_min = PivotPairZ(candidate_pivots[0][0], candidate_pivots[0][1], 
		&best_alpha);
	z_min_pivot1 = candidate_pivots[0][0];
	z_min_pivot2 = candidate_pivots[0][1];
	vint8 i;
	vPrint("\n");

	// go through the candidate pivot pairs, and find the best one.
	for (i = 1; i < projection_candidate_number; i++)
	{
		float z = PivotPairZ(candidate_pivots[i][0], candidate_pivots[i][1],
			&alpha);
		if (z < z_min)
		{
			z_min = z;
			z_min_pivot1 = candidate_pivots[i][0];
			z_min_pivot2 = candidate_pivots[i][1];
			best_alpha = alpha;
		}
		vPrint("evaluated projection classifier %li of %li\r",
			(long) (i+1), (long) projection_candidate_number);
	}
	vPrint("\n");

	class_triple_classifier result((long) z_min_pivot1, (long) z_min_pivot2, best_alpha, z_min);
	return result;
}


// returns a number x 2 matrix, where every row is a pair of indices
// occuring in candidate_indices. candidate_indices specifies the
// indices of objects that we should use to define 1D embeddings.
vint8_matrix class_BoostMap::RandomPivots(vint8_matrix candidate_indices,
	vint8 number)
{
	vint8_matrix result_matrix(number, 2);
	vArray2(vint8) result = result_matrix.Matrix2();
	vint8 number_of_indices = candidate_indices.Size();
	if (number_of_indices <= 1)
	{
		exit_error("Error, fewer than two candidates available\n");
	}
	vArray(vint8) indices = candidate_indices.Matrix();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		// pick two numbers randomly, make sure they are not equal.
		vint8 pivot1_index = function_random_vint8(0, number_of_indices-1);
		vint8 pivot2_index = function_random_vint8(0, number_of_indices-1);
		if (pivot1_index == pivot2_index)
		{
			i--;
			continue;
		}
		vint8 pivot1 = indices[pivot1_index];
		vint8 pivot2 = indices[pivot2_index];
		result[i][0] = pivot1;
		result[i][1] = pivot2;
	}

	return result_matrix;
}



// result(i) = i. A trivial way to generate a candidates_matrix
// that includes all candidates.
vint8_matrix class_BoostMap::AllCandidates(vint8 in_number)
{
	if (in_number <= 0)
	{
		exit_error("AllCandidates called with bad training number:\n",
			in_number);
	}

	vint8_matrix result = vint8_matrix::Range(0, in_number-1);
	return result;
}


// find the best z and alpha for the specified reference-object-based
// classifier.
float class_BoostMap::ClassifierZ(vint8 index, float * alpha)
{
	//  vPrint("ClassifierZ, index = %li\n", index);
	// results[i] is what is called u_i in Schapire and Singer.
	vector<float> results((vector_size) training_triple_number);
	LipschitzResults(index, &results,
		training_triple_matrix, ground_truth,
		candtrain_distances_matrix);  

	float a = 0, z = 0;
	iterations = iterations + vas_int16((long) MinimizeZ(&results, &z, &a));
	*alpha = a;
	return z;
}


// find the best alpha and the corresponding minimum z for
// the weak classifier that gives these results.
vint8 class_BoostMap::MinimizeZ(vector<float> * results, float * z, float * a)
{
	vint8 number = results->size();
	if (number == 0)
	{
		exit_error("Error: MinimizeZ called without objects\n");
	}

	vMatrix<float> resultm = matrix_from_vector(results);

	float alpha_min = 0, alpha_max = 0;
	ComputeAlphaLimits(resultm, &alpha_min, &alpha_max);
	if (allow_negative == 0)
	{
		alpha_min = Max(alpha_min, (float) 0);
	}

	// First check if we have a positive z' at alpha_min
	float dz_min = (float) Z_Prime(alpha_min, results);
	if (dz_min >= 0)
	{
		//    vPrint("dz_min >= 0\n");
		*z = (float) Z(alpha_min, results);
		*a = alpha_min;
		return 0;
	}

	// Now check if we have a negative z' at alpha_max
	float dz_max = (float) Z_Prime(alpha_max, results);
	if (dz_max <= 0)
	{
		vPrint("dz_max <= 0\n");
		*z = (float) Z(alpha_max, results);
		*a = alpha_max;
		return 0;
	}

	// Check for the pathological case where all result entries
	// have the same sign.
	vint8 found_positive = 0, found_negative = 0;
	vint8 i;
	for (i = 0; i < number; i++)
	{
		if ((*results)[(vector_size) i] > 0)
		{
			found_positive = 1;
		}
		else if ((*results)[(vector_size) i] < 0)
		{
			found_negative = 1;
		}
	}

	if ((found_positive == 1) && (found_negative == 0))
	{
		*a = 1;
		*z = (float) Z(*a, results);
		return 0;
	}

	else if ((found_positive == 0) && (found_negative == 1))
	{
		if (allow_negative == 0)
		{
			*a = 0;
		}
		else
		{
			*a = -1;
		}
		*z = (float) Z(*a, results);
		return 0;
	}

	else if ((found_positive == 0) && (found_negative == 0))
	{
		*a = 0;
		*z = (float) Z(*a, results);
		return 0;
	}

	float alpha_hi = 0;
	float alpha_low = 0;
	float step = Min((float) 1.0, vAbs(alpha_max) / (float) 100.0);
	float dz_low = 0, dz_high = 0;

	vint8 iterations;
	vint8 max_iterations = 20;

	// find a value of alpha (alpha_hi) for which Z'(alpha) >= 0.
	iterations = 0;
	while((dz_high = (float) Z_Prime(alpha_hi, results)) < 0)
	{
		alpha_hi = alpha_hi + step;
		step = step * (float) 2.0;
		iterations++;
		if (iterations > max_iterations)
		{
			//      vPrint("iterations exceeded\n");
			break;
		}
	}
	//  vPrint("alpha_hi = %f\n", alpha_hi);

	step = Min(((float) 1.0), vAbs(alpha_min) / (float) 100.0);

	// find a value of alpha (alpha_low) for which Z'(alpha) <= 0.
	iterations = 0;
	while((dz_low = (float) Z_Prime(alpha_low, results)) > 0)
	{
		alpha_low = alpha_low - step;
		step = step * (float) 2.0;
		iterations++;
		if (iterations > max_iterations)
		{
			//      vPrint("iterations exceeded\n");
			break;
		}
	}
	//  vPrint("alpha_low = %f\n", alpha_low);

	alpha_low = alpha_min;
	alpha_hi = alpha_max;

	float alpha;
	vint8 counter = 0;
	const float adjustment = (float) 0.01;
	iterations = 0;
	// Now use binary search to find a root of Z'.
	while(1)
	{
		// The next few lines do search using linear interpolation
		// to improve the guess and minimize the number of iterations
		float range = (dz_high - dz_low) * (float) 1.01;
		if (range < 0.000001) break;

		float high_weight = -dz_low / range;
		float low_weight = dz_high / range;
		if (high_weight < low_weight) 
		{
			high_weight = high_weight + adjustment;
		}
		else
		{
			low_weight = low_weight + adjustment;
		}

		alpha = low_weight * alpha_low + high_weight * alpha_hi;

		// The next line is good for simple binary search
		//    alpha = (alpha_hi + alpha_low) / 2.0;

		if ((alpha_hi - alpha < 0.000001) || 
			(alpha - alpha_low < 0.000001))
		{
			break;
		}


		//    vPrint("alpha - alpha_low = %f, alpha_hi - alpha = %f\n",
		//            alpha - alpha_low, alpha_hi - alpha);
		//    vPrint("(%li, %li, %li)\n", 
		//            training_triples[53][0], training_triples[53][1], training_triples[53][2]);
		//    vint8 check1 = (alpha == alpha_low);
		//    vint8 check2 = (alpha == alpha_hi);
		//  vPrint("check1 = %li, check2 = %li\n", check1, check2);
		float z_prime = (float) Z_Prime(alpha, results);
		if (z_prime == 0) 
		{
			break;
		}
		else if (z_prime < 0)
		{
			alpha_low = alpha;
			dz_low = z_prime;
		}
		else
		{
			alpha_hi = alpha;
			dz_high = z_prime;
		}
		//    vPrint("%li, %.15f - %.15f - %.15f, %f\n", counter, 
		//            alpha_low, alpha, alpha_hi, z_prime);
		//    vPrint("alpha - alpha_low = %f, alpha_hi - alpha = %f\n\n",
		//            alpha - alpha_low, alpha_hi - alpha);
		counter++;
		iterations++;
		if (iterations > max_iterations)
		{
			//      vPrint("iterations exceeded\n");
			break;
		}
	}

	*z = (float) Z(alpha, results);
	*a = alpha;

	// this is a hack, that eventually should be useless by
	// finding the allowed ranges for alpha for each classifier.
	if (*z < 0)
	{
		exit_error("error: we shouldn't get here\n");
		*z = 1;
		*a = 0;
	}

	return counter;
}


// This function is not used right now, but it should probably be 
// used when we evaluate changing weights of classifiers.
// Same as MinimizeZ, but with the additional constraint that
// *a should be in the range [alpha_low, alpha_hi].
vint8 class_BoostMap::MinimizeZ5(vector<float> * results, float * z, float * a,
	float alpha_low, float alpha_hi)
{
	vint8 number = results->size();
	if (number == 0)
	{
		exit_error("Error: MinimizeZ called without objects\n");
	}

	float dz_low = (float) Z_Prime(alpha_low, results);
	if (dz_low >= 0)
	{
		*a = alpha_low;
		*z = (float) Z(alpha_low, results);
		return 0;
	}

	float dz_high = (float) Z_Prime(alpha_hi, results);
	if (dz_high <= 0)
	{
		*a = dz_high;
		*z = (float) Z(alpha_hi, results);
		return 0;
	}

	float alpha;
	vint8 counter = 0;
	const float adjustment = (float) 0.01;
	// Now use binary search to find a root of Z'.
	while(1)
	{
		// The next few lines do search using linear interpolation
		// to improve the guess and minimize the number of iterations
		float range = (dz_high - dz_low) * (float) 1.01;
		if (range < 0.000001) break;

		float high_weight = -dz_low / range;
		float low_weight = dz_high / range;
		if (high_weight < low_weight) 
		{
			high_weight = high_weight + adjustment;
		}
		else
		{
			low_weight = low_weight + adjustment;
		}

		alpha = low_weight * alpha_low + high_weight * alpha_hi;

		// The next line is good for simple binary search
		//    alpha = (alpha_hi + alpha_low) / 2.0;

		if ((alpha_hi - alpha < 0.000001) || 
			(alpha - alpha_low < 0.000001))
		{
			break;
		}

		float z_prime = (float) Z_Prime(alpha, results);
		if (z_prime == 0) 
		{
			break;
		}
		else if (z_prime < 0)
		{
			alpha_low = alpha;
			dz_low = z_prime;
		}
		else
		{
			alpha_hi = alpha;
			dz_high = z_prime;
		}

		counter++;
	}

	*z = (float) Z(alpha, results);
	*a = alpha;

	// this is a hack, that eventually should be useless by
	// finding the allowed ranges for alpha for each classifier.
	if (*z < 0)
	{
		*z = 1;
		*a = 0;
	}
	return counter;
}


// compute z for a classifier with the given weight (a) and
// that gives the given results 
double class_BoostMap::Z(double a, vector<float> * results)
{
	vint8 i;
	double sum = 0;
	for (i = 0; i < training_triple_number; i++)
	{
		double d_i = training_factors[i];
		double u_i = (*results)[(vector_size) i];
		double exp_i = vPrecomputedExp::Exp(-a * u_i);
		sum = sum + d_i * exp_i;
	}

	return sum;
}


// compute the derivative of z with respect to a
// for a classifier with the given weight (a) and
// that gives the given results 
double class_BoostMap::Z_Prime(double a, vector<float> * results)
{
	vint8 i;
	double sum = 0;
	for (i = 0; i < training_triple_number; i++)
	{
		double d_i = training_factors[i];
		double u_i = (*results)[(vector_size) i];
		double exponent = -a * u_i;
		if (exponent < min_exp) 
		{
			min_exp = (float) exponent;
		}
		if (exponent > max_exp) 
		{
			max_exp = (float) exponent;
		}
		double exp_i = vPrecomputedExp::Exp(exponent);
		//    float exp_i = exponent;
		sum = sum - d_i * u_i * exp_i;
	}

	return sum;
}

// assuming that we just chose the weak classifier that gives these
// results, and that achieves the given z with the given alpha,
// update the training weights as specified by AdaBoost.
float class_BoostMap::UpdateWeights(vector<float> * results, float z, float alpha)
{
	if (z <= 0)
	{
		exit_error("Error: non-positive z in UpdateWeights: %f\n", z);
	}

	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		double d_i = training_factors[i];
		double u_i = (*results)[(vector_size) i];
		double exp_i = vPrecomputedExp::Exp(-alpha * u_i);
		training_factors[i] = d_i * exp_i / z;
	}

	return z;
}


// Computes errors and margins for the last chosen weak classifier,
// which was assigned the given alpha (weight) and achieves
// the given results (multiplied by class label, as usual). One
// version for feature-space distances, one for parameter-space distances.
float class_BoostMap::ComputeLastError(float alpha, vector<float> * results)
{
	if (is_valid <= 0) return 0;
	double sum = 0;
	vint8 i;
	double accuracy = 0;
	for (i = 0; i < training_triple_number; i++)
	{
		double d_i = training_factors[i];
		double u_i = (*results)[(vector_size) i];
		sum = sum + d_i * u_i;
		if (u_i > 0) accuracy = accuracy + d_i;
		else if (u_i ==0) accuracy = accuracy + d_i / 2;
	}

	last_correlation = (float) sum;
	last_et = ((float) 1.0 - last_correlation) / (float) 2.0;
	last_error = (float) (1.0 - accuracy);
	last_scaled_correlation = (float) (alpha * sum);
	last_scaled_et = ((float) 1.0 - last_scaled_correlation) / (float) 2.0;
	return last_et;
}


// The next functions compute errors and margins 
// for the strong classifier assembled
// so far, possibly including the sensitive classifiers.
float class_BoostMap::ComputeTrainingError(vector<float> * results,
	vint8 use_sensitive)
{
	UpdateMargins(training_triple_matrix,
		results, training_margins_matrix,
		&training_error, &training_margin,
		use_sensitive);

	return training_margin;
}


float class_BoostMap::ComputeValidationError(vector<float> * results,
	vint8 use_sensitive)
{
	float validation_margin = 0;

	UpdateMargins(validation_triple_matrix,
		results, validation_margins_matrix,
		&validation_error, &validation_margin,
		use_sensitive);

	return validation_error;
}


// Like ComputeLastError, but for parameter-space distances.
float class_BoostMap::ComputeLastPerror(float alpha, vector<float> * results)
{
	if (is_valid <= 0) return 0;
	double sum = 0;
	vint8 i;
	double accuracy = 0;
	for (i = 0; i < training_triple_number; i++)
	{
		double d_i = training_factors[i];
		double u_i = (*results)[(vector_size) i];
		sum = sum + d_i * u_i;
		if (u_i > 0) accuracy = accuracy + d_i;
		else if (u_i ==0) accuracy = accuracy + d_i / 2;
	}

	last_pcorrelation = (float) sum;
	last_pet = ((float) 1.0 - last_correlation) / (float) 2.0;
	last_perror = (float) 1.0 - (float) accuracy;
	last_scaled_pcorrelation = (float) (alpha * sum);
	last_scaled_pet = ((float) 1.0 - last_scaled_pcorrelation) / (float) 2.0;
	return last_pet;
}


// Like ComputeTrainingError, for parameter-space distances.
float class_BoostMap::ComputeTrainingPerror(vector<float> * results,
	vint8 use_sensitive)
{
	float junk = 0;
	UpdateMargins(training_triple_matrix,
		results, training_pmargins_matrix,
		&training_perror, &junk,
		use_sensitive);

	return training_perror;
}


// Like ComputeValidationError, for parameter-space distances.
float class_BoostMap::ComputeValidationPerror(vector<float> * results,
	vint8 use_sensitive)
{
	float validation_margin = 0;

	UpdateMargins(validation_triple_matrix,
		results, validation_pmargins_matrix,
		&validation_perror, &validation_margin,
		use_sensitive);

	return validation_perror;
}


// UpdateMargins does all the work for 
// computing training or validation errors or perrors for the
// strong classifier, so it does the work for four functions.
vint8 class_BoostMap::UpdateMargins(vint8_matrix triple_matrix,
	vector<float> * results,
	vMatrix<float> margins_matrix,
	float * errorp, float * marginp,
	vint8 use_sensitive)
{
	vArray2(vint8) triples = triple_matrix.Matrix2();
	vArray(float) margins = margins_matrix.Matrix();
	vint8 number = triple_matrix.Rows();
	float weight = (float) 1.0 / (float) number;
	vint8 rounds = classifiers.size();
	class_triple_classifier last_classifier  ;

	if (use_sensitive == 0)
	{
		vint8 last_index = rounds - 1;
		if (last_index < 0)
		{
			exit_error("error: updatemargins, bad last index\n");
		}
		last_classifier = classifiers[(vector_size) last_index];
	}
	else
	{
		vint8 last_index = sensitive_classifiers.size() - 1;
		if (last_index < 0)
		{
			exit_error("error: updatemargins, bad last index\n");
		}
		last_classifier = sensitive_classifiers[(vector_size) last_index].classifier;
	}

	float reference_weight = last_classifier.weight;
	float error = 0, margin = 0;

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 q_index = triples[i][0];
		vint8 a_index = triples[i][1];
		vint8 b_index = triples[i][2];

		float current_estimate = reference_weight * (*results)[(vector_size) i];

		// Add the contribution of the last chosen reference images to 
		// the contributions of all previous reference images.
		float total_estimate = margins[i] + current_estimate;
		margins[i] = total_estimate;

		if (total_estimate == 0) 
		{
			error = error + weight / (float) 2.0;
		}
		else if (total_estimate < 0)
		{
			error = error + weight;
		}
		margin = margin + weight * total_estimate;
	}

	*errorp = error;
	*marginp = margin;
	return 1;
}


/*! 
	[static] This should be called to load from a file: translates a
	simple filename into a complete pathname, and adds the 
	extension ".txt".
*/
char * class_BoostMap::Pathname(const char * root_dir, const char * filename)
{
	char * directory = Directory(g_data_directory);
	char * temp_path_name = vJoinPaths(directory, filename);
	char * path_name = vMergeStrings2(temp_path_name, ".txt");
	vdelete2(directory);
	vdelete2(temp_path_name);
	return path_name;
}

/*! 
	This should be called to save unique classifiers to a file.
	Translates simple filename into complete pathname, and
	adds an extension that depends on the number of training
	rounds (including query-sensitive training).
*/
char * class_BoostMap::Pathname2(const char * filename) const
{
	return Pathname(data->RootDataDirectory(), filename);
}


// This should be called to save detailed classifiers to a file.
// Translates simple filename into complete pathname, and
// adds an extension that depends on the number of training
// rounds (including query-sensitive training).
char * class_BoostMap::Pathname3(const char * filename) const
{
	//vint8 number = (classifiers.size() + sensitive_classifiers.size()) % 10;
	//char * number_string = string_from_number(number);
	//char * directory = Directory();
	//char * temp_path_name = vJoinPaths(directory, filename);
	//char * path_name = vMergeStrings4(temp_path_name, "_", 
	//                                    number_string, "d.txt");
	//vdelete2(number_string);
	//vdelete2(directory);
	//vdelete2(temp_path_name);
	//return path_name;

	char * directory = Directory();
	char * temp_path_name = vJoinPaths(directory, filename);
	char * path_name = vMergeStrings2(temp_path_name, "d.txt");
	vdelete2(directory);
	vdelete2(temp_path_name);
	return path_name;
}


float class_BoostMap::TrainingError()
{
	return training_error;
}


float class_BoostMap::TrainingMargin()
{
	return training_margin;
}


float class_BoostMap::ValidationError()
{
	return validation_error;
}


float class_BoostMap::LastError()
{
	return last_error;
}


float class_BoostMap::LastEt()
{
	return last_et;
}


float class_BoostMap::LastCorrelation()
{
	return last_correlation;
}


float class_BoostMap::StepZ(vint8 step)
{
	if ((step < 0) || (step >= (vint8) classifiers.size()))
	{
		return -1;
	}
	return classifiers[(vector_size) step].z;
}


float class_BoostMap::StepTrainingError(vint8 step)
{
	if ((step < 0) || (step >= (vint8) training_errors.size()))
	{
		return -1;
	}
	return training_errors[(vector_size) step];
}


float class_BoostMap::StepValidationError(vint8 step)
{
	if ((step < 0) || (step >= (vint8) validation_errors.size()))
	{
		return -1;
	}
	return validation_errors[(vector_size) step];
}


// given distances from an object to all training objects,
// and given the ids of the reference objects, compute
// the distances to all reference objects.
vMatrix<float> class_BoostMap::ReferenceDistances(vMatrix<float> all_distancesm,
	vint8_matrix reference_idsm)
{
	if ((all_distancesm.valid() <= 0) || (reference_idsm.valid() <= 0))
	{
		vPrint("invalid arguments to ReferenceDistances\n");
		return vMatrix<float>();
	}

	vint8 training_number = all_distancesm.Size();
	vint8 number = reference_idsm.Size();
	vMatrix<float> resultm(1, number);

	vArray(float) result = resultm.Matrix();
	vArray(float) all_distances = all_distancesm.Matrix();
	vArray(vint8) reference_ids = reference_idsm.Matrix();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 id = reference_ids[i];
		// some ids will be negative, because we have two reference
		// ids per 1D embedding, but lipschitz-type embeddings only
		// have a meaningful value for the first of those ids.
		if (id < 0)
		{
			result[i] = -1;
		}
		else if (id >= training_number)
		{
			exit_error("error: in ReferenceDistances, id = %li, training_number = %li\n",
				(long) id, (long) training_number);
		}
		else
		{
			result[i] = all_distances[id];
		}
	}

	return resultm;
}


// this function only handles reference-object-type embeddings. 
// "references" includes all the reference objects. "index" is
// the index of the object (with respect to the training or
// validation set). candset_distances_matrix should be either 
// candtrain_distances_matrix or candval_distances_matrix,
// depending on whether index refers to a training or validation 
// object.
vMatrix<float> class_BoostMap::Embedding3a(vint8 index, vector<vint8> * references,
	vMatrix<float> candset_distances_matrix)
{
	vint8 steps = references->size();
	if (steps <= 0)
	{
		exit_error("Error: Embedding2 called after zero steps\n");
	}

	vArray2(float) distances = candset_distances_matrix.Matrix2();
	vMatrix<float> result_matrix(1, steps);
	vArray(float) result = result_matrix.Matrix();

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		vint8 index2 = (*references)[(vector_size) i];
		result[i] = distances[index2][index];
	}

	return result_matrix;

}


// "index" and "candset_distances_matrix" are as in Embedding3a. However,
// this function (contrary to Embedding3a) handles also line-projection 
// embeddings. It produces the embedding of object specified by index
// based on the classifiers stored in unique_classifiers.
vMatrix<float> class_BoostMap::Embedding2(vint8 index, 
	vMatrix<float> candset_distances_matrix)
{
	vint8 dimensions = unique_classifiers.size();
	vMatrix<float> query_distances_matrix(1, 2*dimensions);
	vMatrix<float> pivot_distances_matrix(1, dimensions);
	vint8_matrix types_matrix(1, dimensions);
	vArray2(float) candset_distances = candset_distances_matrix.Matrix2();

	function_enter_value(&query_distances_matrix, (float) -1);
	function_enter_value(&pivot_distances_matrix, (float) -1);

	vArray(float) query_distances = query_distances_matrix.Matrix();
	vArray(float) pivot_distances = pivot_distances_matrix.Matrix();
	vArray(vint8) types = types_matrix.Matrix();

	vint8 i;
	for (i = 0; i < dimensions; i++)
	{
		class_triple_classifier classifier = unique_classifiers[(vector_size) i];
		vint8 object1 = classifier.object1;
		vint8 object2 = classifier.object2;
		vint8 type = classifier.type;

		query_distances[2*i] = candset_distances[object1][index];
		types[i] = type;
		if (type == 1)
		{
			query_distances[2*i+1] = candset_distances[object2][index];
			pivot_distances[i] = candcand_distances[object1][object2];
		}
	}

	return Embedding3(query_distances_matrix, pivot_distances_matrix,
		types_matrix);
}

/*!
	Query_distances is a matrix with dimensions x 2 
	entries, of distances
	from the query to each of the two objects corresponding
	to each dimension. If a dimension corresponds to a lipschitz
	embedding, the second col is ignored for that dimension.
	pivot_distances has, at position i, the distance between the i-th
	pivot points (if the dimension corresponds to a projection).
	If the number of rows of query_distances is less than the
	number of unique classifiers, we use only the first classifiers.
	types_matrix(i) is 0 if the i-th dimension is a lipschitz embedding,
	and 1 if the i-th dimension is a pseudo-projection.
*/
vMatrix<float> class_BoostMap::Embedding3(vMatrix<float> query_distances_matrix,
	vMatrix<float> pivot_distances_matrix,
	vint8_matrix types_matrix)
{
	vint8 dimensions = query_distances_matrix.Size() / 2;
	if ((dimensions != pivot_distances_matrix.Size()) || 
		(dimensions != types_matrix.Size()) || 
		(2 * dimensions != query_distances_matrix.Size()))
	{
		exit_error("Error in Embedding3: dimensions don't match\n");
	}

	vArray(float) query_distances = query_distances_matrix.Matrix();
	vArray(float) pivot_distances = pivot_distances_matrix.Matrix();
	vArray(vint8) types = types_matrix.Matrix();

	vMatrix<float> result_matrix(1, dimensions);
	vArray(float) result = result_matrix.Matrix();

	vint8 i;
	for (i = 0; i < dimensions; i++)
	{
		if (types[i] == 0)
		{
			result[i] = query_distances[2*i];
		}
		else if (types[i] == 1)
		{
			float q_pivot1 = query_distances[2*i];
			float q_pivot2 = query_distances[2*i+1];
			float pivot_distance = pivot_distances[i];
			result[i] = V_FastMap::LineProjection3(q_pivot1, q_pivot2, pivot_distance);
		}
		else
		{
			exit_error("Error in Embedding3: we shouldn't get here\n");
		}
	}

	return result_matrix;
}


// it returns a row matrix, which at positions 2*i and 2*i+1 stores
// the indices of the objects that define the i-th classifier 
// stored in unique_classifiers (i-th
// coordinate of the embedding). Of course, if the i-th classifier
// is a reference-object type of classifier, then position 2*i+1
// does not matter and it can hold anything (most likely -1, to
// indicate that no object is needed there).
vint8_matrix class_BoostMap::ExtractReferenceObjects(vMatrix<float> classifiers)
{
	vint8 number = classifiers.Rows();
	vint8_matrix result_matrix(1, 2*number);
	if (classifiers.Cols() <= 6)
	{
		function_print("\ncannot extract reference objects, crossfire matrix has %li columns\n", 
			(long) classifiers.horizontal());
		//    return extract_references_from_unique_classifiers(classifiers);
		return vint8_matrix();
	}

	vint8 i;
	for (i = 0; i < number; i++)
	{
		result_matrix(2*i) = round_number(classifiers(i, 5));
		result_matrix(2*i+1) = round_number(classifiers(i, 6));
	}

	return result_matrix;
}


vint8_matrix class_BoostMap::extract_references_from_unique_classifiers(vMatrix<float> classifiers)
{
	vint8 number = classifiers.Rows();
	vint8_matrix result_matrix(1, 2*number);
	if (classifiers.Cols() <= 2)
	{
		exit_error("error in extract references\n");
	}

	vint8 i;
	for (i = 0; i < number; i++)
	{
		result_matrix(2*i) = round_number(classifiers(i, 1));
		result_matrix(2*i+1) = round_number(classifiers(i, 2));
	}

	return result_matrix;
}


// It returns a row matrix, whose i-th entry is the weight of the
// i-th unique classifier.
vMatrix<float> class_BoostMap::ExtractWeights(vMatrix<float> classifiers)
{
	vint8 number = classifiers.Rows();
	vMatrix<float> result_matrix(1, number);

	vint8 i;
	for (i = 0; i < number; i++)
	{
		result_matrix(i) = classifiers(i, 3);
	}

	return result_matrix;
}


// it returns a row matrix whose i-th entry is the type of
// the i-th unique classifier.
vint8_matrix class_BoostMap::ExtractTypes(vMatrix<float> classifiers)
{
	vint8 number = classifiers.Rows();
	vint8_matrix result_matrix(1, number);

	vint8 i;
	for (i = 0; i < number; i++)
	{
		result_matrix(i) = round_number(classifiers(i, 0));
	}

	return result_matrix;
}


// assuming that v1 and v2 are embeddings of two objects, as 
// specified by the embedding defined in unique_classifiers,
// here we compute the L1 distance between those vectors.
// This function cannot handle query-sensitive embeddings.
float class_BoostMap::L1_Distance(vMatrix<float> v1, vMatrix<float> v2)
{
	return L1_Distance3(v1, v2, &classifiers);
}


// it seems to me right now that this function should be static.
// v1 and v2 are embeddings of two objects, as specified by
// the embedding defined by the classifiers in "weights". 
// Since the objects have already been embedded, the only useful
// information in "weights" is the weights of those classifiers.
float class_BoostMap::L1_Distance3(vMatrix<float> v1, vMatrix<float> v2,
	vector<class_triple_classifier> * classifiers)
{
	float sum = 0;
	vint8 length = v1.Cols();
	vArray(float) v3 = v1.Matrix();
	vArray(float) v4 = v2.Matrix();

	vint8 i;
	for (i = 0; i < length; i++)
	{
		float distance = vAbs(v3[i] - v4[i]);
		sum = sum + (*classifiers)[(vector_size) i].weight * distance;
	}
	return sum;
}


// this function should also probably be static. 
// Similar to L1_Distance3, but here instead of passing
// a vector of classifiers, we pass in a vector of weights,
// since we don't need any other information from the classifiers
// anyway.
float class_BoostMap::L1_Distance3a(vMatrix<float> v1, vMatrix<float> v2,
	vector<float> * weights)
{
	float sum = 0;
	vint8 length = v1.Cols();
	vArray(float) v3 = v1.Matrix();
	vArray(float) v4 = v2.Matrix();

	vint8 i;
	for (i = 0; i < length; i++)
	{
		float distance = vAbs(v3[i] - v4[i]);
		sum = sum + (*weights)[(vector_size) i] * distance;
	}
	return sum;
}


// assuming that v1 and v2 are embeddings of two objects, as 
// specified by the embedding defined in unique_classifiers,
// here we compute the L1 distance between those vectors.
// This function cannot handle query-sensitive embeddings.
float class_BoostMap::Lp_Distance(vMatrix<float> v1, vMatrix<float> v2)
{
	return Lp_Distance3(v1, v2, &classifiers);
}


// it seems to me right now that this function should be static.
// v1 and v2 are embeddings of two objects, as specified by
// the embedding defined by the classifiers in "weights". 
// Since the objects have already been embedded, the only useful
// information in "weights" is the weights of those classifiers.
float class_BoostMap::Lp_Distance3(vMatrix<float> v1, vMatrix<float> v2,
	vector<class_triple_classifier> * classifiers)
{
	float sum = 0;
	vint8 length = v1.Cols();
	vArray(float) v3 = v1.Matrix();
	vArray(float) v4 = v2.Matrix();

	vint8 i;
	for (i = 0; i < length; i++)
	{
		float distance = Power(vAbs(v3[i] - v4[i]));
		sum = sum + (*classifiers)[(vector_size) i].weight * distance;
	}
	return sum;
}


// this function should also probably be static. 
// Similar to L1_Distance3, but here instead of passing
// a vector of classifiers, we pass in a vector of weights,
// since we don't need any other information from the classifiers
// anyway.
float class_BoostMap::Lp_Distance3a(vMatrix<float> v1, vMatrix<float> v2,
	vector<float> * weights)
{
	float sum = 0;
	vint8 length = v1.Cols();
	vArray(float) v3 = v1.Matrix();
	vArray(float) v4 = v2.Matrix();

	vint8 i;
	for (i = 0; i < length; i++)
	{
		float distance = Power(vAbs(v3[i] - v4[i]));
		sum = sum + (*weights)[(vector_size) i] * distance;
	}
	return sum;
}



// For debugging: weights should sum up to 1.
float class_BoostMap::SumWeights()
{
	double sum = 0;
	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		sum = sum + training_factors[i];
	}

	return (float) sum;
}


float class_BoostMap::SumClassifierWeights()
{
	float sum = 0;
	vint8 i;
	for (i = 0; i < (vint8) classifiers.size(); i++)
	{
		sum = sum + classifiers[(vector_size) i].weight;
	}

	return sum;
}


vint8 class_BoostMap::PrintSummary()
{
	if (is_valid == 0) return 0;
	vPrint("\n");
	vPrint("BoostMap (p = %li): %li candidates, %li training objects, %li validation objects\n",
		(long) distance_p, (long) candidate_number, (long) training_number, (long) validation_number);
	vPrint("%li (%li small, %li large) training triples, %li validation triples\n",
		(long) training_triple_number, (long) small_triple_number,
		(long) large_triple_number, (long) validation_triple_number);
	vPrint("%li picked candidates, %li projection candidates, ", 
		(long) number_of_picked_candidates, (long) projection_candidate_number);
	vPrint("iterations = ");
	iterations.Print();
	vPrint("\n");
	vPrint("use_pdistances = %li, allow_negative = %li, allow_removals = %li\n",
		(long) use_pdistances, (long) allow_negative, (long) allow_removals);
	vPrint("allow_lipschitz = %li, allow_projections = %li\n",
		(long) allow_lipschitz, (long) allow_projections);
	vPrint("allow_sensitive = %li, new_sensitive = %li, use_best_k = %li\n",
		(long) allow_sensitive, (long) new_sensitive, (long) use_best_k);
	vPrint("--------------------------\n");
	vPrint("min_exp = %f, max_exp = %f\n", min_exp, max_exp);
	vPrint("last error = %f, et = %f, correlation = %f\n",
		last_error, last_et, last_correlation);
	vPrint("last_scaled_et = %f, last_scaled_correlation = %f\n",
		last_scaled_et, last_scaled_correlation);
	if (classifiers.size() > 0)
	{
		vint8 number = classifiers.size() - 1;
		vPrint("global: last weight = %f, last z = %f, last_new_z = %f\n",
			classifiers[(vector_size) number].weight, classifiers[(vector_size) number].z, last_new_z);
	}
	if (sensitive_classifiers.size() > 0)
	{
		vint8 number = sensitive_classifiers.size() - 1;
		vPrint("special: last weight = %f, last z = %f\n",
			sensitive_classifiers[(vector_size) number].classifier.weight, 
			sensitive_classifiers[(vector_size) number].classifier.z);
	}
	vPrint("last perror = %f, pet = %f, pcorrelation = %f\n",
		last_perror, last_pet, last_pcorrelation);
	vPrint("last_scaled_pet = %f, last_pscaled_correlation = %f\n",
		last_scaled_pet, last_scaled_pcorrelation);
	vPrint("weight_sum = %f, training_margin = %f\n", 
		SumWeights(), training_margin);
	vPrint("--------------------------\n");
	vPrint("TRAINING_ERROR  = %f, VALIDATION_ERROR  = %f\n",
		training_error, validation_error);
	vPrint("TRAINING_PERROR = %f, VALIDATION PERROR = %f\n",
		training_perror, validation_perror);

	vint8 steps = classifiers.size();
	vint8 unique_steps = unique_classifiers.size();
	vint8 lipschitz_steps = LipschitzSteps();
	vint8 projection_steps = ProjectionSteps();
	vPrint("%li steps, %li unique_steps, %li lipschitz, %li projections\n",
		(long) steps, (long) unique_steps, (long) lipschitz_steps, (long) projection_steps);

	vint8 sensitive_steps = sensitive_classifiers.size();
	if (sensitive_steps > 0)
	{
		vPrint("%li sensitive classifiers\n", (long) sensitive_steps);
	}

	vPrint("\n");
	return 1;
}


vint8 class_BoostMap::PrintClassifier()
{
	vMatrix<float> classifier_matrix = ClassifierMatrix();
	classifier_matrix.Print("classifier_matrix");

	vint8 steps = classifiers.size();
	vint8 unique_steps = unique_classifiers.size();
	vint8 lipschitz_steps = LipschitzSteps();
	vint8 projection_steps = ProjectionSteps();
	vPrint("\n%li steps, %li unique steps, %li lipschitz, %li projections\n",
		(long) steps, (long) unique_steps, (long) lipschitz_steps, (long) projection_steps);
	return 1;
}


vint8 class_BoostMap::PrintAll()
{
	if (is_valid <= 0) return 0;
	candcand_distances_matrix.Print("cand-cand distances");
	candtrain_distances_matrix.Print("cand-train distances");
	candval_distances_matrix.Print("cand-val distances");
	training_triple_matrix.Print("training indices");
	validation_triple_matrix.Print("validation indices");
	training_distances.Print("training distances");
	training_factors_matrix.Print("training weights");
	PrintClassifier();
	PrintSummary();
	vPrint("\n");

	return 1;
}

// Pick random reference points, and compute error and margin for them.
vint8 class_BoostMap::PickRandom(vint8 number)
{
	return PickRandom3(number, &training_error, &validation_error);
}


vint8 class_BoostMap::PickRandom3(vint8 number, float * training_errorp,
	float * validation_errorp)
{
	if (number <= 0) return 0;
	if (training_number == 0) return 0;
	if (is_valid <= 0) return 0;

	// Pick random points
	vector<vint8> random_references((vector_size) number);
	vector<float> uniform_weights((vector_size) number);

	vint8 i;
	for (i = 0; i < number; i++)
	{
		random_references[(vector_size) i] = function_random_vint8(0, candidate_number-1);
		uniform_weights[(vector_size) i] = 1.0;
	}

	// Compute errors
	float error_training = ComputeError(&random_references, &uniform_weights,
		training_triple_matrix, ground_truth,
		candtrain_distances_matrix);
	float error_validation = ComputeError(&random_references, &uniform_weights,
		validation_triple_matrix, validation_distances,
		candval_distances_matrix);

	*training_errorp = error_training;
	*validation_errorp = error_validation;
	return 1;
}


// ComputeError is used in conjunction with PickRandom, to 
// compute the statistics of the embedding (errors, margins).
// It does not handle parameter space errors, at least not right
// now.
float class_BoostMap::ComputeError(vector<vint8> * references, 
	vector<float> * weights,
	vint8_matrix triples_matrix,
	vMatrix<float> ground_truth,
	vMatrix<float> candset_distances_matrix)
{
	vint8 number = triples_matrix.Rows();
	vint8 dimensions = references->size();
	vArray2(vint8) triples = triples_matrix.Matrix2();
	vArray(float) distances = ground_truth.Matrix();
	float error = 0;
	float factor = 1 / (float) number;

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 q_index = triples[i][0];
		vint8 a_index = triples[i][1];
		vint8 b_index = triples[i][2];

		float label = Label(distances[i]);

		// embed q, a, b.
		vMatrix<float> e_q = Embedding3a(q_index, references,
			candset_distances_matrix);
		vMatrix<float> e_a = Embedding3a(a_index, references,
			candset_distances_matrix);
		vMatrix<float> e_b = Embedding3a(b_index, references,
			candset_distances_matrix);

		// compute embedding-based distances from q to a and b.
		float e_qa = Lp_Distance3a(e_q, e_a, weights);
		float e_qb = Lp_Distance3a(e_q, e_b, weights);

		// compute the classification margin.
		float margin = (e_qb - e_qa) * label;
		if (margin == 0) 
		{
			error = error + factor / (float) 2.0;
		}
		else if (margin < 0)
		{
			error = error + factor;
		}
		margin = margin + factor * margin;
	}

	return error;
}


// set whether we allow negative weights for a weak classifier in
// unique_classifiers. We always allow a negative weight for a 
// classifier in classifiers, as vint8 as the corresponding weight
// in unique_classifiers (after we sum up weights for all other
// occurrences of the same classifier in classifiers) is non-negative.
vint8 class_BoostMap::SetAllowNegative(vint8 in_value)
{
	allow_negative = in_value;
	return 1;
}


// set whether we allow, at each training step, removal of 
// previously chosen weak classifiers, if that removal is 
// deemed to improve classification accuracy.
vint8 class_BoostMap::SetAllowRemovals(vint8 in_value)
{
	allow_removals = in_value;
	return 1;
}


// add the specified weak classifier to unique_classifiers. If 
// the classifier already appears in unique_classifiers, we 
// just modify the weight there, otherwise we append this
// classifier to the end of unique_classifiers.
vint8 class_BoostMap::clean_up_classifier(class_triple_classifier classifier)
{
	vint8 result = clean_up_classifier(classifier, &unique_classifiers);
	vint8 steps = classifiers.size();
	vint8 unique_steps = unique_classifiers.size();
	vint8 lipschitz_number = LipschitzSteps();
	vint8 projection_number = ProjectionSteps();
	return result;
}


// static version of the previous function.
// adds classifier to unique_classifiers, in a way that avoids repetitions.
// if classifier does not occur in unique classifiers, it gets added
// to them. Otherwise, we find an occurrence of classifier in classifiers,
// and we adjust the weight of that occurrence by adding to it the
// weight of classifier.
vint8 class_BoostMap::clean_up_classifier(class_triple_classifier classifier,
	vector<class_triple_classifier> * unique_classifiers)
{
	vint8 number = unique_classifiers->size();

	vector<vint8> indices_found;
	vint8 type = classifier.type;
	vint8 object1 = classifier.object1;
	vint8 object2 = classifier.object2;
	float sum_of_weights = classifier.weight;

	vint8 i;
	// go through all classifiers, check if any of them is the
	// same as classifier.
	for (i = 0; i < number; i++)
	{
		class_triple_classifier & c = (*unique_classifiers)[(vector_size) i];
		if (type != c.type) continue;
		if (object1 != c.object1) continue;
		if ((type == 1) && (object2 != c.object2)) continue;

		// if we got here, we found a matching classifier.
		indices_found.push_back(i);
		sum_of_weights = sum_of_weights + c.weight;
	}

	vint8 found = indices_found.size();
	// if no matching classifier was found, then we just add "classifier".
	if (found == 0)
	{
		unique_classifiers->push_back(classifier);
	}

	// If we found one matching classifier, then we just add the
	// weight of "classifier" to the weight of the matching classifier.
	// If the resulted combined weight is zero, then we remove that
	// classifier.
	else if (found == 1)
	{
		vint8 index = indices_found[0];
		if (vAbs(sum_of_weights) > 0.000001)
		{
			(*unique_classifiers)[(vector_size) index].weight = sum_of_weights;
		}
		else  // In this case, remove the classifier.
		{
			for (i = index; i < number-1; i++)
			{
				(*unique_classifiers)[(vector_size) i] = (*unique_classifiers)[(vector_size) i+1];
			}
			unique_classifiers->pop_back();
		}
	}

	// if we found more than one matching classifier, it means
	// that the classifiers in "unique_classifiers" are not
	// really unique, and that is a problem.
	else
	{
		vPrint("found = %li\n", (long) found);
		vint8 i;
		for (i = 0; i < found; i++)
		{
			vPrint("indices_found[%li] = %li\n", (long) i, (long) indices_found[(vector_size) i]);
			(*unique_classifiers)[(vector_size) indices_found[(vector_size) i]].Print();
		}

		exit_error("Error: Duplicate classifiers in unique_classifiers\n");
	}

	if (found == 0)
	{
		return unique_classifiers->size() - 1;
	}
	else 
	{
		return indices_found[0];
	}
}

// returns the location of the directory where we save and load 
// embeddings.
char * class_BoostMap::Directory() const
{
	return Directory(data->RootDataDirectory());
} 

// [static] returns the location of the directory where we save and load 
// embeddings.
char * class_BoostMap::Directory(const char* root_dir)
{
	char * directory = vJoinPaths3(root_dir, "experiments", "boost_map");
	return directory;
} 


// save_itinerary does the actual work for the save function,
// and is called once for each of the two copies that we save.
vint8 class_BoostMap::save_auxiliary(const char * filename)
{
	// Pathname2 adds the appropriate directory and extension to
	// filename.
	char * path_name = Pathname2(filename);

	// convert query-insensitive part of strong
	// classifier to matrix form, and write it to file.
	vMatrix<float> classifier = ClassifierMatrix();
	vint8 success = classifier.WriteText(path_name);
	if (success <= 0)
	{
		vPrint("Failed to save classifier to %s\n", path_name);
	}
	else
	{
		vPrint("Saved classifier to %s\n", path_name);
	}

	// get pathname for the sensitive classifiers.
	char * path_name2 = Pathname3(filename);

	// convert detailed version of strong classifier
	// into matrix form, (which stores weak classifies in the
	// order in which they were picked, and includes repetitions)
	// and write it to file.
	classifier = DetailedClassifierMatrix();
	vint8 success2 = classifier.WriteText(path_name2);
	if (success2 <= 0)
	{
		vPrint("Failed to save classifier to %s\n", path_name2);
	}
	else
	{
		vPrint("Saved classifier to %s\n", path_name2);
	}

	vdelete2(path_name);
	vdelete2(path_name2);

	// save query-sensitive part of the strong classifier
	SaveSensitiveClassifier(filename);

	vint8 result = Min(success, success2);
	return result;
}

/*!
	Save the classifier to disk. We actually create three different 
	files: one for the unique classifiers, one for classifiers 
	(useful if we want to load them in the same order in which
	they were picked, for example to see the step-by-step errors,
	or to stop at a given number of dimensions), and one for
	sensitive_classifiers. Also, we add an extension that 
	depends on the total number of classifiers we have so far.
	Adding that extension is useful because, at training, 
	instead of writing over the same file over and over again,
	we actually have ten different files, so we can easily
	compare the last ten steps, and so that if writing the 
	latest file breaks down we still have the previous one intact.
*/
vint8 class_BoostMap::save_classifier(const char * filename)
{
	vint8 first_success = save_auxiliary(filename);
	char * round_robin_name = round_robin_filename(filename);
	vint8 second_success = save_auxiliary(round_robin_name);
	delete_pointer(round_robin_name);

	vint8 success = first_success * second_success;
	return success;
}

char * class_BoostMap::round_robin_filename(const char * filename)
{
	vint8 number = (classifiers.size() + sensitive_classifiers.size()) % 10;
	char * number_string = string_from_number(number);
	char * result = function_merge_strings_three(filename, "_robin_", number_string);
	delete_pointer(number_string);
	return result;
}



// This function is useful when we want to fully specify the classifier,
// for example if we want to load an existing classifier, so we want
// to use the weight that has already been computed.
vint8 class_BoostMap::AddClassifier(class_triple_classifier classifier)
{
	// Check if the classifier type is allowed.
	if ((classifier.type == 0) && (allow_lipschitz == 0)) 
	{
		return 0;
	}

	// Check if the classifier type is allowed.
	if ((classifier.type == 1) && (allow_projections == 0)) 
	{
		return 0;
	}

	// compute error and perror for classifier.
	vector<float> results((vector_size) training_triple_number);
	ClassifierResults(&classifier, &results, training_triple_matrix,
		training_distances, candtrain_distances_matrix);
	ComputeLastError(classifier.weight, &results);
	ClassifierResults(&classifier, &results, training_triple_matrix,
		training_pdistances, candtrain_distances_matrix);
	ComputeLastPerror(classifier.weight, &results);

	// compute classifier results
	ClassifierResults(&classifier, &results, training_triple_matrix,
		ground_truth, candtrain_distances_matrix);

	// add classifier to detailed classifiers.
	classifiers.push_back(classifier);

	// compute z for the classifier.
	float z = (float) Z(classifier.weight, &results);

	// update weights for training triples, based on z and the weight
	// of the classifier.
	UpdateWeights(&results, z, classifier.weight);

	// add classifier to unique_classifiers.
	clean_up_classifier(classifier);

	// Compute training and validation error and perror
	ClassifierResults(&classifier, &results, training_triple_matrix,
		training_distances, candtrain_distances_matrix);
	float margin = ComputeTrainingError(&results);

	ClassifierResults(&classifier, &results, training_triple_matrix,
		training_pdistances, candtrain_distances_matrix);
	ComputeTrainingPerror(&results);

	vector<float> validation_results((vector_size) validation_triple_number);
	ClassifierResults(&classifier, &validation_results, validation_triple_matrix,
		validation_distances, candval_distances_matrix);
	ComputeValidationError(&validation_results);

	ClassifierResults(&classifier, &validation_results, validation_triple_matrix,
		validation_pdistances, candval_distances_matrix);
	ComputeValidationPerror(&validation_results);

	// store training and validation error and perror of strong classifier,
	// for this step.
	training_errors.push_back(training_error);
	validation_errors.push_back(validation_error);
	training_perrors.push_back(training_perror);
	validation_perrors.push_back(validation_perror);

	return (vint8) margin;
}


float_matrix class_BoostMap::load_insensitive_classifier(const char* filename)
{
	char * path_name = Pathname(g_data_directory, filename);

	v3dMatrix<float> * temp_classifier_matrix = 
		v3dMatrix<float>::ReadText(path_name);
	if (temp_classifier_matrix == 0)
	{
		vPrint("Failed to load classifier from %s\n", path_name);
		return float_matrix ();
	}
	else
	{
		vPrint("Loaded classifier from %s\n", path_name);
	}

	vMatrix<float> classifier_matrix(temp_classifier_matrix);
	vdelete(temp_classifier_matrix);
	vdelete2(path_name);
	return classifier_matrix;
}


// We load a classifier from disk. The filename should be 
// a filename where we store classifiers (the ".txt" extension
// is not necessary, but the stuff that Pathname2 and Pathname3 
// adds to the name should be included here). Note that,
// if the file was generated with save_classifier, and 
// sensitive classifiers were also stored, then we can 
// use either the filename that was generated from Pathname2
// or the filename generated from Pathname3. sensitive_pathname
// is smart enough to figure out the corresponding name
// for the file where the sensitive classifiers get saved.
vint8 class_BoostMap::load_classifier(const char * filename)
{
	return load_classifier_b(filename, -1);
}


// Here we load the most classifiers we can load without
// getting over the specified number of dimensions.
// Note that the filename passed in here should not be the 
// same as the filename passed to save_classifier, because
// save_classifier adds a number to it. For example, we
// may call save_classifier("name"), and load_classifier("name_4").
vint8 class_BoostMap::load_classifier_b(const char * filename, vint8 dimensions)
{
	char * path_name = Pathname(g_data_directory, filename);

	v3dMatrix<float> * temp_classifier_matrix = 
		v3dMatrix<float>::ReadText(path_name);
	if (temp_classifier_matrix == 0)
	{
		vPrint("Failed to load classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded classifier from %s\n", path_name);
	}

	vMatrix<float> classifier_matrix(temp_classifier_matrix);
	vdelete(temp_classifier_matrix);
	AddClassifierMatrix2(classifier_matrix, dimensions);
	vdelete2(path_name);

	LoadSensitiveClassifier(filename);

	return 1;
}


// LoadQsClassifier is useful for loading classifiers/embeddings
// trained by using (from the beginning of the training)
// the bmallow_sensitive 1 1 command, i.e.
// with allow_sensitive = 1, and new_sensitive = 1. In those
// cases, we want to load the first dimensions of the detailed
// classifiers, and the first dimensions of the sensitive classifiers.
vint8 class_BoostMap::load_sensitive_classifier(const char * filename, vint8 dimensions)
{
	char * path_name = Pathname(g_data_directory, filename);

	v3dMatrix<float> * temp_classifier_matrix = 
		v3dMatrix<float>::ReadText(path_name);
	if (temp_classifier_matrix == 0)
	{
		vPrint("Failed to load classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded classifier from %s\n", path_name);
	}

	vint8 number = temp_classifier_matrix->Rows();
	if (dimensions == -1)
	{
		dimensions = number;
	}
	else
	{
		dimensions = Min(number, dimensions);
	}

	vMatrix<float> classifier_matrix(dimensions, temp_classifier_matrix->Cols());
	vCopyRectangle2(temp_classifier_matrix, &classifier_matrix, 0, 0, 
		(long) dimensions, (long) temp_classifier_matrix->Cols(), 0, 0);
	vdelete(temp_classifier_matrix);
	AddClassifierMatrix2(classifier_matrix, dimensions);
	vdelete2(path_name);

	LoadSensitiveClassifier2(filename, dimensions);

	return 1;
}


// Adds, in the order in which they are given, all the classifiers
// stored in the rows of the matrix.
vint8 class_BoostMap::AddClassifierMatrix(vMatrix<float> classifier_matrix)
{
	return AddClassifierMatrix2(classifier_matrix, -1);
}


// Adds, in the order in which they are given, all the classifiers
// stored in the rows of the matrix. It stops when either it has
// gone through all the rows of the matrix, or the next row of the
// matrix will increase the number of unique classifiers to dimensions+1.
vint8 class_BoostMap::AddClassifierMatrix2(vMatrix<float> classifier_matrix, 
	vint8 dimensions)
{
	vint8 number = classifier_matrix.Rows();
	vArray2(float) classifiers = classifier_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 type = round_number(classifiers[i][0]);
		vint8 object1 = round_number(classifiers[i][1]);
		vint8 object2 = round_number(classifiers[i][2]);
		float weight = classifiers[i][3];
		float z = classifiers[i][4];

		class_triple_classifier classifier;
		if (type == 0)
		{
			classifier = class_triple_classifier((long) object1, weight, z);
		}
		else if (type == 1)
		{
			classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
		}
		else
		{
			exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
		}

		AddClassifier(classifier);

		// Check if we exceeded the desired number of dimensions, in
		// which case we need to backtrack and return.
		if (dimensions >= 0)
		{
			vint8 current_dimensions = unique_classifiers.size();
			if (current_dimensions == dimensions + 1)
			{
				// backtrack
				classifier.weight = -classifier.weight;
				AddClassifier(classifier);
				current_dimensions = unique_classifiers.size();
				if (current_dimensions != dimensions)
				{
					exit_error("Error: unsuccessful backtracking\n");
				}
				break;
			}
		}

		PrintSummary();
	}

	return 1;
}


// find, in classifiers (represented in matrix form), a classifier 
// matching the specified type, object1, object2.
vint8 sFindIndex(vint8 type, vint8 object1, vint8 object2, 
	vMatrix<float> classifiers)
{
	vint8 number = classifiers.Rows();

	vector<vint8> indices_found;

	vint8 i;
	for (i = 0; i < number; i++)
	{
		if (type != classifiers(i, 0)) continue;
		if (object1 != classifiers(i, 1)) continue;
		if ((type == 1) && (object2 != classifiers(i, 2))) continue;
		return i;
	}

	exit_error("error: no indices found in sFindIndex\n");
	return -1;
}


// the next function is useful when passed in a detailed classifier,
// by giving us only the first dimensions dimensions learned in the process
// of learning that classifier.
vMatrix<float> class_BoostMap::select_first_dimensions(vMatrix<float> classifier_matrix,
	vint8 dimensions)
{
	if ((classifier_matrix.valid() <= 0) || (classifier_matrix.Cols() < 5))
	{
		return vMatrix<float>();
	}

	vint8 rows = classifier_matrix.Rows();

	// we will be adding classifiers into bm.unique_classifiers, so that we 
	// can easily check at each step how many unique classifiers we got.
	class_BoostMap bm;

	vArray2(float) classifiers = classifier_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < rows; i++)
	{
		// get a classifier corresponding to the i-th row.
		vint8 type = round_number(classifiers[i][0]);
		vint8 object1 = round_number(classifiers[i][1]);
		vint8 object2 = round_number(classifiers[i][2]);
		float weight = classifiers[i][3];
		float z = classifiers[i][4];

		class_triple_classifier classifier;
		if (type == 0)
		{
			classifier = class_triple_classifier((long) object1, weight, z);
		}
		else if (type == 1)
		{
			classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
		}
		else
		{
			exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
		}

		if (classifier_matrix.horizontal() >= 7)
		{
			classifier.database_first = (long) round_number(classifiers[i][5]);
			classifier.database_second = (long) round_number(classifiers[i][6]);
		}

		// add classifier to unique_classifiers of bm.
		vint8 index = bm.clean_up_classifier(classifier);

		// Check if we exceeded the desired number of dimensions, in
		// which case we need to backtrack and return.
		if (dimensions > 0)
		{
			vint8 current_dimensions = bm.unique_classifiers.size();
			if (current_dimensions == dimensions + 1)
			{
				// backtrack
				classifier.weight = -classifier.weight;
				bm.clean_up_classifier(classifier);
				current_dimensions = bm.unique_classifiers.size();
				if (current_dimensions != dimensions)
				{
					exit_error("Error: unsuccessful backtracking\n");
				}
				break;
			}
		}
	}

	vMatrix<float> new_matrix = bm.ClassifierMatrix();
	if (classifier_matrix.Cols() < 7)
	{
		return new_matrix;
	}

	// if classifier_matrix has information about the original objects
	// to which the indices correspond, then we should keep that
	// in the result.
	rows = new_matrix.Rows();
	vint8 cols = 7;
	vMatrix<float> result(rows, cols);
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < 5; col++)
		{
			result(row, col) = new_matrix(row, col);
		}
		vint8 index = sFindIndex((vint8) new_matrix(row, 0), (vint8) new_matrix(row, 1),
			(vint8) new_matrix(row, 2), classifier_matrix);
		// to catch bugs, verify it is the right index
		if ((new_matrix(row, 1) != classifier_matrix(index, 1)) || 
			(new_matrix(row, 2) != classifier_matrix(index, 2)))
		{
			exit_error("error: bad index in select_first_dimensions\n");
		}

		result(row, 5) = classifier_matrix(index, 5);
		result(row, 6) = classifier_matrix(index, 6);
	}

	return result;
}



// The cutoff under which z must be in order to accept weight modifications.
float class_BoostMap::ChangeCutoff()
{
	float diff = 1 - last_new_z;
	float threshold1 = 1 - (diff / (float) 3.0);
	float threshold2 = Max((float) .999, threshold1);
	return threshold2;
}


// returns number of lipschitz-embedding-type classifiers 
// in cleaned_classifiers.
vint8 class_BoostMap::LipschitzSteps()
{
	vint8 count = 0;
	vint8 i;
	vint8 number = unique_classifiers.size();

	for (i = 0; i < number; i++)
	{
		if (unique_classifiers[(vector_size) i].type == 0)
		{
			count++;
		}
	}
	return count;
}


// returns number of line-projection-type classifiers 
// in cleaned_classifiers.
vint8 class_BoostMap::ProjectionSteps()
{
	vint8 count = 0;
	vint8 i;
	vint8 number = unique_classifiers.size();

	for (i = 0; i < number; i++)
	{
		if (unique_classifiers[(vector_size) i].type == 1)
		{
			count++;
		}
	}
	return count;
}


// Returns the classifiers in a matrix form, that is easier to 
// save and load.
vMatrix<float> class_BoostMap::ClassifierMatrix()
{
	vint8 steps = unique_classifiers.size();
	vMatrix<float> result_matrix(steps, 8);
	vArray2(float) result = result_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		class_triple_classifier c = unique_classifiers[(vector_size) i];
		result[i][0] = (float) c.type;
		result[i][1] = (float) c.object1;
		result[i][2] = (float) c.object2;
		result[i][3] = c.weight;
		result[i][4] = c.z;

		// if we don't have candidates (for example if we do feature selection
		// for alignment to prototypes), we should put in result5 the same
		// as in result1
		if ((allow_lipschitz == 0) && (allow_projections == 0))
		{
			result[i][5] = result[i][1];
			result[i][6] = result[i][2];
		}
		// candidate ids can be zero, for example if we call this function
		// from IndexErrors, when we don't have a valid boostmap object,
		// we just want to obtain a list of unique classifiers.
		else if (predicate_zero(candidate_ids))
		{
			result[i][5] = (float) c.database_first;
			result[i][6] = (float) c.database_second;
		}
		else
		{
			result[i][5] = (float) candidate_ids[c.object1];
			if (c.object2 >= 0)
			{
				result[i][6] = (float) candidate_ids[c.object2];
			}
			else
			{
				result[i][6] = (float) -1;
			}
		}

		result[i][7] = (float) i;
	}

	return result_matrix;
}


// Returns the classifiers (not from unique_classifiers, but from
// classifiers) in a matrix form, that is easier to  save and load.
// This matrix contains more information than the matrix returned
// in ClassifierMatrix. Here we have a row for each training step
// that was performed. We also have additional columns, for example
// for training and validation errors and perrors.
vMatrix<float> class_BoostMap::DetailedClassifierMatrix()
{
	vint8 steps = classifiers.size();
	vMatrix<float> result_matrix(steps, 12);
	vArray2(float) result = result_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		class_triple_classifier c = classifiers[(vector_size) i];
		result[i][0] = (float) c.type;
		result[i][1] = (float) c.object1;
		result[i][2] = (float) c.object2;
		result[i][3] = c.weight;
		result[i][4] = c.z;

		// if we don't have candidates (for example if we do feature selection
		// for alignment to prototypes), we should put in result5 the same
		// as in result1
		if ((allow_lipschitz == 0) && (allow_projections == 0))
		{
			result[i][5] = result[i][1];
			result[i][6] = result[i][2];
		}
		else
		{
			result[i][5] = (float) candidate_ids[c.object1];
			if (c.object2 >= 0)
			{
				result[i][6] = (float) candidate_ids[c.object2];
			}
			else
			{
				result[i][6] = (float) -1;
			}
		}

		result[i][7] = training_errors[(vector_size) i];
		result[i][8] = validation_errors[(vector_size) i];
		result[i][9] = training_perrors[(vector_size) i];
		result[i][10] = validation_perrors[(vector_size) i];
		result[i][11] = (float) i;
	}

	return result_matrix;
}


// For debugging.
// TestTriple prints out info about a triple, prints the current
// margin for that triple, and verifies that margin by computing
// the distance from scratch. We print that info for the index-th
// training triple and the index-th validation triple.
vint8 class_BoostMap::TestTriple(vint8 index)
{
	if (index < 0) 
	{
		return 0;
	}

	if (training_triple_matrix.valid() <= 0)
	{
		return 0;
	}

	if (unique_classifiers.size() == 0)
	{
		return 0;
	}

	vMatrix<float> classifier_matrix = ClassifierMatrix();
	vMatrix<float> weights = ExtractWeights(classifier_matrix);

	if (index < training_triple_matrix.Rows())
	{
		// get q, a, b corresponding to index-th triple.
		vint8 q = training_triples[index][0];
		vint8 a = training_triples[index][1];
		vint8 b = training_triples[index][2];

		// get the id of those triples in the database.
		vint8 q_id = training_ids[q];
		vint8 a_id = training_ids[a];
		vint8 b_id = training_ids[b];

		// get the corresponding distances and margins, in feature space
		// and parameter space.
		float margin = training_margins_matrix(index);
		float pmargin = training_pmargins_matrix(index);
		float distance = training_distances(index);
		float pdistance = training_pdistances(index);

		// embed q, a, b.
		vMatrix<float> q_vector = Embedding2(q, candtrain_distances_matrix);
		vMatrix<float> a_vector = Embedding2(a, candtrain_distances_matrix);
		vMatrix<float> b_vector = Embedding2(b, candtrain_distances_matrix);

		// compute L1 distances from embedding of q to embeddings of a and b.
		vMatrix<float> qa_temp = BoostMap_data::L1Distances(&q_vector, a_vector, weights);
		float qa_distance = qa_temp(0);
		vMatrix<float> qb_temp = BoostMap_data::L1Distances(&q_vector, b_vector, weights);
		float qb_distance = qb_temp(0);

		// compute what should be equal to the margin (or pmargin, 
		// depending on whether we optimize on feature space or parameter space).
		float result = qb_distance - qa_distance;

		vPrint("not including sensitive classifiers:\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("qa_distance = %10.5f, qb_distance = %10.5f\n", qa_distance, qb_distance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
		vPrint("result      = %10.5f\n", result);
		vPrint("\n");

		// now redo things including weights of sensitive clasifiers.
		weights = QueryWeights(&q_vector);

		qa_temp = BoostMap_data::L1Distances(&q_vector, a_vector, weights);
		qa_distance = qa_temp(0);
		qb_temp = BoostMap_data::L1Distances(&q_vector, b_vector, weights);
		qb_distance = qb_temp(0);
		result = qb_distance - qa_distance;

		vPrint("including sensitive classifiers:\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("qa_distance = %10.5f, qb_distance = %10.5f\n", qa_distance, qb_distance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
		vPrint("result      = %10.5f\n", result);
		vPrint("\n");
	}

	vPrint("\n");

	// now repeat what we did for the index-th triple, for
	// the index-th validation triple.
	if (index < validation_triple_matrix.Rows())
	{
		vint8 q = validation_triples[index][0];
		vint8 a = validation_triples[index][1];
		vint8 b = validation_triples[index][2];

		vint8 q_id = validation_ids[q];
		vint8 a_id = validation_ids[a];
		vint8 b_id = validation_ids[b];

		float margin = validation_margins_matrix(index);
		float pmargin = validation_pmargins_matrix(index);
		float distance = validation_distances(index);
		float pdistance = validation_pdistances(index);

		vMatrix<float> q_vector = Embedding2(q, candval_distances_matrix);
		vMatrix<float> a_vector = Embedding2(a, candval_distances_matrix);
		vMatrix<float> b_vector = Embedding2(b, candval_distances_matrix);

		vMatrix<float> qa_temp = BoostMap_data::L1Distances(&q_vector, a_vector, weights);
		float qa_distance = qa_temp(0);
		vMatrix<float> qb_temp = BoostMap_data::L1Distances(&q_vector, b_vector, weights);
		float qb_distance = qb_temp(0);
		float result = qb_distance - qa_distance;

		vPrint("not including sensitive classifiers:\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("qa_distance = %10.5f, qb_distance = %10.5f\n", qa_distance, qb_distance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
		vPrint("result      = %10.5f\n", result);
		vPrint("\n");

		// now redo things including weights of sensitive clasifiers.
		weights = QueryWeights(&q_vector);

		qa_temp = BoostMap_data::L1Distances(&q_vector, a_vector, weights);
		qa_distance = qa_temp(0);
		qb_temp = BoostMap_data::L1Distances(&q_vector, b_vector, weights);
		qb_distance = qb_temp(0);
		result = qb_distance - qa_distance;

		vPrint("including sensitive classifiers:\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("qa_distance = %10.5f, qb_distance = %10.5f\n", qa_distance, qb_distance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
		vPrint("result      = %10.5f\n", result);
		vPrint("\n");
	}

	return 1;
}


// print a range of triples, whose indices are in {start, ..., end}
vint8 class_BoostMap::PrintTrainingTriples(vint8 start, vint8 end)
{
	if (training_triple_matrix.valid() == 0)
	{
		return 0;
	}

	if (start < 0) 
	{
		start = 0;
	}

	if ((end >= training_triple_matrix.Rows()) ||
		(end < 0))
	{
		end = training_triple_matrix.Rows() -1;
	}

	vint8 i;
	for (i = start; i <= end; i++)
	{
		vint8 q = training_triples[i][0];
		vint8 a = training_triples[i][1];
		vint8 b = training_triples[i][2];

		vint8 q_id = training_ids[q];
		vint8 a_id = training_ids[a];
		vint8 b_id = training_ids[b];

		vPrint("%10i: (%6li, %6li, %6li), (%6li, %6li, %6li)\n",
			(long) i, (long) q, (long) a, (long) b, (long) q_id, (long) a_id, (long) b_id);
	}

	return 1;
}


vint8 class_BoostMap::PrintValidationTriples(vint8 start, vint8 end)
{
	if (validation_triple_matrix.valid() == 0)
	{
		return 0;
	}

	if (start < 0) 
	{
		start = 0;
	}

	if ((end >= validation_triple_matrix.Rows()) ||
		(end < 0))
	{
		end = validation_triple_matrix.Rows() -1;
	}

	vint8 i;
	for (i = start; i <= end; i++)
	{
		vint8 q = validation_triples[i][0];
		vint8 a = validation_triples[i][1];
		vint8 b = validation_triples[i][2];

		vint8 q_id = validation_ids[q];
		vint8 a_id = validation_ids[a];
		vint8 b_id = validation_ids[b];

		vPrint("%10i: (%6li, %6li, %6li), (%6li, %6li, %6li)\n",
			(long) i, (long) q, (long) a, (long) b, (long) q_id, (long) a_id, (long) b_id);
	}

	return 1;
}

// TripleEmbeddings prints out the embedding for each object in 
// a triple, the distances qa and qb, and qa-qb.
vint8 class_BoostMap::TripleEmbeddings(vint8 index)
{
	if (index < 0) 
	{
		return 0;
	}

	if (training_triple_matrix.valid() <= 0)
	{
		return 0;
	}

	if (unique_classifiers.size() == 0)
	{
		return 0;
	}

	vMatrix<float> classifier_matrix = ClassifierMatrix();
	vMatrix<float> weights = ExtractWeights(classifier_matrix);

	if (index < training_triple_matrix.Rows())
	{
		// get (q, a, b) and the corresponding ids into the 
		// database, for the index-th triple.
		vint8 q = training_triples[index][0];
		vint8 a = training_triples[index][1];
		vint8 b = training_triples[index][2];

		vint8 q_id = training_ids[q];
		vint8 a_id = training_ids[a];
		vint8 b_id = training_ids[b];

		// get distances and margins, in feature and parameter space.
		float margin = training_margins_matrix(index);
		float pmargin = training_pmargins_matrix(index);
		float distance = training_distances(index);
		float pdistance = training_pdistances(index);

		// embed q, a, b.
		vMatrix<float> q_vector = Embedding2(q, candtrain_distances_matrix);
		vMatrix<float> a_vector = Embedding2(a, candtrain_distances_matrix);
		vMatrix<float> b_vector = Embedding2(b, candtrain_distances_matrix);

		// compute embedding distances from q to a and b (excluding 
		// query-sensitive part).
		vMatrix<float> qa_temp = BoostMap_data::L1Distances(&q_vector, a_vector, weights);
		float qa_distance = qa_temp(0);
		vMatrix<float> qb_temp = BoostMap_data::L1Distances(&q_vector, b_vector, weights);
		float qb_distance = qb_temp(0);
		float result = qb_distance - qa_distance;

		vint8 size = q_vector.Size();
		vint8 i;
		float margin2 = 0;
		for (i = 0; i < size; i++)
		{
			float qi = q_vector(i);
			float ai = a_vector(i);
			float bi = b_vector(i);
			float current = Power(vAbs(qi - bi)) - Power(vAbs(qi - ai));
			float weight = weights(i);
			margin2 = margin2 + weight * current;

			vPrint("%5li:%10.5f%10.5f%10.5f%10.5f%10.5f%10.5f\n",
				(long) i, qi, ai, bi, current, weight * current, margin2);
		}

		vPrint("\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("qa_distance = %10.5f, qb_distance = %10.5f\n", qa_distance, qb_distance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
		vPrint("result      = %10.5f, margin2 =   %10.5f\n", result, margin2);
	}

	return 1;
}


// TripleResults simulates the process by which training_margins[index]
// got its current value, and prints, for each classifier, the contribution
// that this classifier makes to this margin.
// It does not handle query-sensitive embeddings.
vint8 class_BoostMap::TripleResults(vint8 index)
{
	if (index < 0) 
	{
		return 0;
	}

	if (training_triple_matrix.valid() <= 0)
	{
		return 0;
	}

	if (unique_classifiers.size() == 0)
	{
		return 0;
	}

	if (index < training_triple_matrix.Rows())
	{
		// get (q, a, b) and corresponding indices into database.    
		vint8 q = training_triples[index][0];
		vint8 a = training_triples[index][1];
		vint8 b = training_triples[index][2];

		vint8 q_id = training_ids[q];
		vint8 a_id = training_ids[a];
		vint8 b_id = training_ids[b];

		// get distances and margins in feature and parameter space.
		float margin = training_margins_matrix(index);
		float pmargin = training_pmargins_matrix(index);
		float distance = training_distances(index);
		float pdistance = training_pdistances(index);

		float margin2 = 0;
		vint8 steps = unique_classifiers.size();
		vector<float> results((vector_size) training_triple_number);
		vint8 i;

		// now recompute margin by going through the strong classifier.
		for (i = 0; i < steps; i++)
		{
			class_triple_classifier classifier = unique_classifiers[(vector_size) i];

			// get results of weak classifier on all triples
			ClassifierResults(&classifier, &results, training_triple_matrix,
				ground_truth, candtrain_distances_matrix);

			// get result of weak classifier on current triple
			float current_estimate = classifier.weight * results[(vector_size) index];

			// Add the contribution of the last chosen reference images to 
			// the contributions of all previous reference images.
			float total_estimate = margin2 + current_estimate;
			margin2 = total_estimate;
			vPrint("%5li:  %10.5f    %10.5f    %10.5f\n",
				(long) i, results[(vector_size) index], current_estimate, total_estimate);
		}


		vPrint("\n");
		vPrint("q = %li, a = %li, b = %li\n", (long) q, (long) a, (long) b);
		vPrint("q_id = %li, a_id = %li, b_id = %li\n", (long) q_id, (long) a_id, (long) b_id);
		vPrint("distance    = %10.5f, pdistance   = %10.5f\n", distance, pdistance);
		vPrint("margin      = %10.5f, pmargin =   %10.5f\n", margin, pmargin);
	}

	return 1;
}


// pick the next query-sensitive weak classifier. We just call
// NextSensitiveClassifier, and add the classifier calling
// AddSensitiveClassifier.
vint8 class_BoostMap::NextSensitiveStep()
{
	if ((is_valid <= 0) || (allow_sensitive == 0))
	{
		return 0;
	}

	class_sensitive_classifier new_classifier = NextSensitiveClassifier();
	last_new_z = new_classifier.classifier.z;
	vPrint("\n");
	new_classifier.Print();
	vPrint("\n");

	vint8 result = AddSensitiveClassifier(new_classifier);
	return result;
}


// identify the next sensitive classifier
class_sensitive_classifier class_BoostMap::NextSensitiveClassifier()
{
	vector<class_sensitive_classifier> sensitive_candidates;

	// get candidate weak classifiers.
	GetSensitiveCandidates(&sensitive_candidates);

	if (sensitive_candidates.size() == 0) 
	{
		vPrint("no sensitive candidates are available\n");
		return class_sensitive_classifier();
	}

	vint8 z_min_index = -1;
	float best_alpha = (float) 0;
	float alpha = 0;
	float z_min = SensitiveZ(&(sensitive_candidates[0]), &best_alpha);

	z_min_index = 0;
	vint8 i;
	vPrint("\n");

	// go through the candidates, and find the one that achieves
	// the lowest z.
	for (i = 1; i < (vint8) sensitive_candidates.size(); i++)
	{
		float z = SensitiveZ(&(sensitive_candidates[(vector_size) i]), &alpha);
		if (z < z_min)
		{
			z_min = z;
			z_min_index = i;
			best_alpha = alpha;
		}
		vPrint("evaluated classifier %li of %li\r", 
			(long) (i+1), (long) sensitive_candidates.size());
	}
	vPrint("\n");

	// store into the best classifier its alpha and z.
	class_sensitive_classifier result = sensitive_candidates[(vector_size) z_min_index];
	result.classifier.weight = best_alpha;
	result.classifier.z = z_min;
	return result;
}


// choose candidate sensitive classifiers. They are built
// based on unique_classifiers (at least in the current, restricted
// implementation). Roughly half of them use the same 1D embedding
// both as splitter and as classifier, and half of them use
// a different splitter. For each combination of splitter/classifier
// we try lots of different ranges for the splitter.
vint8 class_BoostMap::GetSensitiveCandidates(vector<class_sensitive_classifier> * classifiers)
{
	// first, construct a list of base classifiers, from which
	// we will pick splitter/decider pairs for the query-sensitive
	// classifiers.
	vector<class_triple_classifier> base_classifiers;

	// Check if we want to limit ourselves to classifiers already
	// appearing in unique_classifiers.
	if (new_sensitive == 0)
	{
		base_classifiers.insert(base_classifiers.begin(),
			unique_classifiers.begin(),
			unique_classifiers.end());
	}
	// otherwise, we can use base classifiers that have not
	// been picked yet.
	else
	{
		exit_error("error: unimplemented\n");
	}

	// There are too many possible ranges for the splitter. We don't
	// want to consider them all, to take less time, and to 
	// avoid overfitting. range_step specifies how we sample ranges.
	const vint8 range_step = training_number / 40;
	if (range_step <= 2)
	{
		return 0;
	}

	vector<float> training_embeddings((vector_size) training_number);
	vint8 i;

	// go through the base classifiers, and use them to define
	// query-sensitive classifiers.
	for (i = 0; i < (vint8) base_classifiers.size(); i++)
	{
		class_triple_classifier base = base_classifiers[(vector_size) i];
		class_triple_classifier splitter;

		// we flip a coin. Half the times the splitter is the same as the 
		// base, and half the times it is different (chosen randomly from
		// all base classifiers).
		vint8 random_pick = function_random_vint8(0, 1);
		if (random_pick == 0)
		{
			splitter = base;
		}
		else
		{
			// pick splitter randomly.
			vint8 random_pick2 = function_random_vint8(0, base_classifiers.size() - 1);
			splitter = base_classifiers[(vector_size) random_pick2];
		}

		// compute the embeddings of all training objects.
		TrainingOneDim(splitter, &training_embeddings);
		std::sort(training_embeddings.begin(), training_embeddings.end());
		class_sensitive_classifier sensitive;
		sensitive.splitter = splitter;
		sensitive.classifier = base;
		sensitive.distance_factor = distance_max_entry;
		vint8 j;

		// construct many different ranges.
		for (j = range_step; j <= training_number - range_step; j = j + range_step)
		{
			// Here we accept all values greater than low, and low is such that
			// it excludes a number of training objects that is a multiple of
			// range_step.
			sensitive.split_type = 0;
			sensitive.low = training_embeddings[(vector_size) j];
			sensitive.high = sensitive.low - 1;
			sensitive.range = training_number - j;
			classifiers->push_back(sensitive);

			// Here we accept all values smaller than high, and high is such that
			// it excludes a number of training objects that is a multiple of
			// range_step.
			sensitive.split_type = 1;
			sensitive.high = training_embeddings[(vector_size) j];
			sensitive.low = sensitive.high + 1;
			sensitive.range = j; 
			classifiers->push_back(sensitive);

			// Here we accept values such that low <= value < high. We sample
			// low and high so that the number of training embeddings
			// with values < low is a multiple of range_step/2, and same 
			// for training embeddings with values >= high.
			vint8 index1 = training_number/2 - j/2;
			vint8 index2 = training_number/2 + j/2;
			if ((index1 < 0) || (index1 >= training_number))
			{
				continue;
			}
			sensitive.low = training_embeddings[(vector_size) index1];
			sensitive.high = training_embeddings[(vector_size) index2];

			sensitive.split_type = 2;
			sensitive.range = j;
			classifiers->push_back(sensitive);

			// Here we use the same structure as before, but we 
			// set the type to 3, meaning that we accept values < low
			// and values >= high.
			sensitive.split_type = 3;
			sensitive.range = training_number - j;
			classifiers->push_back(sensitive);
		}
	}

	return 1;
}


// Number of query-sensitive classifiers chosen so far.
vint8 class_BoostMap::SensitiveStepsDone()
{
	return sensitive_classifiers.size();
}


// measure z and alpha (weight) for the given query-sensitive classifier.
float class_BoostMap::SensitiveZ(class_sensitive_classifier * classifier, 
	float * alpha)
{
	vector<float> splitter_one_dim((vector_size) training_number);
	vector<float> classifier_one_dim((vector_size) training_number);
	vector<float> results((vector_size) training_triple_number);

	// get embeddings of training objects based on splitter and decider.
	TrainingOneDim(classifier->splitter, &splitter_one_dim);
	TrainingOneDim(classifier->classifier, &classifier_one_dim);

	// get classification results 
	SensitiveResults(classifier, &splitter_one_dim, &classifier_one_dim,
		training_triple_matrix,
		ground_truth, &results);  

	float a = 0, z = 0;
	// find the best z and associated weight (alpha), based on the 
	// classification results.
	iterations = iterations + vas_int16((long) MinimizeZ(&results, &z, &a));
	*alpha = a;
	return z;
}


// get the result (classification result times class label)
// corresponding to each triple in triples_matrix, for the
// given classifier. results is an output argument.
// splitter_one_dim and classifier_one_dim are the one-D 
// embeddings of all (training or validation) objects
// based on the splitter and the classifier embedding.
// triples_matrix should be either training triples
// or validation triples, and triple_distances_matrix
// should be the matrix that corresponds to triples_matrix.
vint8 class_BoostMap::SensitiveResults(class_sensitive_classifier * classifier, 
	vector<float> * splitter_one_dim,
	vector<float> * classifier_one_dim,
	vint8_matrix triple_matrix,
	vMatrix<float> triple_distances_matrix,
	vector<float> * results)
{
	vint8 number = triple_matrix.Rows();

	vint8 i;
	vArray2(vint8) triples = triple_matrix.Matrix2();
	vArray(float) triple_distances = triple_distances_matrix.Matrix();
	vint8 set_size = splitter_one_dim->size();
	if (set_size != classifier_one_dim->size())
	{
		exit_error("error: sensitiveresults, set_size = %li, != %li\n",
			(long) set_size, (long) classifier_one_dim->size());
	}

	for (i = 0; i < number; i++)
	{
		vint8 q = triples[i][0];
		vint8 a = triples[i][1];
		vint8 b = triples[i][2];

		if ((q < 0) || (a < 0) || (b < 0) ||
			(q >= set_size) || (a >= set_size) || (b >= set_size))
		{
			exit_error("Error: (q,a,b) = (%li ,%li ,%li), set_size = %li\n",
				(long) q, (long) a, (long) b, (long) set_size);
		}

		float label = Label(triple_distances[i]);

		float q_1d = (*splitter_one_dim)[(vector_size) q];
		if (classifier->Split(q_1d) == 0)
		{
			(*results)[(vector_size) i] = (float) 0;
			continue;
		}

		float qr = (*classifier_one_dim)[(vector_size) q];
		float ar = (*classifier_one_dim)[(vector_size) a];
		float br = (*classifier_one_dim)[(vector_size) b];

		(*results)[(vector_size) i] = (Power(vAbs(qr - br)) - Power(vAbs(qr - ar))) * label;
	}

	return 1;
}


// Add the sensitive classifier to sensitive_classifiers,
// and do the necessary updates (compute errors, update training
// weights).
vint8 class_BoostMap::AddSensitiveClassifier(class_sensitive_classifier classifier)
{
	vint8 strictness_flag = !new_sensitive;
	classifier.classifier_index = FindClassifierIndex(classifier.classifier,
		strictness_flag);
	if ((classifier.classifier_index < 0) || (new_sensitive != 0))
	{
		class_triple_classifier classifier_copy = classifier.classifier;
		// This weight is just over 0.000001, which is the threshold for 
		// removing a classifier in unique_classifier.
		classifier_copy.weight = (float) .0000011;
		AddClassifier(classifier_copy);
	}

	vector<float> results((vector_size) training_triple_number);

	vector<float> training_splitter((vector_size) training_number);
	vector<float> training_classifier((vector_size) training_number);
	vector<float> validation_splitter((vector_size) validation_number);
	vector<float> validation_classifier((vector_size) validation_number);

	// embed training objects based on splitter, and based on
	// decider.
	TrainingOneDim(classifier.splitter, &training_splitter);
	TrainingOneDim(classifier.classifier, &training_classifier);
	ValidationOneDim(classifier.splitter, &validation_splitter);
	ValidationOneDim(classifier.classifier, &validation_classifier);

	// compute error and perror info for the weak classifier.
	SensitiveResults(&classifier, &training_splitter, &training_classifier,
		training_triple_matrix, training_distances, &results);
	ComputeLastError(classifier.classifier.weight, &results);

	SensitiveResults(&classifier, &training_splitter, &training_classifier,
		training_triple_matrix, training_pdistances, &results);
	ComputeLastPerror(classifier.classifier.weight, &results);


	// add unique classifiers, find the indices;
	// Note that this will not work if these classifiers were not
	// already in unique_classifiers (i.e. in the case where
	// new_sensitive is 1). I need to write code to handle that.
	// One case where it wouldn't work, would be if we have already
	// added a zero-weight classifier into unique_classifiers, and
	// then we try to add it again. Then, the way clean_up_classifier
	// works, we would just end up removing the first classifier.
	// Again, this should be easy to enforce if we have a member
	// variable that specifies when to remove stuff and when not
	// to remove stuff from unique_classifiers.
	//
	// Here we add zero-weight copies of splitter and decider,
	// so that we add them and ensure they are in unique_classifiers,
	// but they don't change the unique_classifiers mathematically.
	// They need to be in unique_classifiers, so that when we compute
	// the embeddings we only need to look at unique_classifiers, and
	// so that we maintain the property that each splitter or decider
	// used in sensitive_classifiers corresponds to a classifier
	// in 
	//  class_triple_classifier splitter_copy = classifier.splitter;
	//  class_triple_classifier classifier_copy = classifier.classifier;
	//  splitter_copy.weight = 0;
	//  classifier_copy.weight = 0;

	// add the zero-weight copies into unique_classifiers
	//  clean_up_classifier(splitter_copy);
	//  clean_up_classifier(classifier_copy);

	// find the indices for splitter and decider into unique_classifiers
	//  classifier.splitter_index = FindClassifierIndex(splitter_copy);
	//  classifier.classifier_index = FindClassifierIndex(classifier_copy);
	classifier.classifier_index = FindClassifierIndex(classifier.classifier);
	classifier.splitter_index = FindClassifierIndex(classifier.splitter);

	// get the classification results for the classifier.
	SensitiveResults(&classifier, &training_splitter, &training_classifier,
		training_triple_matrix, ground_truth, &results);
	sensitive_classifiers.push_back(classifier);

	// compute z for the classifier
	float z = (float) Z(classifier.classifier.weight, &results);
	classifier.classifier.z = z;
	UpdateWeights(&results, z, classifier.classifier.weight);

	// compute training and validation errors and perrors.
	SensitiveResults(&classifier, &training_splitter, &training_classifier,
		training_triple_matrix, training_distances, &results);
	float margin = ComputeTrainingError(&results, 1);

	SensitiveResults(&classifier, &training_splitter, &training_classifier,
		training_triple_matrix, training_pdistances, &results);
	ComputeTrainingPerror(&results, 1);

	vector<float> validation_results((vector_size) validation_triple_number);
	SensitiveResults(&classifier, &validation_splitter, &validation_classifier,
		validation_triple_matrix, validation_distances, 
		&validation_results);
	ComputeValidationError(&validation_results, 1);

	SensitiveResults(&classifier, &validation_splitter, &validation_classifier,
		validation_triple_matrix, validation_pdistances, 
		&validation_results);
	ComputeValidationPerror(&validation_results, 1);

	training_errors.push_back(training_error);
	validation_errors.push_back(validation_error);
	training_perrors.push_back(training_perror);
	validation_perrors.push_back(validation_perror);


	return 1;
}


// we return the one-dimensional embeddings of all training objects
// based on this classifier. We assume that results already has
// enough allocated space to store the embeddings.
vint8 class_BoostMap::TrainingOneDim(class_triple_classifier classifier,
	vector<float> * results)
{
	if (is_valid == 0)
	{
		return 0;
	}

	vint8 i;
	// go through training objects
	for (i = 0; i < training_number; i++)
	{
		// check if we have a reference-object-type embedding
		if (classifier.type == 0)
		{
			vint8 reference = classifier.object1;
			float distance = candtrain_distances[reference][i];
			(*results)[(vector_size) i] = distance;
		}

		// check if we have a line-projection-type embedding
		else if (classifier.type == 1)
		{
			vint8 pivot1 = classifier.object1;
			vint8 pivot2 = classifier.object2;

			float i_pivot1 = candtrain_distances[pivot1][i];
			float i_pivot2 = candtrain_distances[pivot2][i];
			float pivot_distance = candcand_distances[pivot1][pivot2];

			float embedding = V_FastMap::LineProjection3(i_pivot1, i_pivot2, 
				pivot_distance);
			(*results)[(vector_size) i] = embedding;
		}
	}

	return 1;
}


float_matrix class_BoostMap::compute_training_embeddings(class_triple_classifier classifier)
{
	if (is_valid == 0)
	{
		return float_matrix();
	}

	float_matrix result(1, training_number);

	vint8 i;
	// go through training objects
	for (i = 0; i < training_number; i++)
	{
		// check if we have a reference-object-type embedding
		if (classifier.type == 0)
		{
			vint8 reference = classifier.object1;
			float distance = candtrain_distances[reference][i];
			result(i) = distance;
		}

		// check if we have a line-projection-type embedding
		else if (classifier.type == 1)
		{
			vint8 pivot1 = classifier.object1;
			vint8 pivot2 = classifier.object2;

			float i_pivot1 = candtrain_distances[pivot1][i];
			float i_pivot2 = candtrain_distances[pivot2][i];
			float pivot_distance = candcand_distances[pivot1][pivot2];

			float embedding = V_FastMap::LineProjection3(i_pivot1, i_pivot2, 
				pivot_distance);
			result(i) = embedding;
		}
	}

	return result;
}


float_matrix class_BoostMap::compute_embeddings(class_triple_classifier classifier,
	float_matrix distances)
{
	if (is_valid == 0)
	{
		return float_matrix();
	}

	float_matrix result(1, distances.Cols());

	vint8 i;
	// go through training objects
	for (i = 0; i < distances.Cols(); i++)
	{
		// check if we have a reference-object-type embedding
		if (classifier.type == 0)
		{
			vint8 reference = classifier.object1;
			float distance = distances(reference, i);
			result(i) = distance;
		}

		// check if we have a line-projection-type embedding
		else if (classifier.type == 1)
		{
			vint8 pivot1 = classifier.object1;
			vint8 pivot2 = classifier.object2;

			float i_pivot1 = distances(pivot1, i);
			float i_pivot2 = distances(pivot2, i);
			float pivot_distance = candcand_distances[pivot1][pivot2];

			float embedding = V_FastMap::LineProjection3(i_pivot1, i_pivot2, 
				pivot_distance);
			result(i) = embedding;
		}
	}

	return result;
}


// we return the one-dimensional embeddings of all validation objects
// based on this classifier. We assume that results already has
// enough allocated space to store the embeddings. Similar
// to what we do for TrainingOneDim.
vint8 class_BoostMap::ValidationOneDim(class_triple_classifier classifier,
	vector<float> * results)
{
	if (is_valid == 0)
	{
		return 0;
	}

	vint8 i;
	for (i = 0; i < validation_number; i++)
	{
		if (classifier.type == 0)
		{
			vint8 reference = classifier.object1;
			float distance = candval_distances[reference][i];
			(*results)[(vector_size) i] = distance;
		}

		else if (classifier.type == 1)
		{
			vint8 pivot1 = classifier.object1;
			vint8 pivot2 = classifier.object2;

			float i_pivot1 = candval_distances[pivot1][i];
			float i_pivot2 = candval_distances[pivot2][i];
			float pivot_distance = candcand_distances[pivot1][pivot2];

			float embedding = V_FastMap::LineProjection3(i_pivot1, i_pivot2, 
				pivot_distance);
			(*results)[(vector_size) i] = embedding;
		}
	}

	return 1;
}


// This should be called to load the sensitive classifiers from a file.
char * class_BoostMap::sensitive_pathname(const char * filename)
{
	char * directory = Directory(g_data_directory);
	char * temp_path_name = vJoinPaths(directory, filename);
	char * path_name;
	if (vStringCaseEndsIn(filename, "d"))
	{
		path_name = vMergeStrings2(temp_path_name, "s.txt");
	}
	else
	{
		path_name = vMergeStrings2(temp_path_name, "ds.txt");
	}

	vdelete2(directory);
	vdelete2(temp_path_name);
	return path_name;
}


// This should be called to save sensitive classifiers to a file.
char * class_BoostMap::sensitive_pathname2(const char * filename)
{
	//vint8 number = (classifiers.size() + sensitive_classifiers.size()) % 10;
	//char * number_string = string_from_number(number);
	//char * directory = Directory();
	//char * temp_path_name = vJoinPaths(directory, filename);
	//char * path_name = vMergeStrings4(temp_path_name, "_", 
	//                                   number_string, "ds.txt");
	//vdelete2(number_string);
	//vdelete2(directory);
	//vdelete2(temp_path_name);
	//return path_name;

	char * directory = Directory();
	char * temp_path_name = vJoinPaths(directory, filename);
	char * path_name = vMergeStrings2(temp_path_name, "ds.txt");
	vdelete2(directory);
	vdelete2(temp_path_name);
	return path_name;
}


// These functions are called from save_classifier and load_classifier
// to write/read the filenames storing the query-sensitive (sensitive)
// classifiers.
vint8 class_BoostMap::SaveSensitiveClassifier(const char * filename)
{
	if (is_valid == 0)
	{
		return 0;
	}

	if (sensitive_classifiers.size() == 0)
	{
		return 0;
	}

	// figure out the pathname.
	char * path_name = sensitive_pathname2(filename);

	// write the classifier into the file (in matrix form.
	vMatrix<float> classifier = SensitiveClassifierMatrix();
	vint8 success = classifier.WriteText(path_name);
	if (success <= 0)
	{
		vPrint("Failed to save sensitive classifier to %s\n", path_name);
	}
	else
	{
		vPrint("Saved sensitive classifier to %s\n", path_name);
	}

	vdelete2(path_name);
	return success;
}


// Called from load_classifier to read the query-sensitive
// classifiers.
vint8 class_BoostMap::LoadSensitiveClassifier(const char * filename)
{
	// make sure the pathname is correct.
	char * path_name = sensitive_pathname(filename);
	if (vFileExists(path_name) == 0)
	{
		vPrint("special path name %s does not exist\n", path_name);
		vdelete2(path_name);
		return 0;
	}

	// load the embedding description, in matrix form.
	vMatrix<float> classifier_matrix = vMatrix<float>::ReadText(path_name);
	if (classifier_matrix.valid() == 0)
	{
		vPrint("Failed to load sensitive classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded sensitive classifier from %s\n", path_name);
	}

	// add one by one all the one-dimensional embeddings
	// specified in rows of classifier_matrix.
	AddSensitiveMatrix(classifier_matrix);
	vdelete2(path_name);
	return 1;
}


// loads the first "number" classifiers.
vint8 class_BoostMap::LoadSensitiveClassifier2(const char * filename,
	vint8 number)
{
	// make sure the pathname is correct.
	char * path_name = sensitive_pathname(filename);
	if (vFileExists(path_name) == 0)
	{
		vPrint("special path name %s does not exist\n", path_name);
		vdelete2(path_name);
		return 0;
	}

	// load the embedding description, in matrix form.
	vMatrix<float> classifier_matrix = vMatrix<float>::ReadText(path_name);
	if (classifier_matrix.valid() == 0)
	{
		vPrint("Failed to load sensitive classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded sensitive classifier from %s\n", path_name);
	}

	if (number > classifier_matrix.Rows())
	{
		number = classifier_matrix.Rows();
	}
	vint8 cols = classifier_matrix.Cols();
	vMatrix<float> selected(number, cols);
	vCopyRectangle2(&classifier_matrix, &selected, 0, 0, (long) number, (long) cols,
		0, 0);

	// add one by one all the one-dimensional embeddings
	// specified in rows of selected.
	AddSensitiveMatrix(selected);
	vdelete2(path_name);
	return 1;
}


// converts the classifiers in sensitive_classifiers into 
// a convenient (for some things) matrix form.
vMatrix<float> class_BoostMap::SensitiveClassifierMatrix()
{
	if (is_valid <= 0)
	{
		return vMatrix<float>();
	}

	if (sensitive_classifiers.size() == 0)
	{
		return vMatrix<float>();
	}

	vint8 rows = sensitive_classifiers.size();
	vint8 cols = SensitiveMatrixCols();
	vMatrix<float> result_matrix(rows, cols);

	vint8 i;
	// go through the rows of the matrix, and convert each of them
	// into a classifier.
	for (i = 0; i < rows; i++)
	{
		vMatrix<float> classifier_row = SensitiveToMatrix(sensitive_classifiers[(vector_size) i]);
		function_put_row(&classifier_row, &result_matrix, (long) i);
		result_matrix(i, cols-1) = (float) i;
	}

	return result_matrix;
}


// we add the query-sensitive classifiers contained in 
// sensitive_matrix into sensitive_classifiers, and we
// treat them like weak classifiers picked during training
// (so we compute the corresponding errors, and we update
// the training weights).
vint8 class_BoostMap::AddSensitiveMatrix(vMatrix<float> sensitive_matrix)
{
	if ((sensitive_matrix.valid() <= 0) || 
		(sensitive_matrix.Cols() < SensitiveMatrixCols()))
	{
		return 0;
	}

	vint8 rows = sensitive_matrix.Rows();
	vint8 cols = sensitive_matrix.Cols();

	vint8 i;
	for (i = 0; i < rows; i++)
	{
		class_sensitive_classifier classifier = MatrixToSensitive(sensitive_matrix, i);
		AddSensitiveClassifier(classifier);
	}

	return 1;
}


// convert a query-sensitive classifier into a row matrix.
vMatrix<float> class_BoostMap::SensitiveToMatrix(class_sensitive_classifier classifier)
{
	vint8 cols = SensitiveMatrixCols();
	vMatrix<float> result_matrix(1, cols);
	vArray(float) result = result_matrix.Matrix();

	vint8 object1 = classifier.splitter.object1;
	vint8 object2 = classifier.splitter.object2;
	vint8 object3 = classifier.classifier.object1;
	vint8 object4 = classifier.classifier.object2;

	result[0] = (float) classifier.splitter.type;
	result[1] = (float) object1;
	result[2] = (float) object2;
	result[3] = (float) candidate_ids[object1];
	if (object2 >= 0)
	{
		result[4] = (float) candidate_ids[object2];
	}
	else
	{
		result[4] = (float) -1;
	}

	result[5] = (float) classifier.classifier.type;
	result[6] = (float) object3;
	result[7] = (float) object4;
	result[8] = (float) candidate_ids[object3];
	if (object4 >= 0)
	{
		result[9] = (float) candidate_ids[object4];
	}
	else
	{
		result[9] = (float) -1;
	}

	result[10] = classifier.low;
	result[11] = classifier.high;
	result[12] = (float) classifier.split_type;
	result[13] = (float) classifier.range;
	result[14] = classifier.classifier.weight;
	result[15] = classifier.classifier.z;
	result[16] = training_error;
	result[17] = validation_error;
	result[18] = training_perror;
	result[19] = validation_perror;
	result[20] = classifier.distance_factor;
	result[21] = classifier.distance_multiplier;

	return result_matrix;
}


// convert the row-th row of matrix into a sensitive classifier.
class_sensitive_classifier class_BoostMap::MatrixToSensitive(vMatrix<float> matrix, vint8 row)
{
	if ((row < 0) || (row >= matrix.Rows()) || (matrix.valid() <= 0))
	{
		exit_error("error: matrix to sensitive, row = %li\n", (long) row);
	}

	if (matrix.Cols() <= 15)
	{
		exit_error("error: matrixtosensitive, cols = %li\n", (long) matrix.Cols());
	}

	vArray2(float) entries = matrix.Matrix2();
	vArray(float) data = entries[row];

	class_sensitive_classifier classifier;
	classifier.splitter.type = (long) round_number(data[0]);
	classifier.splitter.object1 = (long) round_number(data[1]);
	classifier.splitter.object2 = (long) round_number(data[2]);

	classifier.classifier.type = (long) round_number(data[5]);
	classifier.classifier.object1 = (long) round_number(data[6]);
	classifier.classifier.object2 = (long) round_number(data[7]);

	classifier.low = data[10];
	classifier.high = data[11];
	classifier.split_type = (long) round_number(data[12]);
	classifier.range = (long) round_number(data[13]);
	classifier.classifier.weight = data[14];
	classifier.classifier.z = data[15];
	classifier.distance_factor = data[20];

	if (matrix.Cols() >= 23)
	{
		classifier.distance_multiplier = data[21];
	}

	return classifier;
}


// returns the number of columns that a matrix of sensitive classifiers
// should contain. This changes when I decide to add to or take things out
// from that matrix, and when we read from file we use this function
// as a sanity check to make sure we are reading something that is
// not obviously an obsolete format. A better way to do this would
// be to store a format version in each matrix.
vint8 class_BoostMap::SensitiveMatrixCols()
{
	vint8 result = 23;
	return result;
}


// the next few functions are for exploring whether we can come up
// with heuristics that would speed up search time. In ClassifierStatistics
// we do almost what we would do for NextStep, but we also collect some 
// statistics, and we may skip optimizing z, since that is costly. 
// Essentially, here we define a classifier based on the arguments, and we
// measure how many other candidate classifiers achieve better training
// errors and margins (calculated on the weighted training set of triples).
vint8 class_BoostMap::ClassifierStatistics(vint8 in_type, vint8 object1, vint8 object2)
{
	// choose candidate weak classifiers to compare with the specified 
	// classifier. Here we only choose reference-object embeddings (the 
	// second half of the function handles line-projection embeddings).
	ChooseCandidates(number_of_picked_candidates);
	vector<float> results((long) training_triple_number);
	class_triple_classifier classifier;
	if (in_type == 0)
	{
		classifier = class_triple_classifier((long) object1, 1, 1);
	}
	else
	{
		classifier = class_triple_classifier((long) object1, (long) object2, 1, 1);
	}
	ClassifierResults(&classifier, &results, training_triple_matrix,
		ground_truth, candtrain_distances_matrix);
	// get error and margin of specified classifier.
	float in_error = ClassifierWeightedError(&results);
	float in_margin = ClassifierWeightedMargin(&results);

	// if negative weights are allowed, then an error rate higher than 50%
	// turns into an error rate lower than 50% by choosing a negative weight.
	if (allow_negative != 0)
	{
		in_error = Min(in_error, 1 - in_error);
		in_margin = vAbs(in_margin);
	}

	float alpha = 0, z = 0;
	z = ClassifierZ(&classifier, &alpha);

	vint8 error_counter = 0;
	vint8 margin_counter = 0;
	float min_error = 100;
	float max_margin = -1000;
	vPrint("\n");
	vint8 i;

	// go through the reference-object candidate 
	// classifiers, to compare them 
	// with the specified classifier. This for 
	for (i = 0; i < number_of_picked_candidates; i++)
	{
		vint8 cobject1 = candidates[i];
		// if the specified classifier (based on the arguments)
		// is a reference-object-type classifier, then discard
		// the current candidate if it is a similar 1D embedding
		// (i.e. that has the same type and same reference object).
		if ((in_type == 0) && (object1 == cobject1))
		{
			continue;
		}
		// get the classification results for current candidate.
		class_triple_classifier classifier((long) cobject1, 1, 1);
		ClassifierResults(&classifier, &results, training_triple_matrix, 
			ground_truth, candtrain_distances_matrix);

		// get weighted training error and margin for current classifier.
		float error = ClassifierWeightedError(&results);
		float margin = ClassifierWeightedMargin(&results);

		// if negative weights are allowed, consider the negation
		// of the current classifier.
		if (allow_negative != 0)
		{
			error = Min(in_error, 1 - error);
			margin = vAbs(margin);
		}

		// compare current classifier to specified classifier.
		if (error < in_error)
		{
			error_counter++;
		}
		if (margin > in_margin)
		{
			margin_counter++;
		}

		if (error < min_error)
		{
			min_error = error;
		}
		if (margin > max_margin)
		{
			max_margin = margin;
		}

		vPrint("%li and %li better, processed %li of %li lipschitz classifiers\r",
			(long) error_counter, (long) margin_counter, (long) (i+1), (long) number_of_picked_candidates);
	}
	vPrint("\n");

	// now produce candidates for line-projection embeddings
	vint8_matrix temp_candidates1 = sample_without_replacement(0, candidate_number - 1, 
		candidate_number);
	vint8_matrix temp_candidates(&temp_candidates1);
	vint8_matrix candidate_pivots_matrix = RandomPivots(temp_candidates, 
		projection_candidate_number);
	vArray2(vint8) candidate_pivots = candidate_pivots_matrix.Matrix2();

	vPrint("\n");
	// go through line-projection embeddings and compare them
	// to specified classifier. Very similar to the previous for-loop.
	for (i = 0; i < projection_candidate_number; i++)
	{
		vint8 cobject1 = candidate_pivots[i][0];
		vint8 cobject2 = candidate_pivots[i][1];

		if ((in_type == 1) && (object1 == cobject1) &&
			(object2 == cobject2))
		{
			continue;
		}
		class_triple_classifier classifier((long) cobject1, (long) cobject2, 1, 1);
		ClassifierResults(&classifier, &results, training_triple_matrix, 
			ground_truth, candtrain_distances_matrix);
		float error = ClassifierWeightedError(&results);
		float margin = ClassifierWeightedMargin(&results);

		if (allow_negative != 0)
		{
			error = Min(in_error, 1 - error);
			margin = vAbs(margin);
		}

		if (error < in_error)
		{
			error_counter++;
		}
		if (margin > in_margin)
		{
			margin_counter++;
		}

		if (error < min_error)
		{
			min_error = error;
		}
		if (margin > max_margin)
		{
			max_margin = margin;
		}

		vPrint("%li and %li better, processed %li of %li projection classifiers\r",
			(long) error_counter, (long) margin_counter, (long) (i+1), (long) number_of_picked_candidates);
	}
	vPrint("\n");

	vPrint("error = %f,       margin = %f,   alpha = %f, z = %f\n",
		in_error, in_margin, alpha, z);
	vPrint("min_error = %f,   max_margin = %f\n", min_error, max_margin);

	return 1;
}

// AppendClassifiers loads some classifiers from file, but it skips
// as many of them as the number of steps performed so far. After
// it skips those, it adds as many dimensions as specified. This is 
// useful if we want to add classifiers a few at a time, and check
// certain things after we add some of them.
vint8 class_BoostMap::AppendClassifiers(const char * filename, vint8 dimensions)
{
	char * path_name = Pathname(g_data_directory, filename);

	vMatrix<float> classifier_matrix = vMatrix<float>::ReadText(path_name);
	if (classifier_matrix.valid() <= 0)
	{
		vPrint("Failed to load classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}

	vdelete2(path_name);

	// check if there are any classifiers left after we skip the first ones.
	vint8 rows = classifier_matrix.Rows();
	if (rows < (vint8) classifiers.size())
	{
		vPrint("only %li classifiers available, %li steps done\n",
			(long) rows, (long) classifiers.size());
		return 0;
	}

	// if we have in classifiers exactly as many classifiers as the rows
	// of classifier_matrix, then we go on to load and add some
	// of the sensitive classifiers.
	else if (rows == classifiers.size())
	{
		return AppendSensitiveClassifiers(filename, dimensions);
	}

	// we figure out how many rows are left to add after we skip the 
	// first ones.
	vint8 remaining = rows - classifiers.size();

	// limit is the number of unique classifiers (dimensionality of embedding)
	// we should have (at most) at the end of this function.
	vint8 limit = unique_classifiers.size() + dimensions;
	vint8 i;
	vint8 number = classifiers.size();

	// keep adding rows, until we run out of rows, or we reach the limit
	// of dimensions (actually, until we exceed the number of dimensions,
	// and then we backtrack).
	for (i = 0; i < remaining; i++)
	{
		v3dMatrix<float> temp = copy_horizontal_line(&classifier_matrix, number + i);
		vMatrix<float> next_row(&temp);

		// convert row into classifier.
		vint8 type = round_number(next_row(0));
		vint8 object1 = round_number(next_row(1));
		vint8 object2 = round_number(next_row(2));
		float weight = next_row(3);
		float z = next_row(4);
		class_triple_classifier classifier;
		if (type == 0)
		{
			classifier = class_triple_classifier((long) object1, weight, z);
		}
		else
		{
			classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
		}

		// add the classifier.
		AddClassifier(classifier);

		// if we have exceeded the limit of dimensions, then we should 
		// backtrack.
		if (unique_classifiers.size() > (ulong) limit)
		{
			classifier.weight = -classifier.weight;
			AddClassifier(classifier);
			classifiers.pop_back();
			classifiers.pop_back();
			break;
		}
	}

	// if we have not reached the limit, add in some of the sensitive
	// classifiers.
	if (unique_classifiers.size() < (ulong) limit)
	{
		return AppendSensitiveClassifiers(filename, dimensions - unique_classifiers.size());
	}

	return 1;
}


// AppendSensitiveClassifiers loads some 
// query-sensitive classifiers from file, but it skips
// as many of them as the number of query-sensitive classifiers
// stored so far into speciailized_classifiers. After
// it skips those, it adds as many dimensions as specified.
vint8 class_BoostMap::AppendSensitiveClassifiers(const char * filename, 
	vint8 dimensions)
{
	// open the file storing query-sensitive classifiers
	char * path_name = sensitive_pathname(filename);
	if (vFileExists(path_name) == 0)
	{
		vPrint("special path name %s does not exist\n", path_name);
		vdelete2(path_name);
		return 0;
	}

	// read the query-sensitive classifiers.
	vMatrix<float> classifier_matrix = vMatrix<float>::ReadText(path_name);
	if (classifier_matrix.valid() == 0)
	{
		vPrint("Failed to load sensitive classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded sensitive classifier from %s\n", path_name);
	}
	vdelete2(path_name);

	vint8 rows = classifier_matrix.Rows();
	if (rows <= (vint8) sensitive_classifiers.size())
	{
		vPrint("only %li classifiers available, %li steps done\n",
			(long) rows, (long) sensitive_classifiers.size());
		return 0;
	}

	// adjust dimensions, in case we don't have enough rows in
	// the classifier matrix.
	dimensions = Min<vint8>(rows - sensitive_classifiers.size(), dimensions);
	vint8 i;
	vint8 number = sensitive_classifiers.size();

	// add the weak classifiers, one by one.
	for (i = 0; i < dimensions; i++)
	{
		v3dMatrix<float> temp = copy_horizontal_line(&classifier_matrix, number + i);
		vMatrix<float> next_row(&temp);
		AddSensitiveMatrix(next_row);
		vPrint("added dimension %li of %li, %li total sensitive classifiers\n",
			(long) (i+1), (long) dimensions, (long) (number + i + 1));
	}

	return 1;
}



// computes the error of specified classifier, on weighted
// training set.
float class_BoostMap::ClassifierWeightedError(vint8 type, vint8 object1, vint8 object2)
{
	class_triple_classifier classifier;
	if (type == 0)
	{
		classifier = class_triple_classifier((long) object1, 1, 1);
	}
	else
	{
		classifier = class_triple_classifier((long) object1, (long) object2, 1, 1);
	}

	float result = ClassifierWeightedError(classifier);
	return result;
}


// computes the error of specified classifier, on weighted
// training set.
float class_BoostMap::ClassifierWeightedError(class_triple_classifier classifier)
{
	vector<float> results((long) training_triple_number);
	ClassifierResults(&classifier, &results, training_triple_matrix,
		ground_truth, candtrain_distances_matrix);
	float result = ClassifierWeightedError(&results);
	return result;
}


// computes the error of classifier with given results, on weighted
// training set.
float class_BoostMap::ClassifierWeightedError(vector<float> * results)
{
	if (results->size() != training_triple_number)
	{
		return 0;
	}

	double sum = 0;
	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		double weight = training_factors[i];
		double result = (*results)[(long) i];
		double error;
		if (result > 0.00000)
		{
			error = 0;
		}
		else if (result < -.00000)
		{
			error = 1;
		}
		else
		{
			error = 0.5;
		}

		double contribution = weight * error;
		sum = sum + contribution;
	}

	return (float) sum;
}


// computes the margin of specified classifier, on weighted
// training set.
float class_BoostMap::ClassifierWeightedMargin(vint8 type, vint8 object1, vint8 object2)
{
	class_triple_classifier classifier;
	if (type == 0)
	{
		classifier = class_triple_classifier((long) object1, 1, 1);
	}
	else
	{
		classifier = class_triple_classifier((long) object1, (long) object2, 1, 1);
	}

	float result = ClassifierWeightedMargin(classifier);
	return result;
}




// computes the margin of specified classifier, on weighted
// training set.
float class_BoostMap::ClassifierWeightedMargin(class_triple_classifier classifier)
{
	vector<float> results((vector_size) training_triple_number);
	ClassifierResults(&classifier, &results, training_triple_matrix,
		ground_truth, candtrain_distances_matrix);
	float result = ClassifierWeightedMargin(&results);
	return result;
}




// computes the margin of classifier with given results, on weighted
// training set.
float class_BoostMap::ClassifierWeightedMargin(vector<float> * results)
{
	if (results->size() != training_triple_number)
	{
		return 0;
	}

	double sum = 0;
	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		double weight = training_factors[(vector_size) i];
		double result = (*results)[(vector_size) i];
		double contribution = weight * result;
		sum = sum + contribution;
	}

	return (float) sum;
}


// returns sensitive classifiers that are limited to 
// using classifiers from unique_classifiers. For each
// splitter/classifier pair we consider, we find the 
// range that leads to the lowest training error using
// BestClassifier.
vint8 class_BoostMap::RestrictedSensitiveCandidates(vector<class_sensitive_classifier> * classifiers)
{
	vint8 number = unique_classifiers.size();
	vint8 i;
	vPrint("\n");
	for (i = 0; i < number; i++)
	{
		class_triple_classifier base = unique_classifiers[(vector_size) i];
		class_triple_classifier splitter;

		// pick a splitter for this base (50% of the times splitter = base).
		vint8 random_pick = function_random_vint8(0, 1);
		if (random_pick == 0)
		{
			splitter = base;
		}
		else
		{
			vint8 random_pick2 = function_random_vint8(0, unique_classifiers.size() - 1);
			splitter = unique_classifiers[(vector_size) random_pick2];
		}

		// find the best range for the splitter/base pair.
		class_sensitive_classifier classifier = BestClassifier(splitter, base);
		classifiers->push_back(classifier);
		vPrint("processed %li of %li base classifiers\r",
			(long) (i+1), (long) number);
	}
	vPrint("\n");

	return 1;
}


// create some random sensitive classifiers (not limiting ourselves
// to using classifiers already in unique_classifiers). For each
// splitter/classifier pair we consider, we find the 
// range that leads to the lowest training error using
// BestClassifier.
// To keep the overall complexity of embedding the query low, we
// always have, in this case, splitter = base. Note that
// in the RestrictedSensitiveCandidates splitter = base only half
// the times.
vint8 class_BoostMap::RandomSensitiveCandidates(vector<class_sensitive_classifier> * classifiers)
{
	vector<class_triple_classifier> base_classifiers;
	RandomCandidates(&base_classifiers);

	vint8 i;
	vPrint("\n");
	vint8 number = base_classifiers.size();
	for (i = 0; i < (vint8) base_classifiers.size(); i++)
	{
		class_triple_classifier base = base_classifiers[(vector_size) i];
		class_triple_classifier splitter = base;

		// find the best range for this classifier.
		class_sensitive_classifier classifier = BestClassifier(splitter, base);
		classifiers->push_back(classifier);
		vPrint("processed %li of %li base classifiers\r",
			(long) (i+1), (long) number);
	}
	vPrint("\n");

	return 1;
}


// create some random candidate weak classifiers, that will compute
// for the next classifier to be chosen. Right now this function
// is only used in FastNextStep(), in principle I should also
// use it in NextStep().
vint8 class_BoostMap::RandomCandidates(vector<class_triple_classifier> * classifiers)
{
	// choose some random reference objects.
	ChooseCandidates(number_of_picked_candidates);
	vint8 i;

	// add the reference-object embeddings corresponding
	// to those objects.
	if (allow_lipschitz != 0)
	{
		for (i = 0; i < candidates_matrix.Size(); i++)
		{
			vint8 object1 = candidates[(vector_size) i];
			class_triple_classifier classifier((long) object1, 1, 1);
			classifiers->push_back(classifier);
		}
	}

	// choose some different reference objects (this time we essentially
	// choose all candidates).
	if (allow_projections != 0)
	{
		vint8_matrix temp_candidates1 = sample_without_replacement(0, candidate_number - 1, 
			candidate_number);

		vint8_matrix temp_candidates(&temp_candidates1);

		// choose random pairs of those objects, to be used as candidate
		// pivot pairs.
		vint8_matrix candidate_pivots_matrix = RandomPivots(temp_candidates, 
			projection_candidate_number);
		vArray2(vint8) candidate_pivots = candidate_pivots_matrix.Matrix2();

		// form the classifier corresponding to each pivot pair.
		for (i = 0; i < candidate_pivots_matrix.Rows(); i++)
		{
			vint8 cobject1 = candidate_pivots[i][0];
			vint8 cobject2 = candidate_pivots[i][1];

			class_triple_classifier classifier((long) cobject1, (long) cobject2, 1, 1);
			classifiers->push_back(classifier);
		}
	}

	return 1;
}



// Given some constraint on the range, we evaluate lots of possible 
// sensitive classifiers built using splitter and classifier, and
// we return the one with the lowest training error.
class_sensitive_classifier class_BoostMap::BestClassifier(class_triple_classifier splitter,
	class_triple_classifier classifier)
{
	vector<float> split_one_dim((vector_size) training_number);
	vector<vint8> range_starts((vector_size) training_number+1);
	vector<float> split_results((vector_size) training_triple_number);
	vector<float> classifier_results((vector_size) training_triple_number);
	vector<float> sorted_sresults((vector_size) training_triple_number);
	vector<float> sorted_cresults((vector_size) training_triple_number);
	vector<double> sorted_weights((vector_size) training_triple_number);

	// get embeddings of training set based on splitter.
	TrainingOneDim(splitter, &split_one_dim);

	// get results on triples based on classifier.
	ClassifierResults(&classifier, &classifier_results, training_triple_matrix,
		ground_truth, candtrain_distances_matrix);

	// split_results[i] will be the result of embedding q, for the i-th
	// triple, using the splitter.
	vint8 i;
	for (i = 0; i < training_triple_number; i++)
	{
		vint8 q = training_triples[i][0];
		float q_1d = split_one_dim[(vector_size) q];
		split_results[(vector_size) i] = q_1d;
	}

	// get ranks of different training triples, based on result
	// of splitter on q.
	vector<vint8> split_ranks;
	vIndexRanks(&split_results, &split_ranks);

	// sorted_sresults[i] is the i-th lowest value of embedding
	// (using splitter) an object q from a training triple (q, a, b).
	// sorted_cresults[i] is (for the same i-th lowest value)
	// the classification result using classifier.
	// sorted_weights[i] is (for the same i-th lowest value) the
	// corresponding training weight.
	for (i = 0; i < training_triple_number; i++)
	{
		vint8 index = split_ranks[(vector_size) i];

		// in principle, sorted_sresults should get values in between,
		// and not exact values.
		sorted_sresults[(vector_size) index] = split_results[(vector_size) i];
		sorted_cresults[(vector_size) index] = classifier_results[(vector_size) i];
		sorted_weights[(vector_size) index] = training_factors[(vector_size) i];
	}

	// add a maximum number into sorted_results, it will make
	// the iteration more convenient (we won't have to 
	// check for bounds).
	float max_1d = sorted_sresults[(vector_size) training_triple_number - 1];
	sorted_sresults.push_back((float) 2 * max_1d + (float) 1.0);

	// sort the splitter-based embeddings of training objects,
	// to figure out the points between which we will define ranges.
	// range_starts[i] will hold the number of training triples
	// for which the splitter-based embedding of q is less than
	// the embedding of the i-th training object.
	std::sort(split_one_dim.begin(), split_one_dim.end(), less<float>());
	vint8 index = 0;
	for (i = 0; i < training_number; i++)
	{
		float q_1d = split_one_dim[(vector_size) i];
		while (sorted_sresults[(vector_size) index] < q_1d)
		{
			index++;
			if (index >= training_triple_number)
			{
				// For the protein dataset, we can get here.
				//        exit_error("error: bestclassifier, index = %li\n", index);
				index--;
				break;
			}
		}
		range_starts[(vector_size) i] = index;
	}

	// combine sorted_weights and sorted_cresults into 
	// errors, which will give the weighted training error
	// for each training triple (in sorted order, based
	// on sorted_sresults).
	vector<double> errors((vector_size) training_triple_number);
	for (i = 0; i < training_triple_number; i++)
	{
		double weight = sorted_weights[(vector_size) i];
		float result = sorted_cresults[(vector_size) i];
		if (result > .000001)
		{
			errors[(vector_size) i] = 0.0;
		}
		else if (result < -.000001)
		{
			errors[(vector_size) i] = weight;
		}
		else 
		{
			errors[(vector_size) i] = weight / (double) 2.0;
		}
	}

	// error_sums[i] is the sum of weighted errors of training triples
	// from 0 to i.
	// weight_sums[i] is the sum of trainign weights of training triples
	// from 0 to i.
	// In both cases, triples are examined in order of increasing
	// splitter-based embedding value of q (i.e. the order in which
	// the splitter-based embedding of q appears in sorted_sresults).
	vector<double> error_sums((vector_size) training_triple_number);
	vector<double> weight_sums((vector_size) training_triple_number);
	error_sums[0] = errors[0];
	weight_sums[0] = sorted_weights[0];
	for (i = 1; i < training_triple_number; i++)
	{
		error_sums[(vector_size) i] = error_sums[(vector_size) i-1] + errors[(vector_size) i];
		weight_sums[(vector_size) i] = weight_sums[(vector_size) i-1] + sorted_weights[(vector_size) i];
	}

	// range_step specifies how many ranges we will try, and
	// how widely apart the endpoints of the ranges will be
	// spaced.
	const vint8 range_step = training_number / 40;
	if (range_step < 2)
	{
		exit_error("error: range step = %li\n", (long) range_step);
	}

	// We initialize min_error with the error we get by not using the
	// splitter.
	vint8 best_type = 4;
	float best_low = -1;
	float best_high = -1;
	vint8 best_range = training_triple_number;
	float min_error2 = ClassifierWeightedError(classifier);
	double min_error = error_sums[(vector_size) training_triple_number - 1];

	vint8 j;
	// go through all boundary points we use for defining a range.
	for (j = range_step; j <= training_number - range_step; j = j + range_step)
	{
		// consider following possible ranges (maybe I got some parentheses
		// and brackets wrong here, but the endpoints are correct).
		// [low_index, infinity),
		// [-infinity, high_index) (note that possibly high_index < low_index)
		// [index1, index2)
		// R - [index1, index2).
		vint8 low_index = range_starts[(vector_size) j];
		vint8 high_index = range_starts[(vector_size) (training_number - j)];
		vint8 index1 = training_number/2 - j/2;
		vint8 index2 = training_number/2 + j/2;

		double error, low, high;
		vint8 type, range;

		// error for type 0, values >= low.
		type = 0;
		low = sorted_sresults[(vector_size) low_index];
		range = training_triple_number - low_index;

		error = error_sums[(vector_size) training_triple_number - 1] - error_sums[(vector_size) low_index];
		error = error + weight_sums[(vector_size) low_index-1] / (float) 2.0;
		if (error < min_error)
		{
			min_error = error;
			best_type = type;
			best_low = (float) low;
			best_range = range;
		}

		// error for type 1, values < high
		type = 1;
		high = sorted_sresults[(vector_size) high_index];
		range = high_index;
		error = error_sums[(vector_size) high_index-1];
		error = error + (weight_sums[(vector_size) training_triple_number - 1] - 
			weight_sums[(vector_size) high_index-1]) / (float) 2.0;
		if (error < min_error)
		{ 
			min_error = error;
			best_type = type;
			best_high = (float) high;
			best_range = range;
		}

		// error for types 2 and 3.
		low_index = range_starts[(vector_size) index1];
		high_index = range_starts[(vector_size) index2];
		low = sorted_sresults[(vector_size) low_index];
		high = sorted_sresults[(vector_size) high_index];

		// error for type 2, low <= value < high
		type = 2;
		range = high_index - low_index;
		error = error_sums[(vector_size) high_index-1] - error_sums[(vector_size) low_index-1];
		error = error + weight_sums[(vector_size) low_index-1] / (float) 2.0;
		error = error + (weight_sums[(vector_size) training_triple_number - 1] - 
			weight_sums[(vector_size) high_index-1]) / (float) 2.0;
		if (error < min_error)
		{ 
			min_error = error;
			best_type = type;
			best_low = (float) low;
			best_high = (float) high;
			best_range = range;
		}

		// error for type 3, value < low OR value >= high
		type = 3;
		range = low_index + (training_triple_number - high_index);
		error = error_sums[(vector_size) low_index-1];
		error = error + error_sums[(vector_size) training_triple_number - 1] - error_sums[(vector_size) high_index-1];
		error = error + (weight_sums[(vector_size) high_index-1] - weight_sums[(vector_size) low_index-1]) / (float) 2.0;
		if (error < min_error)
		{ 
			min_error = error;
			best_type = type;
			best_low = (float) low;
			best_high = (float) high;
			best_range = range;
		}
	}

	// form a sensitive (query-sensitive) classifier from the optimal
	// values that we found.
	class_sensitive_classifier result;
	result.splitter = splitter;
	result.classifier = classifier;
	result.splitter.weight = (float) min_error;
	result.split_type = best_type;
	result.low = best_low;
	result.high = best_high;
	result.range = best_range;
	result.distance_factor = distance_max_entry;

	return result;
}


// here we assume that, in classifiers, classifier.splitter.weight
// holds the weighted trainig error rate for that classifier.
// We put into selected the "number" classifiers with the lowest
// weighted training error rate.
vint8 class_BoostMap::SelectBest(vector<class_sensitive_classifier> * classifiers,
	vector<class_sensitive_classifier> * selected,
	vint8 k)
{
	vint8 i;
	vint8 number = classifiers->size();
	if (number == 0)
	{
		exit_error("error: selectbest, number = %li\n", (long) number);
	}
	else if (k > number)
	{
		k = number;
	}

	// prepare a vector of errors (so that we can call
	// kth_smallest_ca on that vector).
	vector<float> errors((vector_size) number);
	for (i = 0; i < number; i++)
	{
		errors[(vector_size) i] = (*classifiers)[(vector_size) i].splitter.weight;
	}

	// find the cutoff threshold.
	long junk = 0;
	float error_k = kth_smallest_ca((long) k, &errors, &junk);

	// select the classifiers that give errors below
	// the cutoff threshold.
	for (i = 0; i < number; i++)
	{
		if (errors[(vector_size) i] <= error_k)
		{
			selected->push_back((*classifiers)[(vector_size) i]);
		}
	}

	return 1;
}


// similar to the previous SelectBest, but here, instead
// of looking for classifier A at A.splitter.weight
// for the training error, we look that up in the 
// corresponding entry in errors. Note that we have to
// do that, because here we handle non-sensitive classifiers,
// whereas the previous SelectBest handles query-sensitive
// classifiers.
vint8 class_BoostMap::SelectBest(vector<class_triple_classifier> * classifiers,
	vector<class_triple_classifier> * selected,
	vector<float> * errors,
	vint8 k)
{
	vint8 i;
	vint8 number = classifiers->size();
	if (number == 0)
	{
		exit_error("error: selectbest, number = %li\n", (long) number);
	}
	else if (k > number)
	{
		k = number;
	}

	// find the cutoff threshold for error.
	long junk = 0;
	float error_k = kth_smallest_ca((long) k, errors, &junk);

	// select the classifiers with errors below the
	// cutoff threshold.
	for (i = 0; i < number; i++)
	{
		if ((*errors)[(vector_size) i] <= error_k)
		{
			selected->push_back((*classifiers)[(vector_size) i]);
		}
	}

	return 1;
}


// similar to the previous SelectBest functions, but this    
// one uses ClassifierWeightedError to figure out the
// training error of each classifier in classifiers.
vint8 class_BoostMap::SelectBest(vector<class_triple_classifier> * classifiers,
	vector<class_triple_classifier> * selected,
	vint8 number)
{
	vint8 size = classifiers->size();
	if (size == 0)
	{
		return 0;
	}
	vector<float> errors((vector_size) size);

	vint8 i;
	vPrint("\n");
	// store information into the vector of errors.
	for (i = 0; i < size; i++)
	{
		errors[(vector_size) i] = ClassifierWeightedError((*classifiers)[(vector_size) i]);
		vPrint("measured error of classifier %li of %li\r", (long) (i+1), (long) size);
	}
	vPrint("\n");

	return SelectBest(classifiers, selected, &errors, number);
}


vint8 class_BoostMap::fast_next_sensitive_step()
{
	use_small_training();
	vint8 result = fast_next_sensitive_actual();
	use_large_training();

	return result;
}

vint8 class_BoostMap::fast_next_sensitive_actual()
{
	if ((is_valid <= 0) || (allow_sensitive == 0))
	{
		return 0;
	}

	// get candidate query-sensitive classifiers.
	vector<class_sensitive_classifier> temp_candidates;
	if (new_sensitive == 0)
	{
		RestrictedSensitiveCandidates(&temp_candidates);
	}
	else
	{
		RandomSensitiveCandidates(&temp_candidates);
	}


	if (temp_candidates.size() == 0) 
	{
		vPrint("no sensitive candidates are available\n");
		return 1;
	}

	// decide how many classifiers to keep.
	vector<class_sensitive_classifier> best_candidates;
	vint8 keep_best;
	if (use_best_k < 0)
	{
		keep_best = temp_candidates.size();
	}
	else
	{
		keep_best = Min<vint8>(use_best_k, temp_candidates.size());
	}

	// select the most promising candidates.
	SelectBest(&temp_candidates, &best_candidates, keep_best);

	// go through the most promising candidates, and pick the
	// one that minimizes Z.
	vint8 z_min_index = -1;
	float best_alpha = (float) 0;
	float alpha = 0;
	float z_min = SensitiveZ(&(best_candidates[0]), &best_alpha);

	z_min_index = 0;
	vint8 i;
	vPrint("\n");
	for (i = 1; i < (vint8) best_candidates.size(); i++)
	{
		float z = SensitiveZ(&(best_candidates[(vector_size) i]), &alpha);
		if (z < z_min)
		{
			z_min = z;
			z_min_index = i;
			best_alpha = alpha;
		}
		vPrint("evaluated classifier %li of %li\r", 
			(long) (i+1), (long) best_candidates.size());
	}
	vPrint("\n");

	// form a classifier out of the result.
	class_sensitive_classifier result = best_candidates[(vector_size) z_min_index];
	result.classifier.weight = best_alpha;

	result.classifier.z = z_min;

	last_new_z = result.classifier.z;
	vPrint("\n");
	result.Print();
	vPrint("\n");

	// add the classifier to the strong clsasifier.
	use_large_training();
	vint8 return_value = AddSensitiveClassifier(result);
	return return_value;
}


// looks in unique_classifiers to find a classifier that 
// has the same type and reference objects as the argument.
// returns -1 if no classifier is found. strictness_flag is 
// an optional parameter. If it is set to 1 (the default value),
// failing to find an index for the given classifier is a 
// terminal error. If the strictness value is 0, then we 
// allow not finding the index. The strictness should be zero
// when we add sensitive classifiers, and we want to find
// the index of their components. In that case, it is OK 
// if we can't find the index, at least in some cases (when
// the new_sensitive flag is set).
vint8 class_BoostMap::FindClassifierIndex(class_triple_classifier classifier,
	vint8 strictness_flag)
{
	return FindClassifierIndex(classifier, &unique_classifiers, 
		strictness_flag);
}


// a static version of the previous function, useful when
// we don't have a BoostMap object, but we do have
// the unique_classifiers vector. 
vint8 class_BoostMap::FindClassifierIndex(class_triple_classifier classifier,
	vector<class_triple_classifier> * unique_classifiers,
	vint8 strictness_flag)
{
	vint8 number = unique_classifiers->size();
	vint8 result = -1;

	vector<vint8> indices_found;
	vint8 type = classifier.type;
	vint8 object1 = classifier.object1;
	vint8 object2 = classifier.object2;

	// find all indices where the classifiers match the input
	// classifier. We only call this function when exactly one index
	// should match. If 0 or more than 1 indics match, something is wrong.
	vint8 i;
	for (i = 0; i < number; i++)
	{
		class_triple_classifier & c = (*unique_classifiers)[(vector_size) i];
		if (type != c.type) continue;
		if (object1 != c.object1) continue;
		if ((type == 1) && (object2 != c.object2)) continue;
		indices_found.push_back(i);
	}

	// check how many indices we found.
	vint8 found = indices_found.size();
	if (found == 1)
	{
		result = indices_found[0];
	}

	// we found 0 or more than 1 indices, so something is wrong.
	else if (strictness_flag != 0)
	{
		vPrint("found = %li\n", (long) found);
		vint8 i;
		for (i = 0; i < found; i++)
		{
			vPrint("indices_found[%li] = %li\n", (long) i, (long) indices_found[(vector_size) i]);
			(*unique_classifiers)[(vector_size) indices_found[(vector_size) i]].Print();
		}

		exit_error("Error: Zero or duplicate classifiers in unique_classifiers\n");
	}

	return result;
}


// To make sure there are no bugs, we go through all sensitive 
// classifiers, and we verify that their splitter_index and
// classifier_index parameters are correct. If they are not correct
// we print a warning. Result is > 0 if there was no problem.
vint8 class_BoostMap::VerifySensitiveIndices()
{
	return VerifySensitiveIndices(&unique_classifiers, &sensitive_classifiers);
}


// a static version of the previous function, useful when we have the
// vectors but we don't have a BoostMap object.
vint8 class_BoostMap::VerifySensitiveIndices(const vector<class_triple_classifier> * unique_classifiers,
	const vector<class_sensitive_classifier> * sensitive_classifiers)
{
	vint8 number = sensitive_classifiers->size();
	vint8 unique_number = unique_classifiers->size();
	vint8 i;
	vint8 result = 1;
	for (i = 0; i < number; i++)
	{
		// get the next query-sensitive classifier.
		class_sensitive_classifier s_classifier = (*sensitive_classifiers)[(vector_size) i];
		if (s_classifier.distance_factor <= 0.0f)
		{
			exit_error("error: in VerifySensitiveIndices, distance_factor <= 0\n");
		}

		class_triple_classifier classifier = s_classifier.classifier;
		class_triple_classifier splitter = s_classifier.splitter;
		vint8 splitter_index = s_classifier.splitter_index;
		vint8 classifier_index = s_classifier.classifier_index;

		// do some sanity checks
		if ((splitter_index < 0) || (splitter_index >= unique_number))
		{
			result = 0;
		}
		else if ((classifier_index < 0) || (classifier_index >= unique_number))
		{
			result = 0;
		}

		// verify the two indices.
		else
		{
			class_triple_classifier s2 = (*unique_classifiers)[(vector_size) splitter_index];
			class_triple_classifier c2 = (*unique_classifiers)[(vector_size) classifier_index];

			if (check_classifiers(splitter, s2) == 0)
			{
				result = 0;
				splitter.Print();
				s2.Print();
			}
			else if (check_classifiers(classifier, c2) == 0)
			{
				result = 0;
				classifier.Print();
				c2.Print();
			}
		}

		if (result == 0)
		{
			exit_error("error: sensitive indices don't match: (%li, %li, %li)\n",
				(long) i, (long) splitter_index, (long) classifier_index);
			break;
		}
	}

	return result;
}


// we check if the two classifiers correspond to the same dimension
// of the embedding. This means that we don't care if the weights
// and z's are equal, but the type and objects must be equal.
vint8 class_BoostMap::check_classifiers(class_triple_classifier c1, 
	class_triple_classifier c2)
{
	vint8 result = 1;
	if ((c1.type != c2.type) || (c1.object1 != c2.object1))
	{
		result = 0;
	}
	else if ((c1.type == 1) && (c1.object2 != c2.object2))
	{
		result = 0;
	}

	return result;
}


// Go through all sensitive classifiers and set their 
// splitter_index and classifier_index parameters.
vint8 class_BoostMap::SetSensitiveIndices()
{
	return SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);
}


// a static version of the previous function.
vint8 class_BoostMap::SetSensitiveIndices(vector<class_triple_classifier> * unique_classifiers,
	vector<class_sensitive_classifier> * sensitive_classifiers)
{
	vint8 number = (*sensitive_classifiers).size();
	vint8 unique_number = (*unique_classifiers).size();
	vint8 i;
	vint8 result = 1;
	for (i = 0; i < number; i++)
	{
		// get the next query-sensitive classifier.
		class_sensitive_classifier & s_classifier = (*sensitive_classifiers)[(vector_size) i];
		class_triple_classifier classifier = s_classifier.classifier;
		class_triple_classifier splitter = s_classifier.splitter;

		// find the two indices.
		vint8 splitter_index = FindClassifierIndex(splitter, unique_classifiers);
		vint8 classifier_index = FindClassifierIndex(classifier, unique_classifiers);

		// do some sanity checks
		if ((splitter_index < 0) || (splitter_index >= unique_number))
		{
			result = 0;
		}
		else if ((classifier_index < 0) || (classifier_index >= unique_number))
		{
			result = 0;
		}

		// store the indices.
		else
		{
			s_classifier.splitter_index = splitter_index;
			s_classifier.classifier_index = classifier_index;
		}
		if (result == 0)
		{
			exit_error("error: sensitive indices don't match: (%li, %li, %li)\n",
				(long) i, (long) splitter_index, (long) classifier_index);
			break;
		}
	}

	// another sanity check
	VerifySensitiveIndices(unique_classifiers, sensitive_classifiers);
	return result;
}


// we return the weights that correspond to each dimension of
// the query, by taking into account which sensitive classifiers
// are applicable to the query. query_matrix is the embedding
// of the query. 
vMatrix<float> class_BoostMap::QueryWeights(const v3dMatrix<float> * query_matrix)
{
	return QueryWeights(query_matrix, &unique_classifiers,
		&sensitive_classifiers);
}


// a static version of the previous function.
vMatrix<float> class_BoostMap::QueryWeights(const v3dMatrix<float> * query_matrix,
	const vector<class_triple_classifier> * unique_classifiers,
	const vector<class_sensitive_classifier> * sensitive_classifiers)
{
	// sanity checks
	VerifySensitiveIndices(unique_classifiers, sensitive_classifiers);

	vArray(float) query = query_matrix->Matrix();
	vint8 query_dimensions = unique_classifiers->size();

	vMatrix<float> weight_matrix(1, query_dimensions);
	function_enter_value(&weight_matrix, (float) 0);
	vArray(float) weights = weight_matrix.Matrix();

	// initialize weights to the query-insensitive part of the distance
	// measure.
	vint8 i;
	for (i = 0; i < query_dimensions; i++)
	{
		weights[i] = (*unique_classifiers)[(vector_size) i].weight;
	}

	// now adjust weights based on the query-sensitive classifiers.
	vint8 sensitive_number = (*sensitive_classifiers).size();
	for (i = 0; i < sensitive_number; i++)
	{
		// get the next query-sensitive classifier.
		class_sensitive_classifier s_classifier = (*sensitive_classifiers)[(vector_size) i];
		class_triple_classifier classifier = s_classifier.classifier;
		vint8 splitter_index = s_classifier.splitter_index;
		vint8 classifier_index = s_classifier.classifier_index;

		// check if the splitter accepts the query. If so, add
		// the classifier's weight into weights.
		float splitter_value = query[splitter_index] / s_classifier.distance_factor;
		if (s_classifier.Split(splitter_value) > 0)
		{
			float weight = s_classifier.classifier.weight;
			weights[classifier_index] += weight;
		}
	}

	return weight_matrix;
}


// create triple classifiers out of the entries of a matrix.
vint8 class_BoostMap::MatrixToTclassifiers(vMatrix<float> classifier_matrix, 
	vector<class_triple_classifier> * unique_classifiers)
{
	vint8 number = classifier_matrix.Rows();
	if ((number == 0) || (classifier_matrix.valid() <= 0))
	{
		return 0;
	}

	vArray2(float) classifiers = classifier_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 type = round_number(classifiers[i][0]);
		vint8 object1 = round_number(classifiers[i][1]);
		vint8 object2 = round_number(classifiers[i][2]);
		float weight = classifiers[i][3];
		float z = classifiers[i][4];

		class_triple_classifier classifier;
		if (type == 0)
		{
			classifier = class_triple_classifier((long) object1, weight, z);
		}
		else if (type == 1)
		{
			classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
		}
		else
		{
			exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
		}

		if (classifier_matrix.horizontal() >= 7)
		{
			classifier.database_first = (long) round_number(classifier_matrix(i, 5));
			classifier.database_second = (long) round_number(classifier_matrix(i, 6));
		}

		clean_up_classifier(classifier, unique_classifiers);
	}

	return 1;
}



// create a triple classifier out of the specified row of the matrix
class_triple_classifier class_BoostMap::classifier_from_matrix(vMatrix<float> classifiers_matrix, 
	vint8 number)
{
	matrix_pointer(float) classifiers = classifiers_matrix.Matrix2();
	vint8 type = round_number(classifiers[number][0]);
	vint8 object1 = round_number(classifiers[number][1]);
	vint8 object2 = round_number(classifiers[number][2]);
	float weight = classifiers[number][3];
	float z = classifiers[number][4];

	class_triple_classifier classifier;
	if (type == 0)
	{
		classifier = class_triple_classifier((long) object1, weight, z);
	}
	else if (type == 1)
	{
		classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
	}
	else
	{
		exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
	}

	return classifier;
}



// create sensitive classifiers out of the entries of a matrix.
vint8 class_BoostMap::MatrixToSclassifiers(vMatrix<float> classifier_matrix, 
	vector<class_sensitive_classifier> * sensitive_classifiers)
{
	vint8 number = classifier_matrix.Rows();
	if ((number == 0) || (classifier_matrix.valid() <= 0))
	{
		return 0;
	}

	vArray2(float) classifiers = classifier_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		class_sensitive_classifier classifier = MatrixToSensitive(classifier_matrix, i);

		sensitive_classifiers->push_back(classifier);
	}

	return 1;
}



vint8 class_BoostMap::ComputeAlphaLimits(vMatrix<float> resultm, 
	float * alpha_min, float * alpha_max)
{
	const float max_exponent = 88.0;
	vint8 number = resultm.Size();

	float max_diff = 0;
	float min_diff = 0;

	vint8 j;
	for (j = 0; j < number; j++)
	{
		float diff = resultm(j);
		if (diff > max_diff)
		{
			max_diff = diff;
		}
		if (diff < min_diff)
		{
			min_diff = diff;
		}
	}

	float max_max = Max(vAbs(max_diff), vAbs(min_diff));
	float adjustment = log(max_max * training_triple_number);
	if (max_diff > 0)
	{
		*alpha_min = (max_exponent - adjustment) / -max_diff;
	}
	else
	{
		*alpha_min = vTypeMin((float) 0);
	}
	if (min_diff < 0)
	{
		*alpha_max = (max_exponent - adjustment) / -min_diff;
	}
	else
	{
		*alpha_max = vTypeMax((float) 0);
	}

	return 1;
}


// adds the specified reference object, with given weight
vint8 class_BoostMap::AddReferenceObject(vint8 index, float weight)
{
	if ((index < 0) || (index >= candidate_number))
	{
		vPrint("invalid index %li\n", (long) index);
		return 0;
	}

	class_triple_classifier classifier((long) index, weight, 0);
	AddClassifier(classifier);
	return 1;
}


// adds the specified range of reference objects, with
// given weight.
vint8 class_BoostMap::AddRange(vint8 start_index, vint8 end_index, float weight)
{
	if (start_index < 0)
	{
		vPrint("setting start index from %li to 0\n", (long) start_index);
		start_index = 0;
	}
	if (end_index >= candidate_number)
	{
		vPrint("setting end_index %li to %li\n", (long) end_index, (long) candidate_number - 1);
		end_index = candidate_number - 1;
	}

	vint8 i;
	for (i = start_index; i <= end_index; i++)
	{
		AddReferenceObject(i, weight);
	}

	return 1;
}


// Here we don't pass a weight as an argument. Instead, the
// optimal weight is computed.
vint8 class_BoostMap::AddWeightedReference(vint8 index)
{
	if ((index < 0) || (index >= candidate_number))
	{
		vPrint("invalid index %li\n", (long) index);
		return 0;
	}

	float weight = 0;
	class_triple_classifier classifier((long) index, 0, 0);
	float z = ClassifierZ(&classifier, &weight);
	classifier.weight = weight;
	classifier.z = z;

	AddClassifier(classifier);
	return 1;
}


vint8 class_BoostMap::AddWeightedRange(vint8 start_index, vint8 end_index)
{
	if (start_index < 0)
	{
		vPrint("setting start index from %li to 0\n", (long) start_index);
		start_index = 0;
	}
	if (end_index >= candidate_number)
	{
		vPrint("setting end_index %li to %li\n", (long) end_index, (long) (candidate_number - 1));
		end_index = candidate_number - 1;
	}

	vint8 i;
	for (i = start_index; i <= end_index; i++)
	{
		AddWeightedReference(i);
	}

	return 1;
}


// adds random references (useful for testing
// the accuracy of choosing reference objects randomly
// and with the same weight).
vint8 class_BoostMap::RandomReferences2(vint8 number, float weight)
{
	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 index = function_random_vint8(0, candidate_number - 1);
		class_triple_classifier classifier((long) index, weight, 0);
		AddClassifier(classifier);
	}

	return 1;
}


// adds random unweighted references (useful for testing
// the accuracy of choosing reference objects randomly
// and with the same weight).
vint8 class_BoostMap::RandomProjections2(vint8 number, float weight)
{
	if (candidate_number < 2)
	{
		exit_error("error: in RandomProjections2, candidate_number = %li\n",
			(long) candidate_number);
	}

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 index1 = function_random_vint8(0, candidate_number - 1);
		vint8 index2 = function_random_vint8(0, candidate_number - 1);
		if (index1 == index2)
		{
			i--;
			continue;
		}
		class_triple_classifier classifier((long) index1, (long) index2, weight, 0);
		AddClassifier(classifier);
	}

	return 1;
}


// This function is useful if we want to use arbitrary weak
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
vint8 class_BoostMap::SetClassifiers(vMatrix<float> training_raw_results, 
	vMatrix<float> validation_raw_results,
	vint8_matrix classifier_ids,
	float in_max_entry)
{
	if ((in_max_entry != distance_max_entry) && (distance_max_entry > 0))
	{
		vPrint("error: inconsistent max entries: from %f to %f\n",
			distance_max_entry, in_max_entry);
		return 0;
	}
	if (in_max_entry <= 0)
	{
		vPrint("error: bad max entry: %f\n", in_max_entry);
		return 0;
	}

	distance_max_entry = in_max_entry;
	// first, we need to convert raw results to classification results.
	ConvertRawResults(training_raw_results, training_distances);
	ConvertRawResults(validation_raw_results, validation_distances);

	vint8 i;
	vArray(float) data = training_raw_results.Matrix();
	vint8 size = training_raw_results.Size();
	for (i = 0; i < size; i++)
	{
		data[i] /= distance_max_entry;
	}

	data = validation_raw_results.Matrix();
	size = validation_raw_results.Size();
	for (i = 0; i < size; i++)
	{
		data[i] /= distance_max_entry;
	}

	m_training_results = training_raw_results;
	m_validation_results = validation_raw_results;
	m_classifier_ids = classifier_ids;

	// make sure we don't allow any other type of weak classifiers, because
	// in principle the user can still try to do regular training.
	allow_lipschitz = 0;
	allow_projections = 0;
	allow_sensitive = 0;
	new_sensitive = 0;

	return 1;
}


vint8 class_BoostMap::ClearClassifierResults()
{
	m_training_results = vMatrix<float>();
	m_validation_results = vMatrix<float>();
	m_classifier_ids = vint8_matrix();

	return 1;
}


vint8 class_BoostMap::NextSteps2(vint8 steps)
{
	vint8 i;
	for (i = 0; i < steps; i++)
	{
		NextStep2();
	}

	return 1;
}


float class_BoostMap::NextStep2()
{
	if ((is_valid <= 0) || (m_training_results.valid() <= 0) ||
		(m_validation_results.valid() <= 0) || (m_classifier_ids.valid() <= 0))
	{
		return 0;
	}

	class_triple_classifier new_classifier;
	vint8 success = 0;

	// first, check if it is beneficial to remove an already chosen
	// classifier.
	if (allow_removals != 0)
	{
		// get "best" classifiers to remove, for each classifier type.
		class_triple_classifier classifier_remove = ClassifierRemoval(m_training_results, m_classifier_ids,
			&success);

		// if no good classifier to remove was found, we don't remove
		// any.
		if (success > 0) 
		{
			new_classifier = classifier_remove;
			vPrint("removing: ");
			new_classifier.Print();
		}
		// if no classifier is getting removed, check whether we 
		// can find a classifier for which it would be beneficial
		// to update the weight.
		else
		{
			// get most beneficial weight changes for each type.
			class_triple_classifier classifier_changed = ClassifierChange(m_training_results, m_classifier_ids, 
				&success);
			if (success > 0)
			{
				new_classifier = classifier_changed;
				vPrint("modifying: last_new_z = %f, cutoff = %f\n", 
					last_new_z, ChangeCutoff());
				new_classifier.Print();
			}
		}
	}

	// if we neither remove classifiers nor modify weights, then
	// we will look to find the best weak classifier to add to 
	// existing classifiers.
	if (success == 0)
	{
		// find the best classifier for each type.
		new_classifier = NextClassifier(m_training_results, m_classifier_ids);
		last_new_z = new_classifier.z;
	}

	// Add the new weak classifier, which, depending on
	// what happened earlier in the function, could lead
	// to removal of a classifier, weight update, or 
	// adding in a new classifier.
	float result = AddClassifier4(new_classifier, m_training_results, 
		m_validation_results, m_classifier_ids);
	return result;
}


// converts raw results to classification accuracy results. raw
// results are simply the outputs of the weak classifier. 
// classification accuracy results are positive when the raw results 
// are correct (same sign as true results), and negative otherwise. 
// The input argument itself
// is modified, so that we avoid creating a new matrix that can 
// be potentially very large.
vint8 class_BoostMap::ConvertRawResults(vMatrix<float> raw_results, 
	vMatrix<float> true_results)
{
	if (raw_results.Cols() != true_results.Size())
	{
		exit_error("\nerror: in ConvertRawResults, incompatible sizes %li %li\n",
			(long) raw_results.Cols(), (long) true_results.Size());
	}

	//  float max_entry = 0;

	vint8 rows = raw_results.Rows();
	vint8 cols = raw_results.Cols();
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			float result = raw_results(row, col);
			float true_result = true_results(0, col);
			//      max_entry = Max(max_entry, vAbs(result));

			if ((result > 0) && (true_result > 0))
			{
				raw_results(row, col) = vAbs(result);
			}
			else if ((result < 0) && (true_result < 0))
			{
				raw_results(row, col) = vAbs(result);
			}
			else if ((result != 0) && (true_result != 0))
			{
				raw_results(row, col) = 0 - vAbs(result);
			}
			else
			{
				raw_results(row, col) = 0;
			}
		}
	}

	return 1;
}


class_triple_classifier class_BoostMap::ClassifierRemoval(vMatrix<float> training_results,
	vint8_matrix classifier_ids,
	vint8 * success)
{
	vint8 steps = unique_classifiers.size();
	*success = 0;
	if (steps == 0) 
	{
		return class_triple_classifier();
	}

	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_index = 0;
	vint8 number_of_ids = classifier_ids.Size();
	vint8 i;
	for (i = 0; i < steps; i++)
	{
		if (unique_classifiers[(vector_size) i].type != 0)
		{
			continue;
		}

		// first, check if the classifier id is in classifier_ids, because
		// otherwise we cannot evaluate it, and hence we cannot remove it.
		vint8 classifier_id = unique_classifiers[(vector_size) i].object1;
		vector<float> results;
		vint8 row = ClassifierResults4(classifier_id, training_results, 
			&results, classifier_ids);
		if (row < 0)
		{
			continue;
		}

		// get the Z corresponding to removing this classifier.
		float weight = unique_classifiers[(vector_size) i].weight;    
		float z = (float) Z(-weight, &results);

		// check if this is the best Z seen so far (among candidate
		// removals).
		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = -weight;
			min_z_index = classifier_id;
		}
	}

	// if min_z is less than 1, then removing the corresponding classifier
	// is beneficial, and should improve the training error.
	if (min_z < 1)
	{
		*success = 1;
	}

	class_triple_classifier result((long) min_z_index, min_z_alpha, min_z);
	return result;
}


class_triple_classifier class_BoostMap::ClassifierChange(vMatrix<float> training_results,
	vint8_matrix classifier_ids,
	vint8 * success)
{
	*success = 0;

	vPrint("Trying weight change for classifiers:\n");
	vint8 steps = unique_classifiers.size();
	if (steps == 0) 
	{
		return class_triple_classifier();
	}

	vint8 number_of_ids = classifier_ids.Size();
	float min_z = (float) 10, min_z_alpha = (float) 0;
	vint8 min_z_index = 0;
	vint8 i;
	vPrint("\n");
	for (i = 0; i < steps; i++)
	{
		// this function only looks at Lipschitz-type embeddings.
		if (unique_classifiers[(vector_size) i].type != 0)
		{
			continue;
		}

		// first, check if the classifier id is in classifier_ids, because
		// otherwise we cannot evaluate it, and hence we cannot remove it.
		vint8 classifier_id = unique_classifiers[(vector_size) i].object1;
		vector<float> results;
		vint8 row = ClassifierResults4(classifier_id, training_results, 
			&results, classifier_ids);
		if (row < 0)
		{
			continue;
		}


		// get the Z corresponding to removing this classifier.
		float weight = unique_classifiers[(vector_size) i].weight;    
		float z = 0, alpha = 0;
		MinimizeZ(&results, &z, &alpha);

		// verify that, if we use this weight, the total sum
		// will be non-negative.
		if (((weight + alpha) < 0) && (allow_negative == 0))
		{
			continue;
		}

		if (z < min_z)
		{
			min_z = z;
			min_z_alpha = alpha;
			min_z_index = classifier_id;
		}
		vPrint("evaluated classifier %li of %li\r",
			(long) (i+1), (long) steps);
	}
	vPrint("\n");

	// although, in theory, we should check if min_z is less than 1, we 
	// want to avoid weight updates that are numerically insignificant,
	// so I  use ChangeCutoff as an empirically determined arbitrary threshold.
	float z_cutoff = ChangeCutoff();
	if (min_z <= z_cutoff) 
	{
		*success = 1;
	}

	class_triple_classifier result((long) min_z_index, min_z_alpha, min_z);
	return result;
}


class_triple_classifier class_BoostMap::NextClassifier(vMatrix<float> training_results,
	vint8_matrix classifier_ids)
{
	if (is_valid <= 0) 
	{
		return class_triple_classifier();
	}

	float z_min = (float) 10;
	vint8 z_min_index = -1;
	float best_alpha = (float) 0;
	if (number_of_picked_candidates == 0) 
	{
		vPrint("no candidate classifiers are available\n");
		return class_triple_classifier();
	}

	candidate_number = classifier_ids.Size();
	// choose candidate reference objects.
	ChooseCandidates(number_of_picked_candidates);

	// initialize alpha and z_min.
	float alpha = 0;
	vector<float> first_results;
	ClassifierResults3(candidates[0], training_results, 
		&first_results);

	MinimizeZ(&first_results, &z_min, &best_alpha);
	z_min_index = classifier_ids(candidates[0]);
	vint8 i;
	vPrint("\n");

	// go through all candidates.
	for (i = 1; i < candidates_matrix.Size(); i++)
	{
		// get the z for current candidate, check if 
		// it is the best (smallest) seen so far.
		vector<float> results;
		ClassifierResults3(candidates[i], training_results, 
			&results);
		float z = 0;
		MinimizeZ(&results, &z, &alpha);
		if (z < z_min)
		{
			z_min = z;
			z_min_index = classifier_ids(candidates[i]);
			best_alpha = alpha;
		}
		vPrint("evaluated classifier %li of %li\n",
			(long) (i+1), (long) candidates_matrix.Size());
	}
	vPrint("\n");
	class_triple_classifier result((long) z_min_index, best_alpha, z_min);
	return result;
}


float class_BoostMap::AddClassifier4(class_triple_classifier classifier, 
	vMatrix<float> training_results,
	vMatrix<float> validation_results,
	vint8_matrix classifier_ids)
{
	vint8 classifier_id = classifier.object1;

	// compute training error and perror for classifier.
	vector<float> training_result_vector;
	ClassifierResults4(classifier_id, training_results, 
		&training_result_vector, classifier_ids);
	ComputeLastError(classifier.weight, &training_result_vector);

	vector<float> training_presult_vector;
	ClassifierResults4(classifier_id, training_results, 
		&training_presult_vector, classifier_ids);
	ResultsToPresults(&training_presult_vector, training_distances,
		training_pdistances);
	ComputeLastPerror(classifier.weight, &training_presult_vector);

	// compute classifier results
	vector<float> ground_truth_vector;
	ClassifierResults4(classifier_id, training_results, 
		&ground_truth_vector, classifier_ids);

	// This is a hack, kind of. Here we are not necessarily converting
	// to presults. If we do parameter space embeddings, then
	// ground_truth = training_pdistances, so we are
	// indeed converting to Presuts. Otherwise, we just leave the 
	// results unchanged. I call it a hack because just by looking
	// at the name of the function we call one would think that
	// we always convert to presults.
	ResultsToPresults(&ground_truth_vector, training_distances,
		ground_truth);
	// add classifier to detailed classifiers.
	classifiers.push_back(classifier);

	// compute z for the classifier.
	float z = (float) Z(classifier.weight, &ground_truth_vector);

	// update weights for training triples, based on z and the weight
	// of the classifier.
	UpdateWeights(&ground_truth_vector, z, classifier.weight);

	// add classifier to unique_classifiers.
	clean_up_classifier(classifier);

	// Compute training and validation error and perror
	float margin = ComputeTrainingError(&training_result_vector);
	ComputeTrainingPerror(&training_presult_vector);

	vector<float> validation_result_vector;
	ClassifierResults4(classifier_id, validation_results, 
		&validation_result_vector, classifier_ids);
	ComputeValidationError(&validation_result_vector);

	vector<float> validation_presult_vector;
	ClassifierResults4(classifier_id, validation_results, 
		&validation_presult_vector, classifier_ids);
	ResultsToPresults(&validation_presult_vector, validation_distances,
		validation_pdistances);
	ComputeValidationPerror(&validation_presult_vector);

	// store training and validation error and perror of strong classifier,
	// for this step.
	training_errors.push_back(training_error);
	validation_errors.push_back(validation_error);
	training_perrors.push_back(training_perror);
	validation_perrors.push_back(validation_perror);

	return margin;
}


float class_BoostMap::AddClassifier4b(vint8 id, float weight)
{
	class_triple_classifier classifier((long) id, weight, -1);
	return AddClassifier4(classifier, m_training_results, m_validation_results,
		m_classifier_ids);
}



vint8 class_BoostMap::ClassifierResults3(vint8 row, vMatrix<float> all_results,
	vector<float> * results)
{
	if ((row < 0) || (row >= all_results.Rows()))
	{
		exit_error("\nerror: in ClassifierResults3, incompatible rows: %li %li\n",
			(long) row, (long) all_results.Rows());
	}

	v3dMatrix<float> results_matrix = copy_horizontal_line(&all_results, row);
	vector_from_matrix(&results_matrix, results);

	return 1;
}


vint8 class_BoostMap::ClassifierResults4(vint8 classifier_id, vMatrix<float> all_results,
	vector<float> * results, vint8_matrix classifier_ids)
{
	vint8 row = FindClassifierRow(classifier_id, classifier_ids);
	if (row >= 0)
	{
		v3dMatrix<float> results_matrix = copy_horizontal_line(&all_results, row);
		vector_from_matrix(&results_matrix, results);
	}
	return row;
}


vint8 class_BoostMap::FindClassifierRow(vint8 classifier_id, vint8_matrix classifier_ids)
{
	vint8 number_of_ids = classifier_ids.Size();
	vint8 row = -1;
	vint8 j;
	for (j = 0; j < number_of_ids; j++)
	{
		if (classifier_id == classifier_ids(j))
		{
			row = j;
			break;
		}
	}

	return row;
}


// This function modifies results, so that they don't reflect classification
// accuracy with respect to proximity, but with respect to class labels. 
// The conversion is simple: if distances and pdistances have a different 
// sign, then we just negate the results.
vint8 class_BoostMap::ResultsToPresults(vector<float> * results, vMatrix<float> distances,
	vMatrix<float> pdistances)
{
	vint8 size1 = results->size();
	vint8 size2 = distances.Size();
	vint8 size3 = pdistances.Size();
	if ((size1 != size2) || (size1 != size3))
	{
		exit_error("\nerror: in ResultstoPresults, bad sizes: %li %li %li\n",
			(long) size1, (long) size2, (long) size3);
	}

	vint8 i;
	for (i = 0; i < size1; i++)
	{
		if (distances(i) * pdistances(i) < 0)
		{
			(*results)[(vector_size) i] = -(*results)[(vector_size) i];
		}
		else if (distances(i) * pdistances(i) == 0)
		{
			(*results)[(vector_size) i] = 0;
		}
	}

	return 1;
}


// directory where SaveData saves the data.
char * class_BoostMap::DataDirectory()
{
	char * initial = data->training_directory();
	char * result = vMergeStrings2(initial, "_data");
	if (vDirectoryExists(result) == 0)
	{
		function_make_directory(result);
	}

	vdelete2(initial);

	return result;
}


char * class_BoostMap::DataDirectory1(const char * root_data_dir, const char * dataset)
{
	char * initial = BoostMap_data::training_directory(root_data_dir, dataset);
	char * result = vMergeStrings2(initial, "_data");
	vdelete2(initial);

	return result;
}


// adds DataDirectory to name.
char * class_BoostMap::DataPathname(const char * name)
{
	char * directory = DataDirectory();
	char * result = vJoinPaths(directory, name);
	vdelete2(directory);
	return result;
}


char * class_BoostMap::DataPathname2(const char * dataset, const char * name)
{
	char * directory = DataDirectory1(g_data_directory, dataset);
	char * result = vJoinPaths(directory, name);
	vdelete2(directory);
	return result;
}


// Saves data (i.e. triples and distances) that we can later load, 
// so that we can initialize
// a BoostMap object.
vint8 class_BoostMap::SaveData(const char * name)
{
	vint8 result = 1;
	char * pathname = DataPathname(name);
	FILE * fp = fopen(pathname, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", pathname);
		vdelete2(pathname);
		return 0;
	}

	vdelete2(pathname);
	vint8 success;

	success = training_triple_matrix.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save training triples\n");
		result = 0;
	}

	success = training_distances.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save training distances\n");
		result = 0;
	}

	success = training_pdistances.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save training pdistances\n");
		result = 0;
	}

	success = validation_triple_matrix.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save validation triples\n");
		result = 0;
	}

	success = validation_distances.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save validation distances\n");
		result = 0;
	}

	success = validation_pdistances.Write(fp);
	if (success <= 0)
	{
		vPrint("failed to save validation pdistances\n");
		result = 0;
	}

	fclose(fp);

	training_triple_matrix.Write("d:\\users\\athitsos\\training.bin");
	validation_triple_matrix.Write("d:\\users\\athitsos\\validation.bin");
	return result;
}



char * class_BoostMap::MaxEntryPath(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last)
{
	char * classifier_path = vTrainingClassifiersPath(training_dataset, triples_file,
		first, last);
	char * max_entry_path = vMergeStrings2(classifier_path, ".max");
	vdelete2(classifier_path);
	return max_entry_path;
}


vint8 class_BoostMap::RecordMaxEntry(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last)
{
	char * classifier_path = vTrainingClassifiersPath(training_dataset, triples_file,
		first, last);
	char * output_path = MaxEntryPath(training_dataset, triples_file, first, last);
	const vint8 block_size = 1000000;
	vMatrix<float> bufferm(1, block_size);
	vArray(float) buffer = bufferm.Matrix();
	float max_entry = 0;
	FILE * fp = fopen(classifier_path, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", classifier_path);
		vdelete2(classifier_path);
		vdelete2(output_path);
		return 0;
	}

	vint8 times = 0;

	vPrint("\n");
	while(1)
	{
		vint8 items = fread(vData(buffer), sizeof(float), block_size,
			fp);
		if (items <= 0)
		{
			break;
		}

		vint8 i;
		for (i = 0; i < items; i++)
		{
			float entry = vAbs(buffer[i]);
			if (entry > max_entry)
			{
				max_entry = entry;
			}
		}

		vPrint("processed chunk %li\r", (long) times);
		times++;
	}

	vPrint("\n");
	fclose(fp);
	vdelete2(classifier_path);
	vPrint("max_entry = %f\n", max_entry);

	fp = fopen(output_path, vFOPEN_WRITE);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", output_path);
		vdelete2(output_path);
		return 0;
	}
	fprintf(fp, "%f", max_entry);
	fclose(fp);
	vdelete2(output_path);

	return 1;
}


float class_BoostMap::ReadMaxEntry(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last)
{
	char * pathname = MaxEntryPath(training_dataset, triples_file, first, last);
	FILE * fp = fopen(pathname, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("\nfailed to open %s\n", pathname);
		vdelete2(pathname);
		return -1;
	}

	float max_entry = 0;
	vint8 items = fscanf(fp, "%f", &max_entry);
	if (items != 1)
	{
		max_entry = -1;
		vPrint("\nfailed to read max entry from %s\n", pathname);
	}

	fclose(fp);
	vdelete2(pathname);
	return max_entry;
}


const char * class_BoostMap::DatasetName()
{
	if (data == 0)
	{
		return 0;
	}
	return data->get_training_name();
}


const char * class_BoostMap::TriplesFile()
{
	return triples_file;
}


vint8_matrix class_BoostMap::ReadTrainingTriples()
{
	if (triples_file == 0)
	{
		vPrint("no triples file available\n");
		return vint8_matrix();
	}

	char * pathname = DataPathname(triples_file);
	FILE * fp = fopen(pathname, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", pathname);
		vdelete2(pathname);
		return vint8_matrix();
	}

	vdelete2(pathname);

	vint8_matrix temp;
	vMatrix<float> tempf;

	// read training triples
	temp = vint8_matrix::Read(fp);
	if (temp.valid() <= 0)
	{
		vPrint("failed to load training triples\n");
		fclose(fp);
		return vint8_matrix();
	}
	return temp;
}


vint8_matrix class_BoostMap::ReadValidationTriples()
{
	if (triples_file == 0)
	{
		vPrint("no triples file available\n");
		return vint8_matrix();
	}

	char * pathname = DataPathname(triples_file);
	FILE * fp = fopen(pathname, vFOPEN_READ);
	if (fp == 0)
	{
		vPrint("failed to open %s\n", pathname);
		vdelete2(pathname);
		return vint8_matrix();
	}

	vdelete2(pathname);

	vint8_matrix temp;
	vMatrix<float> tempf;

	// read training triples
	temp = vint8_matrix::Read(fp);
	if (temp.valid() <= 0)
	{
		vPrint("failed to load training triples\n");
		fclose(fp);
		return vint8_matrix();
	}

	// read training distances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load training distances\n");
		fclose(fp);
		return vint8_matrix();
	}

	// read training pdistances
	tempf = vMatrix<float>::Read(fp);
	if (tempf.valid() <= 0)
	{
		vPrint("failed to load training pdistances\n");
		fclose(fp);
		return vint8_matrix();
	}

	// read validation triples
	vint8_matrix result = vint8_matrix::Read(fp);
	return result;
}


vint8 class_BoostMap::TrainingTripleNumber()
{
	return training_triple_number;
}


vint8 class_BoostMap::ValidationTripleNumber()
{
	return validation_triple_number;
}


vint8 class_BoostMap::TripleResult(vint8 triple, vint8 classifier)
{
	if (triple < 0)
	{
		return 0;
	}

	vint8 row = FindClassifierRow(classifier, m_classifier_ids);
	if (row < 0)
	{
		vPrint("no classifier found\n");
		return 0;
	}

	if (triple < training_triple_number)
	{
		vPrint("training triple info:\n");

		vint8 q = training_triples[triple][0];
		vint8 a = training_triples[triple][1];
		vint8 b = training_triples[triple][2];

		vint8 qid = training_ids[q];
		vint8 aid = training_ids[a];
		vint8 bid = training_ids[b];

		float distance = ground_truth(triple);
		vector<float> results;
		ClassifierResults4(classifier, m_training_results, &results, m_classifier_ids);
		vPrint("(%li, %li, %li): dist = %f, result = %f, %f, %f\n",
			(long) q, (long) a, (long) b, distance, 
			m_training_results(row, triple), results[(vector_size) triple],
			distance_max_entry * results[(vector_size) triple]);
		vPrint("ids: (%li, %li, %li)\n\n", (long) qid, (long) aid, (long) bid);
	}

	if (triple < validation_triple_number)
	{
		vPrint("validation triple info:\n");

		vint8 q = validation_triples[triple][0];
		vint8 a = validation_triples[triple][1];
		vint8 b = validation_triples[triple][2];

		vint8 qid = validation_ids[q];
		vint8 aid = validation_ids[a];
		vint8 bid = validation_ids[b];

		float distance = validation_distances(triple);
		vector<float> results;
		ClassifierResults4(classifier, m_validation_results, &results, m_classifier_ids);
		vPrint("(%li, %li, %li): dist = %f, result = %f, %f, %f\n",
			(long) q, (long) a, (long) b, distance, 
			m_validation_results(row, triple), results[(vector_size) triple],
			distance_max_entry * results[(vector_size) triple]);
		vPrint("ids: (%li, %li, %li)\n\n", (long) qid, (long) aid, (long) bid);
	}

	return 1;
}


vint8 class_BoostMap::load_classifier_c(const char * filename, vint8 dimensions)
{
	char * path_name = Pathname(g_data_directory, filename);

	v3dMatrix<float> * temp_classifier_matrix = 
		v3dMatrix<float>::ReadText(path_name);
	if (temp_classifier_matrix == 0)
	{
		vPrint("Failed to load classifier from %s\n", path_name);
		vdelete2(path_name);
		return 0;
	}
	else
	{
		vPrint("Loaded classifier from %s\n", path_name);
	}

	vMatrix<float> classifier_matrix(temp_classifier_matrix);
	vdelete(temp_classifier_matrix);
	vint8 result = AddClassifierMatrix3(classifier_matrix, dimensions);
	vdelete2(path_name);
	return result;
}


vint8 class_BoostMap::AddClassifierMatrix3(vMatrix<float> classifier_matrix, 
	vint8 dimensions)
{
	vint8 number = classifier_matrix.Rows();
	vArray2(float) classifiers = classifier_matrix.Matrix2();

	vint8 i;
	for (i = 0; i < number; i++)
	{
		vint8 type = round_number(classifiers[i][0]);
		if (type != 0)
		{
			vPrint("error: type = %li in AddClassifierMatrix3\n", (long) type);
			return 0;
		}

		vint8 object1 = round_number(classifiers[i][1]);
		vint8 object2 = round_number(classifiers[i][2]);
		if (object2 != -1)
		{
			vPrint("error: object2 = %li in AddClassifierMatrix3\n", (long) object2);
			return 0;
		}

		vint8 row = FindClassifierRow(object1, m_classifier_ids);
		if (row < 0)
		{
			//      vPrint("error: row = %li in AddClassifierMatrix3\n", row);
			//      return 0;
			continue;
		}

		float weight = classifiers[i][3];
		float z = classifiers[i][4];

		class_triple_classifier classifier = class_triple_classifier((long) object1, weight, z);
		AddClassifier4(classifier, m_training_results,
			m_validation_results, m_classifier_ids);

		// Check if we exceeded the desired number of dimensions, in
		// which case we need to backtrack and return.
		if (dimensions >= 0)
		{
			vint8 current_dimensions = unique_classifiers.size();
			if (current_dimensions == dimensions + 1)
			{
				// backtrack
				classifier.weight = -classifier.weight;
				AddClassifier4(classifier, m_training_results,
					m_validation_results, m_classifier_ids);
				current_dimensions = unique_classifiers.size();
				if (current_dimensions != dimensions)
				{
					exit_error("Error: unsuccessful backtracking\n");
				}
				break;
			}
		}

		PrintSummary();
	}

	return 1;
}


// the next function is useful in the following case:
// embedding is an embedding that we have read from disk.
// classifier_name is the filename of the classifier corresponding to
// that embedding.  We want to choose the dimensions that correspond
// to the specified number of dimensions, and that is what we return.
// At the same time, we need to extract the matrix of classifiers
// that corresponds to the specified dimensions, and we store that into
// selected_classifiers_pointer.
float_matrix class_BoostMap:: choose_first_dimensions (const char * classifier_name, float_matrix embedding, 
	vint8 dimensions, float_matrix * selected_classifiers_pointer)
{
	float_matrix classifier_matrix = load_insensitive_classifier(classifier_name);
	if ((classifier_matrix.valid() <= 0) || (classifier_matrix.Cols() < 5))
	{
		return vMatrix<float>();
	}

	float_matrix unique_classifiers = get_unique_classifiers (classifier_matrix);
	float_matrix dimension_classifiers = select_first_dimensions (classifier_matrix, dimensions);

	vector <vint8> classifier_indices;
	vint8 horizontal = classifier_matrix.horizontal ();
	vector <vint8> horizontal_vector;
	function_insert_range (& horizontal_vector, 0, horizontal -1);
	find_classifier_indices(dimension_classifiers, unique_classifiers, & classifier_indices);

	float_image temporary_result = function_select_grid (&embedding, &classifier_indices, &horizontal_vector);
	float_matrix result (&temporary_result);
	return result;
}


// for each classifier described by a row of selected_classifiers,
// find its index in all_classifiers.  We assume that one and only one row
// is relevant, otherwise the program exits with an error message.
vint8 class_BoostMap::find_classifier_indices(float_matrix selected_classifiers, float_matrix all_classifiers, 
	vector <vint8> * classifier_indices)
{
	vint8 selected_vertical = selected_classifiers.vertical ();
	vint8 all_vertical = all_classifiers.vertical();

	vint8 i;
	for (i = 0; i < selected_vertical; i++)
	{
		// get a classifier corresponding to the i-th row of selected_classifiers.
		vint8 type = round_number(selected_classifiers(i,0));
		vint8 object1 = round_number(selected_classifiers(i,1));
		vint8 object2 = round_number(selected_classifiers(i,2));
		float weight = selected_classifiers(i,3);
		float z = selected_classifiers(i,4);

		class_triple_classifier classifier;
		if (type == 0)
		{
			classifier = class_triple_classifier((long) object1, weight, z);
		}
		else if (type == 1)
		{
			classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
		}
		else
		{
			exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
		}

		// find classifier in all_classifiers
		vint8 found_index = -1;
		vint8 counter = 0;

		for (counter = 0; counter < all_vertical; counter ++)
		{
			type = round_number(all_classifiers(counter,0));
			object1 = round_number(all_classifiers(counter,1));
			object2 = round_number(all_classifiers(counter,2));
			weight = all_classifiers(counter,3);
			z = all_classifiers(counter,4);
			class_triple_classifier second_classifier;
			if (type == 0)
			{
				second_classifier = class_triple_classifier((long) object1, weight, z);
			}
			else if (type == 1)
			{
				second_classifier = class_triple_classifier((long) object1, (long) object2, weight, z);
			}
			else
			{
				exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
			}
			if (check_classifiers(classifier, second_classifier) == 1)
			{
				if (found_index == -1)
				{
					found_index = counter;
				}
				else
				{
					exit_error ("error: multiple indices (%li, %li) for %li in find_classifier_indices\n",
						(long) found_index, (long) counter, (long) i);
				}
			}
		}

		if (found_index >= 0)
		{
			classifier_indices->push_back (found_index);
		}
		else
		{
			exit_error ("error: no indices found for %li in find_classifier_indices\n", (long) i);
		}
	}

	return 1;
}


// this function samples from the large set of training triples,
// so that we can efficiently choose the next weak classifier.
vint8 class_BoostMap::use_small_training()
{
	if (small_triple_number > large_triple_number)
	{
		small_triple_number = large_triple_number;
	}

	// first, find the triples that are classified and misclassified.
	vint8_matrix classified = classified_triples(large_training_margins);
	vint8_matrix misclassified = misclassified_triples(large_training_margins);
	double classified_factor = index_total(large_training_factors, classified);
	double misclassified_factor = index_total(large_training_factors, misclassified);

	vint8 misclassified_number = small_triple_number / 2;

	// make sure that classified_number and misclassified_number are 
	// not larger than the number of available triples.
	if (misclassified.length() < misclassified_number)
	{
		misclassified_number = misclassified.length();
	}
	vint8 classified_number = small_triple_number - misclassified_number;
	if (classified.length() < classified_number)
	{
		classified_number = classified.length();
		misclassified_number = small_triple_number - classified_number;
	}

	vint8_matrix chosen_misclassified1 = sample_without_replacement(0, misclassified.length() -1,
		misclassified_number);

	vint8_matrix chosen_misclassified(&chosen_misclassified1);

	vint8_matrix chosen_classified1 = sample_without_replacement(0, classified.length() -1,
		classified_number);

	vint8_matrix chosen_classified(&chosen_classified1);

	training_triple_matrix = vint8_matrix(small_triple_number, 3);
	training_factors_matrix = double_matrix(1, small_triple_number);
	training_margins_matrix = float_matrix(1, small_triple_number);
	training_pmargins_matrix = float_matrix(1, small_triple_number);
	training_distances = float_matrix(1, small_triple_number);
	training_pdistances = float_matrix(1, small_triple_number);
	ground_truth = float_matrix(1, small_triple_number);

	vint8 counter;
	vint8 index = 0;
	double classified_total = 0, misclassified_total = 0;
	// sample from the correctly classified triples
	for (counter = 0; counter < classified_number; counter++)
	{
		vint8 triple_index_first = chosen_classified(counter);
		vint8 triple_index = classified(triple_index_first);
		training_triple_matrix(index, 0) = large_training_triples(triple_index, 0);
		training_triple_matrix(index, 1) = large_training_triples(triple_index, 1);
		training_triple_matrix(index, 2) = large_training_triples(triple_index, 2);

		double factor = large_training_factors(triple_index);
		classified_total = classified_total + factor;
		training_factors_matrix(index) = factor;

		training_margins_matrix(index) = large_training_margins(triple_index);
		training_pmargins_matrix(index) = large_training_pmargins(triple_index);
		training_distances(index) = large_training_distances(triple_index);
		training_pdistances(index) = large_training_pdistances(triple_index);
		ground_truth(index) = large_ground_truth(triple_index);
		index++;
	}

	// sample from the incorrectly classified triples
	for (counter = 0; counter < misclassified_number; counter++)
	{
		vint8 triple_index_first = chosen_misclassified(counter);
		vint8 triple_index = misclassified(triple_index_first);
		training_triple_matrix(index, 0) = large_training_triples(triple_index, 0);
		training_triple_matrix(index, 1) = large_training_triples(triple_index, 1);
		training_triple_matrix(index, 2) = large_training_triples(triple_index, 2);

		double factor = large_training_factors(triple_index);
		misclassified_total = misclassified_total + factor;
		training_factors_matrix(index) = factor;

		training_margins_matrix(index) = large_training_margins(triple_index);
		training_pmargins_matrix(index) = large_training_pmargins(triple_index);
		training_distances(index) = large_training_distances(triple_index);
		training_pdistances(index) = large_training_pdistances(triple_index);
		ground_truth(index) = large_ground_truth(triple_index);
		index++;
	}

	double total_factors = classified_factor + misclassified_factor;
	if (total_factors > 0)
	{
		classified_factor = classified_factor / total_factors;
		misclassified_factor = misclassified_factor / total_factors;
	}

	// adjust training factors
	index = 0;
	double classified_multiplier = 0, misclassified_multiplier = 0;
	double check1 = 0, check2 = 0;
	if (classified_total > 0)
	{
		classified_multiplier = classified_factor / classified_total;
	}
	if (misclassified_total > 0)
	{
		misclassified_multiplier = misclassified_factor / misclassified_total;
	}

	for (counter = 0; counter < classified_number; counter++)
	{
		check1 += training_factors_matrix(index);
		training_factors_matrix(index) = training_factors_matrix(index) * classified_multiplier;
		check2 += training_factors_matrix(index);
		index++;
	}

	// sample from the incorrectly classified triples
	for (counter = 0; counter < misclassified_number; counter++)
	{
		training_factors_matrix(index) = training_factors_matrix(index) * misclassified_multiplier;
		index++;
	}  

	training_triple_number = small_triple_number;
	SetAllArrays();

	return 1;
}


// this function sets as training triples the large set of training triples,
// so that we can compute the weight of the next weak classifier.
vint8 class_BoostMap::use_large_training()
{
	training_factors_matrix = large_training_factors;
	training_triple_matrix = large_training_triples;
	training_margins_matrix = large_training_margins;
	training_pmargins_matrix = large_training_pmargins;
	training_distances = large_training_distances;
	training_pdistances = large_training_pdistances;
	ground_truth = large_ground_truth;

	training_triple_number = large_triple_number;

	SetAllArrays();
	return 1;
}


vint8_matrix class_BoostMap::classified_triples(float_matrix margins)
{
	vector<vint8> indices;
	vint8 number = margins.length();

	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		float margin = margins(counter);
		if (margin >= 0)
		{
			indices.push_back(counter);
		}
	}

	vint8_matrix result = matrix_from_vector(& indices);
	return result;
}


vint8_matrix class_BoostMap::misclassified_triples(float_matrix margins)
{
	vector<vint8> indices;
	vint8 number = margins.length();

	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		float margin = margins(counter);
		if (margin <= 0)
		{
			indices.push_back(counter);
		}
	}

	vint8_matrix result = matrix_from_vector(& indices);
	return result;
}


double class_BoostMap::index_total(double_matrix values, vint8_matrix indices)
{
	double result = 0;
	vint8 number = indices.length();

	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		vint8 index = indices(counter);
		double value = values(index);
		result = result + value;
	}

	return result;
}

// prints the sum of weights for the classified objects 
// and the misclassified objects
vint8 class_BoostMap::print_factor_totals()
{
	double classified_total = 0;
	double misclassified_total = 0;

	vint8 number = training_factors_matrix.length();
	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		float margin = training_margins_matrix(counter);
		if (margin < 0)
		{
			misclassified_total = misclassified_total + training_factors_matrix(counter);
		}
		else
		{
			classified_total = classified_total + training_factors_matrix(counter);
		}
	}

	function_print("\nclassified_total = %f, misclassified_total = %f\n",
		classified_total, misclassified_total);

	return 1;
}


vint8 class_BoostMap::set_small_triple_number(vint8 new_number)
{
	if (new_number <= 10)
	{
		return 0;
	}

	small_triple_number = new_number;
	return 1;
}


// for query-aware experiment.
// set cumulative sum of weights to factor for the specified label,
vint8 class_BoostMap::set_distribution(vint8 class_label, float factor)
{
	if ((factor <= 0) || (factor >= 1))
	{
		function_print("\nfactor should be in (0,1)\n");
		return 0;
	}

	use_large_training();
	vint8_matrix labels = data->training_training_labels();
	double label_total = 0;
	double other_total = 0;
	vint8 label_counter = 0;

	vint8 counter;
	for (counter = 0; counter < training_triple_number; counter++)
	{
		vint8 query = training_triple_matrix(counter, 0);
		vint8 label = labels(query);
		double factor = training_factors_matrix(counter);
		if (label != class_label) 
		{
			other_total = other_total + factor;
			continue;
		}

		label_counter ++;
		label_total = label_total + factor;
	}

	if ((label_counter == 0) || (label_total == 0))
	{
		function_print("\nlabel %li is not present\n", (long) class_label);
	}

	double other_factor = 1.0f - factor;
	double label_multiplier = factor / label_total;
	double other_multiplier = other_factor / other_total;

	for (counter = 0; counter < training_triple_number; counter++)
	{
		vint8 query = training_triple_matrix(counter, 0);
		vint8 label = labels(query);
		double factor = training_factors_matrix(counter);
		if (label != class_label) 
		{
			training_factors_matrix(counter) = factor * other_multiplier;
		}
		else
		{
			training_factors_matrix(counter) = factor * label_multiplier;
		}
	}

	return 1;
}


vint8 class_BoostMap::print_class_total(vint8 class_label)
{
	vint8_matrix labels = data->training_training_labels();
	double label_total = 0;
	vint8 label_counter = 0;

	vint8 counter;
	for (counter = 0; counter < training_triple_number; counter++)
	{
		vint8 query = training_triple_matrix(counter, 0);
		vint8 label = labels(query);
		double factor = training_factors_matrix(counter);
		if (label == class_label) 
		{
			label_counter++;
			label_total = label_total + factor;
		}
	}

	function_print("\nlabel = %li, counter = %li, total = %lf\n",
		(long) class_label, (long) label_counter, label_total);
	return 1;
}


// end of definition of class_BoostMap



char * vTrainingClassifiersPath(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last)
{
	char * directory = class_BoostMap::DataDirectory1(g_data_directory, training_dataset);
	char buffer[300];
	sprintf(buffer, "%s_tra_%li_%li.bin2", triples_file, (long) first, (long) last);

	char * result = vJoinPaths(directory, buffer);
	vdelete2(directory);
	return result;
}


char * vValidationClassifiersPath(const char * training_dataset, 
	const char * triples_file,
	vint8 first, vint8 last)
{
	char * directory = class_BoostMap::DataDirectory1(g_data_directory, training_dataset);
	char buffer[300];
	sprintf(buffer, "%s_val_%li_%li.bin2", triples_file, (long) first, (long) last);

	char * result = vJoinPaths(directory, buffer);
	vdelete2(directory);
	return result;
}

/*!
	[static] 
*/
vint8 class_BoostMap::print_retrieval_results(vint8_matrix result)
{
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	vint8 max_neighbors = (vint8) result.horizontal() - 1;
	vint8_image temp_ranks1 = copy_vertical_line(&result, 1);
	vint8_image temp_ranksk = copy_vertical_line(&result, max_neighbors);
	vint8_image temp_ranks10;
	if (max_neighbors > 10)
	{
		temp_ranks10 = copy_vertical_line(& result, 10);
	}

	vector<vint8> ranks1;
	vector<vint8> ranks10;
	vector<vint8> ranksk;
	vector_from_matrix(&temp_ranks1, &ranks1);
	vector_from_matrix(&temp_ranks10, &ranks10);
	vector_from_matrix(&temp_ranksk, &ranksk);

	std::sort(ranks1.begin(), ranks1.end(), less<vint8>());
	std::sort(ranks10.begin(), ranks10.end(), less<vint8>());
	std::sort(ranksk.begin(), ranksk.end(), less<vint8>());

	vint8 size = ranks1.size();
	float number = 100;
	float i;
	
	vPrint("max_neighbors: %5li\n", (long) max_neighbors);

	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		if (index == size)
		{
			index--;
		}
		vint8 entry1 = ranks1[(vector_size) index];
		vint8 entryk = ranksk[(vector_size) index];
		
		if (max_neighbors > 10)
		{
			vint8 entry10 = ranks10[(vector_size) index];
			vPrint("%5li: %6.2f, %8li %8li %8li\n", 
				(long) index, percentile, (long) entry1, (long) entry10, (long) entryk);
		}
		else
		{
			vPrint("%5li: %6.2f, %8li %8li\n", 
				(long) round_number(index), percentile, (long) entry1, (long) entryk);
		}
	}

	return 1;
}
