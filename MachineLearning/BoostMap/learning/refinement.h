#ifndef VASSILIS_REFINEMENT_H
#define VASSILIS_REFINEMENT_H

#include "boosting.h"
#include "boost_kdd.h"


// cascade_step contains information about one step in the refinement cascade
class cascade_step
{
protected:
  // the classifier predicts if the query should be handled by this step 
  // in the cascade. We will use the convention that if the classifier is zero
  // then we just accept every query.
  AdaBoost * classifier;

  // number of dimensions we should use in this step in the cascade
  vint8 dimensions;

  // number of exact distances we should evaluate during refinement
  // in this step in the cascade
  vint8 refinement_length;

  // used to threshold the classification output of AdaBoost,
  // so as to make it more conservative.
  float classifier_threshold;

public:
  cascade_step(vint8 in_dimensions, vint8 in_refinement_length);
  ~cascade_step();

  vint8 get_dimensions() const;
  vint8 get_length() const;

  // Note that here we implement the convention that if the classifier is zero
  // then we just accept every query.
  vint8 hard_classify(const float_matrix query) const;
  vint8 set_classifier(AdaBoost * new_classifier);
  AdaBoost * get_classifier();
  
  // returns true iff the arguments match the dimensions and length variables
  vint8 check_parameters(const vint8 input_dimensions, const vint8 input_length) const;

  vint8 set_threshold(float new_threshold);
  vint8 print();
};


// class refinement_cascade will encapsulate a retrieval cascade
// that identifies, for each query, the degree of difficulty for that query
// and allocate resources accordingly by deciding which step in the cascade
// to use for retrieving matches to that query.
class refinement_cascade
{
protected:
  vector<cascade_step *> steps;
  vint8 last_dimensions;
  vint8 last_length;

public:
  refinement_cascade();
  refinement_cascade(const vint8 argument_last_dimensions, const vint8 argument_last_length);
  ~refinement_cascade();

  // insert a retrieval step to the cascade
  vint8 insert_step(cascade_step * step);

  // given the embedding of a query object, choose the step that is applicable
  vint8 choose_step(float_matrix query, vint8 * dimensions_pointer, vint8 * length_pointer) const;

  cascade_step * find_step(vint8 dimensions, vint8 length);
  AdaBoost * find_classifier(vint8 dimensions, vint8 length);
  vint8 set_last_step(const vint8 argument_last_dimensions, const vint8 argument_last_length);
  vint8 get_max_dimensions() const;
  vint8 print() const;
};

  
  
// class class_refinement encapsulates methods for learning
// and predicting which queries will be handled correctly
// by some embedding
class class_refinement
{
protected:
  class_embedding embedding;
  char * original_dataset_name;
  BoostMap_data * data;
  char * embedding_file;
  refinement_cascade cascade;

  // the number of neighbors that we want to retrieve.
  vint8 max_neighbors;

  // embeddings of the training set and the test set
  float_matrix database_embedding;
  float_matrix test_embedding;

  // retrieval results for training set and test set
  vint8_matrix database_result;
  vint8_matrix test_result;

  // directory where we save information about this refinement.
  char * directory();

public:
  class_refinement(const char* root_dir, 
	  char * in_original_name, char * in_training_name, 
                   char * in_embedding_file, vint8 in_max);
  ~class_refinement();

protected:
  vint8 initialize();

public:
  vint8 valid();

  AdaBoost * find_classifier(vint8 dimensions, vint8 length);

  // create a new step in the cascade.
  vint8 create_classifier(vint8 dimensions, vint8 length, 
                         float error_threshold, vint8 maximum_rounds);

  // perform additional training rounds for the specified step in the cascade
  vint8 train_more(vint8 dimensions, vint8 length, 
                  float error_threshold, vint8 maximum_rounds);

  char * classifier_filename(vint8 dimensions, vint8 length);

  vint8_matrix accuracy_labels(vint8_matrix retrieval_output, const vint8 length);
  static vint8_matrix accuracy_labels(vint8_matrix retrieval_output, vint8 length, vint8 max_neighbors);

  vint8 prepare_AdaBoost_input(vint8 dimensions, vint8 length, 
                              float_matrix & training_vectors, vint8_matrix & training_labels,
                              float_matrix & test_vectors, vint8_matrix & test_labels);

  vint8 save(const char * filename);
  vint8 load(const char * filename);

  vint8 retrieval_results(vint8 last_dimensions, vint8 last_length,
                          float & distances, float & accuracy, vint8 & accuracy_counter);

  vint8 upper_bound_distances() const;
  vint8 set_step_threshold(vint8 dimensions, vint8 length, float new_threshold);
  vint8 print_cascade() const;
};


  

























#endif // VASSILIS_REFINEMENT_H
