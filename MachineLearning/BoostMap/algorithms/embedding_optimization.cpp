
#include "embedding_optimization.h"
#include "basics/simple_algo_templates.h"

#include "basics/definitions.h"


embedding_optimizer::embedding_optimizer(const char* root_dir, const char * dataset_name, long argument_number_of_couples) :
class_BoostMap(root_dir, dataset_name, 1000, 0)
{
  delete_pointer(class_name);
  class_name = function_copy_string("embedding_optimizer");
  number_of_couples = argument_number_of_couples;
  choose_training_couples();
  choose_validation_couples();

  distance_multiplier = 1;
  allow_lipschitz = 1;
  allow_projections = 0;
  allow_sensitive = 1;
  new_sensitive = 1;

  // setting number_of_thresholds to one is the setting for optimizing
  // a query-insensitive embedding.
  number_of_thresholds = 1; 

  training_factors = float_matrix(1, training_number);
  function_enter_value(& training_factors, 0.0f);
  validation_factors = float_matrix(1, validation_number);
  function_enter_value(& validation_factors, 0.0f);
}


embedding_optimizer::~embedding_optimizer()
{
}


long embedding_optimizer::choose_training_couples()
{
  training_couples_matrix = data->make_couples(number_of_couples, training_number);
  training_couples = training_couples_matrix.planar();

  training_couple_distances_matrix = data->training_couple_distances(training_couples_matrix);
  training_couple_distances = training_couple_distances_matrix.flat();

  training_estimated_distances_matrix = float_matrix(1, number_of_couples);
  function_enter_value(& training_estimated_distances_matrix, (float) 0);
  return 1;
}


long embedding_optimizer::choose_validation_couples()
{
  validation_couples_matrix = data->make_couples(number_of_couples, validation_number);
  validation_couples = validation_couples_matrix.planar();

  validation_couple_distances_matrix = data->validation_couple_distances(validation_couples_matrix);
  validation_couple_distances = validation_couple_distances_matrix.flat();

  validation_estimated_distances_matrix = float_matrix(1, number_of_couples);
  function_enter_value(& validation_estimated_distances_matrix, (float) 0);
  return 1;
}


long embedding_optimizer::stress_optimization_step()
{
  if ((is_valid <= 0) || (allow_sensitive == 0))
  {
    return 0;
  }

  // get candidate query-sensitive classifiers.
  vector<class_sensitive_classifier> classifier_candidates_vector;
  if (new_sensitive == 0)
  {
    restricted_candidates(&classifier_candidates_vector);
  }
  else
  {
    random_candidates(&classifier_candidates_vector);
  }

  long number = (long) classifier_candidates_vector.size();
  if (number == 0) 
  {
    vPrint("no sensitive candidates are available\n");
    return 1;
  }

  vector<class_sensitive_classifier> classifier_candidates(number);
  
  // I am permuting the indices to see if that makes a difference
  // in the number of unique classifiers chosen (the code
  // breaks ties in favor of the classifier examined first).
  vint8_matrix indices = vPermutation(0, number-1);

  long i;
  for (i = 0; i < number; i++)
  {
    vint8 index = indices(i);
    classifier_candidates[i] = classifier_candidates_vector[(vector_size) index];
  }

  // go through the most promising candidates, and pick the
  // one that minimizes Z.
  long best_index = 0;
  double best_alpha = (double) 0;
  double smallest_stress = 0, alpha = 0, constant = 0;
  classifier_stress(&(classifier_candidates[0]), &smallest_stress,
                    & constant, & best_alpha);

  vPrint("\n");
  for (i = 1; i < number; i++)
  {
    double stress = 0;
    classifier_stress(&(classifier_candidates[i]), &stress,
                      & constant, & alpha);
    if (stress < smallest_stress)
    {
      smallest_stress = stress;
      best_index = i;
      best_alpha = alpha;
    }
    vPrint("evaluated classifier %li of %li\r", 
           (long) (i+1), (long) classifier_candidates.size());
  }
  vPrint("\n");

  // form a classifier out of the result.
  class_sensitive_classifier result = classifier_candidates[best_index];
  result.classifier.weight = (float) best_alpha;

  result.classifier.z = (float) smallest_stress;
  
  last_new_z = result.classifier.z;
  vPrint("\n");
  result.Print();
  vPrint("\n");

  // add the classifier to the strong clsasifier.
  vint8 return_value = AddSensitiveClassifier(result);
  return (long) return_value;  
}


