
#include "uci_learning.h"
#include "basics/local_data.h"
#include "basics/drawing_temp.h"
#include "basics/simple_algo.h"
#include "basics/simple_algo_templates.h"

#include "basics/definitions.h"


vDatasetInfo::vDatasetInfo()
{
  name = 0;
  training_file = 0;
  test_file = 0;
  
  class_col = -1;
  training_start = -1;
  training_end = -1;
  test_start = -1;
  test_end = -1;
  subject_col = -1;
}


vDatasetInfo::~vDatasetInfo()
{
  vdelete2(name);
  vdelete2(training_file);
  vdelete2(test_file);
}


vUciDataset::vUciDataset()
{
  Zero();
  StoreDatasetInfo();
}


// Each number corresponds to a dataset.
vUciDataset::vUciDataset(vint8 number)
{
  Zero();

  // Initialize information about each dataset.
  StoreDatasetInfo();

  // Based on that information, look up the dataset
  // corresponding to number.
  vint8 size = dataset_infos->size();
  if ((number < 0) || (number >= size))
  {
    return;
  }
  else
  {
    // store in info all the information needed to load
    // the specified dataset.
    info = (*dataset_infos)[(long) number];
    vint8 success;
    success = ReadTrainingSet();
    if (success <= 0) 
    {
      return;
    }
    success = ReadTestSet();
    if (success <= 0) 
    {
      return;
    }
    // if we get here it means everything was loaded successfully.
    is_valid = 1;
  }
}


vUciDataset::~vUciDataset()
{
  vint8 i; 
  vint8 size = dataset_infos->size();

  // delete information stored in dataset_infos.
  for (i = 0; i < size; i++)
  {
    vdelete((*dataset_infos)[(long) i]);
  }
  vdelete(dataset_infos);

  // delete the class ids table.
  class_ids.Clear();

  vdelete2(training_indices);
}


// initialize everything to default values.
vint8 vUciDataset::Zero()
{
  training_number = 0;
  test_number = 0;
  attributes = 0;
  classes = 0;
  is_valid = 0;
  training_indices = vZero(vector<vint8> );
  dataset_infos = 0;
  last_split = 0;

  return 1;
}


