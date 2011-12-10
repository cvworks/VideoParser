
#include "boosting.h"
#include "basics/simple_algo_templates.h"

#include "basics/definitions.h"

extern const char * g_data_directory;


void simple_classifier::initialize()
{
  feature_index = 0;
  threshold = 0.0f;
  direction = 0;
  factor = 0.0f;

  training_error = 2.0f;
  test_error = 2.0f;
  loss = 2.0f;

  strong_training_error = 2.0f;
  strong_test_error = 2.0f;
}


simple_classifier::simple_classifier()
{
  initialize();
}

simple_classifier::simple_classifier(float_matrix data)
{
  initialize();

  vint8 number = data.length();
  feature_index = round_number(data(0));
  threshold = data(1);
  direction = round_number(data(2));
  factor = data(3);

  if (number >= 6)
  {
    training_error = data(4);
  }
  if (number >= 10)
  {
    strong_training_error = data(7);
    strong_test_error = data(8);
  }
}


vint8 simple_classifier::save(file_handle * file_pointer) const
{
  vint8 result = 1;
  vint8 items;
  items = store_vint8s(file_pointer, &feature_index, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_save_floats(file_pointer, &threshold, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = store_vint8s(file_pointer, &direction, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_save_floats(file_pointer, &factor, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_save_floats(file_pointer, &loss, 1);
  if (items != 1)
  {
    result = 0;
  }

  return result;
}


vint8 simple_classifier::load(file_handle * file_pointer)
{
  vint8 result = 1;
  vint8 items;
  items = read_vint8s(file_pointer, &feature_index, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_read_floats(file_pointer, &threshold, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = read_vint8s(file_pointer, &direction, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_read_floats(file_pointer, &factor, 1);
  if (items != 1)
  {
    result = 0;
  }

  items = function_read_floats(file_pointer, &loss, 1);
  if (items != 1)
  {
    result = 0;
  }

  return result;
}


vint8 simple_classifier::print()  const
{
  function_print("\n");
  function_print("feature_index = %li\n", feature_index);
  function_print("threshold = %f\n", threshold);
  function_print("direction = %li\n", direction);
  function_print("factor = %f\n", factor);

  function_print("training_error = %f\n", training_error);
  function_print("test_error = %f\n", test_error);

  return 1;
}


class_result_summary::class_result_summary(vint8_matrix labels, float_matrix results,
                                           float threshold)
{
  vint8 number = labels.length();
  if (number != results.length())
  {
    exit_error("\nerror in class_result_summary constructor\n");
  }

  classified_positives = 0;
  misclassified_positives = 0;
  tie_positives = 0;

  classified_negatives = 0;
  misclassified_negatives = 0;
  tie_negatives = 0;

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    vint8 label = labels(counter);
    float result = results(counter) - threshold;
    float product = result * (float) label;

    if (label > 0)
    {
      if (product < 0)
      {
        misclassified_positives++;
      }
      else if (product > 0)
      {
        classified_positives++;
      }
      else
      {
        tie_positives++;
      }
    }
    else
    {
      if (product < 0)
      {
        misclassified_negatives++;
      }
      else if (product > 0)
      {
        classified_negatives++;
      }
      else
      {
        tie_negatives++;
      }
    }
  }

  update();
}


vint8 class_result_summary::update()
{
  total_positives = classified_positives + misclassified_positives + tie_positives;
  total_negatives = classified_negatives + misclassified_negatives + tie_negatives;
  total_number = total_positives + total_negatives;
  total_classified = classified_positives + classified_negatives;
  total_misclassified = misclassified_positives + misclassified_negatives;
  total_ties = tie_positives + tie_negatives;

  if ((total_positives == 0) || (total_negatives == 0))
  {
    exit_error("\nerror in class_result_summary\n");
  }

  classified_positive_fraction = 0.0f, misclassified_positive_fraction = 0.0f;
  classified_negative_fraction = 0.0f, misclassified_negative_fraction = 0.0f;
  classified_fraction = 0.0f, misclassified_fraction = 0.0f;

  classified_positive_fraction = ((float) classified_positives) / total_positives;
  misclassified_positive_fraction = ((float) misclassified_positives) / total_positives;
  tie_positive_fraction = ((float) tie_positives) / total_positives;

  classified_negative_fraction = ((float) classified_negatives) / total_negatives;
  misclassified_negative_fraction = ((float) misclassified_negatives) / total_negatives;
  tie_negative_fraction = ((float) tie_negatives) / total_negatives;

  classified_fraction = ((float) total_classified) / total_number;
  misclassified_fraction = ((float) total_misclassified) / total_number;
  tie_fraction = ((float) total_ties) / total_number;

  accuracy = classified_fraction + tie_fraction / 2.0f;
  error = misclassified_fraction + tie_fraction / 2.0f;

  return 1;
}


vint8 class_result_summary::print()
{
  update();

  function_print("\n");

  function_print("%i objects, %i positive, %i negative\n", 
                 (int) total_number, (int) total_positives, (int) total_negatives);
  function_print("+: %i (%.5f) classified, %i (%.5f) misclassified, %i (%.5f) ties\n", 
                 (int) classified_positives, classified_positive_fraction, (int) misclassified_positives,
                 misclassified_positive_fraction, (int) tie_positives, tie_positive_fraction);
  function_print("-: %i (%.5f) classified, %i (%.5f) misclassified, %i (%.5f) ties\n", 
                 (int) classified_negatives, classified_negative_fraction, (int) misclassified_negatives,
                 misclassified_negative_fraction, (int) tie_negatives, tie_negative_fraction);
  function_print("%i (%.5f) classified, %i (%.5f) misclassified, %i (%.5f) ties\n", 
                 (int) total_classified, classified_fraction, (int) total_misclassified,
                 misclassified_fraction, (int) total_ties, tie_fraction);
  function_print("error: %.5f, accuracy: %.5f\n", error, accuracy);

  return 1;
}


vint8 class_result_summary::print(file_handle * file_pointer)
{
  update();

  fprintf(file_pointer, "\n");

  fprintf(file_pointer, "%li objects, %li positive, %li negative\n", 
                 total_number, total_positives, total_negatives);
  fprintf(file_pointer, "+: %li (%.5f) classified, %li (%.5f) misclassified, %li (%.5f) ties\n", 
                 classified_positives, classified_positive_fraction, misclassified_positives,
                 misclassified_positive_fraction, tie_positives, tie_positive_fraction);
  fprintf(file_pointer, "+: %li (%.5f) classified, %li (%.5f) misclassified, %li (%.5f) ties\n", 
                 classified_negatives, classified_negative_fraction, misclassified_negatives,
                 misclassified_negative_fraction, tie_negatives, tie_negative_fraction);
  fprintf(file_pointer, "%li (%.5f) classified, %li (%.5f) misclassified, %li (%.5f) ties\n", 
                 total_classified, classified_fraction, total_misclassified,
                 misclassified_fraction, total_ties, tie_fraction);
  fprintf(file_pointer, "error: %.5f, accuracy: %.5f\n", error, accuracy);

  return 1;
}



AdaBoost::AdaBoost(float_matrix argument_training_vectors, 
                   vint8_matrix argument_training_labels)
{
  initialize(argument_training_vectors, argument_training_labels);
}

  
AdaBoost::AdaBoost(const char * positive_filename, const char * negative_filename, vint8 * success)
{
  float_matrix positives = float_matrix::read(positive_filename);
  if (positives.valid() <= 0)
  {
    *success = 0;
    return;
  }
  float_matrix negatives = float_matrix::read(negative_filename);
  if (negatives.valid() <= 0)
  {
    *success = 0;
    return;
  }

  vint8 number = positives.vertical() + negatives.vertical();
  float_matrix training_samples(number, positives.horizontal());
  vint8_matrix labels(1, number);

  positives.Copy(&training_samples, 0, 0);
  negatives.Copy(&training_samples, positives.vertical(), 0);
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    if (counter < positives.vertical())
    {
      labels(counter) = 1;
    }
    else
    {
      labels(counter) = -1;
    }
  }

  initialize(training_samples, labels);
  *success = 1;
}



vint8 AdaBoost::initialize(float_matrix argument_training_vectors, 
                          vint8_matrix argument_training_labels)
{
  training_vectors = argument_training_vectors;
  training_labels = argument_training_labels;
  training_number = (vint8) training_vectors.vertical();
  if (training_number != training_labels.length())
  {
    exit_error("error: incompatible sizes of matrices passed to AdaBoost constructor\n");
  }

  test_number = 0;
  training_factors = double_matrix(1, training_number);
  function_enter_value(& training_factors, 1.0/(double) training_number);
  training_results = float_matrix(1, training_number);
  function_enter_value(& training_results, (float) 0);
  training_loss = float_matrix(1, training_number);
  function_enter_value(& training_loss, (float) 0);

  generate_classifier_thresholds();

  return 1;
}


AdaBoost::AdaBoost(float_matrix argument_training_vectors, vint8_matrix argument_training_labels,
                   float_matrix argument_test_vectors, vint8_matrix argument_test_labels)
{
  initialize(argument_training_vectors, argument_training_labels);
  test_vectors = argument_test_vectors;
  test_labels = argument_test_labels;
  test_number = (vint8) test_vectors.vertical();
  if (test_number != test_labels.length())
  {
    exit_error("error: incompatible sizes of matrices passed to AdaBoost constructor\n");
  }

  test_results = float_matrix(1, test_number);
  function_enter_value(& test_results, (float) 0);
  test_loss = float_matrix(1, test_number);
  function_enter_value(& test_loss, (float) 0);
}


// create a deep copy of this object.
AdaBoost * AdaBoost::copy()
{
  AdaBoost * result = new AdaBoost(training_vectors, training_labels, test_vectors, test_labels);
  vint8 number = classifiers.size();

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    result->insert_classifier(classifiers[(vector_size) counter]);
  }

  return result;
}




vint8 AdaBoost::classifier_number()
{
  return classifiers.size();
}


vint8 AdaBoost::feature_number()
{
  return (vint8) training_vectors.horizontal();
}


vint8_matrix AdaBoost::get_training_labels() const
{
  return vint8_matrix(training_labels);
}


vint8_matrix AdaBoost::get_test_labels() const
{
  return vint8_matrix(test_labels);
}


vint8_matrix AdaBoost::classifier_hard_results(simple_classifier classifier, 
                                              float_matrix objects)
{
  vint8 number = objects.vertical();
  vint8_matrix result(1, number);

  vint8 object;
  for (object = 0; object < number; object++)
  {
    float object_feature = objects(object, classifier.feature_index);
    result(object) = classifier.hard_classify(object_feature);
  }

  return result;
}


float_matrix AdaBoost::classifier_soft_results(simple_classifier classifier, 
                                              float_matrix objects)
{
  vint8 number = objects.vertical();
  float_matrix result(1, number);

  vint8 object;
  for (object = 0; object < number; object++)
  {
    float object_feature = objects(object, classifier.feature_index);
    result(object) = classifier.soft_classify(object_feature);
  }

  return result;
}


// result(i) = -1, 0, 1, depending on whether the i-th object
// was misclassified, not classified (i.e., classifier output was zero), 
// or correctly classified.
vint8_matrix AdaBoost::classifier_accuracies(simple_classifier classifier, 
                                            float_matrix objects, vint8_matrix labels)
{
  vint8 number = labels.length();
  vint8_matrix classifications = classifier_hard_results(classifier, objects);
  vint8_matrix result(1, number);

  vint8 object;
  for (object = 0; object < number; object++)
  {
    result(object) = classifications(object) * labels(object);
  }

  return result;
}


vint8_matrix AdaBoost::classifications(float_matrix objects, vint8_matrix labels, float threshold)
{
  vint8 number = objects.vertical();
  if (number != labels.length())
  {
    exit_error("\nerror in classifier_accuracies\n");
  }
  vint8_matrix result(1, number);
  
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    float_matrix current = objects.horizontal_line(counter);
    vint8 classification = hard_classify(current, threshold);
    result(counter) = classification;
  }

  return result;
}

  
vint8_matrix AdaBoost::accuracies(float_matrix objects, vint8_matrix labels, float threshold)
{
  vint8 number = objects.vertical();
  if (number != labels.length())
  {
    exit_error("\nerror in classifier_accuracies\n");
  }
  vint8_matrix result(1, number);
  
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    float_matrix current = objects.horizontal_line(counter);
    vint8 classification = hard_classify(current, threshold);
    vint8 label = labels(counter);
    if (classification == label)
    {
      result(counter) = 1;
    }
    else
    {
      result(counter) = 0;
    }
  }

  return result;
}

  
vint8_matrix AdaBoost::training_hard_results(simple_classifier classifier)
{
  return classifier_hard_results(classifier, training_vectors);
}

float_matrix AdaBoost::training_soft_results(simple_classifier classifier)
{
  return classifier_soft_results(classifier, training_vectors);
}

vint8_matrix AdaBoost::training_accuracies(simple_classifier classifier)
{
    return classifier_accuracies(classifier, training_vectors, training_labels);
}

vint8_matrix AdaBoost::training_classifications(float threshold)
{
  return classifications(training_vectors, training_labels, threshold);
}

vint8_matrix AdaBoost::training_accuracies(float threshold)
{
  return accuracies(training_vectors, training_labels, threshold);
}

vint8_matrix AdaBoost::test_hard_results(simple_classifier classifier)
{
  return classifier_hard_results(classifier, test_vectors);
}

float_matrix AdaBoost::test_soft_results(simple_classifier classifier)
{
  return classifier_soft_results(classifier, test_vectors);
}

vint8_matrix AdaBoost::test_accuracies(simple_classifier classifier)
{
    return classifier_accuracies(classifier, test_vectors, test_labels);
}

vint8_matrix AdaBoost::test_classifications(float threshold)
{
  return classifications(test_vectors, test_labels, threshold);
}

vint8_matrix AdaBoost::test_accuracies(float threshold)
{
  return accuracies(test_vectors, test_labels, threshold);
}

  
vint8 AdaBoost::generate_classifier_thresholds()
{
  vint8 features = feature_number();
  vint8 threshold_number = training_number - 1;
  float_matrix first_classifier_thresholds = float_matrix(features, threshold_number);

  vint8 feature, object;
  for (feature = 0; feature < features; feature++)
  {
    vector<float> temporary;
    temporary.reserve((vector_size) training_number);
    for (object = 0; object < training_number; object++)
    {
      temporary.push_back(training_vectors(object, feature));
    }
    std::sort(temporary.begin(), temporary.end(), less<float>());

    for (object = 0; object < threshold_number; object++)
    {
      float first = temporary[(vector_size) object];
      float second = temporary[(vector_size) (object+1)];
      float threshold = (first + second) / 2.0f;
      first_classifier_thresholds(feature, object) = threshold;
    }
  }

  if (threshold_number <= 1000)
  {
    classifier_thresholds = first_classifier_thresholds;
  }
  else
  {
    classifier_thresholds = float_matrix(features, 1000);
    vint8 step = threshold_number / 1000;
    for (feature = 0; feature < features; feature++)
    {
      vint8 counter;
      for (counter = 0; counter < 1000; counter++)
      {
        vint8 index = counter * step;
        classifier_thresholds(feature, counter) = first_classifier_thresholds(feature, index);
      }
    }
  }

  return 1;
}


// finds the best classifier we can define using the current training weights
// and the given feature.
simple_classifier AdaBoost::feature_classifier_slow(vint8 feature)
{
  simple_classifier best;
  vint8 number = classifier_thresholds.horizontal();

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    float threshold = classifier_thresholds(feature, counter);
    simple_classifier classifier;
    classifier.feature_index = feature;
    classifier.threshold = threshold;
    classifier.direction = 1;

    vint8_matrix results = training_hard_results(classifier);

    // compute direction, error, and loss
    update_classifier(results, classifier);

    if (classifier.better(best))
    {
      best = classifier;
    }
  }

  return best;
}


// updates the given classifier by computing error and loss statistics
vint8 AdaBoost::update_classifier(vint8_matrix results, simple_classifier & classifier)
{
  if (results.length() != training_number)
  {
    exit_error("\nerror: bad size of results in update_classifier\n");
  }

  float classified = 0.0f, misclassified = 0.0f, ties = 0.0f;

  vint8 counter;
  for (counter = 0; counter < training_number; counter++)
  {
    vint8 label =  training_labels(counter);
    vint8 result = results(counter);
    vint8 product = label * result;
    float factor = (float) training_factors(counter);

    if (product < 0)
    {
      misclassified = misclassified + factor;
    }
    else if (product > 0)
    {
      classified = classified + factor;
    }
    else
    {
      ties = ties + factor;
    }
  }

  float error = misclassified + ties / 2.0f;
  if (error > 0.5f)
  {
    error = 1.0f - error;
    classifier.direction = -classifier.direction;
  }

  classifier.training_error = error;
  classifier.loss = 1.0f - 2.0f * error;

  return 1;
}
    

float AdaBoost::compute_classifier_factor(simple_classifier classifier, 
                                          vint8_matrix accuracies)
{
  float error = classifier.training_error;

  if (error <= 0)
  {
    // strictly speaking this is not an error, at least not when error is equal to zero.
    // I just need to write code to handle that case.
    //exit_error("\nerror: in compute_classifier_factor, error = %f\n", error);
    return 0;
  }

  float factor = 0.5f * log((1.0f - error) / error);
  return factor;
}


vint8 AdaBoost::update_training_factors(simple_classifier classifier, vint8_matrix accuracies)
{
  vint8 number = training_number;
  double total = 0;
  double factor = classifier.factor;

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    double accuracy = (double) accuracies(counter);
    double current = training_factors(counter);
    double adjustment = exp(-factor * accuracy);
    double new_value = current * adjustment;
    training_factors(counter) = new_value;
    total = total + new_value;
  }

  for (counter = 0; counter < number; counter++)
  {
    double current = training_factors(counter);
    double new_value = current / total;
    training_factors(counter) = new_value;
  }

  return 1;
}


