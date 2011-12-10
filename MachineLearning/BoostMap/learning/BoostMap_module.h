#ifndef VASSILIS_BoostMap_MODULE_H
#define VASSILIS_BoostMap_MODULE_H

#include "basics/base_interpreter.h"
#include "learning/boost_kdd.h"
#include "learning/uci_learning.h"
#include "learning/boost_nn.h"
#include "learning/boosting.h"
#include "algorithms/vp_tree.h"
#include "algorithms/embeddings.h"
#include "algorithms/embedding_optimization.h"
#include "learning/refinement.h"

class class_BoostMap_module : public class_module
{
public:
  // the next two lines are mandatory for any module
  typedef vint8 (class_BoostMap_module::*type_procedure) ();
  std::vector<type_procedure> procedures;

  // BoostMap-related variables
  BoostMap_data * bm_data;
  class_BoostMap * bm;

  float_matrix embedding_classifier;
  float_matrix test_embedding;
  float_matrix database_embedding;

  // boost-nn related variables
  vUciDataset * uci_set;
  vBoostNN * bnn;
  vector<vBoostNN *> bnns;
  vBnnqs * bnnqs;
  vVpTree * vp_tree;
  AdaBoost * boosting;
  class_FastMap * fm;

  // other variables
  class_refinement * refinement;

  // bnn_max_z defines the value of z at which we stop the current
  // iteration of training of boost-nn.
  float bnn_max_z;
  vint8 bnn_allow_negative;
  vint8 bnn_ring_size;


public:

  //////////////////////////////////////////////////////////
  //                                                      //
  // start of obligatory member functions for any module  //
  //                                                      //
  //////////////////////////////////////////////////////////

  class_BoostMap_module(class_interpreter * interpreter);
  virtual ~class_BoostMap_module();
  
  virtual vint8 initialize();
  virtual vint8 clean_up();
  
  vint8 enter_command(char * command, type_procedure procedure);
  virtual vint8 process_command(const char * command);
  vint8 sort_commands ();
  virtual ushort process_key(char command) 
  {
    return 0;
  }
  virtual ushort process_mouse(vint8 button_down, vint8 row, vint8 col)
  {
    return 0;
  }

  //////////////////////////////////////////////////////////
  //                                                      //
  //  end of obligatory member functions for any module   //
  //                                                      //
  //////////////////////////////////////////////////////////

  // functions related to vBmDataset
  vint8 BmSetLoad0();
  vint8 BmSetLoad(const char * name);
  vint8 BmSetMake();
  
  // here we don't specify the ranges for candidates, training and validation,
  // we just specify how many objects should be used, and those objects are
  // chosen randomly.
  vint8 BmSetMake5();

  // BmSetMake2 was designed for the protein dataset.
  vint8 BmSetMake2();
  vint8 BmSetCandidates();
  vint8 BmSetTraining();
  vint8 BmSetValidation();
  vint8 BmSetPrint();
  vint8 BmSetDistance0();
  vint8 BmSetDistance(vint8 index1, vint8 index2);
  vint8 BmSetValidity();

  vint8 BmSetLabel0();
  vint8 BmSetLabel(const char * name, vint8 index);
  vint8 BmSetTestTrain0();
  vint8 BmSetTestTrain(const char * name, vint8 index1, vint8 index2);
  vint8 BmSetTrainTrain0();
  vint8 BmSetTrainTrain(const char * name, vint8 index1, vint8 index2);
  vint8 BmSetWknn0();
  vint8 BmSetWknn(const char * name, vint8 index, vint8 test_flag, vint8 k);
  vint8 BmSetKnn0();
  vint8 BmSetKnn(const char * name, vint8 max_k);
  vint8 BmSetKnnEmbedding0();
  vint8 BmSetKnnEmbedding(const char * dataset, const char * embedding,
                         vint8 dimensions, vint8 max_k);
  vint8 BmSetCascadeStats();
  vint8 BmSetCascadeStatsFr();
  vint8 BmSetSknnEmbedding0();
  vint8 BmSetSknnEmbedding(const char * dataset, const char * embedding,
                          vint8 dimensions, vint8 max_k);
  vint8 BmSetFilterRefine();
  vint8 BmSetIndexError0();
  vint8 BmSetIndexError(const char * dataset, const char * embedding,
                       vint8 dimensions);
  vint8 BmSetIndexErrors0();  
  vint8 BmSetDistanceBound();
  vint8 BmSetIndexErrors(const char * dataset, const char * embedding,
                        vint8 dimensions, vint8 max_k);
  vint8 BmSetIndexDistribution();

