#include "BoostMap_module.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "algorithms/embedding_optimization.h"
#include "basics/local_data.h"

#include "basics/definitions.h"


//////////////////////////////////////////////////////////
//                                                      //
// start of obligatory member functions for any module  //
//                                                      //
//////////////////////////////////////////////////////////


class_BoostMap_module::class_BoostMap_module(class_interpreter * interpreter) :
class_module(interpreter)
{
	initialize();
	sort_commands();
}


class_BoostMap_module::~class_BoostMap_module()
{
	class_module::clean_up();
	clean_up();
}


vint8 class_BoostMap_module::clean_up()
{
	vdelete(bm_data);
	BmDelete();
	vdelete(uci_set);

	BNN_EraseAll();
	vdelete(bnnqs);
	vdelete(vp_tree);
	function_delete(fm);
	AdaBoost_destroy();

	function_delete(refinement);

	return 1;
}


vint8 class_BoostMap_module::initialize()
{
	bm_data = 0;
	bm = 0;

	uci_set = 0;
	bnn = 0;
	bnnqs = 0;

	bnn_max_z = (float) .9999;
	bnn_allow_negative = 0;
	bnn_ring_size = 5;

	vp_tree = 0;
	boosting = 0;
	fm = 0;
	refinement = 0;

	// If we are not using the interpreter, we should return right now
	if (base_interpreter == NULL)
		return 1;

	enter_command("AdaBoost_synthetic", &class_BoostMap_module::AdaBoost_synthetic);
	enter_command("AdaBoost_destroy", &class_BoostMap_module::AdaBoost_destroy);
	enter_command("AdaBoost_print", &class_BoostMap_module::AdaBoost_print);
	enter_command("AdaBoost_errors", &class_BoostMap_module::AdaBoost_errors);
	enter_command("AdaBoost_factors", &class_BoostMap_module::AdaBoost_factors);
	enter_command("AdaBoost_save", &class_BoostMap_module::AdaBoost_save);
	enter_command("AdaBoost_load", &class_BoostMap_module::AdaBoost_load);
	enter_command("AdaBoost_step", &class_BoostMap_module::AdaBoost_step);
	enter_command("AdaBoost_steps", &class_BoostMap_module::AdaBoost_steps);
	enter_command("AdaBoost_threshold_test", &class_BoostMap_module::AdaBoost_threshold_test);
	enter_command("AdaBoost_threshold_training", &class_BoostMap_module::AdaBoost_threshold_training);
	enter_command("AdaBoost_train", &class_BoostMap_module::AdaBoost_train);

	enter_command("bmadd_range", &class_BoostMap_module::BmAddRange);
	enter_command("bmadd_reference", &class_BoostMap_module::BmAddReference);
	enter_command("bmadd_wrange", &class_BoostMap_module::BmWeightedRange);
	enter_command("bmadd_wreference", &class_BoostMap_module::BmWeightedReference);
	enter_command("bmallow_lipschitz", &class_BoostMap_module::BmAllowLipschitz0);
	enter_command("bmallow_negative", &class_BoostMap_module::BmAllowNegative0);
	enter_command("bmallow_projections", &class_BoostMap_module::BmAllowProjections0);
	enter_command("bmallow_removals", &class_BoostMap_module::BmAllowRemovals0);
	enter_command("bmallow_specialized", &class_BoostMap_module::BmAllowSensitive0);
	enter_command("bmallow_sensitive", &class_BoostMap_module::BmAllowSensitive0);
	enter_command("bmappend_dimensions", &class_BoostMap_module::BmAppendDimensions0);
	enter_command("bmbest_k", &class_BoostMap_module::BmBestK0);
	enter_command("bmclassifier_stats", &class_BoostMap_module::BmClassifierStatistics0);
	enter_command("bmdelete", &class_BoostMap_module::BmDelete);
	enter_command("bmdistancep", &class_BoostMap_module::BmDistanceP);
	enter_command("bmload", &class_BoostMap_module::BmLoad0);
	enter_command("bmload2", &class_BoostMap_module::BmLoad2);
	enter_command("bmload_qs", &class_BoostMap_module::BmLoadQs);
	enter_command("bmnew", &class_BoostMap_module::BmNew0);
	enter_command("bmnew2", &class_BoostMap_module::BmNew20);
	enter_command("bmnew3", &class_BoostMap_module::BmNew30);
	enter_command("bmnew4", &class_BoostMap_module::BmNew4);
	enter_command("bmnew5", &class_BoostMap_module::BmNew5);
	enter_command("bmnew_load", &class_BoostMap_module::BmNewLoad);
	enter_command("bmpicked", &class_BoostMap_module::BmPickedCandidates0);
	enter_command("bmprint_classifier", &class_BoostMap_module::BmPrintClassifier);
	enter_command("bmprint_summary", &class_BoostMap_module::BmPrintSummary);
	enter_command("bmprint_all", &class_BoostMap_module::BmPrintAll);
	enter_command("bmrandom", &class_BoostMap_module::BmRandom0);
	enter_command("bmrandom_projections", &class_BoostMap_module::BmRandomProjections);
	enter_command("bmrandom_references", &class_BoostMap_module::BmRandomReferences);
	enter_command("bmsave", &class_BoostMap_module::BmSave0);
	enter_command("bmsave_data", &class_BoostMap_module::BmSaveData);
	enter_command("bmselect", &class_BoostMap_module::BmSelectFirst0);
	enter_command("bmsfast", &class_BoostMap_module::BmFastSensitiveStep0);
	enter_command("bmsfasts", &class_BoostMap_module::BmFastSensitiveSteps0);
	enter_command("bmstep", &class_BoostMap_module::BmStep0);
	enter_command("bmsteps", &class_BoostMap_module::BmSteps0);
	enter_command("bmstep2", &class_BoostMap_module::BmStep2);
	enter_command("bmsteps2", &class_BoostMap_module::BmSteps2);
	enter_command("bmsstep", &class_BoostMap_module::BmSensitiveStep0);
	enter_command("bmssteps", &class_BoostMap_module::BmSensitiveSteps0);
	enter_command("bmtest_triple", &class_BoostMap_module::BmTestTriple0);
	enter_command("bmtraining_triples", &class_BoostMap_module::BmTrainingTriples0);
	enter_command("bmtriple_embeddings", &class_BoostMap_module::BmTripleEmbeddings0);
	enter_command("bmtriple_results", &class_BoostMap_module::BmTripleResults0);
	enter_command("bmvalidation_triples", &class_BoostMap_module::BmValidationTriples0);

	enter_command("BoostMap_add_range", &class_BoostMap_module::BmAddRange);
	enter_command("BoostMap_add_reference", &class_BoostMap_module::BmAddReference);
	enter_command("BoostMap_add_factored_range", &class_BoostMap_module::BmWeightedRange);
	enter_command("BoostMap_add_factored_reference", &class_BoostMap_module::BmWeightedReference);
	enter_command("BoostMap_allow_lipschitz", &class_BoostMap_module::BmAllowLipschitz0);
	enter_command("BoostMap_allow_negative", &class_BoostMap_module::BmAllowNegative0);
	enter_command("BoostMap_allow_projections", &class_BoostMap_module::BmAllowProjections0);
	enter_command("BoostMap_allow_removals", &class_BoostMap_module::BmAllowRemovals0);
	enter_command("BoostMap_allow_sensitive", &class_BoostMap_module::BmAllowSensitive0);
	enter_command("BoostMap_append_dimensions", &class_BoostMap_module::BmAppendDimensions0);
	enter_command("BoostMap_best_k", &class_BoostMap_module::BmBestK0);
	enter_command("BoostMap_classifier_stats", &class_BoostMap_module::BmClassifierStatistics0);
	enter_command("BoostMap_class_total", &class_BoostMap_module::BoostMap_class_total);
	enter_command("BoostMap_delete", &class_BoostMap_module::BmDelete);
	enter_command("BoostMap_distance_p", &class_BoostMap_module::BmDistanceP);
	enter_command("BoostMap_distribution", &class_BoostMap_module::BoostMap_distribution);
	enter_command("BoostMap_factor_totals", &class_BoostMap_module::BoostMap_factor_totals);
	enter_command("BoostMap_load", &class_BoostMap_module::BmLoad0);
	enter_command("BoostMap_load2", &class_BoostMap_module::BmLoad2);
	enter_command("BoostMap_load_sensitive", &class_BoostMap_module::BmLoadQs);
	enter_command("BoostMap_new", &class_BoostMap_module::BmNew0);
	enter_command("BoostMap_new2", &class_BoostMap_module::BmNew20);
	enter_command("BoostMap_new3", &class_BoostMap_module::BmNew30);
	enter_command("BoostMap_new4", &class_BoostMap_module::BmNew4);
	enter_command("BoostMap_new5", &class_BoostMap_module::BmNew5);
	enter_command("BoostMap_new_load", &class_BoostMap_module::BmNewLoad);
	enter_command("BoostMap_picked", &class_BoostMap_module::BmPickedCandidates0);
	enter_command("BoostMap_print_classifier", &class_BoostMap_module::BmPrintClassifier);
	enter_command("BoostMap_print_summary", &class_BoostMap_module::BmPrintSummary);
	enter_command("BoostMap_print_all", &class_BoostMap_module::BmPrintAll);
	enter_command("BoostMap_random", &class_BoostMap_module::BmRandom0);
	enter_command("BoostMap_random_projections", &class_BoostMap_module::BmRandomProjections);
	enter_command("BoostMap_random_references", &class_BoostMap_module::BmRandomReferences);
	enter_command("BoostMap_save", &class_BoostMap_module::BmSave0);
	enter_command("BoostMap_save_data", &class_BoostMap_module::BmSaveData);
	enter_command("BoostMap_select", &class_BoostMap_module::BmSelectFirst0);
	enter_command("BoostMap_sensitive_fast_step", &class_BoostMap_module::BmFastSensitiveStep0);
	enter_command("BoostMap_sensitive_fast", &class_BoostMap_module::BmFastSensitiveSteps0);
	enter_command("BoostMap_small_triples", &class_BoostMap_module::BoostMap_small_triples);
	enter_command("BoostMap_step", &class_BoostMap_module::BmStep0);
	enter_command("BoostMap_steps", &class_BoostMap_module::BmSteps0);
	enter_command("BoostMap_step2", &class_BoostMap_module::BmStep2);
	enter_command("BoostMap_steps2", &class_BoostMap_module::BmSteps2);
	enter_command("BoostMap_sensitive_step", &class_BoostMap_module::BmSensitiveStep0);
	enter_command("BoostMap_sensitive_steps", &class_BoostMap_module::BmSensitiveSteps0);
	enter_command("BoostMap_test_triple", &class_BoostMap_module::BmTestTriple0);
	enter_command("BoostMap_training_triples", &class_BoostMap_module::BmTrainingTriples0);
	enter_command("BoostMap_triple_embeddings", &class_BoostMap_module::BmTripleEmbeddings0);
	enter_command("BoostMap_triple_results", &class_BoostMap_module::BmTripleResults0);
	enter_command("BoostMap_use_small", &class_BoostMap_module::BoostMap_use_small);
	enter_command("BoostMap_use_large", &class_BoostMap_module::BoostMap_use_large);
	enter_command("BoostMap_validation_triples", &class_BoostMap_module::BmValidationTriples0);

	enter_command("bmset_candidates", &class_BoostMap_module::BmSetCandidates);
	enter_command("bmset_cascade_stats_old", &class_BoostMap_module::BmSetCascadeStats);
	enter_command("bmset_cascade_stats", &class_BoostMap_module::BmSetCascadeStatsFr);
	enter_command("bmset_check_rank", &class_BoostMap_module::BmSetCheckRank);
	enter_command("bmset_count_closer", &class_BoostMap_module::BmSetCountCloser);
	enter_command("bmset_distance", &class_BoostMap_module::BmSetDistance0);
	enter_command("bmset_distance_bound", &class_BoostMap_module::BmSetDistanceBound);
	enter_command("bmset_embed_triple", &class_BoostMap_module::BmSetEmbedTriple0);
	enter_command("bmset_filter_refine", &class_BoostMap_module::BmSetFilterRefine);
	enter_command("bmset_index_error", &class_BoostMap_module::BmSetIndexError0);
	enter_command("bmset_index_errors", &class_BoostMap_module::BmSetIndexErrors0);
	enter_command("bmset_index_errors_distribution", &class_BoostMap_module::BmSetIndexDistribution);
	enter_command("bmset_index_errorsb", &class_BoostMap_module::BmSetIndexErrorsb);
	enter_command("bmset_knn", &class_BoostMap_module::BmSetKnn0);
	enter_command("bmset_knn_embedding", &class_BoostMap_module::BmSetKnnEmbedding0);
	enter_command("bmset_label", &class_BoostMap_module::BmSetLabel0);
	enter_command("bmset_load", &class_BoostMap_module::BmSetLoad0);
	enter_command("bmset_make", &class_BoostMap_module::BmSetMake);
	enter_command("bmset_make5", &class_BoostMap_module::BmSetMake5);
	enter_command("bmset_make2", &class_BoostMap_module::BmSetMake2);
	enter_command("bmset_merge", &class_BoostMap_module::BmSetMerge);
	enter_command("bmset_object_knns", &class_BoostMap_module::BmSetObjectKnns);
	enter_command("bmset_trobject_knns", &class_BoostMap_module::BmSetTrainingObjectKnns);
	enter_command("bmset_print", &class_BoostMap_module::BmSetPrint);
	enter_command("bmset_sknn_embedding", &class_BoostMap_module::BmSetSknnEmbedding0);
	enter_command("bmset_split", &class_BoostMap_module::BmSetSplit);
	enter_command("bmset_testtrain", &class_BoostMap_module::BmSetTestTrain0);
	enter_command("bmset_training", &class_BoostMap_module::BmSetTraining);
	enter_command("bmset_training_knn", &class_BoostMap_module::BmSetTrainingKnn0);
	enter_command("bmset_training_knn_embedding", &class_BoostMap_module::BmSetTrainingKnnEmbedding0);
	enter_command("bmset_traintrain", &class_BoostMap_module::BmSetTrainTrain0);
	enter_command("bmset_validation", &class_BoostMap_module::BmSetValidation);
	enter_command("bmset_validity", &class_BoostMap_module::BmSetValidity);
	enter_command("bmset_wknn", &class_BoostMap_module::BmSetWknn0);

	enter_command("BoostMap_data_candidates", &class_BoostMap_module::BoostMap_data_candidates);
	enter_command("BoostMap_data_cascade_statistics_old", &class_BoostMap_module::BoostMap_data_cascade_statistics);
	enter_command("BoostMap_data_cascade_statistics", &class_BoostMap_module::BoostMap_data_cascade_statistics_fr);
	enter_command("BoostMap_data_check_rank", &class_BoostMap_module::BoostMap_data_check_rank);
	enter_command("BoostMap_data_count_closer", &class_BoostMap_module::BoostMap_data_count_closer);
	enter_command("BoostMap_data_distance", &class_BoostMap_module::BoostMap_data_distance0);
	enter_command("BoostMap_data_distance_bound", &class_BoostMap_module::BmSetDistanceBound);
	enter_command("BoostMap_data_embed_triple", &class_BoostMap_module::BoostMap_data_embed_triple0);
	enter_command("BoostMap_data_embedding_distances", &class_BoostMap_module::BoostMap_data_embedding_distances);
	enter_command("BoostMap_data_filter_refine", &class_BoostMap_module::BoostMap_data_filter_refine);
	enter_command("BoostMap_data_index_error", &class_BoostMap_module::BoostMap_data_index_error0);
	enter_command("BoostMap_data_index_errors", &class_BoostMap_module::BoostMap_data_index_errors0);
	enter_command("BoostMap_data_index_errorsb", &class_BoostMap_module::BoostMap_data_index_errorsb);
	enter_command("BoostMap_data_knn", &class_BoostMap_module::BoostMap_data_knn0);
	enter_command("BoostMap_data_knn_embedding", &class_BoostMap_module::BoostMap_data_knn_embedding0);
	enter_command("BoostMap_data_label", &class_BoostMap_module::BoostMap_data_label0);
	enter_command("BoostMap_data_load", &class_BoostMap_module::BoostMap_data_load0);
	enter_command("BoostMap_data_make", &class_BoostMap_module::BoostMap_data_make);
	enter_command("BoostMap_data_make5", &class_BoostMap_module::BoostMap_data_make5);
	enter_command("BoostMap_data_make2", &class_BoostMap_module::BoostMap_data_make2);
	enter_command("BoostMap_data_make_smaller", &class_BoostMap_module::BoostMap_data_make_smaller);
	enter_command("BoostMap_data_merge", &class_BoostMap_module::BoostMap_data_merge);
	enter_command("BoostMap_data_object_knns", &class_BoostMap_module::BoostMap_data_object_knns);
	enter_command("BoostMap_data_trobject_knns", &class_BoostMap_module::BoostMap_data_training_object_knns);
	enter_command("BoostMap_data_print", &class_BoostMap_module::BoostMap_data_print);
	enter_command("BoostMap_data_random_subdataset", &class_BoostMap_module::BoostMap_data_random_subdataset);
	enter_command("BoostMap_data_retrieval", &class_BoostMap_module::BoostMap_data_retrieval);
	enter_command("BoostMap_data_retrieval_training", &class_BoostMap_module::BoostMap_data_retrieval_training);
	enter_command("BoostMap_data_save_embeddings", &class_BoostMap_module::BoostMap_data_save_embeddings);
	enter_command("BoostMap_data_sknn_embedding", &class_BoostMap_module::BoostMap_data_sknn_embedding0);
	enter_command("BoostMap_data_split", &class_BoostMap_module::BoostMap_data_split);
	enter_command("BoostMap_data_test_triple", &class_BoostMap_module::BoostMap_data_test_triple0);
	enter_command("BoostMap_data_test_train", &class_BoostMap_module::BoostMap_data_test_train);
	enter_command("BoostMap_data_test_train_distance", &class_BoostMap_module::BoostMap_data_test_train_distance0);
	enter_command("BoostMap_data_training", &class_BoostMap_module::BoostMap_data_training);
	enter_command("BoostMap_data_training_knn", &class_BoostMap_module::BoostMap_data_training_knn0);
	enter_command("BoostMap_data_training_knn_embedding", &class_BoostMap_module::BoostMap_data_training_knn_embedding0);
	enter_command("BoostMap_data_train_train_distance", &class_BoostMap_module::BoostMap_data_train_train_distance0);
	enter_command("BoostMap_data_train_train", &class_BoostMap_module::BoostMap_data_train_train);
	enter_command("BoostMap_data_validation", &class_BoostMap_module::BoostMap_data_validation);
	enter_command("BoostMap_data_validity", &class_BoostMap_module::BoostMap_data_validity);
	enter_command("BoostMap_data_wknn", &class_BoostMap_module::BoostMap_data_wknn0);

	enter_command("bnn_add", &class_BoostMap_module::BNN_AddClassifier0);
	enter_command("bnn_add2", &class_BoostMap_module::BNN_AddClassifiers0);
	enter_command("bnn_analyze", &class_BoostMap_module::BNN_AnalyzeTriples);
	enter_command("bnn_analyzet", &class_BoostMap_module::BNN_AnalyzeTestTriples);
	enter_command("bnn_backtrack", &class_BoostMap_module::BNN_Backtrack0);
	enter_command("bnn_bad_triples", &class_BoostMap_module::BNN_BadTriples);
	enter_command("bnn_bad_triples2", &class_BoostMap_module::BNN_BadTriples20);
	enter_command("bnn_classifier", &class_BoostMap_module::BNN_Classifier);
	enter_command("bnn_confusions", &class_BoostMap_module::BNN_Confusions);
	enter_command("bnn_cross_validate", &class_BoostMap_module::BNN_CrossValidate0);
	enter_command("bnn_cross_validate_bad", &class_BoostMap_module::BNN_CrossValidateBad0);
	enter_command("bnn_create", &class_BoostMap_module::BNN_Create0);
	enter_command("bnn_delete", &class_BoostMap_module::BNN_delete);
	enter_command("bnn_erase_all", &class_BoostMap_module::BNN_EraseAll);
	enter_command("bnn_histogram", &class_BoostMap_module::BNN_BadNumberHistogram);
	enter_command("bnn_knn_errors", &class_BoostMap_module::BNN_KnnErrors0);
	enter_command("bnn_knn_info", &class_BoostMap_module::BNN_KnnInfo0);
	enter_command("bnn_knn_training", &class_BoostMap_module::BNN_KNNTraining0);
	enter_command("bnn_knn_test", &class_BoostMap_module::BNN_KNNTest0);
	enter_command("bnn_load", &class_BoostMap_module::BNN_Load0);
	enter_command("bnn_majority1", &class_BoostMap_module::BNN_Majority10);
	enter_command("bnn_max_z", &class_BoostMap_module::BNN_MaxZ0);
	enter_command("bnn_naive_knn", &class_BoostMap_module::BNN_NaiveKnn0);
	enter_command("bnn_naive_knn_cross", &class_BoostMap_module::BNN_NaiveKnnCross0);
	enter_command("bnn_negative_weights", &class_BoostMap_module::BNN_AllowNegative0);
	enter_command("bnn_new_bnn", &class_BoostMap_module::BNN_NewBNN0);
	enter_command("bnn_new_bnn2", &class_BoostMap_module::BNN_NewBNN2);
	enter_command("bnn_new_training", &class_BoostMap_module::BNN_NewTraining);
	enter_command("bnn_norm_ranges", &class_BoostMap_module::BNN_NormalizeRanges);
	enter_command("bnn_norm_ranges2", &class_BoostMap_module::BNN_NormalizeRanges2);
	enter_command("bnn_norm_stds", &class_BoostMap_module::BNN_NormalizeStds);
	enter_command("bnn_number", &class_BoostMap_module::BNN_Number);
	enter_command("bnn_print", &class_BoostMap_module::BNN_PrintSummary);
	enter_command("bnn_print_all", &class_BoostMap_module::BNN_PrintAll);
	enter_command("bnn_print_training", &class_BoostMap_module::BNN_PrintTraining);
	enter_command("bnn_print_validation", &class_BoostMap_module::BNN_PrintValidation);
	enter_command("bnn_ranges", &class_BoostMap_module::BNN_Ranges);
	enter_command("bnn_rank_info", &class_BoostMap_module::BNN_RankInfo0);
	enter_command("bnn_ring_size", &class_BoostMap_module::BNN_RingSize0);
	enter_command("bnn_qnumbers", &class_BoostMap_module::BNN_QNumbers0);
	enter_command("bnn_qnumbers2", &class_BoostMap_module::BNN_QNumbers20);
	enter_command("bnn_save", &class_BoostMap_module::BNN_Save0);
	enter_command("bnn_select", &class_BoostMap_module::BNN_Select0);
	enter_command("bnn_some_weights", &class_BoostMap_module::BNN_SomeWeights0);
	enter_command("bnn_sort_weights", &class_BoostMap_module::BNN_SortWeights);
	enter_command("bnn_square", &class_BoostMap_module::BNN_Square);
	enter_command("bnn_stds", &class_BoostMap_module::BNN_Stds);
	enter_command("bnn_step", &class_BoostMap_module::BNN_Step0);
	enter_command("bnn_steps", &class_BoostMap_module::BNN_Steps0);
	enter_command("bnn_test_triples", &class_BoostMap_module::BNN_TestTriples);
	enter_command("bnn_train", &class_BoostMap_module::BNN_Train0);
	enter_command("bnn_train2", &class_BoostMap_module::BNN_Train20);
	enter_command("bnn_train3", &class_BoostMap_module::BNN_Train30);
	enter_command("bnn_train_loop", &class_BoostMap_module::BNN_TrainLoop0);
	enter_command("bnn_train_loop2", &class_BoostMap_module::BNN_TrainLoop20);
	enter_command("bnn_triple", &class_BoostMap_module::BNN_Triple0);
	enter_command("bnn_triple_stats", &class_BoostMap_module::BNN_TripleStats);
	enter_command("bnn_weight", &class_BoostMap_module::BNN_Weight0);
	enter_command("bnn_weights", &class_BoostMap_module::BNN_Weights0);
	enter_command("bnn_weight_sum", &class_BoostMap_module::BNN_WeightSum0);
	enter_command("bnn_z_contributions", &class_BoostMap_module::BNN_ZContributions0);

	enter_command("bnnqs_add", &class_BoostMap_module::BnnqsAddClassifier);
	enter_command("bnnqs_add2", &class_BoostMap_module::BnnqsAddClassifiers0);
	enter_command("bnnqs_analyze", &class_BoostMap_module::BnnqsAnalyzeTriples);
	enter_command("bnnqs_analyzet", &class_BoostMap_module::BnnqsAnalyzeTestTriples);
	enter_command("bnnqs_bad_triples2", &class_BoostMap_module::BnnqsBadTriples2);
	enter_command("bnnqs_check", &class_BoostMap_module::BnnqsCheck);
	enter_command("bnnqs_check_tr", &class_BoostMap_module::BnnqsCheckTr);
	enter_command("bnnqs_classifier", &class_BoostMap_module::BnnqsClassifier);
	enter_command("bnnqs_confusions", &class_BoostMap_module::BnnqsConfusions);
	enter_command("bnnqs_cross_validate", &class_BoostMap_module::BnnqsCrossValidate);
	enter_command("bnnqs_create", &class_BoostMap_module::BnnqsCreate0);
	enter_command("bnnqs_histogram", &class_BoostMap_module::BnnqsBadNumberHistogram);
	enter_command("bnnqs_knn_errors", &class_BoostMap_module::BnnqsKnnErrors);
	enter_command("bnnqs_knn_info", &class_BoostMap_module::BnnqsKnnInfo);
	enter_command("bnnqs_knn_training", &class_BoostMap_module::BnnqsKNNTraining);
	enter_command("bnnqs_knn_test", &class_BoostMap_module::BnnqsKNNTest);
	enter_command("bnnqs_load", &class_BoostMap_module::BnnqsLoad);
	enter_command("bnnqs_negative_weights", &class_BoostMap_module::BnnqsAllowNegative);
	enter_command("bnnqs_new_bnn", &class_BoostMap_module::BnnqsNewBNN0);
	enter_command("bnnqs_new_bnn2", &class_BoostMap_module::BnnqsNewBNN2);
	enter_command("bnnqs_norm_ranges", &class_BoostMap_module::BnnqsNormalizeRanges);
	enter_command("bnnqs_norm_ranges2", &class_BoostMap_module::BnnqsNormalizeRanges2);
	enter_command("bnnqs_norm_stds", &class_BoostMap_module::BnnqsNormalizeStds);
	enter_command("bnnqs_print", &class_BoostMap_module::BnnqsPrintSummary);
	enter_command("bnnqs_print_training", &class_BoostMap_module::BnnqsPrintTraining);
	enter_command("bnnqs_print_validation", &class_BoostMap_module::BnnqsPrintValidation);
	enter_command("bnnqs_ranges", &class_BoostMap_module::BnnqsRanges);
	enter_command("bnnqs_rank_info", &class_BoostMap_module::BnnqsRankInfo);
	enter_command("bnnqs_qnumbers", &class_BoostMap_module::BnnqsQNumbers);
	enter_command("bnnqs_qnumbers2", &class_BoostMap_module::BnnqsQNumbers2);
	enter_command("bnnqs_save", &class_BoostMap_module::BnnqsSave);
	enter_command("bnnqs_some_weights", &class_BoostMap_module::BnnqsSomeWeights);
	enter_command("bnnqs_sort_weights", &class_BoostMap_module::BnnqsSortWeights);
	enter_command("bnnqs_stds", &class_BoostMap_module::BnnqsStds);
	enter_command("bnnqs_step", &class_BoostMap_module::BnnqsStep0);
	enter_command("bnnqs_stepqs", &class_BoostMap_module::BnnqsStepqs0);
	enter_command("bnnqs_stepsqs", &class_BoostMap_module::BnnqsStepsqs);
	enter_command("bnnqs_steps", &class_BoostMap_module::BnnqsSteps);
	enter_command("bnnqs_test_triples", &class_BoostMap_module::BnnqsTestTriples);
	enter_command("bnnqs_train", &class_BoostMap_module::BnnqsTrain0);
	enter_command("bnnqs_train2", &class_BoostMap_module::BnnqsTrain2);
	enter_command("bnnqs_train3", &class_BoostMap_module::BnnqsTrain3);
	enter_command("bnnqs_train_loop", &class_BoostMap_module::BnnqsTrainLoop);
	enter_command("bnnqs_train_loop2", &class_BoostMap_module::BnnqsTrainLoop2);
	enter_command("bnnqs_triple", &class_BoostMap_module::BnnqsTriple);
	enter_command("bnnqs_triple_stats", &class_BoostMap_module::BnnqsTripleStats);
	enter_command("bnnqs_weight", &class_BoostMap_module::BnnqsWeight);
	enter_command("bnnqs_weights", &class_BoostMap_module::BnnqsWeights);
	enter_command("bnnqs_weight_sum", &class_BoostMap_module::BnnqsWeightSum);
	enter_command("bnnqs_z_contributions", &class_BoostMap_module::BnnqsZContributions);

	enter_command("fastmap_filter_refine", &class_BoostMap_module::FastMapFilterRefine);
	enter_command("fastmap_index_errors", &class_BoostMap_module::FastMapIndexErrors);
	enter_command("fastmap_knn", &class_BoostMap_module::FastMapKnn);
	enter_command("fmembed_test", &class_BoostMap_module::FmEmbedTest);
	enter_command("fmembed_train", &class_BoostMap_module::FmEmbedTraining);
	enter_command("fmnew2", &class_BoostMap_module::FmMake20);
	enter_command("fmnew3", &class_BoostMap_module::FmMake30);
	enter_command("fmpivots", &class_BoostMap_module::FmPrintPivots);
	enter_command("fmsample", &class_BoostMap_module::FmPrintSample0);
	enter_command("fmsave", &class_BoostMap_module::FmSave0);
	enter_command("fmstep", &class_BoostMap_module::FmNextStep);
	enter_command("fmsteps", &class_BoostMap_module::FmNextSteps0);
	enter_command("fmtraining", &class_BoostMap_module::FmPrintTraining0);

	enter_command("optimizer_new", &class_BoostMap_module::optimizer_new);
	enter_command("optimizer_threshold_number", &class_BoostMap_module::optimizer_threshold_number);
	enter_command("optimizer_stress_step", &class_BoostMap_module::optimizer_stress_step);
	enter_command("optimizer_stress_steps", &class_BoostMap_module::optimizer_stress_steps);
	enter_command("optimizer_distortion_step", &class_BoostMap_module::optimizer_distortion_step);
	enter_command("optimizer_distortion_steps", &class_BoostMap_module::optimizer_distortion_steps);
	enter_command("optimizer_magnification_step", &class_BoostMap_module::optimizer_magnification_step);
	enter_command("optimizer_magnification_steps", &class_BoostMap_module::optimizer_magnification_steps);
	enter_command("optimizer_test_magnification", &class_BoostMap_module::optimizer_test_magnification);

	enter_command("refinement_AdaBoost", &class_BoostMap_module::refinement_AdaBoost);
	enter_command("refinement_AdaBoost_load", &class_BoostMap_module::refinement_AdaBoost_load);
	enter_command("refinement_delete", &class_BoostMap_module::refinement_delete);
	enter_command("refinement_load", &class_BoostMap_module::refinement_load);
	enter_command("refinement_new", &class_BoostMap_module::refinement_new);
	enter_command("refinement_print", &class_BoostMap_module::refinement_print);
	enter_command("refinement_results", &class_BoostMap_module::refinement_results);
	enter_command("refinement_save", &class_BoostMap_module::refinement_save);
	enter_command("refinement_threshold", &class_BoostMap_module::refinement_threshold);
	enter_command("refinement_train_more", &class_BoostMap_module::refinement_train_more);
	enter_command("refinement_train_step", &class_BoostMap_module::refinement_train_step);

	enter_command("similarity_new", &class_BoostMap_module::similarity_new);
	enter_command("similarity_change_triples", &class_BoostMap_module::similarity_change_triples);
	enter_command("similarity_third_triples", &class_BoostMap_module::similarity_third_triples);
	enter_command("similarity_cross_validate", &class_BoostMap_module::similarity_cross_validate);
	enter_command("similarity_train", &class_BoostMap_module::similarity_train);
	enter_command("similarity_normalize_factors", &class_BoostMap_module::similarity_normalize_factors);
	enter_command("similarity_AdaBoost_create", &class_BoostMap_module::similarity_AdaBoost_create);
	enter_command("similarity_simultaneous_step", &class_BoostMap_module::similarity_simultaneous_step);
	enter_command("similarity_next_level", &class_BoostMap_module::similarity_next_level);

	enter_command("test_mean", &class_BoostMap_module::TestMean0);
	enter_command("test_std", &class_BoostMap_module::TestStd0);
	enter_command("test_sum", &class_BoostMap_module::TestSum0);
	enter_command("test_variance", &class_BoostMap_module::TestVariance0);

	enter_command("uci_datasets", &class_BoostMap_module::UciDatasets);
	enter_command("uci_ids", &class_BoostMap_module::UciIds);
	enter_command("uci_indices", &class_BoostMap_module::UciIndices0);
	enter_command("uci_load", &class_BoostMap_module::UciLoad0);
	enter_command("uci_print", &class_BoostMap_module::UciPrint);
	enter_command("uci_row", &class_BoostMap_module::UciRow0);
	enter_command("uci_sample", &class_BoostMap_module::UciSample0);
	enter_command("uci_sample2", &class_BoostMap_module::UciSample20);
	enter_command("uci_sizes", &class_BoostMap_module::UciSizes);
	enter_command("uci_training_object", &class_BoostMap_module::UciTrainingObject0);

	enter_command("vp_delete", &class_BoostMap_module::VpDelete);
	enter_command("vp_knn_stats", &class_BoostMap_module::VpKnnStats);
	enter_command("vp_knn_error", &class_BoostMap_module::VpKnnClassification);
	enter_command("vp_new", &class_BoostMap_module::VpNew);
	enter_command("vp_load", &class_BoostMap_module::VpLoad);
	enter_command("vp_nn_stats", &class_BoostMap_module::VpNnStats);
	enter_command("vp_print", &class_BoostMap_module::VpPrint);
	enter_command("vp_save", &class_BoostMap_module::VpSave);
	enter_command("vp_test_range", &class_BoostMap_module::VpTestRange);
	enter_command("vp_test_nn", &class_BoostMap_module::VpTestNn);
	enter_command("vp_test_knn", &class_BoostMap_module::VpTestKnn);


	return 1;
}


