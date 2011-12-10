
#include "boost_nn.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"
#include "boosting.h"

#include "basics/definitions.h"




vBnn::vBnn()
{
  Zero();
  class_name = function_copy_string("vBnn");
}



vBnn::vBnn(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vint8 in_training_number, vint8 in_validation_number)
{
  Zero();
  class_name = function_copy_string("vBnn");
  InitializeData(in_training_vectors, in_training_labels,
                 in_test_vectors, in_test_labels);
  InitializeA(in_training_number, in_validation_number);
}


vBnn::vBnn(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vMatrix<vint8> in_training_triples,
                     vMatrix<vint8> in_validation_triples)
{
  Zero();
  class_name = function_copy_string("vBnn");
  InitializeData(in_training_vectors, in_training_labels,
                 in_test_vectors, in_test_labels);
  InitializeB(in_training_triples, in_validation_triples);
}

vBnn::~vBnn()
{
  vdelete2(class_indices);
  delete_pointer(class_name);
}


vint8 vBnn::Zero()
{
  class_name = 0;
  training_number = 0;
  test_number = 0;
  classes = 0;
  attributes = 0;

  training_triple_number = 0;
  validation_triple_number = 0;

  last_error = -1; 
  last_et = -1;
  last_correlation = -1;
  last_scaled_et = -1;
  last_scaled_correlation = -1;

  training_error = -1;
  training_margin = -1;
  validation_error = -1;
  test_error2 = -1;
  test_triple_error2 = -1;

  iterations = 0;
  min_exp = 0;
  max_exp = 0;
  allow_negative_weights = 0;
  subject_ids_set = 0;
  best_k = -1;
  best_knn_training = (float) 2;
  best_knn_test = (float) 2;

  class_indices = vZero(vector<vint8> );  

  return 1;
}


// Initialize training and test data.
vint8 vBnn::InitializeData(vMatrix<float> in_training_vectors,
                          vMatrix<vint8> in_training_labels,
                          vMatrix<float> in_test_vectors,
                          vMatrix<vint8> in_test_labels)
{
  training_vectors_matrix = in_training_vectors;
  training_labels_matrix = in_training_labels;
  test_vectors_matrix = in_test_vectors;
  test_labels_matrix = in_test_labels;

  training_vectors = training_vectors_matrix.Matrix2();
  training_labels = training_labels_matrix.Matrix();
  test_vectors = test_vectors_matrix.Matrix2();
  test_labels = test_labels_matrix.Matrix();

  training_number = training_vectors_matrix.Rows();
  test_number = test_vectors_matrix.Rows();

  if (training_number < 4)
  {
    exit_error("Error: training_number = %li < 4\n", training_number);
  }

  vdelete2(class_indices);
  // get number of classes. Remember that training labels take values
  // from 1 to C, where C is the number of classes.
  classes = function_image_maximum(&training_labels_matrix);
  // make sure the number of classes in the test set makes sense.
  vint8 test_classes = function_image_maximum(&test_labels_matrix);
  if ((classes < test_classes) || (classes < 2))
  {
    exit_error("Error: training classes = %li, test classes = %li\n",
                    classes, test_classes);
  }

  // Just to make sure, verify that no class label is smaller than 1.
  vint8 training_min = function_image_minimum(&training_labels_matrix);
  vint8 test_min = function_image_minimum(&test_labels_matrix);
  if ((training_min < 1) || (test_min < 1))
  {
    exit_error("Error: training_min = %li, test_min = %li\n",
                    training_min, test_min);
  }

  attributes = training_vectors_matrix.Cols();
  // Make various sanity checks on the sizes of the matrices.
  if (attributes != test_vectors_matrix.Cols())
  {
    exit_error("Error: attributes = %li, test vectors has %li cols\n",
                    attributes, test_vectors_matrix.Cols());
  }
  if (training_labels_matrix.Size() != training_number)
  {
    exit_error("Error: %li training objects, only %li labels\n",
                    training_number, training_labels_matrix.Size());
  }
  if (test_labels_matrix.Size() != test_number)
  {
    exit_error("Error: %li test objects, only %li labels\n",
                    test_number, test_labels_matrix.Size());
  }

  // initialize class indices and class sizes. For structures
  // where we store class-specific information, we allocate classes+1
  // slots, so that we can index based on class id (we need to add 1
  // since no class has ID 0).
  class_indices = vnew(vector<vint8>, (vector_size) classes+1);
  class_sizes_matrix = vMatrix<vint8>(1, classes+1);
  class_sizes = class_sizes_matrix.Matrix();
  function_enter_value(&class_sizes_matrix, (vint8) 0);

  long i;
  for (i = 0; i < training_number; i++)
  {
    vint8 label = training_labels[i];
    class_indices[label].push_back(i);
    class_sizes[label] = class_sizes[label] + 1;
  }

  // We want to ensure that there are at least two training
  // objects for each class. Otherwise the code should be 
  // a bit more complicated, to allow for the case when, for
  // some class, no training triple (q,a,b) has q and a be elements 
  // of that class.
  for (i = 1; i <= classes; i++)
  {
    if (class_sizes[i] < 2)
    {
      vPrint("Error: class_sizes[%li] = %li\n",
              i, class_sizes[i]);
    }
  }

  test_distances_matrix = vMatrix<float>(test_number, training_number);
  function_enter_value(&test_distances_matrix, (float) 0);

  active_training = vint8_matrix (1, training_number);
  function_enter_value (& active_training, (vint8) 1);
  previous_training_labels = vint8_matrix (1, training_number);
  function_enter_value (& previous_training_labels, (vint8) -1);

  active_test = vint8_matrix (1, test_number);
  function_enter_value (& active_test, (vint8) 1);
  previous_test_labels = vint8_matrix (1, test_number);
  function_enter_value (& previous_test_labels, (vint8) -1);

  return 1;
}


// Initialize training and validation triples.
vint8 vBnn::InitializeA(vint8 in_training_number,
                            vint8 in_validation_number)
{
  vMatrix<vint8> in_training_triples = ChooseTriples(in_training_number);
  vMatrix<vint8> in_validation_triples = ChooseTriples(in_validation_number);
  vint8 result = InitializeB(in_training_triples, in_validation_triples);
  return result;
}


vint8 vBnn::InitializeB(vMatrix<vint8> in_training_triples,
                            vMatrix<vint8> in_validation_triples)
{
  training_triples_matrix = in_training_triples;
  training_triples = training_triples_matrix.Matrix2();
  training_triple_number = training_triples_matrix.Rows();

  validation_triples_matrix = in_validation_triples;
  validation_triples = validation_triples_matrix.Matrix2();
  validation_triple_number = validation_triples_matrix.Rows();
  
  // In this line we are making a choice, that the number of 
  // test triples (which is not specified anywhere else) will be
  // equal to 10000
  test_triples_matrix = ChooseTestTriples(10000);
  test_triples = test_triples_matrix.Matrix2();

  InitialWeights();
  training_margins_matrix = vMatrix<float>(1, training_triple_number);
  training_margins = training_margins_matrix.Matrix();
  validation_margins_matrix = vMatrix<float>(1, validation_triple_number);
  validation_margins = validation_margins_matrix.Matrix();
  function_enter_value(&training_margins_matrix, (float) 0);
  function_enter_value(&validation_margins_matrix, (float) 0);
  alpha_limits_matrix = ComputeAlphaLimits(training_triples_matrix);
  alpha_limits = alpha_limits_matrix.Matrix2();

  test_confusion = vMatrix<float>(classes+1, classes+1);
  test_triple_confusion = vMatrix<float>(classes+1, classes+1);
  function_enter_value(&test_confusion, (float) 0);
  function_enter_value(&test_triple_confusion, (float) 0);
  initial_factors = float_matrix(&training_factors_matrix);

  return 1;
}


// chooses a set of triples, suitable for use as a training or 
// validation set.
vMatrix<vint8> vBnn::ChooseTriples(vint8 number)
{
  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();
  long i;
  for (i = 0; i < number; i++)
  {
    // Pick a random q.
    vint8 q = function_random_vint8(0, training_number - 1);
    while (active_training (q) == 0)
    {
      q = function_random_vint8(0, training_number - 1);
    }

    // Get the class of q.
    vint8 q_class = training_labels[q];

    // Pick a != q so it is in the same class as q.
    // Keep choosing random objects from q_class until
    // we pick one that is not equal to q. As a sanity check,
    // we ensure that there are at least two training objects
    // for that class.
    vint8 a;
    vint8 size = class_sizes[q_class];
    if (size < 2)
    {
      exit_error("Error: class_sizes[%li] = %li\n",
                      q_class, size);
    }
    while(1)
    {
      // Choose a random object from q_class
      vint8 index = function_random_vint8(0, size - 1);
      // Set a equal to this tentative value.
      a = class_indices[q_class][(vector_size) index];
      // If a != q then it is a valid choice, and we are done.
      if (a != q)
      {
        break;
      }
    }

    // Now choose b to be a random training object whose class
    // is not q_class. The method I use here to choose b can
    // be very slow if almost all training objects belong to
    // q_class.
    vint8 b;
    while(1)
    {
      b = function_random_vint8(0, training_number - 1);
      vint8 b_class = training_labels[b];
      if (b_class != q_class)
      {
        break;
      }
    }

    result[i][0] = q;
    result[i][1] = a;
    result[i][2] = b;
  }

  return result_matrix;
}


// chooses a set of triples suitable for use as a test triple set.
// Here, triples (q, a, b) are chosen, such that q is a test object,
// and a,b are training objects, and so that q and a are the same
// class, whereas b is a different class.
vMatrix<vint8> vBnn::ChooseTestTriples(vint8 number)
{
  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    // Pick a random q.
    vint8 q = function_random_vint8(0, test_number - 1);
    while (active_test (q) == 0)
    {
      q = function_random_vint8(0, training_number - 1);
    }
    
    // Get the class of q.
    vint8 q_class = test_labels[q];

    // Pick a != q so it is in the same class as q.
    // Keep choosing random objects from q_class until
    // we pick one that is not equal to q. As a sanity check,
    // we ensure that there are at least two training objects
    // for that class.
    vint8 a;
    vint8 size = class_sizes[q_class];
    if (size < 2)
    {
      exit_error("Error: class_sizes[%li] = %li\n",
                      q_class, size);
    }
    while(1)
    {
      // Choose a random object from q_class
      vint8 index = function_random_vint8(0, size - 1);
      // Set a equal to this tentative value.
      a = class_indices[q_class][(vector_size) index];
      // If a != q then it is a valid choice, and we are done.
      if (a != q)
      {
        break;
      }
    }

    // Now choose b to be a random training object whose class
    // is not q_class. The method I use here to choose b can
    // be very slow if almost all training objects belong to
    // q_class.
    vint8 b;
    while(1)
    {
      b = function_random_vint8(0, training_number - 1);
      vint8 b_class = training_labels[b];
      if (b_class != q_class)
      {
        break;
      }
    }

    result[i][0] = q;
    result[i][1] = a;
    result[i][2] = b;
  }

  return result_matrix;
}



vint8 vBnn::InitialWeights()
{
  if (training_triple_number == 0)
  {
    return 1;
  }
  training_factors_matrix = vMatrix<float>(1, training_triple_number);
  training_factors = training_factors_matrix.Matrix();

  float weight = ((float) 1) / (float) training_triple_number;
  function_enter_value(&training_factors_matrix, weight);

  return 1;
}


// We need to call this function every time we choose training triples,
// to store the necessary info into alpha_limits.
vMatrix<float> vBnn::ComputeAlphaLimits(vMatrix<vint8> triples_matrix)
{
  const float max_exponent = 88.0;
  vMatrix<float> result_matrix(2, attributes);
  vArray2(float) result = result_matrix.Matrix2();
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vint8 number = triples_matrix.Rows();
  
  vint8 i;
  for (i = 0; i < attributes; i++)
  {
    // max_diff and min_diff determine the ranges of exponents
    // we will encounter.
    float max_diff = 0;
    float min_diff = 0;
    
    vint8 j;
    for (j = 0; j < number; j++)
    {
      vint8 q = triples[j][0];
      vint8 a = triples[j][1];
      vint8 b = triples[j][2];
  
      float q_value = training_vectors[q][i];
      float a_value = training_vectors[a][i];
      float b_value = training_vectors[b][i];

      float qa = vAbs(q_value - a_value);
      float qb = vAbs(q_value - b_value);
      float diff = qb - qa;
      if (diff > max_diff)
      {
        max_diff = diff;
      }
      if (diff < min_diff)
      {
        min_diff = diff;
      }
    }

    float max_max = Max(vAbs(max_diff), vAbs(min_diff));
    float adjustment = log(max_max * training_triple_number);
    if (max_diff > 0)
    {
      result[0][i] = (max_exponent - adjustment) / -max_diff;
    }
    else
    {
      result[0][i] = vTypeMin((float) 0);
    }
    if (min_diff < 0)
    {
      result[1][i] = (max_exponent - adjustment) / -min_diff;
    }
    else
    {
      result[1][i] = vTypeMax((float) 0);
    }
  }

  return result_matrix;
}



vint8 vBnn::NextSteps(vint8 steps)
{
  vint8 i;
  vint8 result;
  for (i = 0; i < steps; i++)
  {
    result = NextStep();
  }

  return result;
}


float vBnn::DistanceZ(vint8 index, float * alpha)
{
  // results[i] is what is called u_i in Schapire and Singer.
  vector<float> results((vector_size) training_triple_number);
  DistanceResults(index, &results, training_triples_matrix);
  float current_weight = CurrentDistanceWeight(index);
  float min_alpha;

  // figure out the allowable ranges for alpha, i.e. for the 
  // weight to be given to the current classifier, if it
  // is defined based on the index-th attribute.
  if (allow_negative_weights == 0)
  {
    min_alpha = Max(-current_weight, alpha_limits[0][index]);
  }
  else
  {
    min_alpha = alpha_limits[0][index];
  }

  float a = 0, z = 0;
  iterations = iterations + vas_int16((int) MinimizeZ(&results, &z, &a,
                                                        min_alpha,
                                                        alpha_limits[1][index]));
  *alpha = a;
  return z;
}


// results(i) is the product between the class label of the i-th triple
// (which is always 1, for the ICML 2004 submission), and
// the classification result of
// the classifier corresponding to index, on the i-th triple.
vint8 vBnn::DistanceResults(vint8 index, vector<float> * results,
                                vMatrix<vint8> triples_matrix)
{
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vint8 triple_number = triples_matrix.Rows();

  vint8 i;
  for (i = 0; i < triple_number; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    // label = 1 means that q is always same class as a and different
    // class than b. Based on the way we construct the training and
    // validation sets, label is always 1. Label = -1 would mean
    // that q is same class as b and different class than a.
    float label = 1;
  
    float q_dim = training_vectors[q][index];
    float a_dim = training_vectors[a][index];
    float b_dim = training_vectors[b][index];
    float distance = (vAbs(q_dim - b_dim) - vAbs(q_dim - a_dim)) * label;

    (*results)[(vector_size) i] = distance;
  }

  return 1;
}


float_matrix vBnn::distance_results(vint8 index, vMatrix<vint8> triples_matrix)
{
  vector<float> results_vector((vector_size) triples_matrix.vertical());
  DistanceResults(index, & results_vector, triples_matrix);
  float_matrix result = matrix_from_vector (& results_vector);
  return result;
}


// this function takes in the results for a given classifier, and
// it also takes as arguments the minimum and maximum values that
// should be considered as weights for that classifier. These values
// are determined in a way that guarantees that we won't run into
// numerical problems. At the same time, I should really check at
// some point whether, by using doubles, I can avoid those problems
// altogether, because maybe they are hurting classification 
// accuracy, by limiting the choices available for the weight
// of each weak classifier.
// The results are stored in *z and *a, and are the minimum z
// attained for those results, and the alpha (weight) corresponding
// to that z.
vint8 vBnn::MinimizeZ(vector<float> * results, float * z, float * a,
                          float min_alpha, float max_alpha)
{
  vint8 number = results->size();
  if (number == 0)
  {
    exit_error("Error: MinimizeZ called without objects\n");
  }

  // Check for the pathological case where all result entries
  // have the same sign.
  float first = (*results)[0];
  vint8 flag = 0;
  vint8 i;
  for (i = 1; i < number; i++)
  {
    if ((*results)[(vector_size) i] * first <= 0)
    {
      flag = 1;
      break;
    }
  }
  if (flag == 0)
  {
    exit_error("Error: perfect classifier, the code can't handle it\n");
//    *a = 1;
//    *z = Z(*a, results);
//    return 0;
  }

  float alpha = 0;
  // Check for numerically pathological cases: either Z_Prime(min_alpha) is positive,
  // or Z_Prime(max_alpha) is negative.
  if (Z_Prime(min_alpha, results) > 0)
  {
    alpha = min_alpha;
    *z = Z(alpha, results);
    *a = alpha;
    return 1;
  }  
  else if (Z_Prime(max_alpha, results) < 0)
  {
    alpha = max_alpha;
    *z = Z(alpha, results);
    *a = alpha;
    return 1;
  }

  float alpha_hi = 0, alpha_low = 0;
  float step = 0.1f;
  float dz_low = 0, dz_high = 0;

  dz_high = Z_Prime(alpha_hi, results);

  // find a value of alpha (alpha_hi) for which Z'(alpha) >= 0 
  // (to avoid infinity problems with numerical overflows)
  while(dz_high < 0)
  {
    alpha_hi = alpha_hi + step;
    if (alpha_hi > max_alpha)
    {
      alpha_hi = max_alpha;
      dz_high = Z_Prime(alpha_hi, results);
      break;
    }

    dz_high = Z_Prime(alpha_hi, results);
    step = step * (float) 2.0;
  }
    
  //  vPrint("alpha_hi = %f\n", alpha_hi);

  step = 1.0;
  // find a value of alpha (alpha_low) for which Z'(alpha) <= 0.
  while((dz_low = Z_Prime(alpha_low, results)) > 0)
  {
    alpha_low = alpha_low - step;
    if (alpha_low < min_alpha)
    {
      alpha_low = min_alpha;
      break;
    }
    step = step * (float) 2.0;
  }
//  vPrint("alpha_low = %f\n", alpha_low);

  // if dz_high is infinity, find an alpha_hi that makes it positive but finite
  step = (alpha_hi - alpha_low);
  while ((dz_high > 100) || (dz_high < 0))
  {
    step = step / 2.0f;
    if (step < .000001f)
    {
      break;
    }
   if (dz_high > 100)
    {
      alpha_hi = alpha_hi - step;
    }
    else
    {
      alpha_hi = alpha_hi + step;
    }

    dz_high = Z_Prime(alpha_hi, results);
  }


  vint8 counter = 0;
  const float adjustment = (float) 0.01;
  // Now use binary search to find a root of Z'.
  while(1)
  {
    // The next few lines do search using linear interpolation
    // to improve the guess and minimize the number of iterations
    float range = (dz_high - dz_low) * (float) 1.01;
    if (range < (float) 0.000001) break;

    float high_weight = -dz_low / range;
    float low_weight = dz_high / range;
    if (high_weight < low_weight) 
    {
      high_weight = high_weight + adjustment;
    }
    else
    {
      low_weight = low_weight + adjustment;
    }

    alpha = low_weight * alpha_low + high_weight * alpha_hi;
    if ((alpha > max_alpha) || (alpha < min_alpha))
    {
      exit_error("Error, alpha = %f, min_alpha = %f, max_alpha = %f\n",
                      alpha, min_alpha, max_alpha);
    }

    // The next line is good for simple binary search
//    alpha = (alpha_hi + alpha_low) / 2.0;

    if ((alpha_hi - alpha < 0.000001) || 
        (alpha - alpha_low < 0.000001))
    {
      break;
    }


//    vPrint("alpha - alpha_low = %f, alpha_hi - alpha = %f\n",
//            alpha - alpha_low, alpha_hi - alpha);
//    vPrint("(%li, %li, %li)\n", 
//            training_triples[53][0], training_triples[53][1], training_triples[53][2]);
//    vint8 check1 = (alpha == alpha_low);
//    vint8 check2 = (alpha == alpha_hi);
//  vPrint("check1 = %li, check2 = %li\n", check1, check2);
    float z_prime = Z_Prime(alpha, results);

    if (z_prime == 0) 
    {
      break;
    }
    else if (z_prime < 0)
    {
      alpha_low = alpha;
      dz_low = z_prime;
    }
    else
    {
      alpha_hi = alpha;
      dz_high = z_prime;
    }

//    vPrint("%li, %.15f - %.15f - %.15f, %f\n", counter, 
//            alpha_low, alpha, alpha_hi, z_prime);
//    vPrint("alpha - alpha_low = %f, alpha_hi - alpha = %f\n\n",
//            alpha - alpha_low, alpha_hi - alpha);
    counter++;
  }

  *z = Z(alpha, results);
  *a = alpha;
  return counter;
}


// this function is like the previous one, but it takes its input
// results as a matrix
vint8 vBnn::MinimizeZ(float_matrix results, float * z, float * a,
                          float min_alpha, float max_alpha)
{
  vector<float> results_vector;
  vector_from_matrix (& results, & results_vector);
  vint8 result = MinimizeZ(& results_vector, z, a, min_alpha, max_alpha);
  return result;
}

                          
float vBnn::Z(float a, vector<float> * results)
{
  long i;
  float sum = 0;
  for (i = 0; i < training_triple_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[i];
    float exp_i = vPrecomputedExp::Exp(-a * u_i);
    sum = sum + d_i * exp_i;
  }

  return sum;
}


// computes the z corresponding to a given weight and to the given
// results.
float vBnn::Z_Prime(float a, vector<float> * results)
{
  long i;
  float sum = 0;
  for (i = 0; i < training_triple_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[i];
    float exponent = -a * u_i;
    if (exponent < min_exp) min_exp = exponent;
    if (exponent > max_exp) max_exp = exponent;
    float exp_i = vPrecomputedExp::Exp(exponent);
//    float exp_i = exponent;
    sum = sum - d_i * u_i * exp_i;

    // The next line is here in order to catch a possible bug. If the 
    // exponent is too high, the sum becomes -infinity. We don't
    // want that, and it should never happen: we set alpha_limits
    // explicitly in order to avoid this case. If it still happens,
    // it gets caught here.
    if (((sum < -1)  || (sum > 1)) && ((sum / sum) == 5))
    {
      function_warning("Warning: sum = -infinity (probably)\n");
      alpha_limits_matrix.Print("alpha_limits");
      float sum2 = sum * (float) 1000000000;
      float temp = sum / sum;
      long temp2 = (sum == 0);
      long temp3 = (sum2 == 0);
      long temp4 = (sum < -1);
      vPrint("a = %f, i = %li, u_i = %f, exponent = %f\n",
              a, i, u_i, exponent);
      vPrint("exp_i = %f, d_i = %f, sum = %f, sum2 = %f, temp = %f\n",
              exp_i, d_i, sum, sum2, temp);
      vPrint("temp2 = %li, temp3 = %li, temp4 = %li\n", temp2, temp3, temp4);
    }
  }

  return sum;
}


// assuming that we picked the weak classifier giving these
// results, and with the specified z and alpha, update the 
// training weights (weights of the training triples).
float vBnn::UpdateWeights(vector<float> * results, 
                               float z, float a)
{
  if (z <= 0)
  {
    exit_error("Error: non-positive z in UpdateWeights: %f\n", z);
  }

  long i;
  for (i = 0; i < training_triple_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[i];
    float exp_i = vPrecomputedExp::Exp(-a * u_i);
    training_factors[i] = d_i * exp_i / z;
  }

  return z;
}


// Computes errors and margins for the last weak classifier that
// we chose. We don't specify the classifier directly, but we 
// specify the results obtained from that classifier, and its
// weight (alpha). 
float vBnn::ComputeLastError(float alpha, vector<float> * results)
{
  if (training_triple_number == 0) return 0;
  float sum = 0;
  long i;
  float accuracy = 0;
  for (i = 0; i < training_triple_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[i];
    sum = sum + d_i * u_i;
    if (u_i > 0) accuracy = accuracy + d_i;
    else if (u_i ==0) accuracy = accuracy + d_i / 2;
  }

  last_correlation = sum;
  last_et = ((float) 1.0 - last_correlation) / (float) 2.0;
  last_error = (float) 1.0 - accuracy;
  last_scaled_correlation = alpha * sum;
  last_scaled_et = ((float) 1.0 - last_scaled_correlation) / (float) 2.0;
  return last_et;
}


// ComputeTrainingError and ComputeValidationError take as input
// the results of the last weak classifier that we chose, on 
// the training or validation triples. These functions update
// training_margins_matrix and validation_margins_matrix, and compute
// the training and validatin error.
float vBnn::ComputeTrainingError(vector<float> * results, float weight)
{
  UpdateMargins(training_triples_matrix, results, training_margins_matrix,
                &training_error, &training_margin, weight);
  
  return training_margin;
}


float vBnn::ComputeValidationError(vector<float> * results, float weight)
{
  float validation_margin = 0;

  UpdateMargins(validation_triples_matrix, results, validation_margins_matrix,
                &validation_error, &validation_margin, weight);
  
  return training_margin;
}




