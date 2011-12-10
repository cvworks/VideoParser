#ifndef VASSILIS_BOOST_MAP_H
#define VASSILIS_BOOST_MAP_H


#include "basics/matrix.h"
#include "basics/algebra.h"


class V_BoostMap
{
protected:
  // class_name is used to identify subtypes of boost_map.
  char * class_name;

  vint8 object_number;
  vint8 candidate_number;
  vint8 training_number;
  vint8 validation_number;

  // In general, it's probably a good idea that 
  // candidate_range + training_range + validation_range = object_number.
  // candidate_range specifies that reference objects are picked
  // from indices 0 to candidate_range-1;
  vint8 candidate_range;
  // training_range specifies that training triples are picked from
  // indices candidate_range to candidate_range + training_range - 1
  vint8 training_range;
  
  // validation_range specifies that training_triples are picked
  // from object_number-validation_range to object_number-1.
  vint8 validation_range;

  // If pick_candidates_flag is set to 1, then at each step 
  // we pick a set of random candidates from the values specified
  // by candidate_range. The number of candidates we pick is 
  // specified by number_of_picked_candidates, which can be different
  // than candidate_number (which stores the number of candidates
  // currently chosen).
  vint8 pick_candidates_flag;
  vint8 number_of_picked_candidates;

  vMatrix<float> distances_matrix;
  vMatrix<vint8> training_triples_matrix;
  vMatrix<vint8> candidates_matrix;
  vMatrix<float> training_distances_matrix;
  vMatrix<float> training_factors_matrix;
  vMatrix<vint8> validation_triples_matrix;
  vMatrix<float> validation_distances_matrix;

  vArray2(float) distances;
  vArray2(vint8) training_triples;
  vArray(vint8) candidates;
  vArray2(float) training_distances;
  vArray(float) training_factors;
  vArray2(vint8) validation_triples;
  vArray2(float) validation_distances;


  vector<vint8> reference_indices;
  vector<float> reference_weights;
  vector<float> reference_zs;
  vector<float> training_errors;
  vector<float> validation_errors;

  vector<vint8> cleaned_indices;
  vector<float> cleaned_factors;

  // training error and margin of classifier that we picked last
  float last_error; 
  float last_et;
  float last_correlation;
  float last_scaled_et;
  float last_scaled_correlation;

  // training error and margin of the strong classifier learned so far.
  float training_error;
  float training_margin;
  float validation_error;

  // some variables that collect information useful for future
  // optimizations:
  // counts the total number of iterations spent on binary search
  // for minimizing z.
  vas_int16 iterations;
  // The smallest x passed to the pow(e, x) function
  float min_exp;
  // The largest x passed to the pow(e, x) function
  float max_exp;

  // 0 if negative weights (alphas) should not be allowed, non-zero
  // otherwise.
  vint8 allow_negative;

  // 0 if removing existing classifiers, or reducing their weight, should not 
  // be tried as part of the training. 1 if those things should be tried.
  vint8 allow_removals;

protected:
  vint8 TrainingDistances();
  vint8 InitialWeights();
  vint8 CleanUpClassifier();

public:
  V_BoostMap();
  V_BoostMap(vMatrix<float> in_distances, 
             vMatrix<vint8> in_training_triples,
             vMatrix<vint8> in_candidates);

  V_BoostMap(vMatrix<float> in_distances,
             vint8 in_candidate_range, vint8 in_training_range,
             vint8 in_number_of_picked_candidates, 
             vint8 in_training_number, vint8 in_validation_number);

  ~V_BoostMap();

  vint8 Zero();
  vint8 Initialize(vMatrix<float> in_distances, 
                  vMatrix<vint8> in_training_triples,
                  vMatrix<vint8> in_candidates);

  vint8 Initialize6(vMatrix<float> in_distances,
                   vint8 in_candidate_range, vint8 in_training_range,
                   vint8 in_number_of_picked_candidates, 
                   vint8 in_training_number, vint8 in_validation_number);

  const char * ClassName();

  virtual float NextStep();
  virtual float NextSteps(vint8 i);
  virtual vint8 StepsDone();
  vint8 Classifier(vector<vint8> * out_indices, vector<float> * out_weights);

