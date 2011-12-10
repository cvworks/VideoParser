#ifndef VASSILIS_BOOST_NN_H
#define VASSILIS_BOOST_NN_H


#include "basics/algebra.h"
#include "boosting.h"


// A multiplier maps an object into some real number between 0 and 1.
// It is the equivalent of a splitter in BoostMap, but it may consist
// of more complicated tests, instead of just testing one dimension.
// I am not sure if the actual implementation will map things into
// just 0 or 1, or it will also use intermediate values.
class vMultiplier
{
public:
  // types:
  // 0 if the multiplier accepts values >= low
  // 1 if the multiplier accepts values < high
  // 2 if the multiplier accepts low <= values < high
  // 3 if the multiplier accepts values < low OR >= high.
  // 4 if the multiplier accepts everything.
  vint8 index;
  vint8 type;
  float low;
  float high;

  vMultiplier()
  {
    index = -1;
  }
  vMultiplier(vint8 in_type, float in_low, float in_high);
  float Factor(float value);
  vint8 Print();
};


// class vQsClassifier is an adaptation of vSpecializedClassifier
// (which is  used in BoostMap), for use in Boost-NN. Qs stands
// for query-sensitive.
class vQsClassifier : public class_unique
{
public:
  // multiplier decides whether to accept the query or not.
  vector<vMultiplier> * multipliers;
  
  // index of the dimension used for the triple classifier
  vint8 index;
  float weight;
  // for debugging, see how many training objects (or training triples,
  // depending on the implementation) were selected
  // by the multiplier.
  vint8 range;

public:
  vQsClassifier();
  vQsClassifier(vint8 in_index, float in_weight)
  {
    multipliers = new vector<vMultiplier>;
    index = in_index;
    weight = in_weight;
    range = -1;
  }

  // This constructor copies the vector of multipliers,
  // so that we can modify it (in the newly constructed object)
  // without modifying the original classifier.
  vQsClassifier(vQsClassifier * classifier);

  ~vQsClassifier();

  float Factor1(v3dMatrix<float> & objectm);
  float Factor2(v3dMatrix<float> & objectsm, vint8 row);

  vint8 Print();

  virtual void delete_unique();
};




// Common superclass of vBoostNN and vBnnqs.
class vBnn
{
protected:
  // this class can be used for run-time checking of the actual subclass
  // that the object bevint8s to.
  char * class_name;
  
  // number of training objects.
  vint8 training_number;

  // number of objects to be used for testing.
  vint8 test_number;

  // number of classes (distinct class labels)
  vint8 classes;

  // number of features. We assume that each object is represented as 
  // a Euclidean vector, with a fixed number of dimensions
  vint8 attributes;

  // number of triples to be used for training.
  vint8 training_triple_number;

  // number of triples to be used for validation.
  vint8 validation_triple_number;

  // each row in training_vectors_matrix is a training object.
  vMatrix<float> training_vectors_matrix;

  // each row in test_vectors_matrix is a test object.
  vMatrix<float> test_vectors_matrix;

  // training_labels_matrix(i) is the class label of the i-th 
  // training object.
  vMatrix<vint8> training_labels_matrix;

  // test_labels_matrix(i) is the class label of the i-th test object.
  vMatrix<vint8> test_labels_matrix;

  // class_sizes[i] will be the number of training data for class with id
  // i. Note that, in this implementation, no class has id 0. Therefore,
  // class_sizes_matrix.Size() = classes + 1.
  vMatrix<vint8> class_sizes_matrix;
  
  // The margins matrices hold the margin for each triple, i.e. how far
  // from the classification threshold (and in which direction) the
  // result for each triple is. I am not sure if these margins are actually
  // meaningful, but they provide a quick way to measure the training 
  // and validation triple error (by thresholding the margin).
  vMatrix<float> training_margins_matrix;
  vMatrix<float> validation_margins_matrix;

  // training_triples_matrix holds the training triples, i.e. 
  // the training set used by AdaBoost. Each row 
  // in training_triples_matrix holds (q, a, b),
  // which are indices into rows of training_vectors_matrix, i.e. they 
  // correspond to training objects.
  // Overall, for all triples in training_triples_matrix, 
  // validation_triples_matrix  and test_triples_matrix, they are of 
  // the form (q, a, b), where q and a have the same class, and b 
  // bevint8s to a different class.
  vMatrix<vint8> training_triples_matrix;

  // validation_triples_matrix has the same format as 
  // training_triples_matrix, but these triples are not used for training,
  // and thus they are good for showing us how well the classifier 
  // generalizes to triples that it has not been trained with.
  // Note that the training triples change at each iteration, whereas the
  // validation triples remain fixed, so overall they are not as informative
  // as they are in BoostMap. Another reason why they are not as informative 
  // is that here we use no separate validation set; the validation triples
  // are formed using objects of the training set.
  vMatrix<vint8> validation_triples_matrix;

  // training_factors_matrix(i) is the training weight of the i-th training
  // triple. These weights are updated by AdaBoost.
  vMatrix<float> training_factors_matrix;

  // initial_factors should be set to a copy of training_factors_matrix
  // so that we can remember the initial weights.  If we use
  // some kind of importance sampling then those weights will not be uniform.
  float_matrix initial_factors;

  // test_triples_matrix contains, at each row, (q, a, b) such that q is 
  // an index into test_vectors_matrix (and therefore corresponds to 
  // a test object) but a and b are training objects.
  vMatrix<vint8> test_triples_matrix;
  // Confusion matrix on test objects.
  vMatrix<float> test_confusion;
  // Confusion matrix on test triples. Entry i,j is the 
  // number of times a triple was found where q was class i
  // and it was found closer to an object of class j.
  vMatrix<float> test_triple_confusion;

  // alpha_limits_matrix has two entries for each attribute, that
  // specify the min and max alpha values that should be tested for
  // that attribute. This is important, because in Z_Prime the
  // exponents depend on alpha, and an exponent over 88 leads
  // to a numerically infinite result. Note that exponentiation
  // also happens in the Z function and in UpdateWeights, but
  // those only use exponents that have been used before in Z_Prime.
  vMatrix<float> alpha_limits_matrix;