float vBnn::ComputeTestError()
{
  if (test_number < 1)
  {
    exit_error("Error: ComputeTestError called, test_number = %li\n",
                    test_number);
  }

  // First, compute test error on objects.

  // counter will count misclassified objects.
  float counter = 0;
  function_enter_value(&test_confusion, (float) 0);
  vArray2(float) confusion = test_confusion.Matrix2();
  vArray2(float) test_distances = test_distances_matrix.Matrix2();

  long i;
  for (i = 0; i < test_number; i++)
  {
    v3dMatrix<float> distances = copy_horizontal_line(&test_distances_matrix, i);
    vint8 nn_row = 0, nn_col = 0;
    float nn_distance = function_image_minimum3(&distances, &nn_row, &nn_col);
    vint8 object_class = test_labels[i];
    vint8 nn_class = training_labels[nn_col];
    confusion[object_class][nn_class] = confusion[object_class][nn_class] + (float) 1;
    if (nn_class != object_class)
    {
      counter = counter + 1;
    }
  }

  test_error = counter / (float) test_number;
  
  // Now compute test_error on triples;
  counter = 0;
  function_enter_value(&test_triple_confusion, (float) 0);
  confusion = test_triple_confusion.Matrix2();
  vint8 number = test_triples_matrix.Rows();
  for (i = 0; i < number; i++)
  {
    vint8 q = test_triples[i][0];
    vint8 a = test_triples[i][1];
    vint8 b = test_triples[i][2];

    vint8 q_class = test_labels[q];
    vint8 a_class = training_labels[a];
    vint8 b_class = training_labels[b];

    // check q_class = a_class != b_class
    if ((q_class != a_class) || (q_class == b_class))
    {
      exit_error("Error: q_class = %li, a_class = %li, b_class = %li\n",
                      q_class, a_class, b_class);
    }

    float qa_distance = test_distances[q][a];
    float qb_distance = test_distances[q][b];
    if (qa_distance < qb_distance)
    {
      confusion[q_class][a_class] = confusion[q_class][a_class] + (float) 1;
    }
    else if (qb_distance < qa_distance)
    {
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 1;
      counter = counter + 1;
    }
    else // if the two distances are equal
    {
      confusion[q_class][a_class] = confusion[q_class][b_class] + (float) 0.5;
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 0.5;
      counter = counter + (float) 0.5;
    }
  }
  
  test_triple_error = counter / (float) number;
  return test_error;
}


// Updates the entries of test_distances_matrix, to add 
// distances in the given feature (indexed by index), weighted
// by alpha.
vint8 vBnn::UpdateDistances(vint8 index, float alpha)
{
  vArray2(float) distances = test_distances_matrix.Matrix2();
  vint8 row, col;
  for (row = 0; row < test_number; row++)
  {
    float test_value = test_vectors[row][index];
    for (col = 0; col < training_number; col++)
    {
      float training_value = training_vectors[col][index];
      float diff = test_value - training_value;
      float weighted = alpha * vAbs(diff);
      distances[row][col] = distances[row][col] + weighted;
    }
  }

  return 1;
}


// UpdateMargins is used in ComputeTrainingError and ComputeValidationError
// to do the actual work, which is pretty similar in both cases, it
// just works on different data.
vint8 vBnn::UpdateMargins(vMatrix<vint8> triples_matrix,
                              vector<float> * results,
                              vMatrix<float> margins_matrix,
                              float * errorp, float * marginp, 
                              float dimension_weight)
{
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray(float) margins = margins_matrix.Matrix();
  vint8 number = triples_matrix.Rows();
  float weight = (float) 1.0 / (float) number;
  vint8 rounds = StepsDone();
  vint8 last_index = rounds - 1;
  float error = 0, margin = 0;

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q_index = triples[i][0];
    vint8 a_index = triples[i][1];
    vint8 b_index = triples[i][2];

    float label = 1;

    float current_estimate = dimension_weight * (*results)[(vector_size) i];

    // Add the contribution of the last chosen reference images to 
    // the contributions of all previous reference images.
    float total_estimate = margins[i] + current_estimate;
    margins[i] = total_estimate;

    if (total_estimate == 0) 
    {
      error = error + weight / (float) 2.0;
    }
    else if (total_estimate < 0)
    {
      error = error + weight;
    }
    margin = margin + weight * total_estimate;
  }

  *errorp = error;
  *marginp = margin;
  return 1;
}


// functions that provide read-only access to member variables or 
// parts of member variables.
vint8 vBnn::Attributes()
{
  return attributes;
}

float vBnn::TrainingError()
{
  return training_error;
}

float vBnn::TrainingMargin()
{
  return training_margin;
}

float vBnn::ValidationError()
{
  return validation_error;
}

float vBnn::LastError()
{
  return last_error;
}

float vBnn::LastEt()
{
  return last_et;
}

float vBnn::LastCorrelation()
{
  return last_correlation;
}


float vBnn::StepZ(vint8 step)
{
  if ((step < 0) || (step >= (vint8) distance_zs.size()))
  {
    return -1;
  }
  return distance_zs[(vector_size) step];
}


float vBnn::LastZ()
{
  vint8 steps = distance_zs.size();
  if (steps == 0)
  {
    return -1;
  }
  float z = distance_zs[(vector_size) steps - 1];
  return z;
}


float vBnn::StepTrainingError(vint8 step)
{
  if ((step < 0) || (step >= (vint8) training_errors.size()))
  {
    return -1;
  }
  return training_errors[(vector_size) step];
}


float vBnn::StepValidationError(vint8 step)
{
  if ((step < 0) || (step >= (vint8) validation_errors.size()))
  {
    return -1;
  }
  return validation_errors[(vector_size) step];
}


// For debugging: return the sum of the weights of all training triples.
// weights should sum up to 1.
float vBnn::SumWeights()
{
  float sum = 0;
  vint8 i;
  for (i = 0; i < training_triple_number; i++)
  {
    sum = sum + training_factors[i];
  }
  
  return sum;
}


// this function takes in reference_indices and reference_weights,
// which typically represent the weak classifiers chosen at each
// training step and the associated weights. The outputs (stored
// into cleaned_indices and cleaned_factors) are cleaned-up versions,
// in which no index (corresponding to a dimension, aka feature, aka
// attribute) is repeated twice. object number is useful in order
// to allocate memory for a scratch array that is used in the computations.
// At least right now, this is identical to class_BoostMap::CleanUp.
vint8 vBnn::CleanUp(vint8 attributes, vector<vint8> * distance_indices,
                        vector<float> * distance_factors,
                        vector<vint8> * cleaned_indices,
                        vector<float> * cleaned_factors)
{
  vMatrix<double> scratch_matrix(1, attributes);
  vArray(double) scratch = scratch_matrix.Matrix();
  cleaned_indices->erase(cleaned_indices->begin(), cleaned_indices->end());
  cleaned_factors->erase(cleaned_factors->begin(), cleaned_factors->end());
  vint8 number = distance_indices->size();

  long i;
  // Zero out all weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*distance_indices)[(vector_size) i];
    // We allow negative indices to signify "don't care". This is useful
    // in cases where the indices come from TripleClassifier objects, where
    // some objects correspond to a single reference object, and some objects
    // correspond to a pair of pivot objects
    if (index < 0) 
    {
      continue;
    }
    scratch[index] = (double) 0;
  }

  // Add up weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*distance_indices)[(vector_size) i];
    if (index < 0) continue;
    double weight = (*distance_factors)[i];
    scratch[index] = scratch[index] + weight;
  }

  // Get the combined weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*distance_indices)[(vector_size) i];
    if (index < 0) continue;
    double weight = scratch[index];
    if (vAbs(weight) > 0.000001)
    {
      cleaned_indices->push_back(index);
      cleaned_factors->push_back((float) weight);
      scratch[index] = 0;
    }
  }

  long number2 = cleaned_indices->size();
  vPrint("Picked %li of %li classifiers\n",
          number2, number);
  return 1;
}


// this version of CleanUp is more appropriate for vBnnqs. In principle
// we should check for every pair of classifiers to see if they 
// match. However, two classifiers match only if their multipliers are
// identical and their attribute indices also match, and this is 
// a bit more complicated to test for. So, at least in the initial
// implementation, we only combine classifiers that have no multipliers.
vint8 vBnn::CleanUp(vint8 attributes, vector<vQsClassifier> * step_classifiers,
                   vector<vQsClassifier> * unique_classifiers)
{
  vMatrix<double> scratch_matrix(1, attributes);
  vArray(double) scratch = scratch_matrix.Matrix();
  unique_classifiers->erase(unique_classifiers->begin(), unique_classifiers->end());
  vint8 number = step_classifiers->size();

  vint8 i;

  // Zero out all weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*step_classifiers)[(vector_size) i].index;
    scratch[index] = (double) 0;
  }

  // Add up weights
  for (i = 0; i < number; i++)
  {
    if ((*step_classifiers)[(vector_size) i].multipliers->size() != 0) 
    {
      continue;
    }
    vint8 index = (*step_classifiers)[(vector_size) i].index;
    double weight = (*step_classifiers)[(vector_size) i].weight;
    scratch[index] = scratch[index] + weight;
  }

  // Get the combined weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*step_classifiers)[(vector_size) i].index;
    if ((*step_classifiers)[(vector_size) i].multipliers->size() != 0) 
    {
      unique_classifiers->push_back((*step_classifiers)[(vector_size) i]);
    }

    else
    {
      float weight = (float) scratch[index];

      if (weight > 0)
      {
        vQsClassifier classifier(index, weight);
        unique_classifiers->push_back(classifier);
        scratch[index] = 0;
      }
    }
  }

  vint8 number2 = unique_classifiers->size();
  vPrint("Picked %li of %li classifiers\n",
          number2, number);
  return 1;
}



// result(i) is the distance between object and the i-th training object.
// dimensions and weights specify the dimensions to be used for 
// measuring distances, and the weight of each dimension.
vMatrix<float> vBnn::TestDistances(v3dMatrix<float> & object, 
                                         vector<vint8> * dimensions,
                                         vector<float> * weights)
{
  vint8 size = object.Size();
  if (size != attributes)
  {
    exit_error("Error in TestDistances: size = %li (%li)\n",
                    size, attributes);
  }

  vMatrix<float> result_matrix(1, training_number);
  vArray(float) result = result_matrix.Matrix();
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    v3dMatrix<float> training_object = copy_horizontal_line(&training_vectors_matrix, i);
    result[i] = ClassifierDistance(object, training_object, dimensions, weights);
  }

  return result_matrix;
}

// Here instead of passing in an object we just pass in an index into 
// vectors_matrix. Typically vectors_matrix is the training vectors
// or the test vectors.
vMatrix<float> vBnn::TestDistances4(vint8 index, 
                                     v3dMatrix<float> & vectors_matrix,
                                     vector<vint8> * dimensions,
                                     vector<float> * weights)
{
  vint8 number = dimensions->size();
  if (number == 0)
  {
    return vMatrix<float>();
  }
  vArray2(float) vectors = vectors_matrix.Matrix2(0);
  vArray(float) object = vectors[index];
  vMatrix<float> result_matrix(1, training_number);
  vArray(float) result = result_matrix.Matrix();

  vint8 i, j;
  for (i = 0; i < training_number; i++)
  {
    float distance = 0;
    for (j = 0; j < number; j++)
    {
      vint8 dimension_index = (*dimensions)[(vector_size) j];
      float weight = (*weights)[(vector_size) j];
      float value1 = object[dimension_index];
      float value2 = training_vectors[i][dimension_index];
      float diff = weight * vAbs(value1 - value2);
      distance = distance + diff;
    }
    result[i] = distance;
  }

  return result_matrix;
}


// Same as TestDistances2, but here we compute L2 distances.
vMatrix<float> vBnn::TestDistances2b(vint8 index, 
                                          v3dMatrix<float> & vectors_matrix,
                                          vector<vint8> * dimensions,
                                          vector<float> * weights)
{
  vArray2(float) vectors = vectors_matrix.Matrix2(0);
  vArray(float) object = vectors[index];
  vMatrix<float> result_matrix(1, training_number);
  vArray(float) result = result_matrix.Matrix();

  vint8 number = dimensions->size();
  vint8 i, j;
  for (i = 0; i < training_number; i++)
  {
    float distance = 0;
    for (j = 0; j < number; j++)
    {
      vint8 dimension_index = (*dimensions)[(vector_size) j];
      float weight = (*weights)[(vector_size) j];
      float value1 = object[dimension_index];
      float value2 = training_vectors[i][dimension_index];
      float diff = value1 - value2;
      distance = distance + weight * diff * diff;
    }
    result[i] = sqrt(distance);
  }

  return result_matrix;
}


// Similar to TestDistances2, but here we only compute distances to
// objects that belong to the class specified by class_id.
vMatrix<float> vBnn::TestDistances5(vint8 index, v3dMatrix<float> & vectors_matrix,
                                          vint8 class_id,
                                          vector<vint8> * dimensions,
                                          vector<float> * weights)
{
  vArray2(float) vectors = vectors_matrix.Matrix2(0);
  vArray(float) object = vectors[index];
  long class_size = class_indices[class_id].size();
  vMatrix<float> result_matrix(1, class_size);
  vArray(float) result = result_matrix.Matrix();

  long number = dimensions->size();
  long i, j;
  for (i = 0; i < class_size; i++)
  {
    float distance = 0;
    vint8 index = class_indices[class_id][(vector_size) i];
    for (j = 0; j < number; j++)
    {
      vint8 dimension_index = (*dimensions)[j];
      float weight = (*weights)[j];
      float value1 = object[dimension_index];
      float value2 = training_vectors[index][dimension_index];
      float diff = weight * vAbs(value1 - value2);
      distance = distance + diff;
    }
    result[i] = distance;
  }

  return result_matrix;
}

  
// Returns the distance between object1 and object2. How to measure the
// distance is specified by dimensions (which dimensions to use) and
// weights (how much to weigh each dimension).
float vBnn::ClassifierDistance(v3dMatrix<float> & object1, 
                                    v3dMatrix<float> & object2,
                                    vector<vint8> * dimensions, 
                                    vector<float> * weights)
{
  vint8 size1 = object1.Size();
  vint8 size2 = object2.Size();
  if (size1 != size2)
  {
    exit_error("Error: size1 = %li, size2 = %li\n", size1, size2);
  }

  vArray(float) data1 = object1.Matrix();
  vArray(float) data2 = object2.Matrix();
  vint8 number = dimensions->size();
  float result = 0;
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 dimension = (*dimensions)[(vector_size) i];
    float weight = (*weights)[(vector_size) i];
    if ((dimension < 0) || (dimension >= size1))
    {
      exit_error("Error: dimension = %li, size = %li\n",
                      dimension, size1);
    }
    float value1 = data1[dimension];
    float value2 = data2[dimension];
    float diff = weight * vAbs(value1 - value2);
    result = result + diff;
  }

  return result;
}


// Here, index1 is an index into a row of vectors1, and index2 is an
// index into a row of vectors2.
float vBnn::ClassifierDistance2(vint8 index1, v3dMatrix<float> & vectors1_matrix,
                                     vint8 index2, v3dMatrix<float> & vectors2_matrix,
                                     vector<vint8> * dimensions, 
                                     vector<float> * weights)
{
  float distance = 0;
  vArray2(float) vectors1 = vectors1_matrix.Matrix2(0);
  vArray(float) object1 = vectors1[index1];
  vArray2(float) vectors2 = vectors2_matrix.Matrix2(0);
  vArray(float) object2 = vectors2[index2];

  vint8 number = dimensions->size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 dimension_index = (*dimensions)[(vector_size) i];
    float weight = (*weights)[(vector_size) i];
    float value1 = object1[dimension_index];
    float value2 = object2[dimension_index];
    float diff = weight * vAbs(value1 - value2);
    distance = distance + diff;
  }
  return distance;
}

  
// This function does the same job as AnalyzeTriples, but for test
// objects.
vint8 vBnn::AnalyzeTestTriples()
{
  // First, initialize some of the matrices.
  // How large should they be? We allocate at most 
  // 10MB for bad_bs and 10MB for good_bs
  const vint8 max_bytes = 10000000; // 10 million
  vint8 bytes_per_triple = sizeof(vint8) * 3;
  vint8 max_triples = max_bytes / bytes_per_triple;
  vint8 triples_per_q = max_triples / test_number;
  vint8 cols = Min(training_number, triples_per_q);

  bad_bs = vMatrix<vint8>(test_number, cols);
  good_bs = vMatrix<vint8>(test_number, cols);
  tie_bs = vMatrix<vint8>(test_number, 10);
  bad_numbers = vMatrix<vint8>(1, test_number);
  good_numbers = vMatrix<vint8>(1, test_number);
  tie_numbers = vMatrix<vint8>(1, test_number);
  good_a_numbers = vMatrix<vint8>(1, test_number);
  same_class_nn = vMatrix<vint8>(1, test_number);
  other_class_nn = vMatrix<vint8>(1, test_number);

  vArray2(vint8) bad_bsm = bad_bs.Matrix2();
  vArray2(vint8) good_bsm = good_bs.Matrix2();
  vArray2(vint8) tie_bsm = tie_bs.Matrix2();
  vArray(vint8) bad_numbersm = bad_numbers.Matrix();
  vArray(vint8) good_numbersm = good_numbers.Matrix();
  vArray(vint8) tie_numbersm = tie_numbers.Matrix();
  vArray(vint8) good_a_numbersm = good_a_numbers.Matrix();
  vArray(vint8) same_class_nnm = same_class_nn.Matrix();
  vArray(vint8) other_class_nnm = other_class_nn.Matrix();

  vector<vint8> good_qsv, bad_qsv;

  vMatrix<vint8> temp_good_matrix(1, training_number);
  vMatrix<vint8> temp_bad_matrix(1, training_number);
  vMatrix<vint8> temp_tie_matrix(1, training_number);
  vArray(vint8) temp_good = temp_good_matrix.Matrix();
  vArray(vint8) temp_bad = temp_bad_matrix.Matrix();
  vArray(vint8) temp_tie = temp_tie_matrix.Matrix();

  bad_triples = 0;
  tie_triples = 0;

  vint8 q;
  for (q = 0; q < test_number; q++)
  {
    // Get distances from q to all objects.
    v3dMatrix<float> q_distancesp = copy_horizontal_line(&test_distances_matrix, q);
    vArray(float) q_distances = q_distancesp.Matrix();
    vint8 q_class = test_labels[q];
    vint8 q_size = class_indices[q_class].size();
    
    float max_distance_limit = function_image_maximum(&q_distancesp) * (float) 2.0 + (float) 1.0;

    // find the same-class and different-class nearest neighbors of q.
    float same_min_distance = max_distance_limit;
    float other_min_distance = max_distance_limit;
    vint8 same_min_index = -1;
    vint8 other_min_index = -1;
    vint8 i;
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      if (training_labels[i] == q_class)
      {
        if (distance < same_min_distance)
        {
          same_min_distance = distance;
          same_min_index = i;
        }
      }
      else
      {
        if (distance < other_min_distance)
        {
          other_min_distance = distance;
          other_min_index = i;
        }
      }
    }

    if ((same_min_index == -1) || (other_min_index == -1))
    {
      exit_error("Error: impossible\n");
    }

    same_class_nnm[q] = same_min_index;
    other_class_nnm[q] = other_min_index;
    
    vint8 good_counter = 0, bad_counter = 0;
    vint8 tie_counter = 0, good_a_counter = 0;
    // now, count good neighbors and bad neighbors.
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      if (training_labels[i] == q_class)
      {
        if (distance < other_min_distance)
        {
          good_a_counter = good_a_counter + 1;
        }
        continue;
      }

      // We get here if i is other class than q.
      if (distance < same_min_distance)
      {
        temp_bad[bad_counter] = i;
        bad_counter = bad_counter + 1;
      }
      else if (distance == same_min_distance)
      {
        temp_tie[tie_counter] = i;
        tie_counter = tie_counter + 1;
      }
      else
      {
        temp_good[good_counter] = i;
        good_counter = good_counter + 1;
      }
    }

    good_numbersm[q] = good_counter;
    bad_numbersm[q] = bad_counter;
    tie_numbersm[q] = tie_counter;
    good_a_numbersm[q] = good_a_counter;
    bad_triples = bad_triples + bad_counter;
    tie_triples = tie_triples + tie_counter;
    if (good_counter > 0)
    {
      good_qsv.push_back(q);
    }
    if (bad_counter > 0)
    {
      bad_qsv.push_back(q);
    }

    // Sample good other-class neighbors of q;
    vint8 sample_number = Min(cols, good_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, good_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 good_b = temp_good[pick];
        good_bsm[q][i] = good_b;
      }
    }

    // Sample bad other-class neighbors of q.
    sample_number = Min(cols, bad_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, bad_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 bad_b = temp_bad[pick];
        bad_bsm[q][i] = bad_b;
      }
    }

    // Sample tie other-class neighbors of q.
    vint8 tie_cols = tie_bs.Cols();
    sample_number = Min(tie_cols, tie_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, tie_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 tie_b = temp_tie[pick];
        tie_bsm[q][i] = tie_b;
      }
    }
  }

  good_qs = matrix_from_vector(&good_qsv);
  bad_qs = matrix_from_vector(&bad_qsv);

  return 1;
}


  
// This function is an alternative for regenerating triples, so that they are 
// more representative (a is close to q, b is importance-sampled).
// - pick q randomly.
// - choose a random rank r between 1 and rank maximum.
// - set a = r-nearest-neighbor of q among q-class objects.
// - if there are different-class objects b between q and a, choose
// a b1 between q and a, and a b2 further away to q than a. Form both
// triples (q, a, b1) and (q, a, b2), but weigh them in a way that
// reflects the sampling bias we used. That is, if there are t1 
// different-class objects between a and q and a total of t2 different-class
// objects, (q, a, b1) gets a weight of t1 and (q, a, b2) gets a weight 
// of t2 - t1.
// - if no different-class objects are between q and a, choose a b 
// that is different-class than q. If t2 such different-class choices exist,
// the weight of (q, a, b) is t2.
// - After we have picked enough triples, normalize weights to 1.
vMatrix<vint8> vBnn::ResampleTriples2(vint8 number, 
                                      vMatrix<float> weights_matrix)
{
  // If we get over 40000 objects, I should check to make sure the code
  // can handle them, I'm not sure it can right now.
  if (training_number > 40000)
  {
    exit_error("Error: code can't handle %li > 40000 training objects\n",
                    training_number);
  }

  const vint8 rank_limit = 1;
  if (weights_matrix.Size() != number)
  {
    exit_error("\nError: weights_matrix.Size() = %li, number = %li\n",
                    weights_matrix.Size(), number);
  }

  vArray(float) weights = weights_matrix.Matrix();

  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();

  AnalyzeTriples();
  vint8 good_cols = good_bs.Cols();
  vint8 bad_cols = bad_bs.Cols();
  vector<vint8> bad_numbersv;
  vector_from_matrix(&bad_numbers, &bad_numbersv);
  vint8 max_value = function_image_maximum(&bad_numbers);
  vPrint("max_value = %li\n", max_value);

  vint8 cols = bad_bs.Cols();
  if (max_value > cols)
  {
    vPrint("Warning: max_value = %li, cols = %li\n", 
            max_value, cols);
  }

  vint8 half1 = number / 2;
  vint8 half2 = number - half1;
  vint8 good_q_number = good_qs.Size();
  vint8 bad_q_number = bad_qs.Size();
  float good_weight_sum = 0, bad_weight_sum = 0;

  // First, choose the "good" triples (i.e. q, same-class nn of q,
  // and a b that is farther away).
  vint8 i;
  for (i = 0; i < half1; i++)
  {
    // Choose a q that can form a good triple.
    vint8 q_index = function_random_vint8(0, good_q_number-1);
    vint8 q = good_qs(q_index);
    vint8 good_number = good_numbers(q);

    // a is the same-class nearest neighbor of q.
    vint8 a = same_class_nn(q);

    // b is a randomly chosen object that is "good" for q.
    vint8 b_choices = good_number;
    if (b_choices > good_cols)
    {
      b_choices = good_cols;
    }

    vint8 b_pick = function_random_vint8(0, b_choices-1);
    vint8 b = good_bs(q, b_pick);
    result[i][0] = q;
    result[i][1] = a;
    result[i][2] = b;
//    weights[i] = (float) good_number;
    weights[i] = (float) 1.0;
    good_weight_sum = good_weight_sum + weights[i];
  }

  // Now, choose the "bad" triples (i.e. q, same-class nn of q,
  // and a b that is closer away).
  for (i = 0; i < half2; i++)
  {
    // Choose a q that can form a bad triple.
    vint8 q_index = function_random_vint8(0, bad_q_number-1);
    vint8 q = bad_qs(q_index);
    vint8 bad_number = bad_numbers(q);

    // a is the same-class nearest neighbor of q.
    vint8 a = same_class_nn(q);

    // b is a randomly chosen object that is "bad" for q.
    vint8 b_choices = bad_number;
    if (b_choices > bad_cols)
    {
      b_choices = bad_cols;
    }

    vint8 b_pick = function_random_vint8(0, b_choices-1);
    vint8 b = bad_bs(q, b_pick);
    vint8 index = half1 + i;
    result[index][0] = q;
    result[index][1] = a;
    result[index][2] = b;
    weights[index] = (float) 1.0 / bad_number;
//    weights[index] = (float) 1;
    bad_weight_sum = bad_weight_sum + weights[index];
  }

  // Now, normalize weights. First, figure out how many
  // good triples and bad triples exist.
  float good_triples = 0, bad_triples = 0;
  for (i = 0; i < training_number; i++)
  {
    good_triples = good_triples + good_numbers(i);
    bad_triples = bad_triples + bad_numbers(i);
  }

  // Now figure out the normalizing factors.
  float total_triples = good_triples + bad_triples;
//  float new_good_sum = good_triples / total_triples;
//  float new_bad_sum = bad_triples / total_triples;
  float new_good_sum = (float) 0.7;
  float new_bad_sum = (float) 0.3;
  float good_factor = new_good_sum / good_weight_sum;
  float bad_factor = new_bad_sum / bad_weight_sum;

  float temp1 = 0;
  for (i = 0; i < half1; i++)
  {
    weights[i] = weights[i] * good_factor;
    temp1 = temp1 + weights[i];
  }
  vPrint("temp1 = %f\n", temp1);
  float temp2 = 0;
  for (i = half1; i < number; i++)
  {
    weights[i] = weights[i] * bad_factor;
    temp2 = temp2 + weights[i];
  }
  vPrint("temp2 = %f\n", temp2);
  vPrint("temp1 + temp2 = %f\n", temp1 + temp2);
  vPrint("good_triples = %li, bad_triples = %li\n",
          round_number(good_triples), round_number(bad_triples));
  vPrint("good qs = %li, bad_qs = %li\n",
          good_qs.Size(), bad_qs.Size());

  return result_matrix;
}




