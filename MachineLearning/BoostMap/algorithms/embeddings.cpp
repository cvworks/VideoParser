
#include "embeddings.h"
#include "basics/simple_algo.h"
#include "basics/simple_algo_templates.h"
#include "basics/local_data.h"


#include "basics/definitions.h"




// initialize a FastMap object based on the names of 
// an entire dataset and a training dataset.
// We use the specified training indices to form the training
// triples. It seems there may be a bug, in that we 
// assume that the number of candidates is equal to the number
// of training objects, and that assumption is not always true.
// Furthermore, it seems that the pivots come from the training set,
// maybe they should be coming from the candidate set.
class_FastMap::class_FastMap(const char * in_dataset_name, const char * in_sample_name)
{
  is_valid = 0;
  dataset_name = vCopyString(in_dataset_name);
  data = new BoostMap_data(g_data_directory, in_sample_name);
  if (data->valid() <= 0)
  {
    return;
  }

  // sanity check, verify that the dataset indices do match
  // the size of the distance matrix.
  dataset_indices = data->candidate_objects();
  vMatrix<float> distances = data->LoadCandCandDistances();
  if ((distances.Rows() != dataset_indices.Size()) ||
      (distances.Cols() != dataset_indices.Size()))
  {
    exit_error("error: class_FastMap, bad sizes\n");
  }

  if ((dataset_indices.valid() <= 0) || 
      (distances.valid() <= 0))
  {
    exit_error("error: class_FastMap, invalid matrices\n");
  }

  vint8 size = dataset_indices.Size();
  vMatrix<vint8> candidate_indices = function_range_matrix(0, size-1);
  vMatrix<vint8> validation_indices = function_range_matrix(0, size-1);
  fastmap = new V_FastMap(distances, candidate_indices, 
                          validation_indices, 100000);
  is_valid = 1;
}


// Here, we initialize a FastMap object, by loading
// from file some pivots that had already been selected.
// Notice that, in this constructor, we do not construct
// an object capable of picking new pivot pairs (i.e. 
// capable of adding dimensions to the embedding).
class_FastMap::class_FastMap(const char * in_dataset_name, const char * in_sample_name,
                   const char * saved_file)
{
  is_valid = 0;
  dataset_name = vCopyString(in_dataset_name);
  data = new BoostMap_data(g_data_directory, in_sample_name);
  if (data->valid() <= 0)
  {
    return;
  }
  dataset_indices = data->candidate_objects();
  fastmap = 0;
  is_valid = Load(saved_file);
}


class_FastMap::~class_FastMap()
{
  vdelete(fastmap);
  vdelete(data);
  vdelete2(dataset_name);
}


// Select the next pair of pivot objects.
long class_FastMap::NextStep()
{
  if (fastmap == 0) 
  {
    return 0;
  }

  fastmap->NextStep();
  fastmap->PrintSummary();
  return 1;
}


// Embed the test set of the entire dataset, and save
// the result into a file. Every row is 
// the embedding of a test object.
vMatrix<float> class_FastMap::EmbedTestSet()
{
  // for debugging
  char * output2 = TestEmbeddingPath();
  vdelete2(output2);

  // do some sanity checks
  if ((pivot_indices.valid() <= 0) ||
      (optimized_pivot_distances.valid() <= 0) ||
      (pivot_indices.Rows() * 2 != optimized_pivot_distances.Rows()) || 
      (pivot_indices.Rows() * 2 != optimized_pivot_distances.Cols()) || 
      (pivot_indices.Cols() != 2))
  {
    return vMatrix<float>();
  }

  // open file holding distances from test set to database.
  char * filename = BoostMap_data::TestTrainDistancesPath(g_data_directory, dataset_name);
  class_file * fp = BoostMap_data::OpenObjectDistancesFile(filename);
  if (fp->file_pointer == 0)
  {
    vPrint("failed to open %s\n", filename);
    vdelete2(filename);
    return vMatrix<float>();
  }
  vdelete2(filename);

  // load test labels and training labels.
  vMatrix<float> test_labels = BoostMap_data::LoadTestLabels(g_data_directory, dataset_name);
  if (test_labels.valid() <= 0)
  {
    vPrint("failed to load test labels\n");
    return vMatrix<float>();
  }

  vMatrix<float> train_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, dataset_name);
  if (train_labels.valid() <= 0)
  {
    vPrint("failed to load train labels\n");
    return vMatrix<float>();
  }

  vint8 test_number = test_labels.Size();
  vint8 training_number = train_labels.Size();
  vint8 dimensions = pivot_indices.Rows();

  vMatrix<float> result(test_number, dimensions);

  long i;
  vPrint("\n");
  // go through all test objects.
  for (i = 0; i < test_number; i++)
  {
    // get distances to entire database for i-th object.
    vMatrix<float> distances = BoostMap_data::NextObjectDistances(fp, training_number);
    if (distances.valid() <= 0)
    {
      vPrint("failed to load distances for %li\n", (long) i);
    }

    // get distances from test object to pivot objects
    vMatrix<float> pivot_distances = class_BoostMap::ReferenceDistances(distances, pivot_indices);

    // compute embedding of test object.
    vMatrix<float> embedding = V_FastMap::EmbeddingB(pivot_distances, 
                                                     optimized_pivot_distances);
    function_put_row(&embedding, &result, i);
    vPrint("embedded object %li of %li\r", (long) (i+1), (long) test_number);
  }
  vPrint("\n");

  // save embedding of test set into a file.
  char * output = TestEmbeddingPath();
  long success = result.Write(output);
  if (success <= 0)
  {
    vPrint("failed to write result to %s\n", output);
  }
  else
  {
    vPrint("wrote result to %s\n", output);
  }

  return result;
}