  // triple_temp is some temp information on triples, which we can access
  // using PrintWeight, PrintWeights, etc. Originally those functions
  // were intended just for weights, but it is useful to apply them
  // to other values of the triples, like the contribution each triple
  // makes to z.
  vMatrix<float> triple_temp;

  // subject_ids_matrix is used for some fairly common cases, where each
  // object (for example a handwritten digit or a vocal utterance) is
  // produced by a subject, and the subjects used in the training are
  // disjoint from the subjects used in the testing. An example is
  // the "vowels" dataset. In that case, when we choose the training
  // triples, we should make sure that q and a were produced by
  // different subjects. Otherwise, ResampleTriples will almost always
  // choose q and a from the same subject, and those triples will be
  // "too easy".
  vMatrix<vint8> subject_ids_matrix;

  // test_distances_matrix will contain
  // in position (i,j) the distance between the i-th test
  // object and the j-th training object, based on the distance indices
  // and weights chosen so far. This will make it much faster to 
  // compute k-nn errors after every step, because we only have to update
  // those distances based on the last index and weight chosen. Note that
  // we may run out of memory, if there are too many entries in this
  // matrix.
  vMatrix<float> test_distances_matrix;

  // training_scores_matrix, if available, will keep at entry (i, j) the
  // score of the i-th triple under the j-th attribute.
  vMatrix<float> training_scores_matrix;

  // all these arrays are arrays that are within the matrices specified
  // above. We store them as separate variables for convenience.
  vArray2(float) training_vectors;
  vArray2(float) test_vectors;
  vArray(vint8) training_labels;
  vArray(vint8) test_labels;
  vArray(vint8) class_sizes;

  // class_indices[i] will be a vector with size training_sizes[i]. Its entries
  // will be the indices (i.e. row numbers) of all training data that bevint8
  // to class with id = i.
  vArray(vector<vint8> ) class_indices;

  vArray(float) training_margins;
  vArray(float) validation_margins;

  vArray2(vint8) training_triples;
  vArray2(vint8) validation_triples;
  vArray(float) training_factors;
  vArray2(vint8) test_triples;
  vArray2(float) alpha_limits;
  vArray(vint8) subject_ids;
  vArray2(float) training_scores;

  vector<float> distance_zs;
  vector<float> training_errors;
  vector<float> validation_errors;
  vector<float> test_errors;
  vector<float> test_triple_errors;

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

  // test errors of the strong classifier learned so far
  // test_error is the percentage of misclassified test objects, which
  // is what we are most interested in.
  float test_error;

  // test_error2 is kept for debugging. It is computed in a slower way
  // than test_error, but the two variables are supposed to eventually
  // have identical values.
  float test_error2;

  // test_triple_error is a statistic that is not as important, but I
  // was curious about it. 
  float test_triple_error;
  float test_triple_error2;

  // some variables that collect information useful for future
  // optimizations:
  // counts the total number of iterations spent on binary search
  // for minimizing z.
  vas_int16 iterations;
  // The smallest x passed to the pow(e, x) function
  float min_exp;
  // The largest x passed to the pow(e, x) function
  float max_exp;

  // This variable tells the training algorithm if it should allow
  // some dimensions to have an overall negative weight (as stored in
  // cleaned_factors).
  vint8 allow_negative_weights;
 
  // subject_ids_set is 1 if the subject_ids_matrix variable contains
  // information about subject ids and is 0 if subject_ids_matrix is
  // uninitialized.
  vint8 subject_ids_set;

  // The next few matrices are not always storing the right value.
  // We need to call AnalyzeTriples to get the right information stored
  // there.
  // contains indices of training objects that are misclassified using
  // 1-nn classification (i.e. they have a different-class neighbor closer
  // than the closest same-class neighbor).
  vMatrix<vint8> bad_qs;
  // contains indices of training objects that have at least one 
  // different-class neighbor farther away than the closest same-class
  // neighbor. This typically includes all objects, it is pretty 
  // pathological not to satisfy this requirement.
  vMatrix<vint8> good_qs;
  // For each training object q, we store some 
  // (maybe all) the other-class neighbors that
  // are closer than its same-class neighbor.
  vMatrix<vint8> bad_bs;
  // Similarly, for each training object q, we store some
  // (usually not all) the other-class objects that are further away
  // than the same-class nearest neighbor.
  vMatrix<vint8> good_bs;
  // Here we store, for each q, other-class objects that are equally
  // far to q as its same-class nn.
  vMatrix<vint8> tie_bs;
  // Number of other-class neighbors closer than the same-class nn.
  vMatrix<vint8> bad_numbers; 
  // Number of other-class objects farther than the same-class nn.
  vMatrix<vint8> good_numbers;
  // Number of other-class objects that are as far from q as its
  // same-class nn.
  vMatrix<vint8> tie_numbers;
  // Number of same-class objects closer to q than its other-class nn.
  vMatrix<vint8> good_a_numbers;
  // same_class_nn(q) is the index of the same-class nn of q.
  vMatrix<vint8> same_class_nn;
  // other_class_nn(q) is the index of the other-class nn of q.
  vMatrix<vint8> other_class_nn;

  // total number of bad triples, i.e. triples (q, a, b) where
  // a is the same-class 1-nn of q, and b is of a different 
  // class than q, but closer to q than a is to q.
  vint8 bad_triples;
  
  // total number of tie triples,  i.e. triples (q, a, b) where
  // a is the same-class 1-nn of q, and b is of a different 
  // class than q, but as close to q as a is to q.
  vint8 tie_triples;

  // knn_training_errors(k) is the training error using k-nn
  // classification
  vMatrix<float> knn_training_errors;

  // knn_test_errors(k) is the test error using k-nn
  // classification
  vMatrix<float> knn_test_errors;

  // best_k stores the value of k that achieves the lowest k-nn
  // training error. This is the value of k that we use to 
  // measure the test error.
  vint8 best_k;

  // training_classification and test_classification
  // will remember the label assigned to each object
  // using the best_k nearest neighbors. They will be reset
  // every time we change the classifier
  vint8_matrix training_classification;
  vint8_matrix test_classification;