  // Here we identify repetitions of classifiers and we combine them.
  vint8 CleanClassifier(vector<vint8> * out_indices, vector<float> * out_weights);

  vint8 SetCandidates(vMatrix<vint8> in_candidates)
  {
    candidates_matrix = in_candidates;
    candidates = candidates_matrix.Matrix();
    candidate_number = candidates_matrix.Rows();
    return 1;
  }
 
  vMatrix<vint8> AllCandidates(vint8 in_number);
  float ClassifierZ(vint8 index, float * alpha);
  vint8 ClassifierResults(vint8 index, vector<float> * results);
  vint8 MinimizeZ(vector<float> * results, float * z, float * a);
  vint8 MinimizeZ5(vector<float> * results, float * z, float * a,
                  float alpha_low, float alpha_hi);
  float Z(float a, vector<float> * results);
  float Z_Prime(float a, vector<float> * results);
  float UpdateWeights(vector<float> * results, float z, float alpha);

  // Computes errors and margins
  float ComputeLastError(float alpha, vector<float> * results);
  virtual float ComputeTrainingError();
  virtual float ComputeValidationError();
  float TrainingError();
  float TrainingMargin();
  float ValidationError();
  float LastError();
  float LastEt();
  float LastCorrelation();  
  float StepZ(vint8 step);
  float StepTrainingError(vint8 step);
  float StepValidationError(vint8 step);

  vMatrix<float> Embedding1(vint8 index);
  vMatrix<float> Embedding2(vint8 index, vector<vint8> * references);
  float L1_Distance(vMatrix<float> v1, vMatrix<float> v2);
  float L1_Distance3(vMatrix<float> v1, vMatrix<float> v2,
                     vector<float> * weights);

  // For debugging: weights should sum up to 1.
  float SumWeights();
  float SumClassifierWeights();
  virtual vint8 PrintSummary();
  virtual vint8 PrintClassifier();
  virtual vint8 PrintAll();

  // Pick random reference points, and compute error and margin for them.
  virtual vint8 PickRandom(vint8 number);

  // Choosing a training set is not allowed if any training rounds 
  // have been performed.
  virtual vint8 ChooseTrainingSet(vint8 in_training_number);
  virtual vint8 ChooseValidationSet(vint8 in_validation_number);
  virtual vint8 ChooseCandidates(vint8 in_candidate_number);
  vMatrix<vint8> ChooseTriples(vint8 number, vint8 start, vint8 end);

  vint8 SetPickCandidatesFlag(vint8 value);
  
  // This function is useful when we want to fully specify the classifier,
  // for example if we want to load an existing classifier.
  vint8 AddClassifier(vint8 index, float alpha);

  vint8 SetAllowNegative(vint8 in_value);
  vint8 SetAllowRemovals(vint8 in_value);
  vint8 TryRemovals(vint8 * z_min_indexp, float * z_minp, float * alphap);
  vint8 TryWeightChange(vint8 * z_min_indexp, float * z_minp, float * alphap);

  static vint8 CleanUp(vint8 object_number, vector<vint8> * reference_indices,
                      vector<float> * reference_weights,
                      vector<vint8> * cleaned_indices,
                      vector<float> * cleaned_factors);

  // We assume that reference_images(i, 3) is the index
  // into the distances matrix corresponding to the i-th 
  // reference image.
  static v3dMatrix<float> * CleanUp(vint8 object_number, 
                                      v3dMatrix<float> * reference_images);

  // For debugging:
  float Distance(vint8 i, vint8 j);
};


// This is hopefully an improved implementation of BoostMap. I wanted
// to keep the older BoostMap code (class V_BoostMap) around so that
// I can compare the two and make sure that there are no bugs in the
// new code.
class V_BoostMap2 : public V_BoostMap
{
protected:
  // The "indices" matrices specify the subset of objects that should
  // be used to generate candidates, training triples, and validation tripes.
  vMatrix<vint8> candidate_indices_matrix;
  vMatrix<vint8> training_indices_matrix;
  vMatrix<vint8> validation_indices_matrix;
  
  vMatrix<float> training_margins_matrix;
  vMatrix<float> validation_margins_matrix;