vint8 class_BoostMap_module::enter_command(char * command, type_procedure procedure)
{
	commands.push_back(function_copy_string (command));
	procedures.push_back(procedure);
	return 1;
}


vint8 class_BoostMap_module::process_command(const char * command)
{
	vint8 index = command_index (command);
	if (index < 0)
	{
		return 0;
	}
	type_procedure procedure = procedures[(vector_size) index];
	(this->*procedure)();
	return 1;
}

vint8 class_BoostMap_module::sort_commands()
{
	vector<vint8> indices;
	function_string_ranks(&commands, & indices);

	vector <char*> temporary;
	temporary.insert(temporary.end (), commands.begin (), commands.end ());
	vector <type_procedure> temporary_procedures;
	temporary_procedures.insert(temporary_procedures.end(), procedures.begin (), procedures.end ());

	vint8 number = commands.size ();
	vint8 counter;
	for (counter = 0; counter < number; counter ++)
	{
		vint8 index = indices[(vector_size) counter];
		commands[(vector_size) counter] = temporary[(vector_size) index];
		procedures[(vector_size) counter] = temporary_procedures[(vector_size) index];
	}

	return 1;
}

//////////////////////////////////////////////////////////
//                                                      //
//  end of obligatory member functions for any module   //
//                                                      //
//////////////////////////////////////////////////////////


vint8 class_BoostMap_module::BmSetLoad0()
{
	char name[1000];
	vPrint("\nenter name:\n");
	vScan("%s", name);
	return BmSetLoad(name);
}


vint8 class_BoostMap_module::BmSetLoad(const char * name)
{
	vdelete(bm_data);
	bm_data = new BoostMap_data(g_data_directory, name);
	if (bm_data->valid() <= 0)
	{
		vdelete(bm_data);
		bm_data = 0;
		vPrint("failed to read from %s\n", name);
		return 0;
	}
	else
	{
		vPrint("loaded bm_data from %s\n", name);
		return 1;
	}
}


vint8 class_BoostMap_module::BmSetMake()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidate_start = 0, candidate_end = 0;
	long training_start = 0, training_end = 0;
	long validation_start = 0, validation_end = 0;

	vPrint("\nenter original_dataset_name, name:\n");
	vScan("%s %s", original_dataset_name, name);

	vPrint("enter candidate start/end, training start/end, validation start/end:\n");
	vScan("%li %li %li %li %li %li", &candidate_start, &candidate_end, 
		&training_start, &training_end, &validation_start, &validation_end);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, candidate_start, candidate_end,
		training_start, training_end,                       
		validation_start, validation_end);
	return 1;
}


// here we don't specify the ranges for candidates, training and validation,
// we just specify how many objects should be used, and those objects are
// chosen randomly.
vint8 class_BoostMap_module::BmSetMake5()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidates = 0, training = 0, validation = 0;
	vPrint("\nenter original_dataset_name, name, candidates, training, validation:\n");
	vScan("%s %s %li %li %li", original_dataset_name, name, 
		&candidates, &training, &validation);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, 
		candidates, training, validation);
	return 1;
}


// BmSetMake2 was designed for the protein dataset.
vint8 class_BoostMap_module::BmSetMake2()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidate_start = 0, candidate_end = 0;
	long training_start1 = 0, training_end1 = 0;
	long training_start2 = 0;
	long validation_start1 = 0, validation_end1 = 0;
	long validation_start2 = 0;

	vPrint("\nenter original_dataset_name, name:\n");
	vScan("%s %s", original_dataset_name, name);

	vPrint("candidate start/end, training start/end/start2, validation start/end/start2:\n");
	vScan("%li %li %li %li %li %li %li %li", &candidate_start, &candidate_end, 
		&training_start1, &training_end1, &training_start2,
		&validation_start1, &validation_end1, &validation_start2);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, candidate_start, candidate_end,
		training_start1, training_end1, training_start2,                     
		validation_start1, validation_end1, validation_start2);
	return 1;
}



vint8 class_BoostMap_module::BmSetCandidates()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintCandidates();
	return 1;
}


vint8 class_BoostMap_module::BmSetTraining()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintTraining();
	return 1;
}


vint8 class_BoostMap_module::BmSetValidation()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintValidation();
	return 1;
}


vint8 class_BoostMap_module::BmSetPrint()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->Print();
	return 1;
}


vint8 class_BoostMap_module::BmSetDistance0()
{
	if (bm_data == 0)
	{
		return 0;
	}

	long index1 = 0, index2 = 0;
	vPrint("\nenter index1, index2:\n");
	vScan("%li %li", &index1, &index2);
	return BmSetDistance(index1, index2);
}


vint8 class_BoostMap_module::BmSetDistance(vint8 index1, vint8 index2)
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintDistance(index1, index2);
	return 1;
}


vint8 class_BoostMap_module::BmSetValidity()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->CheckValidity();
	return 1;
}


vint8 class_BoostMap_module::BmSetLabel0()
{
	char name[1000];
	long index = 0;
	vPrint("\nenter name, index\n");
	vScan("%s %li", name, &index);
	return BmSetLabel(name, index);
}


vint8 class_BoostMap_module::BmSetLabel(const char * name, vint8 index)
{
	if (index < 0) 
	{
		return 0;
	}
	vMatrix<float> labels = BoostMap_data::LoadTrainingLabels(g_data_directory, name);
	if ((labels.valid() > 0) && (labels.Size() > index))
	{
		float label = labels(index);
		vPrint("Training label for %li: %f\n", (long) index, label);
	}

	labels = BoostMap_data::LoadTestLabels(g_data_directory, name);
	if ((labels.valid() > 0) && (labels.Size() > index))
	{
		float label = labels(index);
		vPrint("Test label for %li: %f\n", (long) index, label);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetTestTrain0()
{
	char name[1000];
	long index1 = 0, index2 = 0;
	vPrint("\nenter name, index1, index2\n");
	vScan("%s %li %li", name, &index1, &index2);
	return BmSetTestTrain(name, index1, index2);
}


vint8 class_BoostMap_module::BmSetTestTrain(const char * name, 
	vint8 index1, vint8 index2)
{
	if ((index1 < 0) || (index2 < 0))
	{
		return 0;
	}

	vMatrix<float> distances = BoostMap_data::TestTrainDistance(name, index1);
	if ((distances.valid() <= 0) || (distances.Size() <= index1))
	{
		return 0;
	}

	if (index2 >= distances.Size())
	{
		vPrint("distances only has %li entries\n", (long) distances.Size());
		return 0;
	}
	float distance = distances(index2);
	vPrint("Distance from test %li to training %li = %f\n", 
		(long) index1, (long) index2, distance);

	return 1;
}


vint8 class_BoostMap_module::BmSetTrainTrain0()
{
	char name[1000];
	long index1 = 0, index2 = 0;
	vPrint("\nenter name, index1, index2\n");
	vScan("%s %li %li", name, &index1, &index2);
	return BmSetTrainTrain(name, index1, index2);
}


vint8 class_BoostMap_module::BmSetTrainTrain(const char * name, 
	vint8 index1, vint8 index2)
{
	if ((index1 < 0) || (index2 < 0))
	{
		return 0;
	}

	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, name, index1);
	if ((distances.valid() <= 0) || (distances.Size() <= index1))
	{
		return 0;
	}

	if (index2 >= distances.Size())
	{
		vPrint("distances only has %li entries\n", (long) distances.Size());
		return 0;
	}
	float distance = distances(index2);
	vPrint("Distance from test %li to training %li = %f\n", 
		(long) index1, (long) index2, distance);

	return 1;
}


vint8 class_BoostMap_module::BmSetWknn0()
{
	char name[1000];
	long test_flag = 0, k = 0, index = 0;
	vPrint("\nenter name, test_flag, k, index:\n");
	vScan("%s %li %li %li", name, &test_flag, &k, &index);
	return BmSetWknn(name, index, test_flag, k);
}


vint8 class_BoostMap_module::BmSetWknn(const char * name, vint8 index, 
	vint8 test_flag, vint8 k)
{
	vMatrix<float> result = BoostMap_data::WknnRanks4(name, index, test_flag, k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_distance = (float) 1000000000;
		for (i = 0; i < result.Rows(); i++)
		{
			vint8 index = round_number(result(i, 0));
			float distance = result(i, 1);
			vPrint("%li class: index = %li, distance = %f\n", (long) i, (long) index, distance);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_index = i;
			}
		}
		vPrint("\nbest class: %li. best distance: %f\n", (long) min_index, min_distance);

		vMatrix<float> labels;

		if (test_flag == 0)
		{
			labels = BoostMap_data::LoadTrainingLabels(g_data_directory, name);
		}
		else
		{
			labels = BoostMap_data::LoadTestLabels(g_data_directory, name);
		}
		if ((labels.valid() > 0) || (labels.Size() > index))
		{
			vPrint("class label = %li\n", (long) round_number(labels(index)));
		}
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetKnn0()
{
	char name[1000];
	long max_k = 0;
	vPrint("\nenter name, max_k:\n");
	vScan("%s %li", name, &max_k);
	return BmSetKnn(name, max_k);
}


vint8 class_BoostMap_module::BmSetKnn(const char * name, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnError2(name, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetKnnEmbedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BmSetKnnEmbedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BmSetKnnEmbedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnEmbeddingError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetCascadeStats()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);

	vMatrix<vint8> results(4, max_k+1);

	vint8 temp = BoostMap_data::CascadeStats4(dataset, embedding, 
		dimensions, results);

	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%5li %7li %7li %7li %7li\n", 
			(long) k, (long) results(0, k), (long) results(1, k), (long) results(2, k), (long) results(3, k));
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetCascadeStatsFr()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, to_keep = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, to_keep, max_k:\n");
	vScan("%s %s %li %li %li", dataset, embedding, &dimensions, &to_keep, &max_k);

	vMatrix<vint8> results(4, max_k+1);

	vMatrix<vint8> test_info = BoostMap_data::CascadeStatsFr(dataset, embedding, 
		dimensions, to_keep, results);
	test_info.Write("d:\\users\\athitsos\\trash.bin");

	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%5li %7li %7li %7li %7li\n", 
			(long) k, (long) results(0, k), (long) results(1, k), (long) results(2, k), (long) results(3, k));
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetSknnEmbedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BmSetSknnEmbedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BmSetSknnEmbedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnSembeddingError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetFilterRefine()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, to_keep = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, to_keep, max_k:\n");
	vScan("%s %s %li %li %li", dataset, embedding, &dimensions, &to_keep, &max_k);

	vMatrix<float> result = BoostMap_data::FilterRefineErrors5(dataset, embedding, 
		dimensions, to_keep, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetIndexError0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0;

	vPrint("\nenter dataset, embedding, dimensions:\n");
	vScan("%s %s %li", dataset, embedding, &dimensions);
	return BmSetIndexError(dataset, embedding, dimensions);
}


vint8 class_BoostMap_module::BmSetIndexError(const char * dataset, 
	const char * embedding,
	vint8 dimensions)
{
	vMatrix<float> result = BoostMap_data::IndexError3(dataset, embedding, 
		dimensions);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	// v3dMatrix<float> temp_ranks = copy_vertical_line(&result, 1);
	v3dMatrix<float> temp_ranks(&result);

	vector<float> ranks;
	vector_from_matrix(&temp_ranks, &ranks);
	std::sort(ranks.begin(), ranks.end(), less<float>());
	vint8 size = ranks.size();
	float number = 20;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		float entry = ranks[(vector_size) index];

		vPrint("%li: %5.2f, %f\n", (long) round_number(index), percentile, entry);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetIndexErrors0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	function_print("***deprecated***, use BoostMap_data_retrieval instead\n");
	//vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	//vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BmSetIndexErrors(dataset, embedding, dimensions, max_k);
}



vint8 class_BoostMap_module::BmSetIndexErrors(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	function_print("***deprecated***, use BoostMap_data_retrieval instead\n");
	//vMatrix<float> result = BoostMap_data::IndexErrors4(dataset, embedding, 
	//                                                    dimensions, max_k);
	//vint8_matrix vint8_result(& result);
	//print_retrieval_results(vint8_result);

	return 1;
}


vint8 class_BoostMap_module::BmSetIndexDistribution()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0, class_label = 0;
	long distribution = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k, class_label, distribution:\n");
	vScan("%s %s %li %li %li %li", dataset, embedding, &dimensions, 
		&max_k, &class_label, &distribution);

	vMatrix<float> result = BoostMap_data::index_errors_distribution(dataset, embedding, 
		dimensions, max_k, class_label,
		distribution);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	v3dMatrix<float> temp_ranks1 = copy_vertical_line(&result, 1);
	v3dMatrix<float> temp_ranks2 = copy_vertical_line(&result, max_k);

	vector<float> ranks1;
	vector<float> ranks2;
	vector_from_matrix(&temp_ranks1, &ranks1);
	vector_from_matrix(&temp_ranks2, &ranks2);
	std::sort(ranks1.begin(), ranks1.end(), less<float>());
	std::sort(ranks2.begin(), ranks2.end(), less<float>());
	vint8 size = ranks1.size();
	float number = 100;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		if (index == size)
		{
			index--;
		}
		float entry1 = ranks1[(vector_size) index];
		float entry2 = ranks2[(vector_size) index];

		vPrint("%5li: %6.2f, %10.2f %10.2f\n", 
			(long) round_number(index), percentile, entry1, entry2);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetDistanceBound()
{
	char embedding[1000];
	long dimensions = 0;

	vPrint("\nenter embedding, dimensions:\n");
	vScan("%s %li", embedding, &dimensions);

	vint8 distance_bound = BoostMap_data::distance_bound(embedding, dimensions);
	print("upper bound of number of distances for embedding step: %li\n", (long) distance_bound);
	return 1;
}


// for the protein dataset
vint8 class_BoostMap_module::BmSetIndexErrorsb()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);

	vMatrix<float> result = BoostMap_data::IndexErrors4b(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	v3dMatrix<float> temp_ranks1 = copy_vertical_line(&result, 1);
	v3dMatrix<float> temp_ranks2 = copy_vertical_line(&result, max_k);

	vector<float> ranks1;
	vector<float> ranks2;
	vector_from_matrix(&temp_ranks1, &ranks1);
	vector_from_matrix(&temp_ranks2, &ranks2);
	std::sort(ranks1.begin(), ranks1.end(), less<float>());
	std::sort(ranks2.begin(), ranks2.end(), less<float>());
	vint8 size = ranks1.size();
	float number = 100;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		if (index == size)
		{
			index--;
		}
		float entry1 = ranks1[(vector_size) index];
		float entry2 = ranks2[(vector_size) index];

		vPrint("%5li: %6.2f, %10.2f %10.2f\n", 
			(long) round_number(index), percentile, entry1, entry2);
	}

	return 1;
}