// This function is in charge of storing the correct info for
// each dataset into dataset_names, dataset_training_files and
// dataset_test_files. This information will be manually entered
// (i.e. it will be hardwired in the code of the function).
vint8 vUciDataset::StoreDatasetInfo()
{
  dataset_infos = new vector<vDatasetInfo *>;
  vDatasetInfo * info;

  // glass: only one datafile, no test set specified. The class attribute 
  // is the last attribute.
  info = new vDatasetInfo();
  info->name = vCopyString("glass");
  info->training_file = vCopyString("glass.data");
  info->test_file = 0;
  info->class_col = 10;
  info->attributes_to_skip.push_back(0);
  info->attributes_to_skip.push_back(info->class_col);
  dataset_infos->push_back(info);

  // image: training and test data are separate. The class attribute 
  // is the first attribute. Allwein calls this dataset "segmentation".
  // Also, Allwein trained and tested (using cross-validation) on the
  // the union of the training set and the test set.
  info = new vDatasetInfo();
  info->name = vCopyString("image");
  info->training_file = vCopyString("segmentation_vassilis.test");
  info->test_file = 0;
  info->class_col = 0;
  dataset_infos->push_back(info);

  // isolet: training and test data are separate. The class attribute 
  // is the last attribute.
  info = new vDatasetInfo();
  info->name = vCopyString("isolet");
  info->training_file = vCopyString("isolet1+2+3+4.data");
  info->test_file = vCopyString("isolet5.data");
  info->class_col = 617;
  dataset_infos->push_back(info);

  // letter-recognition: there is only one data file, first 16000 are 
  // training, last 4000 are test data.
  // class attribute is the first attribute.
  info = new vDatasetInfo();
  info->name = vCopyString("letter-recognition");
  info->training_file = vCopyString("letter-recognition.data");
  info->test_file = 0;
  info->training_start = 0;
  info->training_end = 15999;
  info->test_start = 16000;
  info->test_end = 19999;
  info->class_col = 0;
  dataset_infos->push_back(info);

  // pendigits: training and test data are separate. The class attribute 
  // is the last attribute.
  info = new vDatasetInfo();
  info->name = vCopyString("pendigits");
  info->training_file = vCopyString("pendigits.tra");
  info->test_file = vCopyString("pendigits.tes");
  info->class_col = 16;
  dataset_infos->push_back(info);

  // satimage: training and test data are separate. The class attribute 
  // is the last attribute.
  info = new vDatasetInfo();
  info->name = vCopyString("satimage");
  info->training_file = vCopyString("sat.trn");
  info->test_file = vCopyString("sat.tst");
  info->class_col = 36;
  dataset_infos->push_back(info);

  // vowel: there is only one data file, first 529 are 
  // training, last 461 are test data.
  // class attribute is the last attribute.
  // The first three features are useless (train or test,
  // speaker id, sex of speaker).
  info = new vDatasetInfo();
  info->name = vCopyString("vowel");
  info->training_file = vCopyString("vowel-context.data");
  info->test_file = 0;
  info->training_start = 0;
  info->training_end = 527;
  info->test_start = 528;
  info->test_end = 989;
  info->class_col = 13;
  info->subject_col = 1;
  function_insert_range(&(info->attributes_to_skip), 0, 2);
  info->attributes_to_skip.push_back(info->class_col);
  dataset_infos->push_back(info);

  // waveform: Only training is available. The class attribute 
  // is the last attribute. Note that this dataset is stored
  // in the same directory as waveform plus noise. Therefore,
  // at least in the initial (ICML 2004 submission) implementation,
  // these two datasets have the same name.
  info = new vDatasetInfo();
  info->name = vCopyString("waveform");
  info->training_file = vCopyString("waveform.data");
  info->test_file = 0;
  info->class_col = 21;
  dataset_infos->push_back(info);

  // waveform plus noise: Only training is available. The class attribute 
  // is the last attribute. 
  info = new vDatasetInfo();
  info->name = vCopyString("waveform");
  info->training_file = vCopyString("waveform-+noise.data");
  info->test_file = 0;
  info->class_col = 40;
  dataset_infos->push_back(info);

  // yeast: Only training is available. The class attribute 
  // is the first attribute. 
  info = new vDatasetInfo();
  info->name = vCopyString("yeast");
  info->training_file = vCopyString("yeast_vassilis.data");
  info->test_file = 0;
  info->class_col = 8;
  dataset_infos->push_back(info);

  // image: training and test data are separate. In this version
  // of the dataset, we use the ".test" file for training, 
  // and the ".data" file for testing.
  info = new vDatasetInfo();
  info->name = vCopyString("image2");
  info->training_file = vCopyString("segmentation_vassilis.test");
  info->test_file = vCopyString("segmentation_vassilis.data");
  info->class_col = 0;
  dataset_infos->push_back(info);

  // image: training and test data are separate. In this version
  // of the dataset, we use the ".test" file for testing, 
  // and the ".data" file for training.
  info = new vDatasetInfo();
  info->name = vCopyString("image3");
  info->training_file = vCopyString("segmentation_vassilis.data");
  info->test_file = vCopyString("segmentation_vassilis.test");
  info->class_col = 0;
  dataset_infos->push_back(info);

  return 1;
}

  
vint8 vUciDataset::valid()
{
  return is_valid;
}


