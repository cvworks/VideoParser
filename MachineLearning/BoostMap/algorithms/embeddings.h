#ifndef VASSILIS_EMBEDDINGS_H
#define VASSILIS_EMBEDDINGS_H

#include "../learning/boost_kdd.h"


// This is a wrapper around class_FastMap, that I wrote quickly, 
// that provides a more convenient
// interface for using FastMap. I implemented V_FastMap for
// CVPR 2004, and I implemented this version for KDD 2004. 
class class_FastMap
{
private:
  // the fastmap object.
  V_FastMap * fastmap;

  // the training dataset based on 
  // which we will construct the embedding.
  BoostMap_data * data;

  // the name of the entire dataset. This is useful for constructing
  // filenames in which we save results.
  char * dataset_name;

  // here we map indices of the training dataset to indices
  // in the entire dataset (see also notes in BoostMap_data).
  vMatrix<vint8> dataset_indices;
  long is_valid;

  // indices of the selected pivot objects. We use indices into
  // the entire set, not into the training set.
  vMatrix<vint8> pivot_indices;

  // result of class_FastMap::OptimizedPivotDistances.
  vMatrix<float> optimized_pivot_distances;

public:
  // initialize a FastMap object based on the names of 
  // an entire dataset and a training dataset.
  // We use the specified training indices to form the training
  // triples. It seems there may be a bug, in that we 
  // assume that the number of candidates is equal to the number
  // of training objects, and that assumption is not always true.
  // Furthermore, it seems that the pivots come from the training set,
  // maybe they should be coming from the candidate set.
  class_FastMap(const char * in_dataset_name, const char * in_sample_name);

  // Here, we initialize a FastMap object, by loading
  // from file some pivots that had already been selected.
  // Notice that, in this constructor, we do not construct
  // an object capable of picking new pivot pairs (i.e. 
  // capable of adding dimensions to the embedding).
  class_FastMap(const char * in_dataset_name, const char * in_sample_name,
           const char * saved_file);

  ~class_FastMap();

  // Select the next pair of pivot objects.
  long NextStep();

  // Embed the test set of the entire dataset, and save
  // the result into a file. Every row is 
  // the embedding of a test object.
  vMatrix<float> EmbedTestSet();

  // Embed the training set of the entire dataset. Every row
  // is the embedding of a training object.
  vMatrix<float> EmbedTrainingSet();

  // Save the chosen pivots into a filename, together with 
  // the optimized distances.
  long Save(const char * filename);

  // load pivots and optimized distances from a file.
  long Load(const char * filename);
  
  // Translate indices, from indices of the training dataset
  // to indices of the entire dataset.
  vMatrix<vint8> TranslateIndices(vMatrix<vint8> indices);

  // prints the index of the object (mapping from sample index to
  // dataset index), and the embedding of the object, based on
  // the current fastmap.
  long PrintSampleObject(long i);

  // prints the embedding of a training object, by reading
  // it from the traintrain file, and then embedding
  // it based on the current fastmap.
  long PrintTrainingObject(long i);

  // Prints pivots (their true indices).
  long PrintPivots();

  long valid();

  // Construct a pathname for saving the embedding 
  // description (using Save), based on filename,
  // and based on the number of pivot pairs chosen
  // so far. We use the number of pivot pairs so
  // that we don't always write over the same file,
  // but instead we cyclically update a bunch of files.
  char * Pathname(const char * filename);

  // construct a complete pathname for loading
  // the description of the embedding (using Load),
  // based on filename.
  char * Pathname2(const char * filename);

  // directory where we save and load embedding
  // descriptions (using Save and Load).
  char * EmbeddingDirectory();

  // Complete pathname where we save the embedding
  // of the test set of the entire dataset.
  char * TestEmbeddingPath();

  // Complete pathname where we save the embedding
  // of the training set of the entire dataset.
  char * TrainingEmbeddingPath();
};


// Implementation of Bourgain embeddings. There is another 
// implementation I did for the CVPR 2004 experiments, but this
// one will work with BoostMap_data, so that it can be easily
// applied to any dataset. Also, in the previous implementation,
// the implementing class was a subclass of class_BoostMap. Here
// it is just a standalone class. The only drawback of this
// is that it does not give us the triple classification error
// for free. The advantage is that the code is much more simple.

class class_Bourgain
{
protected:
  vint8 number; //  number of training objects

  // objects_to_use is useful for creating lower-dimensional embeddings.
  // We then choose objects_to_use out of the n objects, and we 
  // build a Bourgain embedding based on them.
  vint8 objects_to_use;

  // is_valid is positive iff the arguments in the constructor make sense
  // (for example, it doesn't make sense to have negative arguments).
  vint8 is_valid;

  vMatrix<vint8> picks;

