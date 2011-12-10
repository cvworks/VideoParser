/*
 * RAG_MergeIt.C
 *
 * Iterator for RAGraph-merging
 *
 * Sven Wachsmuth, 11.11.2003
 */
#include <fstream>
#include "RAG_MergeIt.h"

using namespace std;
using namespace leda;

ostream& RAG_MergeIt::write(ostream& s, MergeIt::NodeList& ns)
{
  leda_node r;
  r = ns.getNode();
  if (r == nil) return (s);

  s << (*pRAG)[r].getLabel() << " ("; 
  forall(r,ns) {
    leda_node _r = get_node(r);
    s << " " << (*pRAG)[_r].getLabel();
  }
  s << ") ";
  return (s);
}

ostream& RAG_MergeIt::write(ostream& s)
{
  list_item i;
  forall_items(i,AP()) {
    MergeIt::NodeList *nl = AP()[i];
    write(s, *nl) << endl;
  }
  return (s);
}

void RAG_MergeIt::_init(RAGraph& RAG)
{
  pRAG = &RAG;
  clear();

  // add all single nodes to list of active parents
  if (verbose_mode > 1) {
      cerr << "TVL_RAGPtr -> RAG_MergeIt::_init: size of next merge: "
	   << ((valid()) ? get_item().size() : -1) << endl;
  }
  while (valid() && get_item().size() <= 1) {
    AP().append(& (MergeIt::NodeList&) get_item());
    ++(*this);
  }
  if (verbose_mode > 1) {
      cerr << "TVL_RAGPtr -> RAG_MergeIt::_init: AP size "
	   << AP().size() << endl;
  }
}

void RAG_MergeIt::compute_merges(int depth/*=0*/) 
{
  //# computes next levels of possible merges up to a depth of <depth>
  //# (<depth>==0 means expand by one node)
  //# and filters them due to the list of active parents

  if (!valid()) {
    AP().clear();
    AM().clear();
    return;
  }

  // store the size of the next following merge
  int m_size = get_item().size();

  // add new merges to list of active merges until defined <depth> is reached
  int n_merges=0, n_active=0;
  for (AM().clear();
       valid() && get_item().size() <= m_size+depth;
       ++(*this)) {

    n_merges++;

    // check for each new merge if it is a superset of an active parent
    if (link_merge((MergeIt::NodeList&) get_item())) {
      AM() += &((MergeIt::NodeList&) get_item());
      n_active++;
    }
  }
  if (verbose_mode > 1) {
      cerr << "TVL_RAGPtr::compute_merges: " << n_active << " of " << n_merges
	   << endl;
  }
  // initialize list of active parents
  AP().clear();
}

bool RAG_MergeIt::link_merge(MergeIt::NodeList& merge) 
{
  //# links one of the active parents that is a subset of merge
  //# to merge as a parent and return true
  //# If no active parent is found, false is returned

  // if the original parent is in the set of active parents
  // keep this link ...
  MergeIt::NodeList *pParent = merge.getParent();
  MergeIt::NodeList *pMerge;
  forall(pMerge,AP()) if (pMerge == pParent) return (true);

  // ... otherwise find a new parent ...
  leda_node_set merge_set(*get_MergeGraph());
  leda_node r;
  forall(r,merge) {
    merge_set.insert(r);
  }

  // ... if parent is subset of merge, link new parent
  // (the list of parents is tested in reverse order to make sure
  //  that one of the largest subsets is taken)
  list_item i;
  for(i=AP().last(); i != nil; i=AP().pred(i)) {
    int is_superset = 1;
    forall(r,*AP()[i]) {
      if (!merge_set.member(r)) {
	is_superset = 0;
	break;
      }
    }
    // if all nodes in AP()[i] are members, then link it as parent ...
    if (is_superset) {
      merge.setParent(AP()[i]);
      return (true);
    }
  }
  return (false);
}

