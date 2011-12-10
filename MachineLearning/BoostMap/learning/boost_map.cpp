
#include "basics/algebra.h"
#include "learning/boost_map.h"
#include "basics/drawing_temp.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"

#include "basics/definitions.h"


static const float e = (float) 2.71828182845905;


float vL1_Distance(vMatrix<float> a, vMatrix<float> b)
{
  vint8 size = a.Size();
  if (size != b.Size())
  {
    exit_error("vL1_Distance: incompatible sizes %li %li\n",
                    (long) size, (long) b.Size());
  }

  vArray(float) c = a.Matrix();
  vArray(float) d = b.Matrix();

  vint8 i;
  float sum = 0;
  for (i = 0; i < size; i++)
  {
    sum = sum + vAbs(c[i] - d[i]);
  }

  return sum;
}


V_BoostMap::V_BoostMap()
{
  Zero();
}


V_BoostMap::V_BoostMap(vMatrix<float> in_distances, 
                       vMatrix<vint8> in_training_triples,
                       vMatrix<vint8> in_candidates)
{
  Zero();
  object_number = in_distances.Rows();
  validation_range = object_number;
  validation_number = in_training_triples.Rows();
  training_range = object_number;
  Initialize(in_distances, in_training_triples, in_candidates);
}


V_BoostMap::V_BoostMap(vMatrix<float> in_distances, vint8 in_candidate_range,
                       vint8 in_training_range,
                       vint8 in_number_of_picked_candidates, 
                       vint8 in_training_number, vint8 in_validation_number)
{
  Zero();
  Initialize6(in_distances, in_candidate_range, in_training_range, 
              in_number_of_picked_candidates,
              in_training_number, in_validation_number);
}
  


V_BoostMap::~V_BoostMap()
{
  vdelete2(class_name);
}


vint8 V_BoostMap::Zero()
{
  class_name = 0;
  training_number = 0;
  candidate_number = 0;
  object_number = 0;
  last_error = -1;
  last_et = -1;
  last_correlation = -1;
  training_error = -1;
  training_margin = -1;
  min_exp = (float) 1000000000; // one billion
  max_exp = (float) -1000000000;

  validation_number = 0;
  candidate_range = 0;
  training_range = 0;
  validation_range = 0;
  pick_candidates_flag = 0;
  number_of_picked_candidates = 0;
  allow_negative = 1;
  allow_removals = 1;

  return 1;
}


vint8 V_BoostMap::Initialize(vMatrix<float> in_distances, 
                            vMatrix<vint8> in_training_triples,
                            vMatrix<vint8> in_candidates)
{
  class_name = vCopyString("V_BoostMap");
  // Verify that the max distance in in_distances is 1. Otherwise
  // the distances are not acceptable.
  vArray2(float) d = in_distances.Matrix2();
  float max_distance = 0;
  vint8 i, j;
  vint8 rows = in_distances.Rows();
  vint8 cols = in_distances.Cols();
  if (rows != cols) return 0;
  for (i = 0; i < rows; i++)
  {
    for (j = 0; j < cols; j++)
    {
      float distance = d[i][j];
      if (distance > max_distance) 
      {
        max_distance = distance;
      }
    }
  }
  if (max_distance > 1)
  {
    return 0;
  }

  distances_matrix = in_distances;
  training_triples_matrix = in_training_triples;
  candidates_matrix = in_candidates;
  
  distances = distances_matrix.Matrix2();
  training_triples = training_triples_matrix.Matrix2();
  candidates = candidates_matrix.Matrix();
  
  training_number = training_triples_matrix.Rows();
  candidate_number = candidates_matrix.Cols();
  object_number = distances_matrix.Rows();

  TrainingDistances();
  InitialWeights();
  ChooseValidationSet((vint8) validation_number);

  return 1;
}


vint8 V_BoostMap::Initialize6(vMatrix<float> in_distances,
                             vint8 in_candidate_range, vint8 in_training_range,
                             vint8 in_number_of_picked_candidates, 
                             vint8 in_training_number, vint8 in_validation_number)
{
  object_number = in_distances.Rows();
  candidate_range = in_candidate_range;
  if (candidate_range > object_number)
  {
    vPrint("Resetting candidate_range to object_number %li\n", 
            (long) object_number);
  }
  training_range = in_training_range;

  validation_range = object_number - candidate_range - training_range;
  if (validation_range < 0)
  {
    vPrint("setting validation_range = training_range = %li\n", 
            (long) training_range);
    validation_range = training_range;
  }
  training_number = in_training_number;
  validation_number = in_validation_number;
  distances_matrix = in_distances;
  distances = distances_matrix.Matrix2();
  ChooseTrainingSet((vint8) training_number);
  vMatrix<vint8> temp_candidates;
  Initialize(in_distances, training_triples_matrix, temp_candidates);
  number_of_picked_candidates = in_number_of_picked_candidates ;
  pick_candidates_flag = 1;
  return 1;
}


vMatrix<vint8> V_BoostMap::AllCandidates(vint8 in_number)
{
  if (in_number <= 0)
  {
    exit_error("AllCandidates called with bad training number:\n",
                    in_number);
  }

  vMatrix<vint8> result = vMatrix<vint8>::Range(0, in_number-1);
  return result;
}

  
vint8 V_BoostMap::TrainingDistances()
{
  training_distances_matrix = vMatrix<float>(training_number, 3);
  training_distances = training_distances_matrix.Matrix2();
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    vint8 index0 = training_triples[i][0];
    vint8 index1 = training_triples[i][1];
    vint8 index2 = training_triples[i][2];

    if ((index0 < 0) || (index1 < 0) || (index2 < 0) ||
        (index0 >= object_number) || (index1 >= object_number) ||
        (index2 >= object_number))
    {
      exit_error("TrainingDistances error: bad indices\n");
    }

    float distance01 = distances[index0][index1];
    float distance02 = distances[index0][index2];
    float distance12 = distances[index1][index2];
    training_distances[i][0] = distance01;
    training_distances[i][1] = distance02;
    training_distances[i][2] = distance12;
  }

  return 1;
}


vint8 V_BoostMap::InitialWeights()
{
  if (training_number == 0)
  {
    return 1;
  }
  training_factors_matrix = vMatrix<float>(1, training_number);
  training_factors = training_factors_matrix.Matrix();

  float weight = ((float) 1) / (float) training_number;
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    training_factors[i] = weight;
  }

  return 1;
}


const char * V_BoostMap::ClassName()
{
  return class_name;
}


float V_BoostMap::NextStep()
{
  if (object_number == 0) return -1;

  vint8 successful_removal_flag = 0;
  float z_min = (float) 10;
  vint8 z_min_index = -1;
  float best_alpha = (float) 0;
  if (allow_removals != 0)
  {
    successful_removal_flag = TryRemovals(&z_min_index, &z_min, &best_alpha);
    if (successful_removal_flag > 0)
    {
      vPrint("removing index %li, alpha = %f, z = %f\n", 
              (long) z_min_index, best_alpha, z_min);
    }
    else
    {
      successful_removal_flag = TryWeightChange(&z_min_index, &z_min, &best_alpha);
      if (successful_removal_flag > 0)
      {
        vPrint("Modifying index %li, alpha = %f, z = %f\n", 
                (long) z_min_index, best_alpha, z_min);
      }
    }
  }

  if (successful_removal_flag == 0)
  {
    if (pick_candidates_flag > 0)
    {
      ChooseCandidates(number_of_picked_candidates);
    }

    if (candidate_number == 0) 
    {
      vPrint("no candidate classifiers are available\n");
      return 0;
    }

    float alpha = 0;
    z_min = ClassifierZ(candidates[0], &best_alpha);
    z_min_index = candidates[0];
    vint8 i;
    for (i = 1; i < candidate_number; i++)
    {
      float z = ClassifierZ(candidates[i], &alpha);
      if (z < z_min)
      {
        z_min = z;
        z_min_index = candidates[i];
        best_alpha = alpha;
      }
    }
  }

  vector<float> results((vector_size) training_number);
  ClassifierResults(z_min_index, &results);
  ComputeLastError(best_alpha, &results);

  reference_indices.push_back(z_min_index);
  reference_weights.push_back(best_alpha);
  reference_zs.push_back(z_min);
  UpdateWeights(&results, z_min, best_alpha);

  CleanUpClassifier();
  float margin = ComputeTrainingError();
  ComputeValidationError();
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);

  return margin;
}


float V_BoostMap::NextSteps(vint8 steps)
{
  vint8 i;
  float error;
  for (i = 0; i < steps; i++)
  {
    error = NextStep();
  }

  return error;
}


vint8 V_BoostMap::StepsDone()
{
  vint8 result = reference_indices.size();
  return result;
}


vint8 V_BoostMap::Classifier(vector<vint8> * out_indices, 
                            vector<float> * out_weights)
{
  vint8 steps = StepsDone();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    out_indices->push_back(reference_indices[(vector_size) i]);
    out_weights->push_back(reference_weights[(vector_size) i]);
  }

  return steps;
}


float V_BoostMap::ClassifierZ(vint8 index, float * alpha)
{
  //  vPrint("ClassifierZ, index = %li\n", index);
  // results[i] is what is called u_i in Schapire and Singer.
  vector<float> results((vector_size) training_number);
  ClassifierResults(index, &results);
  float a = 0, z = 0;
  iterations = iterations + vas_int16((long) MinimizeZ(&results, &z, &a));
  *alpha = a;
  return z;
}


// We assume that results already have enough space allocated.
vint8 V_BoostMap::ClassifierResults(vint8 index, vector<float> * results)
{
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    vint8 q = training_triples[i][0];
    vint8 a = training_triples[i][1];
    vint8 b = training_triples[i][2];

    float label;
    if (training_distances[i][0] < training_distances[i][1])
    {
      label = 1;
    }
    else
    {
      label = -1;
    }

    float qr = distances[q][index];
    float ar = distances[a][index];
    float br = distances[b][index];

    (*results)[(vector_size) i] = (vAbs(qr - br) - vAbs(qr - ar)) * label;
  }

  return 1;
}


vint8 V_BoostMap::MinimizeZ(vector<float> * results, float * z, float * a)
{
  vint8 number = results->size();
  if (number == 0)
  {
    exit_error("Error: MinimizeZ called without objects\n");
  }

  // First check if negative weights are allowed
  if (allow_negative == 0)
  {
    float dz_zero = Z_Prime((float) 0, results);
    if (dz_zero >= 0)
    {
      *z = Z((float) 0, results);
      *a = (float) 0;
      return 0;
    }
  }


  // Check for the pathological case where all result entries
  // have the same sign.
  float first = (*results)[0];
  vint8 flag = 0;
  vint8 i;
  for (i = 1; i < number; i++)
  {
    if ((*results)[(vector_size) i] * first < 0)
    {
      flag = 1;
    }
  }
  if (flag == 0)
  {
    *a = 1;
    *z = Z(*a, results);
    return 0;
  }

  float alpha_hi = 0, alpha_low = 0;
  float step = 1.0;
  float dz_low = 0, dz_high = 0;

  // find a value of alpha (alpha_hi) for which Z'(alpha) >= 0.
  while((dz_high = Z_Prime(alpha_hi, results)) < 0)
  {
    alpha_hi = alpha_hi + step;
    step = step * (float) 2.0;
  }
//  vPrint("alpha_hi = %f\n", alpha_hi);

  step = 1.0;
  // find a value of alpha (alpha_low) for which Z'(alpha) <= 0.
  while((dz_low = Z_Prime(alpha_low, results)) > 0)
  {
    alpha_low = alpha_low - step;
    step = step * (float) 2.0;
  }
//  vPrint("alpha_low = %f\n", alpha_low);

  float alpha;
  vint8 counter = 0;
  const float adjustment = (float) 0.01;
  // Now use binary search to find a root of Z'.
  while(1)
  {
    // The next few lines do search using linear interpolation
    // to improve the guess and minimize the number of iterations
    float range = (dz_high - dz_low) * (float) 1.01;
    if (range < 0.000001) break;

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


vint8 V_BoostMap::MinimizeZ5(vector<float> * results, float * z, float * a,
                            float alpha_low, float alpha_hi)
{
  vint8 number = results->size();
  if (number == 0)
  {
    exit_error("Error: MinimizeZ called without objects\n");
  }

  float dz_low = Z_Prime(alpha_low, results);
  if (dz_low >= 0)
  {
    *a = alpha_low;
    *z = Z(alpha_low, results);
    return 0;
  }

  float dz_high = Z_Prime(alpha_hi, results);
  if (dz_high <= 0)
  {
    *a = dz_high;
    *z = Z(alpha_hi, results);
    return 0;
  }
    
  float alpha;
  vint8 counter = 0;
  const float adjustment = (float) 0.01;
  // Now use binary search to find a root of Z'.
  while(1)
  {
    // The next few lines do search using linear interpolation
    // to improve the guess and minimize the number of iterations
    float range = (dz_high - dz_low) * (float) 1.01;
    if (range < 0.000001) break;

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

    // The next line is good for simple binary search
//    alpha = (alpha_hi + alpha_low) / 2.0;

    if ((alpha_hi - alpha < 0.000001) || 
        (alpha - alpha_low < 0.000001))
    {
      break;
    }

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

    counter++;
  }

  *z = Z(alpha, results);
  *a = alpha;
  return counter;
}


float V_BoostMap::Z(float a, vector<float> * results)
{
  vint8 i;
  float sum = 0;
  for (i = 0; i < training_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[(vector_size) i];
    float exp_i = vPrecomputedExp::Exp(-a * u_i);
    sum = sum + d_i * exp_i;
  }

  return sum;
}


float V_BoostMap::Z_Prime(float a, vector<float> * results)
{
  vint8 i;
  float sum = 0;
  for (i = 0; i < training_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[(vector_size) i];
    float exponent = -a * u_i;
    if (exponent < min_exp) min_exp = exponent;
    if (exponent > max_exp) max_exp = exponent;
    float exp_i = vPrecomputedExp::Exp(exponent);
//    float exp_i = exponent;
    sum = sum - d_i * u_i * exp_i;
  }

  return sum;
}

  
float V_BoostMap::UpdateWeights(vector<float> * results, float z, float a)
{
  if (z <= 0)
  {
    exit_error("Error: non-positive z in UpdateWeights: %f\n", z);
  }

  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[(vector_size) i];
    float exp_i = vPrecomputedExp::Exp(-a * u_i);
    training_factors[i] = d_i * exp_i / z;
  }

  return z;
}


float V_BoostMap::ComputeLastError(float alpha, vector<float> * results)
{
  if (object_number == 0) return 0;
  float sum = 0;
  vint8 i;
  float accuracy = 0;
  for (i = 0; i < training_number; i++)
  {
    float d_i = training_factors[i];
    float u_i = (*results)[(vector_size) i];
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


float V_BoostMap::ComputeTrainingError()
{
  vint8 steps = cleaned_indices.size();
  if ((steps <= 0) || (training_number == 0))
  {
    exit_error("Error in ComputeTrainingError\n");
  }

  vint8 i;
  float weight = (float) 1.0 / (float) training_number;
  training_error = training_margin = 0.0;
  for (i = 0; i < training_number; i++)
  {
    vint8 q_index = training_triples[i][0];
    vint8 a_index = training_triples[i][1];
    vint8 b_index = training_triples[i][2];

    float label;
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long) i, (long) training_number, (long) q_index, (long) a_index, (long) b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      vPrint("Error: training triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    vMatrix<float> e_q = Embedding1(q_index);
    vMatrix<float> e_a = Embedding1(a_index);
    vMatrix<float> e_b = Embedding1(b_index);

    float e_qa = L1_Distance(e_q, e_a);
    float e_qb = L1_Distance(e_q, e_b);
    float margin = (e_qb - e_qa) * label;
    if (margin == 0) 
    {
      training_error = training_error + weight / (float) 2.0;
    }
    else if (margin < 0)
    {
      training_error = training_error + weight;
    }
    training_margin = training_margin + weight * margin;
  }

  return training_margin;
}


float V_BoostMap::TrainingError()
{
  return training_error;
}


float V_BoostMap::TrainingMargin()
{
  return training_margin;
}


float V_BoostMap::ValidationError()
{
  return validation_error;
}


float V_BoostMap::LastError()
{
  return last_error;
}


float V_BoostMap::LastEt()
{
  return last_et;
}


float V_BoostMap::LastCorrelation()
{
  return last_correlation;
}


vMatrix<float> V_BoostMap::Embedding1(vint8 index)
{
  return Embedding2(index, &reference_indices);
}


vMatrix<float> V_BoostMap::Embedding2(vint8 index, vector<vint8> * references)
{
  vint8 steps = references->size();
  if (steps <= 0)
  {
    exit_error("Error: Embedding2 called after zero steps\n");
  }

  vMatrix<float> result_matrix(1, steps);
  vArray(float) result = result_matrix.Matrix();
  
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    vint8 index2 = (*references)[(vector_size) i];
    result[i] = distances[index][index2];
  }

  return result_matrix;

}

  
float V_BoostMap::L1_Distance(vMatrix<float> v1, vMatrix<float> v2)
{
  return L1_Distance3(v1, v2, &reference_weights);
}


float V_BoostMap::L1_Distance3(vMatrix<float> v1, vMatrix<float> v2,
                               vector<float> * weights)
{
  float sum = 0;
  vint8 length = v1.Cols();
  vArray(float) v3 = v1.Matrix();
  vArray(float) v4 = v2.Matrix();

  vint8 i;
  for (i = 0; i < length; i++)
  {
    float distance = vAbs(v3[i] - v4[i]);
    sum = sum + (*weights)[(vector_size) i] * distance;
  }
  return sum;
}

  
float V_BoostMap::SumWeights()
{
  float sum = 0;
  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    sum = sum + training_factors[i];
  }
  
  return sum;
}


float V_BoostMap::SumClassifierWeights()
{
  float sum = 0;
  vint8 i;
  for (i = 0; i < (vint8) reference_weights.size(); i++)
  {
    sum = sum + reference_weights[(vector_size) i];
  }
  
  return sum;
}


vint8 V_BoostMap::PrintSummary()
{
  if (object_number == 0) return 0;
  vPrint("\n");
  vPrint("BoostMap: %li objects, %li triples, %li validation, %li candidates\n",
          (long) object_number, (long) training_number, (long) validation_number, (long) candidate_number);
  vPrint("ranges: candidate = %li, training = %li, validation = %li\n",
          (long) candidate_range, (long) training_range, (long) validation_range);
  vPrint("min_exp = %f, max_exp = %f\n", min_exp, max_exp);
  vPrint("iterations = ");
  iterations.Print();
  vPrint("\n");

  vPrint("weight sum = %.3f, validation size = %li\n",
          SumWeights(), (long) validation_number);
  vPrint("last error = %f, et = %f, correlation = %f\n",
          last_error, last_et, last_correlation);
  vPrint("last_scaled_et = %f, last_scaled_correlation = %f\n",
          last_scaled_et, last_scaled_correlation);
  vPrint("TRAINING_ERROR = %f, margin = %f, validation = %f\n",
          training_error, training_margin, validation_error);

  vint8 steps = StepsDone();
  vPrint("%li steps, net classifiers = %li\n", (long) steps, (long) cleaned_indices.size());

  vPrint("\n");
  return 1;
}


vint8 V_BoostMap::PrintClassifier()
{
  vint8 steps = StepsDone();
  vPrint("%li steps\n", (long) steps);
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    vPrint("%li %li %f %f\n", 
            (long) i, (long) reference_indices[(vector_size) i], reference_weights[(vector_size) i], 
            reference_zs[(vector_size) i]);
  }

  vPrint("\n");
  return 1;
}


