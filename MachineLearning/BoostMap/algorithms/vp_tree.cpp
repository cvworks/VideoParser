
#include "vp_tree.h"
#include "learning/boost_kdd.h"
#include "basics/simple_algo_templates.h"
#include "basics/simple_algo.h"
#include "basics/local_data.h"

#include "basics/definitions.h"


static vint8 s_nodes_created = 0;


vint8 vVpTree::Initialize()
{
  left = 0;
  right = 0;
  is_leaf = 1;
  pivot_number = -1;
  left_low_radius = -1;
  left_high_radius = -1;
  right_low_radius = -1;
  right_high_radius = -1;
  constant = (float) 1.0;

  return 1;
}


vint8 vVpTree::CleanUp()
{
  vdelete(left);
  vdelete(right);

  return 1;
}


vVpTree::vVpTree()
{
  Initialize();
}


// construct a vp-tree for the training set of a dataset.
// To access the dataset we will use the BoostMap_data class.
vVpTree::vVpTree(const char * dataset_name)
{
  vint8 number = BoostMap_data::TrainingNumber(dataset_name);
  vMatrix<vint8> object_ids = function_range_matrix(0, number - 1);

  s_nodes_created = 0;
  vPrint("\n");
  vint8 success = BuildSubtree(dataset_name, object_ids);
  vPrint("\n");
  if (success <= 0)
  {
    CleanUp();
    Initialize();
  }
}



vVpTree::~vVpTree()
{
  CleanUp();
}


// this is the function that is called recursively to actually
// build the tree. It builds a subtree for the specified objects
// from the dataset's training set.
vint8 vVpTree::BuildSubtree(const char * dataset_name, vMatrix<vint8> objects)
{
  Initialize();
  
  if (objects.valid() <= 0)
  {
    return 0;
  }

  if (objects.Size() == 1)
  {
    pivot_number = (vint8) objects(0);
    left_low_radius = 0;
    left_high_radius = 0;
    right_low_radius = 0;
    right_high_radius = 0;
    is_leaf = 1;
    return 1;
  }

  is_leaf = 0;
  pivot_number = FindPivot(dataset_name, objects);
  vMatrix<float> all_distances = BoostMap_data::TrainTrainDistance(g_data_directory,
	  dataset_name, pivot_number);

  left_high_radius = PickRadius(dataset_name, objects, pivot_number, all_distances);
  vMatrix<vint8> left_objects = PickLeftObjects(dataset_name, objects, 
                                               pivot_number, left_high_radius, all_distances);
  vMatrix<vint8> right_objects = PickRightObjects(dataset_name, objects, 
                                                 pivot_number, left_high_radius, all_distances);

  left_low_radius = FindLowRadius(all_distances, left_objects);
  right_low_radius = FindLowRadius(all_distances, right_objects);
  right_high_radius = FindHighRadius(all_distances, right_objects);

  vint8 success = 1;

  s_nodes_created = s_nodes_created + 1;
  vPrint("created %li nodes\r", (long) s_nodes_created);
  if (left_objects.valid() > 0)
  {
    left = new vVpTree();
    success = left->BuildSubtree(dataset_name, left_objects);
    if (success <= 0)
    {
      return success;
    }
  }

  if (right_objects.valid() > 0)
  {
    right = new vVpTree();
    success = right->BuildSubtree(dataset_name, right_objects);
  }
  return success;
}



vint8 vVpTree::Save(const char * filename)
{
  char * pathname = MakePathname(filename);
  FILE * fp = fopen(pathname, vFOPEN_WRITE);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", pathname);
    vdelete2(pathname);
    return 0;
  }

  vdelete2(pathname);
  vint8 success = Save(fp);
  fclose(fp);
  return success;
}