  // lowest k-nn training (or test) error (based on all possible values of k, 
  // up to an upper limit).
  float best_knn_training;
  float best_knn_test;

  // the next stuff is useful for implementing a decision tree
  // of distance measures.

  // active are the objects that we want to train and test this
  // distance measure with.  If we don't use a decision tree
  // but a single classifier, as in LCVPR 2005, then
  // all objects are active.
  // active_training(index) = 0 iff the object corresponding to that 
  // index is inactive.
  vint8_matrix active_training;
  vint8_matrix active_test;

  // previous labels are the labels that have been assigned to objects
  // by previous parts of the tree.  This actually only makes sense
  // in a cascade, whereby the time we get to the current classifier,
  // we have already assigned labels to the inactive objects.
  vint8_matrix previous_training_labels;
  vint8_matrix previous_test_labels;

protected:
  vint8 InitialWeights();  

public:
  vBnn();

  vBnn(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vint8 in_training_number, vint8 in_validation_number);

  vBnn(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vMatrix<vint8> in_training_triples,
            vMatrix<vint8> in_validation_triples);

  ~vBnn();

  vint8 Zero();

  // Initialize training and test data.
  vint8 InitializeData(vMatrix<float> in_training_vectors,
                      vMatrix<vint8> in_training_labels,
                      vMatrix<float> in_test_vectors,
                      vMatrix<vint8> in_test_labels);

  // Initialize training and validation triples.
  vint8 InitializeA(vint8 in_training_number, vint8 in_validation_number);

  vint8 InitializeB(vMatrix<vint8> in_training_triples,
                    vMatrix<vint8> in_validation_triples);

  const char * get_class_name()
  {
    return class_name;
  }

  // chooses a set of triples, suitable for use as a training or 
  // validation set.
  vMatrix<vint8> ChooseTriples(vint8 number);

  // chooses a set of triples suitable for use as a test triple set.
  vMatrix<vint8> ChooseTestTriples(vint8 number);

  // We need to call this function every time we choose training triples,
  // to store the necessary info into alpha_limits.
  vMatrix<float> ComputeAlphaLimits(vMatrix<vint8> triples_matrix);

  // Finds first the attribute with the lowest weighted error, and 
  // adds that.
//  virtual float NextStepFast() = 0;

  vint8 NextSteps(vint8 i);
  // get the z and alpha (weight) that correspond to the attribute
  // specified by index.
  float DistanceZ(vint8 index, float * alpha);

  // get the results corresponding to the given attribute. the results
  // are essentially the product of the prediction made by the classifier
  // corresponding to index, and the binary (-1 or 1) class label of
  // the i-th triple. The results are stored inside the vector "results",
  // which is assumed to already have enough storage space allocated.
  vint8 DistanceResults(vint8 index, vector<float> * results, 
                       vMatrix<vint8> triples_matrix);

  float_matrix distance_results(vint8 index, vMatrix<vint8> triples_matrix);

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
  vint8 MinimizeZ(vector<float> * results, float * z, float * a,
                 float min_alpha, float max_alpha);

  // this function is like the previous one, but it takes its input
  // results as a matrix
  vint8 MinimizeZ(float_matrix results, float * z, float * a,
                 float min_alpha, float max_alpha);

  // computes the z corresponding to a given weight and to the given
  // results.
  float Z(float a, vector<float> * results);

  // computes the derivative of Z with respect to alpha (a), given
  // the results.
  float Z_Prime(float a, vector<float> * results);

  // assuming that we picked the weak classifier giving these
  // results, and with the specified z and alpha, update the 
  // training weights (weights of the training triples).
  float UpdateWeights(vector<float> * results, float z, float alpha);

  // Computes errors and margins for the last weak classifier that
  // we chose. We don't specify the classifier directly, but we 
  // specify the results obtained from that classifier, and its
  // weight (alpha). 
  float ComputeLastError(float alpha, vector<float> * results);

  // ComputeTrainingError and ComputeValidationError take as input
  // the results of the last weak classifier that we chose, on 
  // the training or validation triples. These functions update
  // training_margins_matrix and validation_margins_matrix, and compute
  // the training and validatin error.
  float ComputeTrainingError(vector<float> * results, float weight);
  float ComputeValidationError(vector<float> * results, float weight);
  float ComputeTestError();
  // Updates the entries of test_distances_matrix, to add 
  // distances in the given feature (indexed by index), weighted
  // by alpha.
  vint8 UpdateDistances(vint8 index, float alpha);

  // functions that provide read-only access to member variables or 
  // parts of member variables.
  vint8 Attributes();
  float TrainingError();
  float TrainingMargin();
  float ValidationError();
  float LastError();
  float LastEt();
  float LastCorrelation();
  float LastZ();
  float StepZ(vint8 step);
  float StepTrainingError(vint8 step);
  float StepValidationError(vint8 step);

  // UpdateMargins is used in ComputeTrainingError and ComputeValidationError
  // to do the actual work, which is pretty similar in both cases, it
  // just works on different data.
  vint8 UpdateMargins(vMatrix<vint8> triples_matrix,
                     vector<float> * results,
                     vMatrix<float> margins_matrix,
                     float * errorp, float * marginp,
                     float dimension_weight);

  // For debugging: return the sum of the weights of all training triples.
  // weights should sum up to 1.
  float SumWeights();

  // this function takes in reference_indices and reference_weights,
  // which typically represent the weak classifiers chosen at each
  // training step and the associated weights. The outputs (stored
  // into cleaned_indices and cleaned_factors) are cleaned-up versions,
  // in which no index (corresponding to a dimension, aka feature, aka
  // attribute) is repeated twice. object number is useful in order
  // to allocate memory for a scratch array that is used in the computations.
  static vint8 CleanUp(vint8 attributes, vector<vint8> * reference_indices,
                      vector<float> * reference_weights,
                      vector<vint8> * cleaned_indices,
                      vector<float> * cleaned_factors);
  