vint8 V_BoostMap::PrintAll()
{
  if (object_number == 0) return 0;
  distances_matrix.Print("distances");
  training_triples_matrix.Print("training indices");
  validation_triples_matrix.Print("validation indices");
  training_distances_matrix.Print("training distances");
  training_factors_matrix.Print("training weights");
  PrintClassifier();
  PrintSummary();
  vPrint("\n");
  return 1;
}


// Pick random reference points, and compute error and margin for them.
vint8 V_BoostMap::PickRandom(vint8 number)
{
  if (number <= 0) return 0;
  if (training_number == 0) return 0;
  if (object_number == 0) return 0;
  // Back up currently chosen objects and weights.
  vint8 steps = reference_indices.size();
  vint8 steps2 = Max(steps, (vint8) 1);
  vector<vint8> indices_backup((vector_size) steps2);
  vector<float> weights_backup((vector_size) steps2);

  vint8 i;
  for (i = 0; i < steps; i++)
  {
    indices_backup[(vector_size) i] = reference_indices[(vector_size) i];
    weights_backup[(vector_size) i] = reference_weights[(vector_size) i];
  }

  // Pick random points
  reference_indices = vector<vint8>((vector_size) number);
  reference_weights = vector<float>((vector_size) number);
  for (i = 0; i < number; i++)
  {
    reference_indices[(vector_size) i] = (vint8) function_random_vint8(0, object_number-1);
    reference_weights[(vector_size) i] = 1.0;
  }

  // Compute errors
  ComputeTrainingError();
  ComputeValidationError();

  // Restore original objects and weights
  if (steps > 0)
  {
    reference_indices = vector<vint8>((vector_size) steps);
    reference_weights = vector<float>((vector_size) steps);
    for (i = 0; i < steps; i++)
    {
      reference_indices[(vector_size) i] = indices_backup[(vector_size) i];
      reference_weights[(vector_size) i] = weights_backup[(vector_size) i];
    }
  }
  else
  {
    reference_indices.erase(reference_indices.begin(), 
                            reference_indices.end());
    reference_weights.erase(reference_weights.begin(), 
                            reference_weights.end());
  }

  return 1;
}


float V_BoostMap::StepZ(vint8 step)
{
  if ((step < 0) || (step >= (vint8) reference_zs.size()))
  {
    return -1;
  }
  return reference_zs[(vector_size) step];
}


float V_BoostMap::StepTrainingError(vint8 step)
{
  if ((step < 0) || (step >= (vint8) training_errors.size()))
  {
    return -1;
  }
  return training_errors[(vector_size) step];
}


float V_BoostMap::StepValidationError(vint8 step)
{
  if ((step < 0) || (step >= (vint8) validation_errors.size()))
  {
    return -1;
  }
  return validation_errors[(vector_size) step];
}


// Choosing a training set is not allowed if any training rounds 
// have been performed.
vint8 V_BoostMap::ChooseTrainingSet(vint8 in_training_number)
{
  vint8 steps = StepsDone();
  if (steps != 0) return 0;

  training_number = in_training_number;
  vint8 start = candidate_range;
  vint8 end = candidate_range + training_range - 1;
  if (end >= object_number) 
  {
    end = object_number - 1;
    start = end - candidate_range + 1;
  }


  training_triples_matrix = ChooseTriples((vint8) training_number, (vint8) start, (vint8) end);
  training_triples = training_triples_matrix.Matrix2();
  TrainingDistances();
  InitialWeights();

  return 1;
}


vint8 V_BoostMap::ChooseValidationSet(vint8 in_validation_number)
{
  validation_number = in_validation_number;
  vint8 start = object_number - validation_range;
  vint8 end = object_number - 1;

  validation_triples_matrix = ChooseTriples((vint8) validation_number, (vint8) start, (vint8) end);
  validation_triples = validation_triples_matrix.Matrix2();

  return 1;
}


float V_BoostMap::ComputeValidationError()
{
  vint8 steps = StepsDone();
  if ((steps <= 0) || (validation_number == 0))
  {
    exit_error("Error in ComputevalidationError\n");
  }

  vint8 i;
  float weight = (float) 1.0 / (float) validation_number;
  validation_error = 0.0;
  for (i = 0; i < validation_number; i++)
  {
    vint8 q_index = validation_triples[i][0];
    vint8 a_index = validation_triples[i][1];
    vint8 b_index = validation_triples[i][2];

    float label;
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long) i, (long) validation_number, (long) q_index, (long) a_index, (long) b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      exit_error("Error: validation triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    vMatrix<float> e_q = Embedding1(q_index);
    vMatrix<float> e_a = Embedding1(a_index);
    vMatrix<float> e_b = Embedding1(b_index);

    float e_qa = L1_Distance(e_q, e_a);
    float e_qb = L1_Distance(e_q, e_b);
    float margin = (e_qb - e_qa) * label;
    if (margin == 0) 
    {
      validation_error = validation_error + weight / (float) 2.0;
    }
    else if (margin < 0)
    {
      validation_error = validation_error + weight;
    }
  }

  return validation_error;
}


vint8 V_BoostMap::ChooseCandidates(vint8 in_candidate_number)
{
  candidate_number = in_candidate_number;
  vint8_matrix temporary = sample_without_replacement(0, candidate_range-1, 
                                                 candidate_number);

  candidates_matrix = vMatrix<vint8>(&temporary);
  candidates = candidates_matrix.Matrix();
  return 1;
}


vMatrix<vint8> V_BoostMap::ChooseTriples(vint8 number, vint8 start, vint8 end)
{
  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q_index = function_random_vint8(start, end);
    vint8 a_index = function_random_vint8(start, end);
    vint8 b_index = function_random_vint8(start, end);
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    float ab = distances[a_index][b_index];

    if ((vAbs(qa - qb) < 0.000001) ||
        (vAbs(qa - ab) < 0.000001) ||
        (vAbs(qb - ab) < 0.000001))
    {
      i--;
      continue;
    }

    result[i][0] = (vint8) q_index;
    result[i][1] = (vint8) a_index;
    result[i][2] = (vint8) b_index;
  }
  return result_matrix;
}


vint8 V_BoostMap::SetPickCandidatesFlag(vint8 value)
{
  pick_candidates_flag = value;
  return 1;
}


float V_BoostMap::Distance(vint8 i, vint8 j)
{
  vint8 rows = (vint8) distances_matrix.Rows();
  vint8 cols = (vint8) distances_matrix.Cols();
  if ((i < 0) || (j < 0) || (i >= rows) || (j >= cols))
  {
    return -1;
  }
  return distances[i][j];
}


// This function is useful when we want to fully specify the classifier,
// for example if we want to load an existing classifier.
vint8 V_BoostMap::AddClassifier(vint8 index, float alpha)
{
  vector<float> results((vector_size) training_number);
  ClassifierResults(index, &results);
  ComputeLastError(alpha, &results);
  float z = Z(alpha, &results);

  reference_indices.push_back(index);
  reference_weights.push_back(alpha);
  reference_zs.push_back(z);
  UpdateWeights(&results, z, alpha);

  CleanUpClassifier();
  float margin = ComputeTrainingError();
  ComputeValidationError();
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);

  return 1;
}


vint8 V_BoostMap::SetAllowNegative(vint8 in_value)
{
  allow_negative = in_value;
  return 1;
}


vint8 V_BoostMap::SetAllowRemovals(vint8 in_value)
{
  allow_removals = in_value;
  return 1;
}


vint8 V_BoostMap::CleanUpClassifier()
{
  CleanUp((vint8) object_number, &reference_indices, &reference_weights,
          &cleaned_indices, &cleaned_factors);
  return 1;
}


vint8 V_BoostMap::CleanUp(vint8 object_number, vector<vint8> * reference_indices,
                         vector<float> * reference_weights,
                         vector<vint8> * cleaned_indices,
                         vector<float> * cleaned_factors)
{
  vMatrix<double> scratch_matrix(1, object_number);
  vArray(double) scratch = scratch_matrix.Matrix();
  cleaned_indices->erase(cleaned_indices->begin(), cleaned_indices->end());
  cleaned_factors->erase(cleaned_factors->begin(), cleaned_factors->end());
  vint8 number = reference_indices->size();

  vint8 i;
  // Zero out all weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*reference_indices)[(vector_size) i];
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
    vint8 index = (*reference_indices)[(vector_size) i];
    if (index < 0) continue;
    double weight = (*reference_weights)[(vector_size) i];
    scratch[index] = scratch[index] + weight;
  }

  // Get the combined weights
  for (i = 0; i < number; i++)
  {
    vint8 index = (*reference_indices)[(vector_size) i];
    if (index < 0) continue;
    double weight = scratch[index];
    if (vAbs(weight) > 0.000001)
    {
      cleaned_indices->push_back(index);
      cleaned_factors->push_back((float) weight);
      scratch[index] = 0;
    }
  }

  vint8 number2 = cleaned_indices->size();
  vPrint("Picked %li of %li classifiers\n",
          (long) number2, (long) number);
  return 1;
}


