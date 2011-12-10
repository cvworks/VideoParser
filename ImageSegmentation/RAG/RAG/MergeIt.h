/*
 * MergeIt.h
 *
 * Iterator for graph-merging
 *
 * Sven Wachsmuth, 6.8.2003
 */
#ifndef RAG_MERGE_IT_H
#define RAG_MERGE_IT_H

#include <LEDA/graph/graph.h>
#include <LEDA/graph/node_set.h>

class MergeIt {

 public:

  class AdjNodeSet : public leda_node_set {
  public:
    AdjNodeSet(leda_graph& G) : leda_node_set(G) {}
    ~AdjNodeSet() {}
    
    AdjNodeSet& set(leda_node r);
    
    AdjNodeSet& set(leda_list<leda_node> &rs, leda_node_array<leda_edge> &es,
		    int (*preference)(leda_edge, leda_edge)=NULL);
  };

  class MergeGraph : public leda::GRAPH<leda_node,leda_list<leda_node> > {
  public:
    MergeGraph(leda_graph& G);
    // initialize with active nodes <rs>
    MergeGraph(leda_graph& G, leda_list<leda_node>& rs);
    // initialize with active nodes <rs> and restrict merges to nodes <ns>
    MergeGraph(leda_graph& G, leda_list<leda_node>& rs,leda_list<leda_node>& ns);
    ~MergeGraph() {}
    
    MergeGraph& reduce_node(leda_node r);
  };

  class NodeList : public leda_list<leda_node> {
    
  protected:
    leda_node merged_node;
    NodeList *merged_from;
    
    void _init() { merged_node=nil; merged_from=NULL; }
  public:
    NodeList(NodeList *_from=NULL) { _init(); merged_from=_from; }
    NodeList(leda_node r, NodeList *_from=NULL) : leda_list<leda_node>(r) 
      { _init(); merged_from=_from; }
    NodeList(leda_list<leda_node>& l, NodeList *_from=NULL) 
      { _init(); append(l); merged_from=_from; }
    
    NodeList& append(leda_node r) {
      leda_list<leda_node>::append(r);
      return (*this);
    }
    NodeList& append(leda_list<leda_node>& l) {
      leda_node r;
      forall(r,l) leda_list<leda_node>::append(r);
      return (*this);
    }
    NodeList& merge(leda_list<leda_node>& l);
    
    leda_node setNode(leda_node r) { 
      leda_node s = merged_node;
      merged_node = r;
      return (s);
    }
    leda_node getNode() const {
      return (merged_node);
    }
    leda_list<leda_node>& getNodeSet(leda_list<leda_node>& node_set) const;

    NodeList *getParent() const { return (merged_from); }
    NodeList *setParent(NodeList *parent) { 
      NodeList *_p = merged_from;
      merged_from = parent;
      return (_p);
    }

    leda_list<leda_node>& getAddedNodes(leda_list<leda_node>& _node_set) const;
  };
  
  class MergeList_item : public leda_list<NodeList> {
    
  public:
    bool completed;
    MergeList_item() { completed=false; }
  };
  
  class MergeList : public leda_list<MergeList_item> {
    
  private:
    static const MergeList_item _merges_nil;
    
  protected:
    MergeGraph *pM;
    
    static int _edge_preference(leda_edge e, leda_edge f) {
      MergeGraph *pM = (MergeGraph *) graph_of(e);
      if ((*pM)[e].size() < (*pM)[f].size())
	return (1);
      else if ((*pM)[e].size() > (*pM)[f].size())
	return (-1);
      else
	return (0);
    }
    
  public:
    MergeList(MergeGraph& M)
      : leda_list<MergeList_item>(_merges_nil) 
      {
	pM = &M;
	leda_list<NodeList>& _head = (*this)[first()];
	leda_node r;
	forall_nodes(r,M) {
	  NodeList _merge(r);
	  _merge.setNode(M[r]);
	  _head += _merge;
	}
      }
    MergeList(MergeGraph& M, leda_list<leda_node>& rs)
      : leda_list<MergeList_item>(_merges_nil) 
      {
	pM = &M;
	leda_list<NodeList>& _head = (*this)[first()];
	leda_node r;
	if (rs.empty()) {
	  forall_nodes(r,M) {
	    NodeList _merge(r);
	    _merge.setNode(M[r]);
	    _head += _merge;
	  }
	} else {
	  int n = rs.size();
	  forall_nodes(r,M) {
	    NodeList _merge(r);
	    _merge.setNode(M[r]);
	    _head += _merge;
	    if ((--n) == 0) break;
	  }
	}
      }
    ~MergeList() {}
    
    leda::list_item compute(leda::list_item it);

    friend std::ostream& operator<<(std::ostream& s, MergeIt::MergeList& m);
  };


  // ** members of the class MergeIt
  
 protected:
  MergeGraph M;
  MergeList  merges;
  leda::list_item  merges_it;
  leda::list_item  merge_it;

 public:
  static int verbose_mode;

  MergeIt(leda_graph &G) : M(G), merges(M) {
    merges_it = merges.first();
    merge_it  = merges[merges_it].first();
  }
  MergeIt(leda_graph &G, leda_list<leda_node>& rs) : M(G,rs), merges(M,rs) {
    merges_it = merges.first();
    merge_it  = merges[merges_it].first();
  }
  MergeIt(leda_graph &G, leda_list<leda_node>& rs, leda_list<leda_node>& ns) 
      : M(G,rs,ns), merges(M,rs) {
      merges_it = merges.first();
      merge_it  = merges[merges_it].first();
  }
  bool valid() {
    return (merges_it != nil && merge_it != nil);
  }
  leda_list<leda_node>& get_item() {
    return merges[merges_it][merge_it];
  }
  const MergeIt::MergeGraph *get_MergeGraph() {
    return &M;
  }
  leda_node get_node(leda_node r) {
    return M[r];
  }
  leda_list<leda_node>& get_nodeSet(leda_list<leda_node>& _node_set) const {
    return merges[merges_it][merge_it].getNodeSet(_node_set);
  }
  leda_node get_parentMerge() const {
    NodeList *parent = merges[merges_it][merge_it].getParent();
    return (parent) ? parent->getNode() : nil;
  }
  leda_list<leda_node>& get_addedNodes(leda_list<leda_node>& _node_set) const {
    return merges[merges_it][merge_it].getAddedNodes(_node_set);
  }
  leda_node set_mergeNode(leda_node r) {
    return merges[merges_it][merge_it].setNode(r);
  }
  leda_node get_mergeNode() const {
    return merges[merges_it][merge_it].getNode();
  }

  MergeIt& operator++() {
    do {
      if ((merge_it = merges[merges_it].succ(merge_it)) == nil) {
	merges_it = merges.compute(merges_it);
	merge_it = merges[merges_it].first();
      }
    } while (merge_it != nil && !validMerge(get_item()));

    return (*this);
  }

  /** look for or compute NodeList that consists of merge r,s */
  NodeList* get_item(leda_node r, leda_node s);
  /** look for NodeList that points to r (starting with merges[i]) */
  NodeList* get_item(leda_node r, leda::list_item& i);
  /** look for or compute NodeList that equals ns (starts at merges[i]) */
  NodeList* get_item(NodeList& ns, leda::list_item& i);

  ~MergeIt() {}

  int validMerge(leda_list<leda_node>& ns);

};

#endif