  // for the protein dataset
  vint8 BmSetIndexErrorsb();

  vint8 BmSetTrainingKnn0();
  vint8 BmSetTrainingKnn(const char * name, vint8 max_k);
  vint8 BmSetTrainingKnnEmbedding0();
  vint8 BmSetTrainingKnnEmbedding(const char * dataset, const char * embedding,
                         vint8 dimensions, vint8 max_k);

  vint8 BmSetEmbedTriple0();
  vint8 BmSetEmbedTriple(const char * dataset, const char * embedding,
                        vint8 dimensions, vint8 q, vint8 a, vint8 b, vint8 distance_p);

  // we get the top k nearest neighbors of a test object, and we print them.
  vint8 BmSetObjectKnns();
  
  // we get the top k nearest neighbors of a training object, and we print them.
  vint8 BmSetTrainingObjectKnns();

  // we get the rank of training object Y with respect to training object X, 
  // looking only at training objects within a specified range of the 
  // database
  vint8 BmSetCheckRank();

  // for a given test object and a given training object, we count
  // how many training objects are mapped closer to the training
  // object using a given reference object. We also print the 
  // closest objects (based on the embedding).
  vint8 BmSetCountCloser();

  vint8 BmSetMerge();
  vint8 BmSetSplit();


  // BoostMap-related functions
  vint8 BoostMap_data_load0();
  vint8 BoostMap_data_load(const char * name);
  vint8 BoostMap_data_make();
  
  // here we don't specify the ranges for candidates, training and validation,
  // we just specify how many objects should be used, and those objects are
  // chosen randomly.
  vint8 BoostMap_data_make5();

  // BoostMap_data_Make2 was designed for the protein dataset.
  vint8 BoostMap_data_make2();
  vint8 BoostMap_data_candidates();
  vint8 BoostMap_data_training();
  vint8 BoostMap_data_validation();
  vint8 BoostMap_data_print();
  vint8 BoostMap_data_distance0();
  vint8 BoostMap_data_distance(vint8 index1, vint8 index2);
  vint8 BoostMap_data_validity();

  vint8 BoostMap_data_label0();
  vint8 BoostMap_data_label(const char * name, vint8 index);
  vint8 BoostMap_data_test_train_distance0();
  vint8 BoostMap_data_test_train_distance(const char * name, vint8 index1, vint8 index2);
  vint8 BoostMap_data_train_train_distance0();
  vint8 BoostMap_data_train_train_distance(const char * name, vint8 index1, vint8 index2);
  vint8 BoostMap_data_wknn0();
  vint8 BoostMap_data_wknn(const char * name, vint8 index, vint8 test_flag, vint8 k);
  vint8 BoostMap_data_knn0();
  vint8 BoostMap_data_knn(const char * name, vint8 max_k);
  vint8 BoostMap_data_knn_embedding0();
  vint8 BoostMap_data_knn_embedding(const char * dataset, const char * embedding,
                         vint8 dimensions, vint8 max_k);
  vint8 BoostMap_data_cascade_statistics();
  vint8 BoostMap_data_cascade_statistics_fr();
  vint8 BoostMap_data_sknn_embedding0();
  vint8 BoostMap_data_sknn_embedding(const char * dataset, const char * embedding,
                          vint8 dimensions, vint8 max_k);
  vint8 BoostMap_data_filter_refine();
  vint8 BoostMap_data_index_error0();
  vint8 BoostMap_data_index_error(const char * dataset, const char * embedding,
                       vint8 dimensions);
  vint8 BoostMap_data_index_errors0();
  vint8 BoostMap_data_index_errors(const char * dataset, const char * embedding,
                        vint8 dimensions, vint8 max_k);

  // for the protein dataset
  vint8 BoostMap_data_index_errorsb();