// read the file specified by path_name. This file will hold
// either the training set, or the test set, or both sets,
// or just a single set from which we will make training and
// test sets using cross-validation.
vMatrix<float> vUciDataset::ReadFile(const char * path_name)
{
  vint8 columns = -1;
  vector<float> features;

  // open file and check for success
  FILE * fp = fopen(path_name, vFOPEN_READ);
  if (fp == 0) 
  {
    return vMatrix<float>();
  }

  vint8 column_counter = 0;
  char c = 0;
  vint8 items;

  // read the data, one by one.
  while(1)
  {
    // check if we are reading a class label (which is a string)
    // or a feature (which is a number)
    if (column_counter == info->class_col)
    {
      // read the label.
      char buffer[1000];
      items = fscanf(fp, " %[0-9a-zA-Z.-]", buffer);
      if (items != 1)
      {
        break;
      }

      // check if the label has already been seen and has an id.
      vint8 exists = class_ids.KeyExists(buffer);
      if (exists <= 0)
      {
        // we add 1 to the size so that no id is 0, so that
        // if Get returns 0 we know that something is wrong.
        vint8 new_id = class_ids.Size() + 1;
        class_ids.Put(buffer, (void *) (long) new_id);
      }
      // Get the id of the label
      vint8 id = (vint8) (long) class_ids.Get(buffer);
      if (id == 0)
      {
        exit_error("Error: class_ids.Get(%s) = %li\n", id);
      }
      features.push_back((float) id);
    }
    else
    {
      // read the feature
      float feature;
      items = fscanf(fp, "%f", &feature);
      if (items != 1) 
      {
        break;
      };
      features.push_back(feature);
    }
    column_counter = column_counter + 1;

    // read the next character, should be period, new line, comma, or
    // nothing (if we reached end of file)
    items = fread(&c, 1, sizeof(char), fp);
    if (items != 1)
    {
      break;
    }

    // check if we reached the end of the current row
    // if it is a period, we just skip it. In principle, after a dot
    // we should always get a newline.
    if (c == '.')
    { 
      items = fread(&c, 1, sizeof(char), fp);
      if (items != 1)
      {
        break;
      }
      if (c != 13)
      {
        exit_error("Error: Character %li %c follows period\n", 
                        (vint8) c, c);
      }
    }

    if (c == 13) // '\r'
    {
      // in this case the next character should be 10, and
      // column counter should be cols.
      items = fread(&c, 1, sizeof(char), fp);
      if ((items != 1) || (c != 10))
      {
        exit_error("Error: improper line end: items = %li, c = %li\n",
                        items, (vint8) c);
      }
    }

    // if the character is '\n', then we have reached the end of the line.
    // If this is the first line we have read, we now know how many
    // columns exist in this file.
    if (c == 10) // '\n'
    {
      // check if this is the end of the first line, to initialize
      // the columns variable.
      if (columns == -1)
      {
        columns = column_counter;
      }
      // if this is not the first line, we should verify that we read
      // the right number of columns, since we assume that each
      // row (line) has the same number of columns
      else if (columns != column_counter)
      {
        exit_error("Error: line ends in middle of row: %li %li\n",
                        column_counter, columns);
      }

      column_counter = 0;
    }
    // we allow a single comma or a single space after each entry.
    else if ((c == ',') || (c == ' '))
    {
      continue;
    }
    else
    {
      exit_error("Error: invalid character %li %c read after attribute\n",
                      (vint8) c, c);
    }
  }

  // if columns == -1, then we haven't read any lines, so we failed
  // to read meaningful data from the file.
  if (columns == -1)
  {
    exit_error("Error: empty data file %s\n", path_name);
  }

  // make sure that the last line had as many columns as all other
  // lines.
  if ((column_counter != 0) && (column_counter != columns))
  {
    exit_error("Error: file ends in middle of row: %li\n", 
                    column_counter);
  }

  // get the total number of items we read.
  vint8 total_items = features.size();
  // split the items we read into rows corresponding to different
  // objects.
  vint8 rows = total_items / columns;
  vMatrix<float> result(rows, columns);
  vArray(float) result_data = result.Matrix();
  long i;
  for (i = 0; i < total_items; i++)
  {
    result_data[i] = features[i];
  }

  return result;
}


vint8 vUciDataset::ReadTrainingSet()
{
  char * path_name = TrainingPath();
  original_set = ReadFile(path_name);
  vdelete2(path_name);
  if (original_set.valid() <= 0)
  {
    return 0;
  }

  // If no test file is available, it means that there is no 
  // separate test data available. In that case, either there
  // is a section of the data that is reserved for testing, (and
  // info should hold that information), or we have to use
  // 10-fold validation or in general some leave-k-out strategy.
  training_vectors = GetFeatures(original_set, info->training_start, 
                                 info->training_end);
  training_labels = GetLabels(original_set, info->training_start, 
                              info->training_end);
  subject_ids = GetSubjectIds(original_set, info->training_start,
                              info->training_end);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();
  return 1;
}