  // each element of this vector is a reference set.
  vector<vMatrix<vint8> > reference_sets;

protected:
  vint8 Initialize();
  vint8 PickSets();

public:
  class_Bourgain();
  class_Bourgain(vint8 in_n, vint8 in_objects_to_use);
  ~class_Bourgain();

  vint8 Number();
  vint8 ObjectsToUse();
  vint8 Dimensions();

  static char * Directory();

  // When we save the test and training set embeddings of
  // a dataset, we generate output filenames automatically. However,
  // we append a number to the end of those filenames, to make sure
  // we do not write over existing files. This function finds the
  // next available number that we can append to a filename, to 
  // generate a filename that does not exist right now.
  vint8 FindNumber(const char * dataset);

  // where to save the embedding of the training set.
  static char * TrainingPath(const char * dataset, vint8 dimensions, vint8 number);

  // where to save the embedding of the test set.
  static char * TestPath(const char * dataset, vint8 dimensions, vint8 number);

  // where to save the reference sets.
  char * EmbeddingPath(const char * dataset, vint8 number);

  vMatrix<float> EmbedTraining(const char * dataset);
  vMatrix<float> EmbedTest(const char * dataset);
  vMatrix<float> EmbedObjects(class_file * fp, vint8 training_number, vint8 size);
  static float MinDistance(vMatrix<float> distances, 
                           vMatrix<vint8> reference_set);
  vint8 SaveEmbeddings(const char * dataset);

  static vMatrix<float> LoadTrainingEmbedding(const char * dataset,
                                              vint8 dimensions, vint8 number);
  static vMatrix<float> LoadTestEmbedding(const char * dataset,
                                          vint8 dimensions, vint8 number);

  // Saves the reference sets.
  vint8 Save(const char * filename);

  // For debugging
  vint8 Print();
};


// Implementation of SparseMap embeddings. The class_SparseMap 
// implementation I did for the CVPR 2004 experiments was misnamed,
// it was actually an implementation of Bourgain embeddings. Overall,
// the implementation here is pretty similar to the implementation
// of Bourgain embeddings.
class class_SparseMap
{
protected:
  vint8 number; //  number of training objects

  // objects_to_use is useful for creating lower-dimensional embeddings.
  // We then choose objects_to_use out of the n objects, and we 
  // build a Bourgain embedding based on them.
  vint8 objects_to_use;

  // is_valid is positive iff the arguments in the constructor make sense
  // (for example, it doesn't make sense to have negative arguments).
  vint8 is_valid;

  vMatrix<vint8> picks;

  // each element of this vector is a reference set.
  vector<vMatrix<vint8> > reference_sets;

protected:
  vint8 Initialize();
  vint8 PickSets();

public:
  class_SparseMap();
  class_SparseMap(vint8 in_n, vint8 in_objects_to_use);
  ~class_SparseMap();

  vint8 Number();
  vint8 ObjectsToUse();
  vint8 Dimensions();

  static char * Directory();

  // When we save the test and training set embeddings of
  // a dataset, we generate output filenames automatically. However,
  // we append a number to the end of those filenames, to make sure
  // we do not write over existing files. This function finds the
  // next available number that we can append to a filename, to 
  // generate a filename that does not exist right now.
  vint8 FindNumber(const char * dataset);

  // where to save the embedding of the training set.
  static char * TrainingPath(const char * dataset, vint8 dimensions, vint8 number);

  // where to save the embedding of the test set.
  static char * TestPath(const char * dataset, vint8 dimensions, vint8 number);

  // where to save the reference sets.
  char * EmbeddingPath(const char * dataset, vint8 number);

  vMatrix<float> EmbedTraining(const char * dataset);
  vMatrix<float> EmbedTest(const char * dataset, vMatrix<float> training_embedding);
  vMatrix<float> EmbedObjects(class_file * fp, vint8 training_number, vint8 size);
  static float MinDistance(vMatrix<float> distances, 
                           vMatrix<vint8> reference_set);
  vint8 SaveEmbeddings(const char * dataset);

  static vMatrix<float> LoadTrainingEmbedding(const char * dataset,
                                              vint8 dimensions, vint8 number);
  static vMatrix<float> LoadTestEmbedding(const char * dataset,
                                          vint8 dimensions, vint8 number);

  // Saves the reference sets.
  vint8 Save(const char * filename);

  // For debugging
  vint8 Print();

  vMatrix<vint8> SigmaMatches(vMatrix<vint8> set, vint8 i, 
                             vMatrix<float> embedding, vint8 sigma);

  vint8 ComputeIndices(vMatrix<vint8> matches, vector<vector<vint8> > * indices);

  vMatrix<vint8> ObjectSigmaMatches(vMatrix<float> test_embeddingm, vMatrix<vint8> set, 
                                   vint8 dimensions, vMatrix<float> embeddingm, vint8 sigma);
};


#endif // VASSILIS_EMBEDDINGS_H