long embedding_optimizer::distortion_optimization_step()
{
  exit_error("\nunimplemented distortion_optimization_step\n");

  return 1;
}

// This function is useful when we want to fully specify the classifier,
// for example if we want to load an existing classifier, so we want
// to use the weight that has already been computed.
//long embedding_optimizer::AddClassifier(class_triple_classifier classifier)
//{
//  class_BoostMap::
//
//  return 1;
//}


// creates a list of candidate query-sensitive classifiers using only
// classifiers that appeared in unique_classifiers, so that the dimensionality
// of the embedding will not increase if we choose any of those classifiers.
long embedding_optimizer::restricted_candidates(vector<class_sensitive_classifier> * classifiers)
{
  long number = unique_classifiers.size();
  long i;
  for (i = 0; i < number; i++)
  {
    class_triple_classifier base = unique_classifiers[i];
    class_triple_classifier splitter = base;

    class_sensitive_classifier classifier;
    classifier.classifier = base;
    classifier.splitter = splitter;
    classifier.split_type = 1;
    classifiers->push_back(classifier);
  }

  return 1;
}


// creates a list of candidate query-sensitive classifiers, chosen among
// all possible classifiers we can define using the candidate objects,
// so that the dimensionality
// of the embedding may possibly increase if we choose any of those classifiers.
long embedding_optimizer::random_candidates(vector<class_sensitive_classifier> * classifiers)
{
  vector<class_triple_classifier> base_classifiers;
  RandomCandidates(&base_classifiers);

  long i;
  long number = base_classifiers.size();
  for (i = 0; i < (long) base_classifiers.size(); i++)
  {
    class_triple_classifier base = base_classifiers[i];
    class_triple_classifier splitter = base;

    class_sensitive_classifier classifier;
    classifier.classifier = base;
    classifier.splitter = splitter;
    classifier.split_type = 1;
    classifiers->push_back(classifier);
  }

  return 1;
}



// Add the sensitive classifier to sensitive_classifiers,
// and do the necessary updates (compute errors, update training
// weights).
vint8 embedding_optimizer::AddSensitiveClassifier(class_sensitive_classifier classifier)
{
  classifier.distance_factor = 1;

  // compute information about performance on the training set
  float_matrix training_embeddings = compute_embeddings(classifier.classifier, 
                                                        candtrain_distances_matrix);
  training_factors = adjust_factors(training_factors, training_embeddings, classifier);

  float_matrix distances = sensitive_couple_distances(classifier,
                                                      training_couples_matrix,
                                                      candtrain_distances_matrix);  
  float_matrix unnormalized_distances = distances + training_estimated_distances_matrix;
  float_matrix new_distances = adjust_distances(unnormalized_distances, training_couples_matrix,
                                                training_factors);
  double stress = 0, constant = 0, alpha = 1.0f;
  measure_stress(new_distances, training_couple_distances_matrix,
                 & stress, & constant);

  distance_multiplier = (float) constant;
  stresses.push_back((float) stress);
  distortions.push_back(-1.0f);
  classifier.distance_multiplier = distance_multiplier;
  training_estimated_distances_matrix = unnormalized_distances;

  // compute information about performance on the validation set
  float_matrix validation_embeddings = compute_embeddings(classifier.classifier, 
                                                          candval_distances_matrix);
  validation_factors = adjust_factors(validation_factors, validation_embeddings, classifier);

  distances = sensitive_couple_distances(classifier,
                                         validation_couples_matrix,
                                         candval_distances_matrix);  
  unnormalized_distances = distances + validation_estimated_distances_matrix;
  new_distances = adjust_distances(unnormalized_distances, validation_couples_matrix,
                                   validation_factors);
//  measure_stress(new_distances, validation_couple_distances_matrix,
//                 & stress, & constant);
  stress = measure_stress(new_distances * distance_multiplier, 
                          validation_couple_distances_matrix);

  validation_stresses.push_back((float) stress);
  validation_distortions.push_back(-1.0f);
  validation_estimated_distances_matrix = unnormalized_distances;

  class_BoostMap::AddSensitiveClassifier(classifier);

  return 1;
}