// same as previous function, but for training set. In principle,
// I should have both EmbedTestSet and EmbedTraining set call a 
// function that would do most of the work, instead of duplicating
// code as I'm doing now.
vMatrix<float> class_FastMap::EmbedTrainingSet()
{
  // for debugging
  char * output2 = TrainingEmbeddingPath();
  vdelete2(output2);
  
  if ((pivot_indices.valid() <= 0) ||
      (optimized_pivot_distances.valid() <= 0) ||
      (pivot_indices.Rows() * 2 != optimized_pivot_distances.Rows()) || 
      (pivot_indices.Rows() * 2 != optimized_pivot_distances.Cols()) || 
      (pivot_indices.Cols() != 2))
  {
    return vMatrix<float>();
  }

  char * filename = BoostMap_data::TrainTrainDistancesPath(g_data_directory, dataset_name);
  class_file * fp = BoostMap_data::OpenObjectDistancesFile(filename);
  if (fp->file_pointer == 0)
  {
    vPrint("failed to open %s\n", filename);
    vdelete2(filename);
    return vMatrix<float>();
  }

  vdelete2(filename);
 
  vMatrix<float> train_labels = BoostMap_data::LoadTrainingLabels(g_data_directory, dataset_name);
  if (train_labels.valid() <= 0)
  {
    vPrint("failed to load train labels\n");
    return vMatrix<float>();
  }

  vint8 training_number = train_labels.Size();
  vint8 dimensions = pivot_indices.Rows();

  vMatrix<float> result(training_number, dimensions);

  long i;
  vPrint("\n");
  for (i = 0; i < training_number; i++)
  {
    vMatrix<float> distances = BoostMap_data::NextObjectDistances(fp, training_number);
    if (distances.valid() <= 0)
    {
      vPrint("failed to load distances for %li\n", (long) i);
    }

    vMatrix<float> pivot_distances = class_BoostMap::ReferenceDistances(distances, pivot_indices);
    vMatrix<float> embedding = V_FastMap::EmbeddingB(pivot_distances, 
                                                     optimized_pivot_distances);
    vPrint("embedded object %li of %li\r", (long) (i+1), (long) training_number);
    function_put_row(&embedding, &result, i);
  }
  vPrint("\n");

  char * output = TrainingEmbeddingPath();
  long success = result.Write(output);
  if (success <= 0)
  {
    vPrint("failed to write result to %s\n", output);
  }
  else
  {
    vPrint("wrote result to %s\n", output);
  }

  return result;
}


// Save the chosen pivots into a filename, together with 
// the optimized distances.
long class_FastMap::Save(const char * filename)
{
  if (fastmap == 0)
  {
    return 0;
  }

  if (fastmap->StepsDone() == 0)
  {
    return 0;
  }

  vint8 steps = fastmap->StepsDone();

  // get pivot indices (indices into training set)
  vMatrix<vint8> temp_pivot_indices1 = fastmap->PivotsMatrix();
  integer_matrix temp_pivot_indices(&temp_pivot_indices1);
  if ((temp_pivot_indices.Rows() != steps) || (temp_pivot_indices.Cols() != 2))
  {
    exit_error("error: adsfjasdlfjsda;\n");
  }

  // get optimized pivot distances
  vMatrix<float> original_distances = fastmap->OriginalPivotDistances(vint8_matrix(&temp_pivot_indices));
  optimized_pivot_distances = fastmap->OptimizedPivotDistances(original_distances);

  // get translated pivot indices (indices into entire dataset)
  pivot_indices = TranslateIndices(vint8_matrix(temp_pivot_indices));

  // open output file
  char * pathname = Pathname(filename);
  FILE * fp = fopen(pathname, vFOPEN_WRITE);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", pathname);
    vdelete2(pathname);
    return 0;
  }

  long problems = 0;
  long success;

  // write data.
  success = temp_pivot_indices.Write(fp);
  if (success <= 0)
  {
    vPrint("failed to save temp_pivot_indices\n");
    problems = 1;
  }
  success = pivot_indices.Write(fp);
  if (success <= 0)
  {
    vPrint("failed to save pivot_indices\n");
    problems = 1;
  }
  success = optimized_pivot_distances.Write(fp);
  if (success <= 0)
  {
    vPrint("failed to save optimized_pivot_distances\n");
    problems = 1;
  }

  fclose(fp);
  
  return (problems == 0);
}


// Translate indices, from indices of the training dataset
// to indices of the entire dataset.
vMatrix<vint8> class_FastMap::TranslateIndices(vMatrix<vint8> indices)
{
  if (indices.valid() <= 0)
  {
    exit_error("error: kdskewhgtjwe\n");
  }

  vint8 steps = indices.Rows();
  vMatrix<vint8> result(steps, 2);
  vint8 i;
  for (i = 0; i < steps; i++)
  {
    vint8 pivot1 = indices(i, 0);
    vint8 index1 = dataset_indices(pivot1);
    result(i, 0) = index1;
    vint8 pivot2 = indices(i, 1);
    vint8 index2 = dataset_indices(pivot2);
    result(i, 1) = index2;
  }

  return result;
}