vint8 class_BoostMap_module::BmSetTrainingKnn0()
{
	char name[1000];
	long max_k = 0;
	vPrint("\nenter name, max_k:\n");
	vScan("%s %li", name, &max_k);
	return BmSetTrainingKnn(name, max_k);
}


vint8 class_BoostMap_module::BmSetTrainingKnn(const char * name, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnTrainError2(name, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetTrainingKnnEmbedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, max_k, dimensions:\n");
	vScan("%s %s %li %li", dataset, embedding, &max_k, &dimensions);
	return BmSetTrainingKnnEmbedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BmSetTrainingKnnEmbedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnEmbeddingTrainError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BmSetEmbedTriple0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, q = 0, a = 0, b = 0, distance_p = 0;

	vPrint("\nenter dataset, embedding, dimensions, q, a, b, distance_p:\n");
	vScan("%s %s %li %li %li %li %li", dataset, embedding, &dimensions, &q, &a, &b, &distance_p);
	return BmSetEmbedTriple(dataset, embedding, dimensions, q, a, b, distance_p);
}


vint8 class_BoostMap_module::BmSetEmbedTriple(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 q, vint8 a, vint8 b,
	vint8 distance_p)
{
	float result = BoostMap_data::EmbedTriple(dataset, embedding, dimensions, q, a, b,
		distance_p);
	if (result <= 0)
	{
		vPrint("failed to test triple\n");
	}
	return 1;
}


// we get the top k nearest neighbors of a test object, and we print them.
vint8 class_BoostMap_module::BmSetObjectKnns()
{
	char dataset[1000];
	long object = 0, k = 0;

	vPrint("\nEnter dataset, test object, k\n");
	vScan("%s %li %li", dataset, &object, &k);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, object, 1, k);
	knns.Print("k-nns");
	return 1;
}


// we get the top k nearest neighbors of a test object, and we print them.
vint8 class_BoostMap_module::BmSetTrainingObjectKnns()
{
	char dataset[1000];
	long object = 0, k = 0;

	vPrint("\nEnter dataset, training object, k\n");
	vScan("%s %li %li", dataset, &object, &k);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, object, 0, k);
	knns.Print("k-nns");
	return 1;
}


// we get the rank of training object Y with respect to training object X, 
// looking only at training objects within a specified range of the 
// database
vint8 class_BoostMap_module::BmSetCheckRank()
{
	long min_index = 0, max_index = 0, object = 0, neighbor = 0;

	char dataset[1000];

	vPrint("\nEnter dataset, min_index, max_index, training object, neighbor, m\n");
	vScan("%s %li %li %li %li", dataset, &min_index, &max_index, &object, &neighbor);

	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, dataset, object);
	if (distances.valid() <= 0)
	{
		vPrint("failed to load distances\n");
		return 0;
	}

	vint8 size = distances.Size();
	if ((neighbor < 0) || (neighbor >= size))
	{
		vPrint("neighbor = %li, size = %li\n", (long) neighbor, (long) size);
		return 0;
	}

	if (min_index < 0) 
	{
		min_index = 0;
	}
	if (max_index >= distances.Size())
	{
		max_index = (long) (distances.Size() - 1);
	}

	vint8 counter = 0, tie_counter = 0;
	vint8 i;
	float distance = distances(neighbor);
	float max_distance = function_image_maximum(&distances);
	float limit = 2 * vAbs(max_distance) + 1;
	distances(object) = limit;
	distances(neighbor) = limit;
	for (i = min_index; i <= max_index; i++)
	{
		if (distances(i) < distance)
		{
			counter++;
		}
		else if (distances(i) == distance)
		{
			tie_counter++;
		}
	}

	vPrint("object = %6li, neighbor = %6li, rank = %6li, ties = %6li\n",
		(long) object, (long) neighbor, (long) (counter + 1), (long) tie_counter);
	return 1;

}


// for a given test object and a given training object, we count
// how many training objects are mapped closer to the training
// object using a given reference object. We also print the 
// closest objects (based on the embedding).
vint8 class_BoostMap_module::BmSetCountCloser()
{
	char dataset[1000];
	long test_object = 0, reference = 0;

	vPrint("\nenter dataset, test object, reference object:\n");
	vScan("%s %li %li", dataset, &test_object, &reference);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, test_object, 1, 5);
	knns.Print("k-nns");

	vint8 training_object = (vint8) knns(0, 0);
	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, dataset, 
		reference);
	vMatrix<float> distances2 = BoostMap_data::TestTrainDistance(dataset, 
		test_object);
	float qr = distances2(reference);
	float ar = distances(training_object);
	float qa = vAbs(qr - ar);

	vint8 i;
	vint8 size = (vint8) distances.Size();
	vMatrix<float> distances3(1, size);
	vint8 counter = 0;
	for (i = 0; i < size; i++)
	{
		float br = distances(i);
		float qb = vAbs(qr - br);
		distances3(i) = qb;
		if ((qb < qa) && (i != training_object))
		{
			counter++;
		}
	}

	vMatrix<float> oned_knns = BoostMap_data::FindKnn2(distances3, 5);
	oned_knns.Print("one-d-knns");
	vPrint("\ncounter = %li\n", (long) counter);
	vPrint("qr = %f, ar = %f, qa = %f\n", qr, ar, qa);
	return 1;
}


vint8 class_BoostMap_module::BmSetMerge()
{
	char name[1000];
	vPrint("\nenter name:\n");
	vScan("%s", name);
	BoostMap_data::MergeTestTrain(name);
	return 1;
}


vint8 class_BoostMap_module::BmSetSplit()
{
	char name1[1000], name2[1000];
	long test_size = 0;
	vPrint("\nenter name1, name2, test_size:\n");
	vScan("%s %s %li", name1, name2, &test_size);
	BoostMap_data::Split(name1, name2, test_size);
	return 1;
}





vint8 class_BoostMap_module::BoostMap_data_load0()
{
	char name[1000];
	vPrint("\nenter name:\n");
	vScan("%s", name);
	return BoostMap_data_load(name);
}


vint8 class_BoostMap_module::BoostMap_data_load(const char * name)
{
	vdelete(bm_data);
	bm_data = new BoostMap_data(g_data_directory, name);
	if (bm_data->valid() <= 0)
	{
		vdelete(bm_data);
		bm_data = 0;
		vPrint("failed to read from %s\n", name);
		return 0;
	}
	else
	{
		vPrint("loaded bm_data from %s\n", name);
		return 1;
	}
}


vint8 class_BoostMap_module::BoostMap_data_make()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidate_start = 0, candidate_end = 0;
	long training_start = 0, training_end = 0;
	long validation_start = 0, validation_end = 0;

	vPrint("\nenter original_dataset_name, name:\n");
	vScan("%s %s", original_dataset_name, name);

	vPrint("enter candidate start/end, training start/end, validation start/end:\n");
	vScan("%li %li %li %li %li %li", &candidate_start, &candidate_end, 
		&training_start, &training_end, &validation_start, &validation_end);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, candidate_start, candidate_end,
		training_start, training_end,                       
		validation_start, validation_end);
	return 1;
}


// here we don't specify the ranges for candidates, training and validation,
// we just specify how many objects should be used, and those objects are
// chosen randomly.
vint8 class_BoostMap_module::BoostMap_data_make5()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidates = 0, training = 0, validation = 0;
	vPrint("\nenter original_dataset_name, name, candidates, training, validation:\n");
	vScan("%s %s %li %li %li", original_dataset_name, name, 
		&candidates, &training, &validation);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, 
		candidates, training, validation);
	return 1;
}


// BmSetMake2 was designed for the protein dataset.
vint8 class_BoostMap_module::BoostMap_data_make2()
{
	vdelete(bm_data);

	char original_dataset_name[1000];
	char name[1000];
	long candidate_start = 0, candidate_end = 0;
	long training_start1 = 0, training_end1 = 0;
	long training_start2 = 0;
	long validation_start1 = 0, validation_end1 = 0;
	long validation_start2 = 0;

	vPrint("\nenter original_dataset_name, name:\n");
	vScan("%s %s", original_dataset_name, name);

	vPrint("candidate start/end, training start/end/start2, validation start/end/start2:\n");
	vScan("%li %li %li %li %li %li %li %li", &candidate_start, &candidate_end, 
		&training_start1, &training_end1, &training_start2,
		&validation_start1, &validation_end1, &validation_start2);

	bm_data = new BoostMap_data(g_data_directory, original_dataset_name, name, candidate_start, candidate_end,
		training_start1, training_end1, training_start2,                     
		validation_start1, validation_end1, validation_start2);
	return 1;
}



vint8 class_BoostMap_module::BoostMap_data_candidates()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintCandidates();
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_training()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintTraining();
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_validation()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintValidation();
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_print()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->Print();
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_distance0()
{
	if (bm_data == 0)
	{
		return 0;
	}

	long index1 = 0, index2 = 0;
	vPrint("\nenter index1, index2:\n");
	vScan("%li %li", &index1, &index2);
	return BoostMap_data_distance(index1, index2);
}