vint8 AdaBoost::update_results(simple_classifier classifier)
{
  float_matrix new_training_results = training_soft_results(classifier);
  training_results = training_results + new_training_results;

  if (test_number > 0)
  {
    float_matrix new_test_results = test_soft_results(classifier);
    test_results = test_results + new_test_results;
  }

  return 1;
}

  
vint8 AdaBoost::insert_classifier(simple_classifier classifier)
{
  vint8_matrix results = training_hard_results(classifier);
  update_classifier(results, classifier);

  vint8_matrix accuracies = training_accuracies(classifier);
  classifier.factor = compute_classifier_factor(classifier, accuracies);

  double total_accuracy = 0.0f;
  vint8 counter;
  for (counter = 0; counter < training_number; counter++)
  {
    float feature = training_vectors(counter, classifier.feature_index);
    vint8 result = results(counter);
    vint8 accuracy = accuracies(counter);
    double factor = training_factors(counter);
    double contribution = 0.0f;
    if (accuracy > 0)
    {
      contribution = factor;
    }
    else if (accuracy == 0)
    {
      contribution = factor/2.0f;
    }
    total_accuracy = total_accuracy + contribution;

 //   function_print("%10.7f, %4li, %4li, %10.7f, %10.7f, %10.7f\n",
 //                   feature, result, accuracy, factor, contribution, total_accuracy);
  }

  update_training_factors(classifier, accuracies);
  update_results(classifier);

  class_result_summary training(training_labels, training_results, 0);
  classifier.strong_training_error = training.error;

  if (test_number > 0)
  {
    class_result_summary test(test_labels, test_results, 0);
    classifier.strong_test_error = test.error;
  }

  classifiers.push_back(classifier);
  return 1;
}


