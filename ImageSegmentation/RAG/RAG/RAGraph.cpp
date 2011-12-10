/*
* RAGraph.c
*
* C++ class for Region-Adjacency-Graph
*
* Sven Wachsmuth, 3.12.2002
* Sven Wachsmuth,  1.5.2003, V0.4 (totally re-organized)
* Sven Wachsmuth,  6.5.2003, V0.4.1 (updated read and write for RAGraph)
* Sven Wachsmuth,  7.5.2003, V0.4.1b (minor corrections)
* Sven Wachsmuth, 11.5.2003, V0.4.1c (RAG::offset included)
* Sven Wachsmuth, 28.7.2003, V0.4.1d (RAGraph::getNodeByExtIndex,
*                                     and png-writing of contours added)
*/
#include "RAGraph.h"
#include "RAG_CompeteGraph.h"
#include <string.h>
#include <pnmadd/pnmadd.h>
#include <pnmadd/pseudo.h>
#include <pnmadd/array.h>
#include <pnmadd/png.h>
#include <region/regionlab.h>
#include <region/regioninfo.h>
#include <region/regionmerge.h>

#define RAGRAPH_CONTOUR_FORMAT "seg_%d.pgm"

using namespace std;

const char *RAGraph::version = "libRAGraph.a (V0.5.1)";
int         RAGraph::skip_colorMap = 0;

ostream& RAGraph::write_options(ostream& s, const char *prefix/*=NULL*/)
{
	if (!prefix) prefix = "";
	s << prefix << "RAGraph: using " << version << "." << endl;
	s << prefix << "RAGraph: using "
		<< ((skip_colorMap) ? "region map" : "color map") << "." << endl;
	s << prefix << "RAGraph: RAG uses map offset " << RAG::offset << endl;

	return (s);
}

RAG RAGraph::mRAG;

RAGraph& RAGraph::init(void) {

	// ** initially the pRAG points on the static class memory mRAG
	pRAG = &mRAG;

	ignore_color = -1;
	ignore_threshold = -1;

	contour_format = RAGRAPH_CONTOUR_FORMAT;
	return (*this);
}

//RAGraph& RAGraph::setRAG(const RAGraph& _RAG)
//{
//  ((GRAPH<RAG_Region,RAG_Adjacency>&) (*this))
//    = (GRAPH<RAG_Region,RAG_Adjacency>&) _RAG;
//  
//  cout << "init(RAG)" << endl;
//  pRAG = _RAG.pRAG;
//  ignore_color = _RAG.ignore_color;
//  ignore_threshold = _RAG.ignore_threshold;
//
//  contour_format = _RAG.contour_format;
//}

void RAGraph::initGraph(int _ignore_color/*=-1*/, int _ignore_threshold/*=-1*/)
{
	int *marginal_count = NULL;
	int n_marginal_count = 0;
	rag_t *rag;

	clear();

	ignore_threshold = _ignore_threshold;
	ignore_color = _ignore_color;

	if (!pRAG || !(rag = (rag_t *)pRAG->get())) return;

	if (ignore_color < -1)
		ignore_color = getIgnoreColor();

	if (ignore_threshold >= 0 || ignore_color < -1) {
		int imax;
		marginal_count = getMarginalCount(NULL, n_marginal_count, imax);
		if (ignore_color < -1)
			ignore_color = imax;
	}
	int i;
	RAG_FORALL_REGIONS(rag,i) {
		region_info_t *reg = RAG_GET_REGION(rag,i);

		if (reg && reg->color != ignore_color && (!marginal_count || marginal_count[i] <= ignore_threshold)) 
		{
			/*leda_node _node = */new_node(RAG_Region(rag, reg->label, getImage()));

#ifdef TRACE
			cerr << "adding region " << rag->region[i].label 
				<< " (pixel:" << rag->region[i].pixelcount << ", contour:" 
				<< rag->region[i].n_contour << ")" << endl;
#endif
		}
	}
	if (marginal_count) free(marginal_count);

	leda_node r,s;
	for (r= first_node(); r != nil; r = succ_node(r)) {
		//forall_nodes(r,*this) {
		int index_r = (*this)[r].getLabel();

		region_adjacency_t **r_adjs = rag_getTmpAdjacencies(rag,0,index_r);
		region_adjacency_t **r_radjs = rag_getTmpRevAdjacencies(rag,1,index_r);

		for (s = succ_node(r); s != nil; s = succ_node(s)) {
			int index_s = (*this)[s].getLabel();

			region_adjacency_t *adj = r_adjs[index_s];
			region_adjacency_t *rev_adj = r_radjs[index_s];

			if (region_isAdjacent(adj)) {
				leda_edge e = new_edge(r,s,RAG_Adjacency(rag,adj->label));

				if (region_isAdjacent(rev_adj)) {
					leda_edge f = new_edge(s,r,RAG_Adjacency(rag,rev_adj->label));

					set_reversal(e,f);
				}
			} else if (region_isAdjacent(rev_adj)) {
				new_edge(s,r,RAG_Adjacency(rag,rev_adj->label));
			}
		}
	}
}