vint8 class_BoostMap_module::BoostMap_data_distance(vint8 index1, vint8 index2)
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->PrintDistance(index1, index2);
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_validity()
{
	if (bm_data == 0)
	{
		return 0;
	}

	bm_data->CheckValidity();
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_label0()
{
	char name[1000];
	long index = 0;
	vPrint("\nenter name, index\n");
	vScan("%s %li", name, &index);
	return BoostMap_data_label(name, index);
}


vint8 class_BoostMap_module::BoostMap_data_label(const char * name, vint8 index)
{
	if (index < 0) 
	{
		return 0;
	}
	vMatrix<float> labels = BoostMap_data::LoadTrainingLabels(g_data_directory, name);
	if ((labels.valid() > 0) && (labels.Size() > index))
	{
		float label = labels(index);
		vPrint("Training label for %li: %f\n", (long) index, label);
	}

	labels = BoostMap_data::LoadTestLabels(g_data_directory, name);
	if ((labels.valid() > 0) && (labels.Size() > index))
	{
		float label = labels(index);
		vPrint("Test label for %li: %f\n", (long) index, label);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_test_train_distance0()
{
	char name[1000];
	long index1 = 0, index2 = 0;
	vPrint("\nenter name, index1, index2\n");
	vScan("%s %li %li", name, &index1, &index2);
	return BoostMap_data_test_train_distance(name, index1, index2);
}


vint8 class_BoostMap_module::BoostMap_data_test_train_distance(const char * name, 
	vint8 index1, vint8 index2)
{
	if ((index1 < 0) || (index2 < 0))
	{
		return 0;
	}

	vMatrix<float> distances = BoostMap_data::TestTrainDistance(name, index1);
	if ((distances.valid() <= 0) || (distances.Size() <= index1))
	{
		return 0;
	}

	if (index2 >= distances.Size())
	{
		vPrint("distances only has %li entries\n", (long) distances.Size());
		return 0;
	}
	float distance = distances(index2);
	vPrint("Distance from test %li to training %li = %f\n", 
		(long) index1, (long) index2, distance);

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_train_train_distance0()
{
	char name[1000];
	long index1 = 0, index2 = 0;
	vPrint("\nenter name, index1, index2\n");
	vScan("%s %li %li", name, &index1, &index2);
	return BoostMap_data_train_train_distance(name, index1, index2);
}


vint8 class_BoostMap_module::BoostMap_data_train_train_distance(const char * name, 
	vint8 index1, vint8 index2)
{
	if ((index1 < 0) || (index2 < 0))
	{
		return 0;
	}

	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, name, index1);
	if ((distances.valid() <= 0) || (distances.Size() <= index1))
	{
		return 0;
	}

	if (index2 >= distances.Size())
	{
		vPrint("distances only has %li entries\n", (long) distances.Size());
		return 0;
	}
	float distance = distances(index2);
	vPrint("Distance from test %li to training %li = %f\n", 
		(long) index1, (long) index2, distance);

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_wknn0()
{
	char name[1000];
	long test_flag = 0, k = 0, index = 0;
	vPrint("\nenter name, test_flag, k, index:\n");
	vScan("%s %li %li %li", name, &test_flag, &k, &index);
	return BoostMap_data_wknn(name, index, test_flag, k);
}


vint8 class_BoostMap_module::BoostMap_data_wknn(const char * name, vint8 index, 
	vint8 test_flag, vint8 k)
{
	vMatrix<float> result = BoostMap_data::WknnRanks4(name, index, test_flag, k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_distance = (float) 1000000000;
		for (i = 0; i < result.Rows(); i++)
		{
			vint8 index = round_number(result(i, 0));
			float distance = result(i, 1);
			vPrint("%li class: index = %li, distance = %f\n", (long) i, (long) index, distance);
			if (distance < min_distance)
			{
				min_distance = distance;
				min_index = i;
			}
		}
		vPrint("\nbest class: %li. best distance: %f\n", (long) min_index, min_distance);

		vMatrix<float> labels;

		if (test_flag == 0)
		{
			labels = BoostMap_data::LoadTrainingLabels(g_data_directory, name);
		}
		else
		{
			labels = BoostMap_data::LoadTestLabels(g_data_directory, name);
		}
		if ((labels.valid() > 0) || (labels.Size() > index))
		{
			vPrint("class label = %li\n", (long) round_number(labels(index)));
		}
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_knn0()
{
	char name[1000];
	long max_k = 0;
	vPrint("\nenter name, max_k:\n");
	vScan("%s %li", name, &max_k);
	return BoostMap_data_knn(name, max_k);
}


vint8 class_BoostMap_module::BoostMap_data_knn(const char * name, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnError2(name, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_knn_embedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BoostMap_data_knn_embedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BoostMap_data_knn_embedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnEmbeddingError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_cascade_statistics()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);

	vMatrix<vint8> results(4, max_k+1);

	vint8 temp = BoostMap_data::CascadeStats4(dataset, embedding, 
		dimensions, results);

	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%5li %7li %7li %7li %7li\n", 
			(long) k, (long) results(0, k), (long) results(1, k), 
			(long) results(2, k), (long) results(3, k));
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_cascade_statistics_fr()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, to_keep = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, to_keep, max_k:\n");
	vScan("%s %s %li %li %li", dataset, embedding, &dimensions, &to_keep, &max_k);

	vMatrix<vint8> results(4, max_k+1);

	vMatrix<vint8> test_info = BoostMap_data::CascadeStatsFr(dataset, embedding, 
		dimensions, to_keep, results);
	test_info.Write("d:\\users\\athitsos\\trash.bin");

	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%5li %7li %7li %7li %7li\n", 
			(long) k, (long) results(0, k), (long) results(1, k), 
			(long) results(2, k), (long) results(3, k));
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_sknn_embedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BoostMap_data_sknn_embedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BoostMap_data_sknn_embedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnSembeddingError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_filter_refine()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, to_keep = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, to_keep, max_k:\n");
	vScan("%s %s %li %li %li", dataset, embedding, &dimensions, &to_keep, &max_k);

	vMatrix<float> result = BoostMap_data::FilterRefineErrors5(dataset, embedding, 
		dimensions, to_keep, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		int i;
		int min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("1-nn error: %f\n", result(1));
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_index_error0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0;

	vPrint("\nenter dataset, embedding, dimensions:\n");
	vScan("%s %s %li", dataset, embedding, &dimensions);
	return BoostMap_data_index_error(dataset, embedding, dimensions);
}


vint8 class_BoostMap_module::BoostMap_data_index_error(const char * dataset, 
	const char * embedding,
	vint8 dimensions)
{
	vMatrix<float> result = BoostMap_data::IndexError3(dataset, embedding, 
		dimensions);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	v3dMatrix<float> temp_ranks = copy_vertical_line(&result, 1);

	vector<float> ranks;
	vector_from_matrix(&temp_ranks, &ranks);
	std::sort(ranks.begin(), ranks.end(), less<float>());
	vint8 size = ranks.size();
	float number = 20;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		float entry = ranks[(vector_size) index];

		vPrint("%li: %5.2f, %f\n", (long) index, percentile, entry);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_index_errors0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	return BoostMap_data_index_errors(dataset, embedding, dimensions, max_k);
}



vint8 class_BoostMap_module::BoostMap_data_index_errors(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::IndexErrors4(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	v3dMatrix<float> temp_ranks1 = copy_vertical_line(&result, 1);
	v3dMatrix<float> temp_ranks2 = copy_vertical_line(&result, max_k);

	vector<float> ranks1;
	vector<float> ranks2;
	vector_from_matrix(&temp_ranks1, &ranks1);
	vector_from_matrix(&temp_ranks2, &ranks2);
	std::sort(ranks1.begin(), ranks1.end(), less<float>());
	std::sort(ranks2.begin(), ranks2.end(), less<float>());
	vint8 size = ranks1.size();
	float number = 100;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		if (index == size)
		{
			index--;
		}
		float entry1 = ranks1[(vector_size) index];
		float entry2 = ranks2[(vector_size) index];

		vPrint("%5li: %6.2f, %10.2f %10.2f\n", 
			(long) round_number(index), percentile, entry1, entry2);
	}

	return 1;
}


// for the protein dataset
vint8 class_BoostMap_module::BoostMap_data_index_errorsb()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);

	vMatrix<float> result = BoostMap_data::IndexErrors4b(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
		return 0;
	}

	v3dMatrix<float> temp_ranks1 = copy_vertical_line(&result, 1);
	v3dMatrix<float> temp_ranks2 = copy_vertical_line(&result, max_k);

	vector<float> ranks1;
	vector<float> ranks2;
	vector_from_matrix(&temp_ranks1, &ranks1);
	vector_from_matrix(&temp_ranks2, &ranks2);
	std::sort(ranks1.begin(), ranks1.end(), less<float>());
	std::sort(ranks2.begin(), ranks2.end(), less<float>());
	vint8 size = ranks1.size();
	float number = 100;
	float i;
	for (i = 0; round_number(i) <= round_number(number); i = i + (float) 1.0)
	{
		float percentile = i * ((float) 100.0 / number);
		vint8 index = round_number(((float) size) / number * i);
		if (index == size)
		{
			index--;
		}
		float entry1 = ranks1[(vector_size) index];
		float entry2 = ranks2[(vector_size) index];

		vPrint("%5li: %6.2f, %10.2f %10.2f\n", 
			(long) round_number(index), percentile, entry1, entry2);
	}

	return 1;
}




vint8 class_BoostMap_module::BoostMap_data_training_knn0()
{
	char name[1000];
	long max_k = 0;
	vPrint("\nenter name, max_k:\n");
	vScan("%s %li", name, &max_k);
	return BoostMap_data_training_knn(name, max_k);
}


vint8 class_BoostMap_module::BoostMap_data_training_knn(const char * name, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnTrainError2(name, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_training_knn_embedding0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, max_k, dimensions:\n");
	vScan("%s %s %li %li", dataset, embedding, &max_k, &dimensions);
	return BoostMap_data_training_knn_embedding(dataset, embedding, dimensions, max_k);
}


vint8 class_BoostMap_module::BoostMap_data_training_knn_embedding(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 max_k)
{
	vMatrix<float> result = BoostMap_data::KnnEmbeddingTrainError(dataset, embedding, 
		dimensions, max_k);
	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_save_embeddings()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0;

	vPrint("\nenter dataset, embedding:\n");
	vScan("%s %s", dataset, embedding);
	return BoostMap_data_save_embeddings(dataset, embedding);
}


vint8 class_BoostMap_module::BoostMap_data_save_embeddings(const char * dataset, 
	const char * embedding)
{
	vint8 result = BoostMap_data::save_embeddings(dataset, embedding);
	if (result <= 0)
	{
		vPrint("failed to embed dataset\n");
	}
	return (vint8) result;
}


vint8 class_BoostMap_module::BoostMap_data_test_triple0()
{
	char dataset[1000];
	long dimensions = 0, q = 0, a = 0, b = 0, distance_p = 0;

	vPrint("\nenter dataset, q, a, b, distance_p:\n");
	vScan("%s %li %li %li %li %li", dataset, &dimensions, &q, &a, &b, &distance_p);
	return BoostMap_data_test_triple(dataset, dimensions, q, a, b, distance_p);
}


vint8 class_BoostMap_module::BoostMap_data_test_triple(const char * dataset, 
	vint8 dimensions,											
	vint8 q, vint8 a, vint8 b,
	vint8 distance_p)
{
	float result = BoostMap_data::TestTriple(dataset, embedding_classifier, database_embedding, q, a, b,
		distance_p);
	if (result <= 0)
	{
		vPrint("failed to test triple\n");
	}
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_embed_triple0()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, q = 0, a = 0, b = 0, distance_p = 0;

	vPrint("\nenter dataset, embedding, dimensions, q, a, b, distance_p:\n");
	vScan("%s %s %li %li %li %li %li", dataset, embedding, &dimensions, &q, &a, &b, &distance_p);
	return BoostMap_data_embed_triple(dataset, embedding, dimensions, q, a, b, distance_p);
}


vint8 class_BoostMap_module::BoostMap_data_embed_triple(const char * dataset, 
	const char * embedding,
	vint8 dimensions, vint8 q, vint8 a, vint8 b,
	vint8 distance_p)
{
	float result = BoostMap_data::EmbedTriple(dataset, embedding, dimensions, q, a, b,
		distance_p);
	if (result <= 0)
	{
		vPrint("failed to test triple\n");
	}
	return 1;
}


// we get the top k nearest neighbors of a test object, and we print them.
vint8 class_BoostMap_module::BoostMap_data_object_knns()
{
	char dataset[1000];
	long object = 0, k = 0;

	vPrint("\nEnter dataset, test object, k\n");
	vScan("%s %li %li", dataset, &object, &k);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, object, 1, k);
	knns.Print("k-nns");
	return 1;
}


// we get the top k nearest neighbors of a test object, and we print them.
vint8 class_BoostMap_module::BoostMap_data_training_object_knns()
{
	char dataset[1000];
	long object = 0, k = 0;

	vPrint("\nEnter dataset, training object, k\n");
	vScan("%s %li %li", dataset, &object, &k);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, object, 0, k);
	knns.Print("k-nns");
	return 1;
}


// we get the rank of training object Y with respect to training object X, 
// looking only at training objects within a specified range of the 
// database
vint8 class_BoostMap_module::BoostMap_data_check_rank()
{
	long min_index = 0, max_index = 0, object = 0, neighbor = 0;

	char dataset[1000];

	vPrint("\nEnter dataset, min_index, max_index, training object, neighbor, m\n");
	vScan("%s %li %li %li %li", dataset, &min_index, &max_index, &object, &neighbor);

	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, dataset, object);
	if (distances.valid() <= 0)
	{
		vPrint("failed to load distances\n");
		return 0;
	}

	vint8 size = distances.Size();
	if ((neighbor < 0) || (neighbor >= size))
	{
		vPrint("neighbor = %li, size = %li\n", (long) neighbor, (long) size);
		return 0;
	}

	if (min_index < 0) 
	{
		min_index = 0;
	}
	if (max_index >= distances.Size())
	{
		max_index = (long) (distances.Size() - 1);
	}

	vint8 counter = 0, tie_counter = 0;
	vint8 i;
	float distance = distances(neighbor);
	float max_distance = function_image_maximum(&distances);
	float limit = 2 * vAbs(max_distance) + 1;
	distances(object) = limit;
	distances(neighbor) = limit;
	for (i = min_index; i <= max_index; i++)
	{
		if (distances(i) < distance)
		{
			counter++;
		}
		else if (distances(i) == distance)
		{
			tie_counter++;
		}
	}

	vPrint("object = %6li, neighbor = %6li, rank = %6li, ties = %6li\n",
		(long) object, (long) neighbor, (long) (counter + 1), (long) tie_counter);
	return 1;

}


// for a given test object and a given training object, we count
// how many training objects are mapped closer to the training
// object using a given reference object. We also print the 
// closest objects (based on the embedding).
vint8 class_BoostMap_module::BoostMap_data_count_closer()
{
	char dataset[1000];
	long test_object = 0, reference = 0;

	vPrint("\nenter dataset, test object, reference object:\n");
	vScan("%s %li %li", dataset, &test_object, &reference);

	vMatrix<float> knns = BoostMap_data::FindKnn4(dataset, test_object, 1, 5);
	knns.Print("k-nns");

	vint8 training_object = (vint8) knns(0, 0);
	vMatrix<float> distances = BoostMap_data::TrainTrainDistance(g_data_directory, dataset, 
		reference);
	vMatrix<float> distances2 = BoostMap_data::TestTrainDistance(dataset, 
		test_object);
	float qr = distances2(reference);
	float ar = distances(training_object);
	float qa = vAbs(qr - ar);

	vint8 i;
	vint8 size = distances.Size();
	vMatrix<float> distances3(1, size);
	vint8 counter = 0;
	for (i = 0; i < size; i++)
	{
		float br = distances(i);
		float qb = vAbs(qr - br);
		distances3(i) = qb;
		if ((qb < qa) && (i != training_object))
		{
			counter++;
		}
	}

	vMatrix<float> oned_knns = BoostMap_data::FindKnn2(distances3, 5);
	oned_knns.Print("one-d-knns");
	vPrint("\ncounter = %li\n", (long) counter);
	vPrint("qr = %f, ar = %f, qa = %f\n", qr, ar, qa);
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_merge()
{
	char name[1000];
	vPrint("\nenter name:\n");
	vScan("%s", name);
	BoostMap_data::MergeTestTrain(name);
	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_split()
{
	char name1[1000], name2[1000];
	long test_size = 0;
	vPrint("\nenter name1, name2, test_size:\n");
	vScan("%s %s %li", name1, name2, &test_size);
	BoostMap_data::Split(name1, name2, test_size);
	return 1;
}

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

// start of BoostMap_data functions for SIGMOD 2007


// make a smaller data set to speed up testing and debugging
vint8 class_BoostMap_module::BoostMap_data_make_smaller()
{
	char original_dataset_name[1000], smaller_name[1000];
	long test_start = 0, test_size = 0, training_start = 0, training_size = 0;

	print("enter original_dataset_name, small dataset name, test start, test size,\ntraining start, training size:\n");
	question("%s %s %li %li %li %li", original_dataset_name, smaller_name,
		& test_start, & test_size, & training_start, & training_size);

	vint8 success = BoostMap_data::OriginalSubdataset(original_dataset_name, smaller_name, test_start, test_size,
		training_start, training_size);

	if (success <= 0)
	{
		print("failed to create smaller data set %s\n", smaller_name);
		return 0;
	}
	else
	{
		print("created smaller data set %s successfully\n", smaller_name);
		return 1;
	}
}


// make a smaller data set to speed up testing and debugging
// here, we keep the full test set and a random sample of training (database) data
vint8 class_BoostMap_module::BoostMap_data_random_subdataset()
{
	char original_dataset_name[1000], smaller_name[1000];
	long test_start = 0, test_size = 0, training_start = 0, training_size = 0;

	print("enter original_dataset_name, small dataset name, training size:\n");
	question("%s %s %li", original_dataset_name, smaller_name, & training_size);

	vint8 success = BoostMap_data::random_subdataset(original_dataset_name, smaller_name, training_size);

	if (success <= 0)
	{
		print("failed to create smaller data set %s\n", smaller_name);
		return 0;
	}
	else
	{
		print("created smaller data set %s successfully\n", smaller_name);
		return 1;
	}
}


vint8 class_BoostMap_module::BoostMap_data_embedding_distances()
{
	char original_dataset_name[1000], embedding_name[1000];

	print("\nenter original name of data set, embedding filename:\n");
	question("%s %s", original_dataset_name, embedding_name);

	BoostMap_data::embedding_test_train(original_dataset_name, embedding_name);
	BoostMap_data::embedding_train_train(original_dataset_name, embedding_name);

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_test_train()
{
	char original_dataset_name[1000], embedding_name[1000];

	print("\nenter original name of data set, embedding filename:\n");
	question("%s %s", original_dataset_name, embedding_name);

	BoostMap_data::embedding_test_train(original_dataset_name, embedding_name);

	return 1;
}


vint8 class_BoostMap_module::BoostMap_data_train_train()
{
	char original_dataset_name[1000], embedding_name[1000];

	print("\nenter original name of data set, embedding filename:\n");
	question("%s %s", original_dataset_name, embedding_name);

	BoostMap_data::embedding_train_train(original_dataset_name, embedding_name);

	return 1;
}


// equivalent to BoostMap_data_index_errors, but using a reimplementation.
vint8 class_BoostMap_module::BoostMap_data_retrieval()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	vint8_matrix result = BoostMap_data::retrieval_results_test(dataset, embedding, 
		dimensions, max_k);
	print_retrieval_results(result);

	return 1;
}


// equivalent to BoostMap_data_retrieval(), but here the query set is
// the set of database objects
vint8 class_BoostMap_module::BoostMap_data_retrieval_training()
{
	char dataset[1000];
	char embedding[1000];
	long dimensions = 0, max_k = 0;

	vPrint("\nenter dataset, embedding, dimensions, max_k:\n");
	vScan("%s %s %li %li", dataset, embedding, &dimensions, &max_k);
	vint8_matrix result = BoostMap_data::retrieval_results_training(dataset, embedding, 
		dimensions, max_k);
	print_retrieval_results(result);

	return 1;
}


vint8 class_BoostMap_module::print_retrieval_results(vint8_matrix result)
{
	return class_BoostMap::print_retrieval_results(result);
}


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
///////////////////////////////////////////////////
///////////////////////////////////////////////////

// end of BoostMap_data functions for SIGMOD 2007


vint8 class_BoostMap_module::BmNew0()
{
	char name[1000];
	long number_of_triples = 0, use_pdistances = 0;
	vPrint("\nEnter name, number_of_triples, use_pdistances:\n");
	vScan("%s %li %li", name, &number_of_triples, &use_pdistances);
	return BmNew(name, number_of_triples, use_pdistances);
}


vint8 class_BoostMap_module::BmNew(const char * name, vint8 number_of_triples, 
	vint8 use_pdistances)
{
	BmDelete();
	bm = new class_BoostMap(g_data_directory, name, number_of_triples, use_pdistances);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmNew20()
{
	char name[1000];
	long number_of_triples = 0, use_pdistances = 0, classes = 0, max_k = 0;
	vPrint("\nEnter name, number_of_triples, use_pdistances, classes, max_k:\n");
	vScan("%s %li %li %li %li", name, &number_of_triples, &use_pdistances,
		&classes, &max_k);
	return BmNew2(name, number_of_triples, use_pdistances, classes, max_k);
}


vint8 class_BoostMap_module::BmNew2(const char * name, vint8 number_of_triples, 
	vint8 use_pdistances, vint8 classes, vint8 max_k)
{
	BmDelete();
	bm = new class_BoostMap(g_data_directory, name, number_of_triples, use_pdistances,
		classes, max_k);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmNew30()
{
	char name[1000];
	long number_of_triples = 0, use_pdistances = 0, classes = 0, max_k = 0;
	vPrint("\nEnter name, number_of_triples, use_pdistances, classes, max_k:\n");
	vScan("%s %li %li %li %li", name, &number_of_triples, &use_pdistances,
		&classes, &max_k);
	return BmNew3(name, number_of_triples, use_pdistances, classes, max_k);
}


vint8 class_BoostMap_module::BmNew3(const char * name, vint8 number_of_triples, 
	vint8 use_pdistances, vint8 classes, vint8 max_k)
{
	BmDelete();
	bm = new class_BoostMap(g_data_directory, name, number_of_triples, use_pdistances,
		classes, max_k);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmNew4()
{
	char name[1000];
	long number_of_triples = 0, min_a = 0, max_a = 0, min_b = 0, max_b = 0;
	vPrint("\nEnter name, number_of_triples, min_a, max_a, min_b, max_b:\n");
	vScan("%s %li %li %li %li %li", name, &number_of_triples,
		&min_a, &max_a, &min_b, &max_b);

	BmDelete();
	bm = new class_BoostMap(g_data_directory, name, number_of_triples, min_a, max_a, min_b, max_b);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmNew5()
{
	char name[1000];
	long number_of_triples = 0, min_a = 0, max_a = 0, min_b = 0, max_b = 0;
	vPrint("\nEnter name, number_of_triples, min_a, max_a, min_b, max_b:\n");
	vScan("%s %li %li %li %li %li", name, &number_of_triples,
		&min_a, &max_a, &min_b, &max_b);

	BmDelete();
	bm = new class_BoostMap(g_data_directory, name, number_of_triples, min_a, max_a, min_b, max_b, 0);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmNewLoad()
{
	char dataset[100], filename[100];
	long training = 0, validation = 0, p_distances = 0;
	vPrint("\nEnter dataset, filename, training triples, validation triples, p_distances:\n");
	vScan("%s %s %li %li %li", dataset, filename, &training, &validation, &p_distances);

	BmDelete();
	bm = new class_BoostMap(dataset, filename, training, validation, p_distances);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}
	return 1;
}


vint8 class_BoostMap_module::BmDelete()
{
	if (bm == 0)
	{
		return 0;
	}

	//  const char * class_name = bm->get_class_name();
	const char * class_name = bm->get_class_name();
	if (strcmp(class_name, "class_BoostMap") == 0)
	{
		function_delete(bm);
	}
	else if (strcmp(class_name, "embedding_optimizer") == 0)
	{
		embedding_optimizer * optimizer = (embedding_optimizer *) bm;
		function_delete(optimizer);
	}
	else
	{
		exit_error("\nunknown subclass %s of BoostMap\n", class_name);
	}

	bm = 0;
	return 1;
}


vint8 class_BoostMap_module::BmPrintSummary()
{
	if (bm == 0) return 0;
	bm->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BmPrintClassifier()
{
	if (bm == 0) return 0;
	bm->PrintClassifier();
	return 1;
}


vint8 class_BoostMap_module::BmPrintAll()
{
	if (bm == 0) return 0;
	bm->PrintAll();
	return 1;
}


vint8 class_BoostMap_module::BmStep0()
{
	if (bm == 0) return 0;
	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BmStep(buffer);
}


vint8 class_BoostMap_module::BmStep(const char * filename)
{
	if (bm == 0) return 0;
	float training_margin = bm->fast_next_step();
	bm->save_classifier(filename);

	BmPrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BmSteps0()
{
	if (bm == 0) return 0;
	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);
	return BmSteps(buffer, steps);
}


vint8 class_BoostMap_module::BmSteps(char * filename, vint8 steps)
{
	if (bm == 0) return 0;
	vint8 i;
	float min_validation_error = 100000;
	for (i = 0; i < steps; i++)
	{
		BmStep(filename);
		float validation_error = bm->ValidationError();
		if (validation_error < min_validation_error)
		{
			min_validation_error = validation_error;
		}
	}

	return 1;
}


vint8 class_BoostMap_module::BmRandom0()
{
	if (bm == 0) return 0;
	long number = 0, times = 0;
	vPrint("\nEnter number, times:\n");
	vScan("%li %li", &number, &times);
	return BmRandom(number, times);
}


vint8 class_BoostMap_module::BmRandom(vint8 number, vint8 times)
{
	if (bm == 0) return 0;
	float min_error = 10;
	vint8 i;
	vector<float> errors((vector_size) times);
	vPrint("\n");
	for (i = 0; i < times; i++)
	{
		bm->PickRandom(number);
		float error = bm->TrainingError();
		if (error < min_error)
		{
			min_error = error;
		}
		errors[(vector_size) i] = error;
		vPrint("done with %li of %li\r", (long) (i+1), (long) times);
	}

	vPrint("\n");
	BmPrintSummary();

	std::sort(errors.begin(), errors.end(), less<float>());
	for (i = 0; i < times; i++)
	{
		vPrint("%li, %f\n", i, (long) errors[(vector_size) i]);
	}

	vPrint("times = %li, min_error = %f\n", (long) times, min_error);

	return 1;
}


vint8 class_BoostMap_module::BmAllowNegative0()
{
	if (bm == 0) return 0;
	long value = 0;
	vPrint("\nEnter allow_negative value:\n");
	vScan("%li", &value);
	return BmAllowNegative(value);
}


vint8 class_BoostMap_module::BmAllowNegative(vint8 value)
{
	if (bm == 0) return 0;
	bm->SetAllowNegative(value);
	return 1;
}


vint8 class_BoostMap_module::BmAllowRemovals0()
{
	if (bm == 0) return 0;
	long value = 0;
	vPrint("\nEnter allow_removals value:\n");
	vScan("%li", &value);
	return BmAllowRemovals(value);
}


vint8 class_BoostMap_module::BmAllowRemovals(vint8 value)
{
	if (bm == 0) return 0;
	bm->SetAllowRemovals(value);
	return 1;
}


vint8 class_BoostMap_module::BmAllowLipschitz0()
{
	if (bm == 0) return 0;
	long value = 0;
	vPrint("\nEnter allow_lipschitz value:\n");
	vScan("%li", &value);
	return BmAllowLipschitz(value);
}


vint8 class_BoostMap_module::BmAllowLipschitz(vint8 value)
{
	if (bm == 0) return 0;
	bm->SetAllowLipschitz(value);
	return 1;
}


vint8 class_BoostMap_module::BmAllowProjections0()
{
	if (bm == 0) return 0;
	long value = 0;
	vPrint("\nEnter allow_projections value:\n");
	vScan("%li", &value);
	return BmAllowProjections(value);
}


vint8 class_BoostMap_module::BmAllowProjections(vint8 value)
{
	if (bm == 0) return 0;
	bm->SetAllowProjections(value);
	return 1;
}


vint8 class_BoostMap_module::BmAllowSensitive0()
{
	if (bm == 0) 
	{
		return 0;
	}
	long value1 = 0, value2 = 0;
	vPrint("\nEnter allow_sensitive, new_sensitive:\n");
	vScan("%li %li", &value1, &value2);
	return BmAllowSensitive(value1, value2);
}


vint8 class_BoostMap_module::BmAllowSensitive(vint8 value1, vint8 value2)
{
	if (bm == 0) return 0;
	bm->SetAllowSensitive(value1, value2);
	bm->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BmPickedCandidates0()
{
	if (bm == 0) return 0;
	long value = 0;
	vPrint("\nEnter number_of_picked_candidates value:\n");
	vScan("%li", &value);
	return BmPickedCandidates(value);
}


vint8 class_BoostMap_module::BmPickedCandidates(vint8 value)
{
	if (bm == 0) return 0;
	bm->SetPickedCandidates(value);
	return 1;
}


vint8 class_BoostMap_module::BmLoad0()
{
	if (bm == 0) return 0;
	long dimensions = 0;
	char buffer[1000];
	vPrint("\nEnter filename, dimensions:\n");
	vScan("%s %li", buffer, &dimensions);
	return BmLoad(buffer, dimensions);
}


vint8 class_BoostMap_module::BmLoad(const char * filename, vint8 dimensions)
{
	if (bm == 0) return 0;
	bm->load_classifier_b(filename, dimensions);
	return 1;
}


vint8 class_BoostMap_module::BmLoad2()
{
	if (bm == 0) return 0;
	long dimensions = 0;
	char buffer[1000];
	vPrint("\nEnter filename, dimensions:\n");
	vScan("%s %li", buffer, &dimensions);

	// This is not a bug, it is on purpose that BmLoad2 should
	// call LoadClassifier3.
	bm->load_classifier_c(buffer, dimensions);
	return 1;
}


vint8 class_BoostMap_module::BmLoadQs()
{
	if (bm == 0) return 0;
	long dimensions = 0;
	char filename[1000];
	vPrint("\nEnter filename, dimensions:\n");
	vScan("%s %li", filename, &dimensions);

	bm->load_sensitive_classifier(filename, dimensions);
	return 1;
}


vint8 class_BoostMap_module::BmSave0()
{
	if (bm == 0) return 0;
	char filename[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", filename);
	return BmSave(filename);
}


vint8 class_BoostMap_module::BmSave(const char * filename)
{
	if (bm == 0) return 0;
	bm->save_classifier(filename);
	return 1;
}


vint8 class_BoostMap_module::BmSelectFirst0()
{
	char filename[1000];
	long dimensions = 0;

	vPrint("\nenter filename, dimensions:\n");
	vScan("%s %li", filename, &dimensions);
	return BmSelectFirst(filename, dimensions);
}


vint8 class_BoostMap_module::BmSelectFirst(const char * filename, vint8 dimensions)
{
	char * pathname = class_BoostMap::Pathname(g_data_directory, filename);
	vMatrix<float> detailed_classifiers = vMatrix<float>::ReadText(pathname);
	if (detailed_classifiers.valid() <= 0)
	{
		vPrint("Failed to load classifier from %s\n", pathname);
		vdelete2(pathname);
		return 0;
	}

	vdelete2(pathname);
	vMatrix<float> selected = class_BoostMap::select_first_dimensions(detailed_classifiers, 
		dimensions);
	selected.Print("selected");
	return 1;
}


vint8 class_BoostMap_module::BmTestTriple0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long index = 0;
	vPrint("\nenter index:\n");
	vScan("%li", &index);
	return BmTestTriple(index);
}


vint8 class_BoostMap_module::BmTestTriple(vint8 index)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->TestTriple(index);
	return 1;
}


vint8 class_BoostMap_module::BmTrainingTriples0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nenter start, end:\n");
	vScan("%li %li", &start, &end);
	return BmTrainingTriples(start, end);
}


vint8 class_BoostMap_module::BmTrainingTriples(vint8 start, vint8 end)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->PrintTrainingTriples(start, end);
	return 1;
}


vint8 class_BoostMap_module::BmValidationTriples0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nenter start, end:\n");
	vScan("%li %li", &start, &end);
	return BmValidationTriples(start, end);
}


vint8 class_BoostMap_module::BmValidationTriples(vint8 start, vint8 end)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->PrintValidationTriples(start, end);
	return 1;
}


vint8 class_BoostMap_module::BmTripleResults0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long index = 0;
	vPrint("\nenter index:\n");
	vScan("%li", &index);
	return BmTripleResults(index);
}


vint8 class_BoostMap_module::BmTripleResults(vint8 index)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->TripleResults(index);
	return 1;
}


vint8 class_BoostMap_module::BmTripleEmbeddings0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long index = 0;
	vPrint("\nenter index:\n");
	vScan("%li", &index);
	return BmTripleEmbeddings(index);
}


vint8 class_BoostMap_module::BmTripleEmbeddings(vint8 index)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->TripleEmbeddings(index);
	return 1;
}

vint8 class_BoostMap_module::BmSensitiveStep0()
{
	if (bm == 0) return 0;
	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BmSensitiveStep(buffer);
}



vint8 class_BoostMap_module::BmSensitiveStep(const char * filename)
{
	if (bm == 0) return 0;
	vint8 result = bm->NextSensitiveStep();

	bm->save_classifier(filename);
	BmPrintSummary();
	return 1;

}


vint8 class_BoostMap_module::BmSensitiveSteps0()
{
	if (bm == 0) return 0;
	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);
	return BmSensitiveSteps(buffer, steps);
}