vint8 AdaBoost::next_step()
{
  simple_classifier best;
  vint8 features = feature_number();

  vint8 feature;
  function_print("\n");
  for (feature = 0; feature < features; feature++)
  {
    simple_classifier classifier = feature_classifier_slow(feature);
    if (classifier.better(best))
    {
      best = classifier;
    }
    function_print("processed %li of %li features\r", feature+1, features);
  }
  function_print("\n");

  insert_classifier(best);
  return 1;
}


vint8 AdaBoost::next_steps(vint8 steps)
{
  vint8 counter;
  for (counter = 0; counter < steps; counter++)
  {
    next_step();
    print();
  }

  return 1;
}


vint8 AdaBoost::train()
{
  float error = 0;
  while (error < .499)
  {
   // next_step();
    vint8 number = classifier_number();
    error = classifiers[(vector_size) (number - 1)].training_error;
  }

  return 1;
}


vint8 AdaBoost::train(const char * filename, const float error_threshold,
                     const vint8 maximum_rounds)
{
  vint8 counter;
  for (counter = 0; counter < maximum_rounds; counter++)
  {
    next_step();
    vint8 number = classifier_number();
    float error = classifiers[(vector_size) (number-1)].training_error;
    if (error >= error_threshold)
    {
      break;
    }

    print();
    save(filename);
  }

  return 1;
}