const leda_node RAGraph::getNode(int label) const
{
	leda_node r;
	forall_nodes(r,*this) {
		if ((*this)[r].getLabel() == label) return (r);
	}
	return (nil);
}

const leda_node RAGraph::getNodeByExtIndex(int index) const
{
	leda_node r;
	forall_nodes(r,*this) {
		//cerr << "RAGraph::getNodeByExtIndex(" << index << "): " 
		//   << (*this)[r].external_index << endl;
		if ((*this)[r].external_index == index) return (r);
	}
	return (nil);
}

int RAGraph::getIgnoreColor(void)
{
	rag_t *rag = (pRAG) ? (rag_t *) pRAG->get() : NULL;

	if (!rag || !rag->colorMap) return (-2);

	// ** search for background color/region-label (most frequent label on
	//     marginal pixels)
	return (region_backgroundCharLabel(rag->colorMap, rag->cols, rag->rows, 1,
		rag->n_colors,NULL));
}

int *RAGraph::getMarginalCount(int *count, int &n_count, int &count_imax)
{
	rag_t *rag = (pRAG) ? (rag_t *) pRAG->get() : NULL;

	// ** search for background region-labels (all regions that have 
	//   marginal pixels)
	n_count = 0;
	count_imax = -1;

	if (!rag || !rag->regionMap)
		return (NULL);

	if (count)
		count = (int *) realloc(count, rag->n_regions * sizeof(int));
	else 
		count = (int *) malloc(rag->n_regions * sizeof(int));

	n_count = rag->n_regions;

	count_imax
		= region_backgroundIntLabel(rag->regionMap, rag->cols, rag->rows, 
		RAG::offset,
		rag->n_regions, count);
	return count;
}

void RAGraph::computeMoments(void)
{
	rag_t *rag = (rag_t *) get()->get();
	region_info_t **regions = rag_getTmpRegions(rag,0);
	int max_label = getMaxLabel();

	region_moments(regions, max_label+1, 
		getRegionMap(), getCols(), getRows(), 10);
}


void RAGraph::align(double &scale, 
					double &offset_x, double &offset_y,
					leda_node r, region_info_t *match)
{
	region_info_t *reg = ((RAG_Region *) &(*this)[r])->get();

	offset_x = match->center_x - reg->center_x;
	offset_y = match->center_y - reg->center_y;
	scale  = sqrt((double) match->m11) / sqrt((double) reg->m11);
}

void RAGraph::getRegionMask(unsigned char *mask, int mask_cols, int mask_rows,
							double scale, double offset_x, double offset_y,
							leda_node r)
{
	memset(mask, 0, mask_cols * mask_rows * sizeof(char));
	setRegionMask(mask, mask_cols, mask_rows, scale, offset_x, offset_y, r, 1);
}

void RAGraph::setRegionMask(unsigned char *mask, int mask_cols, int mask_rows,
							double scale, double offset_x, double offset_y,
							leda_node r, int value)
{
	RAG_Region *reg = (RAG_Region *) &(*this)[r];
	int n_labels;
	const int *labels = getSubLabels(r,n_labels);
	int max_label = getMaxLabel();
	char *labelMask = (char *) malloc((max_label+1) * sizeof(char));
	memset(labelMask,0,(max_label+1) * sizeof(char));
	int i;
	for (i=0; i < n_labels; i++) {
		if (labels[i] < 0 || labels[i] > max_label) {
			cerr << "RAGraph::setRegionMask: illegal label " << labels[i] 
			<< " at position " << i << endl;
			exit(1);
		}
		labelMask[labels[i]] = 1;
	}
	const int *regionMap = getRegionMap();
	int cols = getCols();
	int rows = getRows();
	//int start_x = reg->get()->start_x;
	int start_y = reg->get()->start_y;
	int i_map = start_y * cols;

	int x,y;
	for (y=start_y; y < rows; y++) {
		int _y = (int) ((((double) y) + offset_y) * scale);
		for (x=0; x < cols; x++, i_map++) {
			if (regionMap[i_map] >= 0 && labelMask[regionMap[i_map]]) {
				int _x = (int) ((((double) x) + offset_x) * scale);
				if (_x + _y * mask_cols < 0 ||
					_x + _y * mask_cols >= mask_cols * mask_rows) {
						cerr << "RAGraph::setRegionMask: illegal index "
							<< (_x + _y * mask_cols)
							<< " in mask " << mask_cols << "x" << mask_rows << endl;
						exit(1);
				}
				mask[_x + _y * mask_cols] = value;
			}
		}
	}
	if (labelMask) free(labelMask);
}