  // this version of CleanUp is more appropriate for vBnnqs. In principle
  // we should check for every pair of classifiers to see if they 
  // match. However, two classifiers match only if their multipliers are
  // identical and their attribute indices also match, and this is 
  // a bit more complicated to test for. So, at least in the initial
  // implementation, we only combine classifiers that have no multipliers.
  static vint8 CleanUp(vint8 attributes, vector<vQsClassifier> * step_classifiers,
                      vector<vQsClassifier> * unique_classifiers);

  
  // result(i) is the distance between object and the i-th training object.
  // dimensions and weights specify the dimensions to be used for 
  // measuring distances, and the weight of each dimension.
  vMatrix<float> TestDistances(v3dMatrix<float> & object, 
                                vector<vint8> * dimensions,
                                vector<float> * weights);

  // Here instead of passing in an object we just pass in an index into 
  // vectors_matrix. Typically vectors_matrix is the training vectors
  // or the test vectors.
  vMatrix<float> TestDistances4(vint8 index, v3dMatrix<float> & vectors,
                                 vector<vint8> * dimensions,
                                 vector<float> * weights);

  // Same as TestDistances4, but here we compute L2 distances.
  vMatrix<float> TestDistances2b(vint8 index, v3dMatrix<float> & vectors,
                                 vector<vint8> * dimensions,
                                 vector<float> * weights);

  // Here instead of passing in an object we just pass in an index into 
  // vectors_matrix. Typically vectors_matrix is the training vectors
  // or the test vectors.
  virtual vMatrix<float> TestDistances2(vint8 index, v3dMatrix<float> & vectors) = 0;

  // Similar to TestDistances2, but here we only compute distances to
  // objects that bevint8 to the class specified by class_id.
  vMatrix<float> TestDistances5(vint8 index, v3dMatrix<float> & vectors,
                                 vint8 class_id,
                                 vector<vint8> * dimensions,
                                 vector<float> * weights);

  // Similar to TestDistances2, but here we only compute distances to
  // objects that bevint8 to the class specified by class_id.
  virtual vMatrix<float> TestDistances3(vint8 index, v3dMatrix<float> & vectors,
                                         vint8 class_id) = 0;

  
  // Returns the distance between object1 and object2. How to measure the
  // distance is specified by dimensions (which dimensions to use) and
  // weights (how much to weigh each dimension).
  float ClassifierDistance(v3dMatrix<float> & object1, 
                           v3dMatrix<float> & object2,
                           vector<vint8> * dimensions, vector<float> * weights);

  // Here, index1 is an index into a row of vectors1, and index2 is an
  // index into a row of vectors2.
  float ClassifierDistance2(vint8 index1, v3dMatrix<float> & vectors1, 
                            vint8 index2, v3dMatrix<float> & vectors2,
                            vector<vint8> * dimensions, vector<float> * weights);

  // This function does the same job as AnalyzeTriples, but for test
  // objects.
  vint8 AnalyzeTestTriples();

  // This function is an alternative regenerate triples, so that they are 
  // more representative (a is close to q, b is importance-sampled).
  vMatrix<vint8> ResampleTriples2(vint8 number, 
                                  vMatrix<float> weights_matrix);
  
  // This function is used to look into the contribution that each training
  // object makes into the value of z. I want to check if a few objects 
  // possibly make too large a contribution on z. Here we measure the
  // contribution that will be made by each object if we choose the
  // given dimension in the next step.
  vMatrix<float> SortZContributions(vint8 dimension);

  vint8 SetAllowNegativeWeights(vint8 in_value);

  virtual float CurrentDistanceWeight(vint8 distance_index) = 0;

  vint8 SetSubjectIds(vMatrix<vint8> subject_ids);

  vMatrix<vint8> BadQs();
  
  vMatrix<vint8> GoodQs();
  
  vMatrix<vint8> BadNumbers();
  
  vMatrix<vint8> GoodNumbers();
  
  vMatrix<vint8> TieNumbers();
  
  vMatrix<vint8> GoodANumbers();
  
  vMatrix<vint8> BadBs();
  
  vMatrix<vint8> GoodBs();
  
  vMatrix<vint8> TieBs();
  
  vMatrix<vint8> SameClassNN();
  
  vMatrix<vint8> OtherClassNN();
  
//  virtual vMatrix<float> ClassifierMatrix() = 0;

  // directory where classifiers should be saved to or loaded from.
  char * BaseDirectory();

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

  // Prints some information about this object.
  virtual vint8 PrintSummary() = 0;

  // Print triples, in a format (q, a, b) - (q_class, a_class, b_class)
  vint8 PrintTraining();
  vint8 PrintValidation();
  vint8 PrintTestTriples();
  // q_labels will be training_labels for training and validation triples,
  // and test_labels for test_triples.
  vint8 PrintTriples(vMatrix<vint8> triples_matrix, vMatrix<vint8> q_labels);

  // In column i, j, put the number of triples where q and a are 
  // from class i and b is from class j.
  vint8 PrintTripleStats();
  vMatrix<vint8> TripleStats(vMatrix<vint8> triples_matrix,
                             vMatrix<vint8> q_labels_matrix);

  // print the index-th training vector and the index-th 
  // test vector.
  vint8 PrintVectors(vint8 index);

  vint8 PrintConfusions();

  virtual vint8 PrintClassifier() = 0;

  vint8 PrintTrainingTriple(vint8 index);

  // Print info about training weights or whatever else 
  // is stored into triple_temp.
  // Print the i-th smallest value.
  vint8 PrintWeight(vint8 index);
  vint8 PrintWeights(vint8 period);
  vint8 PrintSomeWeights(vint8 start, vint8 end, vint8 period);
  vint8 PrintWeightSum(vint8 start, vint8 end);

  // Compute and print the k-nearest neighbor error on the 
  // training set and the test set.
  vint8 KnnErrors(vint8 k, float * test_error = 0, 
                 float * training_error = 0);

  // KnnError returns the error, on the objects described
  // as columns of evaluation_set, of the current k-nearest 
  // neighbor clssifier described by cleaned_indices and
  // cleaned_factors. discard_first should be 1 if 
  // the evaluation set is a subset of the training set and
  // 0 if the evaluation set has no intersection with the training
  // set. It tells the program to discard the nearest neighbor, because
  // that would be the query object itself.
  float KnnError(vMatrix<float> evaluation_set, 
                 vMatrix<vint8> evaluation_labels,
                 vint8 k, vint8 discard_first);