// This function is used to look into the contribution that each training
// object makes into the value of z. I want to check if a few objects 
// possibly make too large a contribution on z. Here we measure the
// contribution that will be made by each object if we choose the
// given dimension in the next step.
vMatrix<float> vBnn::SortZContributions(vint8 dimension)
{
  if (training_triple_number == 0)
  {
    return vMatrix<float>();
  }

  if ((dimension < 0) || (dimension >= attributes))
  {
    return vMatrix<float>();
  }

  vector<float> results((vector_size) training_triple_number);
  DistanceResults(dimension, &results, training_triples_matrix);
  float z = 0, alpha = 0;
  MinimizeZ(&results, &z, &alpha, 
            alpha_limits[0][dimension], alpha_limits[1][dimension]);

  vector<class_couple> contributions((vector_size) training_triple_number);
  long i;
  float sum = 0;
  for (i = 0; i < training_triple_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = results[i];
    float exp_i = vPrecomputedExp::Exp(-alpha * u_i);
    float contribution = d_i * exp_i;
    contributions[i].value = contribution;
    contributions[i].object = (void *) i;
    sum = sum + contribution;
  }

  // Sum should equal z.
  vPrint("alpha = %f, z = %f, sum = %f\n", alpha, z, sum);
  std::sort(contributions.begin(), contributions.end(), couple_less());
  vMatrix<float> result(2, training_triple_number);
  for (i = 0; i < training_triple_number; i++)
  {
    result(0, i) = contributions[i].value;
    result(1, i) = (float) ((long) contributions[i].object);
  }

  triple_temp = result;
  return result;
}


vint8 vBnn::SetAllowNegativeWeights(vint8 in_value)
{
  allow_negative_weights = in_value;
  return 1;
}


vint8 vBnn::SetSubjectIds(vMatrix<vint8> in_subject_ids)
{
  // If we get a 1x1 subject_ids matrix, that is just an 
  // uninitialized matrix.
  if (in_subject_ids.valid() <= 0)
  {
    return 0;
  }
  if (in_subject_ids.Size() != training_number)
  {
    function_warning("Warning: Bad subject ids, size = %li\n",
              in_subject_ids.Size());
    return 0;
  }

  vPrint("Setting subject ids:\n");
  subject_ids_matrix = in_subject_ids;
  subject_ids = subject_ids_matrix.Matrix();
  subject_ids_set = 1;
  return 1;
}


vMatrix<vint8> vBnn::BadQs()
{
  return bad_qs;
}


vMatrix<vint8> vBnn::GoodQs()
{
  return good_qs;
}


vMatrix<vint8> vBnn:: BadNumbers()
{
  return bad_numbers;
}


vMatrix<vint8> vBnn::GoodNumbers()
{
  return good_numbers;
}


vMatrix<vint8> vBnn::TieNumbers()
{
  return tie_numbers;
}


vMatrix<vint8> vBnn::GoodANumbers()
{
  return good_a_numbers;
}


vMatrix<vint8> vBnn::BadBs()
{
  return bad_bs;
}


vMatrix<vint8> vBnn::GoodBs()
{
  return good_bs;
}


vMatrix<vint8> vBnn::TieBs()
{
  return tie_bs;
}


vMatrix<vint8> vBnn::SameClassNN()
{
  return same_class_nn;
}


vMatrix<vint8> vBnn::OtherClassNN()
{
  return other_class_nn;
}


// directory where classifiers should be saved to or loaded from.
char * vBnn::BaseDirectory()
{
  char * result = vJoinPaths3(g_data_directory, "experiments", "boost_nn");
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
char * vBnn::make_round_robin_pathname(const char * filename)
{
  vint8 number = StepsDone() % 10;
  char * number_string = string_from_number(number);

  char * directory = BaseDirectory();
  char * temporary_name = vJoinPaths(directory, filename);
  char * pathname = vMergeStrings4(temporary_name, "_", 
                                     number_string, ".txt");

  delete_pointer(number_string);
  delete_pointer(temporary_name);
  vdelete2(directory);
  return pathname;
}


char * vBnn::make_pathname(const char * filename)
{
  char * directory = BaseDirectory();
  char * temporary_name = vJoinPaths(directory, filename);
  char * pathname = vMergeStrings2(temporary_name, ".txt");

  delete_pointer(temporary_name);
  vdelete2(directory);
  return pathname;
}


// Print triples, in a format (q, a, b) - (q_class, a_class, b_class)
vint8 vBnn::PrintTraining()
{
  PrintTriples(training_triples_matrix, training_labels_matrix);
  return 1;
}


vint8 vBnn::PrintValidation()
{
  PrintTriples(validation_triples_matrix, training_labels_matrix);
  return 1;
}


vint8 vBnn::PrintTestTriples()
{
  PrintTriples(test_triples_matrix, test_labels_matrix);
  return 1;
}


vint8 vBnn::PrintTriples(vMatrix<vint8> triples_matrix,
                             vMatrix<vint8> q_labels_matrix)
{
  vint8 rows = triples_matrix.Rows();
  vint8 cols = triples_matrix.Cols();
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray(vint8) q_labels = q_labels_matrix.Matrix();

  if (cols != 3)
  {
    exit_error("Error: cols of triples_matrix = %li\n", cols);
  }

  vint8 i;
  for (i = 0; i < rows; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    vint8 q_class = q_labels[q];
    vint8 a_class = training_labels[a];
    vint8 b_class = training_labels[b];
    vPrint("%5li: (%5li, %5li, %5li)  -  (%5li, %5li, %5li)\n",
            i, q, a, b, q_class, a_class, b_class);
  }

  return 1;
}


// In column i, j, put the number of triples where q and a are 
// from class i and b is from class j.
vint8 vBnn::PrintTripleStats()
{
  vMatrix<vint8> training_stats = TripleStats(training_triples_matrix,
                                              training_labels_matrix);
  training_stats.PrintInt("training_stats");
  vMatrix<vint8> validation_stats = TripleStats(validation_triples_matrix,
                                                training_labels_matrix);
  validation_stats.PrintInt("validation_stats");

  vMatrix<vint8> test_stats = TripleStats(test_triples_matrix,
                                          test_labels_matrix);
  validation_stats.PrintInt("test_triple_stats");

  return 1;
}


vMatrix<vint8> vBnn::TripleStats(vMatrix<vint8> triples_matrix,
                                      vMatrix<vint8> q_labels_matrix)
{
  vMatrix<vint8> result_matrix(classes+1, classes+1);
  vArray2(vint8) result = result_matrix.Matrix2();
  vArray(vint8) q_labels = q_labels_matrix.Matrix();

  vint8 rows = triples_matrix.Rows();
  vint8 cols = triples_matrix.Cols();
  vArray2(vint8) triples = triples_matrix.Matrix2();

  if (cols != 3)
  {
    exit_error("Error: cols of triples_matrix = %li\n", cols);
  }

  function_enter_value(&result_matrix, (vint8) 0);
  vint8 i;
  for (i = 0; i < rows; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    vint8 q_class = q_labels[q];
    vint8 a_class = training_labels[a];
    vint8 b_class = training_labels[b];
    if ((q_class != a_class) || (q_class == b_class))
    {
      vPrint("Error on triple %li: (%li, %li, %li) - (%li, %li, %li)\n",
              q, a, b, q_class, a_class, b_class);
    }

    result[a_class][b_class] = result[a_class][b_class] + 1;
  }

  return result_matrix;
}


// print the index-th training vector and the index-th 
// test vector.
vint8 vBnn::PrintVectors(vint8 row)
{
  vPrint("\n");
  if (row < 0) 
  {
    return 0;
  }
  vint8 col;

  for (col = 0; col < attributes; col++)
  {
    vPrint("Col %5i: ", col);
    if (row < training_number)
    {
      vPrint("%10.4f ", training_vectors[row][col]);
    }
    else
    {
      vPrint("           ");
    }
    if (row < test_number)
    {
      vPrint("%10.4f ", test_vectors[row][col]);
    }
    else
    {
      vPrint("           ");
    }

    vPrint("\n");
  }

  if (row < training_number)
  {
    vint8 class_id = training_labels[row];
    vPrint("training: class_id = %li\n", class_id);
  }

  if (row < test_number)
  {
    vint8 class_id = test_labels[row];
    vPrint("test:     class_id = %li\n", class_id);
  }

  return 1;
}


vint8 vBnn::PrintConfusions()
{
  test_triple_confusion.Print("test_triple_confusion");
  test_confusion.Print("test_confusion");
  return 1;
}


vint8 vBnn::PrintTrainingTriple(vint8 index)
{
  if ((index < 0) || (index >= training_triple_number))
  {
    vPrint("Bad index (%li), there are %li triples\n",
            index, training_triple_number);
  }

  vint8 q = training_triples[index][0];
  vint8 a = training_triples[index][1];
  vint8 b = training_triples[index][2];

  vint8 q_class = training_labels[q];
  vint8 a_class = training_labels[a];
  vint8 b_class = training_labels[b];

  vPrint("triple %li: (%li, %li, %li) - (%li, %li, %li)\n\n",
          index, q, a, b, q_class, a_class, b_class);

  v3dMatrix<float> q_object = copy_horizontal_line(&training_vectors_matrix, q);
  v3dMatrix<float> a_object = copy_horizontal_line(&training_vectors_matrix, a);
  v3dMatrix<float> b_object = copy_horizontal_line(&training_vectors_matrix, b);

  vMatrix<float> q_object2(&q_object);
  vMatrix<float> a_object2(&a_object);
  vMatrix<float> b_object2(&b_object);
  q_object2.Print("q");
  a_object2.Print("a");
  b_object2.Print("b");

  vPrint("margin = %f\n", training_margins[index]);

  vMatrix<float> q_distancesp = TestDistances2(q, training_vectors_matrix);
  vPrint("distance(q,a) = %f\n", q_distancesp(a));
  vPrint("distance(q,b) = %f\n", q_distancesp(b));

  return 1;
}


// Print info about training weights, or whatever else
// is stored into triple_temp.
// Print the i-th smallest weight.
vint8 vBnn::PrintWeight(vint8 index)
{
  if ((index < 0) || (index >= training_triple_number))
  {
    vPrint("Bad index (%li), there are %li triples\n",
            index, training_triple_number);
    return 0;
  }

  SortTrainingWeights();

  float weight = triple_temp(0, index);
  vint8 triple_index = round_number(triple_temp(1, index));
  vPrint("%8li: weight = %10.8f, index = %li\n", index, weight, triple_index);
  return 1;
}


vint8 vBnn::PrintWeights(vint8 period)
{
  SortTrainingWeights();
  vint8 counter = 0;

  vint8 i;
  for (i = 0; i < training_triple_number; i = i + period)
  {
    float weight = triple_temp(0, i);
    vint8 triple_index = round_number(triple_temp(1, i));
    vPrint("%8li: weight = %15.13f, index = %li\n", i, weight, triple_index);
    if (weight <= 0.0f)
    {
      counter++;
    }
  }
  vPrint("%li triples have zero weight\n", counter);

  return 1;
}


vint8 vBnn::PrintSomeWeights(vint8 start, vint8 end, vint8 period)
{
  if (start < 0) 
  {
    start = 0;
  }
  if (end >= training_triple_number)
  {
    end = training_triple_number - 1;
  }
  if (end < start)
  {
    return 1;
  }

  SortTrainingWeights();

  vint8 i;
  for (i = start; i <= end; i = i + period)
  {
    float weight = triple_temp(0, i);
    vint8 triple_index = round_number(triple_temp(1, i));
    vPrint("%8li: weight = %10.8f, index = %li\n", i, weight, triple_index);
  }

  return 1;
}


vint8 vBnn::PrintWeightSum(vint8 start, vint8 end)
{
  if (start < 0) 
  {
    start = 0;
  }
  if (end >= training_triple_number)
  {
    end = training_triple_number - 1;
  }
  if (end < start)
  {
    return 1;
  }

  if (triple_temp.Cols() != training_triple_number)
  {
    SortTrainingWeights();
  }
  float sum = 0;
  vint8 i; 
  for (i = start; i <= end; i++)
  {
    float weight= triple_temp(0, i);
    sum = sum + weight;
  }

  vPrint("sum of weights from %7li to %7li = %f\n", start, end, sum);
  return 1;
}


// Compute and print the k-nearest neighbor error on the 
// training set and the test set.
vint8 vBnn::KnnErrors(vint8 k, float * test_errorp, 
                          float * training_errorp)
{
  float test_error = KnnError(test_vectors_matrix, 
                              test_labels_matrix, k, 0);
  vPrint("%li-nn test error = %f\n", k, test_error);
  if (test_errorp != 0)
  {
    *test_errorp = test_error;
  }
  float training_error = KnnError(training_vectors_matrix, 
                                  training_labels_matrix, k, 1);
  if (training_errorp != 0)
  {
    *training_errorp = training_error;
  }
  vPrint("%li-nn training error = %f\n", k, training_error);
  return 1;
}


// Same thing, but for the test error rate, which is faster to
// compute because of test_distances_matrix;
vMatrix<float> vBnn::KnnTestErrors(vint8 max_k)
{
  vMatrix<float> result(1, max_k + 1);
  function_enter_value(&result, (float) 0);
  result(0) = 2;

  // counter will count misclassified objects.
  float counter = 0;
  vint8 threshold_index = 0;
  long i, j, k;
  vMatrix<long> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(long) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
  long negative_counter = 0;
  long tie_counter = 0;

  // here we store the label assigned to each object 
  // using knn classification for k = 1,..., max_k.
  vint8_matrix classification_labels(max_k+1, test_number);
  function_enter_value(& classification_labels, (vint8) 0);
  
  function_print("\n");
  for (i = 0; i < test_number; i++)
  {
    function_print("processing test object %li of %li\r", i+1, test_number);
    vint8 object_class = test_labels[i];

    if (active_test (i) == 0)
    {
      vint8 previous_label = previous_test_labels(i);
      if (previous_label != object_class)
      {
        for (k = 1; k <= max_k; k++)
        {
          classification_labels(k,i) = previous_label;
          result(k) = result(k) + 1.0f;
        }
      }
      continue;
    }

    v3dMatrix<float> distancesm = copy_horizontal_line(&test_distances_matrix, i);
    vArray(float) distances = distancesm.Matrix();
    
    float threshold_distance = kth_smallest_cb(max_k, &distancesm,
                                               &threshold_index);
    if (threshold_distance < 0)
    {
      negative_counter++;
    }
    
    vector<float> nn_distances;
    vector<vint8> nn_classes;

    // collect the max_k neighbors (and neighbors after that that are
    // tied to the max_k neighbor).
    for (j = 0; j < training_number; j++)
    {
      if (distances[j] <= threshold_distance)
      {
        nn_distances.push_back(distances[j]);
        nn_classes.push_back(training_labels[j]);
      }
    }

    long number = nn_distances.size();
    // Now, collect votes, and get the recognition, for each k.
    for (k = 1; k <= max_k; k++)
    {
      function_enter_value(&votes_matrix, (long) 0);
      function_enter_value(&max_distances_matrix, (float) -1000000000);
      long junk = 0;
      float k_threshold_distance = kth_smallest_ca(k, &nn_distances, &junk);
      for (j = 0; j < number; j++)
      {
        float distance = nn_distances[j];
        if (distance <= k_threshold_distance)
        {
          vint8 j_class = nn_classes[j];
          votes[j_class] = votes[j_class] + 1;
          if (max_distances[j_class] < distance)
          {
            max_distances[j_class] = distance;
          }
        }
      }

      // Find the max number of votes received:
      long max_votes = function_image_maximum(&votes_matrix);

      // Find all classes that received the max number of votes.
      vector<long> candidate_classes;
      long c;
      for (c = 1; c <= classes; c++)
      {
        if (votes[c] == max_votes)
        {
          candidate_classes.push_back(c);
        }
      }

      if (candidate_classes.size() == 0)
      {
        exit_error("Error: impossible, 0 candidate classes\n");
      }

      float min_max_distance = max_distances[candidate_classes[0]];
      long candidates = candidate_classes.size();
      // Find the min max distance among all candidates (to break ties).
      for (c = 1; c < candidates; c++)
      {
        long candidate_class = candidate_classes[c];
        float current_max = max_distances[candidate_class];
        if (current_max < min_max_distance)
        {
          min_max_distance = current_max;
        }
      }

      // Now go through candidates again, and only keep the one whose
      // max distance is min_max_distance;
      vector<long> final_candidates;
      long q_class_flag = 0;
      for (c = 0; c < candidates; c++)
      {
        long candidate_class = candidate_classes[c];
        float current_max = max_distances[candidate_class];
        if (current_max == min_max_distance)
        {
          final_candidates.push_back(candidate_class);
          if (candidate_class == object_class)
          {
            q_class_flag = 1;
          }
        }
      }

      long final_size = final_candidates.size();
      vint8 decision = function_random_vint8(0, final_size-1);
      classification_labels(k,i) = decision;
      if (q_class_flag == 0)
      {
        result(k) = result(k) + 1;
      }
      else
      {
        if (final_size > 1)
        {
          tie_counter = tie_counter + 1;
          float final = (float) final_size;
          result(k) = result(k) + (final - (float) 1.0) / final;
        }
      }
    }
  }

  function_print("\n");

  vPrint("ties = %li\n", tie_counter);
  for (k = 1; k <= max_k; k++)
  {
    result(k) = result(k) / (float) test_number;
  }

  knn_test_errors = result;
  if ((best_k >= 1) && (knn_test_errors.Size() > best_k))
  {
    best_knn_test = knn_test_errors(best_k);
    test_classification = classification_labels.horizontal_line(best_k);
  }

  return result;
}




// print some bad triples for each training object. Bad triples are
// triples (q, a, b) formed by training object q, its same-class
// 1-nearest neighbor a, and an other-class object b that is closer
// to q than a is to q.
vint8 vBnn::PrintBadTriples()
{
  if (bad_numbers.Size() == 1)
  {
    return 0;
  }
  vint8 number = bad_numbers.Size();

  vint8 q, i;
  vPrint("\nBad triples:\n");
  vint8 bad_counter = 0, tie_counter = 0, counter = 0;
  for (q = 0; q < number; q++)
  {
    vint8 bad = bad_numbers(q);
    vint8 tie = tie_numbers(q);
    if (bad + tie == 0)
    {
      continue;
    }
    counter = counter + 1;
    vPrint("%li: ", q);
    if (bad != 0)
    {
      bad_counter = bad_counter + 1;
      vPrint("%li bad:", bad);
      vint8 cols = bad_bs.Cols();
      vint8 limit = Min(cols, Min(bad, (vint8) 5));
      for (i = 0; i < limit; i++)
      {
        vPrint(" %li", bad_bs(q, i));
      }
    }
    if (tie != 0)
    {
      tie_counter = tie_counter + 1;
      vPrint("%li tie:", tie);
      vint8 cols = tie_bs.Cols();
      vint8 limit = Min(cols, Min(tie, (vint8) 5));
      for (i = 0; i < limit; i++)
      {
        vPrint(" %li", tie_bs(q, i));
      }
    }
    vPrint("\n");
  }

  vPrint("%li bad triples, %li tie triples\n", bad_triples, tie_triples);
  vPrint("counter = %li, bad counter = %li, tie counter = %li\n", 
          counter, bad_counter, tie_counter);
  return 1;
}


// same as PrintBadTriples, but here we can specify a range of training
// objects.
vint8 vBnn::PrintBadTriples2(vint8 start, vint8 end)
{
  if (bad_numbers.Size() == 1)
  {
    return 0;
  }
  vint8 number = bad_numbers.Size();


  vint8 q, i;
  vPrint("\nBad triples:\n");
  vint8 bad_counter = 0, tie_counter = 0, counter = 0;
  for (q = 0; q < number; q++)
  {
    vint8 bad = bad_numbers(q);
    vint8 tie = tie_numbers(q);
    if (bad + tie == 0)
    {
      continue;
    }
    counter = counter + 1;
    if (tie != 0)
    {
      tie_counter = tie_counter + 1;
    }
 
    if (bad != 0)
    {
      bad_counter = bad_counter + 1;
      if ((bad >= start) && (bad <= end))
      {
        vPrint("%li: ", q);
        vPrint("%li bad:", bad);
        vint8 cols = bad_bs.Cols();
        vint8 limit = Min(cols, Min(bad, (vint8) 5));
        for (i = 0; i < limit; i++)
        {
          vPrint(" %li", bad_bs(q, i));
        }

        if (tie != 0)
        {
          vPrint("%li tie:", tie);
          vint8 cols = tie_bs.Cols();
          vint8 limit = Min(cols, Min(tie, (vint8) 5));
          for (i = 0; i < limit; i++)
          {
            vPrint(" %li", tie_bs(q, i));
          }
        }
        vPrint("\n");
      }
    }
  }

  vPrint("%li bad triples, %li tie triples\n", bad_triples, tie_triples);
  vPrint("counter = %li, bad counter = %li, tie counter = %li\n", 
          counter, bad_counter, tie_counter);
  return 1;
}


// print a histogram of bad numbers.
vint8 vBnn::BadNumberHistogram()
{
  if (bad_numbers.Size() == 1)
  {
    return 0;
  }
  vint8 number = bad_numbers.Size();

  vMatrix<vint8> counters_matrix(1, number);
  function_enter_value(&counters_matrix, (vint8) 0);

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 bad = bad_numbers(i);
    counters_matrix(bad) = counters_matrix(bad) + 1;
  }

  for (i = 0; i < number; i++)
  {
    if (counters_matrix(i) == 0)
    {
      continue;
    }
    vPrint("%5i: %li\n", i, counters_matrix(i));
  }

  return 1;
}


// print the bad, tie, and good_a numbers for q.
vint8 vBnn::QNumbers(vint8 q)
{
  if (bad_numbers.Size() == 1)
  {
    return 0;
  }
  vint8 number = bad_numbers.Size();

  if ((q < 0) || (q >= number))
  {
    return 0;
  }

  vint8 bad = bad_numbers(q);
  vint8 tie = tie_numbers(q);
  vint8 good_a = good_a_numbers(q);
  vPrint("%li bad, %li tie, %li good_a\n", bad, tie, good_a);
  return 1;
}


// print the bad, tie and good_a numbers for a range of training 
// objects, from start to end.
vint8 vBnn::QNumbers2(vint8 start, vint8 end)
{
  vint8 q;
  for (q = start; q <= end; q++)
  {
    QNumbers(q);
  }

  return 1;
}


// return a matrix which, at the i-th column, stores the minimum
// and maximum value attained by any training object in the i-th
// attribute.
vMatrix<float> vBnn::DimensionRanges()
{
  vMatrix<float> result(2, attributes);
  vint8 i;
  vint8 junk = 0;
  for (i = 0; i < attributes; i++)
  {
    result(0, i) = color_imageColMin3(&training_vectors_matrix, i, &junk);
    result(1, i) = color_imageColMax3(&training_vectors_matrix, i, &junk);
  }

  return result;
}


vint8 vBnn::PrintDimensionRanges()
{
  vMatrix<float> ranges = DimensionRanges();
  ranges.Print("ranges");
  return 1;
}


// return a matrix which, at the i-th column, stores the mean
// and std for the i-th attribute values of all training objects.
vMatrix<float> vBnn::DimensionStds()
{
  vMatrix<float> result(2, attributes);
  long i;
  long junk = 0;
  for (i = 0; i < attributes; i++)
  {
    v3dMatrix<float> column_matrix = copy_vertical_line(&training_vectors_matrix, i);
    result(0, i) = (float) function_image_average(&column_matrix);
    result(1, i) = (float) function_image_deviation(&column_matrix);
  }

  return result;
}


vint8 vBnn::PrintDimensionStds()
{
  vMatrix<float> stds = DimensionStds();
  stds.Print("stds");
  return 1;
}


// normalize values of training vectors, so that, 
// at each attribute, the minimum
// value by any training object is 0, and the max value is 1.
vint8 vBnn::NormalizeRanges()
{
  vMatrix<float> ranges = DimensionRanges();
  vArray2(float) range_data = ranges.Matrix2();

  long row, col;
  for (row = 0; row < training_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float min = range_data[0][col];
      float max = range_data[1][col];
      float range = max - min;
      float value = training_vectors[row][col];
      value = value - min;
      if (range != 0)
      {
        value = value / range;
      }
      training_vectors[row][col] = value;
    }
  }

  for (row = 0; row < test_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float min = range_data[0][col];
      float max = range_data[1][col];
      float range = max - min;
      float value = test_vectors[row][col];
      value = value - min;
      if (range != 0)
      {
        value = value / range;
      }
      test_vectors[row][col] = value;
    }
  }

  return 1;
}


// normalize values of training vectors so that, at each attribute,
// the mean is 0.5 and the std is 0.5, as computed based on the
// values of all training objects for that attribute. Essentially,
// the mean and std for each column of training_vectors_matrix 
// becomes 0.5.
vint8 vBnn::NormalizeStds()
{
  vMatrix<float> stds = DimensionStds();
  vArray2(float) std_data = stds.Matrix2();

  long row, col;
  for (row = 0; row < training_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float mean = std_data[0][col];
      float std = std_data[1][col];
      float value = training_vectors[row][col];
      value = value - mean;
      if (std != 0)
      {
        value = value / ((float) 2.0 * std);
      }
      value = value + (float) 0.5;
      training_vectors[row][col] = value;
    }
  }

  for (row = 0; row < test_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float mean = std_data[0][col];
      float std = std_data[1][col];
      float value = test_vectors[row][col];
      value = value - mean;
      if (std != 0)
      {
        value = value / ((float) 2.0 * std);
      }
      value = value + (float) 0.5;
      test_vectors[row][col] = value;
    }
  }

  return 1;
}


// for all training and test vectors we square all their attributes.
// This is part of a MISTAKE that I should fix: for "naive-knn" 
// experiments, I thought that to compute Euclidean distances, all
// I had to do was square entries and then compute L1 distances.
// That is not correct, therefore the results I put into the 
// submission are not correct.
vint8 vBnn::SquareEntries()
{
  long row, col;
  for (row = 0; row < training_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float value = training_vectors[row][col];
      training_vectors[row][col] = value * value;
    }
  }

  for (row = 0; row < test_number; row++)
  {
    for (col = 0; col < attributes; col++)
    {
      float value = test_vectors[row][col];
      test_vectors[row][col] = value * value;
    }
  }

  return 1;
}


// training_scores holds, at each column, the results of the i-th
// attribute on all training triples.
vint8 vBnn::TrainingScoresExist()
{
  if (training_scores_matrix.Rows() != training_triple_number)
  {
    return 0;
  }
  if (training_scores_matrix.Cols() != attributes)
  {
    return 0;
  }
  return 1;
}


