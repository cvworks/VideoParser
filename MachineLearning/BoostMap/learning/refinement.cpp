
#include "refinement.h"
#include "basics/simple_algo_templates.h"
#include "basics/local_data.h"

#include "basics/definitions.h"



cascade_step::cascade_step(vint8 in_dimensions, vint8 in_refinement_length)
{
  dimensions = in_dimensions;
  refinement_length = in_refinement_length;
  classifier = 0;
  classifier_threshold = 0;
}


cascade_step::~cascade_step()
{
  function_delete(classifier);
}


vint8 cascade_step::get_dimensions() const
{
  return dimensions;
}


vint8 cascade_step::get_length() const
{
  return refinement_length;
}


// Note that here we implement the convention that if the classifier is zero
// then we just accept every query.
vint8 cascade_step::hard_classify(const float_matrix query) const
{
  if (classifier == 0)
  {
    return 1;
  }

  vint8 result = classifier->hard_classify(query, classifier_threshold);
  return result;
}


vint8 cascade_step::set_classifier(AdaBoost * new_classifier)
{
  if (classifier != 0)
  {
    exit_error("\nerror: in cascade_step::set_classifier, nonzero existing classifier\n");
  }

  classifier = new_classifier;
  return 1;
}


AdaBoost * cascade_step::get_classifier()
{
  return classifier;
}


// returns true iff the arguments match the dimensions and length variables
vint8 cascade_step::check_parameters(const vint8 input_dimensions, const vint8 input_length) const
{
  if ((dimensions != input_dimensions) || (refinement_length != input_length))
  {
    return 0;
  }
  else 
  {
    return 1;
  }
}


vint8 cascade_step::set_threshold(float new_threshold)
{
  classifier_threshold = new_threshold;
  return 1;
}


vint8 cascade_step::print()
{
  function_print("%5li dimensions, %7li distances, threshold = %f",
                 (long) dimensions, (long) refinement_length, classifier_threshold);

  return 1;
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////


refinement_cascade::refinement_cascade()
{
  last_dimensions = -1;
  last_length = -1;
}


refinement_cascade::refinement_cascade(const vint8 argument_last_dimensions, const vint8 argument_last_length)
{
  last_dimensions = argument_last_dimensions;
  last_length = argument_last_length;
}


refinement_cascade::~refinement_cascade()
{
  vint8 number = steps.size();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    function_delete(steps[(vector_size) counter]);
  }
}


// insert a retrieval step to the cascade
vint8 refinement_cascade::insert_step(cascade_step * step)
{
  steps.push_back(step);

  return 1;
}


// given the embedding of a query object, choose the step that is applicable
vint8 refinement_cascade::choose_step(float_matrix query, vint8 * dimensions_pointer, 
                                     vint8 * length_pointer) const
{
  vint8 number = steps.size();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    vint8 classification = steps[(vector_size) counter]->hard_classify(query);
    if (classification > 0)
    {
      *dimensions_pointer = steps[(vector_size) counter]->get_dimensions();
      *length_pointer = steps[(vector_size) counter]->get_length();
      return 1;
    }
  }

  if ((last_dimensions <= 0) || (last_length < 0))
  {
    exit_error("error: in choose_step, last dimensions = %li, last_length =%li\n",
               last_dimensions, last_length);
  }

  *dimensions_pointer = last_dimensions;
  *length_pointer = last_length;

  return 1;
}


cascade_step * refinement_cascade::find_step(vint8 dimensions, vint8 length)
{
  vint8 number = steps.size();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    if (steps[(vector_size) counter]->check_parameters(dimensions, length))
    {
      return steps[(vector_size) counter];
    }
  }

  return 0;
}

  
AdaBoost * refinement_cascade::find_classifier(vint8 dimensions, vint8 length)
{
  vint8 number = steps.size();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    if (steps[(vector_size) counter]->check_parameters(dimensions, length))
    {
      return steps[(vector_size) counter]->get_classifier();
    }
  }

  return 0;
}


vint8 refinement_cascade::set_last_step(const vint8 argument_last_dimensions, const vint8 argument_last_length)
{
  last_dimensions = argument_last_dimensions;
  last_length = argument_last_length;

  return 1;
}


vint8 refinement_cascade::get_max_dimensions() const
{
  vint8 result = last_dimensions;
  vint8 number = steps.size();

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    vint8 dimensions = steps[(vector_size) counter]->get_dimensions();
    if (dimensions > result)
    {
      result = dimensions;
    }
  }

  return result;
}