float AdaBoost::soft_classify(float_matrix object, float threshold)
{
  float result = 0;
  vint8 number = classifier_number();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    result = result + classifiers[(vector_size) counter].soft_classify(object);
  }

  result = result - threshold;
  return result;
}


vint8 AdaBoost::hard_classify(float_matrix object, float threshold)
{
  float soft_result = soft_classify(object, threshold);
  if (soft_result < 0)
  {
    return -1;
  }
  else if (soft_result > 0)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


float_matrix AdaBoost::get_training_results()
{
  float_matrix result(training_results);
  return result;
}


float_matrix AdaBoost::get_test_results()
{
  float_matrix result(test_results);
  return result;
}


// this function is useful for finding appropriate thresholds
// for classifiers that are used as steps in a cascade. Those thresholds
// typically depend on the number of misclassified negative objects,
// as a fraction of the total number of training objects.
float AdaBoost::find_threshold_training(const double misclassified_negatives) const
{
  return find_threshold(training_labels, training_results, misclassified_negatives);
}


// here we find the threshold based on the test set.
float AdaBoost::find_threshold_test(const double misclassified_negatives) const
{
  return find_threshold(test_labels, test_results, misclassified_negatives);
}


float AdaBoost::find_threshold(const vint8_matrix labels, const float_matrix estimates, 
                               const double misclassified_negatives)
{
  vint8 number = labels.length();
  vint8 negatives_total = count_negatives(labels);
  vector<float> negative_results;
  negative_results.reserve((vector_size) negatives_total);

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    if (labels(counter) < 0)
    {
      negative_results.push_back(estimates(counter));
    }
  }

  if (negative_results.size() != negatives_total)
  {
    exit_error("error: in AdaBoost::find_threshold\n");
  }
  if (negative_results.size() == 0)
  {
    return 0.0f;
  }

  std::sort(negative_results.begin(), negative_results.end(), std::less<float>());
  double accuracy = 1.0 - misclassified_negatives;
  vint8 misclassified_number = round_number(misclassified_negatives * (double) number);
  vint8 correctly_classified = negatives_total - misclassified_number;
  vint8 index = correctly_classified;
  
  if (index <= 0)
  {
    return (float) (negative_results[0] - 0.0001);
  }
  else if (index >= negatives_total)
  {
    return (float) (negative_results[(vector_size) negatives_total - 1] + 0.0001);
  }
  else
  {
    float current = negative_results[(vector_size) index];
    float previous = negative_results[(vector_size) index - 1];
    float result = (current + previous) / 2.0f;
    return result;
  }
}


