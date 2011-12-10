#ifndef VASSILIS_EMBEDDING_OPTIMIZATION_H
#define VASSILIS_EMBEDDING_OPTIMIZATION_H


#include "learning/boost_kdd.h"


//In this file we define procedures for optimizing embeddings 
//for stress and distortion. We produce both query-insensitive and
//query-sensitive embeddings.


class embedding_optimizer : public class_BoostMap
{
protected:
  long number_of_couples;
  long number_of_thresholds;

  // these matrices specify the pairs of objects on which we will measure
  // stress and distortion year during training
  vint8_matrix training_couples_matrix;
  vint8_matrix validation_couples_matrix;
  matrix_pointer(vint8) training_couples;
  matrix_pointer(vint8) validation_couples;

  // these matrices specify the true distances between
  // the selected pairs of objects
  float_matrix training_couple_distances_matrix;
  float_matrix validation_couple_distances_matrix;
  class_pointer(float) training_couple_distances;
  class_pointer(float) validation_couple_distances;

  // these matrices specify the current estimated distances,
  // according to the embedding, between the selected
  // pairs of objects
  float_matrix training_estimated_distances_matrix;
  float_matrix validation_estimated_distances_matrix;

  // here we record the stress and distortion attained after each
  // training step
  std::vector<float> stresses;
  std::vector<float> distortions;
  std::vector<float> validation_stresses;
  std::vector<float> validation_distortions;

  // a constant that we must scale all distances by in order to
  // minimize stress;
  float distance_multiplier;

  // in these matrices we store, for each training and validation object,
  // the number of dimensions for which that object is within the
  // area of influence for that dimension.
  float_matrix training_factors;
  float_matrix validation_factors;

  // for query-sensitive magnification
  vector<long> magnification_objects;
  vector<float> magnification_thresholds;
  vector<float> magnification_factors;

protected:
  long choose_training_couples();
  long choose_validation_couples();

public:
  embedding_optimizer(const char* root_dir, const char * dataset_name, long number_of_couples);
  ~embedding_optimizer();

  long stress_optimization_step();
  long distortion_optimization_step();

  // This function is useful when we want to fully specify the classifier,
  // for example if we want to load an existing classifier, so we want
  // to use the weight that has already been computed.
  //virtual long AddClassifier(class_triple_classifier classifier);

  // creates a list of candidate query-sensitive classifiers using only
  // classifiers that appeared in unique_classifiers, so that the dimensionality
  // of the embedding will not increase if we choose any of those classifiers.
  long restricted_candidates(vector<class_sensitive_classifier> * classifiers);

  // creates a list of candidate query-sensitive classifiers, chosen among
  // all possible classifiers we can define using the candidate objects,
  // so that the dimensionality
  // of the embedding may possibly increase if we choose any of those classifiers.
  long random_candidates(vector<class_sensitive_classifier> * classifiers);

  // Add the sensitive classifier to sensitive_classifiers,
  // and do the necessary updates (compute errors, update training
  // weights).
  virtual vint8 AddSensitiveClassifier(class_sensitive_classifier classifier);
  virtual vint8 AddSensitiveClassifier2(class_sensitive_classifier classifier);

  virtual vint8 PrintSummary();

  // this function find the best area of influence for the given classifier
  long classifier_stress(class_sensitive_classifier * classifier, double * stress_pointer,
                         double * constant_pointer, double * alpha_pointer);

  // this function simply computes the stress, constant, and alpha for 
  // the specified classifier, area of influence, and weight.
  float_matrix classifier_stress(const class_sensitive_classifier classifier, double * stress_pointer,
                                 double * constant_pointer);

  float classifier_distortion(class_sensitive_classifier * classifier, double * alpha_pointer);

  // here we do not multiply by a weight
  float_matrix insensitive_couple_distances(const vint8 candidate_index, 
                                            const vint8_matrix training_couples_matrix,
                                            const float_matrix distances_matrix);

  // here we multiply distances by the weight of the classifier
  float_matrix sensitive_couple_distances(const class_sensitive_classifier & classifier, 
                                          const vint8_matrix training_couples_matrix,
                                          const float_matrix distances_matrix);


  // this function find the best area of influence for the given classifier
  long minimize_stress(class_sensitive_classifier * classifier, 
                       const float_matrix classifier_embeddings, 
                       const float_matrix results, double * stress_pointer, 
                       double * constant_pointer, double * alpha_pointer);

  long set_number_of_thresholds(long new_number);

  float_matrix candidate_thresholds(const float_matrix classifier_embeddings, 
                                    const long number);

  float_matrix query_sensitive_distances(const float_matrix distances, const float_matrix classifier_embeddings,
                                         const vint8_matrix couples_matrix, const float threshold);

  long measure_stress(const float_matrix estimated_distances, const float_matrix true_distances,
                      double * stress_pointer, double * factor_pointer);

  // in this version, we do not multiply distances by any factor,
  // we just compare distances as they are
  float measure_stress(const float_matrix estimated_distances,
                       const float_matrix true_distances);

  static float_matrix adjust_distances(float_matrix distances, vint8_matrix couples,
                                       float_matrix factors);
  static float_matrix adjust_factors(float_matrix factors, float_matrix embeddings,
                                     const class_sensitive_classifier classifier);

  long test_magnification(long reference_object, long number_of_neighbors);

  long magnification_statistics(long reference_object, long number_of_neighbors,
                                float * improvement_pointer, float * magnification_pointer,
                                float * threshold_pointer);

  long magnification_step(long step, long number);

  long add_magnification(long reference_object, float threshold, float magnification);
};













#endif // VASSILIS_EMBEDDING_OPTIMIZATION_H