  vArray(vint8) candidate_indices;
  vArray(vint8) training_indices;
  vArray(vint8) validation_indices;

  vArray(float) training_margins;
  vArray(float) validation_margins;

public:
  V_BoostMap2();

  V_BoostMap2(vMatrix<float> in_distances,
              vint8 in_candidate_range, vint8 in_training_range,
              vint8 in_number_of_picked_candidates, 
              vint8 in_training_number, vint8 in_validation_number);

  V_BoostMap2(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_training_indices,
              vMatrix<vint8> in_validation_indices,
              vint8 in_candidate_number,
              vint8 in_training_number, 
              vint8 in_validation_number);

  V_BoostMap2(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_training_triples,
              vMatrix<vint8> in_validation_triples,
              vint8 in_candidate_number);


  ~V_BoostMap2();

  vint8 Initialize5(vMatrix<float> in_distances, 
                  vMatrix<vint8> in_candidate_indices,
                  vMatrix<vint8> in_training_triples,
                  vMatrix<vint8> in_validation_triples,
                  vint8 in_candidate_number);


  vint8 Initialize7(vMatrix<float> in_distances, 
                  vMatrix<vint8> in_candidate_indices,
                  vMatrix<vint8> in_training_indices,
                  vMatrix<vint8> in_validation_indices,
                  vint8 in_candidate_number,
                  vint8 in_training_number, 
                  vint8 in_validation_number);

  // Computes errors and margins
  virtual float ComputeTrainingError();
  virtual float ComputeValidationError();
  virtual vint8 PickRandom(vint8 number);

  virtual vint8 ChooseTrainingSet(vint8 in_training_number);
  virtual vint8 ChooseValidationSet(vint8 in_validation_number);
  virtual vint8 ChooseCandidates(vint8 in_candidate_number);
 
  vMatrix<vint8> ChooseTriples(vint8 number, 
                               vMatrix<vint8> indices_matrix);

  vMatrix<float> TripleDistances(vMatrix<vint8> triples_matrix);

  vint8 TriplesAndDistances(vint8 number, vMatrix<vint8> indices_matrix,
                           vMatrix<vint8> * triples_matrix,
                           vMatrix<float> * distances_matrix,
                           vMatrix<float> * scores_matrix);

  virtual vint8 UpdateMargins(vMatrix<vint8> triples_matrix,
                             vMatrix<float> margins_matrix,
                             float * errorp, float * marginp);

  float ComputeError(vector<vint8> * references, vector<float> * weights,
                     vMatrix<vint8> triples);

  vint8 PickRandom3(vint8 number, float * training_errorp,
                   float * validation_errorp);

};


// A class_triple_classifier object holds information about how to map
// objects into the real line. It can be of different types:
// type -1: invalid classifier, uninitialized.
// type 0: mapping is defined as distance to a reference object.
// type 1: mapping is defined as "projection" to a line between
//         two objects.
class class_triple_classifier
{
public:
  vint8 type;

  // object1 and object2 are IDs referring to the set of candidate objects,
  // not to the set of training objects in the whole database.
  // The IDs corresponding to the training objects in the whole database
  // are stored in variables database_first in database_second
  vint8 object1;
  vint8 object2;
  float weight;
  float z;

  vint8 database_first;
  vint8 database_second;

  class_triple_classifier()
  {
    type = -1;
    object1 = -1;
    object2 = -1;
    weight = 0;
    z = 1;

    database_first = database_second = -1;
  }

  class_triple_classifier(vint8 in_object1, float in_weight, float in_z)
  {
    type = 0;
    object1 = in_object1;
    object2 = -1;
    weight = in_weight;
    z = in_z;

    database_first = database_second = -1;
  }

  class_triple_classifier(vint8 in_object1, vint8 in_object2, 
                     float in_weight, float in_z)
  {
    type = 1;
    object1 = in_object1;
    object2 = in_object2;
    weight = in_weight;
    z = in_z;

    database_first = database_second = -1;
  }

  vint8 Print();
};


// In V_BoostMap and V_BoostMap2, the weak classifiers are defined
// using distances to reference points. Here, we also consider weak
// classifiers corresponding to FastMap-like projections onto a line
// defined by two pivot objects.
class V_BoostMap3 : public V_BoostMap2
{
protected:
  vector<class_triple_classifier> classifiers;
  vector<class_triple_classifier> unique_classifiers;

