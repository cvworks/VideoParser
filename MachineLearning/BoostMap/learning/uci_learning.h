#ifndef VASSILIS_UCI_LEARNING
#define VASSILIS_UCI_LEARNING


#include "basics/algebra.h"


// This is an auxiliary class for vUciDataset. Each object of this
// class contains information about a specific dataset, including
// information about how to load the dataset from files.
class vDatasetInfo
{
public:
  // name is the name of the directory where the dataset is stored.
  // in cases like waveform, there are two datasets in the same 
  // directory. Those datasets have therefore the same name, and
  // we specify them separately by giving them a different training
  // file and test file. In principle I should change this implementation,
  // so that we can have a separate dataset name and directory name.
  char * name;
  char * training_file;
  char * test_file;
  
  // the column in the datafile where the class label appears.
  vint8 class_col;

  // the column where the subject ids appear, if applicable 
  // (-1 if not applicable).
  vint8 subject_col;

  // In cases (like in the "letter-recognition" dataset) where 
  // the training and test set both appear in one file, we need
  // to specify the ranges for the training and the test set.
  // At this point I'm not sure if this is sufficient for all 
  // datasets. It could be that in some cases the training or
  // test set is not a contiguous range of data.
  vint8 training_start, training_end;
  vint8 test_start, test_end;

  // I put in this variable because in the "vowel" dataset, and potentially
  // in other datasets, some of the attributes are not really features, and
  // should not be used for classification. For example, in the "vowel" 
  // dataset the first three features are train/test, skeaper id, speaker sex.
  // This vector, if empty, means that all features should be used. If 
  // non-empty, it contains the column indices of attributes that should be
  // skipped. Note that if the vector is non-empty, it must include the
  // class column, so that that column is also skipped.
  vector<vint8> attributes_to_skip;

  vDatasetInfo();
  ~vDatasetInfo();
};


// This class is used to store in memory both the training and test set
// for a UCI dataset, and also to produce splits of the training set
// for cross-validation (when no separate test set is given).
class vUciDataset
{
protected:
  // number of training objects and test objects
  vint8 training_number, test_number;

  // number of features. We assume that each object is represented as 
  // a feature vector of fixed dimensions, in Euclidean space. This is
  // restricting, because it does not handle two things: nominal 
  // attributes, and missing attributes. I am not quite sure how to handle
  // missing attributes, but at some point 
  vint8 attributes;

  // number of classes. 
  vint8 classes;
  vint8 is_valid;
  // here we store information about how to read the dataset from disk
  vDatasetInfo * info;

  // dataset_infos[i] will hold information about how to load
  // the i-th dataset, in particular the directory where the info
  // is stored, and the names for the test and the training file.
  vector<vDatasetInfo *> * dataset_infos;

  // class_ids will map class names (which are strings) into integers. Note
  // that the range of class ids is NOT from 0 to classes -1, but from 
  // 1 to classes. This way, if we get a zero result from class_ids.Get,
  // we know that there is no class for that key.
  vStringTable class_ids;

  // training_sizes[i] will be the number of training data for class with id
  // i.
  vMatrix<vint8> training_sizes;

  // training_indices[i] will be a vector with size training_sizes[i]. Its entries
  // will be the indices (i.e. row numbers) of all training data that belong
  // to class with id = i.
  vArray(vector<vint8> ) training_indices;

  // in cases where no test set is available, we will test using
  // 10-fold validation (as suggested by Allwein) or something like that.
  // In that case, original_set is the original training set, and it
  // is used to generate different splits into training and test
  // sets. If a test set is available for a datset, then
  // original training_set will be equal to the training set.
  vMatrix<float> original_set;

  // each row is a training vector (i.e. the representation of a 
  // training object).
  vMatrix<float> training_vectors;

  // Each row is a test vector.
  vMatrix<float> test_vectors;

  // classes of training objects. Note that, even if the classes are 
  // denoted as numbers in the actual file, the same classes can 
  // receive different labels here, based on where the original numbers
  // have been mapped using class_ids.
  vMatrix<vint8> training_labels;

