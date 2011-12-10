#ifndef SKELETON_H
#define SKELETON_H


#include <stdio.h>
#include <stdlib.h>
#include <Tools/MathUtils.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <time.h>
#include "flags.h"
#include "field.h"
#include "genrl.h"
#include "io.h"
#include "mfmm.h"
#include "evol.h"
#include "distseq.h"
#include "thin.h"
#include "treeskel.h"
#include "darray.h"

#include <map>
#include <sstream>

#include "DDSGraph.h"
#include "DiscreteSegCurve.h"
#include "DistanceTransform.h"
#include "DivergenceMap.h"
#include "DivergenceSkeletonMaker.h"
#include "ShapeMaker.h"

#include "ShapeDiff.h"
#include "AFMMSkeleton.h"

struct terms
{
	int number_branches;
	float likelihood_term;
};

struct TauBrnchErrorIndices
{
	int nBrnch;
	double dError;
	double dTau;
	DARRAY< int > indices;
};     


struct LengthBrnchError
{
	int nBrnch;
	double dError;
	double dLength;
};   
	

struct BrnchIndexError
{
	int nIndex;
	double dError;
	bool operator<(const BrnchIndexError& rhs) const
	{
		return (dError < rhs.dError);
	}
};  

class Skeleton
{
	public:
		SkelCompParams params;
		double dMinSlope;
		double dMinError;
		double dMaxAccelChg;
		Skeleton();
		~Skeleton();
		std::vector<TreeSkel*> determine_skeletons(SkelCompParams);
		AFMMSkeleton* get_DDSkeleton(SkelCompParams);
		
		void GetDTByInflation(FIELD<float>* grad2, FIELD<float>* dt_b_b, FLAGS*& flags_s, FIELD<float>*& dt_s);
		
		void GetDTByInflation(FIELD<float>* grad2, FIELD<float>* dt_b_b, FLAGS*& flags_s);
		
		double GetReconstructionError(FIELD<float>* grad2, FIELD<float>* dt_b_b, ShapeDiff& sd);
	
	private:
		
		void thickenObject(FIELD<float>* pObjectField);
		
		bool hasObjNbr(FIELD<float>* pObjectField, int x ,int y);
	
		bool ThickeningRequired(FIELD<float>* pObjectField);
		
		bool OnePixelBridge(FIELD<float>* pObjectField, int x ,int y);
		
		bool TwoPixelBridge(FIELD<float>* pObjectField, int x ,int y);
		
		FIELD<float>* do_one_pass(FIELD<float>*,FLAGS*,FIELD<std::multimap<float,int> >*,int,ModifiedFastMarchingMethod::METHOD,float&,int&,FIELD<float>*&);
		FIELD<float>* inflate_obj(FIELD<float>*,FLAGS*,int,FIELD<float>*,float);
		void comp_grad(FLAGS*,FIELD<float>*,FIELD<float>*);
		void comp_grad2(FLAGS*,FIELD<float>*,FIELD<float>*,FIELD<float>*);
		void postprocess(FIELD<float>*,FIELD<float>*,float);
		void postprocess2(FIELD<float>*,float);
		void likelihood_term_ed(FLAGS* obj1, FIELD<float>* dt1, FLAGS* obj2, FIELD<float>* dt2, float&);
		void calculate_errors(float, float, FLAGS*,FLAGS*,FIELD<float>*,FIELD<float>*,FIELD<float>*,FIELD<float>*, FIELD<float>*,int,int,int);
		
		
		std::pair< DARRAY< int >, double > 
				calculate_errors_new_boundary(float, FLAGS*,
					FLAGS*,FIELD<float>*,FIELD<float>*,FIELD<float>*,
					FIELD<float>*, FIELD<float>*,int,int,int);
		
		
		void write_error_terms(int& sk_lev, int& sh_len, char*);
		void write_error_terms_boundary(int& sk_lev, int& sh_len, float& sk_lev_upperbound, float& sk_lev_stepsize);
		void write_error_terms_structural(int& sk_lev, int& sh_len, float& sh_len_upperbound, float& sh_len_stepsize);
		void write_error_terms(int& sk_lev, int& sh_len, float& sk_lev_upperbound, float& sk_lev_stepsize, float& sh_len_upperbound, float& sh_len_stepsize);
		std::pair<unsigned int,unsigned int> get_global_minimum(int&, float&, float&, int&, int);
		void get_local_minima(int&,float&,float&,int&);
		std::vector<std::pair<unsigned int,unsigned int> > get_all_local_minima(int&,int&,float&,float&,float&,float&);
		TreeSkel* get_treeskel_pointer(float, float, FLAGS*,FIELD<float>*,FIELD<float>*,FIELD<float>*, FIELD<float>*,int,int,int);
		
		double getOptimumStructural(DARRAY< LengthBrnchError > CompleteDimension2);
		TauBrnchErrorIndices getOptimumBoundary(DARRAY< TauBrnchErrorIndices > CompleteDimension);
		
		TreeSkel* get_new_treeskel_pointer(DARRAY< int > indices, float do_sk_lev, float do_sh_len, 
				FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,FIELD<float>* b_grad, FIELD<float>* b_f,int i,int j,int N);
		
			
		void setLineToZero(FIELD<float>* pField, TreeSkel::TreePoint* prev, TreeSkel::TreePoint* p);
		void setBranchPointsToZero(TreeSkel::TreeBranch* pTBRemoved, FIELD<float>* pField);
		
		AFMMSkeleton* 
// 		pDDSandDimensions
			get_DDSl_pointer(float do_sk_lev, float do_sh_len,
						 FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,
						 FIELD<float>* b_grad, FIELD<float>* b_f,int N, int nBndrySmoothIter);
};
   


#endif


