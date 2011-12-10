#pragma once

/*!
Class_embedding contains all the information we need in order to
compute the embedding of an object and measure the distance between 
the embeddings of two objects.
*/
class class_embedding : public class_unique
{
public:
	// true if we have successfully loaded all needed information
	vint8 valid; 

	// name of the data set for which this embedding has been defined.
	char * original_dataset_name;

	// name of file where the embedding has been loaded from
	char * embedding_name;

	// list of unique classifiers/1D embeddings that this embedding consists of
	vector<class_triple_classifier> unique_classifiers;

	// list (possibly empty, for example for query-insensitive embeddings)
	// of query-sensitive classifiers/1D embeddings that are part
	// of this embedding
	vector<class_sensitive_classifier> sensitive_classifiers;

	float_matrix pivot_distances;
	vint8 database_size;
	vint8 test_size;

protected:
	vint8 initialize();

public:
	class_embedding();

	/*!
	Create the embedding definition by reading from a filename
	where we saved the results of BoostMap training.
	We can specify the number of dimensions we want to load,
	but this argument will be ignored if there is a query-sensitive classifier
	associated with this embedding. Also, if the dimensions are negative or zero
	we just load all dimensions.
	*/
	class_embedding(const char * in_original_name, const char * filename, vint8 dimensions);
	~class_embedding();

protected:
	virtual void delete_unique();

public:
	// returns the dimensionality of the embedding
	vint8 dimensionality() const;

	// quick and dirty way to provide an upper bound on the number of 
	// exact distance computations required to compute the embedding of an object.
	// I should write an exact function that computes the actual number
	// of exact distances, as opposed to an upper bound
	vint8 upper_bound_distances() const;

	// compute the embedding of an object. We can read the distances between the object
	// and all database objects from file_pointer
	float_matrix embed(class_file * file_pointer) const;

	// compute the embedding of an object. all_distances conveys the distances between 
	// the object and all database objects.
	float_matrix embed(const float_matrix all_distances) const;

	float_matrix factors_matrix() const;
	class_pointer(float) factors() const;

	// "name" is the name of an entire dataset. result(i) is 
	// the intrapivot distance for the i-th dimension.
	// this is a reimplementation of deprecated LoadPivotDistances
	float_matrix load_pivot_distances();

	vint8_matrix extract_types();
	vint8_matrix extract_references();
	vint8 print_information();
};