vint8 vBnn::MakeTrainingScores()
{
  training_scores_matrix = vMatrix<float>(training_triple_number, attributes);
  training_scores = training_scores_matrix.Matrix2();

  vint8 row, col;
  for (row = 0; row < training_triple_number; row++)
  {
    vint8 q = training_triples[row][0];
    vint8 a = training_triples[row][1];
    vint8 b = training_triples[row][2];

    for (col = 0; col < attributes; col++)
    {
      // label = 1 means that q is always same class as a and different
      // class than b. Based on the way we construct the training and
      // validation sets, label is always 1. Label = -1 would mean
      // that q is same class as b and different class than a.
      float label = 1;
  
      float q_dim = training_vectors[q][col];
      float a_dim = training_vectors[a][col];
      float b_dim = training_vectors[b][col];
      float distance = (vAbs(q_dim - b_dim) - vAbs(q_dim - a_dim)) * label;

      training_scores[row][col] = distance;
    }
  }

  return 1;
}


vint8 vBnn::DeleteTrainingScores()
{
  training_scores_matrix = vMatrix<float>();
  return 1;
}


// compute the total margin based on each attribute, on the weighted 
// training set of triples. This is a somewhat misnamed function, it
// should be called AttributeMargins.
// In ICML 2004 experiments, I found that using this function 
// as a heuristic to choose the next weak classifier didn't give that
// good results. For the KDD 2004 experiments, I found that choosing
// the weak classifier from the top 50 or so (ranked with respect
// to training error, not margin) gave very good results. I should,
// at some point, try to reconcile these two results. Possibly if
// I used error and not margin in here, it would work better.
vMatrix<float> vBnn::AttributeErrors()
{
  vMatrix<float> result(1, attributes);
  vint8 flag = TrainingScoresExist();
  // If the training scores don't exist, we will create them now,
  // and delete them at the end of the function. Otherwise we just
  // use them.
  if (flag == 0)
  {
    MakeTrainingScores();
  }

  vint8 i;
  for (i = 0; i < attributes; i++)
  {
    vint8 j;
    float accuracy = 0;
    for (j = 0; j < training_triple_number; j++)
    {
      float distance = training_scores[j][i];
      float weight = training_factors[j];
      accuracy = accuracy + weight * distance;
    }

    float error = -accuracy;
    if (error > 0)
    {
      error = -error;
    }
    result(i) = error;
  }

  if (flag == 0)
  {
    DeleteTrainingScores();
  }

  return result;
}


vint8 vBnn::BestK()
{
  return best_k;
}


float vBnn::BestKnnTraining()
{
  return best_knn_training;
}


float vBnn::BestKnnTest()
{
  return best_knn_test;
}


vint8 vBnn::TrainingNumber()
{
  return training_number;
}


vint8 vBnn::TestNumber()
{
  return test_number;
}



// finds the specified number of nearest neighbors to a particular
// training object.
// indices[k] will be the k-1 nearest-neighbor.
// distances[k] will be the distance of the neighbor to the specified object
// actual_number will store the number of neighbors actually found,
// and this will be greater than number if there is a tie
// (in that case we will include all objects tying for
// number-th nearest neighbor).

vint8 vBnn::find_nearest_neighbors(float_matrix distances_matrix, vint8 neighbor_number, 
                            vector<vint8> * indices_vector,
                            vector<float> * distances_vector, vint8 * actual_number)
{
  vint8 threshold_index = 0;
  float threshold_distance = kth_smallest_cb(neighbor_number, &distances_matrix,
                                              &threshold_index);
  vector<class_couple> neighbors;
  vint8 j;
  for (j = 0; j < training_number; j++)
  {
    float distance = distances_matrix(j);
    if (distance <= threshold_distance)
    {
      neighbors.push_back(class_couple(distance, (void *) (long) j));
    }
  }

  std::sort(neighbors.begin(), neighbors.end(), couple_less());
  vint8 number = neighbors.size();
  for (j = 0; j < number; j++)
  {
    vint8 index = (vint8) (long) neighbors[(vector_size) j].object;
    float distance = neighbors[(vector_size) j].value;
    indices_vector->push_back(index);

    if (distances_vector != 0)
    {
      distances_vector->push_back(distance);
    }
  }

  if (actual_number != 0)
  {
    *actual_number = number;
  }
  return 1;
}


// finds the specified number of nearest neighbors to a particular
// training object.
// indices[k] will be the k-1 nearest-neighbor.
// distances[k] will be the distance of the neighbor to the specified object
// actual_number will store the number of neighbors actually found,
// and this will be greater than number if there is a tie
// (in that case we will include all objects tying for
// number-th nearest neighbor).
vint8 vBnn::nearest_neighbors(vint8 object_index, vint8 neighbor_number, vector<vint8> * indices_vector,
                              vector<float> * distances_vector, vint8 * actual_number)
{
  // as is, this function will not work with query-sensitive distance measures,
  // unless we use some virtual function to get the actual distances to all
  // training objects.  Until I implement that, we should make sure this 
  // function is not used with query-sensitive distance measures.
  if (class_name == "vBnnqs")
  {
    exit_error("\nerror: nearest_neighbors cannot handle query-sensitive distances\n");
  }

  if ((object_index < 0) || (object_index >= training_number))
  {
    if (actual_number != 0)
    {
      *actual_number = 0;
    }
    return 0;
  }

  vint8 object_class = training_labels_matrix(object_index);
  
  vMatrix<float> distances_matrix = TestDistances2(object_index, training_vectors_matrix);
  vArray(float) distances = distances_matrix.Matrix();

  // Overwrite the distance that corresponds to the object itself.
  float max_distance_limit = function_image_maximum(&distances_matrix) * (float) 2.0 + (float) 1.0;
  distances[object_index] = max_distance_limit;

  // Also, overwrite the distances that correspond to same-class objects 
  // coming from the same subject.
  if (subject_ids_set == 1)
  {
    vint8 object_subject = subject_ids[object_index];
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      if ((subject_ids[j] == object_subject) && 
          (training_labels[j] == object_class))
      {
        distances[j] = max_distance_limit;
      }
    }
  }

  return find_nearest_neighbors(distances_matrix, neighbor_number, indices_vector,
                                distances_vector, actual_number);
}


vint8 vBnn::similar_nearest_neighbors(vint8 object_index, vint8 neighbor_number, vector<vint8> * indices_vector,
                              vector<float> * distances_vector, vint8 * actual_number)
{
  // as is, this function will not work with query-sensitive distance measures,
  // unless we use some virtual function to get the actual distances to all
  // training objects.  Until I implement that, we should make sure this 
  // function is not used with query-sensitive distance measures.
  if (class_name == "vBnnqs")
  {
    exit_error("\nerror: nearest_neighbors cannot handle query-sensitive distances\n");
  }

  if ((object_index < 0) || (object_index >= training_number))
  {
    if (actual_number != 0)
    {
      *actual_number = 0;
    }
    return 0;
  }

  vint8 object_class = training_labels_matrix(object_index);
  
  vint8 threshold_index = 0;
  vMatrix<float> distances_matrix = TestDistances2(object_index, training_vectors_matrix);
  vArray(float) distances = distances_matrix.Matrix();
  // Overwrite the distance that corresponds to the object itself.
  float max_distance_limit = function_image_maximum(&distances_matrix) * (float) 2.0 + (float) 1.0;
  distances[object_index] = max_distance_limit;

  // Also, overwrite the distances that correspond to same-class objects
  // from the same subject.
  if (subject_ids_set == 1)
  {
    vint8 object_subject = subject_ids[object_index];
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      if ((subject_ids[j] == object_subject) && 
          (training_labels[j] == object_class))
      {
        distances[j] = max_distance_limit;
      }
    }
  }

  // Also, overwrite the distances that correspond to objects 
  // from a different class class than 
  // the object indexed by object_index.
  long j;
  for (j = 0; j < training_number; j++)
  {
    if (training_labels[j] != object_class)
    {
      distances[j] = max_distance_limit;
    }
  }
  return find_nearest_neighbors(distances_matrix, neighbor_number, indices_vector,
                                distances_vector, actual_number);
}


vint8 vBnn::different_nearest_neighbors(vint8 object_index, vint8 neighbor_number, vector<vint8> * indices_vector,
                              vector<float> * distances_vector, vint8 * actual_number)
{
  // as is, this function will not work with query-sensitive distance measures,
  // unless we use some virtual function to get the actual distances to all
  // training objects.  Until I implement that, we should make sure this 
  // function is not used with query-sensitive distance measures.
  if (class_name == "vBnnqs")
  {
    exit_error("\nerror: nearest_neighbors cannot handle query-sensitive distances\n");
  }

  if ((object_index < 0) || (object_index >= training_number))
  {
    if (actual_number != 0)
    {
      *actual_number = 0;
    }
    return 0;
  }

  vint8 object_class = training_labels_matrix(object_index);
  
  vint8 threshold_index = 0;
  vMatrix<float> distances_matrix = TestDistances2(object_index, training_vectors_matrix);
  vArray(float) distances = distances_matrix.Matrix();
  // Overwrite the distance that corresponds to the object itself.
  float max_distance_limit = function_image_maximum(&distances_matrix) * (float) 2.0 + (float) 1.0;
  distances[object_index] = max_distance_limit;

  // Also, overwrite the distances that correspond to same-class objects 
  vint8 j;
  for (j = 0; j < training_number; j++)
  {
    if (training_labels[j] == object_class)
    {
      distances[j] = max_distance_limit;
    }
  }

  return find_nearest_neighbors(distances_matrix, neighbor_number, indices_vector,
                                distances_vector, actual_number);
}


// result(k) is the class label that is the result of
// knn classification for each k in 1, ..., neighbor_number
// for the specified object.  The object is simply specified
// by the set of distances to all training objects.
// if the object is actually a training object
// and also if we need to exclude training objects from the same subject,
// it is the responsibility of the calling function to take care of those things.
//
// if there is a tie for the winning class, then we put a -1
// for the class label, so the error rate computed using this function
// can be larger than the true error rate.
vint8_matrix vBnn::knn_classifications(float_matrix distances, 
                                       vint8 neighbor_number)
{
  vint8_matrix result (1, neighbor_number +1);
  // there is no result defined using zero nearest neighbors
  result (0) = -1;

  vint8 threshold_index = 0;
  float threshold_distance = kth_smallest_cb(neighbor_number, &distances,
                                             &threshold_index);
    
  vector<float> nn_distances;
  vector<vint8> nn_classes;

  // collect the max_k neighbors (and neighbors after that that are
  // tied to the max_k neighbor).
  vint8 j;
  for (j = 0; j < training_number; j++)
  {
    if (distances(j) <= threshold_distance)
    {
      nn_distances.push_back(distances(j));
      nn_classes.push_back(training_labels[j]);
    }
  }

  vMatrix<vint8> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(vint8) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
 
  vint8 number = nn_distances.size();
  // Now, collect votes, and get the recognition, for each k.
  vint8 k;
  for (k = 1; k <= neighbor_number; k++)
  {
    function_enter_value(&votes_matrix, (vint8) 0);
    function_enter_value(&max_distances_matrix, (float) -1000000000);
    long junk = 0;
    float k_threshold_distance = kth_smallest_ca((long) k, &nn_distances, &junk);
    vint8 counter = 0;
    for (j = 0; j < number; j++)
    {
      float distance = nn_distances[(vector_size) j];
      if (distance <= k_threshold_distance)
      {
        counter++;
        vint8 j_class = nn_classes[(vector_size) j];
        votes[j_class] = votes[j_class] + 1;
        if (max_distances[j_class] < distance)
        {
          max_distances[j_class] = distance;
        }
      }
    }

    vint8 max_votes = function_image_maximum(&votes_matrix);

    // Find all classes that received the max number of votes.
    vector<vint8> candidate_classes;
    vint8 c;
    for (c = 1; c <= classes; c++)
    {
      if (votes[c] == max_votes)
      {
        candidate_classes.push_back(c);
      }
    }

    if (candidate_classes.size() == 0)
    {
      exit_error("Error: impossible, 0 candidate classes\n");
    }

    float min_max_distance = max_distances[candidate_classes[0]];
    vint8 candidates = candidate_classes.size();
    // Find the min max distance among all candidates (to break ties).
    for (c = 1; c < candidates; c++)
    {
      vint8 candidate_class = candidate_classes[(vector_size) c];
      float current_max = max_distances[candidate_class];
      if (current_max < min_max_distance)
      {
        min_max_distance = current_max;
      }
    }

    // Now go through candidates again, and only keep the one whose
    // max distance is min_max_distance;
    vector<vint8> final_candidates;
    vint8 q_class_flag = 0;
    for (c = 0; c < candidates; c++)
    {
      vint8 candidate_class = candidate_classes[(vector_size) c];
      float current_max = max_distances[candidate_class];
      if (current_max == min_max_distance)
      {
        final_candidates.push_back(candidate_class);
      }
    }

    vint8 final_size = final_candidates.size();
    if (final_size > 1)
    {
      result (k) = -1;
    }
    else
    {
      result(k) = final_candidates[0];
    }
  }

  return result;
}


// result(k) represents the accuracy of 
// knn classification for each k in 1, ..., neighbor_number
// for the specified object.  The object is simply specified
// by the set of distances to all training objects.
// if the object is actually a training object
// and also if we need to exclude training objects from the same subject,
// it is the responsibility of the calling function to take care of those things.
//
// in the result, 1 stands for correct classification, and -1
// stands for incorrect classification.
vint8_matrix vBnn::knn_accuracies(float_matrix distances, vint8 true_label,
                                 vint8 neighbor_number)
{
  vint8_matrix labels = knn_classifications (distances, neighbor_number);
  vint8 number = labels.length();
  vint8_matrix result(1, number);

  vint8 counter;
  for (counter = 0; counter <number; counter++)
  {
    if (labels (counter) == true_label)
    {
      result(counter) = 1;
    }
    else
    {
      result (counter) = -1;
    }
  }

  return result;
}


// result(k) is the class label that is the result of
// knn classification for each k in 1, ..., neighbor_number
// for the specified training object
vint8_matrix vBnn::knn_classification_training (vint8 object, vint8 neighbor_number)
{
  vMatrix<float> distances = TestDistances2(object, training_vectors_matrix);
    
  // Overwrite the distance that corresponds to i itself.
  float max_distance_limit = function_image_maximum(&distances) * (float) 2.0 + (float) 1.0;
  distances(object) = max_distance_limit;

  // Also, overwrite the distances that correspond to objects 
  // coming from the same subjects as the subject who produced q.
  if (subject_ids_set == 1)
  {
    vint8 subject = subject_ids[object];
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
//        if ((subject_ids[j] == i_subject) && 
//            (training_labels[j] == object_class))
      if (subject_ids[j] == subject)
      {
        distances(j) = max_distance_limit;
      }
    }
  }

  vint8_matrix result = knn_classifications (distances, neighbor_number);
  return result;
}


// result(k) is the class label that is the result of
// knn classification for each k in 1, ..., neighbor_number
// for the specified training object
vint8_matrix vBnn::knn_classification_test (vint8 object, vint8 neighbor_number)
{
  vMatrix<float> distancesm = TestDistances2(object, test_vectors_matrix);
  vint8_matrix result = knn_classifications (distancesm, neighbor_number);
  return result;
}




// neighbor_number is the number k of nearest neighbors
// to use for classification
vint8_matrix vBnn::training_accuracies(vint8 neighbor_number)
{
  vint8_matrix result(1, training_number);

  long object;
  for (object = 0; object <training_number; object++)
  {
    vint8_matrix class_labels = knn_classification_training(object, neighbor_number);
    vint8 true_label = training_labels_matrix (object);
    if (true_label == class_labels (neighbor_number))
    {
      result (object) = 1;
    }
    else
    {
      result (object) = -1;
    }
  }

  return result;
}


vint8_matrix vBnn::test_accuracies(vint8 neighbor_number)
{
  vint8_matrix result(1, test_number);

  long object;
  for (object = 0; object < test_number; object++)
  {
    vint8_matrix class_labels = knn_classification_test(object, neighbor_number);
    vint8 true_label = test_labels_matrix (object);
    if (true_label == class_labels (neighbor_number))
    {
      result (object) = 1;
    }
    else
    {
      result (object) = -1;
    }
  }

  return result;
}



// initializes a binary learner, which, after training,
// can be used as a component for a decision tree of distance measures.
// number is the number k of nearest neighbors to use
// for classification.
AdaBoost * vBnn::binary_learner(vint8 neighbor_number)
{
  vint8_matrix new_training_labels = training_accuracies (neighbor_number);
  vint8_matrix new_test_labels = test_accuracies (neighbor_number);

  AdaBoost * result = new AdaBoost (training_vectors_matrix, new_training_labels,
                                    test_vectors_matrix, new_test_labels);

  return result;
}



//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////
//////////////////////////////////////////////


vBoostNN::vBoostNN() : vBnn()
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBoostNN");
}



vBoostNN::vBoostNN(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vint8 in_training_number, vint8 in_validation_number) :
vBnn(in_training_vectors, in_training_labels,
                              in_test_vectors, in_test_labels,
                              in_training_number, in_validation_number)
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBoostNN");
}


vBoostNN::vBoostNN(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vMatrix<vint8> in_training_triples,
                     vMatrix<vint8> in_validation_triples) :
vBnn(in_training_vectors, in_training_labels,
     in_test_vectors, in_test_labels,
     in_training_triples, in_validation_triples)
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBoostNN");
}


vBoostNN::vBoostNN(vMatrix<float> in_training_vectors,
                    vMatrix<vint8> in_training_labels,
                    vMatrix<float> in_test_vectors,
                    vMatrix<vint8> in_test_labels,
                    vint8 in_training_number, vint8 in_validation_number,
                    vint8_matrix argument_active_training,
                    vint8_matrix argument_previous_training_labels,
                    vint8_matrix argument_active_test,
                    vint8_matrix argument_previous_test_labels) :
vBnn(in_training_vectors, in_training_labels,
     in_test_vectors, in_test_labels,
     in_training_number, in_validation_number)
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBoostNN");

  active_training = vint8_matrix(&argument_active_training);
  previous_training_labels = vint8_matrix(&argument_previous_training_labels);
  active_test = vint8_matrix(&argument_active_test);
  previous_test_labels = vint8_matrix(&argument_previous_test_labels);
}


vBoostNN::~vBoostNN()
{
}


// This function uses ResampleTriples to create training and validation
// triples, which are used to initialize a new vBoostNN object.
vBoostNN * vBoostNN::NewBoostNN(vint8 max_k)
{
  vMatrix<vint8> new_training_triples = ResampleTriples(training_triple_number,
                                                        max_k);
//  vMatrix<vint8> new_training_triples = ResampleTriples5();
  vMatrix<vint8> new_validation_triples = ResampleTriples(validation_triple_number,
                                                          max_k);
  vBoostNN * result = new vBoostNN(training_vectors_matrix, training_labels_matrix,
                                     test_vectors_matrix, test_labels_matrix, 
                                     new_training_triples, new_validation_triples);

  result->SetAllowNegativeWeights(allow_negative_weights);
  if (subject_ids_set == 1)
  {
    result->SetSubjectIds(subject_ids_matrix);
  }

  result->active_training = vint8_matrix(&active_training);
  result->previous_training_labels = vint8_matrix(&previous_training_labels);
  result->active_test = vint8_matrix(&active_test);
  result->previous_test_labels = vint8_matrix(&previous_test_labels);

  return result;
}
  

// This function uses ResampleTriples2 to create training and validation
// triples, which are used to initialize a new vBoostNN object.
vBoostNN * vBoostNN::NewBoostNN2()
{
  vMatrix<float> training_factorsp(1, training_triple_number);
  vMatrix<float> validation_weightsp(1, validation_triple_number);
  vMatrix<vint8> new_training_triples = ResampleTriples2(training_triple_number, 
                                                         training_factorsp);
  vMatrix<vint8> new_validation_triples = ResampleTriples(validation_triple_number, 2);
  vBoostNN * result = new vBoostNN(training_vectors_matrix, training_labels_matrix,
                                     test_vectors_matrix, test_labels_matrix, 
                                     new_training_triples, new_validation_triples);
  result->training_factors_matrix = training_factorsp;
  result->training_factors = training_factorsp.Matrix();
  result->SetAllowNegativeWeights(allow_negative_weights);
  if (subject_ids_set == 1)
  {
    result->SetSubjectIds(subject_ids_matrix);
  }
  return result;
}
  

// this function is useful for training a decision tree
// of distance measures.  We assume that the splitter has already been 
// trained to tell which objects are classified correctly
// or incorrectly by the current boost-nn.
vBoostNN * vBoostNN::next_level(AdaBoost * splitter, float threshold)
{
  // make sure we have computed the classification results
  if (best_k < 1)
  {
    KnnTrainingErrors(101);
    KnnTestErrors(101);
  }

  vint8_matrix training_decisions = splitter->training_classifications(threshold);
  vint8_matrix new_active_training = ((vint8) -1) * training_decisions;

  vint8_matrix test_decisions = splitter->test_classifications(threshold);
  vint8_matrix new_active_test = ((vint8) -1) * test_decisions;

  vBoostNN * result = new vBoostNN(training_vectors_matrix, training_labels_matrix, 
                                   test_vectors_matrix, test_labels_matrix,
                                   training_triple_number, validation_triple_number,
                                   new_active_training, training_classification,
                                   new_active_test, test_classification);

  return result;
}


// Here, for each test object, we just take the winning class from 
// each bnn in bnns, using 1-nn. Then, we find the class that
// receives the most votes (i.e. that won in the most bnns). Note
// that in order for this function to give the right result, we
// must have called AnalyzeTestTriples for each object stored
// in bnns.
vint8 vBoostNN::Majority1(vector<vBoostNN *> * bnns)
{
  vint8 number = bnns->size();
  if (number == 0) 
  {
    return 0;
  }

  vint8 tie_counter = 0;
  vint8 distance_ties = 0;
  vint8 q;
  vint8 classes = (*bnns)[0]->classes;
  vint8 test_number = (*bnns)[0]->test_number;
  vArray(vint8) test_labels = (*bnns)[0]->test_labels;
  vArray(vint8) training_labels = (*bnns)[0]->training_labels;
  vMatrix<float> class_counters(1, classes+1);
  vint8 problem_counter = 0;
  float accuracy_rate = 0;
  for (q = 0; q < test_number; q++)
  {
    vint8 q_class = test_labels[q];
    function_enter_value(&class_counters, (float) 0);

    vint8 i;
    for (i = 0; i < number; i++)
    {
      vBoostNN * bnn = (*bnns)[(vector_size) i];
      vint8 same_class_nn = bnn->same_class_nn(q);
      vint8 other_class_nn = bnn->other_class_nn(q);
      vint8 other_class = training_labels[other_class_nn];
      float same_distance = bnn->test_distances_matrix(q, same_class_nn);
      float other_distance = bnn->test_distances_matrix(q, other_class_nn);
      if (same_distance < other_distance)
      { 
        class_counters(q_class)++;
      }
      else if (same_distance == other_distance)
      {
        class_counters(other_class) += 1;
        distance_ties++;
      }
      else
      { 
        class_counters(other_class)++;
      }
    }

    vint8 success = 0;
    float max_votes = function_image_maximum(&class_counters);
    float counter = 0;
    if (class_counters(q_class) == max_votes)
    {
      for (i = 1; i <= classes; i++)
      {
        if (class_counters(i) == max_votes)
        {
          counter = counter + 1;
        }
      }
      if (counter == 0)
      {
        exit_error("Error: Impossible\n");
      }
      accuracy_rate = accuracy_rate + (float) 1.0 / counter;
      if (counter > 1)
      {
        tie_counter++;
        success = 1;
      }
      else
      {
        success = 2;
      }
    }

    if (class_counters(q_class) > number)
    {
      vPrint("here\n");
    }

    if (class_counters(q_class) != number)
    {
      problem_counter++;
      vPrint("q = %5li: ", q);
      for (i = 0; i < number; i++)
      {
        vBoostNN * bnn = (*bnns)[(vector_size) i];
        vint8 same_class_nn = bnn->same_class_nn(q);
        vint8 other_class_nn = bnn->other_class_nn(q);
        vint8 other_class = training_labels[other_class_nn];
        float same_distance = bnn->test_distances_matrix(q, same_class_nn);
        float other_distance = bnn->test_distances_matrix(q, other_class_nn);
        if (same_distance < other_distance)
        { 
          vPrint("  1  ");
        }
        else
        {
          vPrint("  0  ");
        }
      }
      vPrint("-  %li\n", success);
    }
  }
  
  accuracy_rate = accuracy_rate / (float) test_number;
  float error_rate = 1 - accuracy_rate;
  vPrint("%li ties, %li distance ties. %li problems\n", 
          tie_counter, distance_ties, problem_counter);
  vPrint("accuracy = %f, error rate = %f\n",
          accuracy_rate, error_rate);
  return 1;
}


vint8 vBoostNN::CleanUpClassifier()
{
  CleanUp(attributes, &distance_indices, &distance_factors,
          &cleaned_indices, &cleaned_factors);
  return 1;
}


// here, in addition to index and alpha, we also specify z, so that
// we don't have to spend any more time to compute z, when z
// is already available.
vint8 vBoostNN::AddClassifier3(vint8 index, float alpha, float z)
{
  // invalidate results of previous classifications
  best_k = -1;
  training_classification = vint8_matrix();
  test_classification = vint8_matrix();

  vector<float> results((vector_size) training_triple_number);
  DistanceResults(index, &results, training_triples_matrix);
  ComputeLastError(alpha, &results);

  distance_indices.push_back(index);
  distance_factors.push_back(alpha);
  distance_zs.push_back(z);
  UpdateWeights(&results, z, alpha);

  CleanUpClassifier();
  float margin = ComputeTrainingError(&results, alpha);

  vector<float> validation_results((vector_size) validation_triple_number);
  DistanceResults(index, &validation_results, validation_triples_matrix);
  ComputeValidationError(&validation_results, alpha);
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);
//  ComputeTestErrorSlow2();
  
  UpdateDistances(index, alpha);
  ComputeTestError();
  test_errors.push_back(test_error);
  test_triple_errors.push_back(test_triple_error);

  return 1;
}