void RAGraph::setRegionMask(int *mask, int mask_cols, int mask_rows,
							double scale, double offset_x, double offset_y,
							leda_node r, int value)
{
	RAG_Region *reg = (RAG_Region *) &(*this)[r];
	int n_labels;
	const int *labels = getSubLabels(r,n_labels);
	int max_label = getMaxLabel();
	char *labelMask = (char *) malloc((max_label+1) * sizeof(char));
	memset(labelMask,0,(max_label+1) * sizeof(char));
	int i;
	for (i=0; i < n_labels; i++) labelMask[labels[i]] = 1;

	const int *regionMap = getRegionMap();
	int cols = getCols();
	int rows = getRows();
	//int start_x = reg->get()->start_x;
	int start_y = reg->get()->start_y;
	int i_map = start_y * cols;

	int x,y;
	for (y=start_y; y < rows; y++) {
		int _y = (int) ((((double) y) + offset_y) * scale);
		for (x=0; x < cols; x++, i_map++) {
			if (labelMask[regionMap[i_map]]) {
				int _x = (int) ((((double) x) + offset_x) * scale);
				mask[_x + _y * mask_cols] = value;
			}
		}
	}
	if (labelMask) free(labelMask);
}

void RAGraph::writeRegionMask(leda_node r, const char *format=NULL)
{
	FILE *fp;
	char fname[256] = "";

	//RAG_Region *reg = (RAG_Region *) &(*this)[r];
	//region_info_t *info = reg->get();
	pgm_t pgm; 
	pnm_pgm_init(&pgm);

	pgm.cols = getCols();
	pgm.rows = getRows();
	pgm.maxval = 255;

	gray *pixmap = (gray *) malloc(pgm.cols*pgm.rows * sizeof(gray));
	pgm.pixels = (gray **)
		pnm_array2matrix(NULL,pgm.cols,pgm.rows,(void *) pixmap,sizeof(gray));
	pnm_pgm_clear(&pgm, 255);

	int i;
	for (i=0; i < pgm.rows; i++) {
		if (!pgm.pixels[i]) {
			cerr << "RAGraph::writeRegionMask: NULL in row " << i << endl;
			exit(1);
		}
	}

	setRegionMask((unsigned char *) pixmap, pgm.cols, pgm.rows, 
		1.0, 0, 0, r, 0);

	for (i=0; i < pgm.rows; i++) {
		if (!pgm.pixels[i]) {
			cerr << "RAGraph::writeRegionMask 2: NULL in row " << i << endl;
			exit(1);
		}
	}

	if (!pgm.pixels) return;

	if (format)
		sprintf(fname, format, (*this)[r].getLabel());
	else
		sprintf(fname, "regionMask_%d.png", (*this)[r].getLabel());

	fp = fopen(fname, "w");
	if (!fp) {
		cerr << "RAGraph::writeContours: cannot open " << fname
			<< " for writing." << endl;
		free(pgm.pixels[0]);
		free(pgm.pixels);
		return;
	}
	char *ending = strrchr(fname,'.');
	if (ending && !strcmp(ending,".png")) {
		pnm_pgm_writepng(fp, &pgm);
	} else {
		pgm_writepgm(fp, pgm.pixels, pgm.cols, pgm.rows, pgm.maxval, 1); 
	}
	free(pgm.pixels[0]);
	free(pgm.pixels);
	fclose(fp);
}


void RAGraph::writeContour(leda_node r, const char *format/*=NULL*/)
{
	FILE *fp;
	char fname[256] = "";

	RAG_Region *reg = (RAG_Region *) &(*this)[r];
	pgm_t pgm; 
	pnm_pgm_init(&pgm);

	reg->getContourPGM(&pgm);
	if (!pgm.pixels) return;

	if (format)
		sprintf(fname, format, (*this)[r].getLabel());
	else
		sprintf(fname, contour_format, (*this)[r].getLabel());

	fp = fopen(fname, "w");
	if (!fp) {
		cerr << "RAGraph::writeContours: cannot open " << fname
			<< " for writing." << endl;
		free(pgm.pixels[0]);
		free(pgm.pixels);
		return;
	}
	char *ending = strrchr(fname,'.');
	if (ending && !strcmp(ending,".png")) {
		pnm_pgm_writepng(fp, &pgm);
	} else {
		pgm_writepgm(fp, pgm.pixels, pgm.cols, pgm.rows, pgm.maxval, 1); 
	}
	free(pgm.pixels[0]);
	free(pgm.pixels);
	fclose(fp);
}

