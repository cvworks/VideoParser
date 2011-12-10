/*
 * MergeIt.C
 *
 * Iterator for graph-merging
 *
 * Sven Wachsmuth, 6.8.2003
 */
#include "MergeIt.h"
#include "RAG_MergeIt.h"

/*
 * The algorithm is producing each merge only once.
 *
 * let G(V,E) be a graph with nodes V and edges E
 * define ordering on nodes V = (v_1,v_2,...,v_N)
 * 
 * compute a triangulated MergeGraph M(V,E') by successive elimination of
 * the nodes in reverse order.
 * (induced edges are labelled by the mediating (eliminated) nodes)
 * [this is necessary because otherwise merges that are connected via a
 *  higher order node are never produced, e.g. for 1-3-2 you need the 1,2
 *  merge beforehand, because 2 is not considered for a merge with 1-3.]
 *  
 *
 * compute all merges consisting of a single node
 * 
 * given all merges with size n on the MergeGraph M,
 * compute all merges with size n+1 on M by adding all adjacent nodes
 * with a higher order than those in the merge.
 *
 * merges that are connected in M but unconnected in G are skipped in the
 * iteration (MergeIt::validMerge()) (but not in the generation process).
 *
 */

using namespace leda;
 
int MergeIt::verbose_mode = 0;

const MergeIt::MergeList_item MergeIt::MergeList::_merges_nil;

MergeIt::AdjNodeSet& MergeIt::AdjNodeSet::set(leda_node r) 
{
  leda_graph *pG = graph_of(r);
  leda_edge e;
  clear();
  forall_inout_edges(e,r) {
    leda_node s = pG->opposite(r,e);
    insert(s);
  }
  return (*this);
}

MergeIt::AdjNodeSet& MergeIt::AdjNodeSet::set(leda_list<leda_node> &rs, 
			    leda_node_array<leda_edge> &es,
			    int (*preference)(leda_edge, leda_edge)/*=NULL*/) 
{
  // ** computes the node_set of all nodes that are adjacent to rs
  //    and store the corresponding (preferred) edges in node_array es

  leda_node r;
  clear();
  // ** forall nodes r in merge rs
  forall(r,rs) {
    leda_edge e;

    // ** forall edges e adjacent to r
    forall_inout_edges(e,r) {

      // ** get opposite node s at edge e
      leda_node s = (source(e) == r) ? target(e) : source(e);

      // ** if s has been already added, ...
      if (member(s) && preference) {

	// ** if e is preferred to the already assigned edge, 
	//    (take the one with higher preference (with less node-labels)) 
	leda_edge _e = es[s];
	if (preference(e,_e) > 0)
	  es[s] = e;
	
      } else {
	// ** if s has not been added, insert node s and edge e
	insert(s);
	es[s] = e;
      }
    }
  }
  // ** exclude nodes in rs from the adjacent node set
  forall(r,rs) del(r);
  
  return (*this);
}

MergeIt::NodeList& MergeIt::NodeList::merge(leda_list<leda_node>& l) 
{
  if (empty()) {
    append(l);
    return (*this);
  }
  leda_node r = head();
  leda_node_set node_set(*graph_of(r));
  forall(r,*this) {
    node_set.insert(r);
  }
  forall(r,l) {
    if (!node_set.member(r)) {
      node_set.insert(r);
    }
    }
  return (*this);
}

leda_list<leda_node>& 
MergeIt::NodeList::getNodeSet(leda_list<leda_node>& node_set) const 
{
  node_set.clear();
  if (empty()) 
    return node_set;
  leda_node r = head();
  MergeGraph *pM = (MergeGraph *) graph_of(r);
  forall(r,*this) 
    node_set.append((*pM)[r]);
  return (node_set);
}

leda_list<leda_node>& 
MergeIt::NodeList::getAddedNodes(leda_list<leda_node>& _node_set) const 
{
  _node_set.clear();
  if (empty())
    return (_node_set);
  if (!merged_from)
    return getNodeSet(_node_set);
  leda_node r = head();
  MergeGraph *pM = (MergeGraph *) graph_of(r);
  leda_node_set parent_set(*pM);
  forall(r,*merged_from)
    parent_set.insert(r);
  forall(r,*this) {
    if (!parent_set.member(r))
      _node_set.append((*pM)[r]);
  }
  return (_node_set);
}