// here, we just pass the index.
vint8 vBoostNN::AddClassifier(vint8 index)
{
  if ((index <0) || (index = attributes))
  {
    return 0;
  }

  // invalidate results of previous classifications
  best_k = -1;
  training_classification = vint8_matrix();
  test_classification = vint8_matrix();

  float alpha = 0;
  float z = DistanceZ(index, &alpha);
  
  vector<float> results((vector_size) training_triple_number);
  DistanceResults(index, &results, training_triples_matrix);
  ComputeLastError(alpha, &results);

  distance_indices.push_back(index);
  distance_factors.push_back(alpha);
  distance_zs.push_back(z);
  UpdateWeights(&results, z, alpha);

  CleanUpClassifier();
  float margin = ComputeTrainingError(&results, alpha);

  vector<float> validation_results((vector_size) validation_triple_number);
  DistanceResults(index, &validation_results, validation_triples_matrix);
  ComputeValidationError(&validation_results, alpha);
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);
//  ComputeTestErrorSlow2();
  
  UpdateDistances(index, alpha);
  ComputeTestError();
  test_errors.push_back(test_error);
  test_triple_errors.push_back(test_triple_error);

  return 1;
}

// return the number of training steps performed so far.
vint8 vBoostNN::StepsDone()
{
  vint8 result = distance_indices.size();
  return result;
}


// Here dimensions (i.e. indices of attributes) can occur many times.
vint8 vBoostNN::Classifier(vector<vint8> * out_dimensions, 
                           vector<float> * out_weights)
{
  vint8 steps = StepsDone();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    out_dimensions->push_back(distance_indices[(vector_size) i]);
    out_weights->push_back(distance_factors[(vector_size) i]);
  }

  return steps;
}


// Here we identify repetitions of the same dimension (i.e. the same
// attribute, i.e. the same feature) and we combine them by adding
// the weights associated with each occurrence.
vint8 vBoostNN::CleanClassifier(vector<vint8> * out_indices, 
                                vector<float> * out_weights)
{
  vint8 steps = cleaned_indices.size();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    out_indices->push_back(cleaned_indices[(vector_size) i]);
    out_weights->push_back(cleaned_factors[(vector_size) i]);
  }

  return steps;
}


// Computes test_error, test_triple_error, test_confusion, and
// test_triple_confusion. 
float vBoostNN::ComputeTestErrorSlow()
{
  if (test_number < 1)
  {
    exit_error("Error: ComputeTestErrorSlow called, test_number = %li\n",
                    test_number);
  }

  // First, compute test error on objects.

  // counter will count misclassified objects.
  float counter = 0;
  function_enter_value(&test_confusion, (float) 0);
  vArray2(float) confusion = test_confusion.Matrix2();

  vint8 i;
  for (i = 0; i < test_number; i++)
  {
    v3dMatrix<float> test_object = copy_horizontal_line(&test_vectors_matrix, i);
    vMatrix<float> distances = TestDistances(test_object, &cleaned_indices,
                                              &cleaned_factors);
    vArray(float) distance_data = distances.Matrix();
    vint8 nn_row = 0, nn_col = 0;
    float nn_distance = function_image_minimum3(&distances, &nn_row, &nn_col);

    // Find all objects with nn_distance from the query (to handle ties).
    vector<vint8> candidates;
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      if (distance_data[j] == nn_distance)
      {
        candidates.push_back(j);
      }
    }

    vint8 object_class = test_labels[i];
    vint8 nn_class;
    vint8 candidate_size = candidates.size();
    if (candidate_size == 1)
    {
      nn_class = training_labels[candidates[0]];
    }
    else if (candidate_size > 1)
    {
      vint8 pick = function_random_vint8(0, candidate_size - 1);
      nn_class = training_labels[candidates[(vector_size) pick]];
    }
    else
    {
      exit_error("Error: no candidates found\n");
    }
      
    confusion[object_class][nn_class] = confusion[object_class][nn_class] + (float) 1;
    if (nn_class != object_class)
    {
      counter = counter + 1;
    }
  }

  test_error2 = counter / (float) test_number;
  
  // Now compute test_error on triples;
  counter = 0;
  function_enter_value(&test_triple_confusion, (float)  0);
  confusion = test_triple_confusion.Matrix2();
  vint8 number = test_triples_matrix.Rows();
  for (i = 0; i < number; i++)
  {
    vint8 q = test_triples[i][0];
    vint8 a = test_triples[i][1];
    vint8 b = test_triples[i][2];

    vint8 q_class = test_labels[q];
    vint8 a_class = training_labels[a];
    vint8 b_class = training_labels[b];

    // check q_class = a_class != b_class
    if ((q_class != a_class) || (q_class == b_class))
    {
      exit_error("Error: q_class = %li, a_class = %li, b_class = %li\n",
                      q_class, a_class, b_class);
    }

    v3dMatrix<float> q_object = copy_horizontal_line(&test_vectors_matrix, q);
    v3dMatrix<float> a_object = copy_horizontal_line(&training_vectors_matrix, a);
    v3dMatrix<float> b_object = copy_horizontal_line(&training_vectors_matrix, b);

    float qa_distance = ClassifierDistance(q_object, a_object,
                                           &cleaned_indices, &cleaned_factors);
    float qb_distance = ClassifierDistance(q_object, b_object,
                                           &cleaned_indices, &cleaned_factors);
    if (qa_distance < qb_distance)
    {
      confusion[q_class][a_class] = confusion[q_class][a_class] + (float) 1;
    }
    else if (qb_distance < qa_distance)
    {
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 1;
      counter = counter + 1; 
    }
    else // if the two distances are equal
    {
      confusion[q_class][a_class] = confusion[q_class][b_class] + (float) 0.5;
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 0.5;
      counter = counter + (float) 0.5;
    }
  }
  
  test_triple_error2 = counter / (float) number;
  return test_error2;
}


float vBoostNN::ComputeTestErrorSlow2()
{
  if (test_number < 1)
  {
    exit_error("Error: ComputeTestErrorSlow2 called, test_number = %li\n",
                    test_number);
  }

  // First, compute test error on objects.

  // counter will count misclassified objects.
  float counter = 0;
  function_enter_value(&test_confusion, (float) 0);
  vArray2(float) confusion = test_confusion.Matrix2();

  vint8 i;
  for (i = 0; i < test_number; i++)
  {
    vMatrix<float> distances = TestDistances4(i, test_vectors_matrix,
                                               &cleaned_indices,
                                               &cleaned_factors);
    vint8 nn_row = 0, nn_col = 0;
    float nn_distance = function_image_minimum3(&distances, &nn_row, &nn_col);
    vint8 object_class = test_labels[i];
    vint8 nn_class = training_labels[nn_col];
    confusion[object_class][nn_class] = confusion[object_class][nn_class] + (float) 1;
    if (nn_class != object_class)
    {
      counter = counter + 1;
    }
  }

  test_error2 = counter / (float) test_number;
  
  // Now compute test_error on triples;
  counter = 0;
  function_enter_value(&test_triple_confusion, (float) 0);
  confusion = test_triple_confusion.Matrix2();
  vint8 number = test_triples_matrix.Rows();
  for (i = 0; i < number; i++)
  {
    vint8 q = test_triples[i][0];
    vint8 a = test_triples[i][1];
    vint8 b = test_triples[i][2];

    vint8 q_class = test_labels[q];
    vint8 a_class = training_labels[a];
    vint8 b_class = training_labels[b];

    // check q_class = a_class != b_class
    if ((q_class != a_class) || (q_class == b_class))
    {
      exit_error("Error: q_class = %li, a_class = %li, b_class = %li\n",
                      q_class, a_class, b_class);
    }

    float qa_distance = ClassifierDistance2(q, test_vectors_matrix,
                                            a, training_vectors_matrix,
                                            &cleaned_indices, 
                                            &cleaned_factors);
    float qb_distance = ClassifierDistance2(q, test_vectors_matrix,
                                            b, training_vectors_matrix,
                                            &cleaned_indices, 
                                            &cleaned_factors);
    if (qa_distance < qb_distance)
    {
      confusion[q_class][a_class] = confusion[q_class][a_class] + (float) 1;
    }
    else if (qb_distance < qa_distance)
    {
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 1;
      counter = counter + 1;
    }
    else // if the two distances are equal
    {
      confusion[q_class][a_class] = confusion[q_class][b_class] + (float) 0.5;
      confusion[q_class][b_class] = confusion[q_class][b_class] + (float) 0.5;
      counter = counter + (float) 0.5;
    }
  }
  
  test_triple_error2 = counter / (float) number;
  return test_error2;
}


// For debugging: sum up the weights of all classifiers. 
float vBoostNN::SumClassifierWeights()
{
  float sum = 0;
  vint8 i;
  for (i = 0; i < (vint8) distance_factors.size(); i++)
  {
    sum = sum + distance_factors[(vector_size) i];
  }
  
  return sum;
}


// This function is called to regenerate triples, so that they are 
// more focused (i.e. both a and b are close to q). The algorithm is
// the following:
// - pick q randomly.
// - pick b_class randomly.
// - set rank maximum = max(rank_limit, # of q-class objects - 1, # of
//   b-class object).
// - choose a random rank r between 1 and rank maximum
// - set a = r-nearest-neighbor of q among q-class objects.
// - set b = r-nearest-neighbor of q among b-class objects.
vMatrix<vint8> vBnn::ResampleTriples(vint8 number, vint8 max_k)
{
  const vint8 rank_limit = max_k;

  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    // Pick a random q.
    vint8 q = function_random_vint8(0, training_number - 1);
    while (active_training (q) != 1)
    {
      q = function_random_vint8(0, training_number - 1);
    }
    
    // Get the class of q.
    vint8 q_class = training_labels[q];

    // choose a random class for b.
    vint8 b_class = -1;
    while(1)
    {
      vint8 b = function_random_vint8(0, training_number - 1);
      b_class = training_labels[b];

//      b_class = function_random_vint8(1, classes);
      if (b_class != q_class)
      {
        break;
      }
    }

    vint8 q_size = class_indices[q_class].size();
    vint8 b_size = class_indices[b_class].size();

    // find the index of q in class_indices[q_class].
    vint8 j;
    vint8 q_index = -1;
    for (j = 0; j < q_size; j++)
    {
      if (class_indices[q_class][(vector_size) j] == q)
      {
        q_index = j;
        break;
      }
    }
    if (q_index == -1)
    {
      exit_error("Error: q_index = -1\n");
    }

    // Get the rank-nearest neighbors of q among q-class objects
    // and b-class objects.
    // First, choose a to be the rank-nearest neighbor of q among
    // q-class objects (excluding q itself).
    vMatrix<float> q_distances = TestDistances3(q, training_vectors_matrix, q_class);
    
    // Overwrite the distance that corresponds to q itself.
    float max_distance_limit = function_image_maximum(&q_distances) * (float) 2.0 + (float) 1.0;
    q_distances(q_index) = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    vint8 counter = 0;
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      for (j = 0; j < q_size; j++)
      {
        vint8 j_index = class_indices[q_class][(vector_size) j];
        if (subject_ids[j_index] == q_subject)
        {
          q_distances(j) = max_distance_limit;
          counter = counter + 1;
        }
      }
    }
    else
    {
      // we still include q itself.
      counter = 1;
    }

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q,
    // among b_class objects
    vMatrix<float> b_distances = TestDistances3(q, training_vectors_matrix, b_class);
    float b_max_distance_limit = function_image_maximum(&b_distances) * (float) 2.0 + (float) 1.0;
    vint8 b_counter = 0;
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      for (j = 0; j < b_size; j++)
      {
        vint8 j_index = class_indices[b_class][(vector_size) j];
        if (subject_ids[j_index] == q_subject)
        {
          b_distances(j) = b_max_distance_limit;
          b_counter = b_counter + 1;
        }
      }
    }

    vint8 max_rank = Min(rank_limit, Min(q_size - counter, b_size - b_counter));
    if (max_rank <= 0)
    {
      exit_error("Error:  max_rank = %li\n", max_rank);
    }
    vint8 rank = function_random_vint8(1, max_rank);
    vint8 a_index = 0;
    vint8 rank2 = rank;
    // a special case for pendigits
//    if ((training_number == 7494) && (test_number == 3498) &&
//        (classes == 10) && (attributes == 16))
//    {
//      rank2 = 7;
//    }
    float a_distance = kth_smallest_cb(rank2, &q_distances, &a_index);
    vint8 a = class_indices[q_class][(vector_size) a_index];
    if (a == q)
    {
      exit_error("Error: a = q\n");
    }

    // Now choose b to be the rank-nearest neighbor of q among b-class
    // objects.
    vint8 b_index = 0;
    float b_distance = kth_smallest_cb(rank, &b_distances, &b_index);
    vint8 b = class_indices[b_class][(vector_size) b_index];

    result[i][0] = q;
    result[i][1] = a;
    result[i][2] = b;
  }

  return result_matrix;
}


// AnalyzeTriples is a preprocessing function called for ResampleTriples2,
// but it can also be used in its own right, to get useful statistics
// about triples.
// bad_qs will receive indices of objects that have "bad" neighbors,
// i.e. different-class objects closest than the same-class nearest 
// neighbor. good_qs will receive objects that have "good" neighobrs, 
// i.e. different-class objects farther away from q than its same-class
// nearest neighbor. Note that bad_qs and good_qs will probably overlap.
// bad_bs(q) is a sampling of bad neighbors of q. If bad_bs->Rows() is
// greater than the number of bad neighbors of q, only the leftmost
// cols of the row bad_bs(q) will contain valid entries. Similar for
// good_bs. bad_numbers(q) is the number of bad_neighbors of q, 
// good_numbers(q) is the number of good neighbors of q, and 
// same_class_nns(q) is the index of the same-class nearest neighbor
// of q. Note that all these arguments are output arguments, they
// don't hold any useful information when this function is called.
vint8 vBnn::AnalyzeTriples()
{
  // First, initialize some of the matrices.
  // How large should they be? We allocate at most 
  // 10MB for bad_bs and 10MB for good_bs
  const vint8 max_bytes = 10000000; // 10 million
  vint8 bytes_per_triple = sizeof(vint8) * 3;
  vint8 max_triples = max_bytes / bytes_per_triple;
  vint8 triples_per_q = max_triples / training_number;
  vint8 cols = Min(training_number, triples_per_q);

  bad_bs = vMatrix<vint8>(training_number, cols);
  good_bs = vMatrix<vint8>(training_number, cols);
  tie_bs = vMatrix<vint8>(training_number, 10);
  bad_numbers = vMatrix<vint8>(1, training_number);
  good_numbers = vMatrix<vint8>(1, training_number);
  tie_numbers = vMatrix<vint8>(1, training_number);
  good_a_numbers = vMatrix<vint8>(1, training_number);
  same_class_nn = vMatrix<vint8>(1, training_number);
  other_class_nn = vMatrix<vint8>(1, training_number);

  vArray2(vint8) bad_bsm = bad_bs.Matrix2();
  vArray2(vint8) good_bsm = good_bs.Matrix2();
  vArray2(vint8) tie_bsm = tie_bs.Matrix2();
  vArray(vint8) bad_numbersm = bad_numbers.Matrix();
  vArray(vint8) good_numbersm = good_numbers.Matrix();
  vArray(vint8) tie_numbersm = tie_numbers.Matrix();
  vArray(vint8) good_a_numbersm = good_a_numbers.Matrix();
  vArray(vint8) same_class_nnm = same_class_nn.Matrix();
  vArray(vint8) other_class_nnm = other_class_nn.Matrix();

  vector<vint8> good_qsv, bad_qsv;

  vMatrix<vint8> temp_good_matrix(1, training_number);
  vMatrix<vint8> temp_bad_matrix(1, training_number);
  vMatrix<vint8> temp_tie_matrix(1, training_number);
  vArray(vint8) temp_good = temp_good_matrix.Matrix();
  vArray(vint8) temp_bad = temp_bad_matrix.Matrix();
  vArray(vint8) temp_tie = temp_tie_matrix.Matrix();

  bad_triples = 0;
  tie_triples = 0;

  vint8 q;
  for (q = 0; q < training_number; q++)
  {
    // Get distances from q to all objects.
    vMatrix<float> q_distancesp = TestDistances2(q, training_vectors_matrix);
    vArray(float) q_distances = q_distancesp.Matrix();
    vint8 q_class = training_labels[q];
    vint8 q_size = class_indices[q_class].size();
    
    // Overwrite the distance that corresponds to q itself.
    float max_distance_limit = function_image_maximum(&q_distancesp) * (float) 2.0 + (float) 1.0;
    q_distances[q] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      vint8 j;
      for (j = 0; j < training_number; j++)
      {
        if (subject_ids[j] == q_subject)
        {
          q_distances[j] = max_distance_limit;
        }
      }
    }

    // find the same-class and different-class nearest neighbors of q.
    float same_min_distance = max_distance_limit;
    float other_min_distance = max_distance_limit;
    vint8 same_min_index = -1;
    vint8 other_min_index = -1;
    vint8 i;
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      if (training_labels[i] == q_class)
      {
        if (distance < same_min_distance)
        {
          same_min_distance = distance;
          same_min_index = i;
        }
      }
      else
      {
        if (distance < other_min_distance)
        {
          other_min_distance = distance;
          other_min_index = i;
        }
      }
    }

    if ((same_min_index == -1) || (other_min_index == -1))
    {
      exit_error("Error: impossible\n");
    }

    same_class_nnm[q] = same_min_index;
    other_class_nnm[q] = other_min_index;
    
    vint8 good_counter = 0, bad_counter = 0;
    vint8 tie_counter = 0, good_a_counter = 0;
    // now, count good neighbors and bad neighbors.
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      if (training_labels[i] == q_class)
      {
        if (distance < other_min_distance)
        {
          good_a_counter = good_a_counter + 1;
        }
        continue;
      }

      // We get here if i is other class than q.
      if (distance < same_min_distance)
      {
        temp_bad[bad_counter] = i;
        bad_counter = bad_counter + 1;
      }
      else if (distance == same_min_distance)
      {
        temp_tie[tie_counter] = i;
        tie_counter = tie_counter + 1;
      }
      else
      {
        temp_good[good_counter] = i;
        good_counter = good_counter + 1;
      }
    }

    good_numbersm[q] = good_counter;
    bad_numbersm[q] = bad_counter;
    tie_numbersm[q] = tie_counter;
    good_a_numbersm[q] = good_a_counter;
    bad_triples = bad_triples + bad_counter;
    tie_triples = tie_triples + tie_counter;
    if (good_counter > 0)
    {
      good_qsv.push_back(q);
    }
    if (bad_counter > 0)
    {
      bad_qsv.push_back(q);
    }

    // Sample good other-class neighbors of q;
    vint8 sample_number = Min(cols, good_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, good_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 good_b = temp_good[pick];
        good_bsm[q][i] = good_b;
      }
    }

    // Sample bad other-class neighbors of q.
    sample_number = Min(cols, bad_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, bad_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 bad_b = temp_bad[pick];
        bad_bsm[q][i] = bad_b;
      }
    }

    // Sample tie other-class neighbors of q.
    vint8 tie_cols = tie_bs.Cols();
    sample_number = Min(tie_cols, tie_counter);
    if (sample_number > 0)
    {
      vMatrix<vint8> picks = sample_without_replacement(0, tie_counter-1, 
                                                        sample_number);
      for (i = 0; i < sample_number; i++)
      {
        vint8 pick = picks(i);
        vint8 tie_b = temp_tie[pick];
        tie_bsm[q][i] = tie_b;
      }
    }
  }

  good_qs = matrix_from_vector(&good_qsv);
  bad_qs = matrix_from_vector(&bad_qsv);

  return 1;
}


// Here triples are sampled deterministically: For each object, we
// make a single triple consisting of the object, its same-class nn
// and its other-class nn.
vMatrix<vint8> vBoostNN::ResampleTriples3()
{
  vint8 number = training_number;
  vMatrix<vint8> result(number, 3);

  vint8 q;
  for (q = 0; q < training_number; q++)
  {
    // Get distances from q to all objects.
    v3dMatrix<float> q_distancesp = TestDistances4(q, training_vectors_matrix,
                                                     &cleaned_indices, &cleaned_factors);
    vArray(float) q_distances = q_distancesp.Matrix();
    vint8 q_class = training_labels[q];
    vint8 q_size = class_indices[q_class].size();
    
    // Overwrite the distance that corresponds to q itself.
    float max_distance_limit = function_image_maximum(&q_distancesp) * (float) 2.0 + (float) 1.0;
    q_distances[q] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      vint8 j;
      for (j = 0; j < training_number; j++)
      {
        if (subject_ids[j] == q_subject)
        {
          q_distances[j] = max_distance_limit;
        }
      }
    }

    // find the same-class and different-class nearest neighbors of q.
    float same_min_distance = max_distance_limit;
    float other_min_distance = max_distance_limit;
    vint8 same_min_index = -1;
    vint8 other_min_index = -1;
    vint8 i;
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      if (training_labels[i] == q_class)
      {
        if (distance < same_min_distance)
        {
          same_min_distance = distance;
          same_min_index = i;
        }
      }
      else
      {
        if (distance < other_min_distance)
        {
          other_min_distance = distance;
          other_min_index = i;
        }
      }
    }

    if ((same_min_index == -1) || (other_min_index == -1))
    {
      exit_error("Error: impossible\n");
    }

    vint8 a = same_min_index;
    vint8 b = other_min_index;
    result(q, 0) = q;
    result(q, 1) = a;
    result(q, 2) = b;
  }

  return result;
}


// Here triples are also sampled deterministically. For each object,
// we make as many triples as the number of other classes. Each triple
// consists of the object, its same-class nn, and its nn from each
// of the other classes.
vMatrix<vint8> vBoostNN::ResampleTriples4()
{
  vint8 number = training_number * (classes - 1);
  vMatrix<vint8> result(number, 3);

  vint8 q;
  for (q = 0; q < training_number; q++)
  {
    // Get distances from q to all objects.
    v3dMatrix<float> q_distancesp = TestDistances4(q, training_vectors_matrix,
                                                     &cleaned_indices, &cleaned_factors);
    vArray(float) q_distances = q_distancesp.Matrix();
    vint8 q_class = training_labels[q];
    vint8 q_size = class_indices[q_class].size();
    
    // Overwrite the distance that corresponds to q itself.
    float max_distance_limit = function_image_maximum(&q_distancesp) * (float) 2.0 + (float) 1.0;
    q_distances[q] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      vint8 j;
      for (j = 0; j < training_number; j++)
      {
        if (subject_ids[j] == q_subject)
        {
          q_distances[j] = max_distance_limit;
        }
      }
    }

    // find the same-class and different-class nearest neighbors of q.
    vMatrix<float> min_distances(1, classes+1);
    function_enter_value(&min_distances, max_distance_limit);
    vMatrix<vint8> min_indices(1, classes+1);
    function_enter_value(&min_indices, (vint8) -1);

    vint8 i;
    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      vint8 i_class = training_labels[i];
      if (distance < min_distances(i_class))
      {
        min_distances(i_class) = distance;
        min_indices(i_class) = i;
      }
    }

    vint8 base = q * (classes - 1);
    vint8 counter = 0;
    vint8 a = min_indices(q_class);
    for (i = 1; i <= classes; i++)
    {
      if (i == q_class)
      {
        continue;
      }
      vint8 b = min_indices(i);
      result(base+counter, 0) = q;
      result(base+counter, 1) = a;
      result(base+counter, 2) = b;
      counter++;
    }
  }

  return result;
}


// Here we exclude training objects that have too many bad neighbors.
vMatrix<vint8> vBoostNN::ResampleTriples5()
{
  vector<vint8> qs, as, bs;

  vint8 q, i;
  for (q = 0; q < training_number; q++)
  {
    // Get distances from q to all objects.
    v3dMatrix<float> q_distancesp = TestDistances4(q, training_vectors_matrix,
                                                     &cleaned_indices, &cleaned_factors);
    vArray(float) q_distances = q_distancesp.Matrix();
    vint8 q_class = training_labels[q];
    vint8 q_size = class_indices[q_class].size();
    
    // Overwrite the distance that corresponds to q itself.
    float max_distance_limit = function_image_maximum(&q_distancesp) * (float) 2.0 + (float) 1.0;
    q_distances[q] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 q_subject = subject_ids[q];
      vint8 j;
      for (j = 0; j < training_number; j++)
      {
        if (subject_ids[j] == q_subject)
        {
          q_distances[j] = max_distance_limit;
        }
      }
    }

    // find the same-class and different-class nearest neighbors of q.
    vMatrix<float> min_distances(1, classes+1);
    function_enter_value(&min_distances, max_distance_limit);
    vMatrix<vint8> min_indices(1, classes+1);
    function_enter_value(&min_indices, (vint8) -1);

    for (i = 0; i < training_number; i++)
    {
      float distance = q_distances[i];
      vint8 i_class = training_labels[i];
      if (distance < min_distances(i_class))
      {
        min_distances(i_class) = distance;
        min_indices(i_class) = i;
      }
    }

    // count the number of other-class neighbors that are closer to q
    // than its same-class nn.
    float same_min_distance = min_distances(q_class);
    vint8 bad_counter = 0;
    for (i = 0; i < training_number; i++)
    {
      if ((q_distances[i] <= same_min_distance) &&
          (training_labels[i] != q_class))
      {
        bad_counter++;
      }
    }
    if (bad_counter > 5)
    {
      continue;
    }

    vint8 base = q * (classes - 1);
    vint8 counter = 0;
    vint8 a = min_indices(q_class);
    for (i = 1; i <= classes; i++)
    {
      if (i == q_class)
      {
        continue;
      }
      vint8 b = min_indices(i);
      qs.push_back(q);
      as.push_back(a);
      bs.push_back(b);
    }
  }

  vint8 number = qs.size();
  vMatrix<vint8> result(number, 3);
  for (i = 0; i < number; i++)
  {
    result(i, 0) = qs[(vector_size) i];
    result(i, 1) = as[(vector_size) i];
    result(i, 2) = bs[(vector_size) i];
  }

  return result;
}