void RAGraph::writeContours(const char *format/*=NULL*/)
{
	leda_node r;
	forall_nodes(r,*this) {
		writeContour(r,format);
	}
}

unsigned char * RAGraph::setMergedColorMap(unsigned char *map)
{
	const rag_t *p_rag = (pRAG) ? pRAG->get() : NULL;
	if (!p_rag) return (map);
	if (!map) {
		map = (unsigned char *) calloc(getCols() * getRows(), sizeof(char));
	}
	//rag_fprint_colorTab(stderr, (rag_t *) p_rag);


	RAG_CompeteGraph CG(*this);
	CG.reduce();

	//pixel *tab = NULL;
	//int n_colors = rag_getColorTabPixels(&tab, (rag_t *) p_rag);

	leda_node cnode;
	forall_nodes(cnode,CG) {
		leda_node r = CG[cnode].getNode();
		int   color = (*this)[r].get()->color;
		//cerr << "RAGraph::setMergedColorMap: set region " << (*this)[r].getLabel()
		// << " with color " << color << ":"
		// << "(" << PPM_GETR(tab[color-1])
		// << "," << PPM_GETG(tab[color-1])
		// << "," << PPM_GETB(tab[color-1])
		// << ")" << endl;
		setRegionMask(map, getCols(), getRows(), 1.0, 0.0, 0.0, r, color);
	}

	//free(tab);
	return (map);
}

void RAGraph::writeMergedColorMap(const char *filename)
{
	const rag_t *p_rag = (pRAG) ? pRAG->get() : NULL;
	if (!p_rag) return;

	FILE *fp;
	ppm_t ppm;
	pnm_ppm_init(&ppm);
	unsigned char *map = NULL;
	pixel *tab = NULL;
	map = setMergedColorMap(map);
	int n_colors = rag_getColorTabPixels(&tab, (rag_t *) p_rag);

	//rag_fprint_colorTab(stderr,(rag_t *) p_rag);

	pnm_charMap2ppmTabColor(&ppm, map, getCols(), getRows(), n_colors, tab);

	fp = fopen(filename, "w");
	if (!fp) {
		cerr << "RAGraph::writeMergedColorMap: cannot open " << filename
			<< " for writing." << endl;
		free(tab);
		free(map);
		free(ppm.pixels[0]);
		free(ppm.pixels);
		return;
	}
	char *ending = strrchr(filename,'.');
	if (ending && !strcmp(ending,".png")) {
		pnm_ppm_writepng(fp, &ppm);
	} else {
		ppm_writeppm(fp, ppm.pixels, ppm.cols, ppm.rows, ppm.maxval, 1);
	}
	free(tab);
	free(map);
	free(ppm.pixels[0]);
	free(ppm.pixels);
	fclose(fp);
}

int RAGraph::readColorMap(const char *filename)
{
	FILE *fp;
	int n_regions=0;

	if (!(fp=fopen(filename,"r"))) {
		cerr << "RAGraph::readColorMap: cannot open file '" << filename << "'!"
			<< endl;
		return (0);
	}
	cerr << "RAGraph::readColorMap: reading file '" << filename << "'."
		<< endl;
	ppm_t ppm_map;
	pnm_ppm_init(&ppm_map);
	int status;
	if ((status=pnm_ppm_readpng(&ppm_map, fp)) == PNM_PNG_NOPNG) {
		ppm_map.pixels = ppm_readppm(fp, 
			&ppm_map.cols,
			&ppm_map.rows,
			&ppm_map.maxval);
		if (!ppm_map.pixels) {
			cerr << "RAGraph::readColorMap: error while reading ppm image from '"
				<< filename << "'!" << endl;
			return(0);
		}
	} else if (status != PNM_PNG_OK) {
		cerr << "RAGraph::readColorMap: error while reading png image from '"
			<< filename << "'!" << endl;
		return(0);
	}
	if (fp) fclose(fp);

	if (skip_colorMap) {
		int *regionMap = (int *)
			malloc(ppm_map.cols*ppm_map.rows * sizeof(int));
		n_regions = pnm_ppmPseudoColor2intMap(regionMap, &ppm_map);
		rag_setRegionMap((rag_t *) pRAG->get(), regionMap, -1, ppm_map.cols,
			ppm_map.rows);

	} else {
		unsigned char *colorMap = (unsigned char *)
			malloc(ppm_map.cols*ppm_map.rows * sizeof(unsigned char));
		pixel *colors = NULL;
		int n_colors = pnm_ppmSegColor2charMap(colorMap, &ppm_map, &colors);
		rag_setColorMap((rag_t *) pRAG->get(), colorMap, -1, ppm_map.cols,
			ppm_map.rows);
		rag_setColorTabByPixel((rag_t *) pRAG->get(), colors, n_colors);

		n_regions = rag_computeRegionMap((rag_t *) pRAG->get());
		rag_setColorMap((rag_t *) pRAG->get(), NULL,0, ppm_map.cols, ppm_map.rows);
		if (colors) free(colors);
		free(colorMap);
	}
	pnm_ppm_free(&ppm_map);

	return (n_regions);
}

