/*
 * MergeIt.h
 *
 * Iterator for RAGraph-merging
 *
 * Sven Wachsmuth, 11.11.2003
 */
#ifndef RAG_RAG_MERGE_IT_H
#define RAG_RAG_MERGE_IT_H

#include "MergeIt.h"
#include "MergeJudge.h"
#include "RAGraph.h"

class RAG_MergeIt : public MergeIt {

 protected:
  RAGraph *pRAG;

  // List of active merge nodes, that are seeds for new merges.
  // This list is initialized by the single nodes of the graph
  leda_list<MergeIt::NodeList *> active_parents;

  // List of new merges that fulfill the filter criterion defined by
  // the class derived from MergeJudge
  leda_list<MergeIt::NodeList *> active_merges;

  // Clears all lists and initializes active_parents
  void _init(RAGraph &RAG);

  // A new merge produced by MergeIt is linked to one of the active parents.
  // If no subset of merge is found, false is returned, otherwise true.
  bool link_merge(MergeIt::NodeList& merge);
  
  // short cuts for active_parents and active_nodes
  leda_list<MergeIt::NodeList *>& AP() { return active_parents; }
  leda_list<MergeIt::NodeList *>& AM() { return active_merges; }

 public:
  
  RAG_MergeIt(RAGraph &RAG) 
    : MergeIt(RAG.hide_merges()) { RAG.restore_merges(); _init(RAG); }
  RAG_MergeIt(RAGraph &RAG, leda_list<leda_node>& rs) 
    : MergeIt(RAG.hide_merges(),rs) { RAG.restore_merges(); _init(RAG); }
  RAG_MergeIt(RAGraph &RAG, leda_list<leda_node>& rs, leda_list<leda_node>& ns) 
    : MergeIt(RAG,rs,ns) { _init(RAG); }


  void clear() { 
    AP().clear();
    AM().clear();
    // ** should pRAG or MergeIt-object be cleared ????
  }

  // computes all active_merges that include one active_parent as a sub-graph
  // clears active_parents afterwards
  void compute_merges(int depth=0);

  // returns the next member of active_merges
  leda::list_item next_merge(leda::list_item it) {
      return (it == nil) ? AM().first() : AM().succ(it); }

  // returns the <it>-member of active_merges
  MergeIt::NodeList * operator[](leda::list_item it) {
      return (it == nil) ? NULL : AM()[it]; }

  // sets new merge in RAGraph
  leda_node set_merge(MergeIt::NodeList *p_merge);
  // adds new merge to active_parents
  void      add_merge(MergeIt::NodeList *p_merge) { AP().append(p_merge); }

  leda_node set_merge(leda_node r, leda_node s);

  // if active_parents is an empty list after compute_merges is applied
  // the graph merging is converged
  bool     converged() { return AP().empty(); }

  // returns number of new merges after iterate is applied
  int      number_of_merges() { return AP().size(); }

  // computes new merges and filters them with regard to judge
  // all judged merges are added to the graph and the list of active parents
  RAG_MergeIt& iterate(RAG_MergeJudge &judge, int depth=0);

  // adds all new merge-nodes in active_parents to list of nodes <nodes>
  leda_list<leda_node> &add_merges2list(leda_list<leda_node> &nodes);

  // filters the list of active parents to the list of graph nodes <nodes>
  RAG_MergeIt& filter(leda_list<leda_node>& nodes);

  void print_merges(std::ostream& s);

  operator RAGraph*() { return pRAG; }

  std::ostream& write(std::ostream& s, MergeIt::NodeList& ns);
  std::ostream& write(std::ostream& s);
    
};

#endif