  vint8 BoostMap_data_training_knn0();
  vint8 BoostMap_data_training_knn(const char * name, vint8 max_k);
  vint8 BoostMap_data_training_knn_embedding0();
  vint8 BoostMap_data_training_knn_embedding(const char * dataset, const char * embedding,
                         vint8 dimensions, vint8 max_k);

  vint8 BoostMap_data_save_embeddings();
  vint8 BoostMap_data_save_embeddings(const char * dataset, const char * embedding);
  vint8 BoostMap_data_test_triple0();
  vint8 BoostMap_data_test_triple(const char * dataset, vint8 dimensions, vint8 q, vint8 a, vint8 b, vint8 distance_p);
  vint8 BoostMap_data_embed_triple0();
  vint8 BoostMap_data_embed_triple(const char * dataset, const char * embedding,
                        vint8 dimensions, vint8 q, vint8 a, vint8 b, vint8 distance_p);

  // we get the top k nearest neighbors of a test object, and we print them.
  vint8 BoostMap_data_object_knns();
  
  // we get the top k nearest neighbors of a training object, and we print them.
  vint8 BoostMap_data_training_object_knns();

  // we get the rank of training object Y with respect to training object X, 
  // looking only at training objects within a specified range of the 
  // database
  vint8 BoostMap_data_check_rank();

  // for a given test object and a given training object, we count
  // how many training objects are mapped closer to the training
  // object using a given reference object. We also print the 
  // closest objects (based on the embedding).
  vint8 BoostMap_data_count_closer();

  vint8 BoostMap_data_merge();
  vint8 BoostMap_data_split();

  // start of BoostMap_data functions for SIGMOD 2007

  // make a smaller data set to speed up testing and debugging
  vint8 BoostMap_data_make_smaller();

  // make a smaller data set to speed up testing and debugging
  // here, we keep the full test set and a random sample of training (database) data
  vint8 BoostMap_data_random_subdataset();

  // save test_train and train_train embedding-based distances 

  vint8 BoostMap_data_embedding_distances();
  vint8 BoostMap_data_test_train();
  vint8 BoostMap_data_train_train();

  // equivalent to BoostMap_data_index_errors, but using a reimplementation.
  vint8 BoostMap_data_retrieval();

  // equivalent to BoostMap_data_retrieval(), but here the query set is
  // the set of database objects
  vint8 BoostMap_data_retrieval_training();

  // useful for printing results from indexerrors other similar functions
  vint8 print_retrieval_results(vint8_matrix results);

  // end of BoostMap_data functions for SIGMOD 2007

  vint8 BmNew0();
  vint8 BmNew(const char * name, vint8 number_of_triples, vint8 use_pdistances);
  vint8 BmNew20();
  vint8 BmNew2(const char * name, vint8 number_of_triples, vint8 use_pdistances,
              vint8 classes, vint8 max_k);
  vint8 BmNew30();
  vint8 BmNew3(const char * name, vint8 number_of_triples, vint8 use_pdistances,
              vint8 classes, vint8 max_k);

  vint8 BmNew4();
  vint8 BmNew5();
  vint8 BmNewLoad();
  vint8 BmDelete();
  vint8 BmPrintSummary();
  vint8 BmPrintClassifier();
  vint8 BmPrintAll();
  vint8 BmStep0();
  vint8 BmStep(const char * filename);
  vint8 BmSteps0();
  vint8 BmSteps(char * filename, vint8 steps);
  vint8 BmRandom0();
  vint8 BmRandom(vint8 number, vint8 times);
  vint8 BmAllowNegative0();
  vint8 BmAllowNegative(vint8 value);
  vint8 BmAllowRemovals0();
  vint8 BmAllowRemovals(vint8 value);
  vint8 BmAllowLipschitz0();
  vint8 BmAllowLipschitz(vint8 value);
  vint8 BmAllowProjections0();
  vint8 BmAllowProjections(vint8 value);
  vint8 BmAllowSensitive0();
  vint8 BmAllowSensitive(vint8 value1, vint8 value2);
  vint8 BmPickedCandidates0();
  vint8 BmPickedCandidates(vint8 value);
  vint8 BmLoad0();
  vint8 BmLoad(const char * filename, vint8 dimensions);
  vint8 BmLoad2();
  vint8 BmLoadQs();
  vint8 BmSave0();
  vint8 BmSave(const char * filename);
  vint8 BmSelectFirst0();
  vint8 BmSelectFirst(const char * filename, vint8 dimensions);

