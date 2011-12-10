/**------------------------------------------------------------------------
 * All Rights Reserved
 * Author: Diego Macrini
 *-----------------------------------------------------------------------*/
#pragma once

namespace vpl {

#ifndef nil
#define nil 0
#endif

/*template<class T, class T_it>
inline void FIRST_ELEMENT(const T& S, T_it& x) 
{
	x = S.begin();
}

template<class T_it>
inline void FIRST_ELEMENT(T_it& x, void* p) { x = (T_it)p; }

// Inspired by LEDA macros in iteration.h

template<class T_it>
inline void LOOP_ASSIGN(T_it& x, void* p) { x = (T_it)p; }

template<class T, class T_it>
inline void* LOOP_ASSIGN_NEXT(const T& S, T_it& x, void*& p) 
{ x = (T_it)p; p = S.next_item(x); return x; }

#define forall_items(x,S)\
LEDA_FORALL_PREAMBLE \
for(void* loop_var=(S).first_item();\
LOOP_ASSIGN_NEXT(S,x,loop_var); )


// The following is an inspirational macro
#define forall_adj_nodes(u,v)\
LEDA_FORALL_PREAMBLE \
for(leda_edge loop_var=First_Adj_Edge(v,0);\
u = loop_var ? opposite(v,loop_var) : 0,\
loop_var=Succ_Adj_Edge(loop_var,v), u;)
/*

/*! 
	Handy funtion to iterate through the items of a list of nodes

	@param v a node variable
	@param L a list of nodes
*/
#define forall_node_items(v, L) \
	for (NodeIt loop_var = (L).begin(); \
	     (v = (loop_var == (L).end()) ? nil : *loop_var) != nil; \
	    ++loop_var)

/*! 
	Handy funtion to iterate through the items of a list of edges

	@param e an edge variable
	@param L a list of edge
*/
#define forall_edge_items(e, L) \
	for (EdgeIt loop_var = (L).begin(); \
	     (e = (loop_var == (L).end()) ? nil : *loop_var) != nil; \
		 ++loop_var)

/*! 
	Handy funtion to iterate through the nodes of a Graph

	@param v a graph::node
	@param G a graph object
*/
#define forall_nodes(v, G) \
	for (v = (G).first_node(); v != nil; v = (G).succ_node(v))

/*! 
	Handy funtion to iterate through the nodes of a Graph

	@param v a graph::node
	@param G a graph object
*/
#define forall_edges(e, G) \
	for (e = (G).first_edge(); e != nil; e = (G).succ_edge(e))


/*! 
	Handy funtion to iterate through the edges adjacent to node w

	@param e a graph::edge
	@param w a graph::node
*/
#define forall_adj_edges(e, w) \
	for (graph::ConstEdgeIt loop_var = graph::DerefData(w).out_edges().begin(); \
		(e = (loop_var == graph::DerefData(w).out_edges().end()) \
			? nil : *loop_var) != nil; \
		++loop_var)

/*! 
	Handy funtion to iterate through the nodes adjacent to node w

	@param v a graph::node
	@param w a graph::node
*/
#define forall_adj_nodes(v, w) \
	for (graph::ConstEdgeIt loop_var = graph::DerefData(w).out_edges().begin(); \
		(v = (loop_var == graph::DerefData(w).out_edges().end()) \
			? nil : graph::DerefData(*loop_var).opposite(w)) != nil; \
		++loop_var)

/*! 
	Handy funtion to iterate through the edges incident to node w

	@param e a graph::edge
	@param w a graph::node
*/
#define forall_in_edges(e, w) \
	for (graph::ConstEdgeIt loop_var = graph::DerefData(w).in_edges().begin(); \
		(e = (loop_var == graph::DerefData(w).in_edges().end()) \
			? nil : *loop_var) != nil; \
		++loop_var)

/*! 
	Handy funtion to iterate through the nodes incident to node w

	@param v a graph::node
	@param w a graph::node
*/
#define forall_in_nodes(v, w) \
	for (graph::ConstEdgeIt loop_var = graph::DerefData(w).in_edges().begin(); \
		(v = (loop_var == graph::DerefData(w).in_edges().end()) \
			? nil : graph::DerefData(*loop_var).opposite(w)) != nil; \
		++loop_var)

} // namespace vpl

