/*
 * CompeteGraph.h
 *
 * Graph that models competing merges in a graph
 * - each node of the base graph has a corresponding node in the compete graph
 * - if node r of the base graph dominates node s of the base graph
 *   there is an dominated-by edge from s to r 
 * - r dominates s, IF s is a sub-merge of r OR r and s have a common sub-merge
 *   AND r has a bigger score than s
 * - reduce() deletes all nodes that are dominated in an ordering from bigger-
 *   ranks to lower ranks (makes sure that merges that are only partly
 *   dominated stay in the not-dominated part)
 *   (higher ranks mean bigger sized merges)
 * 
 * Sven Wachsmuth, 20.11.2003
 */
#ifndef _COMPETE_GRAPH_H
#define _COMPETE_GRAPH_H

#include <LEDA/graph/graph.h>

struct CompeteGraphNode {
    leda_node gnode;
    double    score;
    int       rank;

    CompeteGraphNode () {}
    CompeteGraphNode(leda_node r, double sc, int rk) {
	gnode = r; score = sc; rank = rk; }

    leda_node getNode () { return (gnode); }

    int compare(const CompeteGraphNode& cnode) const {
	return ((rank < cnode.rank)   ? (-1) :
		(rank > cnode.rank)   ? ( 1) : 
		(score < cnode.score) ? (-1) :
		(score > cnode.score) ? ( 1) : 0); }
    int dominates(const CompeteGraphNode& cnode) const {
	return (score > cnode.score) ? ( 1) : (0); }

    int operator<(const CompeteGraphNode& cnode) { return compare(cnode) < 0; }
    int operator>(const CompeteGraphNode& cnode) { return compare(cnode) > 0; }

    friend int compare(const CompeteGraphNode& cnode1,
		       const CompeteGraphNode& cnode2) {
	return cnode1.compare(cnode2); }
    friend int dominates(const CompeteGraphNode& cnode1,
			 const CompeteGraphNode& cnode2) {
	return cnode1.dominates(cnode2); }

    friend std::istream& operator>>(std::istream& s, CompeteGraphNode &cg) {
	return (s); }
    friend std::ostream& operator<<(std::ostream& s, const CompeteGraphNode &cg) {
	return (s); }
};

class CompeteGraph : public leda::GRAPH<CompeteGraphNode,int> {
 public:
    CompeteGraph() {};
    ~CompeteGraph() {};

    CompeteGraph& update();
    CompeteGraph& reduce();

    leda_node getNode(leda_node r) { return (*this)[r].getNode(); }

    friend std::istream& operator>>(std::istream& s, CompeteGraph &cg) {
	return (s); }
    friend std::ostream& operator<<(std::ostream& s, const CompeteGraph &cg) {
	return (s); }
};

#endif