// Add the sensitive classifier to sensitive_classifiers,
// and do the necessary updates (compute errors, update training
// weights). This does not update distance multiplier
vint8 embedding_optimizer::AddSensitiveClassifier2(class_sensitive_classifier classifier)
{
  classifier.distance_factor = 1;

  // compute information about performance on the training set
  float_matrix training_embeddings = compute_embeddings(classifier.classifier, 
                                                        candtrain_distances_matrix);
  training_factors = adjust_factors(training_factors, training_embeddings, classifier);

  float_matrix distances = sensitive_couple_distances(classifier,
                                                      training_couples_matrix,
                                                      candtrain_distances_matrix);  
  float_matrix unnormalized_distances = distances + training_estimated_distances_matrix;
  float_matrix new_distances = adjust_distances(unnormalized_distances, training_couples_matrix,
                                                training_factors);
  float stress = measure_stress(new_distances * distance_multiplier, training_couple_distances_matrix);

  stresses.push_back((float) stress);
  distortions.push_back(-1.0f);
  classifier.distance_multiplier = distance_multiplier;
  training_estimated_distances_matrix = unnormalized_distances;

  // compute information about performance on the validation set
  float_matrix validation_embeddings = compute_embeddings(classifier.classifier, 
                                                          candval_distances_matrix);
  validation_factors = adjust_factors(validation_factors, validation_embeddings, classifier);

  distances = sensitive_couple_distances(classifier,
                                         validation_couples_matrix,
                                         candval_distances_matrix);  
  unnormalized_distances = distances + validation_estimated_distances_matrix;
  new_distances = adjust_distances(unnormalized_distances, validation_couples_matrix,
                                   validation_factors);
//  measure_stress(new_distances, validation_couple_distances_matrix,
//                 & stress, & constant);
  stress = measure_stress(new_distances * distance_multiplier, 
                          validation_couple_distances_matrix);

  validation_stresses.push_back((float) stress);
  validation_distortions.push_back(-1.0f);
  validation_estimated_distances_matrix = unnormalized_distances;

  class_BoostMap::AddSensitiveClassifier(classifier);

  return 1;
}


vint8 embedding_optimizer::PrintSummary()
{
  class_BoostMap::PrintSummary();
  function_print("%li couples, %li thresholds\n",
        (long) number_of_couples, (long) number_of_thresholds);
  function_print("%li couples per threshold, %li objects per threshold\n",
                 (long) (number_of_couples / number_of_thresholds), 
                 (long) (training_number / number_of_thresholds));
  if (stresses.size() > 0)
  {
    function_print("last stress = %f, validation stress = %f\n",
          stresses[stresses.size() - 1], validation_stresses[stresses.size() - 1]);
    function_print("last distortion = %f, validation distortion = %f\n",
                   validation_distortions[validation_distortions.size() - 1], 
                   validation_distortions[validation_distortions.size() - 1]);
    function_print("distance_multiplier = %f\n", distance_multiplier);
  }

  return 1;
}


long embedding_optimizer::classifier_stress(class_sensitive_classifier * classifier, 
                                            double * stress_pointer,
                                            double * constant_pointer, double * alpha_pointer)
{
  if (classifier->classifier.type != 0)
  {
    exit_error("\nerror: classifier_stress cannot handle line projections\n");
  }

  // get embeddings of training objects based on classifier
  // we assume that classifier.classifier = classifier.splitter
  float_matrix classifier_embeddings = compute_training_embeddings(classifier->classifier);
  float_matrix new_distances = insensitive_couple_distances(classifier->classifier.object1, training_couples_matrix,
                                                            candtrain_distances_matrix);  
  
  // find the best stress, and associated weight (alpha) and classifier
  // area of influence.
  minimize_stress(classifier, classifier_embeddings, new_distances, stress_pointer, 
                  constant_pointer, alpha_pointer);
  
  return 1;
}