vint8 vVpTree::Save(FILE * fp)
{
  vint8 success = 1;
  vint8 items = 0;
  items = store_old_longs(fp, &pivot_number, 1);
  if (items != 1)
  {
    vPrint("failed to write pivot id\n");
    success = 0;
  }
  items = store_old_longs(fp, &is_leaf, 1);
  if (items != 1)
  {
    vPrint("failed to write is_leaf\n");
    success = 0;
  }

  items = fwrite(&left_low_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to write left_low_radius\n");
    success = 0;
  }
  items = fwrite(&left_high_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to write left_high_radius\n");
    success = 0;
  } 
  
  items = fwrite(&right_low_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to write right_low_radius\n");
    success = 0;
  }
  items = fwrite(&right_high_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to write right_high_radius\n");
    success = 0;
  } 
  
  vint8 success2;
  
  if (left == 0)
  {
    items = store_integer(fp, (integer) 0);
    if (items != 1)
    {
      vPrint("failed to write boolean for left tree\n");
      success = 0;
    } 
  }
  else 
  {
    items = store_integer(fp, (integer) 1);
    if (items != 1)
    {
      vPrint("failed to write boolean for left tree\n");
      success = 0;
    } 

    success2 = left->Save(fp);
    if (success2 <= 0)
    {
      success = success2;
    }
  }
    
  if (right == 0)
  {
    items = store_integer(fp, (integer) 0);
    if (items != 1)
    {
      vPrint("failed to write boolean for right tree\n");
      success = 0;
    } 
  }
  else 
  {
    items = store_integer(fp, (integer) 1);
    if (items != 1)
    {
      vPrint("failed to write boolean for right tree\n");
      success = 0;
    } 

    success2 = right->Save(fp);
    if (success2 <= 0)
    {
      success = success2;
    }
  }

  return success;
}


vint8 vVpTree::Load(const char * filename)
{
  char * pathname = MakePathname(filename);
  FILE * fp = fopen(pathname, vFOPEN_READ);
  if (fp == 0)
  {
    vPrint("failed to open %s\n", pathname);
    vdelete2(pathname);
    return 0;
  }

  vdelete2(pathname);
  vint8 success = Load(fp);
  fclose(fp);
  return success;
}


vint8 vVpTree::Load(FILE * fp)
{
  vint8 success = 1;
  vint8 items = 0;
  items = read_old_longs(fp, &pivot_number, 1);
  if (items != 1)
  {
    vPrint("failed to read pivot id\n");
    success = 0;
  }
  items = read_old_longs(fp, &is_leaf, 1);
  if (items != 1)
  {
    vPrint("failed to read is_leaf\n");
    success = 0;
  }

  items = fread(&left_low_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to read left_low_radius\n");
    success = 0;
  }
  items = fread(&left_high_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to read left_high_radius\n");
    success = 0;
  } 

  items = fread(&right_low_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to read right_low_radius\n");
    success = 0;
  }
  items = fread(&right_high_radius, sizeof(float), 1, fp);
  if (items != 1)
  {
    vPrint("failed to read right_high_radius\n");
    success = 0;
  } 

  vint8 success2;
  integer child_flag = 0;

  items =  read_integer(fp, &child_flag);
  if (items != 1)
  {
    vPrint("failed to read left child boolean\n");
    success = 0;
  } 

  if (child_flag != 0)
  {
    left = new vVpTree();
    success2 = left->Load(fp);
    if (success2 <= 0)
    {
      success = success2;
    }
  }

  items =  read_integer(fp, &child_flag);
  if (items != 1)
  {
    vPrint("failed to read right child boolean\n");
    success = 0;
  } 

  if (child_flag != 0)
  {
    right = new vVpTree();
    success2 = right->Load(fp);
    if (success2 <= 0)
    {
      success = success2;
    }
  }

  return success;
}