RAGraph& RAGraph::updateAdjacencies(void)
{
	leda_edge e;
	forall_edges(e,*this) {
		leda_node src = source(e);
		leda_node tar = target(e);

		RAG_Adjacency *adj = (RAG_Adjacency *) &inf(e);
		region_info_t *reg1 = inf(src).get();
		region_info_t *reg2 = inf(tar).get();
		adj->initRegions(reg1, reg2);
	}
	return (*this);
}

RAGraph& RAGraph::updateRAG(RAG *_pRAG/*=NULL*/)
{
	rag_t *rag = (rag_t *)((_pRAG) ? _pRAG->get() : (pRAG) ? pRAG->get() : NULL);
	if (!rag) return (*this);

	if (_pRAG) pRAG = _pRAG;

	leda_node r;
	forall_nodes(r,*this) {
		region_info_t *reg = (region_info_t *) (*this)[r].get();
		if (!reg) continue;

		region_info_t *new_reg = rag_getRegion4Label(rag,reg->label,NULL);
		if (reg != new_reg) {
			region_info_cpy(new_reg, reg);
			(*this)[r].set((region_info_t *) NULL);
			(*this)[r].RAG_RefRegion::set(rag, new_reg->label, getImage());
		}
	}
	rag_blockRegionMapLabels(rag);

	updateAdjacencies();

	leda_edge e;
	forall_edges(e,*this) {
		region_adjacency_t *adj = (region_adjacency_t *) (*this)[e].get();

		/*** adj-info nach rag verschieben */
		region_adjacency_t *new_adj = rag_getAdjacency4Label(rag,adj->label,NULL);
		if (adj != new_adj) {
			region_adjacency_cpy(new_adj,adj);
			(*this)[e].set((region_adjacency_t *) NULL);
			(*this)[e].RAG_RefAdjacency::set(rag, new_adj->label);
		}
	}
	return (*this);
}    

leda_node RAGraph::merge_OLD(const leda_node& r1, const leda_node& r2)
{
	int labelM, indexM;
	region_info_t *info1 = (*this)[r1].get();
	region_info_t *info2 = (*this)[r2].get();
	region_info_t *infoM;
	rag_t *rag = (rag_t *) ((pRAG) ? pRAG->get() : NULL);

	if (!info1 || !info2 || !rag) return (nil);

	if (!areDisjunct(r1,r2)) return (nil);

	if (!(infoM = rag_newRegion(rag, &indexM))) return (nil);

	info1 = (*this)[r1].get();
	info2 = (*this)[r2].get();

	labelM = infoM->label;

	rag_realloc(rag,0,rag->n_adjacency_index+2*rag->n_regions);
	region_adjacency_t **
		adjacenciesM = rag_newTmpAdjacencies(rag,0,labelM);
	region_adjacency_t **
		rev_adjacenciesM = rag_newTmpRevAdjacencies(rag,1,labelM);

	// ** get current adjacencies of nodes to merge
	region_adjacency_t **adjacencies1 = getTmpAdjacencyMem(2);
	region_adjacency_t **adjacencies2 = getTmpAdjacencyMem(3);
	int                n_adjacencies  = getMaxLabel() + 1;

	adjacencies1 = getAdjacencies(adjacencies1,n_adjacencies,r1,r2);
	adjacencies2 = getAdjacencies(adjacencies2,n_adjacencies,r2,r1);

	//cerr << "call rag_computeMerge ..." << endl;
	rag_computeMergeOutEdges(infoM,adjacenciesM,rev_adjacenciesM,
		rag, info1->label, info2->label,
		adjacencies1, adjacencies2);
	if (infoM->n_contour <= 0) {
		rag_delLastRegion(rag);
		return (nil);
	}

	adjacencies1 = getRevAdjacencies(adjacencies1,n_adjacencies,r1,r2);
	adjacencies2 = getRevAdjacencies(adjacencies2,n_adjacencies,r2,r1);

	rag_computeMergeInEdges(adjacenciesM, rev_adjacenciesM,
		rag, labelM, info1->label, info2->label,
		adjacencies1);

	rag_computeMergeInEdges(adjacenciesM, rev_adjacenciesM,
		rag, labelM, info2->label, info1->label,
		adjacencies2);

	rag_clearUnusedAdjacencies(rag, adjacenciesM, rag->n_regions);
	rag_clearUnusedAdjacencies(rag, rev_adjacenciesM, rag->n_regions);

	//cerr << "add node " << labelM << "." << endl;
	leda_node rM = new_node(RAG_Region(rag, labelM, getImage()));

	/** create graph edges */
	leda_node s;
	forall_nodes(s,*this) {
		int index_s = (*this)[s].getLabel();
		region_adjacency_t *adj, *rev_adj;
		if (index_s == labelM) continue;

		adj = adjacenciesM[index_s];
		rev_adj = rev_adjacenciesM[index_s];

		if (region_isAdjacent(adj)) {
			leda_edge e = new_edge(rM, s, RAG_Adjacency(rag,adj->label));

			if (region_isAdjacent(rev_adj)) {
				leda_edge f = new_edge(s, rM, RAG_Adjacency(rag,rev_adj->label));

				set_reversal(e,f);
			}
		} else if (region_isAdjacent(rev_adj)) {
			new_edge(s, rM, RAG_Adjacency(rag,rev_adj->label));
			//cout << "EDGE from " << index_s << endl;
		}
	}

	return rM;
}