// This function uses ResampleTriples to create a new training and
// validation set of triples. We must run the already chosen classifiers
// on the new training triples, to set the training weights correctly.
vint8 vBoostNN::NewTrainingSet(vint8 max_k)
{
  training_triples_matrix = ResampleTriples(training_triple_number, max_k);
  training_triples = training_triples_matrix.Matrix2();
  validation_triples_matrix = ResampleTriples(validation_triple_number, max_k);
  validation_triples = validation_triples_matrix.Matrix2();

  InitialWeights();
  training_margins_matrix = vMatrix<float>(1, training_triple_number);
  training_margins = training_margins_matrix.Matrix();
  validation_margins_matrix = vMatrix<float>(1, validation_triple_number);
  validation_margins = validation_margins_matrix.Matrix();
  function_enter_value(&training_margins_matrix, (float) 0);
  function_enter_value(&validation_margins_matrix, (float) 0);
  alpha_limits_matrix = ComputeAlphaLimits(training_triples_matrix);
  alpha_limits = alpha_limits_matrix.Matrix2();

  vint8 classifier_number = distance_indices.size();
  vint8 i;
  for (i = 0; i < classifier_number; i++)
  {
    // We don't use cleaned indices and cleaned weights to avoid getting
    // large alphas that may cause numerical problems.
    vint8 index = distance_indices[(vector_size) i];
    float alpha = distance_factors[(vector_size) i];
    // We check if alpha needs to be modified. Possibly
    // alpha will be too high and z will not be computed correctly.
    // However, by modifying it, the "clean" classifier does not
    // correspond any more to the classifier with repetitions. If this
    // case arises in practice, I should make sure I do something reasonable.
    if (alpha < alpha_limits[0][index])
    {
      function_warning("Warning: (may cause bugs): in NewTrainingSet, alpha = %f, limit = %f\n",
                alpha, alpha_limits[0][index]);
      alpha = alpha_limits[0][index];
    }
    if (alpha > alpha_limits[1][index])
    {
      function_warning("Warning: (may cause bugs): in NewTrainingSet, alpha = %f, limit = %f\n",
                alpha, alpha_limits[1][index]);
      alpha = alpha_limits[1][index];
    }

    AddClassifier(index, alpha);

    /*
    // Reading this line on 08/26/2004, it seems that it may be a bug. I
    // did not fully check because this function is currently not used
    // for anything.
    cleaned_factors[i] = alpha;

    vector<float> results(training_triple_number);
    DistanceResults(index, &results, training_triples_matrix);
    float z = Z(alpha, &results);
    UpdateWeights(&results, z, alpha);
    float margin = ComputeTrainingError(&results, alpha);

    vector<float> validation_results(validation_triple_number);
    DistanceResults(index, &validation_results,
                    validation_triples_matrix);
    ComputeValidationError(&validation_results, alpha);
    training_errors.push_back(training_error);
    validation_errors.push_back(validation_error);
    */
  }

  return 1;
}


// Undo the last steps.
vint8 vBoostNN::Backtrack(vint8 steps)
{
  // First add the negations of the steps, so that weights and everything
  // else is set right.
  vint8 total_steps = StepsDone();
  if (steps < 0) 
  {
    return 0;
  }
  if (steps > total_steps)
  {
    steps = total_steps;
  }

  vint8 i;
  vint8 start = total_steps - steps;
  vint8 end = total_steps - 1;
  for (i = start; i <= end; i++)
  {
    vint8 index = distance_indices[(vector_size) i];
    float weight = distance_factors[(vector_size) i];
    AddClassifier(index, -weight);
  }

  // Now erase all information about the last 2*steps steps (to include
  // both the negations of steps we have just added, and the original
  // steps that we wanted to erase in the first place).

  for (i = 0; i < 2 * steps; i++)
  {
    distance_indices.pop_back();
    distance_factors.pop_back();
    distance_zs.pop_back();
    training_errors.pop_back();
    validation_errors.pop_back();
    test_errors.pop_back();
    test_triple_errors.pop_back();
  }

  return 1;
}


// This function is used for debugging and understanding what's going on.
// result[0][i] is the i-the smallest weight, and result[1][i] is the index
// of the i-th smallest weight (i.e. the index of the triple with that
// weight).
vMatrix<float> vBnn::SortTrainingWeights()
{
  if (training_triple_number == 0)
  {
    return vMatrix<float>();
  }

  vector<class_couple> weights((vector_size) training_triple_number);
  vint8 i;
  for (i = 0; i < training_triple_number; i++)
  {
    float weight = training_factors[i];
    weights[(vector_size) i] = class_couple(weight, (void *) (long) i);
  }

  std::sort(weights.begin(), weights.end(), couple_less());
  vMatrix<float> result(2, training_triple_number);
  for (i = 0; i < training_triple_number; i++)
  {
    result(0, i) = weights[(vector_size) i].value;
    result(1, i) = (float) ((long) weights[(vector_size) i].object);
  }

  triple_temp = result;
  return result;
}


float vBoostNN::CurrentDistanceWeight(vint8 distance_index)
{
  float result = 0;
  vint8 number = cleaned_indices.size();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index = cleaned_indices[(vector_size) i];
    if (index == distance_index)
    {
      result = result + cleaned_factors[(vector_size) i];
    }
  }

  return result;
}


vMatrix<float> vBoostNN::ClassifierMatrix()
{
  vint8 rows = distance_indices.size();
  vMatrix<float> result(rows, 6);

  vint8 i;
  for (i = 0; i < rows; i++)
  {
    result(i, 0) = (float) distance_indices[(vector_size) i];
    result(i, 1) = distance_factors[(vector_size) i];
    result(i, 2) = distance_zs[(vector_size) i];
    result(i, 3) = training_errors[(vector_size) i];
    result(i, 4) = test_errors[(vector_size) i];
    result(i, 5) = (float) i;
  }
  return result;
}


// Prints some information about this object.
vint8 vBoostNN::PrintSummary()
{
  if (training_number == 0) return 0;
  vPrint("\n");
  vPrint("BoostNN: %li training, %li test, %li classes, %li attributes\n",
          training_number, test_number, classes, attributes);
  vPrint("Triples: %li training, %li validation\n",
          training_triple_number, validation_triple_number);
  vPrint("min_exp = %f, max_exp = %f\n", min_exp, max_exp);
  vPrint("iterations = ");
  iterations.Print();
  vPrint("\n");

  vPrint("weight sum = %.3f\n", SumWeights());
  vPrint("last error = %f, et = %f, correlation = %f\n",
          last_error, last_et, last_correlation);
  vPrint("last_scaled_et = %f, last_scaled_correlation = %f\n",
          last_scaled_et, last_scaled_correlation);
  vPrint("training_error = %f, margin = %f, validation = %f\n",
          training_error, training_margin, validation_error);
  vPrint("test_triple_error = %f, test_error = %f\n\n", 
          test_triple_error, test_error);

  vint8 steps = StepsDone();
  if (steps > 0)
  {
    float last_index = (float) distance_indices[(vector_size) steps-1];
    float last_weight = distance_factors[(vector_size) steps-1];
    float last_z = distance_zs[(vector_size) steps-1];
    vPrint("last_index = %f, last_weight = %f, last_z = %f\n",
            last_index, last_weight, last_z);
  }
  vPrint("%li steps, net classifiers = %li\n", steps, cleaned_indices.size());

  vPrint("\n");
  return 1;
}


vint8 vBoostNN::PrintClassifier()
{
  vint8 i;

  vint8 number = distance_indices.size();
  vPrint("classifier, step by step:\n");
  for (i = 0; i < number; i++)
  {
    vPrint("%4li: %4li, %f\n", i, distance_indices[(vector_size) i], distance_factors[(vector_size) i]);
  }
  vPrint("\n");

  number = cleaned_indices.size();
  vPrint("classifier, without repetitions:\n");
  for (i = 0; i < number; i++)
  {
    vPrint("%4li: %4li, %f\n", i, cleaned_indices[(vector_size) i], cleaned_factors[(vector_size) i]);
  }

  vPrint("\n");
  return 1;
}


// KnnError returns the error, on the objects described
// as columns of evaluation_set, of the current k-nearest 
// neighbor clssifier described by cleaned_indices and
// cleaned_factors. discard_first should be 1 if 
// the evaluation set is a subset of the training set and
// 0 if the evaluation set has no intersection with the training
// set. It tells the program to discard the nearest neighbor, because
// that would be the query object itself.
float vBnn::KnnError(vMatrix<float> evaluation_set, 
                     vMatrix<vint8> evaluation_labels,
                     vint8 k, vint8 discard_first)
{
  vint8_matrix active_matrix;
  vint8_matrix previous_labels;
  if (discard_first == 1)
  {
    active_matrix = active_training;
    previous_labels = previous_training_labels;
    if (evaluation_labels.length() != training_number)
    {
      exit_error("error: the code must be revised to handle this\n");
    }
  }
  else
  {
    active_matrix = active_test;
    previous_labels = previous_test_labels;
    if (evaluation_labels.length() != test_number)
    {
      exit_error("error: the code must be revised to handle this\n");
    }
  }

  vint8 object_number = evaluation_set.Rows();
  if (object_number < 1)
  {
    function_warning("Warning: in KnnError, object_number = %li\n", object_number);
    return -1;
  }
  if (evaluation_set.Cols() != attributes)
  {
    function_warning("Warning: in KnnError, evaluation set has %li != %li cols\n",
              evaluation_set.Cols(), attributes);
    return -1;
  }
  if (k < 1)
  {
    return -1;
  }

  // counter will count misclassified objects.
  float counter = 0;
  vint8 threshold_index = 0;
  vint8 i;
  vMatrix<vint8> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(vint8) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
  vint8 negative_counter = 0;
  vint8 tie_counter = 0;
  for (i = 0; i < object_number; i++)
  {
    function_print("processing object %li of %li\r", i+1, object_number);
    vint8 object_class = evaluation_labels(i);

    if (active_matrix (i) == 0)
    {
      vint8 previous_label = previous_labels(i);
      if (previous_label != object_class)
      {
        negative_counter = negative_counter + 1;
      }
      continue;
    }
    vMatrix<float> distances = TestDistances2(i, evaluation_set);
    
    // if this is training error, we should handle subject ids.
    if (discard_first == 1)
    {
      // Overwrite the distance that corresponds to i itself.
      float max_distance_limit = function_image_maximum(&distances) * (float) 2.0 + (float) 1.0;
      distances(i) = max_distance_limit;

      // Also, overwrite the distances that correspond to objects 
      // coming from the same subjects as the subject who produced q.
      if (subject_ids_set == 1)
      {
        vint8 i_subject = subject_ids[i];
        vint8 j;
        for (j = 0; j < training_number; j++)
        {
//          if ((subject_ids[j] == i_subject) && 
//              (training_labels[j] == object_class))
          if (subject_ids[j] == i_subject)
          {
            distances(j) = max_distance_limit;
          }
        }
      }
    }

    vint8 rank = k;
    // a special case for pendigits
//    if ((training_number == 7494) && (test_number == 3498) &&
//        (classes == 10) && (attributes == 16) && (discard_first == 1))
//    {
//      rank = k + 7;
//    }
    float threshold_distance = kth_smallest_cb(rank, &distances,
                                               &threshold_index);
    if (threshold_distance < 0)
    {
      negative_counter++;
    }
    
    // Now, collect votes.
    function_enter_value(&votes_matrix, (vint8) 0);
    function_enter_value(&max_distances_matrix, (float) -1000000000);
    vArray(float) distance_data = distances.Matrix();
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      float distance = distance_data[j];
      if (distance <= threshold_distance)
      {
        vint8 j_class = training_labels[j];
        votes[j_class] = votes[j_class] + 1;
        if (max_distances[j_class] < distance)
        {
          max_distances[j_class] = distance;
        }
      }
    }

    // another special instruction for pendigits:
    //votes_matrix(object_class) -= (rank - k);
    // Find the max number of votes received:
    vint8 max_votes = function_image_maximum(&votes_matrix);

    // Find all classes that received the max number of votes.
    vector<vint8> candidate_classes;
    vint8 c;
    for (c = 1; c <= classes; c++)
    {
      if (votes[c] == max_votes)
      {
        candidate_classes.push_back(c);
      }
    }

    if (candidate_classes.size() == 0)
    {
      exit_error("Error: impossible, 0 candidate classes\n");
    }

    float min_max_distance = max_distances[candidate_classes[0]];
    vint8 candidates = candidate_classes.size();
    // Find the min max distance among all candidates (to break ties).
    for (c = 1; c < candidates; c++)
    {
      vint8 candidate_class = candidate_classes[(vector_size) c];
      float current_max = max_distances[candidate_class];
      if (current_max < min_max_distance)
      {
        min_max_distance = current_max;
      }
    }

    // Now go through candidates again, and only keep the one whose
    // max distance is min_max_distance;
    vector<vint8> final_candidates;
    for (c = 0; c < candidates; c++)
    {
      vint8 candidate_class = candidate_classes[(vector_size) c];
      float current_max = max_distances[candidate_class];
      if (current_max == min_max_distance)
      {
        final_candidates.push_back(candidate_class);
      }
    }

    vint8 final_size = final_candidates.size();
    if (final_size == 0)
    {
      exit_error("Error: impossible, 0 final candidate classes\n");
    }

    vint8 winning_class = -1;
    if (final_size == 1)
    {
      winning_class = final_candidates[0];
    }
    else
    {
      // Here we have a total tie, we decide randomly.
      vint8 random_pick = function_random_vint8(0, final_size - 1);
      winning_class = final_candidates[(vector_size) random_pick];
      tie_counter = tie_counter + 1;
    }
    
    if (winning_class != object_class)
    {
      counter = counter+1;
    }
  }

  vPrint("negative_counter = %li, ties = %li, of %li\n", 
          negative_counter, tie_counter, object_number);
  float result = counter / (float) object_number;
  return result;
}


// Returns a matrix whose k-th entry is the k-nn nearest
// neighbor accuracy, using L1 distance. Entry 0 is not meaningful.
vMatrix<float> vBnn::KnnTrainingErrors(vint8 max_k)
{
  if (StepsDone() == 0)
  {
    return float_matrix();
  }

  vMatrix<float> result(1, max_k + 1);
  function_enter_value(&result, (float) 0);
  result(0) = 2;

  // counter will count misclassified objects.
  vint8 threshold_index = 0;
  vint8 i, j, k;
  vMatrix<vint8> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(vint8) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
  vint8 negative_counter = 0;
  vint8 tie_counter = 0;
  vint8 distance_tie_counter = 0;

  // here we store the label assigned to each object 
  // using knn classification for k = 1,..., max_k.
  vint8_matrix classification_labels(max_k+1, training_number);
  function_enter_value(& classification_labels, (vint8) 0);

  function_print("\n");
  for (i = 0; i < training_number; i++)
  {
    function_print("processing training object %li of %li\r", i+1, training_number);
    vint8 object_class = training_labels[i];

    if (active_training (i) == 0)
    {
      vint8 previous_label = previous_training_labels(i);
      if (previous_label != object_class)
      {
        for (k = 1; k <= max_k; k++)
        {
          classification_labels(k,i) = previous_label;
          result(k) = result(k) + 1.0f;
        }
      }
      continue;
    }

    vMatrix<float> distancesm = TestDistances2(i, training_vectors_matrix);
    vArray(float) distances = distancesm.Matrix();
    
    // Overwrite the distance that corresponds to i itself.
    float max_distance_limit = function_image_maximum(&distancesm) * (float) 2.0 + (float) 1.0;
    distances[i] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 i_subject = subject_ids[i];
      for (j = 0; j < training_number; j++)
      {
//        if ((subject_ids[j] == i_subject) && 
//            (training_labels[j] == object_class))
        if (subject_ids[j] == i_subject)
        {
          distances[j] = max_distance_limit;
        }
      }
    }

    // a special case for pendigits
//    if ((training_number == 7494) && (test_number == 3498) &&
//        (classes == 10) && (attributes == 16) && (discard_first == 1))
//    {
//      rank = k + 7;
//    }
    float threshold_distance = kth_smallest_cb(max_k, &distancesm,
                                               &threshold_index);
    if (threshold_distance < 0)
    {
      negative_counter++;
    }
    
    vector<float> nn_distances;
    vector<vint8> nn_classes;

    // collect the max_k neighbors (and neighbors after that that are
    // tied to the max_k neighbor).
    for (j = 0; j < training_number; j++)
    {
      if (distances[j] <= threshold_distance)
      {
        nn_distances.push_back(distances[j]);
        nn_classes.push_back(training_labels[j]);
      }
    }

    vint8 number = nn_distances.size();
    // Now, collect votes, and get the recognition, for each k.
    for (k = 1; k <= max_k; k++)
    {
      function_enter_value(&votes_matrix, (vint8) 0);
      function_enter_value(&max_distances_matrix, (float) -1000000000);
      long junk = 0;
      float k_threshold_distance = kth_smallest_ca((long) k, &nn_distances, &junk);
      vint8 counter = 0;
      for (j = 0; j < number; j++)
      {
        float distance = nn_distances[(vector_size) j];
        if (distance <= k_threshold_distance)
        {
          counter++;
          vint8 j_class = nn_classes[(vector_size) j];
          votes[j_class] = votes[j_class] + 1;
          if (max_distances[j_class] < distance)
          {
            max_distances[j_class] = distance;
          }
        }
      }

      if (counter > k)
      {
        distance_tie_counter++;
      }
      // another special instruction for pendigits:
  //    votes_matrix(object_class) -= (rank - k);
      // Find the max number of votes received:
      vint8 max_votes = function_image_maximum(&votes_matrix);

      // Find all classes that received the max number of votes.
      vector<vint8> candidate_classes;
      vint8 c;
      for (c = 1; c <= classes; c++)
      {
        if (votes[c] == max_votes)
        {
          candidate_classes.push_back(c);
        }
      }

      if (candidate_classes.size() == 0)
      {
        exit_error("Error: impossible, 0 candidate classes\n");
      }

      float min_max_distance = max_distances[candidate_classes[0]];
      vint8 candidates = candidate_classes.size();
      // Find the min max distance among all candidates (to break ties).
      for (c = 1; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max < min_max_distance)
        {
          min_max_distance = current_max;
        }
      }

      // Now go through candidates again, and only keep the one whose
      // max distance is min_max_distance;
      vector<vint8> final_candidates;
      vint8 q_class_flag = 0;
      for (c = 0; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max == min_max_distance)
        {
          final_candidates.push_back(candidate_class);
          if (candidate_class == object_class)
          {
            q_class_flag = 1;
          }
        }
      }

      vint8 final_size = final_candidates.size();
      vint8 decision = function_random_vint8(0, final_size-1);
      classification_labels(k,i) = decision;
      if (q_class_flag == 0)
      {
        result(k) = result(k) + 1;
      }
      else
      {
        if (final_size > 1)
        {
          tie_counter = tie_counter + 1;
          float final = (float) final_size;
          result(k) = result(k) + (final - (float) 1.0) / final;
        }
      }
    }
  }
  function_print("\n");

  vPrint("distance ties = %li, ties = %li\n", 
          distance_tie_counter, tie_counter);
  for (k = 1; k <= max_k; k++)
  {
    result(k) = result(k) / (float) training_number;
  }

  knn_training_errors = result;
  vint8 junk = 0;
  best_knn_training = function_image_minimum3(&result, &junk, &best_k);
  training_classification = classification_labels.horizontal_line(best_k);
  if (knn_test_errors.Size() > best_k)
  {
    best_knn_test = knn_test_errors(best_k);
  }

  return result;
}


// Returns a matrix whose k-th entry is the k-nn nearest
// neighbor accuracy, using L2 distance. Entry 0 is not meaningful.
vMatrix<float> vBoostNN::KnnTrainingErrors2(vint8 max_k)
{
  vMatrix<float> result(1, max_k + 1);
  function_enter_value(&result, (float) 0);
  result(0) = 2;

  // counter will count misclassified objects.
  vint8 threshold_index = 0;
  vint8 i, j, k;
  vMatrix<vint8> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(vint8) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
  vint8 negative_counter = 0;
  vint8 tie_counter = 0;
  vint8 distance_tie_counter = 0;
  for (i = 0; i < training_number; i++)
  {
    vint8 object_class = training_labels[i];
    vMatrix<float> distancesm = TestDistances2b(i, training_vectors_matrix,
                                                &cleaned_indices,
                                                &cleaned_factors);
    vArray(float) distances = distancesm.Matrix();
    
    // Overwrite the distance that corresponds to i itself.
    float max_distance_limit = function_image_maximum(&distancesm) * (float) 2.0 + (float) 1.0;
    distances[i] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 i_subject = subject_ids[i];
      for (j = 0; j < training_number; j++)
      {
//        if ((subject_ids[j] == i_subject) && 
//            (training_labels[j] == object_class))
        if (subject_ids[j] == i_subject)
        {
          distances[j] = max_distance_limit;
        }
      }
    }

    // a special case for pendigits
//    if ((training_number == 7494) && (test_number == 3498) &&
//        (classes == 10) && (attributes == 16) && (discard_first == 1))
//    {
//      rank = k + 7;
//    }
    float threshold_distance = kth_smallest_cb(max_k, &distancesm,
                                               &threshold_index);
    if (threshold_distance < 0)
    {
      negative_counter++;
    }
    
    vector<float> nn_distances;
    vector<vint8> nn_classes;

    // collect the max_k neighbors (and neighbors after that that are
    // tied to the max_k neighbor).
    for (j = 0; j < training_number; j++)
    {
      if (distances[j] <= threshold_distance)
      {
        nn_distances.push_back(distances[j]);
        nn_classes.push_back(training_labels[j]);
      }
    }

    vint8 number = nn_distances.size();
    // Now, collect votes, and get the recognition, for each k.
    for (k = 1; k <= max_k; k++)
    {
      function_enter_value(&votes_matrix, (vint8) 0);
      function_enter_value(&max_distances_matrix, (float) -1000000000);
      long junk = 0;
      float k_threshold_distance = kth_smallest_ca((long) k, &nn_distances, &junk);
      vint8 counter = 0;
      for (j = 0; j < number; j++)
      {
        float distance = nn_distances[(vector_size) j];
        if (distance <= k_threshold_distance)
        {
          counter++;
          vint8 j_class = nn_classes[(vector_size) j];
          votes[j_class] = votes[j_class] + 1;
          if (max_distances[j_class] < distance)
          {
            max_distances[j_class] = distance;
          }
        }
      }

      if (counter > k)
      {
        distance_tie_counter++;
      }
      // another special instruction for pendigits:
  //    votes_matrix(object_class) -= (rank - k);
      // Find the max number of votes received:
      vint8 max_votes = function_image_maximum(&votes_matrix);

      // Find all classes that received the max number of votes.
      vector<vint8> candidate_classes;
      vint8 c;
      for (c = 1; c <= classes; c++)
      {
        if (votes[c] == max_votes)
        {
          candidate_classes.push_back(c);
        }
      }

      if (candidate_classes.size() == 0)
      {
        exit_error("Error: impossible, 0 candidate classes\n");
      }

      float min_max_distance = max_distances[candidate_classes[0]];
      vint8 candidates = candidate_classes.size();
      // Find the min max distance among all candidates (to break ties).
      for (c = 1; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max < min_max_distance)
        {
          min_max_distance = current_max;
        }
      }

      // Now go through candidates again, and only keep the one whose
      // max distance is min_max_distance;
      vector<vint8> final_candidates;
      vint8 q_class_flag = 0;
      for (c = 0; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max == min_max_distance)
        {
          final_candidates.push_back(candidate_class);
          if (candidate_class == object_class)
          {
            q_class_flag = 1;
          }
        }
      }

      if (q_class_flag == 0)
      {
        result(k) = result(k) + 1;
      }
      else
      {
        vint8 final_size = final_candidates.size();
        if (final_size == 0)
        {
          exit_error("Error: impossible, 0 final candidate classes\n");
        }

        if (final_size > 1)
        {
          tie_counter = tie_counter + 1;
          float final = (float) final_size;
          result(k) = result(k) + (final - (float) 1.0) / final;
        }
      }
    }
  }

  vPrint("distance ties = %li, ties = %li\n", 
          distance_tie_counter, tie_counter);
  for (k = 1; k <= max_k; k++)
  {
    result(k) = result(k) / (float) training_number;
  }

  knn_training_errors = result;
  vint8 junk = 0;
  best_knn_training = function_image_minimum3(&result, &junk, &best_k);
  if (knn_test_errors.Size() > best_k)
  {
    best_knn_test = knn_test_errors(best_k);
  }

  return result;
}