// prints the first levels of the tree.
vint8 vVpTree::Print(vint8 levels)
{
  if (levels <= 0)
  {
    return 1;
  }

  vint8 spaces;
  if (levels <= 20)
  {
    spaces = 2 * (20 - levels);
  }
  else
  {
    spaces = 0;
  }

  vint8 i;
  for (i = 0; i < spaces; i++)
  {
    vPrint(" ");
  }

  vint8 count = CountObjects();
  vPrint("L%-2li: pivot = %6li, objects = %li\n", (long) levels, (long) pivot_number, (long) count);
  for (i = 0; i < spaces; i++)
  {
    vPrint(" ");
  }

  vPrint("     is_leaf = %li\n", (long) is_leaf);
  for (i = 0; i < spaces; i++)
  {
    vPrint(" ");
  }

  vPrint("     [%f, %f], [%f, %f]\n", 
         left_low_radius, left_high_radius, right_low_radius, right_high_radius);

  vPrint("\n");
  if (left != 0)
  {
    left->Print(levels - 1);
  }
  if (right != 0)
  {
    right->Print(levels - 1);
  }

  return 1;
}


vint8 vVpTree::FindPivot(const char * dataset_name, vMatrix<vint8> objects)
{
  vint8 size = objects.Size();
  vint8 pick = function_random_vint8(0, size-1);
  vint8 result = objects(pick);
  return result;
}


float vVpTree::PickRadius(const char * dataset_name, vMatrix<vint8> objects, 
                          vint8 pivot_number, vMatrix<float> all_distances)
{
  vint8 size = objects.Size();
  vMatrix<float> distances(1, size-1);

  // copy the relevant distances.
  vint8 i;
  vint8 counter = 0;
  for (i = 0; i < size; i++)
  {
    vint8 index = objects(i);
    if (index == pivot_number)
    {
      continue;
    }
    float distance = all_distances(index);
    distances(counter) = distance;
    counter++;
  }

  vint8 median = (size - 1) / 2;
  vint8 junk = 0;
  float distance;
  if (median == 0)
  {
    distance = 0;
  }
  else
  {
    distance = kth_smallest_cb(median, &distances, &junk);
  }

  return distance;
}


// assuming that objects describes all objects included in the high
// radius around the pivot, and distances are distances of all
// training objects to the pivot, we find the distance of the closest
// object in objects to the pivot.
float vVpTree::FindLowRadius(vMatrix<float> distances, vMatrix<vint8> objects)
{
  if (distances.valid() <= 0)
  {
    exit_error("error: in FindLowRadius, invalid distances\n");
  }

  if (objects.valid() <= 0)
  {
    return (float) 0;
  }

  float min_distance = function_image_maximum(&distances);
  vint8 number = objects.Size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index = objects(i);
    float distance = distances(index);
    if (distance < min_distance)
    {
      min_distance = distance;
    }
  }

  return min_distance;
}


float vVpTree::FindHighRadius(vMatrix<float> distances, vMatrix<vint8> objects)
{
  if (distances.valid() <= 0)
  {
    exit_error("error: in FindHighRadius, invalid distances\n");
  }

  if (objects.valid() <= 0)
  {
    return (float) 0;
  }

  float max_distance = function_image_minimum(&distances);
  vint8 number = objects.Size();
  vint8 i;
  for (i = 0; i < number; i++)
  {
    vint8 index = objects(i);
    float distance = distances(index);
    if (distance > max_distance)
    {
      max_distance = distance;
    }
  }

  return max_distance;
}


vMatrix<vint8> vVpTree::PickLeftObjects(const char * dataset_name, vMatrix<vint8> objects, 
                                       vint8 pivot_number, float high_radius, 
                                       vMatrix<float> all_distances)
{
  vector<vint8> indices;
  vint8 size = objects.Size();

  vint8 i;
  for (i = 0; i < size; i++)
  {
    vint8 index = objects(i);
    if (index == pivot_number)
    {
      continue;
    }
    float distance = all_distances(index);
    if (distance <= high_radius)
    {
      indices.push_back(index);
    }
  }

  if (indices.size() == 0)
  {
    return vMatrix<vint8>();
  }
  vMatrix<vint8> result = matrix_from_vector(&indices);
  return result;
}