  // Returns a matrix whose k-th entry is the k-nn nearest
  // neighbor accuracy, using L1 distance. Entry 0 is not meaningful.
  vMatrix<float> KnnTrainingErrors(vint8 max_k);

  // Same thing, but for the test error rate, which is faster to
  // compute because of test_distances_matrix;
  vMatrix<float> KnnTestErrors(vint8 max_k);

  // Same thing, but for the test error rate, which is faster to
  // compute because of test_distances_matrix;
//  virtual vMatrix<float> KnnTestErrors2(vint8 max_k) = 0;

  // For the specified training triple (q, a, b,) indexed by triple_index,
  // show how many same-class and total objects are closer to q than a,
  // and similarly how many are closer to q than b. Hopefully this will
  // be a useful measure in deciding whether a given training triple is
  // useful or not.
  vint8 TripleRankInfo(vint8 triple_index);

  // We print the k nearest neighbors of the two objects (one in the
  // training set and one in the test set) that are indexed by object_index.
  vint8 KnnInfo(vint8 object_index, vint8 k);

  // print some bad triples for each training object. Bad triples are
  // triples (q, a, b) formed by training object q, its same-class
  // 1-nearest neighbor a, and an other-class object b that is closer
  // to q than a is to q.
  vint8 PrintBadTriples();

  // same as PrintBadTriples, but here we can specify a range of training
  // objects.
  vint8 PrintBadTriples2(vint8 start, vint8 end);

  // print a histogram of bad numbers.
  vint8 BadNumberHistogram();

  // print the bad, tie, and good_a numbers for q.
  vint8 QNumbers(vint8 q);

  // print the bad, tie and good_a numbers for a range of training 
  // objects, from start to end.
  vint8 QNumbers2(vint8 start, vint8 end);

  // return a matrix which, at the i-th column, stores the minimum
  // and maximum value attained by any training object in the i-th
  // attribute.
  vMatrix<float> DimensionRanges();
  vint8 PrintDimensionRanges();

  // return a matrix which, at the i-th column, stores the mean
  // and std for the i-th attribute values of all training objects.
  vMatrix<float> DimensionStds();
  vint8 PrintDimensionStds();

  // normalize values of training vectors, so that, 
  // at each attribute, the minimum
  // value by any training object is 0, and the max value is 1.
  vint8 NormalizeRanges();
  
  // normalize values of training vectors so that, at each attribute,
  // the mean is 0.5 and the std is 0.5, as computed based on the
  // values of all training objects for that attribute. Essentially,
  // the mean and std for each column of training_vectors_matrix 
  // becomes 0.5.
  vint8 NormalizeStds();

  // for all training and test vectors we square all their attributes.
  // This is part of a MISTAKE that I should fix: for "naive-knn" 
  // experiments, I thought that to compute Euclidean distances, all
  // I had to do was square entries and then compute L1 distances.
  // That is not correct, therefore the results I put into the 
  // submission are not correct.
  vint8 SquareEntries();

  // training_scores holds, at each column, the results of the i-th
  // attribute on all training triples.
  vint8 TrainingScoresExist();
  vint8 MakeTrainingScores();
  vint8 DeleteTrainingScores();

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
  vMatrix<float> AttributeErrors();

  // return the corresponding member variables.
  vint8 BestK();
  float BestKnnTraining();
  float BestKnnTest();
  vint8 TrainingNumber();
  vint8 TestNumber();

  // This function is used for debugging and understanding what's going on.
  // result[0][i] is the i-the smallest weight, and result[1][i] is the index
  // of the i-th smallest weight (i.e. the index of the triple with that
  // weight).
  vMatrix<float> SortTrainingWeights();

  // Finds the attribute that achieves the smallest z, and chooses
  // the next weak classifier to depend on that attribute.
  virtual vint8 NextStep() = 0;
  // return the number of training steps performed so far.
  virtual vint8 StepsDone() = 0;

  // AnalyzeTriples is a preprocessing function called for ResampleTriples2.
  // bad_qs will receive indices of objects that have "bad" neighbors,
  // i.e. different-class objects closest than the same-class 1-nearest 
  // neighbor. good_qs will receive objects that have "good" neighobrs, 
  // i.e. different-class objects farther away from q than its same-class
  // 1-nearest neighbor. Note that bad_qs and good_qs will probably overlap.
  // bad_bs(q) is a sampling of bad neighbors of q. If bad_bs->Rows() is
  // greater than the number of bad neighbors of q, only the leftmost
  // cols of the row bad_bs(q) will contain valid entries. Similar for
  // good_bs. bad_numbers(q) is the number of bad_neighbors of q, 
  // good_numbers(q) is the number of good neighbors of q, and 
  // same_class_nns(q) is the index of the same-class nearest neighbor
  // of q. Note that all these arguments are output arguments, they
  // don't hold any useful information when this function is called.
  vint8 AnalyzeTriples();

  // This function is called to regenerate triples, so that they are 
  // more focused (i.e. both a and b are close to q).
  vMatrix<vint8> ResampleTriples(vint8 number, vint8 max_k);

  // Here triples are sampled deterministically: For each object, we
  // make a single triple consisting of the object, its same-class nn
  // and its other-class nn.
//  virtual vMatrix<vint8> ResampleTriples3() = 0;

  // Here triples are also sampled deterministically. For each object,
  // we make as many triples as the number of other classes. Each triple
  // consists of the object, its same-class nn, and its nn from each
  // of the other classes.
//  virtual vMatrix<vint8> ResampleTriples4() = 0;

  // Here we exclude training objects that have too many bad neighbors.
//  virtual vMatrix<vint8> ResampleTriples5() = 0;

