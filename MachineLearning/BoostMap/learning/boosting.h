#ifndef VASSILIS_BOOSTING_H
#define VASSILIS_BOOSTING_H


#include "basics/algebra.h"





class simple_classifier
{
public:
  vint8 feature_index;
  float threshold;
  vint8 direction;
  float factor;

  // weighted errors, i.e., errors as measured on the re-weighted set
  float training_error;
  float test_error;
  float loss;

  // unweighted errors of strong classifier up to and including this classifier.
  float strong_training_error;
  float strong_test_error;

protected:
  void initialize();

public:
  simple_classifier();
  simple_classifier(const float_matrix data);

  inline vint8 hard_classify(const float feature) const
  {
    if (feature > threshold)
    {
      return round_number(direction);
    }
    else if (feature < threshold)
    {
      return round_number(-direction);
    }
    else
    {
      return 0;
    }
  }

  inline vint8 hard_classify(const float_matrix object) const
  {
    float feature = object(feature_index);
    vint8 result = hard_classify(feature);
    return result;
  }

  inline float soft_classify(const float feature) const
  {
    if (feature > threshold)
    {
      return factor * (float) direction;
    }
    else if (feature < threshold)
    {
      return factor * (-direction);
    }
    else
    {
      return 0.0f;
    }
  }

  inline float soft_classify(const float_matrix object) const
  {
    float feature = object(feature_index);
    float result = soft_classify(feature);
    return result;
  }

  // returns true if the subject has a lower
  // loss than the other object.
  inline vint8 better(simple_classifier & classifier) const
  {
    vint8 result = (training_error < classifier.training_error);
    return result;
  }


  vint8 save(file_handle * file_pointer) const;
  vint8 load(file_handle * file_pointer);
  vint8 print() const;
};


class class_result_summary
{
public:
  // statistics on objects with positive true class labels
  vint8 classified_positives;
  vint8 misclassified_positives;
  vint8 tie_positives;

  // statistics on objects with negative true class labels
  vint8 classified_negatives;
  vint8 misclassified_negatives;
  vint8 tie_negatives;

  // the rest of the variables should be updated (using update()) once
  // the above variables are specified
  vint8 total_positives;
  vint8 total_negatives;
  vint8 total_number;
  vint8 total_classified;
  vint8 total_misclassified;
  vint8 total_ties;

  float classified_positive_fraction;
  float tie_positive_fraction;
  float misclassified_positive_fraction;

  float classified_negative_fraction;
  float misclassified_negative_fraction;
  float tie_negative_fraction;

  float classified_fraction;
  float tie_fraction;
  float misclassified_fraction;

  float accuracy;
  float error;

public:
  // given statistics on objects with positive and negative true class labels,
  // update the rest of the variables.
  class_result_summary(vint8_matrix labels, float_matrix results, float threshold);
  vint8 update();
  vint8 print();
  vint8 print(file_handle * file_pointer);
};



class AdaBoost
{
protected:
  vint8 training_number;
  vint8 test_number;

  float_matrix training_vectors;
  float_matrix test_vectors;
  vint8_matrix training_labels;
  vint8_matrix test_labels;

  double_matrix training_factors;

  float_matrix training_results;
  float_matrix test_results;
  float_matrix training_loss;
  float_matrix test_loss;

  // each row in this matrix is an ordered set of thresholds
  // for a particular feature.  Those thresholds should be tried
  // when we build classifiers based on that feature.
  float_matrix classifier_thresholds;

  std::vector<simple_classifier> classifiers;

  vint8 generate_classifier_thresholds();
  vint8 update_training_factors(simple_classifier classified, vint8_matrix accuracies);
  vint8 update_results(simple_classifier classifier);

  // save a strong classifier to file.
  vint8 save_classifier_matrix(const char * filename, float_matrix classifier_matrix);
  vint8 initialize(float_matrix argument_training_vectors, vint8_matrix argument_training_labels);
  
  // this does the actual work of saving all the info to the specified filename.
  vint8 save_auxiliary(const char * filename);

public:
  AdaBoost(float_matrix argument_training_vectors, vint8_matrix argument_training_labels);
  AdaBoost(float_matrix argument_training_vectors, vint8_matrix argument_training_labels,
           float_matrix argument_test_vectors, vint8_matrix argument_test_labels);
  AdaBoost(const char * positive_filename, const char * negative_filename, vint8 * success);