  vint8 BmTestTriple0();
  vint8 BmTestTriple(vint8 index);
  vint8 BmTrainingTriples0();
  vint8 BmTrainingTriples(vint8 start, vint8 end);
  vint8 BmValidationTriples0();
  vint8 BmValidationTriples(vint8 start, vint8 end);
  vint8 BmTripleEmbeddings0();
  vint8 BmTripleEmbeddings(vint8 index);
  vint8 BmTripleResults0();
  vint8 BmTripleResults(vint8 index);

  vint8 BmSensitiveStep0();
  vint8 BmSensitiveStep(const char * filename);
  vint8 BmSensitiveSteps0();
  vint8 BmSensitiveSteps(char * filename, vint8 steps);
  vint8 BmFastSensitiveStep0();
  vint8 BmFastSensitiveStep(const char * filename);
  vint8 BmFastSensitiveSteps0();
  vint8 BmFastSensitiveSteps(char * filename, vint8 steps);

  vint8 BmAppendDimensions0();
  vint8 BmAppendDimensions(char * filename, vint8 dimensions);
  vint8 BmClassifierStatistics0();
  vint8 BmClassifierStatistics(vint8 type, vint8 object1, vint8 object2);
  vint8 BmBestK0();
  vint8 BmBestK(vint8 in_k);

  vint8 BmDistanceP();
  vint8 BmAddReference();
  vint8 BmAddRange();
  vint8 BmWeightedReference();
  vint8 BmWeightedRange();
  vint8 BmRandomReferences();
  vint8 BmRandomProjections();

  vint8 BmSaveData();
  vint8 BmStep2();
  vint8 BmStep2(const char * filename);
  vint8 BmSteps2();

  vint8 BoostMap_small_triples();
  vint8 BoostMap_use_small();
  vint8 BoostMap_use_large();
  vint8 BoostMap_factor_totals();
  vint8 BoostMap_distribution();
  vint8 BoostMap_class_total();

  char * EmbeddingDirectory2();
  
  
  // Boost-NN related functions
  // Beginning of functions written for submission to ICML 2004.
  vint8 TestSum0();
  vint8 TestSum(vint8 rows, vint8 cols, double min, double max);
  vint8 TestMean0();
  vint8 TestMean(vint8 rows, vint8 cols, double min, double max);
  vint8 TestVariance0();
  vint8 TestVariance(vint8 rows, vint8 cols, double min, double max);
  vint8 TestStd0();
  vint8 TestStd(vint8 rows, vint8 cols, double min, double max);

  // Test the function vPermutation
  vint8 Permutation0();
  vint8 Permutation(vint8 start, vint8 end);

  // Load a UCI dataset.
  vint8 UciLoad0();
  vint8 UciLoad(vint8 number);

  // For debugging
  vint8 UciPrint();
  vint8 UciRow0();
  vint8 UciRow(vint8 row);
  vint8 UciSizes();
  vint8 UciTrainingObject0();
  vint8 UciTrainingObject(vint8 class_id, vint8 index);
  vint8 UciIds();
  vint8 UciDatasets();
  vint8 UciIndices0();
  vint8 UciIndices(vint8 class_id);

  // Randomly split objects into training and test set.
  vint8 UciSample0();
  vint8 UciSample(float fraction);
  vint8 UciSample20();
  vint8 UciSample2(float fraction);