vint8 AdaBoost::count_negatives(vint8_matrix labels)
{
  vint8 number = labels.length();
  vint8 result = 0;

  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    if (labels(counter) < 0)
    {
      result++;
    }
  }

  return result;
}


vint8 AdaBoost::print_errors(const float threshold) const
{
  class_result_summary training(training_labels, training_results, threshold);
  training.print();

  if (test_number > 0)
  {
    class_result_summary test(test_labels, test_results, threshold);
    test.print();
  }

  return 1;
}


vint8 AdaBoost::print() const
{
  print_errors(0);
  vint8 number = classifiers.size();
  function_print("\n%li classifiers\n", number);

  if (number > 0)
  {
    function_print("last classifier:\n");
    classifiers[(vector_size) number-1].print();
  }
  return 1;
}


vint8 AdaBoost::print_factors() const
{
  training_factors.print("training weights:", "%lf ");
  return 1;
}

  
float_matrix AdaBoost::classifier_matrix()
{
  vint8 number = classifiers.size();
  if (number == 0)
  {
    return float_matrix();
  }

  float_matrix result(number, 10);
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    const simple_classifier classifier = classifiers[(vector_size) counter];
    result(counter, 0) = (float) classifier.feature_index;
    result(counter, 1) = classifier.threshold;
    result(counter, 2) = (float) classifier.direction;
    result(counter, 3) = classifier.factor;

    result(counter, 4) = classifier.training_error;
    result(counter, 5) = classifier.test_error;
    result(counter, 6) = classifier.loss;
    result(counter, 7) = classifier.strong_training_error;
    result(counter, 8) = classifier.strong_test_error;
    result(counter, 9) = (float) counter;
  }

  return result;
}