leda_node RAGraph::merge(const leda_node& r1, const leda_node& r2)
{
	//cerr << "RAGraph::merge" << endl;
	int labelM, indexM;
	region_info_t *info1 = (*this)[r1].get();
	region_info_t *info2 = (*this)[r2].get();
	region_info_t *infoM;
	rag_t *rag = (rag_t *) ((pRAG) ? pRAG->get() : NULL);

	rag_consistent(rag,"MERGE start");

	if (!info1 || !info2 || !rag) return (nil);

	if (!areDisjunct(r1,r2)) return (nil);

	if (!(infoM = rag_newRegion(rag, &indexM))) return (nil);

	rag_consistent(rag,"MERGE newRegion");

	info1 = (*this)[r1].get();
	info2 = (*this)[r2].get();

	labelM = infoM->label;

	if (info1->label == labelM || info2->label == labelM) {
		fprintf(stderr,"ILLEGAL MERGE!\n");
		exit(1);
	}

	rag_realloc(rag,0,rag->n_adjacency_index+2*rag->n_regions);
	consistent("MERGE rag_realloc");
	rag_consistent(rag,"MERGE rag_realloc");

	region_adjacency_t **
		adjacenciesM = rag_newTmpAdjacencies(rag,0,labelM);

	consistent("MERGE newTmpAdjacencies");
	rag_consistent(rag,"MERGE newTmpAdjacencies");

	region_adjacency_t **
		rev_adjacenciesM = rag_newTmpRevAdjacencies(rag,1,labelM);

	rag_consistent(rag,"MERGE newTmpMem");

	// ** get current adjacencies of nodes to merge
	region_adjacency_t **adjacencies1 = getTmpAdjacencyMem(2);
	region_adjacency_t **adjacencies2 = getTmpAdjacencyMem(3);
	int                n_adjacencies  = getMaxLabel() + 1;

	adjacencies1 = getAdjacencies(adjacencies1,n_adjacencies,r1,r2);
	adjacencies2 = getAdjacencies(adjacencies2,n_adjacencies,r2,r1);

	rag_consistent(rag,"MERGE");

	region_info_t **regions = rag_getTmpRegions(rag,0);

	if (!rag->contourIndexMap)
		rag_computeContourIndexMap(rag);

	//cerr << "call rag_computeMerge ..." << endl;

	region_mergeOutEdges2(infoM,
		regions, rag->n_regions,
		adjacenciesM,
		adjacencies1[info2->label],
		adjacencies2[info1->label],
		labelM, labelM,
		NULL, NULL,
		rag->regionMap,
		rag->contourIndexMap,
		rag->cols, rag->rows);

	if (infoM->n_contour <= 0) {
		rag_delLastRegion(rag);
		return (nil);
	}

	adjacencies1 = getRevAdjacencies(adjacencies1,n_adjacencies,r1,r2);
	adjacencies2 = getRevAdjacencies(adjacencies2,n_adjacencies,r2,r1);

	region_mergeInEdges2(rev_adjacenciesM,
		regions, rag->n_regions, 
		labelM, info1->label, info2->label,
		adjacencies1);

	region_mergeInEdges2(rev_adjacenciesM,
		regions, rag->n_regions,
		labelM, info2->label, info1->label,
		adjacencies2);

	rag_clearUnusedAdjacencies(rag, adjacenciesM, rag->n_regions);
	rag_clearUnusedAdjacencies(rag, rev_adjacenciesM, rag->n_regions);

	rag_setMergeColor(infoM,rag,info1,info2);

	//cerr << "add node " << labelM << "." << endl;
	leda_node rM = new_node(RAG_Region(rag, labelM, getImage()));

	/** create graph edges */
	leda_node s;
	forall_nodes(s,*this) {
		int index_s = (*this)[s].getLabel();
		region_adjacency_t *adj, *rev_adj;
		if (index_s == labelM) continue;

		adj = adjacenciesM[index_s];
		rev_adj = rev_adjacenciesM[index_s];

		if (region_isAdjacent(adj)) {
			RAG_Adjacency radj(rag,adj->label);
			leda_edge e = new_edge(rM, s, radj);

			// leda_edge e = new_edge(rM, s, RAG_Adjacency(rag,adj->label));

			if (region_isAdjacent(rev_adj)) {
				RAG_Adjacency rev_radj(rag,rev_adj->label);
				leda_edge f = new_edge(s, rM, rev_radj);
				//leda_edge f = new_edge(s, rM, RAG_Adjacency(rag,rev_adj->label));

				set_reversal(e,f);
			}
		} else if (region_isAdjacent(rev_adj)) {
			RAG_Adjacency rev_radj(rag,rev_adj->label);
			/*leda_edge f = */new_edge(s, rM, rev_radj);

			//new_edge(s, rM, RAG_Adjacency(rag,rev_adj->label));

			//cout << "EDGE from " << index_s << endl;
		}
	}

	return rM;
}

