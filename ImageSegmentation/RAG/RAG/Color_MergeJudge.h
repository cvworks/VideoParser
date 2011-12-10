/*
 * ColorMergeJudge.h
 *
 * class for judging merges in a RAG by RGB-value
 *
 * Sven Wachsmuth, 11.11.2003
 */
#ifndef RAG_COLOR_MERGEJUDGE_H
#define RAG_COLOR_MERGEJUDGE_H

#include "MergeJudge.h"
#include "RAGraph.h"

class RAG_ColorMergeJudge: public RAG_MergeJudge {

 public:
    static float threshold;
    static float max_val;
    static float white_bg;
    static float black_bg;
    static int   verbose_mode;

    static float color_dist(rag_color_t *rgb1, rag_color_t *rgb2);

    RAG_ColorMergeJudge() {};
    virtual ~RAG_ColorMergeJudge() {};

    virtual int valid(MergeIt& iterator, MergeIt::NodeList *p_merge);
    virtual int valid(MergeIt& iterator, MergeIt::NodeList *p_merge,
		      float &score);
    virtual int post_processing()
	{ return (0); }
};

#endif
