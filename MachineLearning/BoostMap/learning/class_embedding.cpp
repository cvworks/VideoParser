#include "boost_map.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"
#include "basics/definitions.h"
#include "class_sensitive_classifier.h"
#include "class_embedding.h"
#include "boost_kdd.h"

vint8 class_embedding::initialize()
{
	valid = 0;
	original_dataset_name = 0;
	embedding_name = 0;
	database_size = -1;
	test_size = -1;

	return 1;
}


class_embedding::class_embedding()
{
	initialize();
}

// create the embedding definition by reading from a filename
// where we saved the results of BoostMap training.
// We can specify the number of dimensions we want to load,
// but this argument will be ignored if there is a query-sensitive classifier
// associated with this embedding. Also, if the dimensions are negative or zero
// we just load all dimensions.
class_embedding::class_embedding(const char * in_original_name, 
	const char * filename, vint8 dimensions)
{
	initialize();
	original_dataset_name = function_copy_string(in_original_name);
	embedding_name = function_copy_string(filename);

	char * pathname;

	// first, check if there is a query-sensitive classifier available
	pathname = class_BoostMap::sensitive_pathname(filename);
	vMatrix<float> sensitive_matrix = vMatrix<float>::ReadText(pathname);
	if (sensitive_matrix.valid() <= 0)
	{
		vPrint("no query-sensitive classifier available at %s\n", pathname);
	}
	else // if there is a query-sensitive classifier, we ignore the dimensions argument.
	{
		function_print("ignoring any dimension arguments\n");
		dimensions = 0;
	}

	delete_pointer(pathname);

	pathname = class_BoostMap::Pathname(g_data_directory, filename);

	vMatrix<float> all_classifiers = vMatrix<float>::ReadText(pathname);

	if (all_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifiers from %s\n", pathname);
		vdelete2(pathname);
		return;
	}
	delete_pointer(pathname);

	float_matrix classifiers = 
		class_BoostMap::select_first_dimensions(all_classifiers, dimensions);

	if (classifiers.valid() <= 0)
	{
		function_print("failed to select first dimensions\n");
		return;
	}

	class_BoostMap::MatrixToTclassifiers(classifiers, &unique_classifiers);
	class_BoostMap::MatrixToSclassifiers(sensitive_matrix, &sensitive_classifiers);
	class_BoostMap::SetSensitiveIndices(&unique_classifiers, &sensitive_classifiers);

	// get pivot distances
	pivot_distances = load_pivot_distances();
	if (pivot_distances.valid() <= 0)
	{
		return;
	}

	// compute database size and test size
	database_size = BoostMap_data::database_size(original_dataset_name);
	test_size = BoostMap_data::test_size(original_dataset_name);
	if ((database_size <= -1) || (test_size <= -1))
	{
		return;
	}

	print_information();
	valid = 1;
}


class_embedding::~class_embedding()
{
	remove_reference();
}


void class_embedding::delete_unique()
{
	delete_pointer(original_dataset_name);
	delete_pointer(embedding_name);
}


// returns the dimensionality of the embedding
vint8 class_embedding::dimensionality() const
{
	return unique_classifiers.size();
}

// quick and dirty way to provide an upper bound on the number of 
// exact distance computations required to compute the embedding of an object.
// I should write an exact function that computes the actual number
// of exact distances, as opposed to an upper bound
vint8 class_embedding::upper_bound_distances() const
{
	vint8 result = 0;
	vint8 counter;
	for (counter = 0; counter < (vint8) unique_classifiers.size(); counter++)
	{
		switch (unique_classifiers[(vector_size) counter].type)
		{
		case 0:
			result = result + 1;
			break;

		case 1:
			result = result + 2;
			break;

		default:
			exit_error("error, type %li in unique classifier %li\n",
				(long) unique_classifiers[(vector_size) counter].type, (long) counter);
			break;
		}
	}

	return result;
}