region_adjacency_t **RAGraph::getTmpAdjacencyMem(int tmp_index) const
{
	rag_t *rag = (pRAG) ? (rag_t *) pRAG->get() : NULL;
	if (!rag) return (NULL);

	region_adjacency_t **adjs
		= rag_reallocTmpAdjacencies(rag, tmp_index, rag->n_regions);
	memset(adjs,0,rag->n_regions * sizeof(region_adjacency_t *));

	return (adjs);
}

region_adjacency_t **
RAGraph::getAdjacencies(region_adjacency_t **adjs, int n_adjs,
						leda_node r,
						leda_node s/*=nil*/) const
{
	if (!adjs)
		adjs = (region_adjacency_t **)malloc(n_adjs*sizeof(region_adjacency_t *));

	memset(adjs,0,n_adjs * sizeof(region_adjacency_t *));

	leda_edge e;
	forall_out_edges(e,r) {
		leda_node t = target(e);
		int _disjunct = areDisjunct(s,t);

		//cerr << (*this)[t].getLabel() << "-" << (*this)[s].getLabel() << ":"
		//	 << _disjunct << endl;
		if (s != t && s != nil && !_disjunct) 
			continue;

		int label = (*this)[t].getLabel();
		if (label >= 0 && label < n_adjs)
			adjs[label] = (region_adjacency_t *) (*this)[e].get();
	}
	return (adjs);
}

region_adjacency_t **
RAGraph::getRevAdjacencies(region_adjacency_t **adjs, int n_adjs,
						   leda_node r,
						   leda_node t/*=nil*/) const
{
	if (!adjs)
		adjs = (region_adjacency_t **)malloc(n_adjs*sizeof(region_adjacency_t *));

	memset(adjs,0,n_adjs * sizeof(region_adjacency_t *));

	//int r_index = (*this)[r].getLabel();

	leda_edge e;
	forall_in_edges(e,r) {
		leda_node s = source(e);
		if (s != t &&
			t != nil
			&& !areDisjunct(s,t)) 
			continue;

		int label = (*this)[s].getLabel();
		if (label >= 0 && label < n_adjs)
			adjs[label] = (region_adjacency_t *) (*this)[e].get();
	}
	return (adjs);
}

void RAGraph::_computeMerges(int depth, leda_node r, int max_index)
{
	if (depth < 1) return;

	leda_list<leda_edge> es = adj_edges(r);
	leda_edge e;
	forall(e,es) {
		if (reversal(e) == nil) continue;

		int s_index = (*this)[opposite(r,e)].getLabel();
		if (s_index >= max_index) continue;

		cout << " " << s_index << "[" << max_index << "]";

		leda_node rM = merge(source(e),target(e));
		_computeMerges(depth - 1, rM, s_index);

		cout << endl << "...";
	}
}