vint8 AdaBoost::insert_classifier_matrix(float_matrix strong_classifier)
{
 if ((strong_classifier.valid() == 0) || (strong_classifier.Cols() < 4))
  {
    exit_error("error: invalid classifier matrix\n");
    return 0;
  }

  vint8 rows = strong_classifier.Rows();
  vint8 i;
  for (i = 0; i < rows; i++)
  {
    float_matrix current = strong_classifier.horizontal_line(i);
    simple_classifier classifier(current);
    insert_classifier(classifier);
    print();
  }

  return 1;
}

  
char * AdaBoost::base_directory()
{
  char * result = function_join_paths_three(g_data_directory, "experiments", "boosting");
  return result;
}

// convert a simple filename into a complete pathname, to which
// we can save a classifier, or from which we can load a classifier.
// convert a simple filename into a complete pathname, to which
// we can save a classifier, or from which we can load a classifier.
// there are two variants, one for when we went to save
// and one for when we want to load.  There is an is that when we want 
// to save we change the filename in a round-robin fashion,
// and when we load we specify the actual filename,
// including the round-robin extension.
char * AdaBoost::make_round_robin_pathname(const char * filename)
{
  vint8 number = classifiers.size() % 10;
  char * number_string = string_from_number(number);

  char * directory = base_directory();
  char * temporary_name = vJoinPaths(directory, filename);
  char * pathname = vMergeStrings4(temporary_name, "_robin_", 
                                     number_string, ".txt");

  delete_pointer(number_string);
  delete_pointer(temporary_name);
  vdelete2(directory);
  return pathname;
}


