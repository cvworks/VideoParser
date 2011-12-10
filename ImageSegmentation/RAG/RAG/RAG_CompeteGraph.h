/*
 * RAG_CompeteGraph.h
 *
 * Interface between RAGraphs and CompeteGraphs
 *
 * Sven Wachsmuth, 20.11.2003
 */
#ifndef _RAG_COMPETE_GRAPH_H
#define _RAG_COMPETE_GRAPH_H

#include "CompeteGraph.h"
#include "RAGraph.h"

class RAG_CompeteGraph: public CompeteGraph {
 public:
    RAG_CompeteGraph() {}
    RAG_CompeteGraph(RAGraph &RAG) { init(RAG); }
    RAG_CompeteGraph(RAGraph &RAG, leda_list<leda_node> &valid_nodes)
	{ init(RAG,valid_nodes); }
    ~RAG_CompeteGraph() {}

    void init(RAGraph &RAG);
    void init(RAGraph &RAG, leda_list<leda_node> &valid_nodes);
};

#endif