vint8 class_BoostMap_module::BmSensitiveSteps(char * filename, vint8 steps)
{
	if (bm == 0) return 0;
	vint8 i;
	float min_validation_error = 100000;
	for (i = 0; i < steps; i++)
	{
		BmSensitiveStep(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::BmFastSensitiveStep0()
{
	if (bm == 0) return 0;
	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BmFastSensitiveStep(buffer);
}



vint8 class_BoostMap_module::BmFastSensitiveStep(const char * filename)
{
	if (bm == 0) return 0;
	vint8 result = bm->fast_next_sensitive_step();

	bm->save_classifier(filename);
	BmPrintSummary();
	return 1;

}


vint8 class_BoostMap_module::BmFastSensitiveSteps0()
{
	if (bm == 0) return 0;
	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);
	return BmFastSensitiveSteps(buffer, steps);
}


vint8 class_BoostMap_module::BmFastSensitiveSteps(char * filename, vint8 steps)
{
	if (bm == 0) return 0;
	if (bm->GetAllowSensitive() <= 0)
	{
		vPrint("adding query-sensitive classifiers not allowed, use bmallow_sensitive\n");
		return 1;
	}

	vint8 i;
	float min_validation_error = 100000;
	for (i = 0; i < steps; i++)
	{
		BmFastSensitiveStep(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::BmAppendDimensions0()
{
	if (bm == 0) 
	{
		return 0;
	}

	char filename[1000];
	long dimensions = 0;
	vPrint("\nenter filename, dimensions\n");
	vScan("%s %li", filename, &dimensions);
	return BmAppendDimensions(filename, dimensions);
}


vint8 class_BoostMap_module::BmAppendDimensions(char * filename, vint8 dimensions)
{
	if (bm == 0)
	{
		return 0;
	}

	bm->AppendClassifiers(filename, dimensions);
	bm->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BmClassifierStatistics0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long in_type = 0, object1 = 0, object2 = 0;
	vPrint("\nenter type, object1, object2\n");
	vScan("%li %li %li", &in_type, &object1, &object2);
	return BmClassifierStatistics(in_type, object1, object2);
}


vint8 class_BoostMap_module::BmClassifierStatistics(vint8 type, vint8 object1, 
	vint8 object2)
{
	if (bm == 0)
	{
		return 0;
	}

	bm->ClassifierStatistics(type, object1, object2);
	return 1;
}


vint8 class_BoostMap_module::BmBestK0()
{
	if (bm == 0) 
	{
		return 0;
	}

	long in_k = 0;
	vPrint("\nenter k:\n");
	vScan("%li", &in_k);
	return BmBestK(in_k);
}


vint8 class_BoostMap_module::BmBestK(vint8 in_k)
{
	if (bm == 0) 
	{
		return 0;
	}

	bm->SetBestK(in_k);
	bm->PrintSummary();
	return 1;
}


// in order to specify a different Lp output distance,
// instead of the default Manhattan distance.
vint8 class_BoostMap_module::BmDistanceP()
{
	if (bm == 0) 
	{
		return 0;
	}

	long distance_p = 0;
	vPrint("\nenter distance_p:\n");
	vScan("%li", &distance_p);
	bm->SetDistanceP(distance_p);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BmAddReference()
{
	if (bm == 0) 
	{
		return 0;
	}

	long index = 0;
	float weight = 0;
	vPrint("\nEnter index, weight:\n");
	vScan("%li %f", &index, &weight);
	bm->AddReferenceObject(index, weight);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BmAddRange()
{
	if (bm == 0) 
	{
		return 0;
	}

	long start_index = 0, end_index = 0;
	float weight = 0;
	vPrint("\nEnter start index, end index, weight:\n");
	vScan("%li %li %f", &start_index, &end_index, &weight);

	vint8 i;
	for (i = start_index; i <= end_index; i++)
	{
		bm->AddReferenceObject(i, weight);
		bm->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BmWeightedReference()
{
	if (bm == 0) 
	{
		return 0;
	}

	long index = 0;
	vPrint("\nEnter index:\n");
	vScan("%li", &index);
	bm->AddWeightedReference(index);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BmWeightedRange()
{
	if (bm == 0) 
	{
		return 0;
	}

	long start_index = 0, end_index = 0;
	vPrint("\nEnter start index, end index:\n");
	vScan("%li %li", &start_index, &end_index);

	vint8 i;
	for (i = start_index; i <= end_index; i++)
	{
		bm->AddWeightedReference(i);
		bm->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BmRandomReferences()
{
	if (bm == 0) 
	{
		return 0;
	}

	long number = 0;
	float weight = (float) 1;
	vPrint("\nEnter number:\n");
	vScan("%li", &number);

	bm->RandomReferences2(number, weight);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BmRandomProjections()
{
	if (bm == 0) 
	{
		return 0;
	}

	long number = 0;
	float weight = (float) 1;
	vPrint("\nEnter number:\n");
	vScan("%li", &number);

	bm->RandomProjections2(number, weight);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BmSaveData()
{
	if (bm == 0)
	{
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter output name:\n");
	vScan("%s", buffer);
	vint8 success = bm->SaveData(buffer);
	if (success <= 0)
	{
		vPrint("failed to save data\n");
	}
	else
	{
		vPrint("saved data to %s\n", buffer);
	}
	return 1;
}


vint8 class_BoostMap_module::BmStep2()
{
	if (bm == 0)
	{
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BmStep2(buffer);
}


vint8 class_BoostMap_module::BmStep2(const char * filename)
{
	if (bm == 0)
	{
		return 0;
	}

	float training_margin = bm->NextStep2();
	bm->save_classifier(filename);

	BmPrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BmSteps2()
{
	if (bm == 0)
	{
		return 0;
	}

	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);

	vint8 i;
	float min_validation_error = 100000;
	for (i = 0; i < steps; i++)
	{
		BmStep2(buffer);
		float validation_error = bm->ValidationError();
		if (validation_error < min_validation_error)
		{
			min_validation_error = validation_error;
		}
	}

	return 1;
}


vint8 class_BoostMap_module::BoostMap_small_triples()
{
	if (bm == 0)
	{
		return 0;
	}

	long number = 0;
	print("\nenter small number of triples:\n");
	question("%li", & number);

	bm->set_small_triple_number(number);
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BoostMap_use_small()
{
	if (bm == 0)
	{
		return 0;
	}

	bm->use_small_training();
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BoostMap_use_large()
{
	if (bm == 0)
	{
		return 0;
	}

	bm->use_large_training();
	bm->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BoostMap_factor_totals()
{
	if (bm == 0)
	{
		return 0;
	}

	bm->print_factor_totals();

	return 1;
}


vint8 class_BoostMap_module::BoostMap_distribution()
{
	if (bm == 0)
	{
		return 0;
	}

	long number = 0;
	float class_total = 0;
	print("\nenter class_label, desired weight for class:\n");
	question("%li %f", & number, & class_total);

	bm->set_distribution(number, class_total);
	bm->print_class_total(number);

	return 1;
}


vint8 class_BoostMap_module::BoostMap_class_total()
{
	if (bm == 0)
	{
		return 0;
	}

	long number = 0;
	print("\nenter class_label:\n");
	question("%li", & number);

	bm->print_class_total(number);

	return 1;
}



char * class_BoostMap_module::EmbeddingDirectory2()
{
	char * result = vJoinPaths4(g_data_directory, "experiments", 
		"bm_datasets", "fastmap");
	return result;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//           Start of functions written for ICML 2004.             //
//                                                                 //
/////////////////////////////////////////////////////////////////////


vint8 class_BoostMap_module::TestSum0()
{
	long rows = 0, cols = 0;
	double min = 0, max = 0;
	vPrint("\nEnter rows, cols, min, max:\n");
	vScan("%li %li %lf %lf", &rows, &cols, &min, &max);
	return TestSum(rows, cols, min, max);
}


vint8 class_BoostMap_module::TestSum(vint8 rows, vint8 cols, double min, double max)
{
	if ((rows < 1) || (cols < 1) || (rows * cols == 0))
	{
		return 0;
	}

	vMatrix<double> random_matrix(rows, cols);
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			random_matrix(row, col) = vRandomDouble(min, max);
		}
	}

	double sum = function_image_total(&random_matrix);
	random_matrix.Print("random matrix");
	vPrint("sum = %lf\n", sum);
	return 1;
}


vint8 class_BoostMap_module::TestMean0()
{
	long rows = 0, cols = 0;
	double min = 0, max = 0;
	vPrint("\nEnter rows, cols, min, max:\n");
	vScan("%li %li %lf %lf", &rows, &cols, &min, &max);
	return TestMean(rows, cols, min, max);
}


vint8 class_BoostMap_module::TestMean(vint8 rows, vint8 cols, double min, double max)
{
	if ((rows < 1) || (cols < 1) || (rows * cols == 0))
	{
		return 0;
	}

	vMatrix<double> random_matrix(rows, cols);
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			random_matrix(row, col) = vRandomDouble(min, max);
		}
	}

	double mean = function_image_average(&random_matrix);
	random_matrix.Print("random matrix");
	vPrint("mean = %lf\n", mean);
	return 1;
}


vint8 class_BoostMap_module::TestVariance0()
{
	long rows = 0, cols = 0;
	double min = 0, max = 0;
	vPrint("\nEnter rows, cols, min, max:\n");
	vScan("%li %li %lf %lf", &rows, &cols, &min, &max);
	return TestVariance(rows, cols, min, max);
}


vint8 class_BoostMap_module::TestVariance(vint8 rows, vint8 cols, double min, double max)
{
	if ((rows < 1) || (cols < 1) || (rows * cols == 1))
	{
		return 0;
	}

	vMatrix<double> random_matrix(rows, cols);
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			random_matrix(row, col) = vRandomDouble(min, max);
		}
	}

	double variance = function_image_variance(&random_matrix);
	random_matrix.Print("random matrix");
	vPrint("variance = %lf\n", variance);
	return 1;
}


vint8 class_BoostMap_module::TestStd0()
{
	long rows = 0, cols = 0;
	double min = 0, max = 0;
	vPrint("\nEnter rows, cols, min, max:\n");
	vScan("%li %li %lf %lf", &rows, &cols, &min, &max);
	return TestStd(rows, cols, min, max);
}


vint8 class_BoostMap_module::TestStd(vint8 rows, vint8 cols, double min, double max)
{
	if ((rows < 1) || (cols < 1) || (rows * cols == 1))
	{
		return 0;
	}

	vMatrix<double> random_matrix(rows, cols);
	vint8 row, col;
	for (row = 0; row < rows; row++)
	{
		for (col = 0; col < cols; col++)
		{
			random_matrix(row, col) = vRandomDouble(min, max);
		}
	}

	double std = function_image_deviation(&random_matrix);
	random_matrix.Print("random matrix");
	vPrint("std = %lf\n", std);
	return 1;
}


// Test the function vPermutation.
vint8 class_BoostMap_module::Permutation0()
{
	long start = 0, end = 0;
	vPrint("\nEnter low value, high value:\n");
	vScan("%li %li", &start, &end);
	return Permutation(start, end);
}


vint8 class_BoostMap_module::Permutation(vint8 start, vint8 end)
{
	vMatrix<vint8> matrix = vPermutation(start, end);
	matrix.PrintInt("permutation");
	return 1;
}


vint8 class_BoostMap_module::UciLoad0()
{
	long number = 0;
	vPrint("\nEnter the dataset number:\n");
	vScan("%li", &number);
	return UciLoad(number);
}


vint8 class_BoostMap_module::UciLoad(vint8 number)
{
	vdelete(uci_set);
	uci_set = new vUciDataset(number);
	if (uci_set->valid() <= 0)
	{
		vdelete(uci_set);
		uci_set = 0;
		print("\nfailed to read UCI dataset\n");
		return 0;
	}

	uci_set->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::UciPrint()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::UciRow0()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		long row = 0;
		vPrint("\nEnter row:\n");
		vScan("%li", &row);
		return UciRow(row);
	}
}


vint8 class_BoostMap_module::UciRow(vint8 row)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintRow(row);
	}
	return 1;
}


vint8 class_BoostMap_module::UciSizes()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		uci_set->PrintTrainingSizes();
		return 1;
	}
}


vint8 class_BoostMap_module::UciTrainingObject0()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		long class_id = 0, index = 0;
		vPrint("\nEnter class, object index:\n");
		vScan("%li %li", &class_id, &index);
		return UciTrainingObject(class_id, index);
	}
}


vint8 class_BoostMap_module::UciTrainingObject(vint8 class_id, vint8 index)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintTrainingObject(class_id, index);
	}
	return 1;
}


vint8 class_BoostMap_module::UciIds()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintClassIds();
	}
	return 1;
}


vint8 class_BoostMap_module::UciDatasets()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintDatasetInfo();
	}
	return 1;
}


vint8 class_BoostMap_module::UciIndices0()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		long class_id = 0;
		vPrint("\nEnter class id:\n");
		vScan("%li", &class_id);
		return UciIndices(class_id);
	}
}


vint8 class_BoostMap_module::UciIndices(vint8 class_id)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->PrintClassIndices(class_id);
	}
	return 1;
}


// Randomly split objects into training and test set.
vint8 class_BoostMap_module::UciSample0()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		float fraction = 0;
		vPrint("\nEnter fraction:\n");
		vScan("%f", &fraction);
		return UciSample(fraction);
	}
}


vint8 class_BoostMap_module::UciSample(float fraction)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->Sample(fraction);
	}
	return 1;
}


vint8 class_BoostMap_module::UciSample20()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return 0;
	}
	else
	{
		float fraction = 0;
		vPrint("\nEnter fraction:\n");
		vScan("%f", &fraction);
		return UciSample2(fraction);
	}
}


vint8 class_BoostMap_module::UciSample2(float fraction)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		uci_set->Sample2(fraction);
	}
	return 1;
}


vint8 class_BoostMap_module::BNN_Create0()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		long training = 0, validation = 0;
		vPrint("\nEnter training, validation:\n");
		vScan("%li %li", &training, &validation);
		BNN_Create(training, validation);
	}
	return 1;
}


vint8 class_BoostMap_module::BNN_Create(vint8 training, vint8 validation)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		vMatrix<float> training_vectors = uci_set->TrainingVectors();
		vMatrix<vint8> training_labels = uci_set->TrainingLabels();
		vMatrix<float> test_vectors = uci_set->TestVectors();
		vMatrix<vint8> test_labels = uci_set->TestLabels();

		bnn = new similarity_learning(training_vectors, training_labels, 
			test_vectors, test_labels,
			training, validation);
		bnn->SetAllowNegativeWeights(bnn_allow_negative);
		if (uci_set->SubjectIdsAvailable() == 1)
		{
			vMatrix<vint8> subject_ids = uci_set->SubjectIds();
			bnn->SetSubjectIds(subject_ids);
		}
		BNN_AddToRing();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_delete()
{
	return (vint8) delete_boosted_similarity(bnn);
}

vint8 class_BoostMap_module::BNN_PrintSummary()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_PrintTraining()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintTraining();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_PrintValidation()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintValidation();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_TestTriples()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintTestTriples();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_TripleStats()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintTripleStats();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Confusions()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintConfusions();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Classifier()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintClassifier();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Triple0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long index = 0;
		vPrint("\nEnter index:\n");
		vScan("%li", &index);
		BNN_Triple(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Triple(vint8 index)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintTrainingTriple(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Weight0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long rank = 0;
		vPrint("\nEnter rank:\n");
		vScan("%li", &rank);
		BNN_Weight(rank);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Weight(vint8 rank)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintWeight(rank);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Weights0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long period = 0;
		vPrint("\nEnter period:\n");
		vScan("%li", &period);
		BNN_Weights(period);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Weights(vint8 period)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintWeights(period);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_SomeWeights0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long start = 0, end = 0, period = 0;
		vPrint("\nEnter start, end, period:\n");
		vScan("%li %li %li", &start, &end, &period);
		BNN_SomeWeights(start, end, period);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_SomeWeights(vint8 start, vint8 end, vint8 period)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintSomeWeights(start, end, period);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_WeightSum0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long start = 0, end = 0;
		vPrint("\nEnter start, end:\n");
		vScan("%li %li", &start, &end);
		BNN_WeightSum(start, end);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_WeightSum(vint8 start, vint8 end)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		bnn->PrintWeightSum(start, end);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_SortWeights()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}

	else
	{
		bnn->SortTrainingWeights();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_ZContributions0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long index = 0;
		vPrint("\nEnter dimension index:\n");
		vScan("%li", &index);
		BNN_ZContributions(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_ZContributions(vint8 index)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}

	else
	{
		bnn->SortZContributions(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_KnnErrors0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long k = 0;
		vPrint("\nEnter k:\n");
		vScan("%li", &k);
		BNN_KnnErrors(k);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_KnnErrors(vint8 k)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}

	else
	{
		bnn->KnnErrors(k);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_RankInfo0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long index = 0;
		vPrint("\nEnter triple index:\n");
		vScan("%li", &index);
		BNN_RankInfo(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_RankInfo(vint8 triple_index)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}

	else
	{
		bnn->TripleRankInfo(triple_index);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_KnnInfo0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}
	else
	{
		long index = 0, k = 0;
		vPrint("\nEnter object index, k:\n");
		vScan("%li %li", &index, &k);
		BNN_KnnInfo(index, k);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_KnnInfo(vint8 index, vint8 k)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
	}

	else
	{
		bnn->KnnInfo(index, k);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Step0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BNN_Step(buffer);
}


vint8 class_BoostMap_module::BNN_Step(const char * filename)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->NextStep();
	bnn->Save(filename);
	bnn->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BNN_Steps0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char buffer[1000];
	long steps = 0;
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li", buffer, &steps);
	return BNN_Steps(buffer, steps);
}


vint8 class_BoostMap_module::BNN_Steps(char * filename, vint8 steps)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		BNN_Step(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_NewTraining()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->NewTrainingSet(2);
	bnn->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BNN_NewBNN0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);
	return BNN_NewBNN(max_k);
}


vint8 class_BoostMap_module::BNN_NewBNN(vint8 max_k)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}
	if (max_k < 1)
	{
		return 0;
	}

	vBoostNN * new_bnn = bnn->NewBoostNN(max_k);
	bnn = new_bnn;
	bnn->PrintSummary();
	BNN_AddToRing();
	return 1;
}


vint8 class_BoostMap_module::BNN_NewBNN2()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	vBoostNN * new_bnn = bnn->NewBoostNN2();
	bnn = new_bnn;
	bnn->PrintSummary();
	BNN_AddToRing();
	return 1;
}


vint8 class_BoostMap_module::BNN_Backtrack0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long steps = 0;
	vPrint("\nEnter steps:\n");
	vScan("%li", &steps);
	return BNN_Backtrack(steps);
}


vint8 class_BoostMap_module::BNN_Backtrack(vint8 steps)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->Backtrack(steps);

	return 1;
}


vint8 class_BoostMap_module::BNN_Load0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BNN_Load(buffer);
}


vint8 class_BoostMap_module::BNN_Load(const char * filename)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->Load(filename);
	return 1;
}


vint8 class_BoostMap_module::BNN_Save0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return BNN_Save(buffer);
}


vint8 class_BoostMap_module::BNN_Save(const char * filename)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->Save(filename);
	return 1;
}




vint8 class_BoostMap_module::BNN_AllowNegative0()
{
	long value = 0;
	vPrint("\nEnter value:\n");
	vScan("%li", &value);
	return BNN_AllowNegative(value);
}


vint8 class_BoostMap_module::BNN_AllowNegative(vint8 value)
{
	bnn_allow_negative = value;
	if (bnn != 0)
	{
		bnn->SetAllowNegativeWeights(value);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Train0()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	char filename[1000];
	vPrint("\nEnter output filename, training triple number:\n");
	vScan("%s %li", filename, &training);
	BNN_Train(filename, training);
	return 1;
}


float class_BoostMap_module::BNN_Train(const char * filename, vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return -100000000;
	}
	const vint8 max_k = 101;

	const vint8 validation_triples = training;
	float last_training_error = 2;
	float last_test_error = 2;
	vint8 last_best_k = 1;

	// rank_limit is not actually used, but it was an attempt to
	// set the max_k argument to ResampleTriples automatically.
	vint8 rank_limit = 2;
	vint8 counter = 0;
	vint8 classes = uci_set->Classes();
	BNN_NormalizeRanges();
	BNN_Create(training, validation_triples);
	while(1)
	{
		char * counter_string = string_from_number(counter);
		char * actual_filename = function_merge_strings_three(filename, "_", counter_string);
		delete_pointer(counter_string);

		similarity_simultaneous_step(actual_filename);
		float training_error = 0, test_error = 0;
		bnn->KnnTrainingErrors(max_k);
		bnn->KnnTestErrors(max_k);

		training_error = bnn->BestKnnTraining();
		test_error = bnn->BestKnnTest();
		vint8 best_k = bnn->BestK();
		vPrint("best_k = %li, training_error = %f, test_error = %f\n",
			(long) best_k, training_error, test_error);
		vPrint("\n------------------------\n\n");
		bnn->Save(actual_filename);
		delete_pointer(actual_filename);

		if ((counter > 1) && (training_error >= last_training_error))
		{
			break;
		}

		last_training_error = training_error;
		last_test_error = test_error;
		last_best_k = (vint8) best_k;

		BNN_NewBNN(1);
		counter++;
	}

	vint8 number = bnns.size();
	if (number >= 2)
	{
		bnn = bnns[(vector_size) (number-2)];
		bnn->PrintSummary();
	}

	vPrint("best k = %li, training error = %f, test error = %f\n",
		(long) last_best_k, last_training_error, last_test_error);

	vPrint("\n-------------------------------------------------\n");
	vPrint("#################################################\n");
	vPrint("-------------------------------------------------\n");

	return last_test_error;
}


float class_BoostMap_module::BNN_Train_old(const char * filename, vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return -100000000;
	}
	const vint8 max_k = 101;

	const vint8 validation_triples = training;
	float last_training_error = 2;
	float last_test_error = 2;
	vint8 last_best_k = 1;

	// rank_limit is not actually used, but it was an attempt to
	// set the max_k argument to ResampleTriples automatically.
	vint8 rank_limit = 2;
	vint8 counter = 0;
	vint8 classes = uci_set->Classes();
	BNN_NormalizeRanges();
	BNN_Create(training, validation_triples);
	while(1)
	{
		char * counter_string = string_from_number(counter);
		char * actual_filename = function_merge_strings_three(filename, "_", counter_string);
		delete_pointer(counter_string);
		while (1)
		{
			BNN_Step("trash");
			bnn->Save(actual_filename);
			float z = bnn->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnn->TrainingError();
			// generate new training in case the previous was overfitted.
			if (training_triple_error < bnn->ValidationError() / 2.0)
			{
				bnn->NewTrainingSet(2);
				bnn->PrintSummary();
			}
			if (bnn->TrainingError() == 0)
			{
				break;
			}
		}
		float training_error = 0, test_error = 0;
		bnn->KnnTrainingErrors(max_k);
		bnn->KnnTestErrors(max_k);
		training_error = bnn->BestKnnTraining();
		test_error = bnn->BestKnnTest();
		vint8 best_k = bnn->BestK();
		vPrint("best_k = %li, training_error = %f, test_error = %f\n",
			(long) best_k, training_error, test_error);
		vPrint("\n------------------------\n\n");
		bnn->Save(actual_filename);
		delete_pointer(actual_filename);

		if (counter == 1)
		{
			last_training_error = training_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}
		else if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}

		rank_limit = last_best_k/2 + 1;
		BNN_NewBNN(2);
		counter++;
	}

	vint8 number = bnns.size();
	if (number >= 2)
	{
		bnn = bnns[(vector_size) (number-2)];
		bnn->PrintSummary();
	}

	vPrint("best k = %li, training error = %f, test error = %f\n",
		(long) last_best_k, last_training_error, last_test_error);

	vPrint("\n-------------------------------------------------\n");
	vPrint("#################################################\n");
	vPrint("-------------------------------------------------\n");

	return last_test_error;
}


vint8 class_BoostMap_module::BNN_Train20()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	vPrint("\nEnter training triple number:\n");
	vScan("%li", &training);
	BNN_Train2(training);
	return 1;
}


// Here we initialize by first trying the unweighted L1 distance.
float class_BoostMap_module::BNN_Train2(vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return -100000000;
	}

	const vint8 validation_triples = 1000;
	float last_training_error = 2;
	float last_test_error = 2;
	BNN_Create(10000, validation_triples);
	vint8 attributes = bnn->Attributes();
	BNN_AddClassifiers(0, (vint8) attributes-1, (float) 0.0001);

	float training_error = 0, test_error = 0;
	bnn->KnnErrors(1, &test_error, &training_error);
	vPrint("test_error = %f, training_error = %f\n",
		test_error, training_error);
	last_training_error = training_error;
	last_test_error = test_error;
	vPrint("\n------------------------\n\n");
	BNN_NewBNN(2);

	while(1)
	{
		while (1)
		{
			BNN_Step("trash");
			float z = bnn->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnn->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}
		bnn->KnnErrors(1, &test_error, &training_error);
		vPrint("test_error = %f, training_error = %f\n",
			test_error, training_error);
		if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			vPrint("\n------------------------\n\n");
			BNN_NewBNN(2);
		}
	}

	vint8 number = bnns.size();
	if (number >= 2)
	{
		bnn = bnns[(vector_size) (number-2)];
		bnn->PrintSummary();
	}

	vPrint("best training error = %f, test error = %f\n",
		last_training_error, last_test_error);

	return last_test_error;
}


vint8 class_BoostMap_module::BNN_Train30()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	vPrint("\nEnter training triple number:\n");
	vScan("%li", &training);
	BNN_Train3(training);
	return 1;
}


float class_BoostMap_module::BNN_Train3(vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return -100000000;
	}

	const vint8 validation_triples = 1000;
	float last_training_error = 2;
	float last_test_error = 2;
	BNN_Create(training, validation_triples);
	while(1)
	{
		while (1)
		{
			BNN_Step("trash");
			float z = bnn->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnn->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}
		float training_error = 0, test_error = 0;
		bnn->KnnErrors(1, &test_error, &training_error);
		vPrint("test_error = %f, training_error = %f\n",
			test_error, training_error);
		if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			vPrint("\n------------------------\n\n");
			BNN_NewBNN2();
		}
	}

	vint8 number = bnns.size();
	if (number >= 2)
	{
		bnn = bnns[(vector_size) (number-2)];
		bnn->PrintSummary();
	}

	vPrint("best training error = %f, test error = %f\n",
		last_training_error, last_test_error);

	return last_test_error;
}


vint8 class_BoostMap_module::BNN_TrainLoop0()
{
	char filename[1000];
	long dataset = 0, training = 0;

	vPrint("\nEnter filename, dataset, training triple number:\n");
	vScan("%s %li %li", filename, &dataset, &training);
	return BNN_TrainLoop(filename, dataset, training);
}


vint8 class_BoostMap_module::BNN_TrainLoop(const char * filename, 
	vint8 dataset, vint8 training)
{
	UciLoad(dataset);
	BNN_Create(10000, 10000);
	BNN_NormalizeRanges();

	vint8 counter = 0;
	while(1)
	{
		BNN_EraseAll();
		BNN_Train("trash", training);

		vint8 number = bnns.size();
		vint8 i;
		vint8 counter2 = 0;
		for (i = number - 1; i >= 0; i--)
		{
			char * string1 = string_from_number(counter);
			char * string2 = string_from_number(counter2);
			char * name = vMergeStrings5(filename, "_", string1, "_", string2);
			bnns[(vector_size) i]->Save(name);
			counter2 = counter2 + 1;
			vdelete2(string1);
			vdelete2(string2);
			vdelete2(name);
		}

		counter = counter + 1;
	}

	// this function never returns.
	return 1;
}