  // finds the specified number of nearest neighbors to a particular
  // training object.
  // distances_matrix(i) is the distance of the specified object
  // to the i-th training object.  This function can be used as an auxiliary
  // function for finding nearest neighbors overall, or among objects
  // of a specific set of classes.  In the latter case, we just have to
  // make the entries in the distances_matrix that we do not care about
  // very large BEFORE we call this function.
  // indices[k] will be the k-1 nearest-neighbor.
  // distances[k] will be the distance of the neighbor to the specified object
  // actual_number will store the number of neighbors actually found,
  // and this will be greater than number if there is a tie
  // (in that case we will include all objects tying for
  // number-th nearest neighbor).
  vint8 find_nearest_neighbors(float_matrix distances_matrix, vint8 neighbor_number, 
                              vector<vint8> * indices,
                              vector<float> * distances, vint8 * actual_number);

  // finds the specified number of nearest neighbors to a particular
  // training object.
  // indices[k] will be the k-1 nearest-neighbor.
  // distances[k] will be the distance of the neighbor to the specified object
  // actual_number will store the number of neighbors actually found,
  // and this will be greater than number if there is a tie
  // (in that case we will include all objects tying for
  // number-th nearest neighbor).
  vint8 nearest_neighbors(vint8 object_index, vint8 number, vector<vint8> * indices,
                         vector<float> * distances, vint8 * actual_number);

  // the same is nearest_neighbors, but here in the results we only
  // include objects of the same class as the specified object
  vint8 similar_nearest_neighbors(vint8 object_index, vint8 number, vector<vint8> * indices,
                         vector<float> * distances, vint8 * actual_number);

  // the same is nearest_neighbors, but here in the results we only
  // include objects that are not of the same class as the specified object
  vint8 different_nearest_neighbors(vint8 object_index, vint8 number, vector<vint8> * indices,
                         vector<float> * distances, vint8 * actual_number);


  // result(k) is the class label that is the result of
  // knn classification for each k in 1, ..., neighbor_number
  // for the specified object.  The object is simply specified
  // by the set of distances to all training objects.
  // if the object is actually a training object
  // and also if we need to exclude training objects from the same subject,
  // it is the responsibility of the calling function to take care of those things.
  vint8_matrix knn_classifications(float_matrix distancesm, 
                                   vint8 neighbor_number);

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
  vint8_matrix knn_accuracies(float_matrix distancesm, vint8 true_label,
                             vint8 neighbor_number);

  // result(k) is the class label that is the result of
  // knn classification for each k in 1, ..., neighbor_number
  // for the specified training object
  vint8_matrix knn_classification_training (vint8 object, vint8 neighbor_number);

  // result(k) is the class label that is the result of
  // knn classification for each k in 1, ..., neighbor_number
  // for the specified training object
  vint8_matrix knn_classification_test (vint8 object, vint8 neighbor_number);


  // number is the number k of nearest neighbors
  // to use for classification
  vint8_matrix training_accuracies(vint8 neighbor_number);

  vint8_matrix test_accuracies(vint8 neighbor_number);

  // initializes a binary learner, which, after training,
  // can be used as a component for a decision tree of distance measures.
  // number is the number k of nearest neighbors to use
  // for classification.
  AdaBoost * binary_learner(vint8 neighbor_number);
};



// vBoostNN is a class that provides an interface for my method of
// applying boosting to multiclass recognition problems. This method
// was described in my ICML 2004 submission.
// A lot of the
// code and ideas from the class_BoostMap class and its subclasses are
// repeated here. If there was more time, the code would be a lot
// more elegant if I avoided duplicating code between class_BoostMap
// and vBoostNN.
// In principle, my method needs as inputs a set of objects, and a 
// "black box" that provides a distance between pairs of objects 
// at each training round. The training round evaluates the distance
// and assigns a weight to it. In this implementation, at least for a 
// start, we make a simpler assumption, i.e. that each object is
// represented by a Euclidean vector, and we want to learn a weighted
// L1 distance in this Euclidean space. This means that the black box
// can be thought of as always picking a one-dimensional feature and
// returning the corresponding distance that is based only on that 
// feature. Also, for the time being I don't handle the possibility 
// that some of the feature values may be missing.
// Another assumptions we are making is that the class labels are 
// integers between 1 and the number of classes.
class vBoostNN : public vBnn
{
protected:
  vector<vint8> distance_indices;
  vector<float> distance_factors;
  vector<vint8> cleaned_indices;
  vector<float> cleaned_factors;

  // save a strong classifier to file.
  vint8 save_classifier_matrix(const char * filename, float_matrix classifier_matrix);

public:
  vBoostNN();

  vBoostNN(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vint8 in_training_number, vint8 in_validation_number);

  vBoostNN(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vMatrix<vint8> in_training_triples,
            vMatrix<vint8> in_validation_triples);

  // this constructor is useful when the BoostNN object is 
  // used in training a decision tree of distance measures.
  vBoostNN(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vint8 in_training_number, vint8 in_validation_number,
            vint8_matrix argument_active_training,
            vint8_matrix argument_previous_training_labels,
            vint8_matrix argument_active_test,
            vint8_matrix argument_previous_test_labels);

  ~vBoostNN();


  // This function uses ResampleTriples to create training and validation
  // triples, which are used to initialize a new vBoostNN object.
  vBoostNN * NewBoostNN(vint8 max_k);

  // This one calls ResampleTriples2.
  vBoostNN * NewBoostNN2();

  // this function is useful for training a decision tree
  // of distance measures
  vBoostNN * next_level(AdaBoost * splitter, float threshold);

  // Here, for each test object, we just take the winning class from 
  // each bnn in bnns, using 1-nn. Then, we find the class that
  // receives the most votes (i.e. that won in the most bnns). Note
  // that in order for this function to give the right result, we
  // must have called AnalyzeTestTriples for each object stored
  // in bnns.
  static vint8 Majority1(vector<vBoostNN *> * bnns);

  vint8 CleanUpClassifier();

  // here, in addition to index and alpha, we also specify z, so that
  // we don't have to spend any more time to compute z, when z
  // is already available.
  vint8 AddClassifier3(vint8 index, float alpha, float z);

  // here, we just pass the index
  vint8 AddClassifier(vint8 index);

  // Here dimensions (i.e. indices of attributes) can occur many times.
  vint8 Classifier(vector<vint8> * out_dimensions, vector<float> * out_weights);

  // Here we identify repetitions of the same dimension (i.e. the same
  // attribute, i.e. the same feature) and we combine them by adding
  // the weights associated with each occurrence.
  vint8 CleanClassifier(vector<vint8> * out_indices, vector<float> * out_weights);