// compute the embedding of an object. We can read the distances between the object
// and all database objects from file_pointer
float_matrix class_embedding::embed(class_file * file_pointer) const
{
	float_matrix  all_distances = BoostMap_data::NextObjectDistances(file_pointer, database_size);
	if (all_distances.valid() <= 0)
	{
		vPrint("Failed to read distances\n");
		return vMatrix<float>();
	}

	return embed(all_distances);
}


// compute the embedding of an object. all_distances conveys the distances between 
// the object and all database objects.
float_matrix class_embedding::embed(const float_matrix all_distances_matrix) const
{
	vint8 dimensions = dimensionality();

	vArray(float) all_distances = all_distances_matrix.Matrix();
	vArray(float) pivots = pivot_distances.Matrix();
	vMatrix<float> result_matrix(1, dimensions);
	vArray(float) result = result_matrix.Matrix();

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		const class_triple_classifier & dimension = unique_classifiers[(vector_size) counter];
		if (dimension.type == 0)
		{
			result[counter] = all_distances[dimension.database_first];
		}
		else if (dimension.type == 1)
		{
			float q_pivot1 = all_distances[dimension.database_first];
			float q_pivot2 = all_distances[dimension.database_second];
			float pivot_distance = pivots[counter];
			result[counter] = V_FastMap::LineProjection3(q_pivot1, q_pivot2, pivot_distance);
		}
		else
		{
			exit_error("Error in Embedding3: we shouldn't get here\n");
		}
	}

	return result_matrix;
}


float_matrix class_embedding::factors_matrix() const
{
	vint8 dimensions = dimensionality();
	float_matrix result(1, dimensions);

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		result(counter) = unique_classifiers[(vector_size) counter].weight;
	}

	return result;
}


class_pointer(float) class_embedding::factors() const
{
	vint8 dimensions = dimensionality();
	class_pointer(float) result = vnew(float, (vector_size) dimensions);

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		result[counter] = unique_classifiers[(vector_size) counter].weight;
	}

	return result;
}


// "name" is the name of an entire dataset. result(i) is 
// the intrapivot distance for the i-th dimension.
// this is a reimplementation of deprecated LoadPivotDistances
float_matrix class_embedding::load_pivot_distances()
{
	vint8 dimensions = dimensionality();
	float_matrix result(1, dimensions);

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		// we only need to worry about "line projection" embeddings.
		if (unique_classifiers[(vector_size) counter].type != 1)
		{
			result(counter) = (float) -1;
			continue;
		}

		// get the two pivots
		vint8 pivot1 = unique_classifiers[(vector_size) counter].database_first;
		vint8 pivot2 = unique_classifiers[(vector_size) counter].database_second;

		// get distances from pivot1 to entire database.
		vint8 success = 0;
		result(counter) = BoostMap_data::train_train_distance(original_dataset_name, pivot1, pivot2, & success);
		if (success <= 0)
		{
			vPrint("Failed to load distances for counter = %li, pivot = %li, name = %li\n",
				(long) counter, (long) pivot1, original_dataset_name);
			return float_matrix();
		}
	}

	return result;
}


vint8_matrix class_embedding::extract_types()
{
	vint8 dimensions = dimensionality();
	vint8_matrix result(1, dimensions);

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		result(counter) = unique_classifiers[(vector_size) counter].type;
	}
	return result;
}


vint8_matrix class_embedding::extract_references()
{
	vint8 dimensions = dimensionality();
	vint8_matrix result(1, 2*dimensions);

	vint8 counter;
	for (counter = 0; counter < dimensions; counter++)
	{
		result(2*counter) = unique_classifiers[(vector_size) counter].database_first;
		result(2*counter+1) = unique_classifiers[(vector_size) counter].database_second;
	}

	return result;
}


vint8 class_embedding::print_information()
{
	vint8_matrix references = extract_references();
	vint8_matrix types = extract_types();
	references.print_integer("references");
	types.print_integer("types");
	pivot_distances.print("pivot_distances");

	return 1;
}