// read the test set from a file, or extract it from original_set if
// that is appropriate for this dataset.
vint8 vUciDataset::ReadTestSet()
{
  vMatrix<float> temp_test_set;
  // check if no separate test file exists.
  if (info->test_file == 0)
  {
    // if no separate test file exists, check if the test set
    // is part of original set.
    if ((info->test_start >= 0) && (info->test_end >= info->test_start))
    {
      temp_test_set = original_set;
    }
    else
    {
      // if we get here, no test set has been specified for this dataset.
      // We will have to use cross-validation for this dataset.
      return 1;
    }
  }
  // Here we handle the most straightforward case, i.e. the case
  // where the test set exists and is stored in a separate file.
  else
  {
    char * path_name = TestPath();
    temp_test_set = ReadFile(path_name);
    // make sure that the test data have as many attributes as the
    // training data. This is actually the wrong thing to do, since
    // for some datasets there are additional columns that need
    // to be skipped (as described in info->attributes_to_skip). 
    // I am keeping it like this for the time being,
    // so that I don't change anything from the code used for
    // the ICML 2004 submission experiments.
    if (temp_test_set.Cols() != attributes+1)
    {
      exit_error("Error: wrong number of columns (%li/%li)\n",
                      temp_test_set.Cols(), attributes+1);
    }
    vdelete2(path_name);
  }

  test_vectors = GetFeatures(temp_test_set, info->test_start, info->test_end);
  test_labels = GetLabels(temp_test_set, info->test_start, info->test_end);
  test_number = test_vectors.Rows();

  return 1;
}


// return the directory that is the default location for all 
// UCI datasets (not just for this dataset). We assume here
// that all datasets are stored in a directory that can 
// be deduced from the value of g_data_directory, regardless of
// the actual machine.
char * vUciDataset::UciDirectory()
{
  char * result = vJoinPaths3(g_data_directory, "uci_learning", "processed");
  return result;
}


// return the complete pathname where the training data can be read.
char * vUciDataset::TrainingPath()
{
  char * uci_dir = UciDirectory();
  char * result = vJoinPaths3(uci_dir, info->name, info->training_file);
  vdelete2(uci_dir);
  return result;
}


// return the complete pathname where the test data can be read.
// there should be a check here (but there isn't) about info->test_file
// being 0.
char * vUciDataset::TestPath()
{
  char * uci_dir = UciDirectory();
  char * result = vJoinPaths3(uci_dir, info->name, info->test_file);
  vdelete2(uci_dir);
  return result;
}


// It is assumed that the features are all columns of the data except for 
// the entries in info->attributes_to_skip. If start 
// or end is negative, then we read from all the 
// rows, otherwise we read from the range of rows that is between start
// and end (including start and end).
vMatrix<float> vUciDataset::GetFeatures(v3dMatrix<float> data, 
                                          vint8 start, vint8 end)
{
  vint8 rows = data.Rows();
  vint8 cols = data.Cols();
  vector<vint8> row_indices, col_indices;
  // read information from info->attributes_to_skip about columns that
  // need to be skipped.
  if (info->attributes_to_skip.size() != 0)
  {
    // get columns to be skipped.
    vMatrix<vint8> skip_matrix = matrix_from_vector(&(info->attributes_to_skip));
    // get columns that need to be kept.
    vMatrix<vint8> keep_matrix = vComplement(&skip_matrix, 0, cols-1);
    vector_from_matrix(&keep_matrix, &col_indices);
  }
  else
  {
    // keep all columns except for the column holding the class label.
    function_insert_range(&col_indices, 0, info->class_col - 1);
    function_insert_range(&col_indices, info->class_col + 1, cols - 1);
  }

  // keep the rows specified by start and end.
  if (start < 0)
  {
    start = 0;
    end = rows - 1;
  }

  function_insert_range(&row_indices, start, end);

  // get the specified rows and cols.
  v3dMatrix<float> result = function_select_grid(&data, &row_indices, &col_indices);
  return vMatrix<float>(&result);
}


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
vMatrix<vint8> vUciDataset::GetSubjectIds(v3dMatrix<float> data, 
                                           vint8 start, vint8 end)
{
  // for the isolet dataset we have a special function, that produces
  // a substitute for the true subject ids.
  if (strcmp(info->name, "isolet") == 0)
  {
    return IsoletSubjectIds();
  }

  // if no subject ids are available for this dataset, we can't do anything.
  if (info->subject_col == -1)
  {
    return vMatrix<vint8>();
  }

  vint8 rows = data.Rows();
  vint8 cols = data.Cols();
  vector<vint8> row_indices, col_indices;

  // we pass -1 for start if we want it to be determined automatically.
  if (start < 0)
  {
    start = 0;
    end = rows - 1;
  }

  function_insert_range(&row_indices, start, end);
  col_indices.push_back(info->subject_col);

  // get the subject ids.
  v3dMatrix<float> temp_ids = function_select_grid(&data, &row_indices, &col_indices);
  vArray(float) temp = temp_ids.Matrix();
  
  // convert result from a 3D float matrix to a 2D vint8 matrix (essentially
  // a row matrix).
  vMatrix<vint8> result_matrix(1, temp_ids.Size());
  vArray(vint8) result = result_matrix.Matrix();
  vint8 i;
  for (i = 0; i < temp_ids.Size(); i++)
  {
    result[i] = round_number(temp[i]);
  }

  return result_matrix;
}