vMatrix<vint8> vVpTree::PickRightObjects(const char * dataset_name, vMatrix<vint8> objects, 
                                        vint8 pivot_number, float high_radius, 
                                        vMatrix<float> all_distances)
{
  vector<vint8> indices;
  vint8 size = objects.Size();

  vint8 i;
  for (i = 0; i < size; i++)
  {
    vint8 index = objects(i);
    if (index == pivot_number)
    {
      continue;
    }
    float distance = all_distances(index);
    if (distance > high_radius)
    {
      indices.push_back(index);
    }
  }

  if (indices.size() == 0)
  {
    return vMatrix<vint8>();
  }
  vMatrix<vint8> result = matrix_from_vector(&indices);
  return result;
}


// counts the number of objects stored under this tree.
vint8 vVpTree::CountObjects()
{
  if (is_leaf != 0)
  {
    return 1;
  }

  vint8 left_count = 0, right_count = 0;
  if (left != 0)
  {
    left_count = left->CountObjects();
  }
  if (right != 0)
  {
    right_count = right->CountObjects();
  }
  vint8 count = 1 + left_count + right_count;

  return count;
}


char * vVpTree::DefaultDirectory()
{
  char * directory = vJoinPaths4(nessied, "data", "experiments", "vp_trees");  
  return directory;
}


char * vVpTree::MakePathname(const char * filename)
{
  char * directory = DefaultDirectory();
  char * pathname = vJoinPaths(directory, filename);
  vdelete2(directory);
  return pathname;
}


vint8 vVpTree::AddChildren(vector<vint8> * matches)
{
  matches->push_back(pivot_number);
  if (left != 0)
  {
    left->AddChildren(matches);
  }
  if (right != 0)
  {
    right->AddChildren(matches);
  }
  return 1;
}


// given that all objects in the subtree have distances in 
// [min_radius, max_radius] from pivot, and that query has
// to_pivot distance from pivot, find the min/max possible distance
// from query to all objects in subtree.
float vVpTree::MinDistance(float to_pivot, float min_radius, float max_radius)
{
  if ((min_radius <= constant * to_pivot) && (to_pivot <= constant * max_radius))
  {
    return (float) 0;
  }
  
  float min1 = vAbs(min_radius/constant - to_pivot);
  float min2 = vAbs(to_pivot/constant - max_radius);
  float result = Min(min1, min2);
  return result;
}


float vVpTree::MaxDistance(float to_pivot, float min_radius, float max_radius)
{
  float max1 = to_pivot + min_radius;
  float max2 = to_pivot + max_radius;
  float result = Max(max1, max2);
  return result;
}


// assuming that distances are the distances from q to all
// database objects, and the matches were found by searching
// for this radius in the tree, we count how many of those
// matches were correct.
vint8 vVpTree::CorrectFound(vMatrix<float> distances, float radius,
                           vMatrix<vint8> matches)
{
  vint8 size = matches.Size();
  vint8 result = 0;

  vint8 i;
  for (i = 0; i < size; i++)
  {
    vint8 index = matches(i);
    float distance = distances(index);
    if (distance < radius)
    {
      result = result + 1;
    }
  }

  return result;
}


// assuming that distances are the distances from q to all
// database objects, and the matches were found by searching
// for this radius in the tree, we count how many correct matches
// were missed.
vint8 vVpTree::MissedMatches(vMatrix<float> distances, float radius,
                            vMatrix<vint8> matches)
{
  vint8 size = distances.Size();
  vint8 correct_found = CorrectFound(distances, radius, matches);
  vint8 result = 0;

  vint8 i;
  for (i = 0; i < size; i++)
  {
    float distance = distances(i);
    if (distance < radius)
    {
      result = result + 1;
    }
  }

  result = result - correct_found;
  return result;
}