// load pivots and optimized distances from a file.
long class_FastMap::Load(const char * filename)
{
  // open the input file
  char * pathname = Pathname2(filename);
  FILE * fp = fopen(pathname, vFOPEN_READ);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", pathname);
    vdelete2(pathname);
    return 0;
  }
  vdelete2(pathname);

  // read the data
  long problems = 0;
  vMatrix<integer> temp_pivot_indices = vMatrix<integer>::Read(fp);
  if (temp_pivot_indices.valid() <= 0)
  {
    vPrint("failed to load temp_pivot_indices\n");
    problems = 1;
  }
  pivot_indices = vMatrix<vint8>::Read(fp);
  if (pivot_indices.valid() <= 0)
  {
    vPrint("failed to load pivot_indices\n");
    problems = 1;
  }
  optimized_pivot_distances = vMatrix<float>::Read(fp);
  if (optimized_pivot_distances.valid() <= 0)
  {
    vPrint("failed to load optimized_pivot_distances\n");
    problems = 1;
  }

  fclose(fp);
  
  return (problems == 0);
}


// prints the index of the object (mapping from sample index to
// dataset index), and the embedding of the object, based on
// the current fastmap.
long class_FastMap::PrintSampleObject(long i)
{
  if (fastmap == 0)
  {
    return 0;
  }

  vMatrix<vint8> pivots = fastmap->PivotsMatrix();
  vMatrix<float> distances = fastmap->OriginalPivotDistances(pivots);
  vMatrix<float> distances2 = fastmap->OptimizedPivotDistances(distances);
  vMatrix<float> embedding = fastmap->Embedding1(i, distances2);
  embedding.Print("embedding");
  vint8 index = dataset_indices(i);
  vPrint("i = %li, index = %li\n", (long) i, (long) index);
  return 1;
}


// prints the embedding of a training object, by reading
// it from the traintrain file, and then embedding
// it based on the current fastmap.
long class_FastMap::PrintTrainingObject(long i)
{
  if ((pivot_indices.valid() <= 0) ||
      (optimized_pivot_distances.valid() <= 0))
  {
    vPrint("invalid info\n");
    return 0;
  }

  char * pathname = data->TrainTrainDistancesPath(g_data_directory, dataset_name);
  vMatrix<float> distances = data->ObjectDistances(pathname, i);
  if (distances.valid() <= 0)
  {
    vPrint("failed to read distances\n");
    vdelete2(pathname);
    return 0;
  }

  vdelete2(pathname);
  vMatrix<float> pivot_distances = class_BoostMap::ReferenceDistances(distances, pivot_indices);
  vMatrix<float> embedding = V_FastMap::EmbeddingB(pivot_distances, 
                                                   optimized_pivot_distances);
  embedding.Print("embedding");
  return 1;
}


// Prints pivots (their true indices).
long class_FastMap::PrintPivots()
{
  pivot_indices.Print("pivots");
  optimized_pivot_distances.Print("optimized_pivot_distances");
  return 1;
}


long class_FastMap::valid()
{
  return is_valid;
}


// Construct a pathname for saving the embedding 
// description (using Save), based on filename,
// and based on the number of pivot pairs chosen
// so far. We use the number of pivot pairs so
// that we don't always write over the same file,
// but instead we cyclically update a bunch of files.
char * class_FastMap::Pathname(const char * filename)
{
  char * directory = vJoinPaths3(g_data_directory, "experiments", "boost_map");
  vint8 steps = fastmap->StepsDone();
  char * step_string = string_from_number(steps);
  char * simple_name = vMergeStrings4(filename, "_", step_string, ".txt");
  char * result = vJoinPaths(directory, simple_name);
  vdelete2(directory);
  vdelete2(step_string);
  vdelete2(simple_name);
  return result;
}


// construct a complete pathname for loading
// the description of the embedding (using Load),
// based on filename.
char * class_FastMap::Pathname2(const char * filename)
{
  char * directory = vJoinPaths3(g_data_directory, "experiments", "boost_map");
  char * simple_name = vMergeStrings2(filename, ".txt");
  char * result = vJoinPaths(directory, simple_name);
  vdelete2(directory);
  vdelete2(simple_name);
  return result;
}


// directory where we save and load embedding
// descriptions (using Save and Load).
char * class_FastMap::EmbeddingDirectory()
{
  char * result = vJoinPaths4(g_data_directory, "experiments", "bm_datasets",
                              "fastmap");
  return result;
}


// Complete pathname where we save the embedding
// of the test set of the entire dataset.
char * class_FastMap::TestEmbeddingPath()
{
  if (pivot_indices.valid() <= 0)
  {
    return 0;
  }

  vint8 dimensions = pivot_indices.Rows();
  char * dim_string = string_from_number(dimensions);

  char * simple_name = vMergeStrings3(data->get_original_name(), "_test_", dim_string);
  char * directory = EmbeddingDirectory();
  char * result = vJoinPaths(directory, simple_name);
  vdelete2(dim_string);
  vdelete2(directory);
  vdelete2(simple_name);
  return result;
}


// Complete pathname where we save the embedding
// of the training set of the entire dataset.
char * class_FastMap::TrainingEmbeddingPath()
{
  if (pivot_indices.valid() <= 0)
  {
    return 0;
  }

  vint8 dimensions = pivot_indices.Rows();
  char * dim_string = string_from_number(dimensions);

  char * simple_name = vMergeStrings3(data->get_original_name(), "_train_", dim_string);
  char * directory = EmbeddingDirectory();
  char * result = vJoinPaths(directory, simple_name);
  vdelete2(dim_string);
  vdelete2(directory);
  vdelete2(simple_name);
  return result;
}



class_Bourgain::class_Bourgain()
{
  Initialize();
}


class_Bourgain::class_Bourgain(vint8 in_n, vint8 in_objects_to_use)
{
  Initialize();
  number = in_n;
  objects_to_use = in_objects_to_use;

  if ((number >= 1) && (objects_to_use <= number))
  {
    is_valid = 1;
    PickSets();
  }
}