// We assume that reference_images(i, 3) is the index
// into the distances matrix corresponding to the i-th 
// reference image.
v3dMatrix<float> * V_BoostMap::CleanUp(vint8 object_number, 
                                         v3dMatrix<float> * reference_images)
{
  vArray2(float) references = reference_images->Matrix2(0);
  vint8 number = (vint8) reference_images->Rows();
  vector<vint8> in_indices((vector_size) number);
  vector<float> in_weights((vector_size) number);

  vint8 i;
  for (i = 0; i < number; i++)
  {
    in_indices[(vector_size) i] = (vint8) round_number(references[(vector_size) i][3]);
    in_weights[(vector_size) i] = references[(vector_size) i][2];
  }

  vector<vint8> out_indices;
  vector<float> out_weights;
  CleanUp(object_number, &in_indices, &in_weights, 
          &out_indices, &out_weights);

  vint8 number2 = out_indices.size();
  v3dMatrix<float> * result_matrix = new v3dMatrix<float>(number2, 4, 1);
  vArray2(float) result = result_matrix->Matrix2(0);

  for (i = 0; i < number2; i++)
  {
    vint8 out_index = out_indices[(vector_size) i];
    float weight = out_weights[(vector_size) i];
    
    vint8 in_index = -1;
    vint8 j;
    for (j = 0; j < number; j++)
    {
      vint8 index = in_indices[(vector_size) j];
      if (index == out_index)
      {
        in_index = j;
        break;
      }
    }

    if (in_index == -1)
    {
      exit_error("Error, index %li not found\n", (long) out_index);
    }

    result[i][0] = references[in_index][0];
    result[i][1] = references[in_index][1];
    result[i][2] = weight;
    result[i][3] = (float) out_index;
  }
  return result_matrix;
}

  
// Here we identify repetitions of classifiers and we combine them.
vint8 V_BoostMap::CleanClassifier(vector<vint8> * out_indices, 
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

  
// Return zero if no removal was successful, one otherwise.
vint8 V_BoostMap::TryRemovals(vint8 * z_min_indexp, float * z_minp, float * alphap)
{
  vint8 steps = cleaned_indices.size();
  if (steps == 0) return 0;

  float min_z = (float) 10, min_z_alpha = 0;
  vint8 min_z_index = 0;
  vector<float> results((vector_size) training_number);
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    vint8 index = cleaned_indices[(vector_size) i];
    float weight = cleaned_factors[(vector_size) i];
    ClassifierResults(index, &results);
    float z = Z(-weight, &results);
    if (z < min_z)
    {
      min_z = z;
      min_z_alpha = -weight;
      min_z_index = index;
    }
  }

  *z_min_indexp = min_z_index;
  *z_minp = min_z;
  *alphap = min_z_alpha;
  if (min_z < 1)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


vint8 V_BoostMap::TryWeightChange(vint8 * z_min_indexp, float * z_minp, float * alphap)
{
  *z_minp = 2;
  vint8 steps = cleaned_indices.size();
  if (steps == 0) return 0;

  float min_z = (float) 10, min_z_alpha = 0;
  vint8 min_z_index = 0;
  vector<float> results((vector_size) training_number);
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    vint8 index = cleaned_indices[(vector_size) i];
    float weight = cleaned_factors[(vector_size) i];
    ClassifierResults(index, &results);
    float z = 0, alpha = 0;
    MinimizeZ(&results, &z, &alpha);

    if (((weight + alpha) < 0) && (allow_negative == 0))
    {
      continue;
    }

    if (z < min_z)
    {
      min_z = z;
      min_z_alpha = alpha;
      min_z_index = index;
    }
  }

  *z_min_indexp = min_z_index;
  *z_minp = min_z;
  *alphap = min_z_alpha;
  if (min_z < .9999)
  {
    return 1;
  }
  else
  {
    return 0;
  }
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//            End of implementation of V_BoostMap                  //
//                                                                 //
/////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////
//                                                                 //
//             Start of implementation of V_BoostMap2              //
//                                                                 //
/////////////////////////////////////////////////////////////////////


V_BoostMap2::V_BoostMap2()
{
  Zero();
}


V_BoostMap2::V_BoostMap2(vMatrix<float> in_distances,
                         vint8 in_candidate_range, vint8 in_training_range,
                         vint8 in_number_of_picked_candidates, 
                         vint8 in_training_number, vint8 in_validation_number)
{
  object_number = in_distances.Rows();
  candidate_range = in_candidate_range;
  if (candidate_range > object_number)
  {
    vPrint("Resetting candidate_range to object_number %li\n", 
            (long) object_number);
  }
  training_range = in_training_range;

  validation_range = object_number - candidate_range - training_range;
  if (validation_range < 0)
  {
    vPrint("setting validation_range = training_range = %li\n", 
            (long) training_range);
    validation_range = training_range;
  }

  vMatrix<vint8> candidate_range_matrix = vMatrix<vint8>::Range(0, candidate_range-1);

  vint8 training_start = (vint8) candidate_range;
  vint8 training_end = (vint8) (candidate_range + training_range - 1);
  vMatrix<vint8> training_range_matrix = vMatrix<vint8>::Range(training_start, training_end);

  vint8 validation_start = (vint8) (object_number - validation_range);
  vint8 validation_end = (vint8) object_number - 1;
  vMatrix<vint8> validation_range_matrix = vMatrix<vint8>::Range(validation_start, validation_end);
  Initialize7(in_distances, candidate_range_matrix, training_range_matrix,
              validation_range_matrix, in_number_of_picked_candidates,
              in_training_number, in_validation_number);
}


V_BoostMap2::V_BoostMap2(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_training_indices,
                         vMatrix<vint8> in_validation_indices,
                         vint8 in_candidate_number,
                         vint8 in_training_number, 
                         vint8 in_validation_number)
{
  Initialize7(in_distances, in_candidate_indices,
              in_training_indices, in_validation_indices,
              in_candidate_number,
              in_training_number, in_validation_number);
}


V_BoostMap2::V_BoostMap2(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_training_triples,
                         vMatrix<vint8> in_validation_triples,
                         vint8 in_candidate_number)
{
  Initialize5(in_distances, in_candidate_indices,
              in_training_triples, in_validation_triples,
              in_candidate_number);
}


V_BoostMap2::~V_BoostMap2()
{
}


vint8 V_BoostMap2::Initialize5(vMatrix<float> in_distances, 
                              vMatrix<vint8> in_candidate_indices,
                              vMatrix<vint8> in_training_triples,
                              vMatrix<vint8> in_validation_triples,
                              vint8 in_candidate_number)
{
  class_name = vCopyString("V_BoostMap2");

  // Verify that the max distance in in_distances is 1. Otherwise
  // the distances are not acceptable.
  float max_distance = function_image_maximum(&in_distances);
  if (max_distance > 1)
  {
    return 0;
  }

  distances_matrix = in_distances;
  candidate_indices_matrix = in_candidate_indices;
  candidate_number = candidates_matrix.Cols();
  object_number = distances_matrix.Rows();
  number_of_picked_candidates = in_candidate_number;

  training_triples_matrix = in_training_triples;
  validation_triples_matrix = in_validation_triples;

  training_triples = training_triples_matrix.Matrix2();
  validation_triples = validation_triples_matrix.Matrix2();
  training_number = training_triples_matrix.Rows();
  validation_number = validation_triples_matrix.Rows();

  candidate_indices = candidate_indices_matrix.Matrix();
  training_indices = training_indices_matrix.Matrix();
  validation_distances = validation_distances_matrix.Matrix2();

  distances = distances_matrix.Matrix2();
  ChooseCandidates(number_of_picked_candidates);
  training_distances_matrix = TripleDistances(training_triples_matrix);
  validation_distances_matrix = TripleDistances(validation_triples_matrix);
  InitialWeights();

  training_margins_matrix = vMatrix<float>(1, training_number);
  function_enter_value(&training_margins_matrix, (float) 0);
  validation_margins_matrix = vMatrix<float>(1, validation_number);
  function_enter_value(&validation_margins_matrix, (float) 0);

  training_distances = training_distances_matrix.Matrix2();
  training_margins = training_margins_matrix.Matrix();

  validation_distances = validation_distances_matrix.Matrix2();
  validation_margins = validation_margins_matrix.Matrix();

  pick_candidates_flag = 1;
  return 1;
}

                              
vint8 V_BoostMap2::Initialize7(vMatrix<float> in_distances, 
                              vMatrix<vint8> in_candidate_indices,
                              vMatrix<vint8> in_training_indices,
                              vMatrix<vint8> in_validation_indices,
                              vint8 in_candidate_number,
                              vint8 in_training_number, 
                              vint8 in_validation_number)
{
  class_name = vCopyString("V_BoostMap2");

  // Verify that the max distance in in_distances is 1. Otherwise
  // the distances are not acceptable.
//  float max_distance = function_image_maximum(&in_distances);
//  if (max_distance > 1)
//  {
//    return 0;
//  }

  distances_matrix = in_distances;
  candidate_indices_matrix = in_candidate_indices;
  training_indices_matrix = in_training_indices;
  validation_indices_matrix = in_validation_indices;
  
  candidate_number = candidates_matrix.Cols();
  object_number = distances_matrix.Rows();
  number_of_picked_candidates = in_candidate_number;
  training_number = in_training_number;
  validation_number = in_validation_number;

  candidate_indices = candidate_indices_matrix.Matrix();
  training_indices = training_indices_matrix.Matrix();
  validation_indices = validation_indices_matrix.Matrix();
  distances = distances_matrix.Matrix2();

  ChooseCandidates((vint8) number_of_picked_candidates);
  ChooseTrainingSet((vint8) training_number);
  ChooseValidationSet((vint8) validation_number);
  InitialWeights();


  training_distances = training_distances_matrix.Matrix2();
  training_margins = training_margins_matrix.Matrix();

  validation_distances = validation_distances_matrix.Matrix2();
  validation_margins = validation_margins_matrix.Matrix();

  pick_candidates_flag = 1;
  return 1;

}


vint8 V_BoostMap2::ChooseCandidates(vint8 in_candidate_number)
{
  vint8 candidate_indices_length = (vint8) candidate_indices_matrix.Cols();
  candidate_number = in_candidate_number;
  candidates_matrix = vMatrix<vint8>(1, candidate_number);
  candidates = candidates_matrix.Matrix();

  
  vint8_matrix temporary = sample_without_replacement(0, candidate_indices_length-1, 
                               candidate_number);

  vMatrix<vint8> random_matrix(& temporary);
  vArray(vint8) random_indices = random_matrix.Matrix();

  vint8 i;
  for (i = 0; i < candidate_number; i++)
  {
    vint8 index = random_indices[i];
    candidates[i] = candidate_indices[index];
  }

  return 1;
}


vint8 V_BoostMap2::ChooseTrainingSet(vint8 in_training_number)
{
  training_number = in_training_number;
  TriplesAndDistances((vint8) training_number, training_indices_matrix,
                      &training_triples_matrix,                  
                      &training_distances_matrix, &training_margins_matrix);
  training_triples = training_triples_matrix.Matrix2();
  training_distances = training_distances_matrix.Matrix2();
  training_margins = training_margins_matrix.Matrix();
  InitialWeights();
  return 1;
}


vint8 V_BoostMap2::ChooseValidationSet(vint8 in_validation_number)
{
  validation_number = in_validation_number;
  TriplesAndDistances((vint8) validation_number, validation_indices_matrix,
                      &validation_triples_matrix,                  
                      &validation_distances_matrix, &validation_margins_matrix);
  validation_triples = validation_triples_matrix.Matrix2();
  validation_distances = validation_distances_matrix.Matrix2();
  validation_margins = validation_margins_matrix.Matrix();
  return 1;
}


vMatrix<vint8> V_BoostMap2::ChooseTriples(vint8 number, 
                                          vMatrix<vint8> indices_matrix)
{
  vMatrix<vint8> result_matrix(number, 3);
  vArray2(vint8) result = result_matrix.Matrix2();
  vArray(vint8) indices = indices_matrix.Matrix();

  vint8 number_of_indices = (vint8) indices_matrix.Cols();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 temp_q_index = function_random_vint8(0, number_of_indices-1);
    vint8 temp_a_index = function_random_vint8(0, number_of_indices-1);
    vint8 temp_b_index = function_random_vint8(0, number_of_indices-1);

    vint8 q_index = indices[temp_q_index];
    vint8 a_index = indices[temp_a_index];
    vint8 b_index = indices[temp_b_index];

    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    float ab = distances[a_index][b_index];

    if ((vAbs(qa - qb) < 0.000001) ||
        (vAbs(qa - ab) < 0.000001) ||
        (vAbs(qb - ab) < 0.000001))
    {
      i--;
      continue;
    }

    result[i][0] = q_index;
    result[i][1] = a_index;
    result[i][2] = b_index;
  }

  return result_matrix;
}


vMatrix<float> V_BoostMap2::TripleDistances(vMatrix<vint8> triples_matrix)
{
  vint8 number = triples_matrix.Rows();
  vArray2(vint8) triples = triples_matrix.Matrix2();
  
  vMatrix<float> out_distances_matrix = vMatrix<float>(number, 3);
  vArray2(float) out_distances = out_distances_matrix.Matrix2();
  
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index0 = triples[i][0];
    vint8 index1 = triples[i][1];
    vint8 index2 = triples[i][2];

    if ((index0 < 0) || (index1 < 0) || (index2 < 0) ||
        (index0 >= object_number) || (index1 >= object_number) ||
        (index2 >= object_number))
    {
      exit_error("TriplesAndDistances error: bad indices\n");
    }

    float distance01 = distances[index0][index1];
    float distance02 = distances[index0][index2];
    float distance12 = distances[index1][index2];
    out_distances[i][0] = distance01;
    out_distances[i][1] = distance02;
    out_distances[i][2] = distance12;
  }

  return out_distances_matrix;
}

vint8 V_BoostMap2::TriplesAndDistances(vint8 number, 
                                      vMatrix<vint8> indices_matrix,
                                      vMatrix<vint8> * triples_matrixp,
                                      vMatrix<float> * distances_matrixp,
                                      vMatrix<float> * margins_matrixp)
{
  vMatrix<vint8> triples_matrix = ChooseTriples(number, indices_matrix);
  vMatrix<float> distances_matrix = TripleDistances(triples_matrix);
  vMatrix<float> margins_matrix(1, number);
  function_enter_value(&margins_matrix, (float) 0);

  *triples_matrixp = triples_matrix;
  *distances_matrixp = distances_matrix;
  *margins_matrixp = margins_matrix;
  return 1;
}