// this function simply computes the stress, constant, and alpha for 
// the specified classifier and the specified area of influence
float_matrix embedding_optimizer::classifier_stress(const class_sensitive_classifier classifier, 
                                                    double * stress_pointer, double * constant_pointer)
{
  if ((classifier.split_type != 1) || (classifier.classifier.type != 0))
  {
    exit_error("\nerror: classifier_stress cannot handle this classifier\n");
  }

  float_matrix distances = sensitive_couple_distances(classifier,
                                                      training_couples_matrix,
                                                      candtrain_distances_matrix);  
  float_matrix unnormalized_distances = distances + training_estimated_distances_matrix;
  float_matrix embeddings = compute_embeddings(classifier.classifier, candtrain_distances_matrix);
  float_matrix factors = adjust_factors(training_factors, embeddings, classifier);
  float_matrix new_distances = adjust_distances(unnormalized_distances, training_couples_matrix,
                                                factors);
  double stress = 0, constant = 0;
  measure_stress(new_distances, training_couple_distances_matrix,
                 & stress, & constant);

  *stress_pointer = stress;
  *constant_pointer = constant;
  
  return distances;
}

  
float embedding_optimizer::classifier_distortion(class_sensitive_classifier * classifier, 
                                                 double * alpha_pointer)
{
  exit_error("\nunimplemented classifier_distortion\n");

  return 1;
}


float_matrix embedding_optimizer::insensitive_couple_distances(const vint8 candidate_index, 
                                                               const vint8_matrix couples,
                                                               const float_matrix distances)
{
  if ((candidate_index <0) || (candidate_index >= distances.vertical()))
  {
    exit_error("\nerror: invalid index %li in classifier_couple_distances\n", (long) candidate_index);
  }
  
  vint8 number = couples.Rows();
  float_matrix result(1, number);

  vint8 set_size = distances.Cols();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q = couples(i, 0);
    vint8 a = couples(i, 1);

    if ((q < 0) || (a < 0)||
        (q >= set_size) || (a >= set_size))
    {
      exit_error("Error: (q,a,) = (%li ,%li), set_size = %li\n",
                      (long) q, (long) a, (long) set_size);
    }

    float qr = distances(candidate_index, q);
    float ar = distances(candidate_index, a);

    result(i) = vAbs(qr - ar);
  }

  return result;
}


float_matrix embedding_optimizer::sensitive_couple_distances(const class_sensitive_classifier & classifier, 
                                                             const vint8_matrix couples_matrix,
                                                             const float_matrix distances_matrix)
{
  float_matrix classifier_embeddings = compute_embeddings(classifier.classifier, distances_matrix);
  float_matrix distances = insensitive_couple_distances(classifier.classifier.object1,
                                                        couples_matrix,
                                                        distances_matrix);  
  distances = distances * classifier.classifier.weight;

  float threshold = classifier.high;
  float_matrix sensitive_distances = query_sensitive_distances(distances, classifier_embeddings,
                                                               couples_matrix, threshold);

  return sensitive_distances;
}


long embedding_optimizer::minimize_stress(class_sensitive_classifier * classifier, 
                                          const float_matrix classifier_embeddings, 
                                          const float_matrix distances, double * stress_pointer, 
                                          double * constant_pointer, double * alpha_pointer)
{
  if ((classifier->split_type != 1) || (classifier->classifier.type != 0))
  {
    exit_error("\nerror: minimize_stress cannot handle this classifier\n");
  }

  float_matrix thresholds = candidate_thresholds(classifier_embeddings, number_of_thresholds);

  long counter;

  long best_counter = -1;
  double best_stress = 0;
  double best_alpha = 0;
  double best_constant = 0;
  for (counter = 0; counter < thresholds.length(); counter++)
  {
    float threshold = thresholds(counter);
    float_matrix sensitive_distances = query_sensitive_distances(distances, classifier_embeddings,
                                                                 training_couples_matrix, threshold);


    float_matrix unnormalized_distances = sensitive_distances + training_estimated_distances_matrix;
    classifier->high = threshold;
    float_matrix embeddings = compute_embeddings(classifier->classifier, candtrain_distances_matrix);
    float_matrix factors = adjust_factors(training_factors, embeddings, *classifier);
    float_matrix new_distances = adjust_distances(unnormalized_distances, training_couples_matrix,
                                                  factors);

    double stress = 0, constant = 0, alpha = 1.0f;
    measure_stress(new_distances, training_couple_distances_matrix,
                   & stress, & constant);
    if ((counter == 0) || (stress < best_stress))
    {
      best_counter = counter;
      best_stress = stress;
      best_constant = constant;
      best_alpha = alpha;
    }
  }

  classifier->low = 0;
  classifier->high = thresholds(best_counter);
  classifier->range = round_number(((float) training_number) / ((float) number_of_thresholds) * (best_counter + 1));
  classifier->distance_factor = 1.0f;
  *stress_pointer = best_stress;
  *constant_pointer = best_constant;
  *alpha_pointer = best_alpha;
  
  return 1;
}

  
long embedding_optimizer::set_number_of_thresholds(long new_number)
{
  if (new_number <= 0)
  {
    function_print("\nnumber_of_thresholds cannot be set to %li\n", (long) new_number);
  }

  number_of_thresholds = new_number;
  return 1;
}