  // add classifier corresponding to the attribute specified by
  // index, and with weight specified by alpha.
  vint8 AddClassifier(vint8 index, float alpha);

  // load a strong classifier from file
  vint8 Load(const char * filename);

  // save a strong classifier to file.
  vint8 Save(const char * filename);

  // Computes test_error, test_triple_error, test_confusion, and
  // test_triple_confusion. 
  float ComputeTestErrorSlow();
  float ComputeTestErrorSlow2();

  // For debugging: sum up the weights of all classifiers. 
  float SumClassifierWeights();

  // This function is called to regenerate triples, so that they are 
  // more focused (i.e. both a and b are close to q).
//  vMatrix<vint8> ResampleTriples(vint8 number, vint8 max_k);
  
  // This function uses ResampleTriples to create a new training and
  // validation set of triples. We must run the already chosen classifiers
  // on the new training triples, to set the training weights correctly.
  vint8 NewTrainingSet(vint8 max_k);

  // Undo the last steps. Right now it is not implemented entirely
  // correctly, in the sense that, instead of erasing the last steps,
  // it just adds the negation of those steps.
  vint8 Backtrack(vint8 steps);

    
  // Finds the attribute that achieves the smallest z, and chooses
  // the next weak classifier to depend on that attribute.
  virtual vint8 NextStep();

  // I don't know if this will work, but what this function does is
  // - compute the best factor for each of the coordinates,
  // - form a classifier that corresponds to the weighted sum
  // of all coordinates (based on these factors) 
  // - compute the factor (between zero and one) for this classifier
  // - add this classifier piecewise, by adding each of the individual
  // components.
  vint8 simultaneous_step();

  // return the number of training steps performed so far.
  virtual vint8 StepsDone();

  // Here triples are sampled deterministically: For each object, we
  // make a single triple consisting of the object, its same-class nn
  // and its other-class nn.
  virtual vMatrix<vint8> ResampleTriples3();

  // Here triples are also sampled deterministically. For each object,
  // we make as many triples as the number of other classes. Each triple
  // consists of the object, its same-class nn, and its nn from each
  // of the other classes.
  virtual vMatrix<vint8> ResampleTriples4();

  // Here we exclude training objects that have too many bad neighbors.
  virtual vMatrix<vint8> ResampleTriples5();

  virtual float CurrentDistanceWeight(vint8 distance_index);

  virtual vMatrix<float> ClassifierMatrix();

  // Prints some information about this object.
  virtual vint8 PrintSummary();

  virtual vint8 PrintClassifier();

  // Returns a matrix whose k-th entry is the k-nn nearest
  // neighbor accuracy, using L2 distance. Entry 0 is not meaningful.
  virtual vMatrix<float> KnnTrainingErrors2(vint8 max_k);

  // Same thing, but for the test error rate, which is faster to
  // compute because of test_distances_matrix;
  virtual vMatrix<float> KnnTestErrors2(vint8 max_k);

  // Finds first the attribute with the lowest weighted error, and 
  // adds that.
  virtual vint8 NextStepFast();

  // Here instead of passing in an object we just pass in an index into 
  // vectors_matrix. Typically vectors_matrix is the training vectors
  // or the test vectors.
  virtual vMatrix<float> TestDistances2(vint8 index, v3dMatrix<float> & vectors);

  // Similar to TestDistances2, but here we only compute distances to
  // objects that bevint8 to the class specified by class_id.
  virtual vMatrix<float> TestDistances3(vint8 index, v3dMatrix<float> & vectors,
                                         vint8 class_id);

  // this function should be called whenever we change the training triples,
  // for example with choose_second_triples or change_triples.
  // the input arguments are the new triples and the initial weights for 
  // those triples.  
  // The task of this function is to store those triples and weights, and also to
  // essentially clear out the existing
  // weak classifiers and then re-insert them, so that we compute the proper
  // updated weights for the new triples
  vint8 adjust_to_triples(vint8_matrix new_training_triples, float_matrix new_weights);


  // this function is useful and when we add classifiers with
  // prespecified factors, and then we want to adjust
  // all factors simultaneously by multiplying them with
  // a single constant.
  vint8 normalize_factors();
};


// vBnnqs is a version of Boost-NN that incorporates a query-sensitive
// distance measure.
class vBnnqs : public vBnn
{
protected:
  vector<vQsClassifier> step_classifiers;
  vector<vQsClassifier> unique_classifiers;

public:
  vBnnqs();

  vBnnqs(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vint8 in_training_number, vint8 in_validation_number);

  vBnnqs(vMatrix<float> in_training_vectors,
            vMatrix<vint8> in_training_labels,
            vMatrix<float> in_test_vectors,
            vMatrix<vint8> in_test_labels,
            vMatrix<vint8> in_training_triples,
            vMatrix<vint8> in_validation_triples);

  ~vBnnqs();


  // This function uses ResampleTriples to create training and validation
  // triples, which are used to initialize a new vBoostNN object.
  vBnnqs * NewBnnqs(vint8 max_k);

  // This one calls ResampleTriples2.
  vBnnqs * NewBnnqs2();

  // Here, for each test object, we just take the winning class from 
  // each bnn in bnns, using 1-nn. Then, we find the class that
  // receives the most votes (i.e. that won in the most bnns). Note
  // that in order for this function to give the right result, we
  // must have called AnalyzeTestTriples for each object stored
  // in bnns.
  //static vint8 Majority1(vector<vBoostNN *> * bnns);

  vint8 CleanUpClassifier();

  // here, in addition to index and alpha, we also specify z, so that
  // we don't have to spend any more time to compute z, when z
  // is already available.
  vint8 AddClassifier(vQsClassifier classifier, float z);

  // Here dimensions (i.e. indices of attributes) can occur many times.
  vint8 Classifier(vector<vQsClassifier> * out_classifier);

  // Here we identify repetitions of the same dimension (i.e. the same
  // attribute, i.e. the same feature) and we combine them by adding
  // the weights associated with each occurrence.
  vint8 CleanClassifier(vector<vQsClassifier> * out_classifier);

