/*
 * testRAG.C
 *
 * test program for C++ Region-Adjacency-Graph class
 *
 * Sven Wachsmuth,  3.12.2002
 * Sven Wachsmuth,  1.5.2003, V0.4 (totally re-organized)
 * Sven Wachsmuth,  6.5.2003, V0.4.1 (updated read and write for RAGraph)
 * Sven Wachsmuth,  7.5.2003, V0.4.1b (version variable added)
 * Sven Wachsmuth, 28.7.2003, V0.4.1d (reading of pngs,print RAG in file added)
 * Sven Wachsmuth,  7.8.2003, V0.4.2  (merge-iterator added)
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <getopt.h>
#include <pnmadd/pnmadd.h>
#include <pnmadd/pseudo.h>
#include <pnmadd/png.h>

#include <LEDA/graph/graph.h>
#include <LEDA/graphics/graphwin.h> 
 
#include "RAGraph.h"
#include "RAGraphWin.h"
#include "RAG_MergeIt.h"
#include "Color_MergeJudge.h"
#include "RAG_CompeteGraph.h"

#define USAGE "\
Usage: %s -[rpP:cf:I:R:o:i:m:g:b:wVh] <image> [<RAG_file>]\n\
"

#define HELP_MSG "\
\t<image>\tppm or png colorMap or '-' for <stdin> (\"\" for no image)\n\
\t<RAG_file>\tread RAG from <RAG_file> or '-' for <stdin>.\n\
\t\t\tif both input streams are '-' the <image> is ignored.\n\
\t-r\t\t<ppm_input> is interpreted as regionMap\n\
\t-I <ppm>\tread original image <ppm>\n\
\t-R <ppm>\twrite regionMap to <ppm>\n\
\t-c\t\twrite file set of region contours (default: seg_%%d.pgm).\n\
\t-f <format>\twrite file set of region masks.\n\
\t-p\t\tprint Region-Adjacency-Graph on <stdout>.\n\
\t-P <file>\tprint Region-Adjacency-Graph to <file>.\n\
\t-o <format>\tset name of region contour files to <format> (.pgm or .png).\n\
\t-i <label>\tignore regions with color/region label <label>.\n\
\t-m <int>\tignore regions with more than <int> marginal pixels.\n\
\t-g [dcC]<value>[:<label>{,<label}] expand graph by merging up to <int> nodes.\n\
\t\t\t<int>==0 computes all merges of the graph.\n\
\t\t\t<label>,<label>,... restricts the merging seeds to these regions.\n\
\t-w\t\tshow generated region-adjacency graph\n\
\t-b<white-bg-thres>\ttheshold for white background [0.0-255.0]\n\
\t-V\t\tprint version\n\
\t-h\t\tprint this message.\n\
"

#define MAX_MSEEDS (32)

using namespace std;

char *program;

int main(int argc, char *argv[])
{
  int c;
  int segment_ignore = -1;
  int segment_ignore_threshold = -1;
  int show_RAG = 0;
  int print_RAG = 0;
  int read_RAG = 0;
  char *print_file = NULL;
  char *colorMap_file = "colorMap.png";

  int merging_flag = 0;
  int merging_depth = 0;
  //float merging_threshold = 0.0;
  int n_merging_seeds = 0;
  int merging_seeds[MAX_MSEEDS];

  char *contour_filename = NULL;
  int   write_contour_files = 0;
  char *regionMask_format = NULL;

  char *regionMap_file = NULL;
  char *RAG_filename = NULL;
  istream *RAG_stream = NULL;

  char          *map_name = NULL;
  FILE          *map_fp = NULL;
  ppm_t          ppm_map;
  char          *orig_name = NULL;
  FILE          *orig_fp = NULL;
  ppm_t          ppm_orig;
  unsigned char *colorMap = NULL;
  int            n_colors = 0;
  int            regionMap_flag = 0;
  int           *regionMap = NULL;
  int            n_regions = 0;

  pnm_ppm_init(&ppm_orig);
  pnm_ppm_init(&ppm_map);

  // ** process options and arguments ...

  program = ((strrchr(argv[0], '/')) ?
	     strrchr(argv[0], '/') + 1 : argv[0]);
  while ((c = getopt(argc, argv, "rpP:cf:I:R:C:o:i:m:g:wb:Vh")) != EOF) {
    switch (c) {
    case 'h':
      fprintf(stderr, USAGE, program);
      fprintf(stderr, HELP_MSG);
      exit(0);
    case 'r':
      regionMap_flag = 1;
      break;
    case 'i':
      segment_ignore = (*optarg == '-') ? -2 : atoi(optarg);
      break;
    case 'm':
      segment_ignore_threshold = atoi(optarg);
      break;
    case 'o':
      contour_filename = optarg;
      break;
    case 'c':
      write_contour_files = 1;
      break;
    case 'f':
      regionMask_format = optarg;
      break;
    case 'p':
      print_RAG = 1;
      break;
    case 'P':
      print_RAG = 2;
      print_file = optarg;
      break;
    case 'I':
      orig_name = optarg;
      break;
    case 'R':
      regionMap_file = optarg;
      break;
    case 'C':
      colorMap_file = optarg;
      break;
    case 'g':
      merging_flag = 1;
      {
	char *list = strchr(optarg,':');
	if (list) *(list++) = '\0';
	char *str = optarg;
	switch (*str) {
	case 'd':
	  merging_flag = 2; /* set merge_depth (= max. merge size) */
	  str++;
	  break;
	case 'c':
	  merging_flag = 3; /* set color threshold (Default: 0.25) */
	  str++;
	  break;
	case 'C':
	  merging_flag = 4; /* set color threshold (and write new colorMap) */
	  str++;         /* colorMap_file: -C <file> (default:colorMap.png) */
	  break;
	default:
	    cerr << program << ": illegal flag '" << *str 
		 << "' in option -g [dcC]<value>[:<id>{,<id>}]"
		 << endl;
	    exit(1);
	}
	if (merging_flag < 3)
	    merging_depth = atoi(str);
	else 
	    RAG_ColorMergeJudge::threshold = atof(str);
	if (list) {
	  char *label = list;
	  while ((list=strchr(list,',')) && n_merging_seeds+1 < MAX_MSEEDS) {
	    *(list++) = '\0';
	    merging_seeds[n_merging_seeds++] = atoi(label);
	    label = list;
	  }
	  merging_seeds[n_merging_seeds++] = atoi(label);
	}
      }
      break;
    case 'b':
      RAG_ColorMergeJudge::white_bg = atof(optarg);
      break;
    case 'w':
      show_RAG = 1;
      break;
    case 'V':
      cerr << program << ": " << RAGraph::version << endl;
      break;
    default:
      fprintf(stderr, USAGE, program);
      exit(1);
    }
  }
  if (optind >= argc) {
    fprintf(stderr, USAGE, program);
    exit(1);
  }

  // ** read map ...

  if (strcmp(argv[optind],"-")) {
    if (*argv[optind]) {
      map_name = argv[optind];
      map_fp = fopen(map_name,"r");
      if (!map_fp) {
	fprintf(stderr, "%s: could not open file '%s'.\n", program, map_name);
	exit(1);
      }
    } else 
      map_fp = NULL;
  } else
    map_fp = stdin;

  if ((++optind) < argc) {
    read_RAG = 1;
    if (strcmp(argv[optind],"-")) {
      RAG_filename = argv[optind];
      RAG_stream = new ifstream(RAG_filename, ios::in);
    } else {
      if (map_fp == stdin) map_fp = NULL;
      RAG_stream = &cin;
    }
  }
  if (map_fp) {
    int status;
    if ((status=pnm_ppm_readpng(&ppm_map, map_fp)) == PNM_PNG_NOPNG) {
      ppm_map.pixels = ppm_readppm(map_fp, 
				   &ppm_map.cols,
				   &ppm_map.rows,
				   &ppm_map.maxval);
      if (!ppm_map.pixels) {
	fprintf(stderr, "%s: error while reading ppm image from '%s'.\n",
		program, (map_fp == stdin) ? "<stdin>":map_name);
	exit(1);
      }
    } else if (status != PNM_PNG_OK) {
      fprintf(stderr, "%s: error while reading png image from '%s'.\n",
	      program, (map_fp == stdin) ? "<stdin>":map_name);
      exit(1);
    }
  }
    
  // ** read original
  
  if (orig_name) {
    orig_fp = fopen(orig_name,"r");
    if (!orig_fp) {
      fprintf(stderr, "%s: could not open file '%s'.\n", program, orig_name);
      exit(1);
    }
    ppm_orig.pixels = ppm_readppm(orig_fp,
				  &ppm_orig.cols,
				  &ppm_orig.rows,
				  &ppm_orig.maxval);
    if (!ppm_orig.pixels) {
      fprintf(stderr, "%s: error while reading ppm image from '%s'.\n",
	      program, orig_name);
    }
  }
  
  // ** transform ppm_map to colorMap or regionMap
  pixel *colorTab = NULL;

	if (map_fp) 
	{
		cerr << "\nLine: " << __LINE__ << endl;
		if (regionMap_flag) 
		{
			cerr << "\nLine: " << __LINE__ << endl;
			regionMap = (int *) malloc(ppm_map.cols*ppm_map.rows * sizeof(int));
			n_regions = pnm_ppmPseudoColor2intMap(regionMap, &ppm_map);
			// ** make sure that marginal pixels have no region label
			pnm_setIntMargin(regionMap, ppm_map.cols, ppm_map.rows, -1);
		} 
		else 
		{
			cerr << "\nLine: " << __LINE__ << endl;
			colorMap = (unsigned char *) 
			malloc(ppm_map.cols*ppm_map.rows * sizeof(unsigned char));
			//n_colors = pnm_ppmPseudoColor2charMap(colorMap, &ppm_map);
			n_colors = pnm_ppmSegColor2charMap(colorMap, &ppm_map, &colorTab);
			cerr << program << ": colorTab computed for " << n_colors << " colors." << endl;
		}
	}
	
  // ** construct RAG
  RAG mRAG;
  if (ppm_orig.pixels) 
    mRAG.setImage(&ppm_orig);
  RAGraph gRAG(&mRAG);

	cerr << "\nLine: " << __LINE__ << endl;
	if (read_RAG) 
	{
		cerr << "\nLine: " << __LINE__ << endl;
		if (map_fp) 
		{
			cerr << "\nLine: " << __LINE__ << endl;
			
			if (regionMap_flag) 
			{
				cerr << "\nLine: " << __LINE__ << endl;
				rag_setRegionMap((rag_t *) mRAG.get(), 
					regionMap, -1, ppm_map.cols, ppm_map.rows);	 
			}
			else 
			{
				cerr << "\nLine: " << __LINE__ << endl;
				rag_setColorMap((rag_t *) mRAG.get(), 
					colorMap, -1, ppm_map.cols, ppm_map.rows);
				rag_setColorTabByPixel((rag_t *) mRAG.get(), colorTab, n_colors);
				cerr << program << ": color Tab set" << endl;
				rag_computeRegionMap((rag_t *) mRAG.get());
			}
		}
		
		cerr << "\nLine: " << __LINE__ << endl;
		(*RAG_stream) >> gRAG;
	} 
	else 
	{
		cerr << "\nLine: " << __LINE__ << endl;
		if (regionMap_flag) 
		{
			cerr << "\nLine: " << __LINE__ << endl;
			mRAG.init(regionMap, -1, ppm_map.cols, ppm_map.rows, n_regions);
		}
		else 
		{
			cerr << "\nLine: " << __LINE__ << endl;
			mRAG.initPixelTab(colorMap, -1, ppm_map.cols, ppm_map.rows, 
				n_colors, colorTab, -1);
		}
		cerr << "\nLine: " << __LINE__ << endl;
		gRAG.update(segment_ignore, segment_ignore_threshold);
	}
  //RAGraph gRAG(&mRAG, segment_ignore, segment_ignore_threshold);

  // ** optionally write regionMap
  if (regionMap_file) {
    ppm_t *ppm = pnm_intMap2ppmPseudoColor(NULL, gRAG.getRegionMap(), 
					   gRAG.getCols(), gRAG.getRows());
    FILE *fp;
    if ((fp = fopen(regionMap_file,"w"))) {
      ppm_writeppm(fp, ppm->pixels, ppm->cols, ppm->rows, ppm->maxval,1);
    }
    pnm_ppm_free(ppm);
  }
  // ** optionally compute merges
  if (merging_flag > 0) {
    leda_list<leda_node> seeds;
    int i;
    for (i=0; i < n_merging_seeds; i++) {
      leda_node r = gRAG.getNode(merging_seeds[i]);
      if (r != nil) {
	seeds.append(r);
	cerr << argv[0] << ": adding seed "
	     << "[" << i << "] " << gRAG[r].getLabel() << endl;
      } else {
	cerr << argv[0] << ": no seed for [" << i << "]" << endl;
      }
    }
    if (merging_flag < 3) {

	// ** init merge generation by seed region-labels <seeds>
	RAG_MergeIt merge_it(gRAG,seeds);

	// ** while next merge is defined and its size <= merging_depth
	while (merge_it.valid()) {
	    if (merge_it.get_item().size() > merging_depth) break;
	    
	    // ** get parent node of next merge
	    leda_node parent = merge_it.get_parentMerge();
	    if (parent) {

		// ** get nodes that are to be added to parent node
		leda_list<leda_node> added;
		merge_it.get_addedNodes(added);

		// ** successively merge parent node with nodes to be added
		leda_node r;
		forall(r,added) {
		    parent = gRAG.merge(parent,r);
		}
		// ** set corresponding node for next merge
		merge_it.set_mergeNode(parent);
		
	    } else {
		/*cerr << "initial merge." << endl;*/
	    }
	    ++merge_it;
	}
    } else {
	leda_list<leda_node> winner_nodes;
	RAG_ColorMergeJudge judge;

	// ** do until no new merges (<winner_nodes>) are produced ...
	do {
	  RAG_MergeIt *pMergeIt;
	  
	  // ** Initialize Merge-Iterator <pMergeIt>
	  //    - initially the <winner_nodes> list is empty
	  //    - otherwise it is initialized by the merges generated
	  //      in the last loop (<winner_nodes>)
	  if (winner_nodes.empty())
	      // ** If <seeds> are defined all merges are considered that
	      //    include one of the seed regions.
	      //    This constraint is dropped in the next iteration,
	      //    because all non-overlapping nodes of gRAG are assigned to
	      //    <winner_nodes>.
	      pMergeIt = new RAG_MergeIt(gRAG,seeds);
	  else
	      // ** All merges between accepted merges in <winner_nodes>
	      //    have to be checked
	      pMergeIt = new RAG_MergeIt(gRAG,winner_nodes,winner_nodes);

	  // ** compute all merges, if a merge is accepted by <judge>
	  //    the merge is added as a node to Graph <gRAG>.
	  //    If no new merges are accepted by <judge> exit loop
	  if (pMergeIt->iterate(judge).converged()) break;

	  cerr << program << ": number of added merges: " 
	       << pMergeIt->number_of_merges() << endl;
	  //pMergeIt->write(cerr);
	  
	  // ** eliminate overlapping merges in a compete-graph
	  RAG_CompeteGraph CG;

	  // ** initially <winner_nodes> is empty.
	  //    The accepted merges have been added to the graph.
	  if (winner_nodes.empty())
	      // ** in order to compute a consistent subgraph of gRAG
	      //    (no overlapping nodes) all nodes of gRAG are inserted
	      //    into the compete-graph
	      CG.init(gRAG);
	  else {
	      // ** add active parents to <winner_nodes>
	      //    because those define the newly merged regions.
	      //    Keep old winner-nodes in the list because
	      //    they define the remaining parts if two regions overlap
	      pMergeIt->add_merges2list(winner_nodes);
	      // ** init compete-graph only based on <winner_nodes>
	      CG.init(gRAG, winner_nodes);
	  }
	  cerr << program << ": number of graph nodes " 
	       << CG.number_of_nodes() << flush;
	  // ** eliminate dominated regions
	  CG.reduce();
	  cerr << " reduced to " << CG.number_of_nodes() << "::";

	  // ** redefine <winner_nodes> as the remaining nodes
	  //    in the compete-graph
	  winner_nodes.clear();
	  leda_node cnode;
	  forall_nodes(cnode,CG) {
	      leda_node r = CG[cnode].getNode();
	      winner_nodes.append(r);
	      cerr << " " << gRAG[r].getLabel() << flush;
	  }
	  cerr << endl;

	  // ** filter active parents to winner_nodes (is not really needed?)
	  pMergeIt->filter(winner_nodes);
	  cerr << program << ": number of filtered merges: " 
	       << pMergeIt->number_of_merges() << endl;
	  //pMergeIt->write(cerr);
	  delete pMergeIt;
	} while (!winner_nodes.empty());

	// ** is this really correct??? the filtering step is not applied
	//    to gRAG!!!
	if (merging_flag == 4) {
	    gRAG.writeMergedColorMap(colorMap_file);
	}
    }
    /*************
	RAG_MergeIt merge_it(gRAG,seeds);
	while (!merge_it.iterate(judge).converged()) {
	    cerr << program << ": number of added merges: " 
		 << merge_it.number_of_merges() << endl;
	    merge_it.write(cerr);

	    RAG_CompeteGraph CG;
	    if (winner_nodes.empty())
		CG.init(gRAG);
	    else {
		merge_it.add_merges2list(winner_nodes);
		CG.init(gRAG, winner_nodes);
	    }
	    cerr << program << ": number of graph nodes " 
		 << CG.number_of_nodes() << flush;
	    CG.reduce();
	    cerr << " reduced to " << CG.number_of_nodes() << "::";

	    winner_nodes.clear();
	    leda_node cnode;
	    forall_nodes(cnode,CG) {
		leda_node r = CG[cnode].getNode();
		winner_nodes.append(r);
		cerr << " " << gRAG[r].getLabel() << flush;
	    }
	    cerr << endl;
	    merge_it.filter(winner_nodes);
	    cerr << program << ": number of filtered merges: " 
		 << merge_it.number_of_merges() << endl;
	    merge_it.write(cerr);
	}
	if (merging_flag == 4) {
	    gRAG.writeMergedColorMap(colorMap_file);
	}
    }
    *************/
  }
  // ** optionally write contours
  if (contour_filename) gRAG.setContourFormat(contour_filename);
  if (write_contour_files) gRAG.writeContours();
  if (print_RAG) {
    if (print_file) {
      ofstream s(print_file);
      gRAG.write(s);
    } else {
      gRAG.write(cout);
    }
  }
  if (regionMask_format) {
    leda_node r;
    forall_nodes(r,gRAG) {
	gRAG.writeRegionMask(r,regionMask_format);
    }
  }

  if (show_RAG) {
    float image_factor = 256.0 / gRAG.getCols();
    int image_scale = ((image_factor > 1.0) ? 
		       ((int) image_factor) - 1 :
		       (1 - (int) (1.0 / image_factor)));
    fprintf(stderr,"%s: starting window with scale %d.\n", argv[0],image_scale);
    RAGraphWin gw(gRAG, "RAG", image_scale);
    gw.edit();
    //fprintf(stderr,"... finished\n");
  }
  /**********
  {
    rag_t *rag = (rag_t *) mRAG.get();
    int i;
    char *s = NULL;
    printf("%d %d\n",RAG_N_REGIONS(rag),RAG_N_ADJACENCIES(rag));
    RAG_FORALL_REGIONS(rag,i) {
      region_info_t *r = RAG_GET_REGION(rag,i);
      s = region_info_sprint(s,r);
      printf("%s\n",s);
    }
    RAG_FORALL_ADJACENCIES(rag,i) {
      region_adjacency_t *adj = RAG_GET_ADJACENCY(rag,i);
      s = region_adjacency_sprint(s,adj);
      printf("%s\n",s);
    }
  }
  ************/

  /*else
   *  gRAG.computeMerges(3);
   */
  exit(0);
}