  // number of projections to try at each step
  vint8 projection_candidate_number;

  // allow_projections is non-zero iff we are allowed at the current
  // training step to consider "projections" onto "lines" defined by
  // pairs of pivot objects.
  vint8 allow_projections;

  // allow_lipschitz is non-zero iff we are allowed at the current
  // training step to consider distances to reference objects.
  vint8 allow_lipschitz;

  // last_new_z is the z of the last new classifier that we added 
  // (i.e. a classifier that was not just a removal or an adjustment
  // of an already chosen classifier). This value is useful to 
  // determine at what level of z we should accept weight modifications.
  float last_new_z;

public:
  V_BoostMap3();

  V_BoostMap3(vMatrix<float> in_distances,
              vint8 in_candidate_range, vint8 in_training_range,
              vint8 in_number_of_picked_candidates, 
              vint8 in_training_number, vint8 in_validation_number);

  V_BoostMap3(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_training_indices,
              vMatrix<vint8> in_validation_indices,
              vint8 in_candidate_number,
              vint8 in_training_number, 
              vint8 in_validation_number);

  V_BoostMap3(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_training_triples,
              vMatrix<vint8> in_validation_triples,
              vint8 in_candidate_number);


  ~V_BoostMap3();

  vint8 Zero();
  vint8 SetAllowProjections(vint8 in_value);
  vint8 SetAllowLipschitz(vint8 in_value);
  vint8 SetProjectionCandidateNumber(vint8 in_number)
  {
    projection_candidate_number = in_number;
    return 1;
  }
    
  static vMatrix<vint8> RandomPivots(vMatrix<vint8> candidate_indices,
                                     vint8 number);

  float PivotPairZ(vint8 pivot1, vint8 pivot2, float * alpha);

  vint8 PivotPairResults(vint8 pivot1, vint8 pivot2, vector<float> * results,
                        vMatrix<vint8> triples_matrix, 
                        vMatrix<float> triple_distances_matrix);

  vint8 LipschitzResults(vint8 index, vector<float> * results,
                        vMatrix<vint8> triples_matrix, 
                        vMatrix<float> triple_distances_matrix);

  float ClassifierZ(class_triple_classifier * classifier, float * alpha);

  vint8 ClassifierResults(class_triple_classifier * classifier, 
                         vector<float> * results,
                         vMatrix<vint8> triples_matrix, 
                         vMatrix<float> triple_distances_matrix);


  // It finds the lipschitz classifier that 
  // by removing it we attain the best accuracy
  class_triple_classifier LipschitzRemoval(vint8 * success);
  // It finds the best weight modification for already
  // selected lipschitz classifiers
  class_triple_classifier LipschitzChange(vint8 * success);
  // It finds the best Lipschitz weak classifier for the
  // current training step
  class_triple_classifier NextLipschitz();


  virtual float NextStep();
  virtual vint8 StepsDone();
  vint8 Dimensions()
  {
    return unique_classifiers.size();
  }

  // We add the classifier to unique_classifiers. If it
  // depends on the same object (or objects) as a classifier
  // that is already in unique_classifiers, and the only difference
  // is the weight, then we just update the weight of the existing
  // classifier. If the weight is zero then we remove the classifier.
  vint8 CleanUpClassifier(class_triple_classifier classifier);
  vint8 CleanClassifier(vector<class_triple_classifier> * result);

  // Returns the unique classifiers in a matrix form, that is easier to 
  // save and load.
  vMatrix<float> ClassifierMatrix();

  // returns a column vector of the weighs associated with the classifiers
  vMatrix<float> WeightsMatrix();

  // Returns the classifiers (not from unique_classifiers, but from
  // classifiers) in a matrix form, that is easier to  save and load.
  // This matrix contains more information than the matrix returned
  // in ClassifierMatrix. Here we have a row for each training step
  // that was performed. We also have additional columns: one for the
  // training error, one for validation error.
  vMatrix<float> DetailedClassifierMatrix();

  // Adds a new classifier
  vint8 AddClassifier(class_triple_classifier classifier);