vint8 refinement_cascade::print() const
{
  vint8 number = steps.size();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    function_print("step %li: ", (long) counter);
    steps[(vector_size) counter]->print();
    function_print("\n");
  }

  function_print("last step: dimensions = %li, length = %li\n", last_dimensions, last_length);
  return 1;
}


////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////
////////////////////////////////////////////////////


class_refinement::class_refinement(const char* root_dir, char * in_original_name, char * in_training_name,
                                   char * in_embedding_file, vint8 in_max)
{
  initialize();
  embedding = class_embedding(in_original_name, in_embedding_file, 0);
  if (embedding.valid <= 0) 
  {
    return;
  }

  cascade.set_last_step(embedding.dimensionality(), embedding.database_size);

  data = new BoostMap_data(root_dir, in_training_name);
  if (data->valid() <= 0)
  {
    delete_pointer(data);
    data = 0;
    return;
  }

  original_dataset_name = function_copy_string(in_original_name);
  embedding_file = function_copy_string(in_embedding_file);
  max_neighbors = in_max;

  // load the embedding-based nearest neighbor retrieval results for the training set
  database_result = BoostMap_data::load_results_training(embedding, max_neighbors);
  if (database_result.valid() <= 0)
  {
    function_print("\nfailed to read retrieval results on training data set for %s %s\n",
                   original_dataset_name, embedding_file);
    return;
  }

  // load the embedding-based nearest neighbor retrieval results for the queries
  test_result = BoostMap_data::load_results_test(embedding, max_neighbors);
  if (test_result.valid() <= 0)
  {
    function_print("\nfailed to read retrieval results on test data set for %s %s\n",
                   original_dataset_name, embedding_file);
    return;
  }

  database_embedding = BoostMap_data::load_training_embedding(embedding.original_dataset_name,
                                                              embedding.embedding_name);
  if (database_embedding.valid() <= 0)
  {
    function_print("\nfailed to read database embedding for %s %s\n",
                   original_dataset_name, embedding_file);
    return;
  }

  test_embedding = BoostMap_data::load_test_embedding(embedding.original_dataset_name,
                                                                  embedding.embedding_name);
  if (test_embedding.valid() <= 0)
  {
    function_print("\nfailed to read test embedding for %s %s\n",
                   original_dataset_name, embedding_file);
    return;
  }
}


class_refinement::~class_refinement()
{
  function_delete(data);
  delete_pointer(original_dataset_name);
  delete_pointer(embedding_file);
}


vint8 class_refinement::initialize()
{
  original_dataset_name = 0;
  data = 0;
  embedding_file = 0;
  max_neighbors = -1;

  return 1;
}


vint8 class_refinement::valid()
{
  if (original_dataset_name == 0)
  {
    return 0;
  }
  else
  {
    return 1;
  }
}


AdaBoost * class_refinement::find_classifier(vint8 dimensions, vint8 length)
{
  return cascade.find_classifier(dimensions, length);
}


char * class_refinement::directory()
{
  char * result = function_join_paths_four(g_data_directory, "experiments", 
                                           "refinement", original_dataset_name);
  function_make_directory(result);

  return result;
}


// create a new step in the cascade.
vint8 class_refinement::create_classifier(vint8 dimensions, vint8 length, 
                                         float error_threshold, vint8 maximum_rounds)
{
  float_matrix training_vectors, test_vectors;
  vint8_matrix training_labels, test_labels;
  vint8 success = prepare_AdaBoost_input(dimensions, length, training_vectors, training_labels,
                                        test_vectors, test_labels);
  if (success <= 0)
  {
    function_print("\nfailed to prepare AdaBoost input\n");
    return 0;
  }

  cascade_step * step = new cascade_step(dimensions, length);
  AdaBoost * classifier = new AdaBoost(training_vectors, training_labels,
                                       test_vectors, test_labels);
  char * filename = classifier_filename(dimensions, length);
  classifier->train(filename, error_threshold, (long) maximum_rounds);
  delete_pointer(filename);
  step->set_classifier(classifier);
  cascade.insert_step(step);
  
  return 1;
}


// perform additional training rounds for the specified step in the cascade
vint8 class_refinement::train_more(vint8 dimensions, vint8 length, 
                                  float error_threshold, vint8 maximum_rounds)
{
  AdaBoost * classifier = find_classifier(dimensions, length);
  if (classifier == 0)
  {
    function_print("could not find classifier for %li dimensions, %li length\n",
                   dimensions, length);
    return 0;
  }

  char * filename = classifier_filename(dimensions, length);
  classifier->train(filename, error_threshold, (long) maximum_rounds);
  
  return 1;
}