vint8 V_BoostMap2::UpdateMargins(vMatrix<vint8> triples_matrix,
                               vMatrix<float> margins_matrix,
                               float * errorp, float * marginp)
{
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray(float) margins = margins_matrix.Matrix();
  vint8 number = (vint8) triples_matrix.Rows();
  float weight = (float) 1.0 / (float) number;
  vint8 rounds = reference_indices.size();
  vint8 last_index = rounds - 1;
  vint8 reference_index = reference_indices[(vector_size) last_index];
  float reference_weight = reference_weights[(vector_size) last_index];
  float error = 0, margin = 0;

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q_index = triples[i][0];
    vint8 a_index = triples[i][1];
    vint8 b_index = triples[i][2];

    float label;
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long) i, (long)number, (long)q_index, (long)a_index, (long)b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      vPrint("Error: training triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    // Compute the contribution that the last chosen reference image
    // makes to the estimation of distances QA and QB
    float q_distance = distances[reference_index][q_index];
    float a_distance = distances[reference_index][a_index];
    float b_distance = distances[reference_index][b_index];
    float qa_estimate = vAbs(q_distance - a_distance);
    float qb_estimate = vAbs(q_distance - b_distance);
    float current_estimate = reference_weight * (qb_estimate - qa_estimate) * label;

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


  
// Computes errors and margins
float V_BoostMap2::ComputeTrainingError()
{
  UpdateMargins(training_triples_matrix, training_margins_matrix,
                &training_error, &training_margin);
  
  return training_margin;
}


float V_BoostMap2::ComputeValidationError()
{
  float validation_margin = 0;

  UpdateMargins(validation_triples_matrix, validation_margins_matrix,
                &validation_error, &validation_margin);
  
  return validation_error;
}


vint8 V_BoostMap2::PickRandom(vint8 number)
{
  return PickRandom3(number, &training_error, &validation_error);
}


vint8 V_BoostMap2::PickRandom3(vint8 number, float * training_errorp,
                              float * validation_errorp)
{
  if (number <= 0) return 0;
  if (training_number == 0) return 0;
  if (object_number == 0) return 0;

  // Pick random points
  vector<vint8> random_references((vector_size) number);
  vector<float> uniform_weights((vector_size) number);

  vint8 i;
  for (i = 0; i < number; i++)
  {
    random_references[(vector_size) i] = (vint8) function_random_vint8(0, object_number-1);
    uniform_weights[(vector_size) i] = 1.0;
  }

  // Compute errors
  float error_training = ComputeError(&random_references, &uniform_weights,
                                      training_triples_matrix);
  float error_validation = ComputeError(&random_references, &uniform_weights,
                                        validation_triples_matrix);

  *training_errorp = error_training;
  *validation_errorp = error_validation;
  return 1;
}


float V_BoostMap2::ComputeError(vector<vint8> * references, vector<float> * weights,
                                vMatrix<vint8> triples_matrix)
{
  vint8 number = (vint8) triples_matrix.Rows();
  vint8 dimensions = references->size();
  vArray2(vint8) triples = triples_matrix.Matrix2();
  float error = 0;
  float factor = 1 / (float) number;

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q_index = triples[i][0];
    vint8 a_index = triples[i][1];
    vint8 b_index = triples[i][2];

    float label;
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long)i, (long)number, (long)q_index, (long)a_index, (long) b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      vPrint("Error: training triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    vMatrix<float> e_q = Embedding2(q_index, references);
    vMatrix<float> e_a = Embedding2(a_index, references);
    vMatrix<float> e_b = Embedding2(b_index, references);

    float e_qa = L1_Distance3(e_q, e_a, weights);
    float e_qb = L1_Distance3(e_q, e_b, weights);
    float margin = (e_qb - e_qa) * label;
    if (margin == 0) 
    {
      error = error + factor / (float) 2.0;
    }
    else if (margin < 0)
    {
      error = error + factor;
    }
    margin = margin + factor * margin;
  }

  return error;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//              End of implementation of V_BoostMap2               //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vint8 class_triple_classifier::Print()
{
  vPrint("type %li: (%li, %li), weight = %f, z = %f\n",
          (long) type, (long) object1, (long) object2, weight, z);
  return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//              Start of implementation of V_BoostMap3             //
//                                                                 //
/////////////////////////////////////////////////////////////////////


V_BoostMap3::V_BoostMap3() : V_BoostMap2()
{
  Zero();
  vdelete2(class_name);
  class_name = vCopyString("V_BoostMap3");
}


V_BoostMap3::V_BoostMap3(vMatrix<float> in_distances,
                         vint8 in_candidate_range, vint8 in_training_range,
                         vint8 in_number_of_picked_candidates, 
                         vint8 in_training_number, vint8 in_validation_number) :
V_BoostMap2(in_distances, in_candidate_range, in_training_range,
            in_number_of_picked_candidates, in_training_number, 
            in_validation_number)
{
  Zero();
  vdelete2(class_name);
  class_name = vCopyString("V_BoostMap3");
  projection_candidate_number = in_number_of_picked_candidates;
}


V_BoostMap3::V_BoostMap3(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_training_indices,
                         vMatrix<vint8> in_validation_indices,
                         vint8 in_candidate_number,
                         vint8 in_training_number, 
                         vint8 in_validation_number) :
V_BoostMap2(in_distances, in_candidate_indices, in_training_indices,
            in_validation_indices, in_candidate_number,
            in_training_number, in_validation_number)
{
  Zero();
  vdelete2(class_name);
  class_name = vCopyString("V_BoostMap3");
  projection_candidate_number = in_candidate_number;
}


V_BoostMap3::V_BoostMap3(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_training_triples,
                         vMatrix<vint8> in_validation_triples,
                         vint8 in_candidate_number) :
V_BoostMap2(in_distances, in_candidate_indices, in_training_triples,
            in_validation_triples, in_candidate_number)
{
  Zero();
  vdelete2(class_name);
  class_name = vCopyString("V_BoostMap3");
  projection_candidate_number = in_candidate_number;
}


V_BoostMap3::~V_BoostMap3()
{
}


vint8 V_BoostMap3::Zero()
{
  allow_projections = 1;
  allow_lipschitz = 1;
  last_new_z = (float) 0;
  return 1;
}


vint8 V_BoostMap3::SetAllowProjections(vint8 in_value)
{
  allow_projections = in_value;
  return allow_projections;
}


vint8 V_BoostMap3::SetAllowLipschitz(vint8 in_value)
{
  allow_lipschitz = in_value;
  return allow_lipschitz;
}


vMatrix<vint8> V_BoostMap3::RandomPivots(vMatrix<vint8> candidate_indices,
                                         vint8 number)
{
  vMatrix<vint8> result_matrix(number, 2);
  vArray2(vint8) result = result_matrix.Matrix2();
  vint8 number_of_indices = (vint8) candidate_indices.Size();
  if (number_of_indices <= 1)
  {
    exit_error("Error, fewer than two candidates available\n");
  }
  vArray(vint8) indices = candidate_indices.Matrix();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 pivot1_index = (vint8) function_random_vint8(0, number_of_indices-1);
    vint8 pivot2_index = (vint8) function_random_vint8(0, number_of_indices-1);
    if (pivot1_index == pivot2_index)
    {
      i--;
      continue;
    }
    vint8 pivot1 = indices[pivot1_index];
    vint8 pivot2 = indices[pivot2_index];
    result[i][0] = pivot1;
    result[i][1] = pivot2;
  }

  return result_matrix;
}


float V_BoostMap3::ClassifierZ(class_triple_classifier * classifier, float * alpha)
{
  switch(classifier->type)
  {
  case 0:
    return V_BoostMap::ClassifierZ(classifier->object1, alpha);
    break;

  case 1:
    return PivotPairZ(classifier->object1, classifier->object2, alpha);
    break;
  }

  exit_error("Error in ClassifierZ: We should never get here\n");
  return 0;
}


vint8 V_BoostMap3::ClassifierResults(class_triple_classifier * classifier, 
                                    vector<float> * results,
                                    vMatrix<vint8> triples_matrix, 
                                    vMatrix<float> triple_distances_matrix)
{
  switch(classifier->type)
  {
  case 0:
    return LipschitzResults(classifier->object1, results,
                            triples_matrix, triple_distances_matrix);
    break;

  case 1:
    return PivotPairResults(classifier->object1, classifier->object2, results,
                            triples_matrix, triple_distances_matrix);
    break;
  }

  exit_error("Error in ClassifierZ: We should never get here\n");
  return 0;
}


float V_BoostMap3::PivotPairZ(vint8 pivot1, vint8 pivot2, float * alpha)
{
  float distance = distances[pivot1][pivot2];
  if (distance < 0.00001) 
  {
    *alpha = 0;
    return 1000000;
  }

  vector<float> results((vector_size) training_number);
  PivotPairResults(pivot1, pivot2, &results, 
                   training_triples_matrix, training_distances_matrix);
  float a = 0, z = 0;
  iterations = iterations + vas_int16((long) MinimizeZ(&results, &z, &a));
  *alpha = a;
  return z;
}


vint8 V_BoostMap3::LipschitzResults(vint8 index, vector<float> * results,
                                   vMatrix<vint8> triples_matrix, 
                                   vMatrix<float> triple_distances_matrix)
{
  vint8 number = (vint8) triples_matrix.Rows();

  vint8 i;
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray2(float) triple_distances = triple_distances_matrix.Matrix2();
  for (i = 0; i < number; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    float label;
    if (triple_distances[i][0] < triple_distances[i][1])
    {
      label = 1;
    }
    else
    {
      label = -1;
    }

    float qr = distances[q][index];
    float ar = distances[a][index];
    float br = distances[b][index];

    (*results)[(vector_size) i] = (vAbs(qr - br) - vAbs(qr - ar)) * label;
  }

  return 1;
}


vint8 V_BoostMap3::PivotPairResults(vint8 pivot1, vint8 pivot2, 
                                   vector<float> * results,
                                   vMatrix<vint8> triples_matrix, 
                                   vMatrix<float> triple_distances_matrix)
{
  vint8 number = (vint8) triples_matrix.Rows();

  vint8 i;
  float pivot_distance = distances[pivot1][pivot2];
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray2(float) triple_distances = triple_distances_matrix.Matrix2();
  for (i = 0; i < number; i++)
  {
    vint8 q = triples[i][0];
    vint8 a = triples[i][1];
    vint8 b = triples[i][2];

    float label;
    if (triple_distances[i][0] < triple_distances[i][1])
    {
      label = 1;
    }
    else
    {
      label = -1;
    }

    float qp1 = distances[q][pivot1];
    float ap1 = distances[a][pivot1];
    float bp1 = distances[b][pivot1];
    float qp2 = distances[q][pivot2];
    float ap2 = distances[a][pivot2];
    float bp2 = distances[b][pivot2];

    float qr = V_FastMap::LineProjection3(qp1, qp2, pivot_distance);
    float ar = V_FastMap::LineProjection3(ap1, ap2, pivot_distance);
    float br = V_FastMap::LineProjection3(bp1, bp2, pivot_distance);

    (*results)[(vector_size) i] = (vAbs(qr - br) - vAbs(qr - ar)) * label;
  }

  return 1;
}


// It finds the lipschitz classifier that 
// by removing it we attain the best accuracy
class_triple_classifier V_BoostMap3::LipschitzRemoval(vint8 * success)
{
  vint8 z_min_index = 0;
  float z_min = 0, best_alpha = 0;
  *success = TryRemovals(&z_min_index, &z_min, &best_alpha);
  class_triple_classifier result(z_min_index, best_alpha, z_min);
  return result;
}


// It finds the best weight modification for already
// selected lipschitz classifiers
class_triple_classifier V_BoostMap3::LipschitzChange(vint8 * success)
{
  vint8 z_min_index = 0;
  float z_min = 0, best_alpha = 0;
  TryWeightChange(&z_min_index, &z_min, &best_alpha);
  float z_cutoff = ChangeCutoff();
  if (z_min <= z_cutoff) 
  {
    *success = 1;
  }
  else
  {
    *success = 0;
  }

  class_triple_classifier result(z_min_index, best_alpha, z_min);
  return result;
}


// It finds the best Lipschitz weak classifier for the
// current training step
class_triple_classifier V_BoostMap3::NextLipschitz()
{
  if (object_number == 0) return class_triple_classifier();

  float z_min = (float) 10;
  vint8 z_min_index = -1;
  float best_alpha = (float) 0;
  if (pick_candidates_flag > 0)
  {
    ChooseCandidates(number_of_picked_candidates);
  }

  if (candidate_number == 0) 
  {
    vPrint("no candidate classifiers are available\n");
    return class_triple_classifier();
  }

  float alpha = 0;
  z_min = V_BoostMap::ClassifierZ(candidates[0], &best_alpha);
  z_min_index = candidates[0];
  vint8 i;
  for (i = 1; i < candidate_number; i++)
  {
    float z = V_BoostMap::ClassifierZ(candidates[i], &alpha);
    if (z < z_min)
    {
      z_min = z;
      z_min_index = candidates[i];
      best_alpha = alpha;
    }
  }
  class_triple_classifier result(z_min_index, best_alpha, z_min);
  return result;
}


float V_BoostMap3::NextStep()
{
  exit_error("V_BoostMap3::NextStep(): unused\n");

  return 0.0f;
}


vint8 V_BoostMap3::StepsDone()
{
  vint8 result = classifiers.size();
  return result;
}


float V_BoostMap3::ComputeTrainingError(vector<float> * results)
{
  UpdateMargins(training_triples_matrix, training_distances_matrix,
                results, training_margins_matrix,
                &training_error, &training_margin);
  
  return training_margin;
}


float V_BoostMap3::ComputeValidationError(vector<float> * results)
{
  float validation_margin = 0;

  UpdateMargins(validation_triples_matrix, validation_distances_matrix,
                results, validation_margins_matrix,
                &validation_error, &validation_margin);
  
  return validation_error;
}



vint8 V_BoostMap3::UpdateMargins(vMatrix<vint8> triples_matrix,
                                vMatrix<float> triple_distances_matrix,
                                vector<float> * results,
                                vMatrix<float> margins_matrix,
                                float * errorp, float * marginp)
{
  vArray2(vint8) triples = triples_matrix.Matrix2();
  vArray2(float) triple_distances = triple_distances_matrix.Matrix2();
  vArray(float) margins = margins_matrix.Matrix();
  vint8 number = (vint8) triples_matrix.Rows();
  float weight = (float) 1.0 / (float) number;
  vint8 rounds = classifiers.size();
  vint8 last_index = rounds - 1;
  class_triple_classifier last_classifier = classifiers[(vector_size) last_index];
  float reference_weight = last_classifier.weight;
  float error = 0, margin = 0;

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 q_index = triples[i][0];
    vint8 a_index = triples[i][1];
    vint8 b_index = triples[i][2];

    float label;
    float qa = triple_distances[i][0];
    float qb = triple_distances[i][1];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long) i, (long) number, (long) q_index, (long) a_index, (long) b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      vPrint("Error: training triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    float current_estimate = reference_weight * (*results)[(vector_size) i];

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


vint8 V_BoostMap3::CleanClassifier(vector<class_triple_classifier> * result)
{
  vint8 steps = unique_classifiers.size();
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    result->push_back(unique_classifiers[(vector_size) i]);
  }

  return steps;
}

  
// Returns the classifiers in a matrix form, that is easier to 
// save and load.
vMatrix<float> V_BoostMap3::ClassifierMatrix()
{
  vint8 steps = unique_classifiers.size();
  vMatrix<float> result_matrix(steps, 6);
  vArray2(float) result = result_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < steps; i++)
  {
    class_triple_classifier c = unique_classifiers[(vector_size) i];
    result[i][0] = (float) c.type;
    result[i][1] = (float) c.object1;
    result[i][2] = (float) c.object2;
    result[i][3] = c.weight;
    result[i][4] = c.z;
    result[i][5] = (float) i;
  }

  return result_matrix;
}


// returns a column vector of the weighs associated with the classifiers
vMatrix<float> V_BoostMap3::WeightsMatrix()
{
  vint8 steps = unique_classifiers.size();
  vMatrix<float> result_matrix(1, steps);
  vArray(float) result = result_matrix.Matrix();

  vint8 i;
  for (i = 0; i < steps; i++)
  {
    class_triple_classifier c = unique_classifiers[(vector_size) i];
    result[i] = c.weight;
  }

  return result_matrix;
}


// Returns the classifiers (not from unique_classifiers, but from
// classifiers) in a matrix form, that is easier to  save and load.
// This matrix contains more information than the matrix returned
// in ClassifierMatrix. Here we have a row for each training step
// that was performed. We also have additional columns: one for the
// training error, one for validation error.
vMatrix<float> V_BoostMap3::DetailedClassifierMatrix()
{
  vint8 steps = classifiers.size();
  vMatrix<float> result_matrix(steps, 8);
  vArray2(float) result = result_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < steps; i++)
  {
    class_triple_classifier c = classifiers[(vector_size) i];
    result[i][0] = (float) c.type;
    result[i][1] = (float) c.object1;
    result[i][2] = (float) c.object2;
    result[i][3] = c.weight;
    result[i][4] = c.z;
    result[i][5] = training_errors[(vector_size) i];
    result[i][6] = validation_errors[(vector_size) i];
    result[i][7] = (float) i;
  }

  return result_matrix;
}

  
// Adds a new classifier, if the type of the classifier is currently
// allowed.
vint8 V_BoostMap3::AddClassifier(class_triple_classifier classifier)
{
  // Check if the classifier type is allowed.
  if ((classifier.type == 0) && (allow_lipschitz == 0)) 
  {
    return 0;
  }

  // Check if the classifier type is allowed.
  if ((classifier.type == 1) && (allow_projections == 0)) 
  {
    return 0;
  }

  vector<float> results((vector_size) training_number);
  ClassifierResults(&classifier, &results, training_triples_matrix,
                    training_distances_matrix);
  ComputeLastError(classifier.weight, &results);

  if (classifier.type == 0)
  {
    reference_indices.push_back(classifier.object1);
    reference_weights.push_back(classifier.weight);
    reference_zs.push_back(classifier.z);
    V_BoostMap::CleanUpClassifier();
  }
  classifiers.push_back(classifier);
  float z = Z(classifier.weight, &results);
  UpdateWeights(&results, z, classifier.weight);

  CleanUpClassifier(classifier);
  float margin = ComputeTrainingError(&results);

  vector<float> validation_results((vector_size) validation_number);
  ClassifierResults(&classifier, &validation_results, validation_triples_matrix,
                    validation_distances_matrix);
  ComputeValidationError(&validation_results);
  training_errors.push_back(training_error);
  validation_errors.push_back(validation_error);

  return 1;
}