void RAGraph::computeMerges(int depth)
{
	if (depth < 1) return;

	leda_list<leda_edge> es = all_edges();

	leda_edge e;
	forall(e,es) {
		if (reversal(e) == nil) continue;

		int s_index = (*this)[source(e)].getLabel();
		int t_index = (*this)[target(e)].getLabel();

		if (s_index <= t_index) continue;

		cout << (*this)[source(e)].getLabel()
			<< " " 
			<< (*this)[target(e)].getLabel();

		leda_node rM = merge(source(e),target(e));
		_computeMerges(depth - 1, rM, t_index);
		//del_node(rM);

		cout << endl;
	}
}

int RAGraph::getMaxLabel(void) const 
{ 
	if (pRAG && pRAG->get()) 
		return (pRAG->get()->n_regions - 1);

	int max_label = -1;
	leda_node r;
	forall_nodes(r,*this) {
		int label = (*this)[r].getLabel();
		if (label > max_label)
			max_label = label;
	}
	return (max_label);
}

RAGraph& RAGraph::hide_merges()
{
	leda_node r;
	forall_nodes(r,*this) {
		int n_sublabels;
		(*this)[r].getSubLabels(n_sublabels);
		if (n_sublabels > 1) {
			hide_node(r);
		}
	}
	return (*this);
}

RAGraph& RAGraph::restore_merges()
{
	// ** assumes that all hidden nodes are merge-nodes
	restore_all_nodes();
	restore_all_edges();

	return (*this);
}

int RAGraph::consistent(const char *text/*=""*/)
{
	const rag_t *rag = get()->get();

	leda_node r;
	forall_nodes(r,*this) {
		const region_info_t *r_reg = (*this)[r].get();
		if (r_reg->pixelcount == 0) {
			cerr << "RAGraph::consistent:" << text << ": ILLEGAL NULL-REGION "
				<< r_reg->label << endl;
			exit(1);
		}
	}
	leda_edge e;
	forall_edges(e,*this) {
		leda_node s = source(e);
		leda_node t = target(e);
		const region_adjacency_t *adj = (*this)[e].get();
		const region_info_t *s_reg = (*this)[s].get();
		const region_info_t *t_reg = (*this)[t].get();

		int k;
		for (k=0; k < rag->n_adjacency_index; k++) {
			if (rag->adjacency_index[k] == adj->label) break;
		}
		if (k >= rag->n_adjacency_index) {
			cerr << "RAGraph::consistent:" << text << ": ILLEGAL ADJINDEX "
				<< adj->label << endl;
			for (k=0; k < rag->n_adjacency_index; k++) {
				cerr << " " << rag->adjacency_index[k];
			}
			cerr << endl << "RAGraph:" ;
			leda_edge f;
			forall_edges(f,*this) {
				cerr << " " << (*this)[f].get()->label;
			}
			cerr << endl;
			exit(1);
		}

		if (adj->region1 != s_reg || adj->region2 != t_reg) {
			cerr << "RAGraph::consistent:" << text << ": ILLEGAL ADJACENCY ("
				<< s_reg->label << "-" << t_reg->label << ") "
				<< adj->label << ":" 
				<< adj->region1->label << "-" << adj->region2->label
				<< "!" << endl;
			cerr << "rag->adjacency_index:";
			for (k=0; k < rag->n_adjacency_index; k++) {
				cerr << " " << rag->adjacency_index[k];
			}
			cerr << endl << "RAGraph:" ;
			leda_edge f;
			forall_edges(f,*this) {
				cerr << " " << (*this)[f].get()->label;
			}
			cerr << endl;
			exit(1);
		}
	}
	return (1);
}

void RAGraph::write(ostream& O/*=cout*/)
{
	O << getCols() << " " << getRows() << endl;
	leda::GRAPH<RAG_Region,RAG_Adjacency>::write(O);
}

int RAGraph::read(istream& I/*=cin*/)
{
	int result;
	rag_t *rag = (rag_t *) get()->get();
	I >> rag->cols >> rag->rows;
	result = leda::GRAPH<RAG_Region,RAG_Adjacency>::read(I);
	if (result == 0) updateRAG();
	return (result);
}

ostream& operator<<(ostream& s, RAGraph& gRAG)
{
	gRAG.write(s);
	return (s);
}

istream& operator>>(istream& s, RAGraph& gRAG)
{
	gRAG.read(s);
	return (s);
}