// Same thing, but for the test error rate, which is faster to
// compute because of test_distances_matrix;
vMatrix<float> vBoostNN::KnnTestErrors2(vint8 max_k)
{
  vMatrix<float> result(1, max_k + 1);
  function_enter_value(&result, (float) 0);
  result(0) = 2;

  // counter will count misclassified objects.
  float counter = 0;
  vint8 threshold_index = 0;
  vint8 i, j, k;
  vMatrix<vint8> votes_matrix(1, classes+1);
  // max_distances[t] will store the highest distance found between
  // i and one of its k nearest neighbors that is of class t.
  vMatrix<float> max_distances_matrix (1, classes+1);
  vArray(vint8) votes = votes_matrix.Matrix();
  vArray(float) max_distances = max_distances_matrix.Matrix();
  vint8 negative_counter = 0;
  vint8 tie_counter = 0;
  for (i = 0; i < test_number; i++)
  {
    vint8 object_class = test_labels[i];
    vMatrix<float> distancesm = TestDistances2b(i, test_vectors_matrix,
                                                &cleaned_indices,
                                                &cleaned_factors);
    vArray(float) distances = distancesm.Matrix();
    
    
    float threshold_distance = kth_smallest_cb(max_k, &distancesm,
                                               &threshold_index);
    if (threshold_distance < 0)
    {
      negative_counter++;
    }
    
    vector<float> nn_distances;
    vector<vint8> nn_classes;

    // collect the max_k neighbors (and neighbors after that that are
    // tied to the max_k neighbor).
    for (j = 0; j < training_number; j++)
    {
      if (distances[j] <= threshold_distance)
      {
        nn_distances.push_back(distances[j]);
        nn_classes.push_back(training_labels[j]);
      }
    }

    vint8 number = nn_distances.size();
    // Now, collect votes, and get the recognition, for each k.
    for (k = 1; k <= max_k; k++)
    {
      function_enter_value(&votes_matrix, (vint8) 0);
      function_enter_value(&max_distances_matrix, (float) -1000000000);
      long junk = 0;
      float k_threshold_distance = kth_smallest_ca((long) k, &nn_distances, &junk);
      for (j = 0; j < number; j++)
      {
        float distance = nn_distances[(vector_size) j];
        if (distance <= k_threshold_distance)
        {
          vint8 j_class = nn_classes[(vector_size) j];
          votes[j_class] = votes[j_class] + 1;
          if (max_distances[j_class] < distance)
          {
            max_distances[j_class] = distance;
          }
        }
      }

      // Find the max number of votes received:
      vint8 max_votes = function_image_maximum(&votes_matrix);

      // Find all classes that received the max number of votes.
      vector<vint8> candidate_classes;
      vint8 c;
      for (c = 1; c <= classes; c++)
      {
        if (votes[c] == max_votes)
        {
          candidate_classes.push_back(c);
        }
      }

      if (candidate_classes.size() == 0)
      {
        exit_error("Error: impossible, 0 candidate classes\n");
      }

      float min_max_distance = max_distances[candidate_classes[0]];
      vint8 candidates = candidate_classes.size();
      // Find the min max distance among all candidates (to break ties).
      for (c = 1; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max < min_max_distance)
        {
          min_max_distance = current_max;
        }
      }

      // Now go through candidates again, and only keep the one whose
      // max distance is min_max_distance;
      vector<vint8> final_candidates;
      vint8 q_class_flag = 0;
      for (c = 0; c < candidates; c++)
      {
        vint8 candidate_class = candidate_classes[(vector_size) c];
        float current_max = max_distances[candidate_class];
        if (current_max == min_max_distance)
        {
          final_candidates.push_back(candidate_class);
          if (candidate_class == object_class)
          {
            q_class_flag = 1;
          }
        }
      }

      if (q_class_flag == 0)
      {
        result(k) = result(k) + 1;
      }
      else
      {
        vint8 final_size = final_candidates.size();
        if (final_size == 0)
        {
          exit_error("Error: impossible, 0 final candidate classes\n");
        }

        if (final_size > 1)
        {
          tie_counter = tie_counter + 1;
          float final = (float) final_size;
          result(k) = result(k) + (final - (float) 1.0) / final;
        }
      }
    }
  }

  vPrint("ties = %li\n", tie_counter);
  for (k = 1; k <= max_k; k++)
  {
    result(k) = result(k) / (float) test_number;
  }

  knn_test_errors = result;
  if ((best_k >= 1) && (knn_test_errors.Size() > best_k))
  {
    best_knn_test = knn_test_errors(best_k);
  }

  return result;
}


// For the specified training triple (q, a, b,) indexed by triple_index,
// show how many same-class and total objects are closer to q than a,
// and similarly how many are closer to q than b. Hopefully this will
// be a useful measure in deciding whether a given training triple is
// useful or not.
vint8 vBnn::TripleRankInfo(vint8 triple_index)
{
  if ((triple_index < 0) || (triple_index >= training_triple_number))
  {
    return -1;
  }

  vint8 q = training_triples[triple_index][0];
  vint8 a = training_triples[triple_index][1];
  vint8 b = training_triples[triple_index][2];
  vint8 q_class = training_labels[q];

  vMatrix<float> distances_matrix = TestDistances2(q, training_vectors_matrix);
  vArray(float) distances = distances_matrix.Matrix();

  vint8 a_counter = 0, same_a_counter = 0, b_counter = 0, same_b_counter = 0;
  
  float a_distance = distances[a];
  float b_distance = distances[b];

  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    if (i == q) 
    {
      continue;
    }
    
    float distance = distances[i];
    
    if (distance < a_distance)
    {
      a_counter = a_counter + 1;
      if (training_labels[i] == q_class)
      {
        same_a_counter = same_a_counter + 1;
      }
    }

    if (distance < b_distance)
    {
      b_counter = b_counter + 1;
      if (training_labels[i] == q_class)
      {
        same_b_counter = same_b_counter + 1;
      }
    }
  }

  vint8 q_class_size = class_indices[q_class].size();
  vPrint("%7li objects between q and a, %7li (of %7li) same class as q\n",
          a_counter, same_a_counter, q_class_size);
  vPrint("%7li objects between q and b, %7li (of %7li) same class as q\n",
          b_counter, same_b_counter, q_class_size);
  return 1;
}


// We print the k nearest neighbors of the two objects (one in the
// training set and one in the test set) that are indexed by object_index.
vint8 vBnn::KnnInfo(vint8 object_index, vint8 k)
{
  if (k < 1)
  {
    return -1;
  }

  if (object_index < 0)
  {
    return -1;
  }

  if (object_index < test_number)
  {
    vint8 object_class = test_labels[object_index];
    vPrint("test object %li, class = %li:\n", object_index, object_class);
    vint8 threshold_index = 0;
    vMatrix<float> distances_matrix = TestDistances2(object_index, test_vectors_matrix);
    vArray(float) distances = distances_matrix.Matrix();
    float threshold_distance = kth_smallest_cb(k, &distances_matrix,
                                               &threshold_index);
    vector<class_couple> neighbors;
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      float distance = distances[j];
      if (distance <= threshold_distance)
      {
        neighbors.push_back(class_couple(distance, (void *) (long) j));
      }
    }

    std::sort(neighbors.begin(), neighbors.end(), couple_less());
    vint8 number = neighbors.size();
    for (j = 0; j < number; j++)
    {
      vint8 index = (vint8) (long) neighbors[(vector_size) j].object;
      float distance = neighbors[(vector_size) j].value;
      vint8 j_class = training_labels[index];
      vPrint("%3li neighbor: index = %7li, class = %5li, distance = %f\n",
              j, index, j_class, distance);
    }
    vPrint("\n");
  }

  if (object_index < training_number)
  {
    vint8 object_class = training_labels[object_index];
    vPrint("training object %li, class = %li:\n", object_index, object_class);
    vint8 threshold_index = 0;
    vMatrix<float> distances_matrix = TestDistances2(object_index, training_vectors_matrix);
    vArray(float) distances = distances_matrix.Matrix();
    // Overwrite the distance that corresponds to i itself.
    float max_distance_limit = function_image_maximum(&distances_matrix) * (float) 2.0 + (float) 1.0;
    distances[object_index] = max_distance_limit;

    // Also, overwrite the distances that correspond to objects 
    // coming from the same subjects as the subject who produced q.
    if (subject_ids_set == 1)
    {
      vint8 object_subject = subject_ids[object_index];
      vint8 j;
      for (j = 0; j < training_number; j++)
      {
        if ((subject_ids[j] == object_subject) && 
            (training_labels[j] == object_class))
        {
          distances[j] = max_distance_limit;
        }
      }
    }
    float threshold_distance = kth_smallest_cb(k, &distances_matrix,
                                               &threshold_index);
    vector<class_couple> neighbors;
    vint8 j;
    for (j = 0; j < training_number; j++)
    {
      float distance = distances[j];
      if (distance <= threshold_distance)
      {
        neighbors.push_back(class_couple(distance, (void *) (long) j));
      }
    }

    std::sort(neighbors.begin(), neighbors.end(), couple_less());
    vint8 number = neighbors.size();
    for (j = 0; j < number; j++)
    {
      vint8 index = (vint8) (long) neighbors[(vector_size) j].object;
      float distance = neighbors[(vector_size) j].value;
      vint8 j_class = training_labels[index];
      vPrint("%3li neighbor: index = %7li, class = %5li, distance = %f\n",
              j, index, j_class, distance);
    }
    vPrint("\n");
  }

  return 1;
}


// add classifier corresponding to the attribute specified by
// index, and with weight specified by alpha.
vint8 vBoostNN::AddClassifier(vint8 index, float alpha)
{
  if ((index < 0) || (index >= attributes))
  {
    function_warning("Warning: index = %li, attributes = %li\n",
              index, attributes);
    return 0;
  }

  // invalidate results of previous classifications
  best_k = -1;
  training_classification = vint8_matrix();
  test_classification = vint8_matrix();

  vector<float> results((vector_size) training_triple_number);
  DistanceResults(index, &results, training_triples_matrix);
  float z = Z(alpha, &results);

  return AddClassifier3(index, alpha, z);
}


// Finds first the attribute with the lowest weighted error, and 
// adds that.
vint8 vBoostNN::NextStepFast()
{
  if ((training_triple_number == 0) || (attributes == 0))
  {
    return -1;
  }

  vMatrix<float> attribute_errors = AttributeErrors();
  vint8 junk = 0, index = 0;
  float min_error = function_image_minimum3(&attribute_errors, &junk, &index);

  float alpha = 0;
  float z = DistanceZ(index, &alpha);

  return AddClassifier3(index, alpha, z);
}


// Finds the attribute that achieves the smallest z, and chooses
// the next weak classifier to depend on that attribute.
vint8 vBoostNN::NextStep()
{
  if ((training_triple_number == 0) || (attributes == 0))
  {
    return -1;
  }

  // For the ICML 2004 submission, the max number of attributes is 617,
  // and the second max number is 36, so the next line just specifies
  // that, for these datasets, we should use all the attributes. 
  const vint8 candidate_limit = 800;
  vint8 candidate_number = Min(candidate_limit, attributes);
  
  // sample from all possible classifiers
  vMatrix<vint8> candidates = sample_without_replacement(0, attributes-1, 
                                                         candidate_number);

  float best_alpha = 0;
  float alpha = 0;
  float z_min = DistanceZ(candidates(0), &best_alpha);
  vint8 z_min_index = candidates(0);
  vint8 i, ii;

  // find the classifier that minimizes z, among the candidate classifiers.
  for (ii = 1; ii < candidate_number; ii++)
  {
    i = candidates(ii);
    float z = DistanceZ(i, &alpha);
    if (z < z_min)
    {
      z_min = z;
      z_min_index = i;
      best_alpha = alpha;
    }
  }

  return AddClassifier3(z_min_index, best_alpha, z_min);
}


// I don't know if this will work, but what this function does is
// - compute the best factor for each of the coordinates,
// - form a classifier that corresponds to the weighted sum
// of all coordinates (based on these factors) 
// - compute the factor (between zero and one) for this classifier
// - add this classifier piecewise, by adding each of the individual
// components.
vint8 vBoostNN::simultaneous_step()
{
  if ((training_triple_number == 0) || (attributes == 0))
  {
    return -1;
  }
  
  // sample from all possible classifiers
  vMatrix<vint8> candidates = vint8_matrix::Range(0, attributes-1);

  float_matrix factors(1, attributes);
  function_enter_value(& factors, (float) 0);
  float_matrix results (1, training_triple_number);
  function_enter_value(& results, (float) 0);

  // find the best current factor for all classifiers,
  // and get the corresponding result matrix 
  // (including multiplication by that factor)
  vint8 counter;
  printf("\n");
  for (counter = 0; counter < attributes; counter++)
  {
    float alpha = 0.0f;
    float z = DistanceZ(counter, &alpha);
    factors (counter) = alpha;
    float_matrix current_results = distance_results (counter, training_triples_matrix);
    current_results = alpha * current_results;
    results = results + current_results;
    printf("processed attribute %li of %li\r", counter+1, attributes);
  }
  printf("\n");

  // computes the right factor for the combined classifier
  float z, factor;
  MinimizeZ(results, &z, &factor, 0.0f, 1.0f);
  factors = factors * factor;
  function_print("combined z = %f\n", z);
  if (z >= 1.0f)
  {
    function_print("here\n");
  }

  for (counter = 0; counter < attributes; counter++)
  {
    AddClassifier(counter, factors(counter));
  }

  return 1;
}



// load a strong classifier from file
vint8 vBoostNN::Load(const char * filename)
{
  char * path_name = make_pathname(filename);
  v3dMatrix<float> * classifier_matrix = v3dMatrix<float>::ReadText(path_name);
  if ((classifier_matrix == 0) || (classifier_matrix->Cols() < 2))
  {
    vPrint("Failed to load classifier from %s\n", path_name);
    vdelete2(path_name);
    function_delete(classifier_matrix);
    return 0;
  }
  vdelete2(path_name);

  vArray2(float) data = classifier_matrix->Matrix2(0);
  vint8 rows = classifier_matrix->Rows();
  vint8 i;
  for (i = 0; i < rows; i++)
  {
    vint8 index = round_number(data[i][0]);
    float weight = data[i][1];
    if ((index < 0) || (index >= attributes))
    {
      vPrint("Error: bad index %li, attributes = %li\n", index, attributes);
    }
    else
    {
      AddClassifier(index, weight);
    }
    PrintSummary();
  }

  function_delete(classifier_matrix);
  return 1;
}

  
// save a strong classifier to file.
vint8 vBoostNN::Save(const char * filename)
{
  char * round_robin_name = make_round_robin_pathname(filename);
  char * pathname = make_pathname(filename);
  vMatrix<float> classifier_matrix = ClassifierMatrix();
  vint8 success = 1;
  success *= save_classifier_matrix(round_robin_name, classifier_matrix);
  success *= save_classifier_matrix(pathname, classifier_matrix);

  delete_pointer(round_robin_name);
  delete_pointer(pathname);
  return success;
}


vint8 vBoostNN::save_classifier_matrix(const char * pathname, float_matrix classifier_matrix)
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

  float one_training_error, one_test_error;
  if (knn_training_errors.Size() > 1)
  {
    one_training_error = knn_training_errors(1);
  }
  else
  {
    one_training_error = 2;
  }
  if (knn_test_errors.Size() > 1)
  {
    one_test_error = knn_test_errors(1);
  }
  else
  {
    one_test_error = 2;
  }

  fprintf(fp, "%-10s %-15s %-15s %-15s %-15s\n", "best_k", "1-nn training", "1-nn test",
          "k-nn training", "knn test");
  fprintf(fp, "%-10li %-8.6f%7s %-8.6f%7s %-8.6f%7s %-8.6f\n",
          best_k, one_training_error, " ", one_test_error, " ",
          best_knn_training, " ", best_knn_test);
  fclose(fp);
  return success;
}


// Here instead of passing in an object we just pass in an index into 
// vectors_matrix. Typically vectors_matrix is the training vectors
// or the test vectors.
vMatrix<float> vBoostNN::TestDistances2(vint8 index, v3dMatrix<float> & vectors)
{
  return TestDistances4(index, vectors, &cleaned_indices, &cleaned_factors);
}

  
// Similar to TestDistances2, but here we only compute distances to
// objects that bevint8 to the class specified by class_id.
vMatrix<float> vBoostNN::TestDistances3(vint8 index, v3dMatrix<float> & vectors,
                                          vint8 class_id)
{
  return TestDistances5(index, vectors, class_id, 
                        &cleaned_indices, &cleaned_factors);
}


// this function should be called whenever we change the training triples,
// for example with choose_second_triples or change_triples.
// the input arguments are the new triples and the initial weights for 
// those triples.  
// The task of this function is to store those triples and weights, and also to
// essentially clear out the existing
// weak classifiers and then re-insert them, so that we compute the proper
// updated weights for the new triples
vint8 vBoostNN::adjust_to_triples(vint8_matrix new_training_triples, float_matrix new_weights)
{
  // the next line basically makes the validation triples equal to the training triples
  InitializeB(new_training_triples, new_training_triples);

  // store the new weights and make sure that they sum up to one.
  float total = (float) function_image_total(& new_weights);
  if (total == 0.0f)
  {
    exit_error("\nerror: in adjust_to_triples weights sum up to zero\n");
  }

  function_image_divide(& new_weights, (double) total, &training_factors_matrix);
  training_factors_matrix = new_weights;
  training_factors = training_factors_matrix.flat();
  initial_factors = float_matrix(& training_factors_matrix);

  // clear out and reinsert all the weak classifiers
  vint8_matrix classifier_indices = matrix_from_vector(&distance_indices);
  float_matrix classifier_weights = matrix_from_vector(&distance_factors);

  distance_indices.clear();
  distance_factors.clear();
  cleaned_indices.clear();
  cleaned_factors.clear();

  distance_zs.clear();
  training_errors.clear();
  validation_errors.clear();
  test_errors.clear();
  test_triple_errors.clear();

  vint8 number = classifier_indices.length();
  vint8 counter;
  for (counter = 0; counter < number; counter++)
  {
    vint8 index = classifier_indices(counter);
    float weight = classifier_weights(counter);
    AddClassifier(index, weight);
  }

  return 1;
}


// this function is useful and when we add classifiers with
// prespecified factors, and then we want to adjust
// all factors simultaneously by multiplying them with
// a single constant.
vint8 vBoostNN::normalize_factors()
{
  if (distance_indices.size() == 0)
  {
    return 0;
  }
  vector<float> results_vector((vector_size) training_triple_number);
  vint8 counter;
  for (counter = 0; counter < training_triple_number; counter++)
  {
    results_vector[(vector_size) counter] = training_margins[(vector_size) counter];
  }

  float minimum = 0.0f, factor = 0.0f;

  training_factors_matrix = float_matrix(& initial_factors);
  training_factors = training_factors_matrix.flat();

  MinimizeZ(&results_vector, &minimum, & factor, 0.0f, 10.0f);
  function_print("\nfactor = %f\n", factor);
  if (factor == 0.0f)
  {
    return 0;
  }
  
  for (counter = 0; counter < (vint8) distance_indices.size(); counter++)
  {
    distance_factors[(vector_size) counter] *= factor;
//    function_print("\nz(%li) = %f\n", counter, distance_zs[counter]);
  }

  for (counter = 0; counter < (vint8) cleaned_indices.size(); counter++)
  {
    cleaned_factors[(vector_size) counter] *= factor;
  }

  adjust_to_triples(training_triples_matrix, initial_factors);

//  for (counter = 0; counter < (vint8) distance_indices.size(); counter++)
  {
//    function_print("\nz(%li) = %f\n", counter, distance_zs[counter]);
  }

  function_print("\nfactor = %f\n", factor);
  return 1;
}




/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////

vMultiplier::vMultiplier(vint8 in_type, float in_low, float in_high)
{
  type = in_type;
  low = in_low;
  high = in_high;
}


float vMultiplier::Factor(float value)
{
  vint8 result = 0;
  switch(type)
  {
  case 0:
    if (value >= low)
    {
      result = 1;
    }
    break;

  case 1:
    if (value < high)
    {
      result = 1;
    }
    break;

  case 2:
    if ((low <= value) && (value < high))
    {
      result = 1;
    }
    break;

  case 3:
    if ((value < low) || (high <= value))
    {
      result = 1;
    }
    break;

  case 4:
    result = 1;
    break;

  default:
    exit_error("error: Factor, impossible type = %li\n",
                    type);
    break;
  }

  return (float) result;
}


vint8 vMultiplier::Print()
{
  vPrint("index = %li, type = %li, low = %f, high = %f\n", 
         index, type, low, high);
  return 1;
}


// This constructor copies the vector of multipliers,
// so that we can modify it (in the newly constructed object)
// without modifying the original classifier.
vQsClassifier::vQsClassifier(vQsClassifier * classifier)
{
  index = classifier->index;
  weight = classifier->weight;
  range = classifier->range;
  
  multipliers = new vector<vMultiplier>;
  vint8 number = classifier->multipliers->size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    multipliers->push_back((*(classifier->multipliers))[(vector_size) i]);
  }
}


vQsClassifier::vQsClassifier()
{
  multipliers = new vector<vMultiplier>;
  index = -1;
  weight = (float) 0;
  range = 0;
}


vQsClassifier::~vQsClassifier()
{
  remove_reference();
}


void vQsClassifier::delete_unique()
{
  vdelete(multipliers);
}


// Returns the product of the multiplers' Factor functions.
float vQsClassifier::Factor1(v3dMatrix<float> & objectm)
{
  return Factor2(objectm, 0);
}


float vQsClassifier::Factor2(v3dMatrix<float> & objectsm, vint8 row)
{
  vArray2(float) objects = objectsm.Matrix2(0);
  vArray(float) object = objects[row];

  float result = (float) 1;
  vint8 number = multipliers->size();
  
  vint8 i; 
  for (i = 0; i < number; i++)
  {
    vMultiplier & multiplier = (*multipliers)[(vector_size) i];
    float value = object[multiplier.index];
    float factor = (*multipliers)[(vector_size) i].Factor(value);
    result = result * factor;
  }

  return result;
}


vint8 vQsClassifier::Print()
{
  vint8 number = multipliers->size();
  vPrint("vQsClassifier: %li multipliers\n\n", number);
  
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vPrint("multiplier %li:\n", i);
    (*multipliers)[(vector_size) i].Print();
    vPrint("\n");
  }

  vPrint("dimension = %li, weight = %f, range = %li\n",
         index, weight, range);
  return 1;
}


vBnnqs::vBnnqs() : vBnn()
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBnnqs");
}


vBnnqs::vBnnqs(vMatrix<float> in_training_vectors,
          vMatrix<vint8> in_training_labels,
          vMatrix<float> in_test_vectors,
          vMatrix<vint8> in_test_labels,
          vint8 in_training_number, vint8 in_validation_number) :
vBnn(in_training_vectors, in_training_labels,
                              in_test_vectors, in_test_labels,
                              in_training_number, in_validation_number)
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBnnqs");
}


vBnnqs::vBnnqs(vMatrix<float> in_training_vectors,
          vMatrix<vint8> in_training_labels,
          vMatrix<float> in_test_vectors,
          vMatrix<vint8> in_test_labels,
          vMatrix<vint8> in_training_triples,
          vMatrix<vint8> in_validation_triples) :
vBnn(in_training_vectors, in_training_labels,
     in_test_vectors, in_test_labels,
     in_training_triples, in_validation_triples)
{
  delete_pointer(class_name);
  class_name = function_copy_string("vBnnqs");
}


vBnnqs::~vBnnqs()
{
}



// This function uses ResampleTriples to create training and validation
// triples, which are used to initialize a new vBoostNN object.
vBnnqs * vBnnqs::NewBnnqs(vint8 max_k)
{
  vMatrix<vint8> new_training_triples = ResampleTriples(training_triple_number,
                                                        max_k);
//  vMatrix<vint8> new_training_triples = ResampleTriples5();

  vMatrix<vint8> new_validation_triples = ResampleTriples(validation_triple_number,
                                                          max_k);
  vBnnqs * result = new vBnnqs(training_vectors_matrix, training_labels_matrix,
                               test_vectors_matrix, test_labels_matrix, 
                               new_training_triples, new_validation_triples);
  result->SetAllowNegativeWeights(allow_negative_weights);
  if (subject_ids_set == 1)
  {
    result->SetSubjectIds(subject_ids_matrix);
  }
  return result;
}


// This one calls ResampleTriples2.
vBnnqs * vBnnqs::NewBnnqs2()
{
  vMatrix<float> training_factorsp(1, training_triple_number);
  vMatrix<float> validation_weightsp(1, validation_triple_number);
  vMatrix<vint8> new_training_triples = ResampleTriples2(training_triple_number, 
                                                         training_factorsp);
  vMatrix<vint8> new_validation_triples = ResampleTriples(validation_triple_number, 2);
  vBnnqs * result = new vBnnqs(training_vectors_matrix, training_labels_matrix,
                               test_vectors_matrix, test_labels_matrix, 
                               new_training_triples, new_validation_triples);
  result->training_factors_matrix = training_factorsp;
  result->training_factors = training_factorsp.Matrix();
  result->SetAllowNegativeWeights(allow_negative_weights);
  if (subject_ids_set == 1)
  {
    result->SetSubjectIds(subject_ids_matrix);
  }
  return result;
}


vint8 vBnnqs::CleanUpClassifier()
{
  CleanUp(attributes, &step_classifiers, &unique_classifiers);
  return 1;
}



// here, in addition to index and alpha, we also specify z, so that
// we don't have to spend any more time to compute z, when z
// is already available.
vint8 vBnnqs::AddClassifier(vQsClassifier classifier, float z)
{
  float alpha = classifier.weight;
  vector<float> results((vector_size) training_triple_number);
  ClassifierResults(classifier, &results, training_triples_matrix);
  ComputeLastError(alpha, &results);

  classifier.range = ClassifierRange(classifier);
  step_classifiers.push_back(classifier);
  distance_zs.push_back(z);
  UpdateWeights(&results, z, alpha);

  CleanUpClassifier();
  float margin = ComputeTrainingError(&results, alpha);

  vector<float> validation_results((vector_size) validation_triple_number);
  ClassifierResults(classifier, &validation_results, validation_triples_matrix);
  ComputeValidationError(&validation_results, alpha);
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);
  
  UpdateDistances(classifier);
  ComputeTestError();
  test_errors.push_back(test_error);
  test_triple_errors.push_back(test_triple_error);

  return 1;
}


// Here dimensions (i.e. indices of attributes) can occur many times.
vint8 vBnnqs::Classifier(vector<vQsClassifier> * out_classifier)
{
  vint8 steps = StepsDone();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    out_classifier->push_back(step_classifiers[(vector_size) i]);
  }

  return steps;
}


// Here we identify repetitions of the same dimension (i.e. the same
// attribute, i.e. the same feature) and we combine them by adding
// the weights associated with each occurrence.
vint8 vBnnqs::CleanClassifier(vector<vQsClassifier> * out_classifier)
{
  vint8 steps = unique_classifiers.size();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    out_classifier->push_back(unique_classifiers[(vector_size) i]);
  }

  return steps;
}


// add classifier corresponding to the attribute specified by
// index, and with weight specified by alpha.
vint8 vBnnqs::AddClassifier(vint8 index, float alpha)
{
  if ((index < 0) || (index >= attributes))
  {
    function_warning("Warning: index = %li, attributes = %li\n",
              index, attributes);
    return 0;
  }

  vQsClassifier classifier(index, alpha);
  vector<float> results((vector_size) training_triple_number);
  ClassifierResults(classifier, &results, training_triples_matrix);
  float z = Z(alpha, &results);

  return AddClassifier(classifier, z);
}