// Adds, in the order in which they are given, all the classifiers
// stored in the rows of the matrix.
vint8 V_BoostMap3::AddClassifierMatrix(vMatrix<float> classifier_matrix)
{
  return AddClassifierMatrix2(classifier_matrix, -1);
}


// Adds, in the order in which they are given, all the classifiers
// stored in the rows of the matrix. It stops when either it has
// gone through all the rows of the matrix, or the next row of the
// matrix will increase the number of unique classifiers to dimensions+1.
vint8 V_BoostMap3::AddClassifierMatrix2(vMatrix<float> classifier_matrix, 
                                       vint8 dimensions)
{
  vint8 number = (vint8) classifier_matrix.Rows();
  vArray2(float) classifiers = classifier_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 type = (vint8) round_number(classifiers[i][0]);
    vint8 object1 = (vint8) round_number(classifiers[i][1]);
    vint8 object2 = (vint8) round_number(classifiers[i][2]);
    float weight = classifiers[i][3];
    float z = classifiers[i][4];
    
    class_triple_classifier classifier;
    if (type == 0)
    {
      classifier = class_triple_classifier(object1, weight, z);
    }
    else if (type == 1)
    {
      classifier = class_triple_classifier(object1, object2, weight, z);
    }
    else
    {
      exit_error("Error in AddClassifierMatrix: we shouldn't get here\n");
    }

    AddClassifier(classifier);

    // Check if we exceeded the desired number of dimensions, in
    // which case we need to backtrack and return.
    if (dimensions >= 0)
    {
      vint8 current_dimensions = unique_classifiers.size();
      if (current_dimensions == dimensions + 1)
      {
        // backtrack
        classifier.weight = -classifier.weight;
        AddClassifier(classifier);
        current_dimensions = unique_classifiers.size();
        if (current_dimensions != dimensions)
        {
          exit_error("Error: unsuccessful backtracking\n");
        }
        break;
      }
    }

    PrintSummary();
  }

  return 1;
}


vint8 V_BoostMap3::PrintSummary()
{
  vint8 steps = unique_classifiers.size();
  vint8 lipschitz_steps = cleaned_indices.size();
  vint8 projection_steps = steps - lipschitz_steps;
  V_BoostMap2::PrintSummary();
  vPrint("%li projection candidates\n", (long) projection_candidate_number);
  vPrint("%li steps, %li lipschitz, %li projections\n",
          (long) steps, (long) lipschitz_steps, (long) projection_steps);
  return 1;
}


vint8 V_BoostMap3::PrintClassifier()
{
  vMatrix<float> classifier_matrix = ClassifierMatrix();
  classifier_matrix.Print("classifier_matrix");

  vint8 steps = unique_classifiers.size();
  vint8 lipschitz_steps = cleaned_indices.size();
  vint8 projection_steps = steps - lipschitz_steps;
  vPrint("\n%li steps, %li lipschitz, %li projections\n",
          (long) steps, (long) lipschitz_steps, (long) projection_steps);
  return 1;
}


vint8 V_BoostMap3::PrintAll()
{
  if (object_number == 0) return 0;
  distances_matrix.Print("distances");
  training_triples_matrix.Print("training indices");
  validation_triples_matrix.Print("validation indices");
  training_distances_matrix.Print("training distances");
  training_factors_matrix.Print("training weights");
  PrintClassifier();
  PrintSummary();
  vPrint("\n");
 
  return 1;
}


// We add the classifier to unique_classifiers. If it
// depends on the same object (or objects) as a classifier
// that is already in unique_classifiers, and the only difference
// is the weight, then we just update the weight of the existing
// classifier. If the weight is zero then we remove the classifier.
vint8 V_BoostMap3::CleanUpClassifier(class_triple_classifier classifier)
{
  vint8 number = unique_classifiers.size();

  vector<vint8> indices_found;
  vint8 type = classifier.type;
  vint8 object1 = classifier.object1;
  vint8 object2 = classifier.object2;
  float sum_of_weights = classifier.weight;

  vint8 i;
  for (i = 0; i < number; i++)
  {
    class_triple_classifier & c = unique_classifiers[(vector_size) i];
    if (type != c.type) continue;
    if (object1 != c.object1) continue;
    if ((type == 1) && (object2 != c.object2)) continue;
    indices_found.push_back(i);
    sum_of_weights = sum_of_weights + c.weight;
  }

  vint8 found = indices_found.size();
  if (found == 0)
  {
    unique_classifiers.push_back(classifier);
  }
  else if (found == 1)
  {
    vint8 index = indices_found[0];
    if (vAbs(sum_of_weights) > 0.000001)
    {
      unique_classifiers[(vector_size) index].weight = sum_of_weights;
    }
    else  // In this case, remove the classifier.
    {
      for (i = index; i < number-1; i++)
      {
        unique_classifiers[(vector_size) i] = unique_classifiers[(vector_size) (i+1)];
      }
      unique_classifiers.pop_back();
    }
  }

  else
  {
    vPrint("found = %li\n", (long) found);
    vint8 i;
    for (i = 0; i < found; i++)
    {
      vPrint("indices_found[%li] = %li\n", (long) i, (long) indices_found[(vector_size) i]);
      unique_classifiers[(vector_size) indices_found[(vector_size) i]].Print();
    }

    exit_error("Error: Duplicate classifiers in unique_classifiers\n");
  }

  number = unique_classifiers.size();
  vint8 lipschitz_number = cleaned_indices.size();
  vint8 projection_number = number - lipschitz_number;
  vPrint("\n%li classifiers, %li lipschitz, %li projections\n",
          (long) number, (long) lipschitz_number, (long) projection_number);
  return 1;
}


char * V_BoostMap3::Directory()
{
  char * directory = "r:\\text\\boost_map\\classifiers";
  char * result = vCopyString(directory);
  return result;
} 


// We take the remainder of classifiers.size by 10, 
// and we add that to the filename. This way, if we 
// save the classifiers at the end of each step,
// we ensure that we don't write over the last file.
// This way, if the program crashes in the middle of saving
// the latest file, we still have the 9 previous files.
// We save both the cleaned up classifier as stored in
// unique_classifiers, and the detailed classifier stored
// in classifiers.
vint8 V_BoostMap3::SaveClassifier(const char * filename)
{
  vint8 number = classifiers.size() % 10;
  char * number_string = string_from_number(number);
  char * directory = Directory();
  char * temp_path_name = vJoinPaths(directory, filename);
  char * path_name = vMergeStrings4(temp_path_name, "_", 
                                     number_string, ".txt");

  vMatrix<float> classifier = ClassifierMatrix();
  vint8 success = classifier.WriteText(path_name);
  if (success <= 0)
  {
    vPrint("Failed to save classifier to %s\n", path_name);
  }
  else
  {
    vPrint("Saved classifier to %s\n", path_name);
  }

  char * path_name2 = vMergeStrings4(temp_path_name, "_", 
                                      number_string, "d.txt");
  
  classifier = DetailedClassifierMatrix();
  vint8 success2 = classifier.WriteText(path_name2);
  if (success2 <= 0)
  {
    vPrint("Failed to save classifier to %s\n", path_name2);
  }
  else
  {
    vPrint("Saved classifier to %s\n", path_name2);
  }

  vdelete2(number_string);
  vdelete2(directory);
  vdelete2(temp_path_name);
  vdelete2(path_name);
  vdelete2(path_name2);
  
  vint8 result = Min(success, success2);
  return result;
}


// Note that the filename passed in here should not be the 
// same as the filename passed to SaveClassifier, because
// SaveClassifier adds a number to it. For example, we
// may call SaveClassifier("name"), and LoadClassifier("name_4").
vint8 V_BoostMap3::LoadClassifier(const char * filename)
{
  return LoadClassifier2(filename, -1);
}


vint8 V_BoostMap3::LoadClassifier2(const char * filename, vint8 dimensions)
{
  char * directory = Directory();
  char * temp_path_name = vJoinPaths(directory, filename);
  char * path_name = vMergeStrings2(temp_path_name, ".txt");
  vdelete2(directory);
  vdelete2(temp_path_name);

  v3dMatrix<float> * temp_classifier_matrix = 
    v3dMatrix<float>::ReadText(path_name);
  if (temp_classifier_matrix == 0)
  {
    vPrint("Failed to load classifier from %s\n", path_name);
    vdelete2(path_name);
    return 0;
  }
  else
  {
    vPrint("Loaded classifier from %s\n", path_name);
  }

  vMatrix<float> classifier_matrix(temp_classifier_matrix);
  vdelete(temp_classifier_matrix);
  AddClassifierMatrix2(classifier_matrix, dimensions);
  vdelete2(path_name);
  return 1;
}


// The cutoff under which z must be in order to accept weight modifications.
float V_BoostMap3::ChangeCutoff()
{
  float diff = 1 - last_new_z;
  float threshold1 = (float) 1 - (diff / (float) 3.0);
  float threshold2 = Max((float) .999, threshold1);
  return threshold2;
}


// Return the embedding of the index-th object.
vMatrix<float> V_BoostMap3::Embedding1(vint8 index)
{
  vint8 dimensions = unique_classifiers.size();
  vMatrix<float> query_distances_matrix(1, 2*dimensions);
  vMatrix<float> pivot_distances_matrix(1, dimensions);
  vMatrix<vint8> types_matrix(1, dimensions);

  function_enter_value(&query_distances_matrix, (float) -1);
  function_enter_value(&pivot_distances_matrix, (float) -1);

  vArray(float) query_distances = query_distances_matrix.Matrix();
  vArray(float) pivot_distances = pivot_distances_matrix.Matrix();
  vArray(vint8) types = types_matrix.Matrix();

  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    class_triple_classifier classifier = unique_classifiers[(vector_size) i];
    vint8 object1 = classifier.object1;
    vint8 object2 = classifier.object2;
    vint8 type = classifier.type;

    query_distances[2*i] = distances[index][object1];
    types[i] = type;
    if (type == 1)
    {
      query_distances[2*i+1] = distances[index][object2];
      pivot_distances[i] = distances[object1][object2];
    }
  }

  return Embedding3(query_distances_matrix, pivot_distances_matrix,
                    types_matrix);
}


// query_distances is a dimensions x 2 matrix, of distances
// from the query to each of the two objects corresponding
// to each dimension. If a dimension corresponds to a lipschitz
// embedding, the second col is ignored for that dimension.
// pivot_distances has, at position i, the distance between the i-th
// pivot points (if the dimension corresponds to a projection).
// If the number of rows of query_distances is less than the
// number of unique classifiers, we use only the first classifiers.
vMatrix<float> V_BoostMap3::Embedding3(vMatrix<float> query_distances_matrix,
                                        vMatrix<float> pivot_distances_matrix,
                                        vMatrix<vint8> types_matrix)
{
  vint8 dimensions = (vint8) query_distances_matrix.Size() / 2;
  if (query_distances_matrix.Size() != 2*pivot_distances_matrix.Size())
  {
    exit_error("Error in Embedding3: dimensions don't match\n");
  }

  vArray(float) query_distances = query_distances_matrix.Matrix();
  vArray(float) pivot_distances = pivot_distances_matrix.Matrix();
  vArray(vint8) types = types_matrix.Matrix();

  vMatrix<float> result_matrix(1, dimensions);
  vArray(float) result = result_matrix.Matrix();

  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    if (types[i] == 0)
    {
      result[i] = query_distances[2*i];
    }
    else if (types[i] == 1)
    {
      float q_pivot1 = query_distances[2*i];
      float q_pivot2 = query_distances[2*i+1];
      float pivot_distance = pivot_distances[i];
      result[i] = V_FastMap::LineProjection3(q_pivot1, q_pivot2, pivot_distance);
    }
    else
    {
      exit_error("Error in Embedding3: we shouldn't get here\n");
    }
  }

  return result_matrix;
}


vMatrix<vint8> V_BoostMap3::ExtractTypes(vMatrix<float> classifiers)
{
  vint8 number = (vint8) classifiers.Rows();
  vMatrix<vint8> result_matrix(1, number);

  vint8 i;
  for (i = 0; i < number; i++)
  {
    result_matrix(i) = (vint8) round_number(classifiers(i, 0));
  }

  return result_matrix;
}

  
/////////////////////////////////////////////////////////////////////
//                                                                 //
//              End of implementation of V_BoostMap3               //
//                                                                 //
/////////////////////////////////////////////////////////////////////


  
/////////////////////////////////////////////////////////////////////
//                                                                 //
//           Start of implementation of vPlaneBoostMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vPlaneBoostMap::vPlaneBoostMap()
{
}


vPlaneBoostMap::vPlaneBoostMap(vint8 in_number_of_points, 
                                 vint8 in_training_number)
{
  object_number = in_number_of_points;
  training_number = in_training_number;
  points_matrix = PickPoints();
  points = points_matrix.Matrix2();
  distances_matrix = PointDistances();

  validation_number = training_number;
  candidate_number = object_number / 3;
  candidate_indices_matrix = vMatrix<vint8>::Range(0, candidate_number-1);
  training_indices_matrix = vMatrix<vint8>::Range(candidate_number, 
                                                  2*candidate_number-1);
  validation_indices_matrix = vMatrix<vint8>::Range(2*candidate_number, 
                                                    object_number-1);

  Initialize7(distances_matrix, candidate_indices_matrix, 
              training_indices_matrix,
              validation_indices_matrix, (vint8) candidate_number, (vint8) training_number,
              (vint8) validation_number);
  vdelete2(class_name);
  class_name = vCopyString("vPlaneBoostMap");
}


vPlaneBoostMap::~vPlaneBoostMap()
{
}


vMatrix<float> vPlaneBoostMap::PickPoints()
{
  vMatrix<float> result_matrix = vMatrix<float>(object_number, 2);
  vArray2(float) result = result_matrix.Matrix2();
  const vint8 limit = 1000000000; // one billion
  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float lx = (float) function_random_vint8(0, limit);
    float ly = (float) function_random_vint8(0, limit);
    result[i][0] = lx / (float) limit;
    result[i][1] = ly / (float) limit / 2;
  }

  return result_matrix;
}


vMatrix<float> vPlaneBoostMap::PointDistances()
{
  vMatrix<float> result_matrix(object_number, object_number);
  vArray2(float) result = result_matrix.Matrix2();
  
  vint8 i, j;
  float max_distance = 0;
  for (i = 0; i < object_number; i++)
  {
    float i_x = points[i][0];
    float i_y = points[i][1];

    for (j = 0; j <= i; j++)
    {
      float j_x = points[j][0];
      float j_y = points[j][1];
      float dx = j_x - i_x;
      float dy = j_y - i_y;
      float distance = sqrt(dx * dx + dy * dy);
      result[i][j] = distance;
      result[j][i] = distance;
      if (distance > max_distance)
      {
        max_distance = distance;
      }
    }
  }

  if (max_distance == 0)
  {
    exit_error("max_distance = 0\n");
  }

  // Normalize distances
  for (i = 0; i < object_number; i++)
  {
    for (j = 0; j < object_number; j++)
    {
      result[i][j] = result[i][j] / max_distance;
    }
  }

  return result_matrix;
}


