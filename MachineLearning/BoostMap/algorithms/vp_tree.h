#ifndef VASSILIS_VP_TREE_H
#define VASSILIS_VP_TREE_H

#include "basics/algebra.h"

class vVpTree
{
protected:
  // left tree stores objects whose distance is <= high_radius
  // from the pivot point.
  vVpTree * left;

  
  // right tree stores objects whose distance is > high_radius
  // from the pivot point.
  vVpTree * right;

  vint8 pivot_number;
  float left_low_radius;
  float left_high_radius;
  float right_low_radius;
  float right_high_radius;
  
  // 1 if it is a leaf node, 0 otherwise.
  vint8 is_leaf;

  float constant; // (greater than 1 for almost metrics).

  vint8 Initialize();
  vint8 CleanUp();

public:
  vVpTree();

  // construct a vp-tree for the training set of a dataset.
  // To access the dataset we will use the BoostMap_dataset class.
  vVpTree(const char * dataset_name);
  ~vVpTree();

  // this is the function that is called recursively to actually
  // build the tree. It builds a subtree for the specified objects
  // from the dataset's training set.
  vint8 BuildSubtree(const char * dataset_name, vMatrix<vint8> objects);

  vint8 Save(const char * filename);
  vint8 Save(FILE * fp);
  vint8 Load(const char * filename);
  vint8 Load(FILE * fp);
  
  vint8 AddChildren(vector<vint8> * matches);

  // prints the first levels of the tree.
  vint8 Print(vint8 levels);

  static vint8 FindPivot(const char * dataset_name, vMatrix<vint8> objects);
  static float PickRadius(const char * dataset_name, vMatrix<vint8> objects, 
                          vint8 pivot_number, vMatrix<float> all_distances);

  // assuming that objects describes all objects sent in one of the two
  // subtrees, and distances are distances of all
  // training objects to the pivot, we find the distance of the closest
  // object in objects to the pivot.
  static float FindLowRadius(vMatrix<float> distances, vMatrix<vint8> objects);
  static float FindHighRadius(vMatrix<float> distances, vMatrix<vint8> objects);

  static vMatrix<vint8> PickLeftObjects(const char * dataset_name, vMatrix<vint8> objects, 
                                       vint8 pivot_number, float high_radius,
                                       vMatrix<float> all_distances);
  static vMatrix<vint8> PickRightObjects(const char * dataset_name, vMatrix<vint8> objects, 
                                        vint8 pivot_number, float high_radius, 
                                        vMatrix<float> all_distances);

  // counts the number of objects stored under this tree.
  vint8 CountObjects();

  char * DefaultDirectory();

  char * MakePathname(const char * filename);

  // given that all objects in the subtree have distances in 
  // [min_radius, max_radius] from pivot, and that query has
  // to_pivot distance from pivot, find the min/max possible distance
  // from query to all objects in subtree.
  float MinDistance(float to_pivot, float min_radius, float max_radius);
  float MaxDistance(float to_pivot, float min_radius, float max_radius);

  // assuming that distances are the distances from q to all
  // database objects, and the matches were found by searching
  // for this radius in the tree, we count how many of those
  // matches were correct.
  static vint8 CorrectFound(vMatrix<float> distances, float radius,
                           vMatrix<vint8> matches);

  // assuming that distances are the distances from q to all
  // database objects, and the matches were found by searching
  // for this radius in the tree, we count how many correct matches
  // were missed.
  static vint8 MissedMatches(vMatrix<float> distances, float radius,
                            vMatrix<vint8> matches);

  // Finds all neighbors within radius using the vp-tree. Since our goal is 
  // just to measure the efficiency of the vp-tree, we pass in the
  // distances to all training objects. In principle, computing those distances
  // is what vp-trees try to avoid, like BoostMap. Howerver, for evaluation 
  // purposes, we have already computed those distances, so we just need to
  // see how many of those the tree would need to evaluate, and how often it
  // would find the true nearest neighbor. Returns the number of distance
  // evaluations.
  vint8 RangeSearch(vMatrix<float> distances, float radius, 
                   vector<vint8> * matches);

  // searches for all objects within radius of specified test object,
  // and prints out some stats about the results.
  vint8 TestRangeSearch(const char * dataset, vint8 index, float radius);

// Finds the nearest neighbor of the query. Stores the index of the 
// nearest neighbor at nnp. Returns number of computed distances.
  vint8 NnSearch(vMatrix<float> distances, vint8 * nnp);

  // NnSearchAux does the work, by keeping track of the smallest distance
  // found so far in min_found. Returns number of computed distances.
  vint8 NnSearchAux(vMatrix<float> distances, vint8 * nnp, float min_found);

  // Finds the k nearest neighbors of the query, 
  // where k is specified by number. Stores the indices of the 
  // nearest neighbors at neighbors. Returns number of computed distances.
  vint8 KnnSearch(vMatrix<float> distances, vector<vint8> * neighbors, vint8 number);

  // KnnSearchAux does the work, by keeping track of the smallest distances
  // found so far in min_found. Returns number of computed distances.
  vint8 KnnSearchAux(vMatrix<float> distances, vMatrix<vint8> neighbors, 
                    vMatrix<float> min_found);

  static vint8 InsertDistance(float distance, vint8 neighbor, 
                      vMatrix<float> distances, vMatrix<vint8> neighbors);

  vint8 TestNnSearch(const char * dataset, vint8 index);

  vint8 TestKnnSearch(const char * dataset, vint8 index, vint8 number);

  // Does nearest neighbor search for each test object in the dataset. 
  // Returns the number of false results. Stores the average number
  // of distances in average_distancesp.
  vint8 NnSearchStats(const char * dataset, float * average_distancesp);
  vint8 KnnSearchStats(const char * dataset, float * average_distancesp, vint8 number);
  
  // the result is a matrix of number+ 1 entries. the entry indexed by zero
  // doesn't hold any useful information.  Every other entry holds the
  // nearest neighbor classification error corresponding to the index of that entry.
  vMatrix<float> KnnClassificationStats(const char * dataset, 
                                        float * average_distancesp, vint8 number);

  vint8 set_constant(float new_value);
};







#endif // VASSILIS_VP_TREE_H