class_Bourgain::~class_Bourgain()
{
}


vint8 class_Bourgain::Initialize()
{
  number = 0;
  objects_to_use = 0;
  return 1;
}


vint8 class_Bourgain::PickSets()
{
  vint8 dimensions = Dimensions();
  vint8 logn = (vint8) (log((float) objects_to_use) / log((float) 2.0));
  picks = sample_without_replacement(0, number-1, objects_to_use);

  vint8 i, j;
  for (i = 1; i <= logn; i++)
  {
    vint8 size = round_number(pow(2.0, (double) i));
    if (size > number)
    {
      function_warning("Error, in PickSets, size > number: %li > %li\n",
                (long) size, (long) number);
    }
    for (j = 0; j < logn; j++)
    {
      vMatrix<vint8> reference_set = sample_without_replacement(0, objects_to_use-1, size);
      vint8 k;
      for (k = 0; k < size; k++)
      {
        vint8 index1 = reference_set(k);
        vint8 index2 = picks(index1);
        reference_set(k) = index2;
      }
      reference_sets.push_back(reference_set);
    }
  }

  return 1;
}


vint8 class_Bourgain::Number()
{
  return number;
}


vint8 class_Bourgain::ObjectsToUse()
{
  return objects_to_use;
}


vint8 class_Bourgain::Dimensions()
{
  vint8 logn = (vint8) (log((float) objects_to_use) / log((float) 2.0));
  vint8 result = logn * logn;
  return result;
}


char * class_Bourgain::Directory()
{
  char * result = vJoinPaths4(g_data_directory, "experiments", "bm_datasets",
                              "bourgain");
  return result;
}


// When we save the test and training set embeddings of
// a dataset, we generate output filenames automatically. However,
// we append a number to the end of those filenames, to make sure
// we do not write over existing files. This function finds the
// next available number that we can append to a filename, to 
// generate a filename that does not exist right now.
vint8 class_Bourgain::FindNumber(const char * dataset)
{
  vint8 dimensions = Dimensions();
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_sets_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  vdelete2(dimension_string);
  vdelete2(directory);
  vdelete2(simple_base);

  vint8 i = 0;
  while(1)
  {
    i = i+1;
    char * i_string = string_from_number(i);
    char * pathname = vMergeStrings3(base, "_", i_string);
    vdelete2(i_string);
    if (vFileExists(pathname) <= 0)
    {
      vdelete2(pathname);
      break;
    }
    vPrint("%s exists\n", pathname);
    vdelete2(pathname);
  }

  vdelete2(base);
  return i;
}


char * class_Bourgain::TrainingPath(const char * dataset, vint8 dimensions, vint8 i)
{
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_tr_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);

  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);

  return pathname;
}


char * class_Bourgain::TestPath(const char * dataset, vint8 dimensions, vint8 i)
{
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_te_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);
 
  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);

  return pathname;
}


// where to save the reference sets.
char * class_Bourgain::EmbeddingPath(const char * dataset, vint8 i)
{
  vint8 dimensions = Dimensions();
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_sets_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);

  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);


  return pathname;
}

  
vMatrix<float> class_Bourgain::EmbedTraining(const char * dataset)
{
  vint8 number = BoostMap_data::TrainingNumber(dataset);
  char * path = BoostMap_data::TrainTrainDistancesPath(g_data_directory, dataset);
  class_file * fp = BoostMap_data::OpenObjectDistancesFile(path);
  if (fp->file_pointer == 0)
  {
    vPrint("failed to open %s\n", path);
    vdelete2(path);
    return vMatrix<float>();
  }
  vdelete2(path);
  vMatrix<float> result = EmbedObjects(fp, number, number);
  fclose(fp);
  return result;
}


vMatrix<float> class_Bourgain::EmbedTest(const char * dataset)
{
  vint8 training_number = BoostMap_data::TrainingNumber(dataset);
  vint8 test_number = BoostMap_data::TestNumber(dataset);
  char * path = BoostMap_data::TestTrainDistancesPath(g_data_directory, dataset);
  class_file * fp = BoostMap_data::OpenObjectDistancesFile(path);
  if (fp->file_pointer == 0)
  {
    vPrint("failed to open %s\n", path);
    vdelete2(path);
    return vMatrix<float>();
  }
  vdelete2(path);
  vMatrix<float> result = EmbedObjects(fp, training_number, test_number);
  fclose(fp);
  return result;
}


vMatrix<float> class_Bourgain::EmbedObjects(class_file * fp, vint8 training, vint8 size)
{
  vint8 dimensions = Dimensions();
  if (reference_sets.size() != dimensions)
  {
    exit_error("error: bourgain dimensions = %li, but only %li sets\n",
                   (long) dimensions, (long) reference_sets.size());
  }

  vMatrix<float> result(size, dimensions);
  vint8 i, j;

  vPrint("\n");
  for (i = 0; i < size; i++)
  {
    vMatrix<float> distances = BoostMap_data::NextObjectDistances(fp, training);
    if (distances.valid() <= 0)
    {
      exit_error("\nerror: invalid distances\n");
    }

    if (distances.Size() < number)
    {
      exit_error("\nerror: distances.Size() = %li, number = %li\n",
                     (long) distances.Size(), (long) number);
    }

    for (j = 0; j < dimensions; j++)
    {
      result(i, j) = MinDistance(distances, reference_sets[(vector_size) j]);
    }
    vPrint("embedded object %li of %li\r", (long) (i+1), (long) size);
  }
  vPrint("\n");

  return result;
}