vint8 class_BoostMap_module::BNN_TrainLoop20()
{
	char filename[1000];
	long dataset = 0, training = 0;
	float fraction = 0;

	vPrint("\nEnter filename, dataset, training triple number, fraction:\n");
	vScan("%s %li %li %f", filename, &dataset, &training, &fraction);
	return BNN_TrainLoop2(filename, dataset, training, fraction);
}


vint8 class_BoostMap_module::BNN_TrainLoop2(const char * filename, 
	vint8 dataset, vint8 training, float fraction)
{
	UciLoad(dataset);

	vint8 counter = 0;
	while(1)
	{
		UciSample(fraction);
		BNN_EraseAll();
		BNN_Train("trash", training);

		vint8 number = bnns.size();
		vint8 i;
		vint8 counter2 = 0;
		for (i = number - 1; i >= 0; i--)
		{
			char * string1 = string_from_number(counter);
			char * string2 = string_from_number(counter2);
			char * name = vMergeStrings5(filename, "_", string1, "_", string2);
			bnns[(vector_size) i]->Save(name);
			counter2 = counter2 + 1;
			vdelete2(string1);
			vdelete2(string2);
			vdelete2(name);
		}

		counter = counter + 1;
	}

	// this function never returns.
	return 1;
}


vint8 class_BoostMap_module::BNN_CrossValidateBad0()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	float fraction = 0;
	long times = 0, training = 0;
	vPrint("\nEnter fraction, times, training triple number:\n");
	vScan("%f %li %li", &fraction, &times, &training);
	return BNN_CrossValidateBad(fraction, times, training);
}


// An initial attempt to cross-validate, now obsolete.
vint8 class_BoostMap_module::BNN_CrossValidateBad(float fraction, vint8 times,
	vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	float error_sum = 0;
	vint8 i;
	for (i = 0; i < times; i++)
	{
		UciSample2(fraction);
		float test_error = BNN_Train("trash", training);
		error_sum = error_sum + test_error;
		float timesf = (float) (i + 1);
		float average = error_sum / timesf;
		vPrint("\n\n\n\n\n%li of (%li) round: current average test error = %f\n\n\n\n\n", 
			(long) (i+1), (long) times, average);
	}

	return 1;
}


// The right way to cross-validate.
vint8 class_BoostMap_module::BNN_CrossValidate0()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long number_of_sets = 0, training = 0;
	vPrint("\nEnter number of sets, training triple number:\n");
	vScan("%li %li", &number_of_sets, &training);
	return BNN_CrossValidate(number_of_sets, training);
}


vint8 class_BoostMap_module::BNN_CrossValidate(vint8 number_of_sets, vint8 training)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	vint8 success = uci_set->Split(number_of_sets);
	if (success == 0)
	{
		vPrint("Failed to split to %li sets\n", (long) number_of_sets);
		return 0;
	}

	float error_sum = 0;
	vint8 i;
	float object_counter = (float) 0;
	for (i = 0; i < number_of_sets; i++)
	{
		uci_set->CrossValidationSet(i);
		float test_error = BNN_Train("trash", training);
		float current_counter = (float) bnn->TestNumber();
		error_sum = error_sum + test_error * current_counter;
		object_counter = object_counter + current_counter;
		float average = error_sum / object_counter;
		vPrint("\n\n\n\n\n%li of (%li) round: current average test error = %f\n\n\n\n\n", 
			(long) (i+1), (long) number_of_sets, average);
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_MaxZ0()
{
	float new_max_z = 0;
	vPrint("\nEnter new bnn_max_z:\n");
	vScan("%f", &new_max_z);
	return BNN_MaxZ(new_max_z);
}


vint8 class_BoostMap_module::BNN_MaxZ(float new_max_z)
{
	if ((new_max_z < 0) || (new_max_z > 1))
	{
		vPrint("Bad bnn_max_z value: %f. Should be in [0,1]\n", new_max_z);
		return 0;
	}

	bnn_max_z = new_max_z;
	vPrint("New bnn_max_z: %f\n", bnn_max_z);
	return 1;
}


vint8 class_BoostMap_module::BNN_AddClassifier0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long index = 0;
	float weight = (float) 0;
	vPrint("\nEnter index, weight:\n");
	vScan("%li %f", &index, &weight);
	return BNN_AddClassifier(index, weight);
}


vint8 class_BoostMap_module::BNN_AddClassifier(vint8 index, float weight)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->AddClassifier(index, weight);
	bnn->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BNN_AddClassifiers0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long start = 0, end = 0;
	float weight = (float) 0;
	vPrint("\nEnter start, end, weight:\n");
	vScan("%li %li %f", &start, &end, &weight);
	return BNN_AddClassifiers(start, end, weight);
}


vint8 class_BoostMap_module::BNN_AddClassifiers(vint8 start, vint8 end, float weight)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	vint8 i;
	for (i = start; i <= end; i++)
	{
		bnn->AddClassifier(i, weight);
		bnn->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_Select0()
{
	if (bnns.size() == 0)
	{
		vPrint("\nNo bnns are available\n");
		return 0;
	}

	long index = 0;
	vPrint("\nEnter index:\n");
	vScan("%li", &index);
	return BNN_Select(index);
}


vint8 class_BoostMap_module::BNN_Select(vint8 index)
{
	if (bnns.size() == 0)
	{
		vPrint("\nNo bnns are available\n");
		return 0;
	}
	if ((index < 0) || (index >= (vint8) bnns.size()))
	{
		vPrint("Bad index = %li, size = %li\n", (long) index, (long) bnns.size());
		return 0;
	}

	bnn = bnns[(vector_size) index];
	return 1 ;
}


vint8 class_BoostMap_module::BNN_PrintAll()
{
	vint8 i;
	for (i = 0; i < (vint8) bnns.size(); i++)
	{
		vPrint("\nbnn %li:\n", (long) i);
		bnns[(vector_size) i]->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_EraseAll()
{
	vint8 i;
	for (i = 0; i < (vint8) bnns.size(); i++)
	{
		delete_boosted_similarity(bnns[(vector_size) i]);
	}

	bnns.clear();
	bnn = 0;
	return 1;
}


vint8 class_BoostMap_module::BNN_Number()
{
	vint8 number = bnns.size();
	vPrint("\nnumber = %li\n", (long) number);
	return 1;
}


vint8 class_BoostMap_module::BNN_AnalyzeTriples()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->AnalyzeTriples();

	return 1;
}


vint8 class_BoostMap_module::BNN_AnalyzeTestTriples()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->AnalyzeTestTriples();

	return 1;
}


vint8 class_BoostMap_module::BNN_BadTriples()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintBadTriples();

	return 1;
}


vint8 class_BoostMap_module::BNN_BadTriples20()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nEnter start, end:\n");
	vScan("%li %li", &start, &end);
	return BNN_BadTriples2(start, end);
}


vint8 class_BoostMap_module::BNN_BadTriples2(vint8 start, vint8 end)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintBadTriples2(start, end);

	return 1;
}


vint8 class_BoostMap_module::BNN_BadNumberHistogram()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->BadNumberHistogram();

	return 1;
}


vint8 class_BoostMap_module::BNN_QNumbers0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long q = 0;
	vPrint("\nEnter q:\n");
	vScan("%li", &q);
	return BNN_QNumbers(q);
}


vint8 class_BoostMap_module::BNN_QNumbers(vint8 q)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->QNumbers(q);

	return 1;
}


vint8 class_BoostMap_module::BNN_QNumbers20()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nEnter start, end:\n");
	vScan("%li %li", &start, &end);
	return BNN_QNumbers2(start, end);
}


vint8 class_BoostMap_module::BNN_QNumbers2(vint8 start, vint8 end)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->QNumbers2(start, end);

	return 1;
}


vint8 class_BoostMap_module::BNN_RingSize0()
{
	long value = 0;
	vPrint("\nEnter value:\n");
	vScan("%li", &value);
	return BNN_RingSize(value);
}


vint8 class_BoostMap_module::BNN_RingSize(vint8 value)
{
	if (value <= 0) return 0;
	bnn_ring_size = value;

	vint8 number = bnns.size();
	if (number > bnn_ring_size)
	{
		vint8 diff = number - bnn_ring_size;
		vint8 i;
		for (i = 0; i < diff; i++)
		{
			delete_boosted_similarity(bnns[(vector_size) i]);
		}
		for (i = diff; i < number; i++)
		{
			bnns[(vector_size) (i-diff)] = bnns[(vector_size) i];
		}
		for (i = 0; i < diff; i++)
		{
			bnns.pop_back();
		}
	}

	return 1;
}


vint8 class_BoostMap_module::BNN_AddToRing()
{
	if (bnn == 0) 
	{
		return 0;
	}
	// Make sure there is room for the current bnn at the ring
	BNN_RingSize(bnn_ring_size - 1);
	// reset bnn_ring size to the original value.
	BNN_RingSize(bnn_ring_size + 1);
	bnns.push_back(bnn);
	return 1;
}


vint8 class_BoostMap_module::BNN_Majority10()
{
	if (bnns.size() == 0) 
	{
		vPrint("\nNo bnns are available\n");
		return 0;
	}

	else
	{
		vPrint("\nEnter start, end\n");
		long start = 0, end = 0;
		vScan("%li %li", &start, &end);
		return BNN_Majority1(start, end);
	}
}


vint8 class_BoostMap_module::BNN_Majority1(vint8 start, vint8 end)
{
	if ((start < 0) || (end >= (vint8) bnns.size()) || (end < start))
	{
		return 0;
	}

	vector<vBoostNN *> temp_bnns;
	vint8 i;
	for (i = start; i <= end; i++)
	{
		temp_bnns.push_back(bnns[(vector_size) i]);
	}

	vBoostNN::Majority1(&temp_bnns);
	return 1;
}


vint8 class_BoostMap_module::BNN_Ranges()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BNN_Stds()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintDimensionStds();
	return 1;
}


vint8 class_BoostMap_module::BNN_NormalizeRanges()
{
	if (uci_set == 0)
	{
		vPrint("No UCI dataset object is available");
		return 0;
	}

	BNN_Create(1000, 1000);
	bnn->PrintDimensionRanges();
	bnn->NormalizeRanges();
	bnn->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BNN_NormalizeRanges2()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintDimensionRanges();
	bnn->NormalizeRanges();
	bnn->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BNN_NormalizeStds()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintDimensionStds();
	bnn->NormalizeStds();
	bnn->PrintDimensionStds();
	return 1;
}


vint8 class_BoostMap_module::BNN_Square()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->PrintDimensionRanges();
	bnn->SquareEntries();
	bnn->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BNN_KNNTraining0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);
	return BNN_KNNTraining(max_k);
}


vint8 class_BoostMap_module::BNN_KNNTraining(vint8 max_k)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	if (max_k < 0) 
	{
		return 0;
	}

	vMatrix<float> result_train = bnn->KnnTrainingErrors(max_k);
	if (result_train.valid() <= 0)
	{
		print("failed to compute errors\n");
		return 1;
	}

	vMatrix<float> result_test = bnn->KnnTestErrors(max_k);
	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%3li-nn training error = %9.7f, test error = %9.7f\n", 
			(long) k, result_train(k), result_test(k));
	}

	vint8 junk = 0, best_k = 0;
	float best_error = function_image_minimum3(&result_train, &junk, &best_k);
	vPrint("\nBest result: %li-nn training_error = %f, test_error = %f\n", 
		(long) best_k, best_error, result_test(best_k));

	return 1;
}


vint8 class_BoostMap_module::BNN_KNNTest0()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);
	return BNN_KNNTest(max_k);
}


vint8 class_BoostMap_module::BNN_KNNTest(vint8 max_k)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	if (max_k < 0) 
	{
		return 0;
	}

	vMatrix<float> result = bnn->KnnTestErrors(max_k);
	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%3li-nn error: %f\n", (long) k, result(k));
	}

	vint8 junk = 0, best_k = 0;
	float best_error = function_image_minimum3(&result, &junk, &best_k);
	vPrint("\nBest result: %li-nn error = %f\n", (long) best_k, best_error);

	return 1;
}


// Compute the naive k-nn training and test errors for all
// six variations (unnormalized, normalizing ranges, 
// normalizing stds, and also L1 or Euclidean).
vint8 class_BoostMap_module::BNN_NaiveKnn0()
{
	long dataset = 0, max_k = 0;
	vPrint("\nEnter dataset number, max_k:\n");
	vScan("%li %li", &dataset, &max_k);
	BNN_NaiveKnn(dataset, max_k);
	return 1;
}


vMatrix<float> class_BoostMap_module::BNN_NaiveKnn(vint8 dataset, vint8 max_k)
{
	if (max_k < 0)
	{
		return float_matrix();
	}
	UciLoad(dataset);
	if (uci_set == 0)
	{
		vPrint("No UCI dataset is loaded\n");
		return float_matrix();
	}

	BNN_Create(1000, 1000);
	vint8 attributes = bnn->Attributes();

	float weight = (float) .0001;
	vint8 index = 0;
	vMatrix<float> result(3, 6);
	vint8 i;

	// First, get the errors using L1 distances.
	// Get the unnormalized test error;
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors(max_k);
	bnn->KnnTestErrors(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	// Get the test error on normalized range
	BNN_Create(1000, 1000);
	BNN_NormalizeRanges();
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors(max_k);
	bnn->KnnTestErrors(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	// Get the test error on normalized std.
	BNN_Create(1000, 1000);
	BNN_NormalizeStds();
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors(max_k);
	bnn->KnnTestErrors(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	// Now, get the errors using Euclidean distances.
	UciLoad(dataset);
	BNN_Create(1000, 1000);
	//  BNN_Square();
	// Get the unnormalized test error;
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors2(max_k);
	bnn->KnnTestErrors2(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	// Get the test error on normalized range
	BNN_Create(1000, 1000);
	BNN_NormalizeRanges();
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors2(max_k);
	bnn->KnnTestErrors2(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	// Get the test error on normalized std.
	BNN_Create(1000, 1000);
	BNN_NormalizeStds();
	for (i = 0; i < attributes; i++)
	{
		bnn->AddClassifier(i, weight);
	}
	bnn->KnnTrainingErrors2(max_k);
	bnn->KnnTestErrors2(max_k);
	result(0, index) = (float) bnn->BestK();
	result(1, index) = bnn->BestKnnTraining();
	result(2, index) = bnn->BestKnnTest();
	index++;

	result.Print("naive k-nn errors:");
	return result;
}


// Compute the cross-validation k-nn training and test errors
// for all six variations.
vint8 class_BoostMap_module::BNN_NaiveKnnCross0()
{
	long dataset = 0, number = 0, max_k = 0;
	vPrint("\nEnter dataset, number of subsets, max_k:\n");
	vScan("%li %li %li", &dataset, &number, &max_k);
	BNN_NaiveKnnCross(dataset, number, max_k);
	return 1;
}


vMatrix<float> class_BoostMap_module::BNN_NaiveKnnCross(vint8 dataset, vint8 number, 
	vint8 max_k)
{
	UciLoad(dataset);
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return vMatrix<float>();
	}

	vint8 success = uci_set->Split(number);
	if (success == 0)
	{
		vPrint("Failed to split to %li sets\n", (long) number);
		return vMatrix<float>();
	}

	vMatrix<float> result(2, 6);
	float training_number_sum, test_number_sum;
	vint8 index = 0;
	vint8 i, j;
	const float weight = (float) .0001;

	// First, get the L1 distances (we will have to use a different
	// split for the Euclidean distances).

	// Get the unnormalized errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors(max_k);
		bnn->KnnTestErrors(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	// Get the normalized-range errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		BNN_NormalizeRanges2();
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors(max_k);
		bnn->KnnTestErrors(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	// Get the normalized-std errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		BNN_NormalizeStds();
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors(max_k);
		bnn->KnnTestErrors(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	// Now get the Euclidean distances (we will have to use a different
	// split for the Euclidean distances).
	UciLoad(dataset);
	uci_set->Split(number);
	uci_set->CrossValidationSet(0);
	BNN_Create(1000, 1000);
	//  BNN_Square();

	// Get the unnormalized errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors2(max_k);
		bnn->KnnTestErrors2(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	// Get the normalized-range errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		BNN_NormalizeRanges2();
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors2(max_k);
		bnn->KnnTestErrors2(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	// Get the normalized-std errors.
	result(0, index) = (float) 0;
	result(1, index) = (float) 0;
	training_number_sum = 0;
	test_number_sum = 0;
	for (i = 0; i < number; i++)
	{
		uci_set->CrossValidationSet(i);
		BNN_Create(1000, 1000);
		BNN_NormalizeStds();
		vint8 attributes = bnn->Attributes();

		for (j = 0; j < attributes; j++)
		{
			bnn->AddClassifier(j, weight);
		}
		bnn->KnnTrainingErrors2(max_k);
		bnn->KnnTestErrors2(max_k);

		float training_number = (float) bnn->TrainingNumber();
		float test_number = (float) bnn->TestNumber();
		float training_error = bnn->BestKnnTraining();
		float test_error = bnn->BestKnnTest();

		result(0, index) += training_number * training_error;
		result(1, index) += test_number * test_error;
		training_number_sum += training_number;
		test_number_sum += test_number;
	}

	result(0, index) = result(0, index) / training_number_sum;
	result(1, index) = result(1, index) / test_number_sum;
	index++;

	result.Print("cross-validation knn errors");    

	return result;
}


/////////////////////////////////////////////////////////////////////
//                                                                 //
//             End of functions written for ICML 2004.             //
//                                                                 //
/////////////////////////////////////////////////////////////////////



vint8 class_BoostMap_module::BnnqsCreate0()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 1;
	}

	long training = 0, validation = 0;
	vPrint("\nEnter training, validation:\n");
	vScan("%li %li", &training, &validation);
	return BnnqsCreate(training, validation);
}

vint8 class_BoostMap_module::BnnqsCreate(vint8 training, vint8 validation)
{
	vMatrix<float> training_vectors = uci_set->TrainingVectors();
	vMatrix<vint8> training_labels = uci_set->TrainingLabels();
	vMatrix<float> test_vectors = uci_set->TestVectors();
	vMatrix<vint8> test_labels = uci_set->TestLabels();

	vdelete(bnnqs);
	bnnqs = new vBnnqs(training_vectors, training_labels, 
		test_vectors, test_labels,
		training, validation);
	bnnqs->SetAllowNegativeWeights(bnn_allow_negative);
	if (uci_set->SubjectIdsAvailable() == 1)
	{
		vMatrix<vint8> subject_ids = uci_set->SubjectIds();
		bnnqs->SetSubjectIds(subject_ids);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsPrintSummary()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsPrintTraining()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintTraining();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsPrintValidation()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintValidation();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsTestTriples()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintTestTriples();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsTripleStats()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintTripleStats();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsConfusions()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintConfusions();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsClassifier()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		bnnqs->PrintClassifier();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsTriple()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 1;
	}

	long index = 0;
	vPrint("\nEnter index:\n");
	vScan("%li", &index);
	bnnqs->PrintTrainingTriple(index);

	return 1;
}


vint8 class_BoostMap_module::BnnqsWeight()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 1;
	}

	long rank = 0;
	vPrint("\nEnter rank:\n");
	vScan("%li", &rank);
	bnnqs->PrintWeight(rank);

	return 1;
}


vint8 class_BoostMap_module::BnnqsWeights()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long period = 0;
		vPrint("\nEnter period:\n");
		vScan("%li", &period);
		bnnqs->PrintWeights(period);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsSomeWeights()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long start = 0, end = 0, period = 0;
		vPrint("\nEnter start, end, period:\n");
		vScan("%li %li %li", &start, &end, &period);
		bnnqs->PrintSomeWeights(start, end, period);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsWeightSum()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long start = 0, end = 0;
		vPrint("\nEnter start, end:\n");
		vScan("%li %li", &start, &end);
		bnnqs->PrintWeightSum(start, end);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsSortWeights()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}

	else
	{
		bnnqs->SortTrainingWeights();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsZContributions()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long index = 0;
		vPrint("\nEnter dimension index:\n");
		vScan("%li", &index);
		bnnqs->SortZContributions(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsKnnErrors()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long k = 0;
		vPrint("\nEnter k:\n");
		vScan("%li", &k);
		bnnqs->KnnErrors(k);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsRankInfo()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long index = 0;
		vPrint("\nEnter triple index:\n");
		vScan("%li", &index);
		bnnqs->TripleRankInfo(index);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsKnnInfo()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
	}
	else
	{
		long index = 0, k = 0;
		vPrint("\nEnter object index, k:\n");
		vScan("%li %li", &index, &k);
		bnnqs->KnnInfo(index, k);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsStep0()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);

	return BnnqsStep(buffer);
}


vint8 class_BoostMap_module::BnnqsStep(const char * filename)
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->NextStep();
	bnnqs->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BnnqsSteps()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	char buffer[1000];
	long steps = 0;
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li", buffer, &steps);

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		BnnqsStep(buffer);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsNewBNN0()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);
	return BnnqsNewBNN(max_k);
}


vint8 class_BoostMap_module::BnnqsNewBNN(vint8 max_k)
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	if (max_k < 1)
	{
		return 0;
	}

	vBnnqs * new_bnnqs = bnnqs->NewBnnqs(max_k);
	bnnqs = new_bnnqs;
	bnnqs->PrintSummary();
	return 1;
}



vint8 class_BoostMap_module::BnnqsNewBNN2()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	vBnnqs * new_bnnqs = bnnqs->NewBnnqs2();
	bnnqs = new_bnnqs;
	bnnqs->PrintSummary();
	return 1;
}


//vint8 class_BoostMap_module::BnnqsBacktrack()
//{
//  if (bnnqs == 0)
//  {
//    vPrint("No vBoostNN-QS object is available");
//    return 0;
//  }
//
//  vint8 steps = 0;
//  vPrint("\nEnter steps:\n");
//  vScan("%li", &steps);
//  bnnqs->Backtrack(steps);
//
//  return 1;
//}


vint8 class_BoostMap_module::BnnqsLoad()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);

	vPrint("not implemented yet\n");
	//  bnnqs->Load(filename);
	return 1;
}


vint8 class_BoostMap_module::BnnqsSave()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);

	vPrint("not implemented yet\n");
	//  bnnqs->Save(filename);
	return 1;
}




vint8 class_BoostMap_module::BnnqsAllowNegative()
{
	long value = 0;
	vPrint("\nEnter value:\n");
	vScan("%li", &value);

	bnn_allow_negative = value;
	if (bnnqs != 0)
	{
		bnnqs->SetAllowNegativeWeights(value);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsTrain0()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	vPrint("\nEnter training triple number:\n");
	vScan("%li", &training);

	BnnqsTrain(training);
	return 1;
}


float class_BoostMap_module::BnnqsTrain(vint8 training)
{
	const vint8 max_k = 101;

	const vint8 validation_triples = training;
	float last_training_error = 2;
	float last_test_error = 2;
	vint8 last_best_k = 1;

	// rank_limit is not actually used, but it was an attempt to
	// set the max_k argument to ResampleTriples automatically.
	vint8 rank_limit = 2;
	vint8 counter = 0;
	vint8 classes = uci_set->Classes();
	BnnqsNormalizeRanges();
	BnnqsCreate(training, validation_triples);
	while(1)
	{
		while (1)
		{
			BnnqsStep("trash");
			float z = bnnqs->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnnqs->TrainingError();
			// generate new training in case the previous was overfitted.
			//      if (training_triple_error < bnnqs->ValidationError() / 2.0)
			//      {
			//        bnnqs->NewTrainingSet(2);
			//        bnnqs->PrintSummary();
			//      }
			if (bnnqs->TrainingError() == 0)
			{
				break;
			}
		}
		float training_error = 0, test_error = 0;
		bnnqs->KnnTrainingErrors(max_k);
		bnnqs->KnnTestErrors(max_k);
		training_error = bnnqs->BestKnnTraining();
		test_error = bnnqs->BestKnnTest();
		vint8 best_k = bnnqs->BestK();
		vPrint("best_k = %li, training_error = %f, test_error = %f\n",
			(long) best_k, training_error, test_error);
		vPrint("\n------------------------\n\n");

		if (counter == 1)
		{
			last_training_error = training_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}
		else if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}

		rank_limit = last_best_k/2 + 1;
		BnnqsNewBNN(2);
		counter++;
	}

	vPrint("best k = %li, training error = %f, test error = %f\n",
		(long) last_best_k, last_training_error, last_test_error);

	vPrint("\n-------------------------------------------------\n");
	vPrint("#################################################\n");
	vPrint("-------------------------------------------------\n");

	return last_test_error;
}