  // add classifier corresponding to the attribute specified by
  // index, and with weight specified by alpha.
  vint8 AddClassifier(vint8 index, float alpha);

  // load a strong classifier from file
  vint8 Load(const char * filename);

  // save a strong classifier to file.
  vint8 Save(const char * filename);

  // Computes test_error, test_triple_error, test_confusion, and
  // test_triple_confusion. 
  float ComputeTestErrorSlow();

  // Updates the entries of test_distances_matrix, to add 
  // distances based on the given classifier, weighted
  // by alpha.
  vint8 UpdateDistances(vQsClassifier classifier);
  
  //  float ComputeTestErrorSlow2();

  // For debugging: sum up the weights of all classifiers. 
//  float SumClassifierWeights();


  
  // Finds the attribute that achieves the smallest z, and chooses
  // the next weak classifier to depend on that attribute.
  virtual vint8 NextStep();

  // return the number of training steps performed so far.
  virtual vint8 StepsDone();

  // Here triples are sampled deterministically: For each object, we
  // make a single triple consisting of the object, its same-class nn
  // and its other-class nn.
//  virtual vMatrix<vint8> ResampleTriples3();

  // Here triples are also sampled deterministically. For each object,
  // we make as many triples as the number of other classes. Each triple
  // consists of the object, its same-class nn, and its nn from each
  // of the other classes.
//  virtual vMatrix<vint8> ResampleTriples4();

  // Here we exclude training objects that have too many bad neighbors.
//  virtual vMatrix<vint8> ResampleTriples5();

  virtual float CurrentDistanceWeight(vint8 distance_index);

//virtual vMatrix<float> ClassifierMatrix();

  // Prints some information about this object.
  virtual vint8 PrintSummary();

  virtual vint8 PrintClassifier();

  // Same thing, but for the test error rate, which is faster to
  // compute because of test_distances_matrix;
//  virtual vMatrix<float> KnnTestErrors2(vint8 max_k);


  // Here instead of passing in an object we just pass in an index into 
  // vectors_matrix. Typically vectors_matrix is the training vectors
  // or the test vectors.
  virtual vMatrix<float> TestDistances2(vint8 index, v3dMatrix<float> & vectors);

  // Similar to TestDistances2, but here we only compute distances to
  // objects that bevint8 to the class specified by class_id.
  virtual vMatrix<float> TestDistances3(vint8 index, v3dMatrix<float> & vectors,
                                         vint8 class_id);

  // Finds first the attribute with the lowest weighted error, and 
  // adds that.
//  virtual float NextStepFast();

  vint8 ClassifierResults(vQsClassifier classifier, vector<float> * results, 
                         vMatrix<vint8> triples_matrix);

  // This function is used for TestDistances2
  vMatrix<float> TestDistances3b(vint8 index, v3dMatrix<float> & vectors,
                                 vector<vQsClassifier> * classifiers);

  // This function is used for TestDistances3
  vMatrix<float> TestDistances4b(vint8 index, v3dMatrix<float> & vectors,
                                 vint8 class_id, vector<vQsClassifier> * classifiers);

  // Minimizes z for each classifier, and returns the best classifier, as well
  // as the z attained with that classifier.
  vQsClassifier FindBest(vector<vQsClassifier> * classifiers, float * z_pointer);

  float ClassifierZ(vQsClassifier classifier, float * alpha_pointer);

  // constructs random query-sensitive classifiers. Those classifiers can
  // be constructed by adding a multiplier
  // to a query-insensitive classifier,
  // or by adding a splitter to one of the classifiers in unique_classifiers.
  // The constructed classifiers go to the vector "classifiers". The argument
  // "number" specifies the number of classifiers we want to construct.
  vint8 MakeRandomClassifiers(vector<vQsClassifier> * classifiers, 
                             vint8 number);

  // constructs a single random query-sensitive classifier.
  vQsClassifier RandomClassifier();

  vMultiplier RandomMultiplier();

  // counts how many training objects are accepted by the multipliers
  // of some classifier.
  vint8 ClassifierRange(vQsClassifier classifier);

  // number simply specifies the number of query-sensitive classifiers to 
  // construct.
  vint8 NextStepQs(vint8 number);

  float CheckPair(vint8 test, vint8 training);

  float CheckPairTraining(vint8 training1, vint8 training2);
};


// this class will include the modifications to the Boost-NN formulation
// that I have made for a possible ICML 2006 submission.
class similarity_learning : public vBoostNN
{
public:
  similarity_learning();

  similarity_learning(vMatrix<float> in_training_vectors,
                      vMatrix<vint8> in_training_labels,
                      vMatrix<float> in_test_vectors,
                      vMatrix<vint8> in_test_labels,
                      vint8 in_training_number, vint8 in_validation_number);

  similarity_learning(vMatrix<float> in_training_vectors,
                      vMatrix<vint8> in_training_labels,
                      vMatrix<float> in_test_vectors,
                      vMatrix<vint8> in_test_labels,
                      vMatrix<vint8> in_training_triples,
                      vMatrix<vint8> in_validation_triples);

  ~similarity_learning();

  // this function is an auxiliary to set_second_triples, and
  // should be called for the first time that we want to
  // to choose triples based on the already computed distance measure
  vint8_matrix choose_second_triples(vint8 neighbor_number, std::vector<float> * weights);
  
  // this function should be called for the first time that we want to
  // to choose triples based on the already computed distance measure
  vint8 set_second_triples(vint8 neighbor_number);

  // discards objects with many "bad" b's.
  vint8_matrix choose_third_triples(vint8 neighbor_number, vint8 cutoff, std::vector<float> * weights);
  vint8 set_third_triples(vint8 neighbor_number, vint8 cutoff);

  // this function should be called at any point after choose_set_triple
  // has been called, to update the triples and possibly insert new triples.
  vint8_matrix change_triples(vint8 neighbor_number, std::vector<float> * weights);
  vint8 set_updated_triples(vint8 neighbor_number);


};


// calls the right destructor for the object, by checking 
// the class_name member variable
vint8 delete_boosted_similarity(vBnn * pointer);


#endif // VASSILIS_BOOST_NN_H