// load a strong classifier from file
//vint8 vBnnqs::Load(const char * filename);


// save a strong classifier to file.
//vint8 vBnnqs::Save(const char * filename);



// return the number of training steps performed so far.
vint8 vBnnqs::StepsDone()
{
  return step_classifiers.size();
}


float vBnnqs::CurrentDistanceWeight(vint8 distance_index)
{
  float result = 0;
  vint8 number = unique_classifiers.size();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index = unique_classifiers[(vector_size) i].index;
    if ((index == distance_index) && 
        (unique_classifiers[(vector_size) i].multipliers->size() == 0))
    {
      result = result + unique_classifiers[(vector_size) i].weight;
    }
  }

  return result;
}


//vMatrix<float> vBnnqs::ClassifierMatrix();


// Prints some information about this object.
vint8 vBnnqs::PrintSummary()
{
  if (training_number == 0) return 0;
  vPrint("\n");
  vPrint("BoostNN-QS: %li training, %li test, %li classes, %li attributes\n",
          training_number, test_number, classes, attributes);
  vPrint("Triples: %li training, %li validation\n",
          training_triple_number, validation_triple_number);
  vPrint("min_exp = %f, max_exp = %f\n", min_exp, max_exp);
  vPrint("iterations = ");
  iterations.Print();
  vPrint("\n");

  vPrint("weight sum = %.3f\n", SumWeights());
  vPrint("last error = %f, et = %f, correlation = %f\n",
          last_error, last_et, last_correlation);
  vPrint("last_scaled_et = %f, last_scaled_correlation = %f\n",
          last_scaled_et, last_scaled_correlation);
  vPrint("training_error = %f, margin = %f, validation = %f\n",
          training_error, training_margin, validation_error);
  vPrint("test_triple_error = %f, test_error = %f\n\n", 
          test_triple_error, test_error);

  vint8 steps = StepsDone();
  if (steps > 0)
  {
    vQsClassifier classifier = step_classifiers[(vector_size) steps-1];
    classifier.Print();
    float last_z = distance_zs[(vector_size) steps-1];
    vPrint("last_z = %f\n", last_z);
  }
  vPrint("%li steps, net classifiers = %li\n", steps, unique_classifiers.size());

  vPrint("\n");
  return 1;
}


vint8 vBnnqs::PrintClassifier()
{
  vint8 i;

  vint8 number = step_classifiers.size();
  vPrint("classifier, step by step:\n");
  for (i = 0; i < number; i++)
  {
    vPrint("classifier %li:\n", i);
    step_classifiers[(vector_size) i].Print();
  }
  vPrint("\n");

  number = unique_classifiers.size();
  vPrint("classifier, without repetitions:\n");
  for (i = 0; i < number; i++)
  {
    vPrint("classifier %li:\n", i);
    unique_classifiers[(vector_size) i].Print();
  }

  vPrint("\n");
  return 1;
}


// Similar to TestDistances2, but here we only compute distances to
// objects that bevint8 to the class specified by class_id.
vMatrix<float> vBnnqs::TestDistances2(vint8 index, v3dMatrix<float> & vectors)
{
  return TestDistances3b(index, vectors, &unique_classifiers);
}


// Similar to TestDistances2, but here we only compute distances to
// objects that bevint8 to the class specified by class_id.
vMatrix<float> vBnnqs::TestDistances3(vint8 index, v3dMatrix<float> & vectors,
                                       vint8 class_id)
{
  return TestDistances4b(index, vectors, class_id, 
                         &unique_classifiers);
}


// Updates the entries of test_distances_matrix, to add 
// distances based on the given classifier, weighted
// by alpha.
vint8 vBnnqs::UpdateDistances(vQsClassifier classifier)
{
  vint8 index = classifier.index;
  float alpha = classifier.weight;
  vArray2(float) distances = test_distances_matrix.Matrix2();
  vint8 row, col;
  for (row = 0; row < test_number; row++)
  {
    float test_value = test_vectors[row][index];
    float factor = classifier.Factor2(test_vectors_matrix, row);
    for (col = 0; col < training_number; col++)
    {
      float training_value = training_vectors[col][index];
      float diff = test_value - training_value;
      float weighted = factor * alpha * vAbs(diff);
      distances[row][col] = distances[row][col] + weighted;
    }
  }

  return 1;
}


vint8 vBnnqs::ClassifierResults(vQsClassifier classifier, vector<float> * results, 
                               vMatrix<vint8> triples_matrix)
{
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vint8 triple_number = triples_matrix.Rows();
  vint8 index = classifier.index;

  vint8 i;
  for (i = 0; i < triple_number; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    // label = 1 means that q is always same class as a and different
    // class than b. Based on the way we construct the training and
    // validation sets, label is always 1. Label = -1 would mean
    // that q is same class as b and different class than a.
    float label = 1;
  
    float q_dim = training_vectors[q][index];
    float a_dim = training_vectors[a][index];
    float b_dim = training_vectors[b][index];
    float factor = classifier.Factor2(training_vectors_matrix, q);
    float distance = factor * (vAbs(q_dim - b_dim) - vAbs(q_dim - a_dim)) * label;

    (*results)[(vector_size) i] = distance;
  }

  return 1;
}


// This function is used for TestDistances2
vMatrix<float> vBnnqs::TestDistances3b(vint8 index, v3dMatrix<float> & vectors_matrix,
                                       vector<vQsClassifier> * classifiers)
{
  vArray2(float) vectors = vectors_matrix.Matrix2(0);
  vArray(float) object = vectors[index];
  vMatrix<float> result_matrix(1, training_number);
  vArray(float) result = result_matrix.Matrix();

  vint8 number = classifiers->size();
  vint8 i, j;
  for (i = 0; i < training_number; i++)
  {
    float distance = 0;
    for (j = 0; j < number; j++)
    {
      vint8 dimension_index = (*classifiers)[(vector_size) j].index;
      float weight = (*classifiers)[(vector_size) j].weight;
      float value1 = object[dimension_index];
      float value2 = training_vectors[(vector_size) i][dimension_index];
      float factor = (*classifiers)[(vector_size) j].Factor2(vectors_matrix, index);
      float diff = factor * weight * vAbs(value1 - value2);
      distance = distance + diff;
    }
    result[i] = distance;
  }

  return result_matrix;
}


// This function is used for TestDistances3
vMatrix<float> vBnnqs::TestDistances4b(vint8 index, v3dMatrix<float> & vectors_matrix,
                                       vint8 class_id, vector<vQsClassifier> * classifiers)
{
  vArray2(float) vectors = vectors_matrix.Matrix2(0);
  vArray(float) object = vectors[index];
  vint8 class_size = class_indices[class_id].size();
  vMatrix<float> result_matrix(1, class_size);
  vArray(float) result = result_matrix.Matrix();

  vint8 number = classifiers->size();
  vint8 i, j;
  for (i = 0; i < class_size; i++)
  {
    float distance = 0;
    vint8 index = class_indices[class_id][(vector_size) i];
    for (j = 0; j < number; j++)
    {
      vint8 dimension_index = (*classifiers)[(vector_size) j].index;
      float weight = (*classifiers)[(vector_size) j].weight;
      float value1 = object[dimension_index];
      float value2 = training_vectors[index][dimension_index];
      float factor = (*classifiers)[(vector_size) j].Factor2(vectors_matrix, index);
      float diff = factor * weight * vAbs(value1 - value2);
      distance = distance + diff;
    }
    result[i] = distance;
  }

  return result_matrix;
}


// Finds the attribute that achieves the smallest z, and chooses
// the next weak classifier to depend on that attribute.
vint8 vBnnqs::NextStep()
{
  if ((training_triple_number == 0) || (attributes == 0))
  {
    return -1;
  }

  // For the ICML 2004 submission, the max number of attributes is 617,
  // and the second max number is 36, so the next line just specifies
  // that, for these datasets, we should use all the attributes. 
  const vint8 candidate_limit = 800;
  vint8 candidate_number = Min(candidate_limit, attributes);
  
  // sample from all possible classifiers
  vMatrix<vint8> candidates = sample_without_replacement(0, attributes-1, 
                                                         candidate_number);

  float best_alpha = 0;
  float alpha = 0;
  float z_min = DistanceZ(candidates(0), &best_alpha);
  vint8 z_min_index = candidates(0);
  vint8 i, ii;

  // find the classifier that minimizes z, among the candidate classifiers.
  for (ii = 1; ii < candidate_number; ii++)
  {
    i = candidates(ii);
    float z = DistanceZ(i, &alpha);
    if (z < z_min)
    {
      z_min = z;
      z_min_index = i;
      best_alpha = alpha;
    }
  }

  vQsClassifier classifier(z_min_index, best_alpha);
  return AddClassifier(classifier, z_min);
}


// Minimizes z for each classifier, and returns the best classifier, as well
// as the z attained with that classifier.
vQsClassifier vBnnqs::FindBest(vector<vQsClassifier> * classifiers, float * z_pointer)
{
  vint8 number = classifiers->size();
  if ((training_triple_number == 0) || (number == 0))
  {
    return vQsClassifier(-1, -1);
  }

  if (classifiers->size() < 1)
  {
    return vQsClassifier(-2, -1);
  }

  float best_alpha = 0;
  float alpha = 0;
  float z_min = ClassifierZ((*classifiers)[0], &best_alpha);
  vint8 z_min_index = 0;
  vint8 i;

  // find the classifier that minimizes z, among the candidate classifiers.
  vPrint("\n");
  for (i = 1; i < number; i++)
  {
    float z = ClassifierZ((*classifiers)[(vector_size) i], &alpha);
    if (z < z_min)
    {
      z_min = z;
      z_min_index = i;
      best_alpha = alpha;
    }
    vPrint("evaluated %li of %li classifiers\r", i+1, number);
  }
  vPrint("\n");

  *z_pointer = z_min;
  vQsClassifier classifier = (*classifiers)[(vector_size) z_min_index];
  classifier.weight = best_alpha;
  return classifier;
}


float vBnnqs::ClassifierZ(vQsClassifier classifier, float * alpha_pointer)
{
  // results[i] is what is called u_i in Schapire and Singer.
  vector<float> results((vector_size) training_triple_number);
  ClassifierResults(classifier, &results, training_triples_matrix);
  float current_weight = CurrentDistanceWeight(classifier.index);
  float min_alpha;

  // figure out the allowable ranges for alpha, i.e. for the 
  // weight to be given to the current classifier, if it
  // is defined based on the index-th attribute.
  
  // For a Qs classifier, we just want weight to be positive.
  if ((allow_negative_weights == 0) && (classifier.multipliers->size() != 0))
  {
    min_alpha = Max((float) 0, alpha_limits[0][classifier.index]);
  }
  // For a non-qs classifier, we allow a negative weight, if it does not
  // make the overall weight in unique_classifiers negative.
  else if (allow_negative_weights == 0) 
  {
    min_alpha = Max(-current_weight, alpha_limits[0][classifier.index]);
  }
  else
  {
    min_alpha = alpha_limits[0][classifier.index];
  }

  float a = 0, z = 0;
  iterations = iterations + vas_int16((int) MinimizeZ(&results, &z, &a,
                                                min_alpha,
                                                alpha_limits[1][(vector_size) classifier.index]));
  *alpha_pointer = a;
  return z;
}


// constructs random query-sensitive classifiers. Those classifiers can
// be constructed by adding a multiplier to a query-insensitive classifier,
// or by adding a splitter to one of the classifiers in unique_classifiers.
// The constructed classifiers go to the vector "classifiers". The argument
// "number" specifies the number of classifiers we want to construct.
vint8 vBnnqs::MakeRandomClassifiers(vector<vQsClassifier> * classifiers, 
                                   vint8 number)
{
  vint8 i;
  vPrint("\n");
  for (i = 0; i < number; i++)
  {
    vQsClassifier classifier = RandomClassifier();
    classifiers->push_back(classifier);
    vPrint("made %li of %li random classifiers\r", i+1, number);
  }
  vPrint("\n");

  return 1;
}


// constructs a single random query-sensitive classifier. This classifier
// is chosen by first picking either a random attribute, or a random
// classifier from unique_classifiers, and then adding a random multiplier
// to it.
vQsClassifier vBnnqs::RandomClassifier()
{
  vint8 unique_number = unique_classifiers.size();
  vint8 number = attributes + unique_number;

  if (number == 0)
  {
    return 0;
  }
  // pick a random number, that will specify which classifier to
  // use as a basis for constructing the result.
  vint8 pick = function_random_vint8(0, number-1);
  vQsClassifier base;
  if (pick < attributes)
  {
    base = vQsClassifier(pick, 0);
  }
  else
  {
    base = unique_classifiers[(vector_size) (pick - attributes)];
  }

  vQsClassifier result(&base);
  vMultiplier multiplier = RandomMultiplier();
  result.multipliers->push_back(multiplier);
  return result;
}


vMultiplier vBnnqs::RandomMultiplier()
{
  if ((attributes == 0) || (training_number < 2))
  {
    return vMultiplier();
  }

  vMultiplier multiplier;
  multiplier.index = function_random_vint8(0, attributes - 1);
  v3dMatrix<float> attribute_values = copy_vertical_line(&training_vectors_matrix, 
                                                multiplier.index);

  vint8 rank1 = function_random_vint8(1, training_number - 1);
  vint8 rank2 = function_random_vint8(1, training_number - 1);
  vint8 min_rank = Min(rank1, rank2);
  vint8 max_rank = Max(rank1, rank2);

  vint8 junk = 0;
  float min_value1 = kth_smallest_cb(min_rank, &attribute_values, &junk);
  float min_value2 = kth_smallest_cb(min_rank+1, &attribute_values, &junk);
  float max_value1 = kth_smallest_cb(max_rank, &attribute_values, &junk);
  float max_value2 = kth_smallest_cb(max_rank+1, &attribute_values, &junk);

  multiplier.low = (min_value1 + min_value2) / 2;
  multiplier.high = (max_value1 + max_value2) / 2;
  multiplier.type = function_random_vint8(0, 3);

  return multiplier;
}


// counts how many training objects are accepted by the multipliers
// of some classifier.
vint8 vBnnqs::ClassifierRange(vQsClassifier classifier)
{
  vint8 counter = 0;
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    float factor = classifier.Factor2(training_vectors_matrix, i);
    if (factor > 0)
    {
      counter++;
    }
  }

  return counter;
}


// number simply specifies the number of query-sensitive classifiers to 
// construct.
vint8 vBnnqs::NextStepQs(vint8 number)
{
  vector<vQsClassifier> classifiers;
  MakeRandomClassifiers(&classifiers, number);

  float best_z = (float) 0;
  vQsClassifier new_classifier = FindBest(&classifiers, &best_z);
  AddClassifier(new_classifier, best_z);

  return 1;
}


float vBnnqs::CheckPair(vint8 test, vint8 training)
{
  vint8 number = unique_classifiers.size();
  float margin = 0;
  vint8 i;
  vPrint("\n");
  for (i = 0; i < number; i++)
  {
    vQsClassifier & classifier = unique_classifiers[(vector_size) i];
    vint8 index = classifier.index;

    // get i-th coordinate of embeddings
    float qi = test_vectors_matrix(test, index);
    float ai = training_vectors_matrix(training, index);
    float factor = classifier.Factor2(test_vectors_matrix, test);

    // compute contribution of i-th coordinate to classification
    // result of embedding on triple (q, a, b).
    float w = classifier.weight;
    float current = vAbs(qi - ai);
    float wcurrent = factor * w * current;
    margin = margin + wcurrent;
    vPrint("%5i, %5li: %10.5f%10.5f%10.5f%10.5f\n",
           i, index, factor, current, wcurrent, margin);
  }
  vPrint("\n");

  float distance = test_distances_matrix(test, training);
  vPrint("distance = %f\n", distance);
  vPrint("margin = %f\n", margin);
  vPrint("\n");

  return margin;
}


float vBnnqs::CheckPairTraining(vint8 training1, vint8 training2)
{
  vint8 number = unique_classifiers.size();
  float margin = 0;
  vint8 i;
  vPrint("\n");
  for (i = 0; i < number; i++)
  {
    vQsClassifier & classifier = unique_classifiers[(vector_size) i];
    vint8 index = classifier.index;

    // get i-th coordinate of embeddings
    float qi = training_vectors_matrix(training1, index);
    float ai = training_vectors_matrix(training2, index);
    float factor = classifier.Factor2(training_vectors_matrix, training1);

    // compute contribution of i-th coordinate to classification
    // result of embedding on triple (q, a, b).
    float w = classifier.weight;
    float current = vAbs(qi - ai);
    float wcurrent = factor * w * current;
    margin = margin + wcurrent;
    vPrint("%5i, %5li: %10.5f%10.5f%10.5f%10.5f\n",
           i, index, factor, current, wcurrent, margin);
  }
  vPrint("\n");

  vPrint("margin = %f\n", margin);
  vPrint("\n");

  return margin;
}


///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////


similarity_learning::similarity_learning() : vBoostNN()
{
  delete_pointer(class_name);
  class_name = function_copy_string("similarity_learning");
}



similarity_learning::similarity_learning(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vint8 in_training_number, vint8 in_validation_number)
{
  delete_pointer(class_name);
  Zero();
  class_name = function_copy_string("similarity_learning");

  InitializeData(in_training_vectors, in_training_labels,
                 in_test_vectors, in_test_labels);
  InitializeA(in_training_number, in_validation_number);
}


similarity_learning::similarity_learning(vMatrix<float> in_training_vectors,
                     vMatrix<vint8> in_training_labels,
                     vMatrix<float> in_test_vectors,
                     vMatrix<vint8> in_test_labels,
                     vMatrix<vint8> in_training_triples,
                     vMatrix<vint8> in_validation_triples) 
{
  delete_pointer(class_name);
  Zero();
  class_name = function_copy_string("similarity_learning");

  InitializeData(in_training_vectors, in_training_labels,
                 in_test_vectors, in_test_labels);
  InitializeB(in_training_triples, in_validation_triples);
}


similarity_learning::~similarity_learning()
{
}


// this function should be called for the first time that we want to
// to choose triples based on the already computed distance measure
vint8_matrix similarity_learning::choose_second_triples(vint8 neighbor_number,
                                                       vector<float> * new_weights)
{
  vint8 maximum_triple_number = neighbor_number * training_number * training_number;
  
  vector<vint8> first_objects;
  first_objects.reserve((vector_size) maximum_triple_number);

  vector<vint8> second_objects;
  second_objects.reserve((vector_size) maximum_triple_number);

  vector<vint8> third_objects;
  third_objects.reserve((vector_size) maximum_triple_number);

  // triple_counter(index) will store the number of triples for which
  // the first object is equal to index.
  vint8_matrix triple_counter(1, training_number);

  // triple_start_index(index) will store the first index where a triple
  // with first object equal to index is stored.  All triples
  // whose first objects are equal to index are stored sequentially.
  vint8_matrix triple_start_index(1, training_number);

  vint8 triple_index = 0;
  vint8 first;
  for (first = 0; first < training_number; first++)
  {
    vint8 neighbor;
    vector<vint8> neighbors;
    vint8 actual_number = 0;
    triple_start_index(first) = triple_index;
    triple_counter(first) = 0;
    vint8 first_class = training_labels_matrix(first);

    vint8 success = similar_nearest_neighbors(first, neighbor_number, & neighbors, 0, & actual_number);
    if (success <= 0)
    {
      continue;
    }

    vint8 neighbors_to_use = function_minimum(actual_number, neighbor_number);

    for (neighbor = 0; neighbor < neighbors_to_use; neighbor++)
    {
      vint8 second = neighbors[(vector_size) neighbor];
      vint8 third;
      for (third = 0; third < training_number; third++)
      {
        if (training_labels_matrix(third) == first_class)
        {
          continue;
        }

        first_objects.push_back(first);
        second_objects.push_back(second);
        third_objects.push_back(third);

        triple_index++;
        triple_counter(first) = triple_counter(first) + 1;
      }
    }
  }

  vint8_matrix result(triple_index, 3);
  vint8 counter;
  for (counter = 0; counter < triple_index; counter++)
  {
    vint8 first = first_objects[(vector_size) counter];
    result(counter, 0) = first;
    result(counter, 1) = second_objects[(vector_size) counter];
    result(counter, 2) = third_objects[(vector_size) counter];

    vint8 triple_number = triple_counter(first);
    float weight = 1.0f / (float) triple_number;
    new_weights->push_back(weight);
  }

  return result;
}


// this function should be called for the first time that we want to
// to choose triples based on the already computed distance measure
vint8 similarity_learning::set_second_triples(vint8 neighbor_number)
{
  vector<float> factors;
  vint8_matrix triples_matrix = choose_second_triples(neighbor_number, & factors);
  float_matrix factors_matrix = matrix_from_vector(& factors);
  
  adjust_to_triples(triples_matrix, factors_matrix);
  normalize_factors();
  return 1;
}

  
vint8_matrix similarity_learning::choose_third_triples(vint8 neighbor_number,
                                                      vint8 cutoff,
                                                       vector<float> * new_weights)
{
  const vint8 maximum_triples = 500000;
  AnalyzeTriples();
  vint8 maximum_triple_number = neighbor_number * training_number * training_number;
  if (maximum_triple_number > maximum_triples)
  {
    maximum_triple_number = maximum_triples;
  }
  
  vint8 triples_per_neighbor = maximum_triple_number / (training_number * neighbor_number);

  vector<vint8> first_objects;
  first_objects.reserve((vector_size) maximum_triple_number);

  vector<vint8> second_objects;
  second_objects.reserve((vector_size) maximum_triple_number);

  vector<vint8> third_objects;
  third_objects.reserve((vector_size) maximum_triple_number);

  // triple_counter(index) will store the number of triples for which
  // the first object is equal to index.
  vint8_matrix triple_counter(1, training_number);

  // triple_start_index(index) will store the first index where a triple
  // with first object equal to index is stored.  All triples
  // whose first objects are equal to index are stored sequentially.
  vint8_matrix triple_start_index(1, training_number);

  vint8 triple_index = 0;
  vint8 first;
  vint8 first_counter = 0;

  for (first = 0; first < training_number; first++)
  {
    if (bad_numbers(first) > cutoff)
    {
      continue;
    }

    first_counter++;

    vint8 neighbor;
    vector<vint8> similar_neighbors;
    vint8 actual_number = 0;
    triple_start_index(first) = triple_index;
    triple_counter(first) = 0;
    vint8 first_class = training_labels_matrix(first);

    vint8 success = similar_nearest_neighbors(first, neighbor_number, & similar_neighbors, 0, & actual_number);
    if (success <= 0)
    {
      continue;
    }

    vint8 similar_neighbors_to_use = function_minimum(actual_number, neighbor_number);

    vector<vint8> different_neighbors;
    vint8 actual_different_number = 0;
    success = different_nearest_neighbors(first, triples_per_neighbor, & different_neighbors, 0, & actual_different_number);
    vint8 different_neighbors_to_use = function_minimum(actual_different_number, triples_per_neighbor);

    for (neighbor = 0; neighbor < similar_neighbors_to_use; neighbor++)
    {
      vint8 second = similar_neighbors[(vector_size) neighbor];
      vint8 third_index;
      for (third_index = 0; third_index < different_neighbors_to_use; third_index++)
      {
        vint8 third = different_neighbors[(vector_size) third_index];
        if ((training_labels_matrix(third) == first_class) ||
            ((subject_ids_set > 0) && (subject_ids[third] == subject_ids[first])))
        {
          continue;
        }

        first_objects.push_back(first);
        second_objects.push_back(second);
        third_objects.push_back(third);

        triple_index++;
        triple_counter(first) = triple_counter(first) + 1;
      }
    }
  }

  vint8_matrix result(triple_index, 3);
  vint8 counter;
  for (counter = 0; counter < triple_index; counter++)
  {
    vint8 first = first_objects[(vector_size) counter];
    result(counter, 0) = first;
    result(counter, 1) = second_objects[(vector_size) counter];
    result(counter, 2) = third_objects[(vector_size) counter];

    vint8 triple_number = triple_counter(first);
    float weight = 1.0f / (float) triple_number;
    new_weights->push_back(weight);
  }

  function_print("%li first objects used\n", first_counter);
  return result;
}


// this function should be called for the first time that we want to
// to choose triples based on the already computed distance measure
vint8 similarity_learning::set_third_triples(vint8 neighbor_number, vint8 cutoff)
{
  vector<float> factors;
  vint8_matrix triples_matrix = choose_third_triples(neighbor_number, cutoff, & factors);
  float_matrix factors_matrix = matrix_from_vector(& factors);
  
  adjust_to_triples(triples_matrix, factors_matrix);
  normalize_factors();
  return 1;
}

  
// this function should be called at any point after choose_set_triple
// has been called, to update the triples and possibly insert new triples.
vint8_matrix similarity_learning::change_triples(vint8 neighbor_number,
                                                vector<float> * new_weights)
{
  return choose_second_triples(neighbor_number, new_weights);
}


// this function should be called for the first time that we want to
// to choose triples based on the already computed distance measure
vint8 similarity_learning::set_updated_triples(vint8 neighbor_number)
{
  vector<float> factors;
  vint8_matrix triples_matrix = change_triples(neighbor_number, & factors);
  float_matrix factors_matrix = matrix_from_vector(& factors);
  
  adjust_to_triples(triples_matrix, factors_matrix);
  normalize_factors();
  return 1;
}

  


// calls the right destructor for the object, by checking 
// the class_name member variable
vint8 delete_boosted_similarity(vBnn * bnn)
{
  if (bnn == 0)
  {
    return 0;
  }

  const char * class_name = bnn->get_class_name();
  if (function_compare_strings(class_name, "vBnn") == 0)
  {
    function_delete((vBnn *&) bnn);
  }

  else if (function_compare_strings(class_name, "vBoostNN") == 0)
  {
    function_delete((vBoostNN *&) bnn);
  }
  else if (function_compare_strings(class_name, "vBnnqs") == 0)
  {
    function_delete((vBnnqs *&) bnn);
  }
  else if (function_compare_strings(class_name, "similarity_learning") == 0)
  {
    function_delete((similarity_learning *&) bnn);
  }
  else
  {
    exit_error("\nerror: unknown class %s in BNN_delete\n");
  }

  return 1;
}