  // classes of test objects.
  vMatrix<vint8> test_labels;

  // subject ids are used in the case where each object has been produced
  // by a given subject, and each subject produced more than one object
  // of the same class. When we construct training triples (q,a,b), it 
  // is important to use a and b objects for which the subject was not
  // the subject of q. The reason is that, when we need to classify a 
  // test object t, its subject will most likely be a subject that was
  // not used to produce the training examples. If we don't take this
  // precaution, in iterations after the first one, we construct triples
  // that are "too easy" to classify, because a will very likely come
  // from the same subject as q, and will be closer to q than a typical
  // same-class w-nearest-neighbor of a test object will be.
  vMatrix<vint8> subject_ids;

  // last argument to Split function. Used in CrossValidationSet, to 
  // make sure that we have a reasonable argument. Strictly speaking,
  // we could eliminate this variable, since the same information
  // can be found by checking split_numbers.Size().
  vint8 last_split;

  // split_numbers(i) is the length of the i-th split set. The split
  // sets are used to generate multiple training and test sets for
  // cross-validation.
  vMatrix<vint8> split_numbers;

  // split_indices(i, j) is the index of the j-th object of the
  // i-th split set.
  vMatrix<vint8> split_indices;

  // Looks up the name of the class with given class id, exits with
  // error message if no class name is found. This is not public 
  // because a call with a bad class id can crash the program.
  const char * ClassName(vint8 class_id);

public:
  vUciDataset();
  // Each number corresponds to a dataset.
  vUciDataset(vint8 number);
  ~vUciDataset();

  vint8 Zero();

  // This function is in charge of storing the correct info for
  // each dataset into dataset_names, dataset_training_files and
  // dataset_test_files. This information will be manually entered
  // (i.e. it will be hardwired in the code of the function).
  // In order to see summary information about each dataset (i.e.
  // to call PrintDatasetInfo) we need to initialize a UCI dataset
  // object, so that StoreDatasetInfo is called.
  vint8 StoreDatasetInfo();

  vint8 valid();

  // Reads a file, and stores the representation of each object
  // into the result matrix. Note that this representation is
  // an intermediate representation, from which we obtain the 
  // final representation by calling GetFeatures and GetLabels.
  vMatrix<float> ReadFile(const char * path_name);

  // Read the training set. In files where no test set is available, this
  // function will read the entire dataset, and store it in original_set.
  vint8 ReadTrainingSet();

  // If a test set is available in a separate file, we read from that file. 
  // If a test set is available as part of the same file from which we 
  // read the training set, we read the test set from original_set. 
  // Otherwise, we do nothing (the test set will be created by calling
  // a function like Sample, Sample2, or CrossValidationSet).
  vint8 ReadTestSet();

  // returns the directory where all the uci datasets are stored.
  static char * UciDirectory();

  // returns the pathname where the training set (or entire dataset)
  // is stored.
  char * TrainingPath();

  // if a test set is stored in a separate file, it returns
  // the pathname of that file.
  char * TestPath();

  // It is assumed that the features are all columns of the data except for 
  // the entries in info->attributes_to_skip. If start 
  // or end is negative, then we read from all the 
  // rows, otherwise we read from the range of rows that is between start
  // and end (including start and end).
  vMatrix<float> GetFeatures(v3dMatrix<float> data, vint8 start, vint8 end);

  // It is assumed that the labels are at info.class_col of the data.
  // If start or end is negative, then we read from all the 
  // rows, otherwise we read from the range of rows that is between start
  // and end (including start and end).
  vMatrix<vint8> GetLabels(v3dMatrix<float> data, vint8 start, vint8 end);