float class_Bourgain::MinDistance(vMatrix<float> distancesm, 
                             vMatrix<vint8> reference_setm)
{
  vint8 i;
  vint8 size = reference_setm.Size();
  vArray(float) distances = distancesm.Matrix();
  vArray(vint8) references = reference_setm.Matrix();

  float min_distance = distances[references[0]];
  for (i = 1; i < size; i++)
  {
    float distance = distances[references[i]];
    if (distance < min_distance)
    {
      min_distance = distance;
    }
  }

  return min_distance;
}


vint8 class_Bourgain::SaveEmbeddings(const char * dataset)
{
  vint8 dimensions = Dimensions();
  vint8 number = FindNumber(dataset);
  vPrint("dimensions = %li, experiment number = %li\n", (long) dimensions, (long) number);
  char * test_file = TestPath(dataset, dimensions, number);
  char * training_file = TrainingPath(dataset, dimensions, number);

  if ((vWritableFileExists(test_file) <= 0) ||
      (vWritableFileExists(test_file) <= 0))
  {
    vPrint("writable files for saving bourgain embedding do not exist\n");
    vdelete2(test_file);
    vdelete2(training_file);
    return 0;
  }

  vint8 result = 1;
  vint8 success;
  char * filename = EmbeddingPath(dataset, number);
  success = Save(filename);
  if (success <= 0)
  {
    result = 0;
  }
  vdelete2(filename);

  vMatrix<float> embedding;

  embedding = EmbedTest(dataset);
  success = embedding.Write(test_file);
  if (success <= 0)
  {
    vPrint("failed to save to %s\n", test_file);
    result = 0;
  }

  embedding = EmbedTraining(dataset);
  success = embedding.Write(training_file);
  if (success <= 0)
  {
    vPrint("failed to save to %s\n", training_file);
    result = 0;
  }

  vdelete2(test_file);
  vdelete2(training_file);
  return result;
}


vint8 class_Bourgain::Save(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", filename);
    return 0;
  }

  fprintf(fp, "bourgain embedding:\n");
  fprintf(fp, "number = %li, objects_to_use = %li\n", (long) number, (long) objects_to_use);
  
  vint8 dimensions = Dimensions();
  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    fprintf(fp, "\nreference_set %li:\n", (long) i);
    reference_sets[(vector_size) i].store_text(fp);
  }
  fclose(fp);

  return 1;
}


vint8 class_Bourgain::Print()
{
  vPrint("bourgain embedding:\n");
  vPrint("number = %li, objects_to_use = %li\n", (long) number, (long) objects_to_use);

  picks.PrintInt("picks");
  vint8 dimensions = Dimensions();
  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    vPrint("\nreference_set %li:\n", (long) i);
    reference_sets[(vector_size) i].PrintInt();
  }

  return 1;
}


vMatrix<float> class_Bourgain::LoadTrainingEmbedding(const char * dataset,
                                                vint8 dimensions, vint8 number)
{
  char * filename = TrainingPath(dataset, dimensions, number);
  vMatrix<float> result = vMatrix<float>::Read(filename);
  if (result.valid() <= 0)
  {
    vPrint("failed to read from %s\n", filename);
  }
  vdelete2(filename);
  return result;
}


vMatrix<float> class_Bourgain::LoadTestEmbedding(const char * dataset,
                                            vint8 dimensions, vint8 number)
{
  char * filename = TestPath(dataset, dimensions, number);
  vMatrix<float> result = vMatrix<float>::Read(filename);
  if (result.valid() <= 0)
  {
    vPrint("failed to read from %s\n", filename);
  }
  vdelete2(filename);
  return result;
}


class_SparseMap::class_SparseMap()
{
  Initialize();
}


class_SparseMap::class_SparseMap(vint8 in_n, vint8 in_objects_to_use)
{
  Initialize();
  number = in_n;
  objects_to_use = in_objects_to_use;

  if ((number >= 1) && (objects_to_use <= number))
  {
    is_valid = 1;
    PickSets();
  }
}


class_SparseMap::~class_SparseMap()
{
}


vint8 class_SparseMap::Initialize()
{
  number = 0;
  objects_to_use = 0;
  return 1;
}


vint8 class_SparseMap::PickSets()
{
  vint8 dimensions = Dimensions();
  vint8 logn = (vint8) (log((float) objects_to_use) / log((float) 2.0));
  picks = sample_without_replacement(0, number-1, objects_to_use);

  vint8 i, j;
  for (i = 1; i <= logn; i++)
  {
    vint8 size = round_number(pow(2.0, (double) i));
    if (size > number)
    {
      function_warning("Error, in PickSets, size > number: %li > %li\n",
                (long) size, (long) number);
    }
    for (j = 0; j < logn; j++)
    {
      vMatrix<vint8> reference_set = sample_without_replacement(0, objects_to_use-1, size);
      vint8 k;
      for (k = 0; k < size; k++)
      {
        vint8 index1 = reference_set(k);
        vint8 index2 = picks(index1);
        reference_set(k) = index2;
      }
      reference_sets.push_back(reference_set);
    }
  }

  // sort the entries of each reference set.
  for (i = 0; i < dimensions; i++)
  {
    vMatrix<vint8> reference_set = reference_sets[(vector_size) i];
    vector<vint8> set_vector;
    vector_from_matrix(&reference_set, &set_vector);
    std::sort(set_vector.begin(), set_vector.end(), less<vint8>());
    reference_sets[(vector_size) i] = matrix_from_vector(&set_vector);
  }

  return 1;
}


vint8 class_SparseMap::Number()
{
  return number;
}


vint8 class_SparseMap::ObjectsToUse()
{
  return objects_to_use;
}