// It is assumed that the labels are at info->class_col of the data.
// If start or end is negative, then we read from all the 
// rows, otherwise we read from the range of rows that is between start
// and end (including start and end).
vMatrix<vint8> vUciDataset::GetLabels(v3dMatrix<float> data, 
                                      vint8 start, vint8 end)
{
  vint8 rows = data.Rows();
  vint8 cols = data.Cols();

  if (start < 0)
  {
    start = 0;
    end = rows - 1;
  }

  vMatrix<vint8> result(1, end-start+1);
  vArray2(float) entries = data.Matrix2(0);
  vint8 i;
  for (i = start; i <= end; i++)
  {
    result(i-start) = round_number(entries[i][info->class_col]);
  }
  return result;
}


// This auxiliary function stores the appropriate info into 
// class_ids, training_sizes and training_indices.
vint8 vUciDataset::ProcessTraining()
{
  // erase whatever may be already stored inside those vectors
  vdelete2(training_indices);
  classes = class_ids.Size();

  // initialize training indices and training sizes.
  training_indices = vnew(vector<vint8>, (vector_size) (classes+1));
  training_sizes = vMatrix<vint8>(1, classes+1);
  function_enter_value(&training_sizes, (vint8) 0);

  vint8 i;
  for (i = 0; i < training_number; i++)
  {
    vint8 label = training_labels(i);
    training_indices[label].push_back(i);
    training_sizes(label) = training_sizes(label) + 1;
  }

  return 1;
}


// return the dataset name corresponding to a number.
char * vUciDataset::NumberToDataset(vint8 number)
{
  vint8 size = dataset_infos->size();
  if ((number < 0) || (number >= size))
  {
    return 0;
  }
  else
  {
    char * result = vCopyString((*dataset_infos)[(vector_size) number]->name);
    return result;
  }
}


// return the number corresponding to a dataset name (negative
// if no such name was found).
vint8 vUciDataset::DatasetToNumber(const char * name)
{
  vint8 size = dataset_infos->size();
  vint8 i;
  vint8 result = -1;
  for (i = 0; i < size; i++)
  {
    if (strcmp(name, (*dataset_infos)[(vector_size) i]->name) == 0)
    {
      result = i;
      break;
    }
  }
  return result;
}


