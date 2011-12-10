/*
 * CompetitionGraph.C
 *
 * Graph that models competing merges in a graph
 *
 * Sven Wachsmuth, 20.11.2003
 */
#include "CompeteGraph.h"
#include "RAGraph.h"

using namespace std;
using namespace leda;

CompeteGraph& CompeteGraph::update()
{
  //sort_nodes(); // DIEGO Feb 21, 2006
  return (*this);
}

CompeteGraph& CompeteGraph::reduce()
{
  update();
  leda_node r,s;
  for (r=last_node(); r != nil; r = s) {
     s = pred_node(r);
     if (outdeg(r) > 0) {
	 //cerr << "CompeteGraph::reduce: processing node "
	 //     << (*((RAGraph *) graph_of(getNode(r))))[getNode(r)].getLabel()
	 //     << " (rank " << (*this)[r].rank
	 //     << ", score " << (*this)[r].score 
	 //     << ") with outdeg " << outdeg(r) << endl;
	 del_node(r);
     }
  }
  return (*this);
}

      