// Finds all neighbors within radius using the vp-tree. Since our goal is 
// just to measure the efficiency of the vp-tree, we pass in the
// distances to all training objects. In principle, computing those distances
// is what vp-trees try to avoid, like BoostMap. Howerver, for evaluation 
// purposes, we have already computed those distances, so we just need to
// see how many of those the tree would need to evaluate, and how often it
// would find the true nearest neighbor. Returns the number of distance
// evaluations.
vint8 vVpTree::RangeSearch(vMatrix<float> distances, float radius, 
                     vector<vint8> * matches)
{
  vint8 result = 1;
  float distance = distances(pivot_number);
  if (distance < radius)
  {
    matches->push_back(pivot_number);
  }

  if (left != 0)
  {
    float min_distance = MinDistance(distance, left_low_radius, left_high_radius);
    float max_distance = MaxDistance(distance, left_low_radius, left_high_radius);

    if (max_distance < radius)
    {
      // add all children without even measuring distances.
      left->AddChildren(matches);
    }
    else if (min_distance < radius)
    {
      vint8 left_result = left->RangeSearch(distances, radius, matches);
      result = result + left_result;
    }
  }

  if (right != 0)
  {
    float min_distance = MinDistance(distance, right_low_radius, right_high_radius);
    float max_distance = MaxDistance(distance, right_low_radius, right_high_radius);

    if (max_distance < radius)
    {
      // add all children without even measuring distances.
      right->AddChildren(matches);
    }
    else if (min_distance < radius)
    {
      vint8 right_result = right->RangeSearch(distances, radius, matches);
      result = result + right_result;
    }
  }
  
  return result;
}


// searches for all objects within radius of specified test object,
// and prints out some stats about the results.
vint8 vVpTree::TestRangeSearch(const char * dataset, vint8 index, float radius)
{
  vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, index);
  if (distances.valid() <= 0)
  {
    vPrint("failed to load distances\n");
    return 0;
  }

  vector<vint8> matches_v;
  vint8 distance_number = RangeSearch(distances, radius, &matches_v);
  vMatrix<vint8> matches = matrix_from_vector(&matches_v);

  vint8 correct_found = CorrectFound(distances, radius, matches);
  vint8 incorrect_found = matches.Size() - correct_found;
  vint8 missed_matches = MissedMatches(distances, radius, matches);

  vPrint("\n");
  vPrint("%li distances computed, %li matches found\n", 
         (long) distance_number, (long) matches.Size());
  vPrint("correct_found = %li\n", (long) correct_found);
  vPrint("incorrect_found = %li\n", (long) incorrect_found);
  vPrint("missed_matches = %li\n", (long) missed_matches);
  vPrint("\n");

  return 1;
}


// Finds the nearest neighbor of the query. Stores the index of the 
// nearest neighbor at nnp. Returns number of computed distances.
vint8 vVpTree::NnSearch(vMatrix<float> distances, vint8 * nnp)
{
  float distance = distances(pivot_number);
  *nnp = pivot_number;
  vint8 result = NnSearchAux(distances, nnp, distance);
  return result;
}


// NnSearchAux does the work, by keeping track of the smallest distance
// found so far in min_found. Returns number of computed distances.
vint8 vVpTree::NnSearchAux(vMatrix<float> distances, vint8 * nnp, float min_found)
{
  vint8 result = 1;
  float distance = distances(pivot_number);
  if (distance < min_found)
  {
    min_found = distance;
    *nnp = pivot_number;
  }

  if (left != 0)
  {
    float min_distance = MinDistance(distance, left_low_radius, left_high_radius);
    if (min_distance < min_found)
    {
      vint8 left_result = left->NnSearchAux(distances, nnp, min_found);
      result = result + left_result;
    }
  }

  if (right != 0)
  {
    // update min_found in case we found a closer match in the left tree.
    min_found = distances(*nnp);

    float min_distance = MinDistance(distance, right_low_radius, right_high_radius);
    if (min_distance < min_found)
    {
      vint8 right_result = right->NnSearchAux(distances, nnp, min_found);
      result = result + right_result;
    }
  }
  
  return result;
}