char * class_refinement::classifier_filename(vint8 dimensions, vint8 length)
{
  char * dimensions_string = string_from_number(dimensions);
  char * length_string = string_from_number(length);
  char * result = function_merge_strings_five(embedding_file, "_", dimensions_string, "_", length_string);

  delete_pointer(dimensions_string);
  delete_pointer(length_string);

  return result;
}


vint8_matrix class_refinement::accuracy_labels(vint8_matrix retrieval_output, const vint8 length)
{
  return accuracy_labels(retrieval_output, length, max_neighbors);
}


vint8_matrix class_refinement::accuracy_labels(const vint8_matrix retrieval_output, const vint8 length, 
                                               const vint8 max_neighbors)
{
  if (retrieval_output.horizontal() <= max_neighbors)
  {
    exit_error("error: in accuracy_labels, %li columns in retrieval_output, max_neighbors = %li\n",
               retrieval_output.horizontal(), max_neighbors);
  }

  vint8 number = retrieval_output.vertical();
  vint8_matrix result(1, number);
  
  vint8 counter;
  

  for (counter = 0; counter < number; counter++)
  {
    vint8 current = retrieval_output(counter, max_neighbors);
    if (current < length)
    {
      result(counter) = 1;
    }
    else
    {
      result(counter) = -1;
    }
  }

  return result;
}

vint8 class_refinement::prepare_AdaBoost_input(vint8 dimensions, vint8 length, 
                                              float_matrix & training_vectors, vint8_matrix & training_labels,
                                              float_matrix & test_vectors, vint8_matrix & test_labels)
{
  vint8 training_size = database_embedding.vertical();
  vint8 test_size = test_embedding.vertical();
  if (dimensions == database_embedding.horizontal())
  {
    training_vectors = database_embedding;
  }
  else
  {
    training_vectors = float_matrix(training_size, dimensions);
    database_embedding.Copy(& training_vectors, 0, training_size - 1, 0, dimensions - 1, 0, 0);
  }
  if (dimensions == test_embedding.horizontal())
  {
    test_vectors = test_embedding;
  }
  else
  {
    test_vectors = float_matrix(test_size, dimensions);
    test_embedding.Copy(&test_embedding, 0, test_size - 1, 0, dimensions - 1, 0, 0);
  }

  training_labels = accuracy_labels(database_result, length);
  test_labels = accuracy_labels(test_result, length);

  //test_labels = vint8_matrix(1, test_size);
  //for (counter = 0; counter < test_size; counter++)
  //{
  //  vint8 result = test_result(counter, max_neighbors);
  //  if (result < length)
  //  {
  //    test_labels(counter) = 1;
  //  }
  //  else
  //  {
  //    test_labels(counter) = -1;
  //  }
  //}

  return 1;
}


vint8 class_refinement::retrieval_results(vint8 last_dimensions, vint8 last_length,
                                         float & distances, float & accuracy, vint8 & accuracy_counter)
{
  if (last_dimensions > embedding.dimensionality())
  {
    function_print("last_dimensions = %li, embedding dimensionality = %li\n",
                   last_dimensions, embedding.dimensionality());
    return 0;
  }

  cascade.set_last_step(last_dimensions, last_length);
  vint8 test_number = test_embedding.vertical();
  vint8 embedding_distances = embedding.upper_bound_distances();
  vint8 dimensions = 0, current_distances = 0;

  vint8 distances_counter = 0;
  accuracy_counter = 0;
  vint8 counter;
  for (counter = 0; counter < test_number; counter++)
  {
    float_matrix current_embedding = float_matrix(copy_horizontal_line(& test_embedding, counter));
    cascade.choose_step(current_embedding, & dimensions, & current_distances);
    distances_counter += current_distances;
    if (test_result(counter, max_neighbors) <= current_distances)
    {
      accuracy_counter++;
    }
  }

  distances = ((float) distances_counter) / (float) test_number;
  accuracy = ((float) accuracy_counter) / (float) test_number;

  return 1;
}


vint8 class_refinement::upper_bound_distances() const
{
  return embedding.upper_bound_distances();
}


vint8 class_refinement::set_step_threshold(vint8 dimensions, vint8 length, float new_threshold)
{
  cascade_step * step = cascade.find_step(dimensions, length);
  if (step == 0)
  {
    function_print("no step found for dimensions = %li, length = %li\n", dimensions, length);
    return 0;
  }

  vint8 result = step->set_threshold(new_threshold);
  return result;
}


vint8 class_refinement::print_cascade() const
{
  return cascade.print();
}