vint8 class_SparseMap::Dimensions()
{
  vint8 logn = (vint8) (log((float) objects_to_use) / log((float) 2.0));
  vint8 result = logn * logn;
  return result;
}


char * class_SparseMap::Directory()
{
  char * result = vJoinPaths4(g_data_directory, "experiments", "bm_datasets",
                              "sparsemap");
  return result;
}


// When we save the test and training set embeddings of
// a dataset, we generate output filenames automatically. However,
// we append a number to the end of those filenames, to make sure
// we do not write over existing files. This function finds the
// next available number that we can append to a filename, to 
// generate a filename that does not exist right now.
vint8 class_SparseMap::FindNumber(const char * dataset)
{
  vint8 dimensions = Dimensions();
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_sets_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  vdelete2(dimension_string);
  vdelete2(directory);
  vdelete2(simple_base);

  vint8 i = 0;
  while(1)
  {
    i = i+1;
    char * i_string = string_from_number(i);
    char * pathname = vMergeStrings3(base, "_", i_string);
    vdelete2(i_string);
    if (vFileExists(pathname) <= 0)
    {
      vdelete2(pathname);
      break;
    }
    vPrint("%s exists\n", pathname);
    vdelete2(pathname);
  }

  vdelete2(base);
  return i;
}


char * class_SparseMap::TrainingPath(const char * dataset, vint8 dimensions, vint8 i)
{
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_tr_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);

  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);

  return pathname;
}


char * class_SparseMap::TestPath(const char * dataset, vint8 dimensions, vint8 i)
{
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_te_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);
 
  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);

  return pathname;
}


// where to save the reference sets.
char * class_SparseMap::EmbeddingPath(const char * dataset, vint8 i)
{
  vint8 dimensions = Dimensions();
  char * dimension_string = string_from_number(dimensions);
  char * simple_base = vMergeStrings3(dataset, "_sets_", dimension_string);
  char * directory = Directory();
  char * base = vJoinPaths2(directory, simple_base);
  char * i_string = string_from_number(i);
  char * pathname = vMergeStrings3(base, "_", i_string);

  vdelete2(dimension_string);
  vdelete2(simple_base);
  vdelete2(directory);
  vdelete2(base);
  vdelete2(i_string);


  return pathname;
}


vMatrix<float> class_SparseMap::EmbedObjects(class_file * fp, vint8 training, vint8 size)
{
  vint8 dimensions = Dimensions();
  if (reference_sets.size() != dimensions)
  {
    exit_error("error: sparsemap dimensions = %li, but only %li sets\n",
                   (long) dimensions, (long) reference_sets.size());
  }

  vMatrix<float> result(size, dimensions);
  vint8 i, j;

  vPrint("\n");
  for (i = 0; i < size; i++)
  {
    vMatrix<float> distances = BoostMap_data::NextObjectDistances(fp, training);
    if (distances.valid() <= 0)
    {
      exit_error("\nerror: invalid distances\n");
    }

    if (distances.Size() < number)
    {
      exit_error("\nerror: distances.Size() = %li, number = %li\n",
                     (long) distances.Size(), (long) number);
    }

    for (j = 0; j < dimensions; j++)
    {
      result(i, j) = MinDistance(distances, reference_sets[(vector_size) j]);
    }
    vPrint("embedded object %li of %li\r", (long) (i+1), (long) size);
  }
  vPrint("\n");

  return result;
}


float class_SparseMap::MinDistance(vMatrix<float> distancesm, 
                             vMatrix<vint8> reference_setm)
{
  vint8 i;
  vint8 size = reference_setm.Size();
  vArray(float) distances = distancesm.Matrix();
  vArray(vint8) references = reference_setm.Matrix();

  float min_distance = distances[references[0]];
  for (i = 1; i < size; i++)
  {
    float distance = distances[references[i]];
    if (distance < min_distance)
    {
      min_distance = distance;
    }
  }

  return min_distance;
}


vint8 class_SparseMap::SaveEmbeddings(const char * dataset)
{
  vint8 dimensions = Dimensions();
  vint8 number = FindNumber(dataset);
  vPrint("dimensions = %li, experiment number = %li\n", (long) dimensions, (long) number);
  char * test_file = TestPath(dataset, dimensions, number);
  char * training_file = TrainingPath(dataset, dimensions, number);

  if ((vWritableFileExists(test_file) <= 0) ||
      (vWritableFileExists(test_file) <= 0))
  {
    vPrint("writable files for saving sparsemap embedding do not exist\n");
    vdelete2(test_file);
    vdelete2(training_file);
    return 0;
  }

  vint8 result = 1;
  vint8 success;
  char * filename = EmbeddingPath(dataset, number);
  success = Save(filename);
  if (success <= 0)
  {
    result = 0;
  }
  vdelete2(filename);

  vMatrix<float> training_embedding = EmbedTraining(dataset);
  success = training_embedding.Write(training_file);
  if (success <= 0)
  {
    vPrint("failed to save to %s\n", training_file);
    result = 0;
  }

  vMatrix<float> test_embedding = EmbedTest(dataset, training_embedding);
  success = test_embedding.Write(test_file);
  if (success <= 0)
  {
    vPrint("failed to save to %s\n", test_file);
    result = 0;
  }

  vdelete2(test_file);
  vdelete2(training_file);
  return result;
}


vint8 class_SparseMap::Save(const char * filename)
{
  FILE * fp = fopen(filename, vFOPEN_WRITE);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", filename);
    return 0;
  }

  fprintf(fp, "sparsemap embedding:\n");
  fprintf(fp, "number = %li, objects_to_use = %li\n", (long) number, (long) objects_to_use);
  
  vint8 dimensions = Dimensions();
  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    fprintf(fp, "\nreference_set %li:\n", (long) i);
    reference_sets[(vector_size) i].WriteText(fp);
  }
  fclose(fp);

  return 1;
}


