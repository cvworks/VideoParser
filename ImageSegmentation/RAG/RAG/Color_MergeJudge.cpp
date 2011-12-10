/*
 * ColorMergeJudge.C
 *
 * class for judging merges in a RAG by RGB-value
 *
 * Sven Wachsmuth, 12.11.2003
 */
#include "Color_MergeJudge.h"

using namespace std;
using namespace leda;

float RAG_ColorMergeJudge::threshold = 0.25;
float RAG_ColorMergeJudge::max_val = 255.0;
float RAG_ColorMergeJudge::white_bg = 250.0;
float RAG_ColorMergeJudge::black_bg = 0.0;

int RAG_ColorMergeJudge::verbose_mode = 0;

float RAG_ColorMergeJudge::color_dist(rag_color_t *rgb1, rag_color_t *rgb2)
{
  float bg1 = (rgb1->r + rgb1->g + rgb1->b) / 3.0;
  float bg2 = (rgb2->r + rgb2->g + rgb2->b) / 3.0;
  if (bg1 > white_bg || bg2 > white_bg ||
      bg1 < black_bg || bg2 < black_bg)
      return (1.0);

  float dr = (rgb1->r - rgb2->r) / max_val;
  float dg = (rgb1->g - rgb2->g) / max_val;
  float db = (rgb1->b - rgb2->b) / max_val;

  return (dr * dr + dg * dg + db * db) / 3.0;
}

    
int RAG_ColorMergeJudge::valid(MergeIt& iterator, MergeIt::NodeList *p_merge)
{
    float score=0.0;
    return valid(iterator,p_merge,score);
}

int RAG_ColorMergeJudge::valid(MergeIt& iterator, MergeIt::NodeList *p_merge,
			       float &score)
{
  if (!p_merge || p_merge->empty()) return (0);

  leda_node r = iterator.get_node(p_merge->head());
  RAGraph *pRAG = (RAGraph *) graph_of(r);
  rag_color_t *colorTab = pRAG->getColorTab();
  if (!colorTab) {
      cerr << "RAG_ColorMergeJudge::valid: no color tab" << endl; 
      return (0);
  }
  float max_dist = 0.0;
  list_item i,j;
  for(i=p_merge->first(); i!=nil; i=p_merge->succ(i)) {
      r = iterator.get_node((*p_merge)[i]);
      region_info_t *p_reg_i = (*pRAG)[r].get();
      
      for (j=p_merge->succ(i); j != nil; j=p_merge->succ(j)) {
	  r = iterator.get_node((*p_merge)[j]);
	  region_info_t *p_reg_j = (*pRAG)[r].get();

	  float dist = color_dist(&colorTab[p_reg_i->color-1],
				  &colorTab[p_reg_j->color-1]);
	  //cerr << "color dist: "
	  //   << p_reg_i->label << ":"
	  //   << colorTab[p_reg_i->color-1].r << ","
	  //   << colorTab[p_reg_i->color-1].g << ","
	  //   << colorTab[p_reg_i->color-1].b << " - "
	  //   << p_reg_j->label << ":"
	  //   << colorTab[p_reg_j->color-1].r << ","
	  //   << colorTab[p_reg_j->color-1].g << ","
	  //   << colorTab[p_reg_j->color-1].b << " = " << dist << endl;

	  if (dist > max_dist)
	      max_dist = dist;
      }
  }
  score = (threshold - max_dist) / threshold;

  if (RAG_ColorMergeJudge::verbose_mode > 0)
      cerr << "RAG_ColorMergeJudge::valid: " 
	   << max_dist << "[< " << threshold << "] " << score << endl;

  return (max_dist < threshold);
}
      