vint8 class_BoostMap_module::BnnqsTrain2()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	vPrint("\nEnter training triple number:\n");
	vScan("%li", &training);

	const vint8 validation_triples = 1000;
	float last_training_error = 2;
	float last_test_error = 2;
	BnnqsCreate(10000, validation_triples);
	vint8 attributes = bnnqs->Attributes();
	BnnqsAddClassifiers((vint8) 0, (vint8) attributes-1, (float) 0.0001);

	float training_error = 0, test_error = 0;
	bnnqs->KnnErrors(1, &test_error, &training_error);
	vPrint("test_error = %f, training_error = %f\n",
		test_error, training_error);
	last_training_error = training_error;
	last_test_error = test_error;
	vPrint("\n------------------------\n\n");
	BnnqsNewBNN(2);

	while(1)
	{
		while (1)
		{
			BnnqsStep("trash");
			float z = bnnqs->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnnqs->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}
		bnnqs->KnnErrors(1, &test_error, &training_error);
		vPrint("test_error = %f, training_error = %f\n",
			test_error, training_error);
		if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			vPrint("\n------------------------\n\n");
			BnnqsNewBNN(2);
		}
	}

	vPrint("best training error = %f, test error = %f\n",
		last_training_error, last_test_error);

	return 1;
}


vint8 class_BoostMap_module::BnnqsTrain3()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long training = 0;
	vPrint("\nEnter training triple number:\n");
	vScan("%li", &training);

	const vint8 validation_triples = 1000;
	float last_training_error = 2;
	float last_test_error = 2;
	BnnqsCreate(training, validation_triples);
	while(1)
	{
		while (1)
		{
			BnnqsStep("trash");
			float z = bnnqs->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnnqs->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}
		float training_error = 0, test_error = 0;
		bnnqs->KnnErrors(1, &test_error, &training_error);
		vPrint("test_error = %f, training_error = %f\n",
			test_error, training_error);
		if (training_error >= last_training_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_test_error = test_error;
			vPrint("\n------------------------\n\n");
			BnnqsNewBNN2();
		}
	}

	vPrint("best training error = %f, test error = %f\n",
		last_training_error, last_test_error);

	return 1;
}


vint8 class_BoostMap_module::BnnqsTrainLoop()
{
	char filename[1000];
	long dataset = 0, training = 0;

	vPrint("\nEnter filename, dataset, training triple number:\n");
	vScan("%s %li %li", filename, &dataset, &training);

	UciLoad(dataset);
	BnnqsCreate(10000, 10000);
	BnnqsNormalizeRanges();

	vint8 counter = 0;
	while(1)
	{
		BnnqsTrain(training);

		vint8 number = bnns.size();
		vint8 i;
		vint8 counter2 = 0;
		for (i = number - 1; i >= 0; i--)
		{
			char * string1 = string_from_number(counter);
			char * string2 = string_from_number(counter2);
			char * name = vMergeStrings5(filename, "_", string1, "_", string2);
			bnns[(vector_size) i]->Save(name);
			counter2 = counter2 + 1;
			vdelete2(string1);
			vdelete2(string2);
			vdelete2(name);
		}

		counter = counter + 1;
	}

	// this function never returns.
	return 1;
}


vint8 class_BoostMap_module::BnnqsTrainLoop2()
{
	char filename[1000];
	long dataset = 0, training = 0;
	float fraction = 0;

	vPrint("\nEnter filename, dataset, training triple number, fraction:\n");
	vScan("%s %li %li %f", filename, &dataset, &training, &fraction);

	UciLoad(dataset);

	vint8 counter = 0;
	while(1)
	{
		UciSample(fraction);
		BnnqsTrain(training);

		vint8 number = bnns.size();
		vint8 i;
		vint8 counter2 = 0;
		for (i = number - 1; i >= 0; i--)
		{
			char * string1 = string_from_number(counter);
			char * string2 = string_from_number(counter2);
			char * name = vMergeStrings5(filename, "_", string1, "_", string2);
			bnns[(vector_size) i]->Save(name);
			counter2 = counter2 + 1;
			vdelete2(string1);
			vdelete2(string2);
			vdelete2(name);
		}

		counter = counter + 1;
	}

	// this function never returns.
	return 1;
}


// The right way to cross-validate.
vint8 class_BoostMap_module::BnnqsCrossValidate()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long number_of_sets = 0, training = 0;
	vPrint("\nEnter number of sets, training triple number:\n");
	vScan("%li %li", &number_of_sets, &training);

	vint8 success = uci_set->Split(number_of_sets);
	if (success == 0)
	{
		vPrint("Failed to split to %li sets\n", (long) number_of_sets);
		return 0;
	}

	float error_sum = 0;
	vint8 i;
	float object_counter = (float) 0;
	for (i = 0; i < number_of_sets; i++)
	{
		uci_set->CrossValidationSet(i);
		float test_error = BnnqsTrain(training);
		float current_counter = (float) bnnqs->TestNumber();
		error_sum = error_sum + test_error * current_counter;
		object_counter = object_counter + current_counter;
		float average = error_sum / object_counter;
		vPrint("\n\n\n\n\n%li of (%li) round: current average test error = %f\n\n\n\n\n", 
			(long) (i+1), (long) number_of_sets, average);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsAddClassifier()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long index = 0;
	float weight = (float) 0;
	vPrint("\nEnter index, weight:\n");
	vScan("%li %f", &index, &weight);

	bnnqs->AddClassifier(index, weight);
	bnnqs->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::BnnqsAddClassifiers0()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long start = 0, end = 0;
	float weight = (float) 0;
	vPrint("\nEnter start, end, weight:\n");
	vScan("%li %li %f", &start, &end, &weight);

	return BnnqsAddClassifiers(start, end, weight);
}


vint8 class_BoostMap_module::BnnqsAddClassifiers(vint8 start, vint8 end, float weight)
{
	vint8 i;
	for (i = start; i <= end; i++)
	{
		bnnqs->AddClassifier(i, weight);
		bnnqs->PrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsAnalyzeTriples()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->AnalyzeTriples();

	return 1;
}


vint8 class_BoostMap_module::BnnqsAnalyzeTestTriples()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->AnalyzeTestTriples();

	return 1;
}


vint8 class_BoostMap_module::BnnqsBadTriples()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->PrintBadTriples();

	return 1;
}


vint8 class_BoostMap_module::BnnqsBadTriples2()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nEnter start, end:\n");
	vScan("%li %li", &start, &end);

	bnnqs->PrintBadTriples2(start, end);

	return 1;
}


vint8 class_BoostMap_module::BnnqsBadNumberHistogram()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->BadNumberHistogram();

	return 1;
}


vint8 class_BoostMap_module::BnnqsQNumbers()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long q = 0;
	vPrint("\nEnter q:\n");
	vScan("%li", &q);
	bnnqs->QNumbers(q);

	return 1;
}


vint8 class_BoostMap_module::BnnqsQNumbers2()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long start = 0, end = 0;
	vPrint("\nEnter start, end:\n");
	vScan("%li %li", &start, &end);

	bnnqs->QNumbers2(start, end);

	return 1;
}


vint8 class_BoostMap_module::BnnqsRanges()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BnnqsStds()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->PrintDimensionStds();
	return 1;
}


vint8 class_BoostMap_module::BnnqsNormalizeRanges()
{
	if (uci_set == 0)
	{
		vPrint("No UCI dataset object is available");
		return 0;
	}

	BnnqsCreate(1000, 1000);
	bnnqs->PrintDimensionRanges();
	bnnqs->NormalizeRanges();
	bnnqs->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BnnqsNormalizeRanges2()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->PrintDimensionRanges();
	bnnqs->NormalizeRanges();
	bnnqs->PrintDimensionRanges();
	return 1;
}


vint8 class_BoostMap_module::BnnqsNormalizeStds()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->PrintDimensionStds();
	bnnqs->NormalizeStds();
	bnnqs->PrintDimensionStds();
	return 1;
}


vint8 class_BoostMap_module::BnnqsKNNTraining()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);

	if (max_k < 0) 
	{
		return 0;
	}

	vMatrix<float> result_train = bnnqs->KnnTrainingErrors(max_k);
	vMatrix<float> result_test = bnnqs->KnnTestErrors(max_k);
	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%3li-nn training error = %9.7f, test error = %9.7f\n", 
			(long) k, result_train(k), result_test(k));
	}

	vint8 junk = 0, best_k = 0;
	float best_error = function_image_minimum3(&result_train, &junk, &best_k);
	vPrint("\nBest result: %li-nn training_error = %f, test_error = %f\n", 
		(long) best_k, best_error, result_test(best_k));

	return 1;
}


vint8 class_BoostMap_module::BnnqsKNNTest()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long max_k = 0;
	vPrint("\nEnter max_k:\n");
	vScan("%li", &max_k);

	if (max_k < 0) 
	{
		return 0;
	}

	vMatrix<float> result = bnnqs->KnnTestErrors(max_k);
	vint8 k;
	for (k = 1; k <= max_k; k++)
	{
		vPrint("%3li-nn error: %f\n", (long) k, result(k));
	}

	vint8 junk = 0, best_k = 0;
	float best_error = function_image_minimum3(&result, &junk, &best_k);
	vPrint("\nBest result: %li-nn error = %f\n", (long) best_k, best_error);

	return 1;
}


vint8 class_BoostMap_module::BnnqsStepqs0()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long number = 0;
	vPrint("\nEnter number:\n");
	vScan("%li", &number);
	return BnnqsStepqs(number);
}


vint8 class_BoostMap_module::BnnqsStepqs(vint8 number)
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	bnnqs->NextStepQs(number);
	bnnqs->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::BnnqsStepsqs()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long number = 0, steps = 0;
	vPrint("\nEnter number, steps\n");
	vScan("%li %li", &number, &steps);

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		BnnqsStepqs(number);
	}

	return 1;
}


vint8 class_BoostMap_module::BnnqsCheck()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long test = 0, training = 0;
	vPrint("\nEnter test, training\n");
	vScan("%li %li", &test, &training);

	bnnqs->CheckPair(test, training);
	return 1;
}


vint8 class_BoostMap_module::BnnqsCheckTr()
{
	if (bnnqs == 0)
	{
		vPrint("No vBoostNN-QS object is available");
		return 0;
	}

	long training1 = 0, training2 = 0;
	vPrint("\nEnter training1, training2\n");
	vScan("%li %li", &training1, &training2);

	bnnqs->CheckPairTraining(training1, training2);
	return 1;
}


vint8 class_BoostMap_module::VpNew()
{
	char dataset[1000];
	vPrint("\nEnter dataset:\n");
	vScan("%s", dataset);

	VpDelete();
	vp_tree = new vVpTree(dataset);
	vPrint("vp_tree ready\n");
	return 1;
}


vint8 class_BoostMap_module::VpDelete()
{
	vdelete(vp_tree);
	vp_tree = 0;

	return 1;
}


vint8 class_BoostMap_module::VpPrint()
{
	if (vp_tree == 0)
	{
		return 0;
	}

	long levels = 0;
	vPrint("\nEnter levels:\n");
	vScan("%li", &levels);
	vp_tree->Print(levels);

	return 1;
}


vint8 class_BoostMap_module::VpLoad()
{
	char filename[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", filename);

	VpDelete();
	vp_tree = new vVpTree();
	vint8 success = vp_tree->Load(filename);
	if (success <= 0)
	{
		vPrint("failed to load vp tree from %s\n", filename);
		VpDelete();
	}

	vPrint("loaded vp tree successfully\n");
	return 1;
}


vint8 class_BoostMap_module::VpSave()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	char filename[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", filename);

	vint8 success = vp_tree->Save(filename);
	if (success <= 0)
	{
		vPrint("failed to save vp tree to %s\n", filename);
	}

	vPrint("saved vp tree successfully\n");

	return 1;
}


vint8 class_BoostMap_module::VpTestRange()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	char dataset[1000];
	long index = 0;
	float radius = (float) 0;
	vPrint("\nEnter dataset, index, radius:\n");
	vScan("%s %li %f", dataset, &index, &radius);

	vp_tree->TestRangeSearch(dataset, index, radius);
	return 1;
}


vint8 class_BoostMap_module::VpTestNn()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	char dataset[1000];
	long index = 0;
	vPrint("\nEnter dataset, index:\n");
	vScan("%s %li", dataset, &index);

	vp_tree->TestNnSearch(dataset, index);
	return 1;
}


vint8 class_BoostMap_module::VpTestKnn()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	char dataset[1000];
	long index = 0, number = 0;
	vPrint("\nEnter dataset, index, number:\n");
	vScan("%s %li %li", dataset, &index, & number);

	vp_tree->TestKnnSearch(dataset, index, number);
	return 1;
}


vint8 class_BoostMap_module::VpNnStats()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	char dataset[1000];
	vPrint("\nEnter dataset:\n");
	vScan("%s", dataset);

	float average_distances = (float) 0;
	vp_tree->NnSearchStats(dataset, &average_distances);
	return 1;
}


vint8 class_BoostMap_module::VpKnnStats()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	long number = 0;
	char dataset[1000];
	float constant = (float) 1;
	vPrint("\nEnter dataset, number, constant:\n");
	vScan("%s %li %f", dataset, & number, &constant);

	vPrint("constant = %f\n", constant);
	float average_distances = (float) 0;
	vp_tree->set_constant(constant);
	vp_tree->KnnSearchStats(dataset, &average_distances, number);
	return 1;
}


static vint8 PrintClassificationError(vMatrix<float> result)
{
	vint8 max_k = result.Size() - 1;

	vint8 i;
	vint8 min_index = -1;
	float min_error = (float) 1000;
	for (i = 0; i <= max_k; i++)
	{
		float error = result(i);
		vPrint("%li-nn error: %f\n", (long) i, error);
		if (i != 0)
		{ 
			if (error < min_error)
			{
				min_error = error;
				min_index = i;
			}
		}
	}
	vPrint("1-nn error: %f\n", result(1));
	vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);

	return 1;
}

vint8 class_BoostMap_module::VpKnnClassification()
{
	if (vp_tree == 0)
	{ 
		return 0;
	}

	long number = 0;
	float constant = 0;
	char dataset[1000];
	vPrint("\nEnter dataset, constant, number:\n");
	vScan("%s %f %li", dataset, & constant, & number);

	float average_distances = (float) 0;
	vp_tree->set_constant(constant);
	vMatrix<float> result = vp_tree->KnnClassificationStats(dataset, 
		&average_distances, number);

	if (result.valid() <0)
	{
		vPrint("failed to estimate classification error\n");
		return 0;
	}

	PrintClassificationError(result);
	return 1;
}


vint8 class_BoostMap_module::similarity_new()
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
	}
	else
	{
		vMatrix<float> training_vectors = uci_set->TrainingVectors();
		vMatrix<vint8> training_labels = uci_set->TrainingLabels();
		vMatrix<float> test_vectors = uci_set->TestVectors();
		vMatrix<vint8> test_labels = uci_set->TestLabels();

		bnn = new similarity_learning(training_vectors, training_labels, 
			test_vectors, test_labels,
			10000, 10000);
		bnn->SetAllowNegativeWeights(bnn_allow_negative);
		if (uci_set->SubjectIdsAvailable() == 1)
		{
			vMatrix<vint8> subject_ids = uci_set->SubjectIds();
			bnn->SetSubjectIds(subject_ids);
		}
		bnn->NormalizeRanges();
		BNN_AddToRing();
	}

	return 1;
}

// this function is called whenever we want to verify that there is a current
// boost-NN object whose actual subclass is similarity_learning.
vint8 class_BoostMap_module::similarity_learning_check()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char * class_name = function_copy_string(bnn->get_class_name());
	if (function_compare_strings(class_name, "similarity_learning") != 0)
	{
		function_print("\ncurrent vBoostNN object is of class %s\n", class_name);
		delete_pointer(class_name);
		return 0;
	}
	delete_pointer(class_name);
	return 1;
}


vint8 class_BoostMap_module::similarity_change_triples()
{
	if (similarity_learning_check() <= 0)
	{
		return 0;
	}

	long neighbor_number = 0;
	print("\nenter number of neighbors from same class:\n");
	question("%li", & neighbor_number);

	similarity_change_triples_auxiliary(neighbor_number);

	return 1;
}


vint8 class_BoostMap_module::similarity_change_triples_auxiliary(vint8 neighbor_number)
{
	if (similarity_learning_check() <= 0)
	{
		return 0;
	}

	similarity_learning * similarity = (similarity_learning *) bnn;
	similarity->set_second_triples(neighbor_number);
	BNN_PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::similarity_third_triples()
{
	if (similarity_learning_check() <= 0)
	{
		return 0;
	}

	long neighbor_number = 0, cutoff = 0;
	print("\nenter number of neighbors from same class, cutoff:\n");
	question("%li %li", & neighbor_number, & cutoff);

	similarity_third_triples_auxiliary(neighbor_number, cutoff);

	return 1;
}


vint8 class_BoostMap_module::similarity_third_triples_auxiliary(vint8 neighbor_number, vint8 cutoff)
{
	if (similarity_learning_check() <= 0)
	{
		return 0;
	}

	similarity_learning * similarity = (similarity_learning *) bnn;
	similarity->set_third_triples(neighbor_number, cutoff);
	BNN_PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::similarity_train()
{
	long neighbor_number = 0, cutoff = 0;

	print("\nenter number of neighbors from same class, cutoff:\n");
	question("%li %li", & neighbor_number, & cutoff);

	similarity_train_auxiliary(neighbor_number, cutoff);
	return 1;
}


float class_BoostMap_module::similarity_train_auxiliary(vint8 neighbor_number, vint8 cutoff)
{
	if (uci_set == 0)
	{
		vPrint("\nNo UCI set loaded\n");
		return -100000000.0f;
	}
	const vint8 max_k = 101;

	if (neighbor_number > max_k)
	{
		vPrint("\nneighbor_number > max_k\n");
		return 0;
	}

	float last_training_error = 2;
	float last_triple_error = 2;
	float last_test_error = 2;
	vint8 last_best_k = 1;

	// rank_limit is not actually used, but it was an attempt to
	// set the max_k argument to ResampleTriples automatically.
	vint8 rank_limit = 2;
	vint8 counter = 0;
	vint8 classes = uci_set->Classes();
	similarity_new();
	while(1)
	{
		while (1)
		{
			BNN_Step("trash");
			float z = bnn->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnn->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}

		float training_error = 0, test_error = 0;
		float_matrix training_error_matrix = bnn->KnnTrainingErrors(max_k);
		float_matrix test_error_matrix = bnn->KnnTestErrors(max_k);

		training_error = training_error_matrix(neighbor_number);
		test_error = test_error_matrix(neighbor_number);

		float best_training_error = bnn->BestKnnTraining();
		float best_test_error = bnn->BestKnnTest();
		vint8 best_k = bnn->BestK();
		float triple_error = bnn->TrainingError();
		print("counter = %li, last_triple_error = %f, triple_error = %f\n", 
			(long) counter, last_triple_error, triple_error);
		vPrint("best_k = %li, best training_error = %f, best_test_error = %f\n",
			best_k, best_training_error, best_test_error);
		vPrint("training_error = %f, test_error = %f\n",
			training_error, test_error);
		vPrint("\n------------------------\n\n");

		if (counter == 1)
		{
			last_training_error = training_error;
			last_triple_error = triple_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}
		else if (triple_error >= last_triple_error)
		{
			break;
		}
		else
		{
			last_training_error = training_error;
			last_triple_error = triple_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}

		//    if (counter > 0)
		{
			break;
		}
		//    similarity_change_triples_auxiliary(neighbor_number);
		//    counter++;
	}

	// now repeat training with cutoffs
	while(1)
	{
		similarity_third_triples_auxiliary(neighbor_number, cutoff);
		while (1)
		{
			BNN_Step("trash");
			float z = bnn->LastZ();
			if (z >= bnn_max_z)
			{
				break;
			}
			float training_triple_error = bnn->TrainingError();
			if (training_triple_error == 0)
			{
				break;
			}
		}
		float training_error = 0, test_error = 0;
		float_matrix training_error_matrix = bnn->KnnTrainingErrors(max_k);
		float_matrix test_error_matrix = bnn->KnnTestErrors(max_k);

		training_error = training_error_matrix(neighbor_number);
		test_error = test_error_matrix(neighbor_number);

		float best_training_error = bnn->BestKnnTraining();
		float best_test_error = bnn->BestKnnTest();
		vint8 best_k = bnn->BestK();
		float triple_error = bnn->TrainingError();
		print("counter = %li, last_triple_error = %f, triple_error = %f\n", 
			(long) counter, last_triple_error, triple_error);
		vPrint("best_k = %li, best training_error = %f, best_test_error = %f\n",
			best_k, best_training_error, best_test_error);
		vPrint("training_error = %f, test_error = %f\n",
			training_error, test_error);
		vPrint("\n------------------------\n\n");

		if (counter == 1)
		{
			last_training_error = training_error;
			last_triple_error = triple_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}
		else if (triple_error >= last_triple_error)
		{
			last_test_error = test_error;
			break;
		}
		else
		{
			last_training_error = training_error;
			last_triple_error = triple_error;
			last_test_error = test_error;
			last_best_k = (vint8) best_k;
		}

		counter++;
		if (counter > 20)
		{
			break;
		}
	}

	vPrint("best k = %li, training error = %f, test error = %f, triple_error = %f\n",
		(long) last_best_k, last_training_error, last_test_error, last_triple_error);

	vPrint("\n-------------------------------------------------\n");
	vPrint("#################################################\n");
	vPrint("-------------------------------------------------\n");

	return last_test_error;
}


vint8 class_BoostMap_module::similarity_cross_validate()
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	long number_of_sets = 0, neighbor_number = 0, cutoff = 0;

	vPrint("\nEnter number of sets, number of neighbors, cutoff:\n");
	vScan("%li %li %li", &number_of_sets, & neighbor_number, & cutoff);

	return similarity_cross_validate_auxiliary(number_of_sets, neighbor_number, cutoff);
}


vint8 class_BoostMap_module::similarity_cross_validate_auxiliary(vint8 number_of_sets, vint8 neighbor_number, vint8 cutoff)
{
	if (uci_set == 0)
	{
		vPrint("No UCI set loaded");
		return 0;
	}

	vint8 success = uci_set->Split(number_of_sets);
	if (success == 0)
	{
		vPrint("Failed to split to %li sets\n", number_of_sets);
		return 0;
	}

	float error_sum = 0;
	vint8 i;
	float object_counter = (float) 0;
	for (i = 0; i < number_of_sets; i++)
	{
		uci_set->CrossValidationSet(i);
		float test_error = similarity_train_auxiliary(neighbor_number, cutoff);
		float current_counter = (float) bnn->TestNumber();
		error_sum = error_sum + test_error * current_counter;
		object_counter = object_counter + current_counter;
		float average = error_sum / object_counter;
		vPrint("\n\n\n\n\n%li of (%li) round: current average test error = %f\n\n\n\n\n", 
			(long) (i+1), (long) number_of_sets, average);
	}

	return 1;
}


vint8 class_BoostMap_module::similarity_normalize_factors()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->normalize_factors();
	return 1;
}


