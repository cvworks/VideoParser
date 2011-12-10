/*
 * RAG_CompeteGraph.C
 *
 * Interface between RAGraphs and CompeteGraphs
 *
 * Sven Wachsmuth, 20.11.2003
 */
#include "RAG_CompeteGraph.h"

void RAG_CompeteGraph::init(RAGraph &RAG)
{
  leda_list<leda_node> valid_nodes = RAG.all_nodes();
  init(RAG, valid_nodes);
}

void RAG_CompeteGraph::init(RAGraph &RAG, leda_list<leda_node> &valid_nodes)
{
  const rag_t *p_rag = (RAG.get()) ? RAG.get()->get() : NULL;
  clear();

  if (!p_rag) return;

  leda_node r;
  forall(r,valid_nodes) {
      CompeteGraphNode cn(r, RAG[r].score, RAG[r].merge_size());
      /*leda_node cr = */new_node(cn);
  }
  forall_nodes(r,*this) {
      leda_node s;
      forall_nodes(s,*this) if (s != r) {
	  if (!RAG.areDisjunct((*this)[r].getNode(),(*this)[s].getNode())) {
	      if (RAG.isSuperSetOf((*this)[r].getNode(),
				   (*this)[s].getNode()) > 0) {
		  //cerr << "NEW EDGE "
		  //     << RAG[(*this)[s].getNode()].getLabel()
		  //     << "->" << RAG[(*this)[r].getNode()].getLabel() << endl;
		  new_edge(s, r, -1);
	      } else if (RAG.isSuperSetOf((*this)[s].getNode(),
					  (*this)[r].getNode()) > 0) {
		  //cerr << "NEW EDGE "
		  //     << RAG[(*this)[r].getNode()].getLabel()
		  //     << "->" << RAG[(*this)[s].getNode()].getLabel() << endl;
		  new_edge(r, s, -1);
	      } else if ((*this)[r].dominates((*this)[s])) {
		  //cerr << "NEW EDGE0 "
		  //     << RAG[(*this)[s].getNode()].getLabel()
		  //     << "->" << RAG[(*this)[r].getNode()].getLabel() << endl;
		  new_edge(s, r, 0);
	      } else {
		  //cerr << "NEW EDGE0 "
		  //     << RAG[(*this)[r].getNode()].getLabel()
		  //     << "->" << RAG[(*this)[s].getNode()].getLabel() << endl;
		  new_edge(r, s, 0);
	      }
	  }
      }
  }
}