  // Adds, in the order in which they are given, all the classifiers
  // stored in the rows of the matrix.
  vint8 AddClassifierMatrix(vMatrix<float> matrix);

  // Adds, in the order in which they are given, all the classifiers
  // stored in the rows of the matrix. It stops when either it has
  // gone through all the rows of the matrix, or the next row of the
  // matrix will increase the number of unique classifiers to dimensions+1.
  // If dimensions < 0, then all the rows of the matrix are processed.
  vint8 AddClassifierMatrix2(vMatrix<float> matrix, vint8 dimensions);

  float ComputeTrainingError(vector<float> * results);
  float ComputeValidationError(vector<float> * results);
  vint8 UpdateMargins(vMatrix<vint8> triples_matrix,
                     vMatrix<float> triple_distances_matrix,
                     vector<float> * results,
                     vMatrix<float> margins_matrix,
                     float * errorp, float * marginp);

  virtual vint8 PrintSummary();
  virtual vint8 PrintClassifier();
  virtual vint8 PrintAll();

  static char * Directory();
  vint8 SaveClassifier(const char * filename);
  vint8 LoadClassifier(const char * filename);
  vint8 LoadClassifier2(const char * filename, vint8 dimensions);

  // The cutoff under which z must be in order to accept weight modifications.
  float ChangeCutoff();

  // Return the embedding of the index-th object.
  vMatrix<float> Embedding1(vint8 index);

  // query_distances is a dimensions x 2 matrix, of distances
  // from the query to each of the two objects corresponding
  // to each dimension. If a dimension corresponds to a lipschitz
  // embedding, the second col is ignored for that dimension.
  // pivot_distances has, at position i, the distance between the i-th
  // pivot points (if the dimension corresponds to a projection).
  // If the number of rows of query_distances is less than the
  // number of unique classifiers, we use only the first classifiers.
  // types_matrix(i) is 0 if the i-th dimension is a lipschitz embedding,
  // and 1 if the i-th dimension is a pseudo-projection.
  static vMatrix<float> Embedding3(vMatrix<float> query_distances,
                                    vMatrix<float> pivot_distances,
                                    vMatrix<vint8> types_matrix);

  static vMatrix<vint8> ExtractTypes(vMatrix<float> classifiers);
};


// This class is a specific application of BoostMap to
// 2D points on the plane.
class vPlaneBoostMap : public V_BoostMap2
{
protected:
  vMatrix<float> points_matrix;
  vArray2(float) points;

  vMatrix<float> PickPoints();
  vMatrix<float> PointDistances();

public:
  vPlaneBoostMap();
  vPlaneBoostMap(vint8 in_number_of_points, 
                  vint8 in_training_number);

  ~vPlaneBoostMap();

  color_image * PointImage(vint8 rows, vint8 cols);

  color_image * BasisImage(vint8 rows, vint8 cols);

  virtual vint8 PrintAll();
};


float vL1_Distance(vMatrix<float> a, vMatrix<float> b);


// This class is an implementation of Bourgain embeddings, as described
// in Hristescu and Farach-Colton's publication
// "Cluster-preserving embedding of proteins"

class V_SparseMap : public V_BoostMap2
{
protected:
  vint8 log_n;
  vint8 number_of_features;
  vint8 features_to_use;

  // embeddings_matrix holds the Bourgain embeddings
  // (i.e. embeddings computed without using SparseMap 
  // heuristics). That is simpler to implement and test than
  // the actual SparseMap heuristics.
  vMatrix<float> embeddings_matrix;
  
  vArray2(float) embeddings;

  vector<vMatrix<vint8> > reference_sets;

protected:
  vint8 Zero();
  vint8 Initialize2(vMatrix<float> in_distances, vint8 in_validation);
  vint8 Initialize3(vMatrix<float> in_distances, 
                   vMatrix<vint8> in_candidate_indices,
                   vMatrix<vint8> in_validation_triples);
  vint8 Initialize4(vMatrix<float> in_distances, 
                   vMatrix<vint8> in_candidate_indices,
                   vMatrix<vint8> in_validation_indices,
                   vint8 in_validation);

public:
  V_SparseMap();
  V_SparseMap(vMatrix<float> in_distances, vint8 in_validation);

