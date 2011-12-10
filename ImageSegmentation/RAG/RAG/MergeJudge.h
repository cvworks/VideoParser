/*
 * MergeJudge.h
 *
 * class for judging merges in a RAG
 *
 * Sven Wachsmuth, 11.11.2003
 */
#ifndef RAG_MERGEJUDGE_H
#define RAG_MERGEJUDGE_H

#include "MergeIt.h"

class RAG_MergeJudge {

 public:
    RAG_MergeJudge() {};
    virtual ~RAG_MergeJudge() {};

    virtual int valid(MergeIt& iterator, MergeIt::NodeList *p_merge)
	{ return (1); }
    virtual int valid(MergeIt& iterator, MergeIt::NodeList *p_merge,
		      float &score)
	{ return (1); }
    virtual int post_processing()=0;
};

#endif