// Looks up the name of the class with given class id, exits with
// error message if no class name is found.
const char * vUciDataset::ClassName(vint8 class_id)
{
  const char * name = class_ids.GetKey((void *) (long) class_id);
  if (name == 0)
  {
    exit_error("Error in ClassName(%li)\n", class_id);
  }
  return name;
}


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
vint8 vUciDataset::Sample(float fraction)
{
  // Make sure that the fraction is in the (0, 1) range.
  if ((fraction < 0.5) || (fraction >= 1))
  {
    vPrint("Fraction %f should be in the [0.5, 1) range\n", fraction);
    return 0;
  }

  // First, get the training sizes and object indices for each class.
  training_vectors = GetFeatures(original_set, -1, -1);
  training_labels = GetLabels(original_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  // training_rows and test_rows will store the row indices
  // for training and test objects.
  vector<vint8> training_rows, test_rows;

  // Now, go through the indices for each class, and split them
  // into training and testing.
  vint8 i;
  for (i = 1; i <= classes; i++)
  {
    vint8 number = training_sizes(i);
    float numberf = (float)  number;
    vint8 training = round_number(numberf * fraction);
    if (training <= 0)
    {
      exit_error("Error: training = %li\n", training);
    }

    // training_picks and test_picks are the indices of objects in 
    // training_indices, not in the original set.
    vMatrix<vint8> training_picks = sample_without_replacement(0, number-1, training);

    vint8 j;
    for (j = 0; j < training_picks.Size(); j++)
    {
      vint8 index1 = training_picks(j);
      vint8 index2 = training_indices[i][(vector_size) index1];
      training_rows.push_back(index2);
    }
    
    if (training < number)
    {
      vMatrix<vint8> test_picks = vComplement(&training_picks, 0, number-1);
      for (j = 0; j < test_picks.Size(); j++)
      {
        vint8 index1 = test_picks(j);
        vint8 index2 = training_indices[i][(vector_size) index1];
        test_rows.push_back(index2);
      }
    }
  }

  vint8 cols = original_set.Cols();
  vector<vint8> col_indices;
  function_insert_range(&col_indices, 0, cols-1);
  v3dMatrix<float> temp_training_set = 
    function_select_grid(&original_set, &training_rows, &col_indices);
  training_vectors = GetFeatures(temp_training_set, -1, -1);
  training_labels = GetLabels(temp_training_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  v3dMatrix<float> temp_test_set = 
    function_select_grid(&original_set, &test_rows, &col_indices);
  test_vectors = GetFeatures(temp_test_set, -1, -1);
  test_labels = GetLabels(temp_test_set, -1, -1);
  test_number = test_vectors.Rows();

  return 1;
}




// Sample2(fraction) is like Sample, but the sampling is done on all objects
// together. This means that the the percentage of each class in 
// the test set, averaged over repeated calls with the same fraction 
// argument, will tend to be equal to the percentage of each class in 
// the original set, and this makes this method appropriate for use
// in cross-validation experiments.
vint8 vUciDataset::Sample2(float fraction)
{
  // Make sure that the fraction is in the (0, 1) range.
  if ((fraction < 0.5) || (fraction >= 1))
  {
    vPrint("Fraction %f should be in the [0.5, 1) range\n", fraction);
    return 0;
  }

  // First, get the training sizes and object indices for each class.
  training_vectors = GetFeatures(original_set, -1, -1);
  training_labels = GetLabels(original_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  // training_rows and test_rows will store the row indices
  // for training and test objects.
  vector<vint8> training_rows, test_rows;

  // training_picks and test_picks are the indices of objects in 
  // training_indices, not in the original set.
  vint8 original_number = original_set.Rows();
  float original_numberf = (float) original_number;

  // pick size of training set.
  vint8 training = round_number(original_numberf * fraction);
  if (training <= 0)
  {
    exit_error("Error: training = %li\n", training);
  }

  // pick objects for training set.
  vMatrix<vint8> training_picks = sample_without_replacement(0, original_number-1, training);

  vint8 j;
  for (j = 0; j < training_picks.Size(); j++)
  {
    vint8 index1 = training_picks(j);
    training_rows.push_back(index1);
  }
  
  // pick objects for test set.
  if (training < original_number)
  {
    vMatrix<vint8> test_picks = vComplement(&training_picks, 0, original_number-1);
    for (j = 0; j < test_picks.Size(); j++)
    {
      vint8 index1 = test_picks(j);
      test_rows.push_back(index1);
    }
  }

  // based on the training picks, create training set.
  vint8 cols = original_set.Cols();
  vector<vint8> col_indices;
  function_insert_range(&col_indices, 0, cols-1);
  v3dMatrix<float> temp_training_set = 
    function_select_grid(&original_set, &training_rows, &col_indices);
  training_vectors = GetFeatures(temp_training_set, -1, -1);
  training_labels = GetLabels(temp_training_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  // based on test picks, create test set.
  v3dMatrix<float> temp_test_set = 
    function_select_grid(&original_set, &test_rows, &col_indices);
  test_vectors = GetFeatures(temp_test_set, -1, -1);
  test_labels = GetLabels(temp_test_set, -1, -1);
  test_number = test_vectors.Rows();

  return 1;
}


// special function that provides a substitute for the unknown
// subject ids of the isolet dataset. The training set for
// isolet is essentially a union of four separate sets, of
// size 1560 (approximately) each. We know from the isolet
// description that subjects were not used for more than one
// of those four sets. We also know that the data from 
// each of the four sets appear in sequence in the concatenated
// training set. Based on that, here we assign to each object
// the id of the set that it came from, so that data with
// different ids should have been produced by different subjects.
// Some data (2 objects) are missing in the concatenated dataset,
// so there may be some mistakes (two at most).
vMatrix<vint8> vUciDataset::IsoletSubjectIds()
{
  vPrint("Isolet subject ids\n");
  vMatrix<vint8> result(1, 6238);
  vint8 i;
  for (i = 0; i < 6238; i++)
  {
    result(i) = i / 1560;
  }
  return result;
}

  
// printing some information, for debugging
vint8 vUciDataset::PrintSummary()
{
  vPrint("\n");
  const char * name = info->name;
  vPrint("dataset name: %s\n", name);
  vPrint("is_valid = %li, %li attributes, %li classes\n", 
          is_valid, attributes, classes);
  vPrint("%li original objects, %li training objects, %li test objects\n",
          original_set.Rows(), training_number, test_number);
  
  return 1;
}


// print the i-th row of original set, training vectors, test vectors.
vint8 vUciDataset::PrintRow(vint8 row)
{
  vPrint("\n");
  if (row < 0) 
  {
    return 0;
  }
  vint8 col, cols;
  cols = original_set.Cols();

  for (col = 0; col < cols; col++)
  {
    vPrint("Col %5i: ", col);
    if (original_set.check_bounds(row, col))
    {
      vPrint("%10.4f ", original_set(row, col));
    }
    else
    {
      vPrint("           ");
    }
    if (training_vectors.check_bounds(row, col))
    {
      vPrint("%10.4f ", training_vectors(row, col));
    }
    else
    {
      vPrint("           ");
    }
    if (test_vectors.check_bounds(row, col))
    {
      vPrint("%10.4f ", test_vectors(row, col));
    }
    else
    {
      vPrint("           ");
    }

    vPrint("\n");
  }

  if (row < training_number)
  {
    vint8 class_id = training_labels(row);
    const char * class_name = class_ids.GetKey((void *) (long) class_id);
    vPrint("training: class_id = %li, class_name = %s\n", class_id, class_name);
  }

  if (row < test_number)
  {
    vint8 class_id = test_labels(row);
    const char * class_name = class_ids.GetKey((void *) (long) class_id);
    vPrint("test:     class_id = %li, class_name = %s\n", class_id, class_name);
  }

  return 1;
}


// Print the number of training objects from each class.
vint8 vUciDataset::PrintTrainingSizes()
{
  vPrint("\n");

  vint8 i;
  for (i = 1; i <= classes; i++)
  {
    const char * class_name = ClassName(i);
    vint8 size = training_sizes(i);
    vPrint("class id = %li, name = %s, size = %li\n",
             i, class_name, size);
  }

  return 1;
}


// Print the attributes of the j-th training object of class i.
vint8 vUciDataset::PrintTrainingObject(vint8 i, vint8 j)
{
  vPrint("\n");

  if ((i < 1) || (i > classes))
  {
    vPrint("There are %li classes\n", classes);
  }
  
  vint8 size = training_indices[i].size();
  if ((j < 0) || (j >= size))
  {
    vPrint("There are %li training objects for class with id %li\n",
            size, i);
  }

  vint8 row, col, cols;
  row = training_indices[i][(vector_size) j];
  cols = training_vectors.Cols();

  vPrint("row = %li\n", row);
  for (col = 0; col < cols; col++)
  {
    vPrint("Col %5i: ", col);
    vPrint("%10.4f ", training_vectors(row, col));
    vPrint("\n");
  }

  return 1;
}


// Print the class_ids table.
vint8 vUciDataset::PrintClassIds()
{
  vPrint("\n");
  vint8 i;
  vPrint("%li classes\n", classes);
  for (i = 1; i <= classes; i++)
  {
    const char * name = class_ids.GetKey((void *) (long) i);
    vPrint("id = %5li, name = %s\n", i, name);
  }

  return 1;
}


// Print info about all the datasets for which info is available.
vint8 vUciDataset::PrintDatasetInfo()
{
  vint8 number = dataset_infos->size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    const char * name = (*dataset_infos)[(vector_size) i]->name;
    vPrint("id = %3li, name = %s\n", i, name);
  }

  return 1;
}


// Print the indices of all objects bevint8ing to class class_id,
// in the training set, and the test set.
vint8 vUciDataset::PrintClassIndices(vint8 class_id)
{
  vint8 size, i, counter;

  vPrint("\n");
  size = training_labels.Size();
  counter = 0;
  vPrint("Training set:\n");
  for (i = 0; i < size; i++)
  {
    if (training_labels(i) == class_id)
    {
      vPrint("%5li: %li\n", counter, i);
      counter++;
    }
  }
  vPrint("%li objects\n", counter);
  vPrint("\n");

  size = test_labels.Size();
  counter = 0;
  vPrint("Test set:\n");
  for (i = 0; i < size; i++)
  {
    if (test_labels(i) == class_id)
    {
      vPrint("%5li: %li\n", counter, i);
      counter++;
    }
  }
  vPrint("%li objects\n", counter);
  vPrint("\n");

  return 1;
}


// Split the original set into number sets, so that we can
// do cross validation. 
vint8 vUciDataset::Split(vint8 number)
{
  if ((number <= 1) || (number >= original_set.Rows()))
  {
    return 0;
  }

  last_split = number;

  // First, get the training sizes and object indices for each class.
  training_vectors = GetFeatures(original_set, -1, -1);
  training_labels = GetLabels(original_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  vint8 original_number = original_set.Rows();
  vint8 max_size = original_number / (number - 1) + 1;
  split_numbers = vMatrix<vint8>(1, number);
  split_indices = vMatrix<vint8>(number, max_size);
  function_enter_value(&split_numbers, (vint8) 0);
  function_enter_value(&split_indices, (vint8) -1);
  vint8 column_counter = 0;
  vint8 row_counter = 0;

  // Now, go through the indices for each class, and split them
  // into training and testing.
  vint8 i, j;
  for (i = 1; i <= classes; i++)
  {
    vint8 class_size = training_sizes(i);
    vMatrix<vint8> permutation = vPermutation(0, class_size - 1);
    for (j = 0; j < class_size; j++)
    {
      vint8 index1 = permutation(j);
      vint8 index = training_indices[i][(vector_size) index1];
      split_indices(row_counter, column_counter) = index;
      split_numbers(row_counter) = split_numbers(row_counter) + 1;
      row_counter = row_counter + 1;
      if (row_counter >= number)
      {
        row_counter = 0;
        column_counter = column_counter + 1;
        if (column_counter >= max_size)
        {
          exit_error("Error: column_counter = %li, max_size = %li\n",
                          column_counter, max_size);
        }
      }
    }
  }

  return 1;
}


// Assuming that we have already called Split with some number
// greater than i, here we choose the i-th test for testing
// and the rest for training.
vint8 vUciDataset::CrossValidationSet(vint8 index)
{
  // Make sure that the fraction is in the (0, 1) range.
  if ((index < 0) || (index >= last_split))
  {
    vPrint("index %li should be from 0 to %li\n", 
            index, last_split - (vint8) 1);
    return 0;
  }

  // training_rows and test_rows will store the row indices
  // for training and test objects.
  vector<vint8> training_rows, test_rows;

  // first, store the training indices.
  vint8 i, j;
  for (i = 0; i < last_split; i++)
  {
    if (i == index)
    {
      continue;
    }
    
    vint8 split_number = split_numbers(i);
    for (j = 0; j < split_number; j++)
    {
      vint8 split_index = split_indices(i, j);
      training_rows.push_back(split_index);
    }
  }
  
  // now, store the test indices
  vint8 split_number = split_numbers(index);
  for (i = 0; i < split_number; i++)
  {
    vint8 split_index = split_indices(index, i);
    test_rows.push_back(split_index);
  }

  vint8 cols = original_set.Cols();
  vector<vint8> col_indices;
  function_insert_range(&col_indices, 0, cols-1);
  v3dMatrix<float> temp_training_set = 
    function_select_grid(&original_set, &training_rows, &col_indices);
  training_vectors = GetFeatures(temp_training_set, -1, -1);
  training_labels = GetLabels(temp_training_set, -1, -1);
  training_number = training_vectors.Rows();
  attributes = training_vectors.Cols();
  ProcessTraining();

  v3dMatrix<float> temp_test_set = 
    function_select_grid(&original_set, &test_rows, &col_indices);
  test_vectors = GetFeatures(temp_test_set, -1, -1);
  test_labels = GetLabels(temp_test_set, -1, -1);
  test_number = test_vectors.Rows();

//  vMatrix<vint8> training_rows_matrix = matrix_from_vector(&training_rows);
//  training_rows_matrix.PrintInt("training rows");
//  v3dMatrix<vint8> test_rows_matrix = copy_horizontal_line(&split_indices, index);
//  vMatrix<vint8> test_rows2(&test_rows_matrix);
//  test_rows2.PrintInt("test rows");

  return 1;
}


vint8 vUciDataset::PrintSplits()
{
  split_indices.PrintInt("split_indices");
  return 1;
}