color_image * vPlaneBoostMap::PointImage(vint8 rows, vint8 cols)
{
  if (object_number == 0) return 0;
  color_image * result = new color_image(rows, cols);
  function_enter_value(result, (uchar) 0);

  float row_scale = (float) (rows - 1);
  float col_scale = (float) (cols - 1);
  vector<uchar> colors(3);
  colors[0] = 255;
  colors[1] = 100;
  colors[2] = 100;

  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float x = points[i][0];
    float y = points[i][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawFilledCircle(result, row, col, 4, colors);
  }

  return result;
}


color_image * vPlaneBoostMap::BasisImage(vint8 rows, vint8 cols)
{
  if (object_number == 0) return 0;
  color_image * result = PointImage(rows, cols);

  float row_scale = (float) (rows - 1);
  float col_scale = (float) (cols - 1);
  vector<uchar> colors(3);
  colors[0] = 255;
  colors[1] = 255;
  colors[2] = 255;

  vint8 i;
  for (i = 0; i < (vint8) reference_indices.size(); i++)
  {
    vint8 index = reference_indices[(vector_size) i];
    float x = points[index][0];
    float y = points[index][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawFilledCircle(result, row, col, 4, colors);
    vDrawCrossv(result, row, col, 2, 8, &colors);
  }

  return result;
}


vint8 vPlaneBoostMap::PrintAll()
{
  if (object_number == 0) return 0;
  points_matrix.Print("points matrix");
  V_BoostMap::PrintAll();
  return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//             End of implementation of vPlaneBoostMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////




V_SparseMap::V_SparseMap()
{
  Zero();
}


V_SparseMap::V_SparseMap(vMatrix<float> in_distances, vint8 in_validation)
{
  Zero();
  Initialize2(in_distances, in_validation);
}


V_SparseMap::V_SparseMap(vMatrix<float> in_distances,
                         vint8 in_candidate_range, vint8 in_validation_range,
                         vint8 in_validation_number)
{
  object_number = in_distances.Rows();
  vint8 candidate_start = 0;
  vint8 candidate_end = in_candidate_range - 1;
  vint8 validation_start = (vint8) (object_number - in_validation_range);
  vint8 validation_end = (vint8) (object_number - 1);

  vMatrix<vint8> temp_candidate_indices = vMatrix<vint8>::Range(candidate_start, candidate_end);
  vMatrix<vint8> temp_validation_indices = vMatrix<vint8>::Range(validation_start, validation_end);
  Initialize4(in_distances, temp_candidate_indices, temp_validation_indices,
              in_validation_number);
}


V_SparseMap::V_SparseMap(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_validation_indices,
                         vint8 in_validation_number)
{
  Initialize4(in_distances, in_candidate_indices, in_validation_indices, 
              in_validation_number);
}


V_SparseMap::V_SparseMap(vMatrix<float> in_distances, 
                         vMatrix<vint8> in_candidate_indices,
                         vMatrix<vint8> in_validation_triples)
{
  Initialize3(in_distances, in_candidate_indices, in_validation_triples);
}
  

V_SparseMap::~V_SparseMap()
{
}


vint8 V_SparseMap::Zero()
{
  V_BoostMap2::Zero();
  log_n = 0;
  number_of_features = 0;
  features_to_use = 0;
  return 1;
}


vint8 V_SparseMap::Initialize2(vMatrix<float> in_distances, vint8 in_validation)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vMatrix<vint8> temp_candidate_range = vMatrix<vint8>::Range(0, object_number-1);
  vMatrix<vint8> temp_validation_range = vMatrix<vint8>::Range(0, object_number-1);
  return Initialize4(in_distances, temp_candidate_range, 
                     temp_validation_range, in_validation);
}


vint8 V_SparseMap::Initialize3(vMatrix<float> in_distances, 
                              vMatrix<vint8> in_candidate_indices,
                              vMatrix<vint8> in_validation_triples)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vMatrix<vint8> temp_training_triples(1, 3);
  temp_training_triples.WriteN(3, 0, 1, 2);
  vint8 candidates_to_pick = 1;
  vint8 training = 1;
  vint8 success = Initialize5(in_distances, in_candidate_indices, temp_training_triples,
                             in_validation_triples, candidates_to_pick);
  if (success <= 0) 
  {
    return success;
  }

  log_n = (vint8) (logf((float) in_candidate_indices.Size()) / logf((float) 2));
  number_of_features = log_n * log_n;
  features_to_use = number_of_features;

  PickReferenceSets0();
  embeddings_matrix = ComputeEmbeddings0();
  embeddings = embeddings_matrix.Matrix2();
  ComputeValidationError();
  vdelete2(class_name);
  class_name = vCopyString("V_SparseMap");
  return 1;
}

  
vint8 V_SparseMap::Initialize4(vMatrix<float> in_distances, 
                              vMatrix<vint8> in_candidate_indices,
                              vMatrix<vint8> in_validation_indices,
                              vint8 in_validation)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vMatrix<vint8> temp_training_range = vMatrix<vint8>::Range(0, object_number-1);
  vint8 candidates_to_pick = 1;
  vint8 training = 1;
  vint8 success = Initialize7(in_distances, in_candidate_indices, temp_training_range,
                in_validation_indices, candidates_to_pick, training, in_validation);
  if (success <= 0) 
  {
    return success;
  }

  log_n = (vint8) (logf((float) in_candidate_indices.Size()) / logf((float) 2));
  number_of_features = log_n * log_n;
  features_to_use = number_of_features;

  PickReferenceSets0();
  embeddings_matrix = ComputeEmbeddings0();
  embeddings = embeddings_matrix.Matrix2();
  ComputeValidationError();
  vdelete2(class_name);
  class_name = vCopyString("V_SparseMap");
  return 1;
}
 

  
vint8 V_SparseMap::LogN()
{
  return log_n;
}


vint8 V_SparseMap::NumberOfFeatures()
{
  return number_of_features;
}


vint8 V_SparseMap::FeaturesToUse()
{
  return features_to_use;
}


vint8 V_SparseMap::SetFeaturesToUse(vint8 in_features_to_use)
{
  if (in_features_to_use < 0) return 0;
  if (in_features_to_use > number_of_features) return 0;
  features_to_use = in_features_to_use;
  return 1;
}


vint8 V_SparseMap::PickReferenceSets0()
{
  return PickReferenceSets1(&reference_sets);
}


vint8 V_SparseMap::PickReferenceSets1(vector<vMatrix<vint8> > * sets)
{
  vint8 i;
  vint8 number = (vint8) candidate_indices_matrix.Size();
  for (i = 1; i <= log_n; i++)
  {
    vint8 cols = round_number(pow(2.0, (int) i));
    vMatrix<vint8> set = PickReferenceSets3(log_n, cols, number);
    if (set.Cols() != cols)
    {
      exit_error("error: cols = %li instead of %li\n",
                      (long) set.Cols(), (long) cols);
    }
    sets->push_back(set);
  }
  return 1;
}


vMatrix<vint8> V_SparseMap::PickReferenceSets3(vint8 rows, vint8 cols, 
                                               vint8 number)
{
  vint8 limit = number - 1;
  vMatrix<vint8> result_matrix(rows, cols);
  vArray2(vint8) result = result_matrix.Matrix2();

  vint8 row;
  for (row = 0; row < rows; row++)
  {
    vMatrix<vint8> picks_matrix = sample_without_replacement(0, limit, cols);
    vArray(vint8) picks = picks_matrix.Matrix();
    vint8 col;
    for (col = 0; col < cols; col++)
    {
      vint8 pick = (vint8) picks[col];
      vint8 index = candidate_indices[pick];
      result[row][col] = index;
    }
  }

  return result_matrix;
}

  
vMatrix<float> V_SparseMap::ComputeEmbeddings0()
{
  return ComputeEmbeddings1(&reference_sets);
}


vMatrix<float> V_SparseMap::ComputeEmbeddings1(vector<vMatrix<vint8> > * sets)
{
  vMatrix<float> result_matrix = vMatrix<float>(object_number, number_of_features);
  vArray2(float) result = result_matrix.Matrix2();

  vint8 i, j;

  for (i = 0; i < object_number; i++)
  {
    vint8 counter = 0;
    for (j = 0; j < log_n; j++)
    {
      vMatrix<vint8> i_sets_matrix = (*sets)[(vector_size) j];
      vArray2(vint8) i_sets = i_sets_matrix.Matrix2();
      vint8 cols = (vint8) i_sets_matrix.Cols();
      vint8 row;
      for (row = 0; row < log_n; row++)
      {
        // Find min distance between object i and objects in
        // the current reference set (i, row)
        vint8 index = i_sets[row][0];
        float min_distance = distances[i][index];
        vint8 col;
        for (col = 1; col < cols; col++)
        {
          index = i_sets[row][col];
          float distance = distances[i][index];
          if (distance < min_distance)
          {
            min_distance = distance;
          }
        }

        result[i][counter] = min_distance;
        counter = counter+1;
      }
    }
  }

  return result_matrix;
}


float V_SparseMap::ComputeValidationError()
{
  float error = 0;
  float weight = (float) 1.0 / (float) validation_number;

  vint8 i;
  for (i = 0; i < validation_number; i++)
  {
    vint8 q_index = validation_triples[i][0];
    vint8 a_index = validation_triples[i][1];
    vint8 b_index = validation_triples[i][2];

    float qa_distance = distances[q_index][a_index];
    float qb_distance = distances[q_index][b_index];
    float label;
    if (qa_distance < qb_distance)
    {
      label = 1;
    }
    else
    {
      label = -1;
    }

    float qa_estimate = 0;
    float qb_estimate = 0;
    vint8 j;
    for (j = 0; j < features_to_use; j++)
    {
      float q_current = embeddings[q_index][j];
      float a_current = embeddings[a_index][j];
      float b_current = embeddings[b_index][j];

      float qa_diff = q_current - a_current;
      float qb_diff = q_current - b_current;
      qa_estimate = qa_estimate + qa_diff * qa_diff;
      qb_estimate = qb_estimate + qb_diff * qb_diff;
    }
    float margin = (qb_estimate - qa_estimate) * label;
    if (margin == 0)
    {
      error = error + weight / (float) 2.0;
    }
    else if (margin < 0)
    {
      error = error + weight;
    }
  }

  validation_error = error;
  return error;
}


vint8 V_SparseMap::PrintAll()
{
  V_BoostMap2::PrintAll();
  vint8 i;
  for (i = 0; i < (vint8) reference_sets.size(); i++)
  {
    vPrint("references[%li]:\n", (long) i);
    reference_sets[(vector_size) i].Print("sets");
  }

  embeddings_matrix.Print("embeddings:");
  return 1;
}


  
/////////////////////////////////////////////////////////////////////
//                                                                 //
//              End of implementation of V_SparseMap               //
//                                                                 //
/////////////////////////////////////////////////////////////////////



/////////////////////////////////////////////////////////////////////
//                                                                 //
//           Start of implementation of vPlaneSparseMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vPlaneSparseMap::vPlaneSparseMap()
{
}


vPlaneSparseMap::vPlaneSparseMap(vint8 in_number_of_points, 
                                   vint8 in_validation_number)
{
  object_number = in_number_of_points;
  validation_number = in_validation_number;
  training_number = 1;
  points_matrix = PickPoints();
  points = points_matrix.Matrix2();
  distances_matrix = PointDistances();

  candidate_number = object_number / 3;
  candidate_indices_matrix = vMatrix<vint8>::Range(0, candidate_number-1);
  validation_indices_matrix = vMatrix<vint8>::Range(2*candidate_number, 
                                                    object_number-1);

  Initialize4(distances_matrix, candidate_indices_matrix, 
              validation_indices_matrix, (vint8) validation_number);
  vdelete2(class_name);
  class_name = vCopyString("vPlaneSparseMap");
}


vPlaneSparseMap::~vPlaneSparseMap()
{
}


vMatrix<float> vPlaneSparseMap::PickPoints()
{
  vMatrix<float> result_matrix = vMatrix<float>(object_number, 2);
  vArray2(float) result = result_matrix.Matrix2();
  const vint8 limit = 1000000000; // one billion
  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float lx = (float) function_random_vint8(0, limit);
    float ly = (float) function_random_vint8(0, limit);
    result[i][0] = lx / (float) limit;
    result[i][1] = ly / (float) limit;
  }

  return result_matrix;
}


vMatrix<float> vPlaneSparseMap::PointDistances()
{
  vMatrix<float> result_matrix(object_number, object_number);
  vArray2(float) result = result_matrix.Matrix2();
  
  vint8 i, j;
  float max_distance = 0;
  for (i = 0; i < object_number; i++)
  {
    float i_x = points[i][0];
    float i_y = points[i][1];

    for (j = 0; j <= i; j++)
    {
      float j_x = points[j][0];
      float j_y = points[j][1];
      float dx = j_x - i_x;
      float dy = j_y - i_y;
      float distance = sqrt(dx * dx + dy * dy);
      result[i][j] = distance;
      result[j][i] = distance;
      if (distance > max_distance)
      {
        max_distance = distance;
      }
    }
  }

  if (max_distance == 0)
  {
    exit_error("max_distance = 0\n");
  }

  // Normalize distances
  for (i = 0; i < object_number; i++)
  {
    for (j = 0; j < object_number; j++)
    {
      result[i][j] = result[i][j] / max_distance;
    }
  }

  return result_matrix;
}


color_image * vPlaneSparseMap::PointImage(vint8 rows, vint8 cols)
{
  if (object_number == 0) return 0;
  color_image * result = new color_image(rows, cols);
  function_enter_value(result, (uchar) 0);

  float row_scale = (float) (rows - 1);
  float col_scale = (float) (cols - 1);
  vector<uchar> colors(3);
  colors[0] = 255;
  colors[1] = 100;
  colors[2] = 100;

  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float x = points[i][0];
    float y = points[i][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawCircle(result, row, col, 2, colors);
  }

  return result;
}


vint8 vPlaneSparseMap::PrintAll()
{
  if (object_number == 0) return 0;
  points_matrix.Print("points matrix");
  V_SparseMap::PrintAll();
  return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//             End of implementation of vPlaneSparseMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vChamferSparseMap::vChamferSparseMap(vMatrix<float> in_distances,
                                       vint8 in_candidate_range, 
                                       vint8 in_validation_range,
                                       vint8 in_validation_number,
                                       vMatrix<vint8> * indices_matrix) :
V_SparseMap(in_distances, in_candidate_range, in_validation_range,
            in_validation_number)
{
  FindReferenceImages(indices_matrix);
  vdelete2(class_name);
  class_name = vCopyString("vChamferSparseMap");
}


// We pass in a matrix mapping indices to (frame, rotation).
vint8 vChamferSparseMap::FindReferenceImages(vMatrix<vint8> * indices_matrix)
{
  reference_images.erase(reference_images.begin(), reference_images.end());
  
  vint8 number = reference_sets.size();
  vArray2(vint8) indices = indices_matrix->Matrix2();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vMatrix<vint8> i_sets_matrix = reference_sets[(vector_size) i];
    vint8 rows = (vint8) i_sets_matrix.Rows();
    vint8 cols = (vint8) i_sets_matrix.Cols();
    vArray2(vint8) i_sets = i_sets_matrix.Matrix2();
    v3dMatrix<vint8> i_images_matrix(rows, cols, 2);
    vArray3(vint8) i_images = i_images_matrix.Matrix3();
    vint8 row, col;
    for (row = 0; row < rows; row++)
    {
      for (col = 0; col < cols; col++)
      {
        vint8 index = i_sets[row][col];
        vint8 frame = indices[index][0];
        vint8 rotation = indices[index][1];
        i_images[0][row][col] = frame;
        i_images[1][row][col] = rotation;
      }
    }
    reference_images.push_back(i_images_matrix);
  }

  return 1;
}