vint8 class_BoostMap_module::similarity_simultaneous_step()
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return similarity_simultaneous_step(buffer);
}


vint8 class_BoostMap_module::similarity_simultaneous_step(const char * filename)
{
	if (bnn == 0)
	{
		vPrint("\nNo vBoostNN object is available\n");
		return 0;
	}

	bnn->simultaneous_step();
	bnn->Save(filename);
	bnn->PrintSummary();
	return 1;
}


vint8 class_BoostMap_module::similarity_AdaBoost_create()
{
	if (bnn == 0)
	{
		print ("\nNo bnn object available\n");
		return 0;
	}

	AdaBoost_destroy();

	long number = 0;
	print ("\nenter number of neighbors:\n");
	question ("%li", & number);
	boosting = bnn->binary_learner(number);

	return 1;
}


vint8 class_BoostMap_module::similarity_next_level()
{
	if (bnn == 0)
	{
		print ("\nNo bnn object available\n");
		return 0;
	}

	if (boosting == 0)
	{
		print ("\nNo boosting object available\n");
		return 0;
	}

	float threshold = 0;
	function_print("\nenter threshold:\n");
	question("%f", & threshold);

	vBoostNN * new_bnn = bnn->next_level(boosting, threshold);
	if (new_bnn == 0)
	{
		print("\nfailed to create new boost-nn object\n");
		return 0;
	}

	bnn = new_bnn; 
	bnn->PrintSummary();
	BNN_AddToRing();

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_synthetic()
{
	AdaBoost_destroy();

	long objects = 0, features = 0, noise = 0;
	print("\nenter objects, features, noise:\n");
	question("%li %li %li", & objects, & features, & noise);

	boosting = synthetic_AdaBoost(objects, features, noise);
	AdaBoost_print();

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_destroy()
{
	function_delete(boosting);

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_print()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	boosting->print();
	return 1;
}


vint8 class_BoostMap_module::AdaBoost_errors()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	float threshold = 0;
	function_print("\nenter threshold:\n");
	question("%f", &threshold);

	boosting->print_errors(threshold);
	return 1;
}


vint8 class_BoostMap_module::AdaBoost_factors()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	boosting->print_factors();
	return 1;
}


vint8 class_BoostMap_module::AdaBoost_save()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	char name[1000];
	function_print("\nenter filename:\n");
	question("%s", name);

	return AdaBoost_save(name);
}


vint8 class_BoostMap_module::AdaBoost_save(const char * name)
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	vint8 success = boosting->save(name);
	return success;
}


vint8 class_BoostMap_module::AdaBoost_load()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	char name[1000];
	function_print("\nenter filename:\n");
	question("%s", name);

	return AdaBoost_load(name);
}


vint8 class_BoostMap_module::AdaBoost_load(const char * name)
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	vint8 success = boosting->load(name);
	return success;
}


vint8 class_BoostMap_module::AdaBoost_step()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	char name[1000];
	function_print("\nenter filename:\n");
	question("%s", name);

	return AdaBoost_step(name);
}


vint8 class_BoostMap_module::AdaBoost_step(const char * filename)
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	boosting->next_step();
	boosting->print();
	AdaBoost_save(filename);

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_steps()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	char name[1000];
	long steps = 0;
	function_print("\nenter filename, steps:\n");
	question("%s %li", name, & steps);

	return AdaBoost_steps(name, steps);
}


vint8 class_BoostMap_module::AdaBoost_steps(const char * filename, vint8 steps)
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	vint8 counter;
	for (counter = 0; counter < steps; counter++)
	{
		AdaBoost_step(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_train()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	return 1;
}


vint8 class_BoostMap_module::AdaBoost_train(const char * filename)
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	boosting->train();
	boosting->print();
	boosting->print_errors();
	boosting->save(filename);

	return 1;
}


// find the threshold for a given fraction (with respect to all test objects)
// of objects with negative labels that get misclassified
vint8 class_BoostMap_module::AdaBoost_threshold_test()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	double misclassified_negatives = 0;
	print("enter misclassified_negative fraction:\n");
	question("%lf", & misclassified_negatives);

	float threshold = boosting->find_threshold_test(misclassified_negatives);
	print("threshold = %f\n", threshold);
	boosting->print_errors(threshold);

	return 1;
}


// find the threshold for a given fraction (with respect to all training objects)
// of objects with negative labels that get misclassified
vint8 class_BoostMap_module::AdaBoost_threshold_training()
{
	if (boosting == 0)
	{
		function_print("\nno boosting object is initialized\n");
		return 0;
	}

	double misclassified_negatives = 0;
	print("enter misclassified_negative fraction:\n");
	question("%lf", & misclassified_negatives);

	float threshold = boosting->find_threshold_training(misclassified_negatives);
	print("threshold = %f\n", threshold);
	boosting->print_errors(threshold);

	return 1;
}



vint8 class_BoostMap_module::FastMapFilterRefine()
{
	long dimensions = 0, max_k = 0, to_keep = 0;
	vPrint("\nenter dataset, test/training embedding files, dimensions, to_keep, max_k:\n");
	char dataset[1000], test_name[1000], train_name[1000];
	vScan("%s %s %s %li %li %li", 
		dataset, test_name, train_name, &dimensions, &to_keep, &max_k);

	char * directory = EmbeddingDirectory2();
	char * test_file = vJoinPaths(directory, test_name);
	char * train_file = vJoinPaths(directory, train_name);

	vdelete2(directory);

	vMatrix<float> test_embedding2;
	vMatrix<float> train_embedding2;

	vint8 problems = 0;

	test_embedding2 = vMatrix<float>::Read(test_file);
	if (test_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", test_file);
		problems = 1;
	}

	train_embedding2 = vMatrix<float>::Read(train_file);
	if (train_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", train_file);
		problems = 1;
	}

	vdelete2(test_file);
	vdelete2(train_file);

	if (problems != 0)
	{
		return 0;
	}

	vint8 test_size = test_embedding2.Rows();
	vint8 training_size = train_embedding2.Rows();
	vMatrix<float> test_embedding(test_size, dimensions);
	vMatrix<float> train_embedding(training_size, dimensions);

	test_embedding2.Copy(&test_embedding, 0, test_size-1, 0, dimensions-1, 0, 0);
	train_embedding2.Copy(&train_embedding, 0, training_size-1, 0, dimensions-1, 0, 0);

	vMatrix<float> weights(1, dimensions);
	function_enter_value(&weights, (float) 1.0);

	vMatrix<float> test_labels = BoostMap_data::LoadTestLabels(g_data_directory, dataset);
	vMatrix<float> train_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, dataset);

	char * distances_file = BoostMap_data::TestTrainDistancesPath(g_data_directory, dataset);
	class_file * fp = BoostMap_data::OpenObjectDistancesFile(distances_file);
	vdelete2(distances_file);

	if (fp == 0)
	{
		vPrint("failed to open distances file\n");
		return 0;
	}

	vMatrix<float> result = BoostMap_data::FilterRefineL2(test_embedding, train_embedding,
		weights, test_labels, train_labels,
		max_k, 0, to_keep, fp);

	fclose(fp);

	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::FastMapKnn()
{
	long dimensions = 0, max_k = 0;
	vPrint("\nenter dataset, test/training embedding files, dimensions, max_k:\n");
	char dataset[1000], test_name[1000], train_name[1000];
	vScan("%s %s %s %li %li", dataset, test_name, train_name, &dimensions, &max_k);

	char * directory = EmbeddingDirectory2();
	char * test_file = vJoinPaths(directory, test_name);
	char * train_file = vJoinPaths(directory, train_name);

	vdelete2(directory);

	vMatrix<float> test_embedding2;
	vMatrix<float> train_embedding2;

	vint8 problems = 0;

	test_embedding2 = vMatrix<float>::Read(test_file);
	if (test_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", test_file);
		problems = 1;
	}

	train_embedding2 = vMatrix<float>::Read(train_file);
	if (train_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", train_file);
		problems = 1;
	}

	vdelete2(test_file);
	vdelete2(train_file);

	if (problems != 0)
	{
		return 0;
	}

	vint8 test_size = test_embedding2.Rows();
	vint8 training_size = train_embedding2.Rows();
	vMatrix<float> test_embedding(test_size, dimensions);
	vMatrix<float> train_embedding(training_size, dimensions);

	test_embedding2.Copy(&test_embedding, 0, test_size-1, 0, dimensions-1, 0, 0);
	train_embedding2.Copy(&train_embedding, 0, training_size-1, 0, dimensions-1, 0, 0);

	vMatrix<float> weights(1, dimensions);
	function_enter_value(&weights, (float) 1.0);

	vMatrix<float> test_labels = BoostMap_data::LoadTestLabels(g_data_directory, dataset);
	vMatrix<float> train_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, dataset);

	vMatrix<float> result = BoostMap_data::KnnError7L2(test_embedding, train_embedding,
		weights, test_labels, 
		train_labels, max_k, 0);

	if (result.valid() == 0)
	{
		vPrint("invalid result\n");
	}
	else
	{
		vint8 i;
		vint8 min_index = -1;
		float min_error = (float) 1000;
		for (i = 0; i <= max_k; i++)
		{
			float error = result(i);
			vPrint("%li-nn error: %f\n", (long) i, error);
			if (i != 0)
			{ 
				if (error < min_error)
				{
					min_error = error;
					min_index = i;
				}
			}
		}
		vPrint("\nbest k: %li. best knn-error: %f\n", (long) min_index, min_error);
	}

	return 1;
}


vint8 class_BoostMap_module::FastMapIndexErrors()
{
	long dimensions = 0, max_k = 0;
	vPrint("\nenter dataset, test/training embedding files, dimensions, max_k:\n");
	char dataset[1000], test_name[1000], train_name[1000];
	vScan("%s %s %s %li %li", dataset, test_name, train_name, &dimensions, &max_k);

	char * directory = EmbeddingDirectory2();
	char * test_file = vJoinPaths(directory, test_name);
	char * train_file = vJoinPaths(directory, train_name);

	vdelete2(directory);

	vMatrix<float> test_embedding2;
	vMatrix<float> train_embedding2;

	vint8 problems = 0;

	test_embedding2 = vMatrix<float>::Read(test_file);
	if (test_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", test_file);
		problems = 1;
	}

	train_embedding2 = vMatrix<float>::Read(train_file);
	if (train_embedding2.valid() <= 0)
	{
		vPrint("failed to read from %s\n", train_file);
		problems = 1;
	}

	vdelete2(test_file);
	vdelete2(train_file);

	if (problems != 0)
	{
		return 0;
	}

	vint8 test_size = test_embedding2.Rows();
	vint8 training_size = train_embedding2.Rows();
	vMatrix<float> test_embedding(test_size, dimensions);
	vMatrix<float> train_embedding(training_size, dimensions);

	test_embedding2.Copy(&test_embedding, 0, test_size-1, 0, dimensions-1, 0, 0);
	train_embedding2.Copy(&train_embedding, 0, training_size-1, 0, dimensions-1, 0, 0);

	vMatrix<float> weights(1, dimensions);
	function_enter_value(&weights, (float) 1.0);

	char * distances_file = BoostMap_data::TestTrainDistancesPath(g_data_directory, dataset);
	class_file * fp = BoostMap_data::OpenObjectDistancesFile(distances_file);
	vdelete2(distances_file);
	if (fp == 0)
	{
		vPrint("failed to open distances file\n");
		return 0;
	}

	vMatrix<float> result = BoostMap_data::IndexErrors6L2(test_embedding, train_embedding,
		weights, fp, 0, max_k);
	fclose(fp);
	vint8_matrix vint8_result(& result);
	print_retrieval_results(vint8_result);

	char * output = BoostMap_data::IndexErrorOutputPath2(dataset, "fastmap", 
		dimensions);

	integer_matrix converted(& result);
	vint8 success = converted.Write(output);
	if (success <= 0)
	{
		vPrint("failed to save result to %s\n", output);
	}
	else
	{
		vPrint("saved result to %s\n", output);
	}
	vdelete2(output);

	return 1;
}


char * class_BoostMap_module::EmbeddingDirectory()
{
	char * result = vJoinPaths4(g_data_directory, "experiments", 
		"bm_datasets", "embeddings");
	return result;
}


vint8 class_BoostMap_module::FmMake20()
{
	char dataset[1000];
	char sample_name[1000];
	vPrint("\nenter dataset, sample_name\n");
	vScan("%s %s", dataset, sample_name);
	return FmMake2(dataset, sample_name);
}


vint8 class_BoostMap_module::FmMake2(const char * dataset, const char * sample_name)
{
	class_FastMap * new_fm = new class_FastMap(dataset, sample_name);
	if (new_fm->valid() > 0)
	{
		vPrint("created new fastmap\n");
		FmDelete();
		fm = new_fm;
	}
	else
	{
		vPrint("failed to make fastmap\n");
	}
	return 1;
}


vint8 class_BoostMap_module::FmMake30()
{
	char dataset[1000];
	char sample_name[1000];
	char saved_file[1000];
	vPrint("\nenter dataset, sample_name, saved_file\n");
	vScan("%s %s %s", dataset, sample_name, saved_file);
	return FmMake3(dataset, sample_name, saved_file);
}


vint8 class_BoostMap_module::FmMake3(const char * dataset, const char * sample_name,
	const char * saved_file)
{
	class_FastMap * new_fm = new class_FastMap(dataset, sample_name, saved_file);
	if (new_fm->valid() > 0)
	{
		vPrint("created new fastmap\n");
		FmDelete();
		fm = new_fm;
	}
	else
	{
		vPrint("failed to make fastmap\n");
	}
	return 1;
}


vint8 class_BoostMap_module::FmDelete()
{
	vdelete(fm);
	fm = 0;
	return 1;
}


vint8 class_BoostMap_module::FmNextStep()
{
	if (fm != 0)
	{
		fm->NextStep();
	}

	return 1;
}


vint8 class_BoostMap_module::FmNextSteps0()
{
	if (fm == 0)
	{
		return 0;
	}

	long steps = 0;
	vPrint("\nenter steps:\n");
	vScan("%li", &steps);

	return FmNextSteps(steps);
}


vint8 class_BoostMap_module::FmNextSteps(vint8 steps)
{
	if (fm == 0)
	{
		return 0;
	}

	vint8 i;
	for (i = 0; i < steps; i++)
	{
		FmNextStep();
	}

	return 1;
}


vint8 class_BoostMap_module::FmEmbedTest()
{
	if (fm == 0)
	{
		return 0;
	}

	fm->EmbedTestSet();
	return 1;
}


vint8 class_BoostMap_module::FmEmbedTraining()
{
	if (fm == 0)
	{
		return 0;
	}

	fm->EmbedTrainingSet();
	return 1;
}


vint8 class_BoostMap_module::FmSave0()
{
	if (fm == 0)
	{
		return 0;
	}

	char filename[1000];
	vPrint("\nenter filename:\n");
	vScan("%s", filename);
	return FmSave(filename);
}


vint8 class_BoostMap_module::FmSave(const char * filename)
{
	if (fm == 0)
	{
		return 0;
	}

	fm->Save(filename);
	return 1;
}


vint8 class_BoostMap_module::FmPrintSample0()
{
	if (fm == 0)
	{
		return 0;
	}

	long i = 0;
	vPrint("\nenter i:\n");
	vScan("%li", &i);
	return FmPrintSample(i);
}


vint8 class_BoostMap_module::FmPrintSample(vint8 i)
{
	if (fm == 0)
	{
		return 0;
	}

	fm->PrintSampleObject((long) i);
	return 1;
}


vint8 class_BoostMap_module::FmPrintTraining0()
{
	if (fm == 0)
	{
		return 0;
	}

	long i = 0;
	vPrint("\nenter i:\n");
	vScan("%li", &i);
	return FmPrintTraining(i);
}


vint8 class_BoostMap_module::FmPrintTraining(vint8 i)
{
	if (fm == 0)
	{
		return 0;
	}

	fm->PrintTrainingObject((long) i);
	return 1;
}


vint8 class_BoostMap_module::FmPrintPivots()
{
	if (fm == 0)
	{
		return 0;
	}

	fm->PrintPivots();
	return 1;
}


// checks if the current BoostMap object is an embedding_optimizer object.
embedding_optimizer * class_BoostMap_module::optimizer_check()
{
	if (bm == 0)
	{
		return 0;
	}

	const char * class_name = bm->get_class_name();
	if (strcmp(class_name, "embedding_optimizer") == 0)
	{
		return (embedding_optimizer *) bm;
	}
	else
	{
		return 0;
	}
}


vint8 class_BoostMap_module::optimizer_new()
{
	char name[1000];
	long number_of_couples = 0;
	vPrint("\nEnter name, number_of_couples:\n");
	vScan("%s %li", name, &number_of_couples);

	BmDelete();
	bm = new embedding_optimizer(g_data_directory, name, (long) number_of_couples);
	if (bm->valid() <= 0)
	{
		vPrint("Failed to load valid bm\n");
		BmDelete();
	}
	else
	{
		BmPrintSummary();
	}

	return 1;
}


vint8 class_BoostMap_module::optimizer_threshold_number()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	long number = 0;
	print("\nenter number:\n");
	question("%li", & number);

	optimizer->set_number_of_thresholds((long) number);
	return 1;
}


vint8 class_BoostMap_module::optimizer_stress_step()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return optimizer_stress_step(buffer);
}


vint8 class_BoostMap_module::optimizer_stress_step(const char * filename)
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	optimizer->stress_optimization_step();
	bm->save_classifier(filename);

	BmPrintSummary();
	return 1;
}


vint8 class_BoostMap_module::optimizer_stress_steps()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);
	return optimizer_stress_steps(buffer, steps);
}


vint8 class_BoostMap_module::optimizer_stress_steps(char * filename, vint8 steps)
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}


	vint8 i;
	for (i = 0; i < steps; i++)
	{
		optimizer_stress_step(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::optimizer_distortion_step()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	char buffer[1000];
	vPrint("\nEnter filename:\n");
	vScan("%s", buffer);
	return optimizer_distortion_step(buffer);
}


vint8 class_BoostMap_module::optimizer_distortion_step(const char * filename)
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	//optimizer->distortion_optimization_step();
	//bm->save_classifier(filename);

	BmPrintSummary();
	return 1;
}


vint8 class_BoostMap_module::optimizer_distortion_steps()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	long steps = 0;
	char buffer[1000];
	vPrint("\nEnter filename, steps:\n");
	vScan("%s %li",  buffer, &steps);
	return optimizer_distortion_steps(buffer, steps);
}


vint8 class_BoostMap_module::optimizer_distortion_steps(char * filename, vint8 steps)
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}


	vint8 i;
	for (i = 0; i < steps; i++)
	{
		optimizer_distortion_step(filename);
	}

	return 1;
}


vint8 class_BoostMap_module::optimizer_test_magnification()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	long reference_object = 0, number_of_neighbors = 0;
	print("\nenter reference object, number of neighbors:\n");
	question("%li %li", & reference_object, & number_of_neighbors);
	optimizer->test_magnification((long) reference_object, (long) number_of_neighbors);

	return 1;
}


vint8 class_BoostMap_module::optimizer_magnification_step()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	optimizer->magnification_step(20, 30);
	optimizer->PrintSummary();

	return 1;
}


vint8 class_BoostMap_module::optimizer_magnification_steps()
{
	embedding_optimizer * optimizer = optimizer_check();
	if (optimizer == 0)
	{
		print("\nno embedding_optimizer object available\n");
	}

	long number = 0;
	print("\nenter number of steps:\n");
	question("%li", & number);

	vint8 counter;
	for (counter = 0; counter < number; counter++)
	{
		optimizer_magnification_step();
	}

	return 1;
}


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
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////
///////////////////////////////////////////////////


vint8 class_BoostMap_module::refinement_new()
{
	refinement_delete();

	char original[1000], training[1000], embedding[1000];
	long max_neighbors = 0;

	print("enter original name, training name, embedding, max_neighbors:\n");
	question("%s %s %s %li", original, training, embedding, &max_neighbors);

	refinement = new class_refinement(g_data_directory, original, training, embedding, max_neighbors);
	if (refinement->valid() <= 0)
	{
		print("invalid refinement\n");
		refinement_delete();
		return 0;
	}

	print("created refinement successfully\n");
	return 1;
}


vint8 class_BoostMap_module::refinement_delete()
{
	function_delete(refinement);
	refinement = 0;
	return 1;
}


vint8 class_BoostMap_module::refinement_train_step()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long dimensions = 0, length = 0, maximum_rounds = 0;
	float error_threshold = 0;

	print("enter dimensions, length, error_threshold, maximum_rounds:\n");
	question("%li %li %f %li", & dimensions, & length, & error_threshold, & maximum_rounds);

	vint8 result = refinement->create_classifier(dimensions, length, error_threshold, maximum_rounds);
	if (result <= 0)
	{
		print("failed to create new classifier in refinement\n");
		return 0;
	}

	print("created new classifier successfully\n");
	return 1;
}


vint8 class_BoostMap_module::refinement_train_more()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long dimensions = 0, length = 0, maximum_rounds = 0;
	float error_threshold = 0;

	print("enter dimensions, length, error_threshold, maximum_rounds:\n");
	question("%li %li %f %li", & dimensions, & length, & error_threshold, & maximum_rounds);

	vint8 result = refinement->train_more(dimensions, length, error_threshold, maximum_rounds);
	if (result <= 0)
	{
		print("failed to performer additional training in refinement\n");
		return 0;
	}

	print("performed additional training successfully\n");
	return 1;
}


vint8 class_BoostMap_module::refinement_save()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	return 1;
}


vint8 class_BoostMap_module::refinement_load()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	return 1;
}


vint8 class_BoostMap_module::refinement_AdaBoost()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long dimensions = 0, length = 0;
	print("enter dimensions, length:\n");
	question("%li %li", & dimensions, & length);

	AdaBoost * result = refinement->find_classifier(dimensions, length);
	if (result == 0)
	{
		print("failed to obtain classifier from refinement\n");
		return 0;
	}

	AdaBoost_destroy();
	boosting = result->copy();
	print("obtained classifier successfully\n");
	return 1;
}


vint8 class_BoostMap_module::refinement_results()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long last_dimensions = 0, last_length = 0;
	vint8 accuracy_counter = 0;
	float distances = 0, accuracy = 0;

	print("enter last dimensions, last length:\n");
	question("%li %li", & last_dimensions, & last_length);

	vint8 result = refinement->retrieval_results(last_dimensions, 
		last_length, distances,	accuracy, accuracy_counter);

	print("distances = %f, accuracy = %f, %li queries processed correctly\n",
		distances, accuracy, (long) accuracy_counter);
	vint8 embedding_distances = refinement->upper_bound_distances();
	print("adjusted distances = %f\n", ((float) embedding_distances) + distances);

	return 1;
}


vint8 class_BoostMap_module::refinement_threshold()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long dimensions = 0, length = 0;
	float threshold = 0;

	print("enter dimensions, length, threshold:\n");
	question("%li %li %f", & dimensions, & length, & threshold);
	refinement->set_step_threshold(dimensions, length, threshold);
	refinement->print_cascade();

	return 1;
}


vint8 class_BoostMap_module::refinement_print()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	refinement->print_cascade();
	return 1;
}


vint8 class_BoostMap_module::refinement_AdaBoost_load()
{
	if (refinement == 0)
	{
		print("no refinement available\n");
		return 0;
	}

	long dimensions = 0, length = 0;
	char filename[1000];
	print("enter dimensions, length, filename:\n");
	question("%li %li %s", & dimensions, & length, filename);

	AdaBoost * result = refinement->find_classifier(dimensions, length);
	if (result == 0)
	{
		print("failed to obtain classifier from refinement\n");
		return 0;
	}

	vint8 success = result->load(filename);
	if (success <= 0)
	{
		print("failed to load classifier from %s\n", filename);
	}
	else
	{
		print("loaded classifier from %s successfully\n", filename);
		AdaBoost_destroy();
		boosting = result->copy();
		AdaBoost_print();
	}

	return 1;
}