MergeIt::MergeGraph::MergeGraph(leda_graph& G)
{
  leda_node_array<leda_node> G2M(G);
  leda_node r;

  // ** copy graph structure ...
  forall_nodes(r,G) {
    G2M[r] = new_node(r);
  }
  leda_edge e;
  forall_edges(e,G) {
    leda_list<leda_node> nil_list;

    // ** ... and initialize edges with empty node-list
    new_edge(G2M[G.source(e)],G2M[G.target(e)],nil_list);
  }
  // ** propagate edge-labels ...
  leda_node s;
  for (r=last_node(); r != nil; r = s) {
    s = pred_node(r);
    reduce_node(r);
  }
  restore_all_nodes();
  restore_all_edges();
}

MergeIt::MergeGraph::MergeGraph(leda_graph& G, leda_list<leda_node>& rs)
{
  leda_node_array<leda_node> G2M(G,nil);
  leda_node r;

  // ** copy graph structure ...
  forall(r,rs) {
    // ** if the set of active nodes in G is restricted by rs,
    // ** the active nodes must be the first nodes in MergeGraph
    G2M[r] = new_node(r);
  }
  forall_nodes(r,G) {
    if (G2M[r] == nil) {
      // ** add non-active nodes of G
      G2M[r] = new_node(r);
    }
  }
  leda_edge e;
  forall_edges(e,G) {
    leda_list<leda_node> nil_list;

    // ** ... and initialize edges with empty node-list
    new_edge(G2M[G.source(e)],G2M[G.target(e)],nil_list);
  }
  // ** propagate edge-labels ...
  leda_node s;
  for (r=last_node(); r != nil; r = s) {
    s = pred_node(r);
    reduce_node(r);
  }
  restore_all_nodes();
  restore_all_edges();
}

MergeIt::MergeGraph::MergeGraph(leda_graph& G, leda_list<leda_node>& rs,
				leda_list<leda_node>& ns)
{
  leda_node_array<leda_node> G2M(G,nil);
  leda_node r;

  // ** copy graph structure ...
  forall(r,rs) {
    // ** if the set of active nodes in G is restricted by rs,
    // ** the active nodes must be the first nodes in MergeGraph
    G2M[r] = new_node(r);
  }
  forall(r,ns) {
    if (G2M[r] == nil) {
      // ** add non-active nodes of G
      G2M[r] = new_node(r);
    }
  }
  leda_edge e;
  forall_edges(e,G) {
    leda_list<leda_node> nil_list;

    // ** ... and initialize edges with empty node-list
    if (G2M[G.source(e)] != nil && G2M[G.target(e)] != nil)
	new_edge(G2M[G.source(e)],G2M[G.target(e)],nil_list);
  }
  // ** propagate edge-labels ...
  leda_node s;
  for (r=last_node(); r != nil; r = s) {
    s = pred_node(r);
    reduce_node(r);
  }
  restore_all_nodes();
  restore_all_edges();
}

MergeIt::MergeGraph& MergeIt::MergeGraph::reduce_node(leda_node r)
{
  leda_edge e;
  leda_node_array<bool> processed(*this,false);

  leda_list<leda_node>       adj_nodes;
  leda_node_array<leda_edge> adj_edges(*this,nil);

  // ** get all adjacent nodes of r ...
  forall_inout_edges(e,r) {
    leda_node s = opposite(r,e);
    if (!processed[s]) {
      adj_nodes.append(s);
      adj_edges[s] = e;
      processed[s] = true;
    }
  }
  // ** link all adjacent nodes with each other ...
  AdjNodeSet adj_set(*this);
  list_item i;
  forall_items(i,adj_nodes) {
    list_item j;
    adj_set.set(adj_nodes[i]);

    for (j = adj_nodes.succ(i); j != nil; j = adj_nodes.succ(j)) {
      if (!adj_set.member(adj_nodes[j])) {

	// ** the new label consists of both edge-labels + node-label r
	NodeList label((*this)[adj_edges[adj_nodes[i]]]);
	label.merge((*this)[adj_edges[adj_nodes[j]]]);
	label.leda_list<leda_node>::append(r);
	new_edge(adj_nodes[i],adj_nodes[j],label);
      }
    }
  }
  hide_node(r);

  return (*this);
}