char * AdaBoost::make_pathname(const char * filename)
{
  char * directory = base_directory();
  char * temporary_name = vJoinPaths(directory, filename);
  char * pathname = vMergeStrings2(temporary_name, ".txt");

  delete_pointer(temporary_name);
  vdelete2(directory);
  return pathname;
}


vint8 AdaBoost::save(const char * filename)
{
  char * round_robin_name = make_round_robin_pathname(filename);
  char * pathname = make_pathname(filename);
  vMatrix<float> strong_classifier = classifier_matrix();
  vint8 success = 1;
  success *= save_classifier_matrix(round_robin_name, strong_classifier);
  success *= save_classifier_matrix(pathname, strong_classifier);

  delete_pointer(round_robin_name);
  delete_pointer(pathname);

  return 1;
}


// save a strong classifier to file.
vint8 AdaBoost::save_classifier_matrix(const char * pathname, float_matrix classifier_matrix)
{
  FILE * fp = fopen(pathname, vFOPEN_WRITE);
  if (fp == 0)
  {
    vPrint("Failed to open %s\n", pathname);
    return 0;
  }

  vint8 success = classifier_matrix.WriteText(fp);
  if (success <= 0)
  {
    vPrint("Failed to save classifier to %s\n", pathname);
  }

  class_result_summary training(training_labels, training_results, 0.0f);
  training.print(fp);

  if (test_number > 0)
  {
    class_result_summary test(test_labels, test_results, 0.0f);
    test.print(fp);
  }

  fclose(fp);
  return success;
}