vint8 class_SparseMap::Print()
{
  vPrint("sparsemap embedding:\n");
  vPrint("number = %li, objects_to_use = %li\n", (long) number, (long) objects_to_use);

  picks.PrintInt("picks");
  vint8 dimensions = Dimensions();
  vint8 i;
  for (i = 0; i < dimensions; i++)
  {
    vPrint("\nreference_set %li:\n", (long) i);
    reference_sets[(vector_size) i].PrintInt();
  }

  return 1;
}


vMatrix<float> class_SparseMap::LoadTrainingEmbedding(const char * dataset,
                                                vint8 dimensions, vint8 number)
{
  char * filename = TrainingPath(dataset, dimensions, number);
  vMatrix<float> result = vMatrix<float>::Read(filename);
  if (result.valid() <= 0)
  {
    vPrint("failed to read from %s\n", filename);
  }
  vdelete2(filename);
  return result;
}


vMatrix<float> class_SparseMap::LoadTestEmbedding(const char * dataset,
                                            vint8 dimensions, vint8 number)
{
  char * filename = TestPath(dataset, dimensions, number);
  vMatrix<float> result = vMatrix<float>::Read(filename);
  if (result.valid() <= 0)
  {
    vPrint("failed to read from %s\n", filename);
  }
  vdelete2(filename);
  return result;
}


vMatrix<float> class_SparseMap::EmbedTraining(const char * dataset)
{
  vint8 training_number = BoostMap_data::TrainingNumber(dataset);
  char * distance_path = BoostMap_data::TrainTrainDistancesPath(g_data_directory, dataset);
  class_file * fp = new class_file(distance_path, vFOPEN_READ);
  if (fp->file_pointer == 0)
  {
    fclose(fp);
    vPrint("failed to open %s\n", distance_path);
    vdelete2(distance_path);
    return vMatrix<float>();
  }
  vdelete2(distance_path);

  vint8 dimensions = Dimensions();
  vMatrix<float> embeddingm(training_number, dimensions);
  vArray2(float) embedding = embeddingm.Matrix2();
  function_enter_value(&embeddingm, (float) -1);

  // get the first dimension
  vMatrix<vint8> set1 = reference_sets[0];
  vint8 first_number = set1.Size();
  vMatrix<float> first_distancesm(first_number, training_number);
  vint8 i, j, k;
  for (i = 0; i < first_number; i++)
  {
    vMatrix<float> distances = BoostMap_data::ObjectDistances3(fp, i, training_number);
    if (distances.valid() <= 0)
    {
      exit_error("\nerror: invalid distances\n");
    }
    function_put_row(&distances, &first_distancesm, i);
  }
  vArray2(float) first_distances = first_distancesm.Matrix2();
  for (i = 0; i < training_number; i++)
  {
    float min_distance = first_distances[0][i];
    for (j = 1; j < first_number; j++)
    {
      float distance = first_distances[j][i];
      if (distance < min_distance)
      {
        min_distance = distance;
      }
      embedding[i][0] = min_distance;
    }
  }

  const vint8 sigma = 1;
  // get rest of dimensions
  vPrint("\n");
  for (i = 1; i < dimensions; i++)
  {
    vMatrix<vint8> set = reference_sets[(vector_size) i];
    
    // For each object, find its closest sigma reference objects
    // in this set, using first i features.
    vMatrix<vint8> matches = SigmaMatches(set, i, embeddingm, sigma);

    // object[i][j] = k means that the k-th training object has
    // the i-th element of the reference set as one of its
    // sigma matches in that set.
    vector<vector<vint8> > indices((vector_size) set.Size());
    ComputeIndices(matches, &indices);

    vint8 number = set.Size();
    for (j = 0; j < number; j++)
    {
      vint8 reference = set(j);
      vMatrix<float> distances = BoostMap_data::ObjectDistances3(fp, reference, training_number);
      if (distances.valid() <= 0)
      {
        exit_error("\nerror: invalid distances\n");
      }
      vint8 index_number = indices[(vector_size) j].size();
      for (k = 0; k < index_number; k++)
      {
        vint8 index = indices[(vector_size) j][(vector_size) k];
        float distance = distances(index);
        if ((embedding[index][i] == (float) -1) ||
            (embedding[index][i] > distance))
        {
          embedding[index][i] = distance;
        }
      }
    }
    vPrint("processed dimension %li of %li\r", (long) (i+1), (long) dimensions);
  }
  vPrint("\n");

  fclose(fp);
  return embeddingm;
}


vMatrix<vint8> class_SparseMap::SigmaMatches(vMatrix<vint8> set, vint8 dimensions, 
                                       vMatrix<float> embeddingm, vint8 sigma)
{
  vint8 rows = embeddingm.Rows();
  vint8 reference_number = set.Size();
  vMatrix<float> reference_vectorsm(reference_number, dimensions);
  vArray2(float) embedding = embeddingm.Matrix2();
  vArray2(float) reference_vectors = reference_vectorsm.Matrix2();

  // extract the first i dimensions of the reference objects, store
  // them in reference_vectors
  vint8 i, j;
  for (i = 0; i < reference_number; i++)
  {
    for (j = 0; j < dimensions; j++)
    {
      vint8 row = set(i);
      float value = embedding[row][j];
      reference_vectors[i][j] = value;
    }
  }

  // if sigma is larger than the size of the reference set, sigma 
  // should be adjusted.
  sigma = Min(sigma, reference_number);
  vint8 training_number = embeddingm.Rows();

  vMatrix<vint8> result(training_number, sigma);
  v3dMatrix<float> objectm(1, dimensions, 1);
  vArray(float) object = objectm.Matrix();
  vMatrix<float> weights(1, dimensions);
  function_enter_value(&weights, (float) 1);

  // compute distances to each reference vector
  for (i = 0; i < training_number; i++)
  {
    for (j = 0; j < dimensions; j++)
    {
      object[j] = embedding[i][j];
    }

    vMatrix<float> distances = BoostMap_data::L2Distances(&objectm, reference_vectorsm, weights);
    vMatrix<float> knns = BoostMap_data::FindKnn2(distances, sigma);
    for (j = 0; j < sigma; j++)
    {
      vint8 index1 = round_number(knns(j, 0));
      result(i,j) = index1;
    }
  }

  return result;
}