list_item MergeIt::MergeList::compute(list_item it) 
{
  // computes next merge level and returns list_item referring to the new level

  // ** if no MergeList_item is following, add an empty one
  if (!succ(it)) {
    append(_merges_nil);
  }     
  // ** get MergeList_item for (it+1)-level merges
  MergeList_item& new_merges = (*this)[succ(it)];
  
  // ** if the (it+1)-level has already been computed,return (it+1)-level index
  if (new_merges.completed)
    return (succ(it));
  else
    new_merges.completed = true;

  // ** forall merges <merge> on level <it>
  list_item i;
  forall_items(i,(*this)[it]) {
    NodeList& merge = (*this)[it][i];

    // ** get last node in merge <merge>
    leda_node last = merge.back();

    // ** compute node set adjacent to merge <merge> in MergeGraph together
    //    with the corresponding edge set 
    //    (the edge with less node labels is preferred)
    AdjNodeSet adjset(*pM);
    leda_node_array<leda_edge> adjset_edges(*pM,nil);
    adjset.set(merge,adjset_edges, 
	      (int (*)(edge_struct*, edge_struct*)) _edge_preference);

    // ** forall nodes r after last node in merge <merge>
    //    (ordering is given by node list in MergeGraph)
    leda_node r;
    for (r = pM->succ_node(last); r != nil; r = pM->succ_node(r)) {

      // ** if node r is adjacent to merge <merge>, ...
      if (adjset.member(r)) {

	// ** generate new merge with parent <merge>
	//    (the merge consists of the same nodes as <merge>)
	NodeList new_merge(merge,&merge);

	// ** append node r to new merge
	//new_merge.append((*pM)[adjset_edges[r]]); // WARUM AUSKOMMENTIERT?
	new_merge.leda_list<leda_node>::append(r);
	
	int n = new_merge.size();
	int m = merge.size();
	// ** if the size of the merge increases by one ...
	if (n <= m+1)
	  // ... insert new merge on the it+1 merge level
	  new_merges += new_merge;
	// ** else insert merge on deeper level (caused by edge labels) ...
	else {
	  // ** get it+1 merge level
	  list_item it2 = succ(it); 
	  // ** search it+2, it+3, ... levels until merge size <n> is reached
	  for (it2=succ(it); (++m) < n; it2=succ(it2)) {

	    // ** add new levels if needed
	    if (!succ(it2)) {
	      append(_merges_nil);
	    }
	  }
	  // ** add new merge to level of merge size <n> 
	  (*this)[it2] += new_merge;
	}
      }
    }
  }
  // ** return list_item referring to it+1 merge level 
  return (succ(it));
}

std::ostream& operator<<(std::ostream& s, MergeIt::MergeList& m)
{
  list_item i,j;
  int k=1;
  forall_items(i,m) {
    s << k << "[" << m[i].completed << "]: ";
    if (m[i].empty()) {
      s << " #-" << std::endl;
    } else {
      forall_items(j,m[i]) {
	s << " #" << m[i][j].size();
      }
      s << std::endl;
    }
    k++;
  }
  return (s);
}