// Finds the k nearest neighbors of the query, 
// where k is specified by number. Stores the indices of the 
// nearest neighbors at neighbors. Returns number of computed distances.
vint8 vVpTree::KnnSearch(vMatrix<float> distances, vector<vint8> * neighbors, vint8 number)
{
  neighbors->erase(neighbors->begin(), neighbors->end());
  vMatrix<vint8> neighbors_matrix(1, number);
  function_enter_value(&neighbors_matrix, (vint8) -1);
  vMatrix<float> min_distances(number, 1);
  float max_distance = function_image_maximum(&distances) + 1;
  function_enter_value(&min_distances, (float) max_distance);

  float distance = distances(pivot_number);
  min_distances(number -1) = distance;
  neighbors_matrix(number - 1) = pivot_number;
  
  vint8 result = KnnSearchAux(distances, neighbors_matrix, min_distances);
  vector_from_matrix(& neighbors_matrix, neighbors);
  return result;
}


// KnnSearchAux does the work, by keeping track of the smallest distances
// found so far in min_found. Returns number of computed distances.
vint8 vVpTree::KnnSearchAux(vMatrix<float> distances, vMatrix<vint8> neighbors, 
                           vMatrix<float> min_found)
{
  vint8 result = 1;
  float distance = distances(pivot_number);
  if (distance < min_found(0))
  {
    InsertDistance(distance, pivot_number, min_found, neighbors);
  }

  if (left != 0)
  {
    float min_distance = MinDistance(distance, left_low_radius, left_high_radius);
    if (min_distance < min_found(0))
    {
      vint8 left_result = left->KnnSearchAux(distances, neighbors, min_found);
      result = result + left_result;
    }
  }

  if (right != 0)
  {
    float min_distance = MinDistance(distance, right_low_radius, right_high_radius);
    if (min_distance < min_found(0))
    {
      vint8 right_result = right->KnnSearchAux(distances, neighbors, min_found);
      result = result + right_result;
    }
  }
  
  return result;
}


vint8 vVpTree::TestNnSearch(const char * dataset, vint8 index)
{
  vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, index);
  if (distances.valid() <= 0)
  {
    vPrint("failed to load distances\n");
    return 0;
  }

  vint8 found_nn_index = -1;
  vint8 distance_number = NnSearch(distances, &found_nn_index);
  float found_distance = distances(found_nn_index);  
  
  vint8 nn_index = -1, junk = 0;
  float nn_distance = function_image_minimum3(&distances, &junk, &nn_index);

  vPrint("\n");
  vPrint("%li distances computed, nn found = %li, distance = %f\n", 
         (long) distance_number, (long) found_nn_index, found_distance);
  if (nn_distance == found_distance)
  {
    vPrint("The result is correct.\n");
  }
  else
  {
    vPrint("true nn index = %li, true nn distance = %f\n",
           (long) nn_index, nn_distance);
  }
  vPrint("\n");

  return 1;
}


vint8 vVpTree::TestKnnSearch(const char * dataset, vint8 index, vint8 number)
{
  vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, index);
  if (distances.valid() <= 0)
  {
    vPrint("failed to load distances\n");
    return 0;
  }

  vector<vint8> neighbors;
  vint8 distance_number = KnnSearch(distances, &neighbors, number);
  float found_distance = distances(neighbors[0]);  
  
  vint8 nn_index = -1, junk = 0;
  float nn_distance = kth_smallest_cb(number, &distances, &nn_index);

  vPrint("\n");
  vPrint("%li distances computed, nn found = %li, distance = %f\n", 
         (long) distance_number, (long) neighbors[0], found_distance);
  if (nn_distance == found_distance)
  {
    vPrint("The result is correct.\n");
  }
  else
  {
    vPrint("true nn index = %li, true nn distance = %f\n",
           (long) nn_index, nn_distance);
  }
  vPrint("\n");

  return 1;
}