  vint8 BNN_Create0();
  vint8 BNN_Create(vint8 training, vint8 validation);
  vint8 BNN_delete();
  vint8 BNN_PrintSummary();
  vint8 BNN_PrintTraining();
  vint8 BNN_PrintValidation();
  vint8 BNN_TestTriples();
  vint8 BNN_TripleStats();
  vint8 BNN_Confusions();
  vint8 BNN_Classifier();
  vint8 BNN_Triple0();
  vint8 BNN_Triple(vint8 index);
  vint8 BNN_Weight0();
  vint8 BNN_Weight(vint8 rank);
  vint8 BNN_Weights0();
  vint8 BNN_Weights(vint8 period);
  vint8 BNN_SomeWeights0();
  vint8 BNN_SomeWeights(vint8 start, vint8 end, vint8 period);
  vint8 BNN_WeightSum0();
  vint8 BNN_WeightSum(vint8 start, vint8 end);
  vint8 BNN_SortWeights();
  vint8 BNN_ZContributions0();
  vint8 BNN_ZContributions(vint8 index);
  vint8 BNN_KnnErrors0();
  vint8 BNN_KnnErrors(vint8 k);
  vint8 BNN_RankInfo0();
  vint8 BNN_RankInfo(vint8 triple_index);
  vint8 BNN_KnnInfo0();
  vint8 BNN_KnnInfo(vint8 index, vint8 k);

  vint8 BNN_Step0();
  vint8 BNN_Step(const char * filename);
  vint8 BNN_Steps0();
  vint8 BNN_Steps(char * filename, vint8 steps);
  vint8 BNN_NewTraining();
  vint8 BNN_NewBNN0();
  vint8 BNN_NewBNN(vint8 max_k);
  vint8 BNN_NewBNN2();
  vint8 BNN_Backtrack0();
  vint8 BNN_Backtrack(vint8 steps);
  vint8 BNN_Load0();
  vint8 BNN_Load(const char * filename);
  vint8 BNN_Save0();
  vint8 BNN_Save(const char * filename);
  vint8 BNN_AllowNegative0();
  vint8 BNN_AllowNegative(vint8 value);

  vint8 BNN_Train0();
  float BNN_Train(const char * filename, vint8 training);
  float BNN_Train_old(const char * filename, vint8 training);
  vint8 BNN_Train20();
  float BNN_Train2(vint8 training);
  vint8 BNN_Train30();
  float BNN_Train3(vint8 training);
  vint8 BNN_TrainLoop0();
  vint8 BNN_TrainLoop(const char * filename, vint8 dataset, vint8 training);
  vint8 BNN_TrainLoop20();
  vint8 BNN_TrainLoop2(const char * filename, vint8 dataset, 
                      vint8 training, float fraction);

  // An initial attempt to cross-validate, now obsolete.
  vint8 BNN_CrossValidateBad0();
  vint8 BNN_CrossValidateBad(float fraction, vint8 times, vint8 training);
  
  // The right way to cross-validate.
  vint8 BNN_CrossValidate0();
  vint8 BNN_CrossValidate(vint8 number_of_sets, vint8 training);
  vint8 BNN_MaxZ0();
  vint8 BNN_MaxZ(float new_max_z);
  vint8 BNN_AddClassifier0();
  vint8 BNN_AddClassifier(vint8 index, float weight);
  vint8 BNN_AddClassifiers0();
  vint8 BNN_AddClassifiers(vint8 start, vint8 end, float weight);
  vint8 BNN_Select0();
  vint8 BNN_Select(vint8 index);
  vint8 BNN_PrintAll();
  vint8 BNN_EraseAll();
  vint8 BNN_Number();
  vint8 BNN_AnalyzeTriples();
  vint8 BNN_AnalyzeTestTriples();
  vint8 BNN_BadTriples();
  vint8 BNN_BadTriples20();
  vint8 BNN_BadTriples2(vint8 start, vint8 end);
  vint8 BNN_BadNumberHistogram();
  vint8 BNN_QNumbers0();
  vint8 BNN_QNumbers(vint8 q);
  vint8 BNN_QNumbers20();
  vint8 BNN_QNumbers2(vint8 start, vint8 end);
  vint8 BNN_RingSize0();
  vint8 BNN_RingSize(vint8 value);
  vint8 BNN_AddToRing();
  vint8 BNN_Majority10();
  vint8 BNN_Majority1(vint8 start, vint8 end);