vint8 class_SparseMap::ComputeIndices(vMatrix<vint8> matchesm, vector<vector<vint8> > * indices)
{
  vint8 rows = matchesm.Rows();
  vint8 sigma = matchesm.Cols();
  vint8 row, col;
  vArray2(vint8) matches = matchesm.Matrix2();

  for (row = 0; row < rows; row++)
  {
    for (col = 0; col < sigma; col++)
    {
      vint8 index = matches[row][col];
      (*indices)[(vector_size) index].push_back(row);
    }
  }

  return 1;
}


vMatrix<float> class_SparseMap::EmbedTest(const char * dataset, 
                                     vMatrix<float> training_embeddingm)
{
  vint8 training_number = BoostMap_data::TrainingNumber(dataset);
  vint8 test_number = BoostMap_data::TestNumber(dataset);
  char * distance_path = BoostMap_data::TestTrainDistancesPath(g_data_directory, dataset);
  class_file * fp = new class_file(distance_path, vFOPEN_READ);
  if (fp->file_pointer == 0)
  {
    fclose(fp);
    vPrint("failed to open %s\n", distance_path);
    vdelete2(distance_path);
    return vMatrix<float>();
  }
  vdelete2(distance_path);

  vint8 dimensions = Dimensions();
  vMatrix<float> embeddingm(test_number, dimensions);
  vArray2(float) embedding = embeddingm.Matrix2();
  function_enter_value(&embeddingm, (float) -1);

  const vint8 sigma = 1;
  vPrint("\n");
  vint8 i, j, k;
  for (i = 0; i < test_number; i++)
  {
    vMatrix<float> distances = BoostMap_data::ObjectDistances3(fp, i, training_number);
    if (distances.valid() <= 0)
    {
      exit_error("\nerror: invalid distances\n");
    }

    vMatrix<float> test_embeddingm(1, dimensions);
    
    // get first dimension
    vMatrix<vint8> set = reference_sets[0];
    float min_distance = distances(set(0));
    for (k = 1; k < set.Size(); k++)
    {
      float distance = distances(set(k));
      if (distance < min_distance)
      {
        min_distance = distance;
      }
    }
    test_embeddingm(0) = min_distance;

    for (j = 1; j < dimensions; j++)
    {
      vMatrix<vint8> set = reference_sets[(vector_size) j];
    
      // For the current test object, find closest sigma reference objects
      // in this set, using first j features.
      vMatrix<vint8> matches = ObjectSigmaMatches(test_embeddingm, set, j, training_embeddingm, sigma);
      vint8 number = matches.Size();

      // find closest object (based on actual distance) among sigma matches
      for (k = 0; k < number; k++)
      {
        vint8 index = matches(k);
        vint8 reference = set(index);
        float distance = distances(reference);
        if ((distance < min_distance) || (k == 0))
        {
          min_distance = distance;
        }
      }
      test_embeddingm(j) = min_distance;
      embeddingm(i, j) = min_distance;
    }
    vPrint("processed object %li of %li\r", (long) (i+1), (long) test_number);
  }
  vPrint("\n");

  fclose(fp);
  return embeddingm;
}


vMatrix<vint8> class_SparseMap::ObjectSigmaMatches(vMatrix<float> test_embeddingm, 
                                             vMatrix<vint8> set, 
                                             vint8 dimensions, vMatrix<float> embeddingm, 
                                             vint8 sigma)
{
  vint8 reference_number = set.Size();
  vMatrix<float> reference_vectorsm(reference_number, dimensions);
  vArray2(float) embedding = embeddingm.Matrix2();
  vArray2(float) reference_vectors = reference_vectorsm.Matrix2();

  // extract the first i dimensions of the reference objects, store
  // them in reference_vectors
  vint8 i, j;
  for (i = 0; i < reference_number; i++)
  {
    for (j = 0; j < dimensions; j++)
    {
      vint8 row = set(i);
      float value = embedding[row][j];
      reference_vectors[i][j] = value;
    }
  }

  // if sigma is larger than the size of the reference set, sigma 
  // should be adjusted.
  sigma = Min(sigma, reference_number);

  vMatrix<vint8> result(1, sigma);
  v3dMatrix<float> objectm(1, dimensions, 1);
  vArray(float) object = objectm.Matrix();
  vMatrix<float> weights(1, dimensions);
  function_enter_value(&weights, (float) 1);

  for (j = 0; j < dimensions; j++)
  {
    object[j] = test_embeddingm(j);
  }

  vMatrix<float> distances = BoostMap_data::L2Distances(&objectm, reference_vectorsm, weights);
  vMatrix<float> knns = BoostMap_data::FindKnn2(distances, sigma);
  for (j = 0; j < sigma; j++)
  {
    vint8 index1 = round_number(knns(j, 0));
    result(j) = index1;
  }

  return result;
}