// Does nearest neighbor search for each test object in the dataset. 
// Returns the number of false results. Stores the average number
// of distances in average_distancesp.
vint8 vVpTree::NnSearchStats(const char * dataset, 
                            float * average_distancesp)
{
  vint8 test_number = BoostMap_data::TestNumber(dataset);
  if (test_number <= 0)
  {
    vPrint("could not find dataset %s\n", dataset);
    return 0;
  }
  
  vint8 result = 0;
  double distance_sum = 0;

  vint8 i;
  vPrint("\n");
  for (i = 0; i < test_number; i++)
  {
    vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, i);
    if (distances.valid() <= 0)
    {
      vPrint("failed to load distances\n");
      return 0;
    }

    vint8 found_nn_index = -1;
    vint8 distance_number = NnSearch(distances, &found_nn_index);
    float found_distance = distances(found_nn_index);  
  
    vint8 nn_index = -1, junk = 0;
    float nn_distance = function_image_minimum3(&distances, &junk, &nn_index);
    distance_sum = distance_sum + distance_number;

    if (nn_distance != found_distance)
    {
      result = result + 1;
    }

    double average_distance = distance_sum / (double) (i + 1);
    vPrint("%10li objects, %10li mistakes, %10li average distances\r",
           (long) (i+1), (long) result, (long) round_number(average_distance));
  }
  vPrint("\n");

  *average_distancesp = (float) (distance_sum / (double) test_number);
  return result;
}


// this function tells us how good a vp-tree is for 
// k-nearest neighbor retrieval. The function stores into
// average_distancesp the average number of distances needed to
// retrieve the estimated "number" nearest neighbors of each object
// in the test set. The result is a number that tells us the number 
// of test objects for which the set of "number" nearest neighbors
// that was retrieved was not identical with that true "number"
// nearest neighbors.
vint8 vVpTree::KnnSearchStats(const char * dataset, 
                            float * average_distancesp,
                            vint8 number)
{
  // get the number of test objects
  vint8 test_number = BoostMap_data::TestNumber(dataset);
  if (test_number <= 0)
  {
    vPrint("could not find dataset %s\n", dataset);
    return 0;
  }
  
  // number of test subjects for which retrieval is not perfectly correct
  vint8 result = 0;

  // sum of the distances needed to compute the nearest neighbors for the 
  // entire data set
  double distance_sum = 0;

  vint8 i;
  vPrint("\n");
  // go through all the test objects
  for (i = 0; i < test_number; i++)
  {
    // get the distances between the i-th object and all database objects
    vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, i);
    if (distances.valid() <= 0)
    {
      vPrint("failed to load distances\n");
      return 0;
    }

    vector <vint8 > neighbors;
    // find the "number" nearest neighbors of the current test object.
    vint8 distance_number = KnnSearch(distances, &neighbors, number);

    // found_distance is the distance between the test object and its
    // estimated "number"-th nearest neighbor according to the 
    // retrieved results
    float found_distance = distances(neighbors[0]);  
    
    vint8 nn_index = -1, junk = 0;
    float nn_distance = kth_smallest_cb(number, &distances, &nn_index);
    distance_sum = distance_sum + distance_number;

    if (nn_distance != found_distance)
    {
      result = result + 1;
    }

    double average_distance = distance_sum / (double) (i + 1);
    vPrint("%10li objects, %10li mistakes, %10li average distances\r",
           (long) (i+1), (long) result, (long) round_number(average_distance));
  }
  vPrint("\n");

  *average_distancesp = (float) (distance_sum / (double) test_number);
  return result;
}


