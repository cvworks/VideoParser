#pragma once

/*!
Retrieval_statistics is an auxiliary class, where we store
temporary information about retrieval results, as we go through
a set of queries and we evaluate embedding-based nearest neighbor 
retrieval on that set. This class is used simply as a convenience,
so that we can pass a set of variables back-and-forth between
the functions that perform retrieval evaluation, such as retrieval_results.
*/
class retrieval_statistics
{
public:
	// k_max in the papers, i.e., the maximum number of nearest neighbors 
	// that we are interested in retrieving
	vint8 max_neighbors;

	// number of queries
	vint8 test_number;

	// number of objects in the database
	vint8 training_number;

	vint8_matrix result;

	// counts the number of objects in the test set for which we have performed 
	// retrieval so far
	vint8 object_counter;

	// number (seen so far) of triples (q, a, b) where a is one of the k_max-nearest
	// neighbors of q, and b is not one of the k_max-nearest neighbors of q.
	vint8 global_counter;

	// number of the above triples that is misclassified.
	vint8 global_error_counter;

	// fraction of the above triples that is misclassified.
	double global_error;

	// in rank_error_counters(i) we store (for the current query only)
	// the number of triples that is  misclassified, with the additional 
	// restriction that a is the i-th nearest neighbor of q.
	vint8_matrix rank_error_counters;

	// the result in rank_error_counters, in percentage form.
	float_matrix rank_errors;

	// in max_counters[i] we store the worst embedding-based rank
	// among all i-nearest neighbors of the current query.
	vint8_matrix max_counters;

	// worst rank of a 1-nearest neighbor, among all queries seen so far.
	vint8 worst_result_first;

	// worst rank of a k_max-nearest neighbor, among all queries seen so far.
	vint8 worst_result_last;

	// sum of ranks of 1-nearest neighbors, among all queries seen so far.
	vint8 ranks_total_first;

	// ranks_total_first decided by the number of queries processed so far
	float ranks_mean_first;

	// sum of ranks of k_max-nearest neighbors, among all queries seen so far.
	vint8 ranks_total_last;

	// ranks_total_last decided by the number of queries processed so far
	float ranks_mean_last;

protected:
	void initialize();

public:
	retrieval_statistics();
	retrieval_statistics(vint8 in_max_neighbors, vint8 in_test_number, vint8 in_training_number);
	~retrieval_statistics();

	vint8 error_averages();
	vint8 process_object_result();
};