float_matrix embedding_optimizer::candidate_thresholds(const float_matrix classifier_embeddings, 
                                                       const long number)
{
  vector<float> sorted_values;
  vector_from_matrix(& classifier_embeddings, & sorted_values);
  std::sort(sorted_values.begin(), sorted_values.end());
  vint8 size = classifier_embeddings.length();

  float_matrix result(1, number);
  vint8 counter;
  for (counter = 1; counter < number; counter++)
  {
    vint8 index = round_number(((float) size) / ((float) number) * ((float) counter));
    if (index > 0)
    {
      float threshold = (sorted_values[(vector_size) (index-1)] + sorted_values[(vector_size) index]) / 2.0f;
      result(counter -1) = threshold;
    }
    else
    {
      float threshold = sorted_values[(vector_size) index];
      result(counter -1) = threshold;
    }
  }

  float maximum = sorted_values[(vector_size) (size -1)];
  // make the last entry a really large number, to make the area of influence
  // virtually global.
  result(number -1) = (1.0f + function_absolute(maximum)) * 1000;

  return result;
}


float_matrix embedding_optimizer::query_sensitive_distances(const float_matrix distances, 
                                                            const float_matrix classifier_embeddings,
                                                            const vint8_matrix couples_matrix, 
                                                            const float threshold)
{
  vint8 number = distances.length();
  if (number != couples_matrix.vertical())
  {
    exit_error("\nerror: in query_sensitive_distances\n");
  }

  float_matrix result(1, number);  
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    vint8 first = couples_matrix(counter, 0);
    float first_embedding = classifier_embeddings(first);
    if (first_embedding >= threshold)
    {
      result(counter) = 0.0f;
    }
    else
    {
      result(counter) = distances(counter);
    }
  }

  return result;
}


long embedding_optimizer::measure_stress(const float_matrix estimated_distances, 
                                         const float_matrix true_distances,
                                         double * stress_pointer, double * factor_pointer)
{
  float true_average = (float) function_image_average(& true_distances);
  float estimated_average = (float) function_image_average(& estimated_distances);
  if ((true_average <= 0) || (estimated_average <= 0))
  {
    exit_error("\nerror: negative distance average in measure_stress\n");
  }

  float constant = true_average / estimated_average;
  float_matrix normalized_distances = constant * estimated_distances;
  float_matrix differencesf = true_distances - normalized_distances;
  differencesf = differencesf / true_average;

  double_matrix differences(&differencesf);
  double_matrix squared_error_total = differences * differences.Trans();
  double stress = sqrt(squared_error_total(0) / (double) estimated_distances.length());

  *stress_pointer = stress;
  *factor_pointer = constant;

  return 1;
}

                            
// in this version, we do not multiply distances by any factor,
// we just compare distances as they are
float embedding_optimizer::measure_stress(const float_matrix estimated_distances, 
                                          const float_matrix true_distances)
{
  float true_average = (float) function_image_average(& true_distances);
  float_matrix differencesf = true_distances - estimated_distances;
  differencesf = differencesf / true_average;

  double_matrix differences(&differencesf);
  double_matrix squared_error_total = differences * differences.Trans();
  double stress = sqrt(squared_error_total(0) / (double) estimated_distances.length());

  return (float) stress;
}


float_matrix embedding_optimizer::adjust_distances(float_matrix distances, 
                                                   vint8_matrix couples,
                                                   float_matrix factors)
{
  vint8 number = distances.length();

  float_matrix result(1, number);
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    float distance = distances(counter);
    vint8 query = couples(counter, 0);
    float factor = factors(query);
    if (factor == 0)
    {
      result(counter) = 0.0f;
    }
    else
    {
      result(counter) = distance / factor;
    }
  }

  return result;
}