leda_node RAG_MergeIt::set_merge(MergeIt::NodeList *p_merge)
{
  if (!p_merge) return (nil);
  if (p_merge->getNode()) return (p_merge->getNode());

  leda_node parent = p_merge->getParent()->getNode();

  // if it exists ...
  if (parent) {
    // compute merge in RAG
    leda_list<leda_node> added;
    leda_node_array<bool> added_flags(*pRAG,false);

    p_merge->getAddedNodes(added);
    leda_node a;
    forall(a,added) added_flags[a] = true;

    list_item it;
    forall_items(it,added) {
      leda_node r;
      leda_edge e;
      forall_inout_edges(e,parent) {
	r = pRAG->opposite(parent,e);
	if (added_flags[r]) break;
      }
      if (added_flags[r]) {
	//cerr << "MERGING:\n" << (*pRAG)[parent] << endl << (*pRAG)[r] << endl;
	//pRAG->info(cerr);
	parent = pRAG->merge(parent,r);
	//cerr << "=> " << (*pRAG)[parent] << endl;
	added_flags[r] = false;
      } else {
	  if (verbose_mode > 0) {
	      cerr << "NO MERGE PATH!" << endl;
	  }
      }
    }
    p_merge->setNode(parent);
  }
  return (parent);
}

leda_node RAG_MergeIt::set_merge(leda_node r, leda_node s)
{
  NodeList  *m_list = get_item(r,s);
  if (!m_list) {
      if (verbose_mode > 0) {
	  cerr << "NO NODELIST FOUND!" << endl;
      }
      return (nil);
  }
  if (m_list->getNode() == nil) {
      leda_node m = pRAG->merge(r,s);
      m_list->setNode(m);
      return (m);
  } else {
      if (verbose_mode > 1) {
	  cerr << "NODE " << (*pRAG)[m_list->getNode()].getLabel()
	       << " ALREADY EXISTS!" << endl;
      }
      return (m_list->getNode());
  }
}

RAG_MergeIt& RAG_MergeIt::iterate(RAG_MergeJudge &judge, int depth/*=0*/)
{
  //int i=0;
  compute_merges(depth);
  list_item it = nil;
  while ((it = next_merge(it))) {
      MergeIt::NodeList *p_merge = (*this)[it];

      //cerr << (i++) << "[" << p_merge->size() << "] " << flush;

      float score=0.0;
      if (judge.valid(*this, p_merge, score)) {
	  leda_node m = set_merge(p_merge);
	  (*pRAG)[m].score = score;

	  add_merge(p_merge);
      }
  }
  judge.post_processing();
  return (*this);
}

leda_list<leda_node>& RAG_MergeIt::add_merges2list(leda_list<leda_node> &nodes)
{
  list_item i;
  forall_items(i,AP()) {
      if (AP()[i]->getNode() != nil) nodes.append(AP()[i]->getNode());
  }
  return (nodes);
}

RAG_MergeIt& RAG_MergeIt::filter(leda_list<leda_node>& nodes)
{
    if (nodes.empty()) return (*this);
    leda_node r = nodes.head();
    leda_graph *pG = graph_of(r);
    leda_node_array<bool> Gnodes(*pG,false);
    forall(r,nodes) Gnodes[r] = true;

    list_item i,j;
    for (i = AP().first(); i != nil; i = j) {
	j = AP().succ(i);
	leda_node r = AP()[i]->getNode();
	if (!Gnodes[r]) {
	    AP().del(i);
	}
    }
    return (*this);
}

void RAG_MergeIt::print_merges(ostream& s)
{
    leda_edge e;
    forall_edges(e,*pRAG) {
	leda_node r = source(e);
	leda_node t = target(e);
	if (pRAG->reversal(e) == nil || r < t) {
	    //MergeIt::NodeList *pM = get_item(r,t);
	    //if (!pM || pM->getNode() == nil) {
	    //    cerr << "RAG_MergeIt::print_merges: missing merge " << endl
	    //	 << "- " << (*pRAG)[r].getInfoString() << endl
	    //	 << "- " << (*pRAG)[t].getInfoString() << endl;
	    //} else {
	    //    s << (*pRAG)[pM->getNode()].getLabel() 
	    s     << "[" << (*pRAG)[r].getLabel() 
		  << " " << (*pRAG)[t].getLabel() 
		  << "] ";
	    //}
	}
    }
    s << endl;
}