  // the subject ids are obtained from the specified column in info, except
  // for the isolet dataset, where subject ids are not really available,
  // but I have a special function that returns a matrix that is kind of 
  // conservative, but does ensure that if two objects have the same true
  // subject id, then they will MOST LIKELY have the same subject id in
  // this matrix as well.
  // I should note that, in general, it is not only subjects that we care
  // about. What we do care about is that, sometimes, there are same-class
  // data in the training set that were generated in a process that makes
  // them more similar to each other than a test object would be to a 
  // same-class nearest neighbor. In such cases, we should try not to
  // use training data that are too similar to each other in the same
  // training triple.
  vMatrix<vint8> GetSubjectIds(v3dMatrix<float> data, vint8 start, vint8 end);

  // This auxiliary function stores the appropriate info into 
  // class_ids, training_sizes and training_indices.
  vint8 ProcessTraining();

  // return the dataset name corresponding to a number.
  char * NumberToDataset(vint8 number);
  
  // return the number corresponding to a dataset name (negative
  // if no such name was found).
  vint8 DatasetToNumber(const char * name);

  // Sample(fraction) splits original set into a training set and a test set.
  // From each class, the specified fraction of objects are chosen to be
  // in the training set, and the rest go to the test set. Of course, fraction
  // should in the (0, 1) range. The sampling is done separately on each class.
  // This means that, at each class, we determine the number of training 
  // points to pick (by multiplying the class size with fraction and rounding)
  // and we pick them. The resulting training and test sets are not as 
  // suitable for cross-validation, because the percentage of each class in 
  // the test set, averaged over repeated calls with the same fraction 
  // argument, is not equal to the percentage of each class in the original
  // set.
  vint8 Sample(float fraction);

  // Sample2(fraction) is like Sample, but the sampling is done on all objects
  // together. This means that the the percentage of each class in 
  // the test set, averaged over repeated calls with the same fraction 
  // argument, will tend to be equal to the percentage of each class in 
  // the original set, and this makes this method appropriate for use
  // in cross-validation experiments.
  vint8 Sample2(float fraction);

  inline vMatrix<float> TrainingVectors()
  {
    return training_vectors;
  }

  inline vMatrix<vint8> TrainingLabels()
  {
    return training_labels;
  }

  inline vMatrix<float> TestVectors()
  {
    return test_vectors;
  }

  inline vMatrix<vint8> TestLabels()
  {
    return test_labels;
  }

  inline vMatrix<vint8> SubjectIds()
  {
    return subject_ids;
  }

  inline vint8 Classes()
  {
    return classes;
  }

  // returns 1 if subject id info is available, 0 otherwise.
  inline vint8 SubjectIdsAvailable()
  {
    if (subject_ids.Size() == 1) 
    {
      return 0;
    }
    return 1;
  }

  // Specialized function for the isolet dataset, where subject ids 
  // are not really available,
  // but I have a special function that returns a matrix that is kind of 
  // conservative, but does ensure that if two objects have the same true
  // subject id, then they will MOST LIKELY have the same subject id in
  // this matrix as well.  
  vMatrix<vint8> IsoletSubjectIds();
  
  // printing some information, for debugging
  vint8 PrintSummary();

  // print the i-th row of original set, training vectors, test vectors.
  vint8 PrintRow(vint8 row);

  // Print the number of training objects from each class.
  vint8 PrintTrainingSizes();

  // Print the attributes of the j-th training object of class i.
  vint8 PrintTrainingObject(vint8 i, vint8 j);

  // Print the class_ids table.
  vint8 PrintClassIds();

  // Print info about all the datasets for which info is available.
  vint8 PrintDatasetInfo();

  // Print the indices of all objects belonging to class class_id,
  // in the training set, and the test set.
  vint8 PrintClassIndices(vint8 class_id);

  // Split the original set into number sets, so that we can
  // do cross validation. 
  vint8 Split(vint8 number);

  // Assuming that we have already called Split with some number
  // greater than i, here we choose the i-th test for testing
  // and the rest for training.
  vint8 CrossValidationSet(vint8 i);

  // for debugging, prints the objects belonging to each set 
  // generated by the last call to Split().
  vint8 PrintSplits();
};







#endif // VASSILIS_UCI_LEARNING