vint8 vChamferSparseMap::ReferenceImages(vector<v3dMatrix<vint8> > * output)
{
  vint8 number = reference_images.size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    output->push_back(reference_images[(vector_size) i]);
  }

  return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//              Start of implementation of V_FastMap               //
//                                                                 //
/////////////////////////////////////////////////////////////////////


V_FastMap::V_FastMap()
{
  Zero();
}


V_FastMap::V_FastMap(vMatrix<float> in_distances, vint8 in_validation)
{
  Zero();
  Initialize2(in_distances, in_validation);
}


V_FastMap::V_FastMap(vMatrix<float> in_distances,
          vint8 in_candidate_range, vint8 in_validation_range,
          vint8 in_validation_number)
{
  object_number = in_distances.Rows();
  vint8 candidate_start = 0;
  vint8 candidate_end = in_candidate_range - 1;
  vint8 validation_start = (vint8) (object_number - in_validation_range);
  vint8 validation_end = (vint8) object_number - 1;

  vMatrix<vint8> temp_candidate_indices = vMatrix<vint8>::Range(candidate_start, candidate_end);
  vMatrix<vint8> temp_validation_indices = vMatrix<vint8>::Range(validation_start, validation_end);
  Initialize4(in_distances, temp_candidate_indices, temp_validation_indices,
              in_validation_number);
}


V_FastMap::V_FastMap(vMatrix<float> in_distances, 
          vMatrix<vint8> in_candidate_indices,
          vMatrix<vint8> in_validation_indices,
          vint8 in_validation_number)
{
  Initialize4(in_distances, in_candidate_indices, in_validation_indices, 
              in_validation_number);
}


V_FastMap::V_FastMap(vMatrix<float> in_distances, 
                     vMatrix<vint8> in_candidate_indices,
                     vMatrix<vint8> in_validation_triples)
{
  Initialize3(in_distances, in_candidate_indices, in_validation_triples);
}

  
V_FastMap::~V_FastMap()
{
}


vint8 V_FastMap::Zero()
{
  V_BoostMap2::Zero();
  return 1;
}


vint8 V_FastMap::Initialize2(vMatrix<float> in_distances, vint8 in_validation)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vint8 third = (vint8) object_number / 3;
  vMatrix<vint8> temp_candidate_range = vMatrix<vint8>::Range(0, third - 1);
  vMatrix<vint8> temp_validation_range = vMatrix<vint8>::Range(2*third, object_number-1);
  return Initialize4(in_distances, temp_candidate_range, 
                     temp_validation_range, in_validation);
}


vint8 V_FastMap::Initialize3(vMatrix<float> in_distances, 
                            vMatrix<vint8> in_candidate_indices,
                            vMatrix<vint8> in_validation_triples)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vMatrix<vint8> temp_training_triples(1, 3);
  temp_training_triples.WriteN(3, 0, 1, 2);
  vint8 candidates_to_pick = 1;
  vint8 training = 1;
  vint8 success = Initialize5(in_distances, in_candidate_indices, temp_training_triples,
                             in_validation_triples, candidates_to_pick);
  if (success <= 0) 
  {
    return success;
  }

  vdelete2(class_name);
  class_name = vCopyString("V_FastMap");

  current_distances_matrix = vMatrix<float>(&distances_matrix);
  current_distances = current_distances_matrix.Matrix2();
  return 1;
}

  
vint8 V_FastMap::Initialize4(vMatrix<float> in_distances, 
                 vMatrix<vint8> in_candidate_indices,
                 vMatrix<vint8> in_validation_indices,
                 vint8 in_validation)
{
  object_number = in_distances.Rows();
  if (object_number < 3) 
  {
    vPrint("Cannot initialize with fewer than 3 objects\n");
    return 0;
  }

  vMatrix<vint8> temp_training_range = vMatrix<vint8>::Range(0, object_number-1);
  vint8 candidates_to_pick = 1;
  vint8 training = 1;
  vint8 success = Initialize7(in_distances, in_candidate_indices, temp_training_range,
                in_validation_indices, candidates_to_pick, training, in_validation);
  if (success <= 0) 
  {
    return success;
  }

  vdelete2(class_name);
  class_name = vCopyString("V_FastMap");

  current_distances_matrix = vMatrix<float>(&distances_matrix);
  current_distances = current_distances_matrix.Matrix2();
  return 1;
}


float V_FastMap::NextStep()
{
  vint8 pivot1, pivot2;
  PickPivots(&pivot1, &pivot2, current_distances_matrix);
  first_pivots.push_back(pivot1);
  second_pivots.push_back(pivot2);
  ComputeValidationError();
  validation_errors.push_back(validation_error);
  vint8 counter = UpdateCurrentDistances(pivot1, pivot2, 
                                        current_distances_matrix);
  float f_object_number = (float) object_number;
  float fraction = ((float) counter) / (f_object_number * f_object_number); 
  vPrint("negative square distance counter = %li, fraction = %f\n", 
          (long) counter, fraction);
  return validation_error;
}


float V_FastMap::ComputeValidationError()
{
  vint8 pivot_number = first_pivots.size();
  
  // Get the last pivots
  vint8 pivot1 = first_pivots[(vector_size) (pivot_number-1)];
  vint8 pivot2 = second_pivots[(vector_size) (pivot_number-1)];

  validation_error = 0;
  float validation_margin = 0;
  vint8 i;
  float weight = (float) 1.0 / (float) validation_number;

  for (i = 0; i < validation_number; i++)
  {
    vint8 q_index = validation_triples[i][0];
    vint8 a_index = validation_triples[i][1];
    vint8 b_index = validation_triples[i][2];

    float label;
    float qa = distances[q_index][a_index];
    float qb = distances[q_index][b_index];
    if (qa == qb)
    {
      vPrint("%li of %li (%li %li %li)\n",
              (long) i, (long) validation_number, (long) q_index, (long) a_index, (long) b_index);
      vPrint("qa = %f, qb = %f\n", qa, qb);
      vPrint("Error: training triplet with equal distances\n");
    }
    if (qa < qb) label = 1.0; 
    else label = -1.0;

    float q_projection = LineProjection(q_index, pivot1, pivot2, 
                                        current_distances_matrix);
    float a_projection = LineProjection(a_index, pivot1, pivot2, 
                                        current_distances_matrix);
    float b_projection = LineProjection(b_index, pivot1, pivot2, 
                                        current_distances_matrix);

    float qa_diff = q_projection - a_projection;
    float qa_estimate = qa_diff * qa_diff;
    float qb_diff = q_projection - b_projection;
    float qb_estimate = qb_diff * qb_diff;

    float current_estimate = (qb_estimate - qa_estimate) * label;

    // Add the contribution of the last chosen pivot points to 
    // the contributions of all previous pivot points.
    float total_estimate = validation_margins[i] + current_estimate;
    validation_margins[i] = total_estimate;

    if (total_estimate == 0) 
    {
      validation_error = validation_error + weight / (float) 2.0;
    }
    else if (total_estimate < 0)
    {
      validation_error = validation_error + weight;
    }
    validation_margin = validation_margin + weight * total_estimate;
  }

  return -1;
}


vint8 V_FastMap::PrintAll()
{
  V_BoostMap::PrintAll();
  current_distances_matrix.Print("current_distances");
  return 1;
}


vint8 V_FastMap::StepsDone()
{
  return first_pivots.size();
}


vint8 V_FastMap::PickPivots(vint8 * pivot1p, vint8 * pivot2p, 
                           vMatrix<float> point_distances_matrix)
{
  // Pick a random candidate object as pivot2.
  vint8 number = (vint8) candidate_indices_matrix.Size();
  if (number == 0) return 0;
  vint8 pivot2_index = (vint8) function_random_vint8(0, number-1);
  vint8 pivot2 = candidate_indices[pivot2_index];
  vArray2(float) point_distances = point_distances_matrix.Matrix2();

  // Set as pivot1 the candidate object that is the farthest away
  // from pivot2.
  vint8 i;
  float max_distance = (float) -1000000000; // one billion
  vint8 max_index = -1;
  for (i = 0; i < number; i++)
  {
    vint8 object = candidate_indices[i];
    float distance = point_distances[pivot2][object];
    if (distance > max_distance)
    {
      max_distance = distance;
      max_index = object;
    }
  }

  if (max_index == -1)
  {
    exit_error("Error: max_index = -1 in PickPivots\n");
  }
  vint8 pivot1 = max_index;
  
  // Set as pivot2 the candidate object that is the farthest away
  // from pivot1.
  max_distance = (float) -1000000000; // one billion
  max_index = -1;
  for (i = 0; i < number; i++)
  {
    vint8 object = candidate_indices[i];
    float distance = point_distances[pivot1][object];
    if (distance > max_distance)
    {
      max_distance = distance;
      max_index = object;
    }
  }

  if (max_index == -1)
  {
    exit_error("Error: max_index = -1 in PickPivots\n");
  }
  pivot2 = max_index;
  vPrint("pivot1 = %li, pivot2 = %li, max_distance = %f\n", 
          (long) pivot1, (long) pivot2, max_distance);
  *pivot1p = pivot1;
  *pivot2p = pivot2;

  return 1;
}


vint8 V_FastMap::UpdateCurrentDistances(vint8 pivot1, vint8 pivot2, 
                                       vMatrix<float> point_distances_matrix)
{
  vint8 rows = (vint8) point_distances_matrix.Rows();
  vint8 cols = (vint8) point_distances_matrix.Cols();
  if (rows != cols)
  {
    exit_error("Error: non-square matrix in UpdateCurrentDistances\n");
  }

  vArray2(float) point_distances = point_distances_matrix.Matrix2();

  vint8 object_number = point_distances_matrix.Rows();
  vMatrix<float> line_projections_matrix(1, object_number);
  vArray(float) line_projections = line_projections_matrix.Matrix();

  // We need to get the projections before we alter the matrix.
  vint8 i, j;
  float ab = point_distances[pivot1][pivot2];
  for (i = 0; i < object_number; i++)
  {
    float ai = point_distances[pivot1][i];
    float bi = point_distances[pivot2][i];

    line_projections[i] = LineProjection3(ai, bi, ab);
  }

  vint8 counter = 0;
  for (i = 0; i < object_number; i++)
  {
    point_distances[i][i] = 0;
    for (j = 0; j < i; j++)
    {
      float xi = line_projections[i];
      float xj = line_projections[j];
      float ij_diff = xi - xj;

      float current_d = point_distances[i][j];
      float new_d_square = current_d * current_d - ij_diff * ij_diff;
      float new_d;
      if (new_d_square >= 0) 
      {
        new_d = sqrt(new_d_square);
      }
      else
      {
        new_d = -sqrt(-new_d_square);
        counter++;
      }
      point_distances[i][j] = new_d;
      point_distances[j][i] = new_d;
    }
  }
  return counter;
}


float V_FastMap::LineProjection(vint8 index, vint8 pivot1, vint8 pivot2,
                                vMatrix<float> point_distances_matrix)
{
  vArray2(float) point_distances = point_distances_matrix.Matrix2();

  float dai = point_distances[pivot1][index];
  float dbi = point_distances[pivot2][index];
  float dab = point_distances[pivot1][pivot2];
  float result = (dai * dai + dab * dab - dbi * dbi) / ((float) 2.0 * dab);
  return result;
}


// i is the object, a and b are the two pivot points.
float V_FastMap::LineProjection3(float ai, float bi, float ab)
{
  float result = (ai * ai + ab * ab - bi * bi) / ((float) 2.0 * ab);
  return result;
}


vMatrix<float> V_FastMap::Embedding(vMatrix<float> distances_to_pivots_matrix,
                                     vMatrix<float> intrapivot_distances_matrix)
{
  vint8 dimensions = distances_to_pivots_matrix.Rows();
  if (dimensions <= 0)
  {
    return vMatrix<float>();
  }
  if (distances_to_pivots_matrix.Cols() != 2)
  {
    return vMatrix<float>();
  }
  if (intrapivot_distances_matrix.Rows() != 2*dimensions)
  {
    return vMatrix<float>();
  }

  if (intrapivot_distances_matrix.Cols() != 2*dimensions)
  {
    return vMatrix<float>();
  }

  vMatrix<float> result_matrix(1, dimensions);
  vArray(float) result = result_matrix.Matrix();
  vArray2(float) distances_to_pivots = distances_to_pivots_matrix.Matrix2();
  vArray2(float) intrapivot_distances = intrapivot_distances_matrix.Matrix2();

  // Will hold the projections of pivots onto the current line
  vMatrix<float> pivot_projections_matrix(dimensions, 2);
  vArray2(float) pivot_projections = pivot_projections_matrix.Matrix2();

  // For each dimension, get projection onto the line, and update the distances.
  vint8 i, j;
  for (i = 0; i < dimensions; i++)
  {
    // Get projection of object onto line.
    float ai = distances_to_pivots[i][0];
    float bi = distances_to_pivots[i][1];
    float ab = intrapivot_distances[2*i][2*i+1];
    float i_coord = LineProjection3(ai, bi, ab);
    result[i] = i_coord;

    // Get projections of all pivot points onto the line
    for (j = 0; j < dimensions; j++)
    {
      float aj = intrapivot_distances[2*i][2*j];
      float bj = intrapivot_distances[2*i+1][2*j];
      pivot_projections[j][0] = LineProjection3(aj, bj, ab);

      aj = intrapivot_distances[2*i][2*j+1];
      bj = intrapivot_distances[2*i+1][2*j+1];
      pivot_projections[j][1] = LineProjection3(aj, bj, ab);
    }

    // update distances to pivots
    for (j = 0; j < dimensions; j++)
    {
      float pivot_coord = pivot_projections[j][0];
      float diff = i_coord - pivot_coord;
      float current_d = distances_to_pivots[j][0];

      float new_d_square = current_d * current_d - diff * diff;
      float new_d;
      if (new_d_square >= 0) 
      {
        new_d = sqrt(new_d_square);
      }
      else
      {
        new_d = -sqrt(-new_d_square);
      }
      distances_to_pivots[j][0] = new_d;

      pivot_coord = pivot_projections[j][1];
      diff = i_coord - pivot_coord;
      current_d = distances_to_pivots[j][1];

      new_d_square = current_d * current_d - diff * diff;
      new_d;
      if (new_d_square >= 0) 
      {
        new_d = sqrt(new_d_square);
      }
      else
      {
        new_d = -sqrt(-new_d_square);
      }
      distances_to_pivots[j][1] = new_d;
    }

    // update intrapivot distances
    vint8 pivot1_index = 2*i;
    vint8 pivot2_index = 2*i+1;
    UpdateCurrentDistances(pivot1_index, pivot2_index, 
                           intrapivot_distances_matrix);
  }
  
  return result_matrix;
}