float_matrix embedding_optimizer::adjust_factors(float_matrix factors, float_matrix embeddings,
                                                 const class_sensitive_classifier classifier)
{
  vint8 number = factors.length();
  if (number != embeddings.length())
  {
    exit_error("\nerror: incompatible sizes in adjust_factors\n");
  }

  float_matrix result(1, number);
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    float embedding = embeddings(counter);
    if (embedding < classifier.high)
    {
      result(counter) = factors(counter) + classifier.classifier.weight;
    }
    else
    {
      result(counter) = factors(counter);
    }
  }

  return result;
}


long embedding_optimizer::test_magnification(long reference_object, 
                                             long number_of_neighbors)
{
  if ((number_of_neighbors >= training_number) || 
      (number_of_neighbors <= 1))
  {
    return 0;
  }
  class_triple_classifier classifier(reference_object, -1, -1);
  float_matrix classifier_embeddings = compute_embeddings(classifier,
                                                          candtrain_distances_matrix);
  vector<float> embeddings;
  vector_from_matrix(& classifier_embeddings, & embeddings);
  std::sort(embeddings.begin(), embeddings.end(), less<float>());
  float threshold = embeddings[number_of_neighbors];

  vector<float> estimated_distances, true_distances;
  long counter;
  for (counter = 0; counter < number_of_couples; counter++)
  {
    vint8 query = training_couples_matrix(counter, 0);
    if (classifier_embeddings(query) >= threshold)
    {
      continue;
    }

    float training_factor = training_factors(query);
    float estimated_distance = training_estimated_distances_matrix(counter);
    estimated_distance = estimated_distance / training_factor;
    estimated_distances.push_back(estimated_distance);
    float true_distance = training_couple_distances_matrix(counter);
    true_distances.push_back(true_distance);
  }

  long number = estimated_distances.size();
  float_matrix estimates = matrix_from_vector(& estimated_distances);
  float_matrix exact = matrix_from_vector(&true_distances);
  double new_stress = 0, factor = 0;
  measure_stress(estimates, exact, & new_stress, & factor);
  float current_stress = measure_stress(estimates * distance_multiplier, exact);
  float new_stress2 = measure_stress(estimates * (float) factor, exact);

  function_print("distance_multiplier = %f, best_multiplier = %f\n",
                 distance_multiplier, factor);
  function_print("current stress = %f, new stress = %f, new_stress2 = %f\n",
                 current_stress, (float) new_stress, new_stress2);

  // now compute statistics for the validation pairs
  estimated_distances.clear();
  true_distances.clear();
  classifier_embeddings = compute_embeddings(classifier, candval_distances_matrix);
  for (counter = 0; counter < number_of_couples; counter++)
  {
    vint8 query = validation_couples_matrix(counter, 0);
    if (classifier_embeddings(query) >= threshold)
    {
      continue;
    }

    float validation_factor = validation_factors(query);
    float estimated_distance = validation_estimated_distances_matrix(counter);
    estimated_distance = estimated_distance / validation_factor;
    estimated_distances.push_back(estimated_distance);
    float true_distance = validation_couple_distances_matrix(counter);
    true_distances.push_back(true_distance);
  }

  number = estimated_distances.size();
  if (number <= 1)
  {
    return 1;
  }
  estimates = matrix_from_vector(& estimated_distances);
  exact = matrix_from_vector(&true_distances);
  new_stress = measure_stress(estimates * (float) factor, exact);
  current_stress = measure_stress(estimates * distance_multiplier, exact);

  function_print("number = %li, distance_multiplier = %f, best_multiplier = %f\n",
                 (long) number, distance_multiplier, factor);
  function_print("validation current stress = %f, validation new stress = %f\n",
                 current_stress, (float) new_stress);

  return 1;
}


