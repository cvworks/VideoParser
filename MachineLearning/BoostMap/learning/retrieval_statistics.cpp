#include "boost_map.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "basics/wrapper.h"
#include "basics/definitions.h"

#include "retrieval_statistics.h"

void retrieval_statistics::initialize()
{
	max_neighbors = test_number = training_number = -1;
	object_counter = 0;
	global_counter = global_error_counter = 0;
	global_error = 0.0;
	worst_result_first = worst_result_last = ranks_total_first = ranks_total_last = 0;

	rank_error_counters = vint8_matrix();
	rank_errors = float_matrix();
	max_counters = vint8_matrix();
}

retrieval_statistics::retrieval_statistics()
{
	initialize();
}


retrieval_statistics::retrieval_statistics(vint8 in_max_neighbors, vint8 in_test_number, 
	vint8 in_training_number)
{
	initialize();
	max_neighbors = in_max_neighbors;
	test_number = in_test_number;
	training_number = in_training_number;

	result = vint8_matrix(test_number, max_neighbors+1);
	rank_error_counters = vint8_matrix(1, max_neighbors);
	rank_errors = float_matrix(max_neighbors, 2);
	vint8 counter;
	for (counter = 0; counter < max_neighbors; counter++)
	{
		rank_errors(counter, 0) = (float) (counter + 1);
	}

	max_counters = vint8_matrix(1, max_neighbors);

	function_enter_value(&result, (vint8) 0);
	function_enter_value(&rank_error_counters, (vint8) 0);
	function_enter_value(&max_counters, (vint8) 0);
}


retrieval_statistics::~retrieval_statistics()
{
}


// can be called whenever, both when we perform retrieval for
// each particular object, or after we have done all retrievals
// and we want to get the global error rates.
vint8 retrieval_statistics::error_averages()
{
	vint8 counter;
	for (counter = 0; counter < max_neighbors; counter++)
	{
		rank_errors(counter, 1) = ((float) rank_error_counters(counter)) / (float) test_number;
	}

	global_error = ((double) global_error_counter) / (double) global_counter;

	return 1;
}


// this function must be called after we have obtained the retrieval
// results for each object in the test set, in order to update some variables
vint8 retrieval_statistics::process_object_result()
{
	object_counter++;
	worst_result_first = Max(worst_result_first, max_counters(0));    
	ranks_total_first = ranks_total_first + max_counters(0);
	ranks_mean_first = ((float) ranks_total_first) / (float) (object_counter);

	worst_result_last = Max(worst_result_last, max_counters(max_neighbors - 1));    
	ranks_total_last = ranks_total_last + max_counters(max_neighbors - 1);
	ranks_mean_last = ((float) ranks_total_last) / (float) (object_counter);

	result(object_counter - 1, 0) = object_counter - 1;

	vint8 counter;
	for (counter = 0; counter < max_neighbors; counter++)
	{
		result(object_counter - 1, counter + 1) = max_counters(counter);
	}

	return 1;
}