  V_SparseMap(vMatrix<float> in_distances,
              vint8 in_candidate_range, vint8 in_validation_range,
              vint8 in_validation_number);

  V_SparseMap(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_validation_indices,
              vint8 in_validation_number);

  V_SparseMap(vMatrix<float> in_distances, 
              vMatrix<vint8> in_candidate_indices,
              vMatrix<vint8> in_validation_triples);
  
  ~V_SparseMap();

  vint8 LogN();
  vint8 NumberOfFeatures();
  vint8 FeaturesToUse();
  vint8 SetFeaturesToUse(vint8 in_features_to_use);

  vint8 PickReferenceSets0();
  vint8 PickReferenceSets1(vector<vMatrix<vint8> > * sets);
  vMatrix<vint8> PickReferenceSets3(vint8 rows, vint8 cols, vint8 number);

  vMatrix<float> ComputeEmbeddings0();
  vMatrix<float> ComputeEmbeddings1(vector<vMatrix<vint8> > * sets);
  inline vint8 Dimensions()
  {
    return number_of_features;
  }

  virtual float ComputeValidationError();
  virtual vint8 PrintAll();
};


// This class is a specific application of SparseMap to
// 2D points on the plane, similar to vPlaneBoostMap
class vPlaneSparseMap : public V_SparseMap
{
protected:
  vMatrix<float> points_matrix;
  vArray2(float) points;

  vMatrix<float> PickPoints();
  vMatrix<float> PointDistances();

public:
  vPlaneSparseMap();
  vPlaneSparseMap(vint8 in_number_of_points, 
                  vint8 in_training_number);

  ~vPlaneSparseMap();

  color_image * PointImage(vint8 rows, vint8 cols);

  virtual vint8 PrintAll();
};


// This class is a specific application of SparseMap to approximating
// chamfer distances in the old hand database. The reason we need to
// extend the original class is that we need to know the actual
// (frame, rotation) specification of each reference image, and 
// V_SparseMap just holds an index that refers to its own distance matrix.
class vChamferSparseMap : public V_SparseMap
{
protected:
  vector<v3dMatrix<vint8> > reference_images;

public:
  vChamferSparseMap(vMatrix<float> in_distances,
                     vint8 in_candidate_range, vint8 in_validation_range,
                     vint8 in_validation_number,
                     vMatrix<vint8> * indices_matrix);


  // We pass in a matrix mapping indices to (frame, rotation).
  vint8 FindReferenceImages(vMatrix<vint8> * indices_matrix);

  vint8 ReferenceImages(vector<v3dMatrix<vint8> > * output);
};


class V_FastMap : public V_BoostMap2
{
protected:
  vMatrix<float> current_distances_matrix;

  vArray2(float) current_distances;

  vector<vint8> first_pivots;
  vector<vint8> second_pivots;
  

protected:
  vint8 Zero();
  vint8 Initialize2(vMatrix<float> in_distances, vint8 in_validation);
  vint8 Initialize3(vMatrix<float> in_distances, 
                   vMatrix<vint8> in_candidate_indices,
                   vMatrix<vint8> in_validation_triples);
  vint8 Initialize4(vMatrix<float> in_distances, 
                   vMatrix<vint8> in_candidate_indices,
                   vMatrix<vint8> in_validation_indices,
                   vint8 in_validation);

public:
  V_FastMap();
  V_FastMap(vMatrix<float> in_distances, vint8 in_validation);

  V_FastMap(vMatrix<float> in_distances,
            vint8 in_candidate_range, vint8 in_validation_range,
            vint8 in_validation_number);

  V_FastMap(vMatrix<float> in_distances, 
            vMatrix<vint8> in_candidate_indices,
            vMatrix<vint8> in_validation_indices,
            vint8 in_validation_number);

  V_FastMap(vMatrix<float> in_distances, 
            vMatrix<vint8> in_candidate_indices,
            vMatrix<vint8> in_validation_triples);

  ~V_FastMap();

  virtual float NextStep();

  virtual float ComputeValidationError();
  virtual vint8 StepsDone();
  virtual vint8 PrintAll();

  vint8 Dimensions()
  {
    return first_pivots.size();
  }