MergeIt::NodeList* MergeIt::get_item(leda_node r, leda_node s)
{
  /** look for or compute NodeList that consists of the merged nodes r,s */
  if (r == nil || s == nil) return (NULL);

  list_item r_it, s_it;
  MergeIt::NodeList *r_list = get_item(r, r_it=merges.first());
  MergeIt::NodeList *s_list = get_item(s, s_it=merges.first());
  if (!r_list || !s_list) {
    std::cerr << "MergeIt::get_item: node " << r << " or " << s << " not found!"
	 << std::endl;
    return (NULL);
  }
  std::cerr << "MergeIt::get_item: nodes found with sizes "
       << r_list->size() << " and " << s_list->size() << "!" << std::endl;

  list_item m_it = (r_list->size() > s_list->size()) ?  r_it : s_it;
  NodeList ns(*r_list); ns.append(*s_list);
  MergeIt::NodeList *m_list = get_item(ns, m_it);
  
  return (m_list);
}

MergeIt::NodeList* MergeIt::get_item(leda_node r, list_item& i)
{
  MergeIt::NodeList *result = NULL;

  /** look for NodeList that points to leda_node r */
  if (r == nil) return (NULL);
  
  for (; i != nil; i = merges.succ(i)) {
    list_item j;
    forall_items(j,merges[i]) {
      result = &merges[i][j];
      if (result->getNode() == r) {
	return (result);
      }
    }
  }
  return (result);
}

MergeIt::NodeList* MergeIt::get_item(NodeList& ns, list_item& _i)
{
  /** look for or compute NodeList that equals ns */
  if (ns.empty()) return (NULL);

  leda_node_set node_setM(M);
  std::cerr << "MergeIt::get_item (";
  leda_node n;
  forall(n,ns) {
    std::cerr << " " << n;
    node_setM.insert(n);
  }
  std::cerr << ") ";
  ((RAG_MergeIt *) this)->write(std::cerr,ns) << std::endl;
  int size = node_setM.size();
  list_item i,j,k;
  int found = 0; //* 0:undecided, -1:not-found, 1:(possibly)found
  
  std::cerr << "MergeIt::get_item: looking for merged node with size " 
       << size << ":";
  
  for(i = merges.succ(_i); i != nil; _i=i, i = merges.succ(i)) {
    std::cerr << " " << ((merges[i].empty()) ? -1 : merges[i].head().size())
	 << std::flush;
    if (!merges[i].completed) break;
    if (merges[i].empty()) { found = -1; break; } 
    if (merges[i].head().size() >= size) { _i=i; found = 1; break; }
  }
  std::cerr << std::endl;

  while (!found) { // == undecided
    _i = merges.compute(_i);
    found = (merges[_i].empty()) ? -1 : (merges[_i].head().size() >= size);

    std::cerr << "MergeIt::get_item: next merges (size=" 
	 << ((merges[_i].empty()) ? -1 : merges[_i].head().size())
	 << " computed!" << std::endl;
  }
  
  if (found > 0 && merges[_i].head().size() == size) {

    std::cerr << "MergeIt::get_item: looking up merges with " << size << " nodes"
	 << std::endl;
    forall_items(j,merges[_i]) {
      NodeList& ns = merges[_i][j];
      ((RAG_MergeIt *) this)->write(std::cerr << "(",ns) << ")";
      found = 1;
      forall_items(k,ns) {
	if (size >= 3) std::cerr << " " << ns[k];
	if (!node_setM.member(ns[k])) { found = 0; break; }
      }
      if (size >= 3) std::cerr << std::endl;
      if (found) break;
    }
  }
  //cerr << merges;

  return (found) ? &merges[_i][j] : NULL;
}

int MergeIt::validMerge(leda_list<leda_node>& ns)
{
  if (ns.empty()) return (0);
  leda_node r = ns.head();
  leda_graph *pG = graph_of(M[r]);

  leda_node_set rs_set(*pG);
  forall(r,ns) { rs_set.insert(M[r]); }

  leda_list<leda_node> rs;
  rs += (r=M[ns.head()]);
  rs_set.del(r);
  if (rs_set.empty()) return 1;

  while (!rs.empty()) {
    r = rs.pop();
    leda_edge e;
    forall_inout_edges(e,r) {
      leda_node s = pG->opposite(r,e);
      if (rs_set.member(s)) {
	rs += s;
	rs_set.del(s);
	if (rs_set.empty()) return 1;
      }
    }
  }
  return (rs_set.empty());
}