// This is an optimized version of Embedding, that hopefully gives the same
// results as Embedding. Here, intrapivot_distances_matrix(i, j) is the 
// distance between the corresponding points right before we perform projection
// to a hyperplane for the (i/2)th time.
vMatrix<float> V_FastMap::EmbeddingB(vMatrix<float> distances_to_pivots_matrix,
                                      vMatrix<float> intrapivot_distances_matrix)
{
  vint8 dimensions = distances_to_pivots_matrix.Size() / 2;
  if ((dimensions * 2 != distances_to_pivots_matrix.Size()) ||
      (dimensions <= 0))
  {
    return vMatrix<float>();
  }
  if (intrapivot_distances_matrix.Rows() != 2*dimensions)
  {
    return vMatrix<float>();
  }

  if (intrapivot_distances_matrix.Cols() != 2*dimensions)
  {
    return vMatrix<float>();
  }

  vMatrix<float> result_matrix(1, dimensions);
  vArray(float) result = result_matrix.Matrix();
  vArray(float) distances_to_pivots = distances_to_pivots_matrix.Matrix();
  vArray2(float) intrapivot_distances = intrapivot_distances_matrix.Matrix2();

  // Will hold the projections of pivots onto the current line
  vMatrix<float> pivot_projections_matrix(dimensions, 2);
  vArray2(float) pivot_projections = pivot_projections_matrix.Matrix2();

  // For each dimension, get projection onto the line, and update the distances.
  vint8 i, j;
  for (i = 0; i < dimensions; i++)
  {
    // Get projection of object onto line.
    float ai = distances_to_pivots[2*i];
    float bi = distances_to_pivots[2*i+1];
    float ab = intrapivot_distances[2*i][2*i+1];
    float i_coord = LineProjection3(ai, bi, ab);
    result[i] = i_coord;

    // Get projections of all pivot points onto the line
    for (j = 0; j < dimensions; j++)
    {
      float aj = intrapivot_distances[2*i][2*j];
      float bj = intrapivot_distances[2*i+1][2*j];
      pivot_projections[j][0] = LineProjection3(aj, bj, ab);

      aj = intrapivot_distances[2*i][2*j+1];
      bj = intrapivot_distances[2*i+1][2*j+1];
      pivot_projections[j][1] = LineProjection3(aj, bj, ab);
    }

    // update distances to pivots
    for (j = 0; j < dimensions; j++)
    {
      vint8 k;
      for (k = 0; k <= 1; k++)
      {
        float pivot_coord = pivot_projections[j][k];
        float diff = i_coord - pivot_coord;
        float current_d = distances_to_pivots[2*j+k];

        float new_d_square = current_d * current_d - diff * diff;
        float new_d;
        if (new_d_square >= 0) 
        {
          new_d = sqrt(new_d_square);
        }
        else
        {
          new_d = -sqrt(-new_d_square);
        }
        distances_to_pivots[2*j+k] = new_d;
      }
    }
  }
  
  return result_matrix;
}


// OptimizedPivotDistances takes as a result a matrix of original distances
// between pivots (like the one used in the function Embedding) and returns
// a matrix of distances like the one used in EmbeddingB.
vMatrix<float> V_FastMap::OptimizedPivotDistances(vMatrix<float> original_matrix)
{
  vint8 rows = (vint8) original_matrix.Rows();
  vint8 cols = (vint8) original_matrix.Cols();
  if (rows != cols)
  {
    exit_error("Error: non-square matrix in OptimizedPivotDistances\n");
  }

  vMatrix<float> result_matrix(rows, cols);
  vMatrix<float> current_distances(&original_matrix);
  vArray2(float) original = original_matrix.Matrix2();
  vArray2(float) result = result_matrix.Matrix2();
  vArray2(float) current = current_distances.Matrix2();

  vint8 dimensions = rows / 2;
  vint8 i;
  vPrint("\n");
  for (i = 0; i < dimensions; i++)
  {
    // The ends of the i-th row and col of current_distances 
    // get recorded into result;
    vint8 j;
    vint8 two_i = 2*i;
    for (j = 2*i; j < rows; j++)
    {
      result[two_i][j] = current[two_i][j];
      result[j][two_i] = current[j][two_i];
      result[two_i+1][j] = current[two_i+1][j];
      result[j][two_i+1] = current[j][two_i+1];
      float first = current[two_i][j];
      float second = current[j][two_i];
      float third = current[two_i+1][j];
      float fourth = current[j][two_i+1];
      if (vAbs(current[two_i][j] - current[j][two_i]) > .000001)
      {
        //function_warning("Error: non-symmetric current distances: (%li, %li), %f, %f\n",
        //                two_i, j, first, second);
      }
      if (vAbs(current[two_i+1][j] - current[j][two_i+1]) > .000001)
      {
        //function_warning("Error: non-symmetric current distances: (%li, %li), %f, %f\n",
        //                two_i, j, third, fourth);
      }
    }
    
    // Update current distances.
    vint8 pivot1 = 2*i;
    vint8 pivot2 = pivot1 + 1;
    UpdateCurrentDistances(pivot1, pivot2, current_distances);
    vPrint("processed dimension %li of %li\r", (long) (i+1), (long) dimensions);
  }
  vPrint("\n");

  return result_matrix;
}

  
// OriginalPivotDistances returns a matrix with the initial (before any 
// projection) distances between the pivots. It returns the kind of matrix
// that is used as argument in OptimizedPivotDistances. The argument is
// a dimensions x 2 matrix like the one returned from PivotsMatrix.
vMatrix<float> V_FastMap::OriginalPivotDistances(vMatrix<vint8> pivots_matrix)
{
  // compute pivot distances;
  vint8 dimensions = (vint8) pivots_matrix.Rows();
  vMatrix<float> pivot_distances_matrix(2*dimensions, 2*dimensions);
  vArray2(vint8) pivots = pivots_matrix.Matrix2();
  vArray2(float) pivot_distances = pivot_distances_matrix.Matrix2();

  vint8 i, j;
  for (i = 0; i < dimensions; i++)
  {
    for (j = 0; j < dimensions; j++)
    {
      vint8 index1 = pivots[i][0];
      vint8 index2 = pivots[j][0];
      float distance = distances[index1][index2];
      pivot_distances[2*i][2*j] = distance;

      index1 = pivots[i][0];
      index2 = pivots[j][1];
      distance = distances[index1][index2];
      pivot_distances[2*i][2*j+1] = distance;

      index1 = pivots[i][1];
      index2 = pivots[j][0];
      distance = distances[index1][index2];
      pivot_distances[2*i+1][2*j] = distance;

      index1 = pivots[i][1];
      index2 = pivots[j][1];
      distance = distances[index1][index2];
      pivot_distances[2*i+1][2*j+1] = distance;
    }
  }

  return pivot_distances_matrix;
}


// Returns a number_of_pivots x 2 matrix, each row is the indices of
// a pivot pair.
vMatrix<vint8> V_FastMap::PivotsMatrix()
{
  vint8 dimensions = first_pivots.size();
  vMatrix<vint8> result_matrix(dimensions, 2);
  vArray2(vint8) result = result_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    result[i][0] = first_pivots[(vector_size) i];
    result[i][1] = second_pivots[(vector_size) i];
  }

  return result_matrix;
}


vMatrix<float> V_FastMap::DistancesToPivots(vint8 index)
{
  vint8 dimensions = first_pivots.size();
  if(dimensions == 0) return vMatrix<float>();
  vMatrix<float> result_matrix(dimensions, 2);
  vArray2(float) result = result_matrix.Matrix2();

  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    vint8 pivot1 = first_pivots[(vector_size) i];
    result[i][0] = distances[index][pivot1];
    vint8 pivot2 = second_pivots[(vector_size) i];
    result[i][1] = distances[index][pivot2];
  }

  return result_matrix;
}


vMatrix<float> V_FastMap::Embedding1(vint8 index, 
                                      vMatrix<float> intrapivot_distances)
{
  vMatrix<float> distances_to_pivots = DistancesToPivots(index);
  vMatrix<float> result = EmbeddingB(distances_to_pivots, 
                                      intrapivot_distances);
  return result;
}


// This function returns a matrix whose size is equal to the number
// of FastMap dimensions (i.e. number of pivot pairs selected so far).
// It returns, for every dimension k, the number of distances to unique
// objects that we need to compute in order to compute the k-dimensional 
// FastMap embedding of a new object. All it has to do is see how many
// unique objects appear in the first k pivot pairs.
vMatrix<vint8> V_FastMap::RequiredDistances()
{
  vint8 dimensions = first_pivots.size();
  if (dimensions == 0) return vMatrix<vint8>();
  vint8 number_of_pivots = 2 * dimensions;
  vMatrix<vint8> result_matrix(dimensions, 2);
  vMatrix<vint8> scratch_matrix(1, object_number);
  function_enter_value(&scratch_matrix, (vint8) 0);

  vint8 i;
  vint8 counter = 0;
  for (i = 0; i < dimensions; i++)
  {
    vint8 pivot;
    pivot = first_pivots[(vector_size) i];
    if (scratch_matrix(pivot) == 0)
    {
      scratch_matrix(pivot) = 1;
      counter++;
    }
    pivot = second_pivots[(vector_size) i];
    if (scratch_matrix(pivot) == 0)
    {
      scratch_matrix(pivot) = 1;
      counter++;
    }
    result_matrix(i, 0) = i+1;
    result_matrix(i, 1) = counter;
  }

  return result_matrix;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//               End of implementation of V_FastMap                //
//                                                                 //
/////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////
//                                                                 //
//           Start of implementation of vPlaneFastMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vPlaneFastMap::vPlaneFastMap()
{
}


vPlaneFastMap::vPlaneFastMap(vint8 in_number_of_points, 
                               vint8 in_validation_number)
{
  object_number = in_number_of_points;
  validation_number = in_validation_number;
  points_matrix = PickPoints();
  points = points_matrix.Matrix2();
  distances_matrix = PointDistances();

  Initialize2(distances_matrix, (vint8) validation_number);
  vdelete2(class_name);
  class_name = vCopyString("vPlaneFastMap");
}


vPlaneFastMap::~vPlaneFastMap()
{
}


vMatrix<float> vPlaneFastMap::PickPoints()
{
  vMatrix<float> result_matrix = vMatrix<float>(object_number, 2);
  vArray2(float) result = result_matrix.Matrix2();
  const vint8 limit = 1000000000; // one billion
  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float lx = (float) function_random_vint8(0, limit);
    float ly = (float) function_random_vint8(0, limit);
    result[i][0] = lx / (float) limit;
    result[i][1] = ly / (float) limit;
  }

  return result_matrix;
}


vMatrix<float> vPlaneFastMap::PointDistances()
{
  vMatrix<float> result_matrix(object_number, object_number);
  vArray2(float) result = result_matrix.Matrix2();
  
  vint8 i, j;
  float max_distance = 0;
  for (i = 0; i < object_number; i++)
  {
    float i_x = points[i][0];
    float i_y = points[i][1];

    for (j = 0; j <= i; j++)
    {
      float j_x = points[j][0];
      float j_y = points[j][1];
      float dx = j_x - i_x;
      float dy = j_y - i_y;
      float distance = sqrt(dx * dx + dy * dy);
      result[i][j] = distance;
      result[j][i] = distance;
      if (distance > max_distance)
      {
        max_distance = distance;
      }
    }
  }

  if (max_distance == 0)
  {
    exit_error("max_distance = 0\n");
  }

  // Normalize distances
  for (i = 0; i < object_number; i++)
  {
    for (j = 0; j < object_number; j++)
    {
      result[i][j] = result[i][j] / max_distance;
    }
  }

  return result_matrix;
}


color_image * vPlaneFastMap::PointImage(vint8 rows, vint8 cols)
{
  if (object_number == 0) return 0;
  color_image * result = new color_image(rows, cols);
  function_enter_value(result, (uchar) 0);

  float row_scale = (float) (rows - 1);
  float col_scale = (float) (cols - 1);
  vector<uchar> colors(3);
  colors[0] = 255;
  colors[1] = 100;
  colors[2] = 100;

  vint8 i;
  for (i = 0; i < object_number; i++)
  {
    float x = points[i][0];
    float y = points[i][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawCircle(result, row, col, 2, colors);
  }

  vint8 candidates = (vint8) candidate_indices_matrix.Size();
  colors[0] = 100;
  colors[1] = 255;
  colors[2] = 100;
  for (i = 0; i < candidates; i++)
  {
    vint8 index = candidate_indices[i];
    float x = points[index][0];
    float y = points[index][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawCircle(result, row, col, 2, colors);
  }

  return result;
}


color_image * vPlaneFastMap::BasisImage(vint8 rows, vint8 cols)
{
  if (object_number == 0) return 0;
  color_image * result = PointImage(rows, cols);

  float row_scale = (float) (rows - 1);
  float col_scale = (float) (cols - 1);
  vector<uchar> colors(3);
  colors[0] = 255;
  colors[1] = 255;
  colors[2] = 0;

  vint8 i;
  for (i = 0; i < (vint8) first_pivots.size(); i++)
  {
    vint8 index = first_pivots[(vector_size) i];
    float x = points[index][0];
    float y = points[index][1];
    vint8 row = (vint8) round_number(y * row_scale);
    vint8 col = (vint8) round_number(x * col_scale);
    vDrawCircle(result, row, col, 2, colors);

    vint8 index2 = second_pivots[(vector_size) i];
    float x2 = points[index2][0];
    float y2 = points[index2][1];
    vint8 row2 = (vint8) round_number(y2 * row_scale);
    vint8 col2 = (vint8) round_number(x2 * col_scale);
    colors[2] = (uchar) i;
    vDrawCircle(result, row2, col2, 2, colors);
    vDrawLine(result, row, col, row2, col2, &colors);
  }

  return result;
}


vint8 vPlaneFastMap::PrintAll()
{
  if (object_number == 0) return 0;
  points_matrix.Print("points matrix");
  V_FastMap::PrintAll();
  return 1;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//             End of implementation of vPlaneFastMap            //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vChamferFastMap::vChamferFastMap(vMatrix<float> in_distances,
                                   vint8 in_candidate_range, 
                                   vint8 in_validation_range,
                                   vint8 in_validation_number,
                                   vMatrix<vint8> * indices_matrix) :
V_FastMap(in_distances, in_candidate_range, in_validation_range,
          in_validation_number)
{
  FindPivotImages(indices_matrix);
  vdelete2(class_name);
  class_name = vCopyString("vChamferFastMap");
}


vChamferFastMap::~vChamferFastMap()
{
}


// We pass in a matrix mapping indices to (frame, rotation).
vint8 vChamferFastMap::FindPivotImages(vMatrix<vint8> * indices_matrix)
{
  vint8 number = first_pivots.size();
  pivot_images = vMatrix<vint8>(number, 4);
  vArray2(vint8) pivot_info = pivot_images.Matrix2();

  vArray2(vint8) indices = indices_matrix->Matrix2();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index = first_pivots[(vector_size) i];
    vint8 frame = indices[index][0];
    vint8 rotation = indices[index][1];
    pivot_info[i][0] = frame;
    pivot_info[i][1] = rotation;

    index = second_pivots[(vector_size) i];
    frame = indices[index][0];
    rotation = indices[index][1];
    pivot_info[i][2] = frame;
    pivot_info[i][3] = rotation;
  }

  return 1;
}



vMatrix<vint8> vChamferFastMap::PivotImages()
{
  return vMatrix<vint8>(&pivot_images);
}