  vint8 BNN_Ranges();
  vint8 BNN_Stds();
  vint8 BNN_NormalizeRanges();
  vint8 BNN_NormalizeRanges2();
  vint8 BNN_NormalizeStds();
  vint8 BNN_Square();
  vint8 BNN_KNNTraining0();
  vint8 BNN_KNNTraining(vint8 max_k);
  vint8 BNN_KNNTest0();
  vint8 BNN_KNNTest(vint8 max_k);

  // Compute the naive k-nn training and test errors for all
  // six variations (unnormalized, normalizing ranges, 
  // normalizing stds, and also L1 or Euclidean).
  vint8 BNN_NaiveKnn0();
  vMatrix<float> BNN_NaiveKnn(vint8 dataset, vint8 max_k);
  // Compute the cross-validation k-nn training and test errors
  // for all six variations.
  vint8 BNN_NaiveKnnCross0();
  vMatrix<float> BNN_NaiveKnnCross(vint8 dataset, vint8 number, vint8 max_k);

  // End of functions written for submission to ICML 2004.

  vint8 BnnqsCreate0();
  vint8 BnnqsCreate(vint8 training, vint8 validation);
  vint8 BnnqsPrintSummary();
  vint8 BnnqsPrintTraining();
  vint8 BnnqsPrintValidation();
  vint8 BnnqsTestTriples();
  vint8 BnnqsTripleStats();
  vint8 BnnqsConfusions();
  vint8 BnnqsClassifier();
  vint8 BnnqsTriple();
  vint8 BnnqsWeight();
  vint8 BnnqsWeights();
  vint8 BnnqsSomeWeights();
  vint8 BnnqsWeightSum();
  vint8 BnnqsSortWeights();
  vint8 BnnqsZContributions();
  vint8 BnnqsKnnErrors();
  vint8 BnnqsRankInfo();
  vint8 BnnqsKnnInfo();

  vint8 BnnqsStep0();
  vint8 BnnqsStep(const char * buffer);
  vint8 BnnqsSteps();
  vint8 BnnqsNewBNN0();
  vint8 BnnqsNewBNN(vint8 max_k);
  vint8 BnnqsNewBNN2();
//  vint8 BnnqsBacktrack();
  vint8 BnnqsLoad();
  vint8 BnnqsSave();
  vint8 BnnqsAllowNegative();

  vint8 BnnqsTrain0();
  float BnnqsTrain(vint8 triples);
  vint8 BnnqsTrain2();
  vint8 BnnqsTrain3();
  vint8 BnnqsTrainLoop();
  vint8 BnnqsTrainLoop2();

  // The right way to cross-validate.
  vint8 BnnqsCrossValidate();
  vint8 BnnqsAddClassifier();
  vint8 BnnqsAddClassifiers0();
  vint8 BnnqsAddClassifiers(vint8 start, vint8 end, float weight);
  vint8 BnnqsAnalyzeTriples();
  vint8 BnnqsAnalyzeTestTriples();
  vint8 BnnqsBadTriples();
  vint8 BnnqsBadTriples2();
  vint8 BnnqsBadNumberHistogram();
  vint8 BnnqsQNumbers();
  vint8 BnnqsQNumbers2();

  vint8 BnnqsRanges();
  vint8 BnnqsStds();
  vint8 BnnqsNormalizeRanges();
  vint8 BnnqsNormalizeRanges2();
  vint8 BnnqsNormalizeStds();
  vint8 BnnqsKNNTraining();
  vint8 BnnqsKNNTest();

  vint8 BnnqsStepqs0();
  vint8 BnnqsStepqs(vint8 number);
  vint8 BnnqsStepsqs();
  vint8 BnnqsCheck();
  vint8 BnnqsCheckTr();

  vint8 VpNew();
  vint8 VpDelete();
  vint8 VpPrint();
  vint8 VpLoad();
  vint8 VpSave();
  vint8 VpTestRange();
  vint8 VpTestNn();
  vint8 VpTestKnn();
  vint8 VpNnStats();
  vint8 VpKnnStats();
  vint8 VpKnnClassification();

  vint8 BoostMap_load_embeddings ();
  vint8 BoostMap_clear_embeddings ();