// number specifies the number of classifiers to load
// if it is 0 or negative we load all classifiers,
// otherwise we load the first "number" of them
vint8 AdaBoost::load(const char * filename, vint8 number)
{
  char * path_name = make_pathname(filename);
  vMatrix<float> strong_classifier = vMatrix<float>::ReadText(path_name);
  if ((strong_classifier.valid() == 0) || (strong_classifier.Cols() < 4))
  {
    vPrint("Failed to load classifier from %s\n", path_name);
    vdelete2(path_name);
    return 0;
  }
  vdelete2(path_name);

  if (number <= 0)
  {
    number = strong_classifier.vertical();
  }
  else
  {
    number = function_minimum(number, strong_classifier.vertical());
  }

  vint8 horizontal = strong_classifier.horizontal();
  float_matrix selected(number, horizontal);
  strong_classifier.Copy(& selected, 0, number -1, 0, horizontal -1, 0, 0);

  return insert_classifier_matrix(selected);
}


float AdaBoost::last_error()
{
  vint8 number = classifiers.size();
  if (number == 0)
  {
    return 0.0f;
  }

  return classifiers[(vector_size) (number-1)].training_error;
}


// initializes an AdaBoost object with a synthetic data set
AdaBoost * synthetic_AdaBoost(vint8 objects, vint8 features, vint8 noise)
{
  float_matrix training_objects(objects, features);
  float_matrix test_objects(objects, features);

  vint8_matrix training_labels(1, objects);
  vint8_matrix test_labels(1, objects);

  vint8 object;
  for (object = 0; object < objects; object++)
  {
    training_labels(object) = function_random_vint8(0, 1) * 2 - 1;
    test_labels(object) = function_random_vint8(0, 1) * 2 - 1;

    vint8 counter;
    for (counter = 0; counter < features; counter++)
    {
      float training_noise = (float) function_random_vint8(- noise, noise);
      training_objects(object, counter) = training_labels(object) + training_noise;

      float test_noise = (float) function_random_vint8(- noise, noise);
      test_objects(object, counter) = test_labels(object) + test_noise;
    }
  }

  AdaBoost* result = new AdaBoost(training_objects, training_labels, test_objects, test_labels);
  return result;
}