vint8 vVpTree::InsertDistance(float distance, vint8 neighbor, 
                             vMatrix<float> distances, vMatrix<vint8> neighbors)
{
  vint8 number = distances.Size();
  vint8 index;
  vint8 position = number -1;
  for (index = 1; index <number; index ++)
  {
    float current = distances(index);
    if (distance > current)
    {
      position = index - 1;
      break;
    }
  }

  for (index = 0; index < position; index++)
  {
    distances(index) = distances(index + 1);
    neighbors(index) = neighbors(index + 1);
  }

  distances(position) = distance;
  neighbors(position) = neighbor;

  return 1;
}


// the result is a matrix of number+ 1 entries. the entry indexed by zero
// doesn't hold any useful information.  Every other entry holds the
// nearest neighbor classification error corresponding to the index of that entry.
vMatrix<float> vVpTree::KnnClassificationStats(const char * dataset, 
                                      float * average_distancesp, vint8 number)
{
  vint8 test_number = BoostMap_data::TestNumber(dataset);
  if (test_number <= 0)
  {
    vPrint("could not find dataset %s\n", dataset);
    return vMatrix<float >();
  }
  
  vMatrix<float> result(1, number + 1);
  function_enter_value(& result, (float) 0);
  double distance_sum = 0;

  vint8 i;
  vMatrix<float> test_labels = BoostMap_data::LoadTestLabels(g_data_directory,
	  dataset);

  vMatrix<float> training_labels = BoostMap_data::LoadTrainingLabels(g_data_directory,
	  dataset);

  vPrint("\n");
  for (i = 0; i < test_number; i++)
  {
    vMatrix<float> distances = BoostMap_data::TestTrainDistance(dataset, i);
    if (distances.valid() <= 0)
    {
      vPrint("failed to load distances\n");
      return float_matrix();
    }

    vector <vint8 > neighbors;
    vint8 distance_number = KnnSearch(distances, &neighbors, number);
    float found_distance = distances(neighbors[0]);  
    distance_sum = distance_sum + distance_number;
    double average_distance = distance_sum / (double) (i + 1);

    vint8 label = round_number(test_labels(i));
    vMatrix<float> selected_labels (1, number);
    vMatrix<float> selected_distances(1, number);

    vint8 index;
    for (index = 0; index <number; index++)
    {
      selected_labels(index) = training_labels (neighbors[(vector_size) index]);
      selected_distances(index) = distances(neighbors[(vector_size) index]);
    }

    // get, for each k (from 1 to max_k) the class label that 
    // is the result of k-nn classification of the object.
    vMatrix<float> result_labels = BoostMap_data::KnnLabel4(selected_distances, selected_labels, 
                                                         label, number);
    if (result_labels.valid() <= 0)
    {
      exit_error("Error: invalid labels in KnnError2\n");
    }

    // convert labels into 0,1 values (0 for the correct label,
    // 1 for all other labels).
    vint8 j;
    for (j = 0; j < number+1; j++)
    {
      if (result_labels(j) == label)
      {
        result_labels(j) = (float) 0;
      }
      else
      {
        result_labels(j) = (float) 1;
      }
    }
    
    // add the results of this objects into result.
    result = result + result_labels;
    vPrint("processed object %li of %li, 1-nn error = %f\r", 
           (vint8) (i+1), (vint8) test_number, result(1) / (float) (i+1));
    float current_accuracy = result(1) / (float) (i+1);

    vPrint("%10li objects, %10li average distances, 1-nn error = %f\r",
           (vint8) (i+1), (vint8) round_number(average_distance), current_accuracy);
  }
  vPrint("\n");

  *average_distancesp = (float) (distance_sum / (double) test_number);
  result = result / (float) test_number;
  return result;
}


vint8 vVpTree::set_constant(float new_value)
{
  constant = new_value;
  if (left != 0)
  {
    left->set_constant(constant);
  }
  if (right != 0)
  {
    right->set_constant(constant);
  }

  return 1;
}