long embedding_optimizer::magnification_statistics(long reference_object, long number_of_neighbors,
                                                   float * improvement_pointer, float * magnification_pointer,
                                                   float * threshold_pointer)
{
  if ((number_of_neighbors >= training_number) || 
      (number_of_neighbors <= 1))
  {
    return 0;
  }
  class_triple_classifier classifier(reference_object, -1, -1);
  float_matrix classifier_embeddings = compute_embeddings(classifier,
                                                          candtrain_distances_matrix);
  vector<float> embeddings;
  vector_from_matrix(& classifier_embeddings, & embeddings);
  std::sort(embeddings.begin(), embeddings.end(), less<float>());
  float threshold = embeddings[number_of_neighbors];

  vector<float> estimated_distances, true_distances;
  long counter;
  for (counter = 0; counter < number_of_couples; counter++)
  {
    vint8 query = training_couples_matrix(counter, 0);
    if (classifier_embeddings(query) >= threshold)
    {
      continue;
    }

    float training_factor = training_factors(query);
    float estimated_distance = training_estimated_distances_matrix(counter);
    estimated_distance = estimated_distance / training_factor;
    estimated_distances.push_back(estimated_distance);
    float true_distance = training_couple_distances_matrix(counter);
    true_distances.push_back(true_distance);
  }

  long number = estimated_distances.size();
  float_matrix estimates = matrix_from_vector(& estimated_distances);
  float_matrix exact = matrix_from_vector(&true_distances);
  double new_stress = 0, factor = 0;
  measure_stress(estimates, exact, & new_stress, & factor);
  float current_stress = measure_stress(estimates * distance_multiplier, exact);
  
  float improvement = (float) (current_stress - new_stress);
  float normalized_improvement = improvement * ((float) number) / (float) number_of_couples;

  *improvement_pointer = normalized_improvement;
  *magnification_pointer = (float) factor;
  *threshold_pointer = threshold;
  return 1;
}


long embedding_optimizer::magnification_step(long step, long number_of_steps)
{
  long number = unique_classifiers.size();
  if (number == 0)
  {
    return 0;
  }

  float best_improvement = 0, best_magnification = 0, best_threshold = 0;
  long best_index = -1;
  long counter;
  function_print("\n");
//  for (counter = 0; counter < number; counter++)
  for (counter = 0; counter < candidate_number; counter++)
  {
    function_print("processing object %li of  %li\r", (long) (counter + 1), (long) candidate_number);
    //class_triple_classifier classifier = unique_classifiers[counter];
    //if (classifier.type != 0)
    //{
    //  exit_error("\nerror: magnification_step only works with type 0 classifiers\n");
    //}
    //long reference_object = classifier.object1;
    long reference_object = counter;
    
    long step_counter;
    for (step_counter = 1; step_counter <= number_of_steps; step_counter++)
    {
      long number_of_neighbors = step_counter * step;
      float improvement = 0, magnification = 0, threshold = 0;
      magnification_statistics(reference_object, number_of_neighbors,
                              & improvement, & magnification, & threshold);
      if (improvement > best_improvement)
      {
        best_improvement = improvement;
        best_magnification = magnification;
        best_threshold = threshold;
        best_index = reference_object;
      }
    }
  }
  function_print("\n");
  function_print("best improvement = %f\n", best_improvement);

  if (best_index == -1)
  {
    function_print("failed to improve stress\n");
    return 0;
  }

  add_magnification(best_index, best_threshold, best_magnification);

  return 1;
}


long embedding_optimizer::add_magnification(long reference_object, float threshold, 
                                            float magnification)
{
  magnification_objects.push_back(reference_object);
  magnification_thresholds.push_back(threshold);
  magnification_factors.push_back(magnification);

  class_triple_classifier classifier(reference_object, -1, -1);
  float_matrix classifier_embeddings = compute_embeddings(classifier,
                                                          candtrain_distances_matrix);
  long counter;
  for (counter = 0; counter < training_number; counter++)
  {
    if (classifier_embeddings(counter) >= threshold)
    {
      continue;
    }

    float training_factor = training_factors(counter);
    training_factors(counter) = training_factor * distance_multiplier / magnification;
  }

  classifier_embeddings = compute_embeddings(classifier, candval_distances_matrix);
  for (counter = 0; counter < validation_number; counter++)
  {
    if (classifier_embeddings(counter) >= threshold)
    {
      continue;
    }

    float validation_factor = validation_factors(counter);
    validation_factors(counter) = validation_factor * distance_multiplier / magnification;
  }

  classifier.weight = 0;
  class_sensitive_classifier sensitive;
  sensitive.classifier = classifier;
  sensitive.splitter = classifier;
  sensitive.split_type = 1;
  sensitive.high = threshold;

  AddSensitiveClassifier2(sensitive);

  return 1;
}






































