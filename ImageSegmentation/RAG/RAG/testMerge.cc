/*
 * testMerge.C
 *
 * startet graphische Eingabe f"ur LEDA-Graphen (Knoten-Label <int>)
 * und berechnet alle Merges in dem Graph
 *
 * Sven Wachsmuth, 11.2.2004
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <LEDA/graph/graph.h>
#include <LEDA/graphics/graphwin.h> 
#include "MergeIt.h"

class MergeGraphWin : public GraphWin {

public:
    MergeGraphWin(leda_graph& G, const char *win_label="")
	: GraphWin(G,win_label) {
	display();
	int menu_id = get_menu("Graph");
	if (menu_id >= 0) {
	    add_separator(menu_id);
	    add_simple_call((void (*)(GraphWin&)) & _merge_,
			    "merge nodes", menu_id);
	}
    }
    static void _merge_(GraphWin& gw);
};

void MergeGraphWin::_merge_(GraphWin& gw)
{
  int j=0;
  GRAPH<int,int> *pG = (GRAPH<int,int> *) &gw.get_graph();
  leda_node r;
  forall_nodes(r,*pG) (*pG)[r] = j++;

  MergeIt M(*pG);
  for (j=1; M.valid(); ++M, j++) {
      MergeIt::NodeList *pMerge = (MergeIt::NodeList *) &M.get_item();
      list_item i;
      cout << j << ":";
      forall_items(i,*pMerge) {
	  cout << " " << (*pG)[M.get_node((*pMerge)[i])];
      }
      cout << " <-";
      MergeIt::NodeList *pParent = pMerge->getParent();
      if (pParent) {
	  forall_items(i,*pParent) {
	      cout << " " << (*pG)[M.get_node((*pParent)[i])];
	  }
      }
      cout << endl;
  }
}

int main(int argc, char *argv[])
{
  GRAPH<int,int> g;
  MergeGraphWin gw(g, "Graph");
  gw.open();
  gw.edit();
  gw.close();
}