  // create a deep copy of this object.
  AdaBoost * copy();

  vint8 classifier_number();
  vint8 feature_number();
  vint8_matrix get_training_labels() const;
  vint8_matrix get_test_labels() const;
  
  static vint8_matrix classifier_hard_results(simple_classifier classifier, 
                                             float_matrix objects);

  static float_matrix classifier_soft_results(simple_classifier classifier, 
                                              float_matrix objects);

  static vint8_matrix classifier_accuracies(simple_classifier classifier, float_matrix objects,
                                           vint8_matrix labels);
  
  vint8_matrix classifications(float_matrix objects, vint8_matrix labels, float threshold = 0);
  vint8_matrix accuracies(float_matrix objects, vint8_matrix labels, float threshold = 0);

  vint8_matrix training_hard_results(simple_classifier classifier);
  float_matrix training_soft_results(simple_classifier classifier);
  vint8_matrix training_accuracies(simple_classifier classifier);
  vint8_matrix training_classifications(float threshold = 0);
  vint8_matrix training_accuracies(float threshold = 0);

  vint8_matrix test_hard_results(simple_classifier classifier);
  float_matrix test_soft_results(simple_classifier classifier);
  vint8_matrix test_accuracies(simple_classifier classifier);
  vint8_matrix test_classifications(float threshold = 0);
  vint8_matrix test_accuracies(float threshold = 0);

  // finds the best classifier we can define using the current training weights
  // and the given feature.
  simple_classifier feature_classifier_slow(vint8 feature);

  // updates the given classifier by computing error and loss statistics
  vint8 update_classifier(vint8_matrix results, simple_classifier & classifier);
  float compute_classifier_factor(simple_classifier classifier, vint8_matrix accuracies);
  vint8 insert_classifier(simple_classifier classifier);

  vint8 next_step();
  vint8 next_steps(vint8 steps);
  vint8 train();
  vint8 train(const char * filename, const float error_threshold,
             const vint8 maximum_rounds);

  float soft_classify(float_matrix object, float threshold = 0);
  vint8 hard_classify(float_matrix object, float threshold = 0);

  float_matrix get_training_results();
  float_matrix get_test_results();

  // this function is useful for finding appropriate thresholds
  // for classifiers that are used as steps in a cascade. Those thresholds
  // typically depend on the number of misclassified negative objects,
  // as a fraction of the total number of training objects.
  float find_threshold_training(const double misclassified_negatives) const;

  // here we find the threshold based on the test set.
  float find_threshold_test(const double misclassified_negatives) const;

  static float find_threshold(const vint8_matrix labels, const float_matrix estimates, 
                              const double misclassified_negatives);

  vint8 print_errors(float threshold = 0) const;
  vint8 print_errors_training(float misclassified_negatives) const;
  vint8 print_errors_test(float misclassified_negatives) const;
  vint8 print_factors() const;
  vint8 print() const;

  float_matrix classifier_matrix();
  vint8 insert_classifier_matrix(float_matrix classifiers);
  char * base_directory();

  // convert a simple filename into a complete pathname, to which
  // we can save a classifier, or from which we can load a classifier.
  // there are two variants, one for when we went to save
  // and one for when we want to load.  There is an is that when we want 
  // to save we change the filename in a round-robin fashion,
  // and the other is for when we want to load or save and we
  // specify the actual filename,
  // including the round-robin extension.
  // note that when we save we save both on the round-robin filename
  // and the fully specified filename.
  char * make_round_robin_pathname(const char * filename);
  char * make_pathname(const char * filename);

  // function save uses save_auxiliary to save information about this object
  // in two files: one is the filename that is specified, and the other is
  // a round-robin filename.
  vint8 save(const char * filename);

  // number specifies the number of classifiers to load
  // if it is 0 or negative we load all classifiers,
  // otherwise we load the first "number" of them
  vint8 load(const char * filename, vint8 number = 0);
  float last_error();

  // simply counts the number of entries that are less than zero.
  static vint8 count_negatives(vint8_matrix labels);
};


// initializes an AdaBoost object with a synthetic data set
AdaBoost * synthetic_AdaBoost(vint8 objects, vint8 features, vint8 noise);























#endif // VASSILIS_BOOSTING_H