  vint8 similarity_new();
  // this function is called whenever we want to verify that there is a current
  // boost-NN object whose actual subclass is similarity_learning.
  vint8 similarity_learning_check();
  vint8 similarity_change_triples();
  vint8 similarity_change_triples_auxiliary(vint8 neighbor_number);
  vint8 similarity_third_triples();
  vint8 similarity_third_triples_auxiliary(vint8 neighbor_number, vint8 cutoff);
  vint8 similarity_train();
  float similarity_train_auxiliary(vint8 neighbor_number, vint8 cutoff);
  vint8 similarity_cross_validate();
  vint8 similarity_cross_validate_auxiliary(vint8 number_of_sets, vint8 neighbor_number, vint8 cutoff);

  // continues training
  vint8 similarity_train_first();
  vint8 similarity_train_first_auxiliary(vint8 neighbor_number, vint8 cutoff);
  vint8 similarity_normalize_factors();
  vint8 similarity_simultaneous_step();
  vint8 similarity_simultaneous_step(const char * filename);

  vint8 similarity_AdaBoost_create();
  vint8 similarity_next_level();

  vint8 AdaBoost_synthetic();
  vint8 AdaBoost_destroy();
  vint8 AdaBoost_print();
  vint8 AdaBoost_errors();
  vint8 AdaBoost_factors();
  vint8 AdaBoost_save();
  vint8 AdaBoost_save(const char * filename);
  vint8 AdaBoost_load();
  vint8 AdaBoost_load(const char * filename);
  vint8 AdaBoost_step();
  vint8 AdaBoost_step(const char * filename);
  vint8 AdaBoost_steps();
  vint8 AdaBoost_steps(const char * filename, vint8 steps);
  vint8 AdaBoost_train();
  vint8 AdaBoost_train(const char * filename);

  // find the threshold for a given fraction (with respect to all test objects)
  // of objects with negative labels that get misclassified
  vint8 AdaBoost_threshold_test();

  // find the threshold for a given fraction (with respect to all training objects)
  // of objects with negative labels that get misclassified
  vint8 AdaBoost_threshold_training();

  char * EmbeddingDirectory();
  vint8 FastMapFilterRefine();
  vint8 FastMapKnn();
  vint8 FastMapIndexErrors();
  vint8 FmMake20();
  vint8 FmMake2(const char * dataset, const char * sample_name);
  vint8 FmMake30();
  vint8 FmMake3(const char * dataset, const char * sample_name,
               const char * saved_file);
  vint8 FmDelete();

  vint8 FmNextStep();
  vint8 FmNextSteps0();
  vint8 FmNextSteps(vint8 steps);
  vint8 FmEmbedTest();
  vint8 FmEmbedTraining();
  vint8 FmSave0();
  vint8 FmSave(const char * filename);
  vint8 FmPrintSample0();
  vint8 FmPrintSample(vint8 i);
  vint8 FmPrintTraining0();
  vint8 FmPrintTraining(vint8 i);
  vint8 FmPrintPivots();

  // checks if the current BoostMap object is an embedding_optimizer object.
  embedding_optimizer * optimizer_check();
  vint8 optimizer_new();
  vint8 optimizer_threshold_number();

  vint8 optimizer_stress_step();
  vint8 optimizer_stress_step(const char * filename);
  vint8 optimizer_stress_steps();
  vint8 optimizer_stress_steps(char * filename, vint8 steps);

  vint8 optimizer_distortion_step();
  vint8 optimizer_distortion_step(const char * filename);
  vint8 optimizer_distortion_steps();
  vint8 optimizer_distortion_steps(char * filename, vint8 steps);
  vint8 optimizer_test_magnification();
  vint8 optimizer_magnification_step();
  vint8 optimizer_magnification_steps();

  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////
  ///////////////////////////////////////////////////

  // start of functions dealing with discriminative predictor models
  // of embedding-based filter-and-refine retrieval accuracy
  vint8 refinement_new();
  vint8 refinement_delete();
  vint8 refinement_train_step();
  vint8 refinement_train_more();
  vint8 refinement_save();
  vint8 refinement_load();
  vint8 refinement_AdaBoost();
  vint8 refinement_results();
  vint8 refinement_threshold();
  vint8 refinement_print();
  vint8 refinement_AdaBoost_load();
};











#endif   // VASSILIS_BoostMap_MODULE_H