  vint8 PickPivots(vint8 * pivot1p, vint8 * pivot2p, 
                  vMatrix<float> point_distances_matrix);
  static vint8 UpdateCurrentDistances(vint8 pivot1, vint8 pivot2, 
                                     vMatrix<float> point_distances_matrix);
  float LineProjection(vint8 index, vint8 pivot1, vint8 pivot2,
                       vMatrix<float> point_distances_matrix);

  // i is the object, a and b are the two pivot points.
  static float LineProjection3(float ai, float bi, float ab);

  // This gives the FastMap embedding of a query given a set of pivots.
  // The first argument is the distances from the query to all the pivots.
  // The second argument is distances between pivots, but I should comment
  // more on that, it is not clear exactly what information should
  // be put there. For the time being, it is simply the original distances
  // between any two pivots.
  static vMatrix<float> Embedding(vMatrix<float> distances_to_pivots_matrix,
                                   vMatrix<float> intrapivot_distances_matrix);

  // This is an optimized version of Embedding, that hopefully gives the same
  // results as Embedding. Here, intrapivot_distances_matrix(i, j) is the 
  // distance between the corresponding points right before we perform projection
  // to a hyperplane for the (i/2)th time.
  static vMatrix<float> EmbeddingB(vMatrix<float> distances_to_pivots_matrix,
                                    vMatrix<float> intrapivot_distances_matrix);

  // OptimizedPivotDistances takes as a result a matrix of original distances
  // between pivots (like the one used in the function Embedding) and returns
  // a matrix of distances like the one used in EmbeddingB.
  static vMatrix<float> OptimizedPivotDistances(vMatrix<float> original_matrix);

  // OriginalPivotDistances returns a matrix with the initial (before any 
  // projection) distances between the pivots. It returns the kind of matrix
  // that is used as argument in OptimizedPivotDistances.
  vMatrix<float> OriginalPivotDistances(vMatrix<vint8> pivots_matrix);

  // Returns a number_of_pivots x 2 matrix, each row is the indices of
  // a pivot pair.
  vMatrix<vint8> PivotsMatrix();

  vMatrix<float> DistancesToPivots(vint8 index);

  vMatrix<float> Embedding1(vint8 index, 
                             vMatrix<float> intrapivot_distances);

  // This function returns a matrix whose size is equal to the number
  // of FastMap dimensions (i.e. number of pivot pairs selected so far).
  // It returns, for every dimension k, the number of distances to unique
  // objects that we need to compute in order to compute the k-dimensional 
  // FastMap embedding of a new object. All it has to do is see how many
  // unique objects appear in the first k pivot pairs.
  vMatrix<vint8> RequiredDistances();
};


class vPlaneFastMap : public V_FastMap
{
protected:
  vMatrix<float> points_matrix;
  vArray2(float) points;

  vMatrix<float> PickPoints();
  vMatrix<float> PointDistances();

public:
  vPlaneFastMap();
  vPlaneFastMap(vint8 in_number_of_points, 
                 vint8 in_training_number);

  ~vPlaneFastMap();

  color_image * PointImage(vint8 rows, vint8 cols);

  color_image * BasisImage(vint8 rows, vint8 cols);

  static vMatrix<float> Embedding(vMatrix<float> distances_to_pivots_matrix,
                                   vMatrix<float> intrapivot_distances_matrix);

  virtual vint8 PrintAll();
};


// This class is a specific application of FastMap to approximating
// chamfer distances in the old hand database. The reason we need to
// extend the original class is that we need to know the actual
// (frame, rotation) specification of each pivot image, and 
// V_FastMap just holds indices that refer to its own distance matrix.
class vChamferFastMap : public V_FastMap
{
protected:
  vMatrix<vint8> pivot_images;

public:
  vChamferFastMap(vMatrix<float> in_distances,
                     vint8 in_candidate_range, vint8 in_validation_range,
                     vint8 in_validation_number,
                     vMatrix<vint8> * indices_matrix);


  ~vChamferFastMap();

  // We pass in a matrix mapping indices to (frame, rotation).
  vint8 FindPivotImages(vMatrix<vint8> * indices_matrix);

  vMatrix<vint8> PivotImages();
};




#endif // VASSILIS_BOOST_MAP_H
