#include <stdio.h>
#include <stdlib.h>
#include <Tools/MathUtils.h>
#include <string>
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
#include <map>
#include <sstream>
#include "skeleton.h"

#include <Tools/connected.h>
#include <Tools/BasicUtils.h>


#define SKEL_TAU  20
#define APROX_ERROR_FIX 0.00025
// #define STRUCT_UPPER_BOUND 101
// #define SCALING_BRANCH_PARAM_BND 1
// #define SCALING_ERROR_PARAM_BND 350
// #define SCALING_BRANCH_PARAM_STR 1
// #define SCALING_ERROR_PARAM_STR 400

typedef std::vector<int> IntVector;

extern DARRAY<Coord> from; //!!hack, should make api for this


enum METHOD {
	COMPARISON,                                   // - compute all DTs with all methods for comparison purposes 
	EUCLIDEAN_VDT,                                // - VDT based on Euclidean distance propagation (the conjuncture)
	AFMM_VDT,                     // - same as AFMM_STAR, but computes complete VDT (origins too, thus)
	AFMM_STAR,                    // - by finding boundary-point the propagaton came from (best)
	AVERAGING,                    // - by averaging the counts of already computed neighbours (suboptimal)
	ONE_POINT
};


//----------------------------------------------------------------


std::map<std::pair<unsigned int, unsigned int>, terms> error_terms;
std::pair<unsigned int, unsigned int> simplif_pair;
terms current_terms;
float         length;
float         sk_lev;
FIELD<float> *skel,*grad;
//FIELD<float>* fields[20];
int           f_display;
int           n_fields;
bool          draw_color = true;

template <class T>
void  writePPM(T* t,char* f,int w) { if (w) t->writePPM(f);  }

char* method_name[] = { "Comparison", "Euclidean VDT", "AFMM Star VDT", "AFMM Star", "AFMM", "Obsolete" };
char* field_name[20];


inline float distance(float cd)
//Computes distance along boundary in a wrap-around fashion...
{ 
	return min(fabs(cd),fabs(length-fabs(cd))); 
}


bool AreTherePhantoms(TreeSkel* tsk)
{
	bool bPhantom = false;
	for(int i = 0; i < tsk->branches.Count(); i++)
	{
		if(tsk->branches[i]->geomLength() == 0)
			bPhantom = true;
	}
	return bPhantom;
}



Skeleton::Skeleton()
{}

Skeleton::~Skeleton()
{}



AFMMSkeleton* Skeleton::get_DDSkeleton(SkelCompParams InputParams)
{
	float k = -1;                      //Threshold for clusters (def)
	float sh_len = InputParams.fThresSimp2;
	sk_lev  = InputParams.fThresSimp1; //10;                      //Skeleton threshold
	const int   N  = 10000000;             //Max # fast marching method iterations
	dMinSlope = InputParams.dMinSlope;
	dMinError = InputParams.dMinError;
	dMaxAccelChg = InputParams.dMaxAccelChg;
	METHOD meth   = AFMM_STAR;                   //CHANGE standard AFMM_STAR will be used    Skeletonization/DT method used

	FIELD<float>* f = NULL;
	params = InputParams;    // save params for later

	if (InputParams.pInputField == NULL)
	{
		//char  inp[500]; 
		//strcpy(inp, InputParams.pInputfile);

		f = FIELD<float>::read((char*)InputParams.pInputfile);   //Read scalar field input

		if (!f) 
		{ 
			ShowError1("Cannot open file: ", InputParams.pInputfile);
			return NULL;
		}
	}
	else 
	{
		f = InputParams.pInputField;
	}

	//Preprocess field, create an extra layer around the object (thicken it) since the AFMM doesn't handle 1 or 2 pixel thin object parts
	//This particular thicken search only adresses the problem found in the umbrella folder

	// 	if(ThickeningRequired(f))
	// 	{
	// 		thickenObject(f);
	// DBG_PRINT1("\nATTENTION: The object has been thickened in order for the skeletonization to work.\n");
	// 	}

	FIELD<float> *cnt0=0; FIELD<std::multimap<float,int> >* origs=0; 
	int iter;

	FLAGS*   flags_b = new FLAGS(*f, k);
	FLAGS*   flags   = new FLAGS(*flags_b); // copy constructor, operator=() in this case
	FIELD<float>*   dt_b = 0;

	ModifiedFastMarchingMethod::METHOD m = (meth==AFMM_STAR)?  ModifiedFastMarchingMethod::AFMM_STAR :
		(meth==AFMM_VDT)?   ModifiedFastMarchingMethod::AFMM_VDT  :
		(meth==COMPARISON)? ModifiedFastMarchingMethod::AFMM_VDT  :
		(meth==AVERAGING)?  ModifiedFastMarchingMethod::AVERAGING : 
		ModifiedFastMarchingMethod::ONE_POINT; 

	grad       = new FIELD<float>(f->dimX(),f->dimY());
	skel       = new FIELD<float>(f->dimX(),f->dimY());

	// Original line 19/Oct/2006 ...
	////cnt0       = do_one_pass(f,flags,origs,0,m,length,iter,dt_b);
	// ... replaced by:

	// Begin new lines 19/Oct/2006 Diego
	cnt0 = new FIELD<float>;
	const int dir = 0;
	dt_b = new FIELD<float>(*f);

	ModifiedFastMarchingMethod fmm(dt_b, flags, cnt0, origs, N);
	fmm.setScanDir(dir);
	fmm.setMethod(m);
	fmm.setStopCriterium(INFINITY);
	fmm.setStatisticsFreq(-1);

	int nfail,nextr;               
	iter   = fmm.execute(nfail, nextr); // do the skeletonization
	length = fmm.getLength();
	// End new lines

	AFMMSkeleton* pDDS = get_DDSl_pointer(sh_len/*SKEL_TAU*/,0,flags,dt_b,cnt0,grad,f,N, 
		InputParams.nBndrySmoothIter);

	pDDS->SetOriginalAFMMFlags(flags);
	pDDS->SetDistanceTransformMap(dt_b);

	// 	comp_grad(flags,cnt0,grad);  
	// 	pDDS->SetOriginalSkelField(grad);
	pDDS->SetRecErrorWeightStr(params.nRecErrorWeightStr);
	pDDS->SetRecErrorWeightBnd(params.nRecErrorWeightBnd);

	//// Begin new lines 19/Oct/2006 Diego
	//// Get the boundary points associated with each skeletal point
	//	sg::DDSEdgeVect& edges = pDDS->getEdges();
	//	sg::DDSEdgeVect::iterator I;
	//	Coord ptCoord;
	//	unsigned int i;
	//
	//	for(I = edges.begin(); I != edges.end(); I++)
	//	{
	//		const sg::FluxPointArray& fpl = (*I)->getFluxPoints();
	//		sg::BoundaryInfoArray& bil = (*I)->getBoundaryInfoArray();
	//		bil.resize(fpl.size());
	//		for (i = 0; i < bil.size(); i++)
	//		{
	//			ptCoord = fmm.getPointOrigin(fpl[i].p.x, fpl[i].p.y);
	//			sg::BoundaryPoint& bp = bil[i].first;
	//
	//			bp.pt.x = ptCoord.i;
	//			bp.pt.y = ptCoord.j;
	//			bp.index = (bp.pt.x >= 0 && bp.pt.y >= 0) ? -2:-1;
	//		}
	//	}
	//// End new lines 19/Oct/2006

	delete f;
	delete cnt0;
	delete origs;
	delete flags_b;
	//delete flags;	Will be deleted by AFMMSkeleton
	//delete dt_b;	Will be deleted by AFMMSkeleton
	//delete grad;	Will be deleted by AFMMSkeleton
	delete skel;

	return pDDS /*pDDSandDim*/;;
}


bool Skeleton::hasObjNbr(FIELD<float>* pObjectField, int x ,int y)
{
	for(int xx = x - 1; xx <= x + 1; xx++){
		for(int yy = y - 1; yy <= y + 1; yy++){
			if((x != xx || y != yy) && pObjectField->value(xx,yy) == 0){
				return true;
			}
		}
	}

	return false;
}


void Skeleton::thickenObject(FIELD<float>* pObjectField)
{
	DARRAY< Coord > layer;
	Coord location;
	//do one pass to collect all the background pixels that need to become object pixels
	for(int x = 0; x < pObjectField->dimX(); x++)
		for(int y = 0; y < pObjectField->dimY(); y++)
		{
			if(pObjectField->value(x,y) == 255 && hasObjNbr(pObjectField,x,y))
			{
				location.i = x;
				location.j = y;
				layer.Add(location);
			}
		}

		//mark the collected pixels as object pixels
		for(int i = 0; i < layer.Count(); i++)
			pObjectField->value(layer[i].i,layer[i].j) = 0;
}

bool Skeleton::TwoPixelBridge(FIELD<float>* pObjectField, int x ,int y)
{
	/*
	|?|?|?|?|     <-- at least 1 object pixel
	| |X|X| |     <-- empty besides middle pixel
	|?|?|?|?|     <-- at least 1 object pixel

	or the rotated version of this schema
	*/

	int nCount1 = 0, nCount2 = 0;

	if(pObjectField->value(x+1,y) == 0 &&
		pObjectField->value(x+2,y) == 255 &&
		pObjectField->value(x-1,y) == 255)
	{
		nCount1 = 0;
		nCount2 = 0;

		nCount1 = (pObjectField->value(x-1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x+1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x+2,y+1) == 0)? nCount1 + 1: nCount1;

		nCount2 = (pObjectField->value(x-1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+2,y-1) == 0)? nCount2 + 1: nCount2;

		if(nCount1 >= 2 && nCount2 >= 2)
			return true;
	}

	//other side
	if(pObjectField->value(x-1,y) == 0 &&
		pObjectField->value(x+1,y) == 255 &&
		pObjectField->value(x-2,y) == 255)
	{
		nCount1 = 0;
		nCount2 = 0;

		nCount1 = (pObjectField->value(x-1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x+1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-2,y+1) == 0)? nCount1 + 1: nCount1;

		nCount2 = (pObjectField->value(x-1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x-2,y-1) == 0)? nCount2 + 1: nCount2;

		if(nCount1 >= 2 && nCount2 >= 2)
			return true;
	}

	//rotated of first
	if(pObjectField->value(x,y+1) == 0 &&
		pObjectField->value(x,y-1) == 255 &&
		pObjectField->value(x,y+2) == 255)
	{
		nCount1 = 0;
		nCount2 = 0;

		nCount1 = (pObjectField->value(x-1,y) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y-1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y+2) == 0)? nCount1 + 1: nCount1;

		nCount2 = (pObjectField->value(x+1,y) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y+1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y+2) == 0)? nCount2 + 1: nCount2;

		if(nCount1 >= 2 && nCount2 >= 2)
			return true;
	}

	//other side of above
	if(pObjectField->value(x,y-1) == 0 &&
		pObjectField->value(x,y+1) == 255 &&
		pObjectField->value(x,y-2) == 255)
	{
		nCount1 = 0;
		nCount2 = 0;

		nCount1 = (pObjectField->value(x-1,y) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y-1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y+1) == 0)? nCount1 + 1: nCount1;
		nCount1 = (pObjectField->value(x-1,y-2) == 0)? nCount1 + 1: nCount1;

		nCount2 = (pObjectField->value(x+1,y) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y-1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y+1) == 0)? nCount2 + 1: nCount2;
		nCount2 = (pObjectField->value(x+1,y-2) == 0)? nCount2 + 1: nCount2;

		if(nCount1 >= 2 && nCount2 >= 2)
			return true;
	}

	return false;
}

/*
The north side is OK whenever the pattern of 2 neighboring
pixels remains or the next line of pixels has 4
neighboring pixels. In that case we look at something like:
|X|X|X|X|
| |X|X| |
| |X|X| |
| |X|X| |

which we need to thicken is the same holds for the south side
*/
bool NorthSideOk(FIELD<float>* pObjectField, int x ,int y)
{
	int searchY = y-1;
	while(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 255 &&
		pObjectField->value(x+1,searchY) == 255)
	{
		searchY--;
	}
	if(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 0 &&
		pObjectField->value(x+1,searchY) == 0)
		return true;

	if(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 255 &&
		pObjectField->value(x+1,searchY) == 0)
		return true;

	if(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 0 &&
		pObjectField->value(x+1,searchY) == 255)
		return true;

	return false;
}

/*
The south side is OK whenever the pattern of 2 neighboring
pixels remains or the next line of pixels has 4
neighboring pixels. In that case we look at something like:
| |X|X| |
| |X|X| |
| |X|X| |
|X|X|X|X|

which we need to thicken is the same holds for the north side
*/
bool SouthSideOk(FIELD<float>* pObjectField, int x ,int y)
{
	int searchY = y+1;
	while(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 255 &&
		pObjectField->value(x+1,searchY) == 255)
	{
		searchY++;
	}
	if(pObjectField->value(x,searchY) == 0 &&
		pObjectField->value(x-1,searchY) == 0 &&
		pObjectField->value(x-2,searchY) == 0 &&
		pObjectField->value(x+1,searchY) == 0)
		return true;

	return false;
}

bool Skeleton::ThickeningRequired(FIELD<float>* pObjectField)
{
	for(int x = 2; x < pObjectField->dimX() - 2; x++)
		for(int y = 2; y < pObjectField->dimY() - 2; y++)
		{
			/*
			| |O|X| |
			X = (x,y)
			O = (x-1,y)
			*/
			if(pObjectField->value(x,y) == 0 &&
				pObjectField->value(x-1,y) == 0 &&
				pObjectField->value(x-2,y) == 255 &&
				pObjectField->value(x+1,y) == 255)
			{
				if(NorthSideOk(pObjectField,x,y) &&
					SouthSideOk(pObjectField,x,y))
					return true;
			}

			/*
			previous case rotated 90 degrees
			TODO implement if necessary
			at the moment this thickening is
			only done for the umbrellas and in 
			those cases we merely deal with vertical
			cases
			*/
		}

		return false;
}

/*!
@brief This function can be removed. It is not used anymore. It was writtem by
Matthijs when he first tried writting the simplification code. Lots of things
change later and the code was rewritten. The function is left here so that with
more time it can be deleted along with all its subfunctions, which are spread around.
*/
std::vector<TreeSkel*> Skeleton::determine_skeletons(SkelCompParams InputParams)
{
	char inpf[200];

	float k       = -1;                      //Threshold for clusters (def)
	sk_lev  = InputParams.fThresSimp1; //10;                      //Skeleton threshold
	int   N  = 10000000;             //Max # fast marching method iterations
	float sh_len = InputParams.fThresSimp2; //0;                //Structural threshold (default=20 pixels)
	int i=0,j=0;
	//int only_boundary = only_bound; //0 
	//int only_structural = only_struc; //0;
	bool only_boundary = InputParams.bOnlyBound;
	bool only_structural = InputParams.bOnlyStruc;
	bool bDo2DField = InputParams.bDo2DField;
	float sk_lev_upperbound = InputParams.fSimp1UpperBound; //simp1_upper; //101;
	float sh_len_upperbound = InputParams.fSimp2UpperBound; //simp2_upper; //101;
	float sk_lev_stepsize = InputParams.fSimp1StepSize; //simp1_step; //5;
	float sh_len_stepsize = InputParams.fSimp2StepSize; //simp2_step; //5;   
	std::vector<TreeSkel*> ptrs_to_skeletons;

	dMinSlope = InputParams.dMinSlope;
	dMinError = InputParams.dMinError;
	dMaxAccelChg = InputParams.dMaxAccelChg;

	METHOD meth   = AFMM_STAR;                   //CHANGE standard AFMM_STAR will be used    Skeletonization/DT method used
	char  inp[200]; 
	strcpy(inp,InputParams.pInputfile);

	FIELD<float>* f = NULL;

	params = InputParams;

	if (InputParams.pInputField == NULL)
	{
		f = FIELD<float>::read(inp);   //Read scalar field input
		if (!f) 
		{ 
			std::cout <<"Can not open file: "<< inp << std::endl; 
			std::cout <<"Input file:" << std::endl;
			std::cin >> inpf; 
			char* input = inpf;
			char  inp[200]; 
			strcpy(inp,input);
			f = FIELD<float>::read(inp);
		}
	}
	else
		f = InputParams.pInputField;

	FIELD<float> *cnt0=0; FIELD<std::multimap<float,int> >* origs=0; 
	int iter;

	FLAGS*   flags_b = new FLAGS(*f,k);
	FLAGS*   flags   = new FLAGS(*flags_b);   
	FIELD<float>*   dt_b = 0;


	ModifiedFastMarchingMethod::METHOD m = (meth==AFMM_STAR)?  ModifiedFastMarchingMethod::AFMM_STAR :
		(meth==AFMM_VDT)?   ModifiedFastMarchingMethod::AFMM_VDT  :
		(meth==COMPARISON)? ModifiedFastMarchingMethod::AFMM_VDT  :
		(meth==AVERAGING)?  ModifiedFastMarchingMethod::AVERAGING : 
		ModifiedFastMarchingMethod::ONE_POINT; 


	grad       = new FIELD<float>(f->dimX(),f->dimY());
	skel       = new FIELD<float>(f->dimX(),f->dimY());

	cnt0       = do_one_pass(f,flags,origs,0,m,length,iter,dt_b);


	FLAGS* backup_flags_b = new FLAGS(*flags_b);
	FLAGS* backup_flags   = new FLAGS(*flags);
	FIELD<float>* backup_dt_b = new FIELD<float>(*dt_b);
	FIELD<float>* backup_cnt0 = new FIELD<float>(*cnt0);
	FIELD<float>* backup_grad = /*grad;*/ new FIELD<float>(*grad);
	FIELD<float>* backup_skel = /*skel;*/ new FIELD<float>(*skel);
	FIELD<float>* backup_f    = new FIELD<float>(*f);



	//###########################################################################
	//###########################################################################
	//REUSE FIELDS!!!
	//###########################################################################
	//###########################################################################

	std::pair<unsigned int,unsigned int> boundary_pair;

	if(only_boundary == false && 
		only_structural == false &&
		bDo2DField == false &&
		InputParams.bGiveSimpleSkel == false &&
		InputParams.fThresSimp1 != 0)
	{
		std::pair< DARRAY< int >, double > optimum = calculate_errors_new_boundary(SKEL_TAU,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
		delete backup_f;
		delete backup_cnt0;
		delete backup_flags_b;
		delete backup_flags;
		delete backup_dt_b;
		delete backup_grad;
		delete backup_skel;

		backup_flags_b = new FLAGS(*flags_b);
		backup_flags   = new FLAGS(*flags);
		backup_dt_b = new FIELD<float>(*dt_b);
		backup_cnt0 = new FIELD<float>(*cnt0);
		backup_grad = /*grad; */new FIELD<float>(*grad);
		backup_skel = /*skel; */new FIELD<float>(*skel);
		backup_f    = new FIELD<float>(*f);
		// 	

		TreeSkel* newtree = get_new_treeskel_pointer(
			optimum.first, SKEL_TAU, (float)optimum.second, backup_flags, 
			backup_dt_b, backup_cnt0, backup_grad, backup_f, i, j, N);

		ptrs_to_skeletons.push_back(newtree);
		delete f;
		delete cnt0;
		delete origs;
		delete flags_b;
		delete flags;
		delete dt_b;

		delete backup_f;
		delete backup_cnt0;
		delete backup_flags_b;
		delete backup_flags;
		delete backup_dt_b;
		delete backup_grad;
		delete backup_skel;

		delete grad;
		delete skel;

		return ptrs_to_skeletons;
		//ASSERT(false)
	}

	if(only_boundary == true)
	{
		float do_sh_len = sh_len;
		for(float do_sk_lev = sk_lev; do_sk_lev < sk_lev_upperbound; do_sk_lev += sk_lev_stepsize)
		{
			calculate_errors(do_sk_lev,do_sh_len,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
			//calculate_errors_new_boundary(do_sk_lev,do_sh_len,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
		}
		int int_sh_len = int(sh_len);
		int int_sk_lev = int(sk_lev);
		write_error_terms_boundary(int_sk_lev,int_sh_len,sk_lev_upperbound,sk_lev_stepsize);
		boundary_pair = get_global_minimum(int_sk_lev, sk_lev_upperbound, sk_lev_stepsize, int_sh_len, 1);
	}


	if(only_boundary == true && only_structural == true)
	{
		float sk_lev_new = (float)boundary_pair.first;
		float sh_len_new = (float)boundary_pair.second;


		float do_sk_lev = sk_lev_new;
		//for(float do_sh_len = sh_len_new; do_sh_len < sh_len_upperbound; do_sh_len += sh_len_stepsize)
		//{
		calculate_errors(do_sk_lev,sh_len_upperbound,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
		//}
		int int_sk_lev = int(sk_lev_new);
		int int_sh_len = int(sh_len_new);
		write_error_terms_structural(int_sk_lev,int_sh_len,sh_len_upperbound,sh_len_stepsize);
		std::pair<unsigned int,unsigned int> minpair = get_global_minimum(int_sh_len, sh_len_upperbound, sh_len_stepsize, int_sk_lev, 0);
		//get_local_minima(int_sh_len, sh_len_upperbound, sh_len_stepsize, int_sk_lev);
		//char getchar;
		//std::cin >> getchar;
		//     for(unsigned int l=0; l < minima_list.size(); l++)
		//     {
		std::cerr << "\n\nADDING ONE SKELETON TO THE OUTPUT LIST!\n";
		float boundary = (float)minpair.first;
		float structural = (float)minpair.second;
		TreeSkel* nexttree = get_treeskel_pointer(boundary,structural,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_f,i,j,N);
		ptrs_to_skeletons.push_back(nexttree);
		//     }

	}
	else if(only_structural == true)
	{
		float do_sk_lev = sk_lev;
		for(float do_sh_len = sh_len; do_sh_len < sh_len_upperbound; do_sh_len += sh_len_stepsize)
		{
			calculate_errors(do_sk_lev,do_sh_len,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
		}
		int int_sk_lev = int(sk_lev);
		int int_sh_len = int(sh_len);
		write_error_terms_structural(int_sk_lev,int_sh_len,sh_len_upperbound,sh_len_stepsize);
		get_global_minimum(int_sh_len, sh_len_upperbound, sh_len_stepsize, int_sk_lev, 0);
		get_local_minima(int_sh_len, sh_len_upperbound, sh_len_stepsize, int_sk_lev);
	}

	if(bDo2DField == true)
	{
		std::vector<std::pair<unsigned int,unsigned int> > minima_list;
		for(float do_sk_lev = sk_lev; do_sk_lev < sk_lev_upperbound; do_sk_lev += sk_lev_stepsize)
		{
			//for(float do_sh_len = sh_len; do_sh_len < sh_len_upperbound; do_sh_len += sh_len_stepsize)
			//{
			calculate_errors(do_sk_lev,sh_len_upperbound,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
			//}
		}
		std::cerr << "All errors have been succesfully calculated" << std::endl;
		int int_sk_lev = int(sk_lev);
		int int_sh_len = int(sh_len);
		write_error_terms(int_sk_lev,int_sh_len,sk_lev_upperbound,sk_lev_stepsize,sh_len_upperbound,sh_len_stepsize);
		minima_list = get_all_local_minima(int_sk_lev, int_sh_len, sk_lev_upperbound, sk_lev_stepsize, sh_len_upperbound, sh_len_stepsize);
		for(unsigned int k=0; k < minima_list.size(); k++)
			std::cout << minima_list[k].first << "," << minima_list[k].second << std::endl;
		//first one in the vector should be a skeleton with only some boundary smoothing
		//if(give_simple_skel)
		//{
		//	float no_simp_sk_lev = 15, no_simp_sh_len = 0;
		//	TreeSkel* newtree = get_treeskel_pointer(no_simp_sk_lev,no_simp_sh_len,backup_flags,backup_dt_b,backup_cnt0,backup_grad);
		//	ptrs_to_skeletons.push_back(newtree);
		//}
		for(unsigned int l=0; l < minima_list.size(); l++)
		{
			std::cerr << "\n\nADDING ONE SKELETON TO THE OUTPUT LIST!\n";
			float boundary = (float)minima_list[l].first;
			float structural = (float)minima_list[l].second;
			TreeSkel* nexttree = get_treeskel_pointer(boundary,structural,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_f,i,j,N);
			ptrs_to_skeletons.push_back(nexttree);
		}
	}

	if(InputParams.fThresSimp1 == 0)
	{
		//calculate_errors(sk_lev,sh_len,backup_flags_b,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_skel,backup_f,i,j,N);
		DARRAY< int > indices;
		TreeSkel* newtree = get_new_treeskel_pointer(indices,SKEL_TAU,0,backup_flags,backup_dt_b,backup_cnt0,backup_grad,backup_f,i,j,N);

		ptrs_to_skeletons.push_back(newtree);

	}

	delete f;
	delete cnt0;
	delete origs;
	delete flags_b;
	delete flags;
	delete dt_b;

	delete backup_f;
	delete backup_cnt0;
	delete backup_flags_b;
	delete backup_flags;
	delete backup_dt_b;
	delete backup_grad;
	delete backup_skel;

	delete grad;
	delete skel;

	return ptrs_to_skeletons;
}



void Skeleton::likelihood_term_ed(FLAGS* obj1, FIELD<float>* dt1, FLAGS* obj2, FIELD<float>* dt2, float& d1)
//Computes the 'distance' from obj 1 to obj 2. The inside of both objects is marked as ALIVE.
//The complete (inside/outside) DTs are given for both obj1 and obj2.
//Object 1 = original object; object 2 = simplified object.
//----------------------------------------------------------------------------------------
//The inter-object distance d is the weighted-sum of two components (d=lambda1*d1+lambda2*d2):
//d1 = sum of DT1 over points in obj1 but not in obj2 and points in obj2 but not in obj1,
//     normalized by sum of DT1 over points in obj1.
//d2 = 'eccentricity' of obj2, measuring how far obj2 is from a perfect circle.
//     This can be normalized by eccentricity of obj1 (in which case d2 ranges from 1 to 0) or not.
//     Argument normalize_2 tells if we normalize or not.
//
//----------------------------------------------------------------------------------------
{
	int i,j; float dsum1=0,nsum1=0,dsum2=0,nsum2=0;

	for(i=0;i<obj1->dimX();i++)              //Sum up DT1 over points in obj1 but not in obj2
		for(j=0;j<obj1->dimY();j++)           //and in obj2 but not in obj1
		{
			if(obj1->alive(i,j) && !obj2->alive(i,j))   //points in S1, but not in S2
				dsum1 += dt1->value(i,j);
			if(obj2->alive(i,j) && !obj1->alive(i,j))   //points in S2, but not in S1
				dsum2 += dt2->value(i,j);
			if (obj1->alive(i,j))               //Sum up DT1 over points in obj1
				nsum1 += dt1->value(i,j);
			if (obj2->alive(i,j))               //Sum up DT2 over points in obj2
				nsum2 += dt2->value(i,j);
		}
		if(dsum2 == 0)
		{    d1 = dsum1/nsum1;   }
		else //dsum2 != 0
		{    d1 = dsum1/nsum1 + dsum2/nsum2; }
}




FIELD<float>* Skeleton::do_one_pass(FIELD<float>* fi,FLAGS* flagsi,FIELD<std::multimap<float,int> >* origs,int dir,ModifiedFastMarchingMethod::METHOD meth,
									float& length,int& iter,FIELD<float>*& f)
{
	//FIELD<float>*      f = (!dir)? new FIELD<float>(*fi) : fi; //Copy input field 

	f = new FIELD<float>(*fi); // this is a return value

	//FLAGS*       flags = new FLAGS(*flagsi);           //Copy flags field
	FIELD<float>*  count = new FIELD<float>;
	const int   N        = 10000000;             //Max # fast marching method iterations

	ModifiedFastMarchingMethod fmm(f,flagsi,count,origs,N);  //Create fast marching method engine
	fmm.setScanDir(dir);                     //Set its various parameters
	fmm.setMethod(meth);

	fmm.setStopCriterium(INFINITY);
	fmm.setStatisticsFreq(-1);

	int nfail,nextr;               
	iter   = fmm.execute(nfail,nextr);               //...and execute the skeletonization
	length = fmm.getLength();

	return count;
}


FIELD<float>* Skeleton::inflate_obj(FIELD<float>* fi,FLAGS* flags,int N,FIELD<float>* max_d,float max_dv)
{
	FIELD<float>* f = new FIELD<float>(*fi);           //Copy input field
	FastMarchingMethod fmm(f,flags,N);           //Create simple fast marching method obj
	if (max_d) fmm.setStopCriterium(max_d);
	else  fmm.setStopCriterium(max_dv);
	fmm.setStatisticsFreq(-1);
	int nfail = 0,nextr = 0;
	fmm.execute(nfail,nextr);

	return f;
}



void Skeleton::comp_grad2(FLAGS* forig,FIELD<float>* cnt1,FIELD<float>* cnt2,FIELD<float>* grad)
//Gradient computation using 2-pass method (i.e. grad computed out of 2 fields)
{
	int i,j;

	for(j=0;j<grad->dimY();j++)              //Compute grad in a special way, i.e. on a 2-pixel 
		for(i=0;i<grad->dimX();i++)            //neighbourhood - this ensures pixel-size skeletons!    
		{
			float ux1 = cnt1->value(i+1,j) - cnt1->value(i,j);
			float uy1 = cnt1->value(i,j+1) - cnt1->value(i,j);
			float g1  = max(fabs(ux1),fabs(uy1));
			float ux2 = cnt2->value(i+1,j) - cnt2->value(i,j);
			float uy2 = cnt2->value(i,j+1) - cnt2->value(i,j);
			float g2  = max(fabs(ux2),fabs(uy2));
			grad->value(i,j) = min(g1,g2);
		}
}




void Skeleton::comp_grad(FLAGS* forig,FIELD<float>* cnt,FIELD<float>* grad)
//Gradient computation using 1-pass method (i.e. grad computed out of 1 field)
{
	int i,j;
	for(j=0;j<grad->dimY();j++)              //Compute grad in a special way, i.e. on a 2-pixel 
		for(i=0;i<grad->dimX();i++)            //neighbourhood - this ensures pixel-size skeletons!    
		{
			float ux = cnt->value(i+1,j) - cnt->value(i,j);
			float uy = cnt->value(i,j+1) - cnt->value(i,j);
			grad->value(i,j) = max(distance(ux),distance(uy));
		}
}


void Skeleton::postprocess(FIELD<float>* grad,FIELD<float>* skel,float level)
//Simple reduction of gradient to binary skeleton via thresholding
{
	for(int j=0;j<grad->dimY();j++)          //Threshold 'grad' to get real skeleton
		for(int i=0;i<grad->dimX();i++)
		{
			float g = grad->value(i,j);
			if (g>INFINITY/2) g = 0.5; else if (g>level) g = 1; else g = 0;
			skel->value(i,j)=g;
		} 
}


void Skeleton::postprocess2(FIELD<float>* grad,float level)
//Simple reduction of gradient to binary skeleton via thresholding
{
	for(int j=0;j<grad->dimY();j++)          //Threshold 'grad' to get real skeleton
		for(int i=0;i<grad->dimX();i++)
		{
			float g = grad->value(i,j);
			if (g>INFINITY/2) g=0; else
				if (g>level) g = 1; else g = 0;
			grad->value(i,j)=g;
		}
}



void Skeleton::write_error_terms_structural(int& sk_lev, int& sh_len, float& sh_len_upperbound, float& sh_len_stepsize)
{
	std::cout << "Start writing results to file... ";
	std::pair<unsigned int,unsigned int> lookuppair;
	terms get_terms;
	FILE *fp;
	fp = fopen("structural_results.m", "w");
	fprintf(fp,"m = [\n");

	for(float do_sh_len = (float)sh_len; do_sh_len < sh_len_upperbound; do_sh_len += sh_len_stepsize)
	{
		lookuppair.first = sk_lev;
		lookuppair.second = int(do_sh_len);
		get_terms = error_terms[lookuppair];
		int firstsimp = sk_lev;
		fprintf(fp,"%d,%f,%d,%f\n",firstsimp,do_sh_len,get_terms.number_branches,get_terms.likelihood_term);
	}

	fprintf(fp,"];\n");
	fclose(fp);
	std::cout << "done\n";
}



void Skeleton::write_error_terms_boundary(int& sk_lev, int& sh_len, float& sk_lev_upperbound, float& sk_lev_stepsize)
{
	std::cout << "Start writing results to file... ";
	std::pair<unsigned int,unsigned int> lookuppair;
	terms get_terms;
	FILE *fp;
	fp = fopen("boundary_results.m", "w");
	fprintf(fp,"m = [\n");

	for(float do_sk_lev = (float)sk_lev; do_sk_lev < sk_lev_upperbound; do_sk_lev += sk_lev_stepsize)
	{
		lookuppair.first = int(do_sk_lev);
		lookuppair.second = sh_len;
		get_terms = error_terms[lookuppair];
		std::cout << std::endl << "--- " << get_terms.number_branches << " --- " << get_terms.likelihood_term << std::endl << std::endl;
		int secondsimp = sh_len;
		fprintf(fp,"%f,%d,%d,%f\n",do_sk_lev,secondsimp,get_terms.number_branches,get_terms.likelihood_term);
	}
	fprintf(fp,"];\n");
	fclose(fp);
	std::cout << "done\n";
}




/*
Just for now this function will only return the actual global minimum
It will still display all other local minima just to be able to see
whether one of those might "look" better visually
*/
std::vector<std::pair<unsigned int,unsigned int> > Skeleton::get_all_local_minima(int& sk_lev_param, int& sh_len_param, float& sk_upperbound, float& sk_stepsize, float& sh_upperbound, float& sh_stepsize)
{
	std::vector<std::pair<unsigned int,unsigned int> > list_of_minima;
	double dGlobalMinimum = 10000000;
	std::pair<unsigned int,unsigned int> coorPairGlobMin;
	if((sk_lev_param + 2 * sk_stepsize > sk_upperbound) || (sh_len_param + 2 * sh_stepsize > sh_upperbound))
	{
		std::cout << "Unable to find local minima, search space is too small\n";  
	}
	else
	{
		std::cout << "\nStart calculating all the local minima in the 2D field\n";
		std::pair<unsigned int,unsigned int> lookuppair;
		terms get_terms;
		double right_error = 0, current_error = 0, left_error = 0, upper_error = 0, lower_error = 0;
		for(float do_sk_lev = sk_lev_param + sk_stepsize; do_sk_lev < sk_upperbound - sk_stepsize; do_sk_lev += sk_stepsize)
		{
			for(float do_sh_len = (float)sh_len_param; do_sh_len < sh_upperbound - sh_stepsize; do_sh_len += sh_stepsize)
			{
				if(do_sh_len == sh_len_param) //bottom row, we can't look at the value "below"
				{
					lookuppair.first = int(do_sk_lev) + int(sk_stepsize);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					right_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev);
					lookuppair.second = int(do_sh_len) + int(sh_stepsize);
					get_terms = error_terms[lookuppair];
					upper_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev) - int(sk_stepsize);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					left_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					current_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					if(current_error < left_error &&
						current_error < right_error &&
						current_error <= upper_error)
					{
						//std::cerr << "Minima found : " << do_sk_lev << "," << do_sh_len << " with total error being: " << current_error << std::endl;
						//list_of_minima.push_back(lookuppair);
						if (current_error < dGlobalMinimum)
						{
							dGlobalMinimum = current_error;
							coorPairGlobMin = lookuppair;
						}
					}

				}
				else //we are not in the bottom row
				{
					lookuppair.first = int(do_sk_lev);
					lookuppair.second = int(do_sh_len) - int(sh_stepsize);
					get_terms = error_terms[lookuppair];
					lower_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev) + int(sk_stepsize);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					right_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev);
					lookuppair.second = int(do_sh_len) + int(sh_stepsize);
					get_terms = error_terms[lookuppair];
					upper_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev) - int(sk_stepsize);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					left_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					lookuppair.first = int(do_sk_lev);
					lookuppair.second = int(do_sh_len);
					get_terms = error_terms[lookuppair];
					current_error = (double(get_terms.number_branches) * 5.0) + (300.0 * double(get_terms.likelihood_term));
					if(current_error < left_error &&
						current_error <= lower_error &&
						current_error < right_error &&
						current_error <= upper_error)
					{
						//std::cerr << "Minima found : " << do_sk_lev << "," << do_sh_len << " with total error being: " << current_error << std::endl;
						//list_of_minima.push_back(lookuppair);
						if (current_error < dGlobalMinimum)
						{
							dGlobalMinimum = current_error;
							coorPairGlobMin = lookuppair;
						}
					}
				}
			}
		}
		list_of_minima.push_back(coorPairGlobMin);
	}
	return list_of_minima;
}



void Skeleton::get_local_minima(int& begin_param, float& upperbound, float& stepsize, int& stationary_param)
{
	if(begin_param + 2 * stepsize > upperbound)
	{
		std::cout << "Unable to find local minima, search space is too small\n";
	}
	else
	{
		std::cout << "\nStart calculating the local minima...\n";
		std::pair<unsigned int,unsigned int> lookuppair;
		terms get_terms;
		float previous_error = 0, current_error = 0, next_error = 0;
		lookuppair.first = stationary_param;
		lookuppair.second = begin_param;
		get_terms = error_terms[lookuppair];
		previous_error = (get_terms.number_branches) + (600 * get_terms.likelihood_term);
		lookuppair.first = stationary_param;
		lookuppair.second = begin_param + int(stepsize);
		get_terms = error_terms[lookuppair];
		current_error = (get_terms.number_branches) + (600 * get_terms.likelihood_term);
		for(float moving_param = begin_param + stepsize; moving_param < upperbound - stepsize; moving_param += stepsize)
		{
			lookuppair.first = stationary_param;
			lookuppair.second = int(moving_param) + int(stepsize);
			get_terms = error_terms[lookuppair];
			next_error = (get_terms.number_branches) + (600 * get_terms.likelihood_term);
			if(current_error < previous_error && current_error <= next_error)
			{
				//std::cout << "Minima found : " << stationary_param << "," << moving_param << " with total error being: " << current_error << std::endl;
			}
			previous_error = current_error;
			current_error = next_error;
		}
	}

}

std::pair<unsigned int,unsigned int> Skeleton::get_global_minimum(int& begin_param, float& upperbound, float& stepsize, int& stationary_param, int boundary)
{
	std::cout << "Start calculating global minimum... \n";
	std::pair<unsigned int,unsigned int> lookuppair;
	std::pair<unsigned int,unsigned int> minpair;
	terms get_terms;
	float minimum_error = 10000000;
	for(float moving_param = (float)begin_param; moving_param < upperbound; moving_param += stepsize)
	{
		if(boundary == 1)
		{
			lookuppair.first = int(moving_param);
			lookuppair.second = stationary_param;
		}
		else
		{
			lookuppair.first = stationary_param;
			lookuppair.second = int(moving_param);    
		}
		get_terms = error_terms[lookuppair];

		float current_total_error = 0;
		if(boundary == 1)
		{
			current_total_error = (get_terms.number_branches) + (300 * get_terms.likelihood_term);
		}
		else	
		{
			current_total_error = (get_terms.number_branches * 6) + (300 * get_terms.likelihood_term);
		}
		//std::cout << current_total_error << std::endl;
		if(minimum_error > current_total_error)
		{
			minimum_error = current_total_error;
			minpair = lookuppair;
		}
	}
	if(boundary == 1)
	{
		std::cout << "Global minimum along boundary is found in the pair : " << minpair.first << "," << minpair.second << std::endl;
	}
	else
	{
		std::cout << "Global minimum along structural is found in the pair : " << minpair.first << "," << minpair.second << std::endl;
	}
	return minpair;
}

void Skeleton::write_error_terms(int& sk_lev, int& sh_len, float& sk_lev_upperbound, float& sk_lev_stepsize, float& sh_len_upperbound, float& sh_len_stepsize)
{
	std::cout << "Start writing results to file... ";
	std::pair<unsigned int,unsigned int> lookuppair;
	terms get_terms;
	FILE *fp;
	fp = fopen("combined_results.m", "w");
	fprintf(fp,"m = [\n");
	for(float do_sk_lev = (float)sk_lev; do_sk_lev < sk_lev_upperbound; do_sk_lev += sk_lev_stepsize)
		for(float do_sh_len = (float)sh_len; do_sh_len < sh_len_upperbound; do_sh_len += sh_len_stepsize)
		{
			lookuppair.first = int(do_sk_lev);
			lookuppair.second = int(do_sh_len);
			get_terms = error_terms[lookuppair];
			//std::cout << std::endl << "--- " << get_terms.number_branches << " --- " << get_terms.likelihood_term << std::endl << std::endl;
			fprintf(fp,"%f,%f,%d,%f\n",do_sk_lev,do_sh_len,get_terms.number_branches,get_terms.likelihood_term);
		}
		fprintf(fp,"];\n");
		fclose(fp);
		std::cout << "done\n";
}



TreeSkel* Skeleton::get_treeskel_pointer(float do_sk_lev, float do_sh_len, FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,FIELD<float>* b_grad, FIELD<float>* b_f,int i,int j,int N)
{     
	if(do_sk_lev == 0)
	{
		do_sk_lev = 20;
	}

	FLAGS* flags   = new FLAGS(*b_flags);
	FIELD<float>* dt_b = new FIELD<float>(*b_dt_b);
	FIELD<float>* cnt0 = new FIELD<float>(*b_cnt0);
	FIELD<float>* grad = new FIELD<float>(*b_grad);
	TreeSkel* tsk;

	comp_grad(flags,cnt0,grad);  
	postprocess2(grad,do_sk_lev);
	FIELD<float>* grad2 = new FIELD<float>(*grad);
	delete flags;

	//   if (do_sh_len >= 0)                       //Apply structural simplification only if it will do anything
	//   {                            //(saves time and also allows disabling this step if e.g. unstable)
	ThinningMethod thm(grad2,dt_b,-0.9f);       //Make boundary-skel 1-pixel-thin by DT-order thinning

	FIELD<int>* thin_skel = thm.thin();    

	tsk = new TreeSkel(thin_skel);       //Reduce 1-pixel-thin skel to graph structure

	tsk->AssignRadius(*dt_b);

	tsk->GetShockGraphParams(dMinSlope, dMinError, dMaxAccelChg);

	//tsk->simplify(do_sh_len,1,500);        //Simplify skel using internal branch length criterion
	//TODO
	//tsk->simplifyLaplacian(do_sh_len);

	std::pair<bool,double> resultSimplification;
	//      for(i = 0; i < do_sh_len; i++)
	//      	resultSimplification = tsk->performNextSimplification();
	//      //resultSimplification = tsk->performNextSimplification();
	//     // resultSimplification = tsk->performNextSimplification();
	int dSimplifiedLenght = 0;

	while((dSimplifiedLenght + int(tsk->getSizeSmallestInsideBranch()) <= int(do_sh_len)) && (tsk->getSizeSmallestInsideBranch() != -1))
	{
		dSimplifiedLenght += int(tsk->getSizeSmallestInsideBranch());
		resultSimplification = tsk->performNextSimplification();
	}

	if(do_sh_len != 0)
	{
		tsk->GetCorrectRadii(dt_b);
		tsk->render(*grad2,1);             //Convert (render) simplified skel to image
	}


	delete thin_skel;     

	FIELD<float>* f = new FIELD<float>(*grad2);            //*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
	flags = new FLAGS(*f,*dt_b,-0.9f);

	FIELD<float>* s_infl = 0;
	s_infl = inflate_obj(f,flags,N,0,0);           //Inflate skeleton till every point reaches its DT_s value.
	delete f; 
	delete flags;              //(this gives us the simplified object)

	FLAGS* flags_s = new FLAGS(*s_infl,-1);
	delete flags_s;

	/*FIELD<float>* s_infl2 = new FIELD<float>(*s_infl);   //---------------------------------------------
	for(i=0;i<s_infl->dimX();i++)            //Compute a mask = the inflated skeleton = the simplified object
	for(j=0;j<s_infl->dimY();j++)
	{
	s_infl->value(i,j)  = (s_infl->value(i,j)<0)? INFINITY : -1;
	s_infl2->value(i,j) = (s_infl->value(i,j)==-1)? INFINITY : -1;
	}

	flags = new FLAGS(*s_infl2,-1);          //Next, compute dt_s2 = DT inside object
	FIELD<float>* dt_s2 = 0;
	inflate_obj(s_infl2,flags,N,0,INFINITY,dt_s2);
	//tsk->AssignRadius(*dt_s2);
	tsk->WriteAllBranchesToMATLABFile();

	delete dt_s2;
	delete s_infl2;
	*/
	delete s_infl;

	delete dt_b;

	delete cnt0;

	delete grad;

	delete grad2;

	return tsk;
}

bool SkelPointIsEndPoint(int x, int y, FIELD<int> *pSkelField)
{
	int count = 0;
	for(int yy = y-1; yy <= y + 1; yy++){
		for(int xx = x -1; xx <= x+1; xx++){
			if((x != xx|| y != yy) && pSkelField->value(xx,yy) == 1){
				count++;
			}
		}
	}
	if(count == 1)
		return true;
	else	
		return false;
}



TreeSkel* Skeleton::get_new_treeskel_pointer(DARRAY< int > indices, float do_sk_lev, float do_sh_len, FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,FIELD<float>* b_grad, FIELD<float>* b_f,int i,int j,int N)
{     
	FLAGS* flags   = new FLAGS(*b_flags);
	writePPM(flags,"APROXobj_init.ppm",1);
	FIELD<float>* dt_b = new FIELD<float>(*b_dt_b);
	FIELD<float>* cnt0 = new FIELD<float>(*b_cnt0);
	FIELD<float>* grad = new FIELD<float>(*b_grad);
	TreeSkel* tsk;

	std::cerr << "\nDo_sk_lev = " << do_sk_lev << " and do_sh_len = " << do_sh_len << std::endl;

	comp_grad(flags,cnt0,grad);  
	postprocess2(grad,do_sk_lev);
	FIELD<float>* grad2 = new FIELD<float>(*grad);
	writePPM(grad2,"ThinSkeleton.ppm",1);
	delete flags;

	ThinningMethod thm(grad2,dt_b,-0.9f);       //Make boundary-skel 1-pixel-thin by DT-order thinning

	FIELD<int>* thin_skel = thm.thin();


	thin_skel->writefieldtofile("PixelThinSkel.txt");




	tsk = new TreeSkel(thin_skel);       //Reduce 1-pixel-thin skel to graph structure
	tsk->AssignRadius(*dt_b);

	tsk->GetShockGraphParams(dMinSlope, dMinError, dMaxAccelChg);
	tsk->GetCorrectRadii(dt_b);



	if(indices.Count() != 0)
	{
		tsk->RemoveOutsideBranch(indices);
	}

	if(do_sh_len != 0)
	{
		std::pair<bool,double> resultSimplification;
		int dSimplifiedLenght = 0;

		while((dSimplifiedLenght + int(tsk->getSizeSmallestInsideBranch()) <= int(do_sh_len)) && (tsk->getSizeSmallestInsideBranch() != -1))
		{
			dSimplifiedLenght += int(tsk->getSizeSmallestInsideBranch());
			resultSimplification = tsk->performNextSimplification();
		}

		tsk->GetCorrectRadii(dt_b);
		tsk->render(*grad2,1);             //Convert (render) simplified skel to image
	}


	FIELD<float>* f = new FIELD<float>(*grad2);            //*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
	flags = new FLAGS(*f,*dt_b,-0.9f);

	FIELD<float>* s_infl = 0;
	s_infl = inflate_obj(f,flags,N,0,0);           //Inflate skeleton till every point reaches its DT_s value.
	delete f; 
	delete flags;              //(this gives us the simplified object)

	FLAGS* flags_s = new FLAGS(*s_infl,-1);
	writePPM(flags_s,"obj_simpl.ppm",1);
	delete flags_s;

	//########################## ADDED JANUARI 09 ########################################################     
	/*
	FIELD<float>* s_infl2 = new FIELD<float>(*s_infl);   //---------------------------------------------
	for(i=0;i<s_infl->dimX();i++)            //Compute a mask = the inflated skeleton = the simplified object
	for(j=0;j<s_infl->dimY();j++)
	{
	s_infl->value(i,j)  = (s_infl->value(i,j)<0)? INFINITY : -1;
	s_infl2->value(i,j) = (s_infl->value(i,j)==-1)? INFINITY : -1;
	}

	flags = new FLAGS(*s_infl2,-1);          //Next, compute dt_s2 = DT inside object
	FIELD<float>* dt_s2 = 0;
	inflate_obj(s_infl2,flags,N,0,INFINITY,dt_s2);
	tsk->AssignRadius(*dt_s2);
	tsk->WriteAllBranchesToMATLABFile();

	delete dt_s2;
	delete s_infl2;
	*/
	//####################################################################################################     

	delete s_infl;
	delete cnt0;
	delete grad;
	delete grad2;
	delete dt_b;
	delete thin_skel;  
	return tsk;
}

/*!
@brief Removes all but the largest white-color connected components in the image

Note: It does not remove small black components, as done in the 
RemoveNonMaxComponents() function in dagmatcher/ImageProcessing.cpp
*/
void RemoveSmallBackgroundComponents(FIELD<float>* pSkelField)
{
	FIELD<float> labeledGradField(pSkelField->dimX(),pSkelField->dimY());
	ConnectedComponents cc(100);
	int n = cc.connected(pSkelField->data(),
		labeledGradField.data(),
		pSkelField->dimX(),
		pSkelField->dimY(),
		std::equal_to<float>(),
		false);

	if(n <= 2)
		return; // nothing to do

	IntVector whiteLbls(n, 0);
	int x,y,i,maxWhiteLbl = 0,maxWhiteSz = 0;

	// Count number of pixels in each "background color" component
	for(x = 0; x < labeledGradField.dimX(); x++)
		for(y = 0; y < labeledGradField.dimY(); y++)
			if(pSkelField->value(x,y) == 0) //background pixel
				whiteLbls[int(labeledGradField.value(x,y))]++;

	// Find largest background color component
	for(i = 0; i < n; i++)
	{
		if(whiteLbls[i] > maxWhiteSz)
		{
			maxWhiteSz = whiteLbls[i];
			maxWhiteLbl = i;
		}
	}

	// Remove "small" white connected components
	for(x = 0; x < labeledGradField.dimX(); x++)
		for(y = 0; y < labeledGradField.dimY(); y++)
		{
			i = int(labeledGradField.value(x,y));
			if(pSkelField->value(x,y) == 0 && i != maxWhiteLbl) //background pixel
				pSkelField->value(x,y) = 1; //turn it into a skeleton pixel (to be thinned)
		}	
}

/*!
@brief Removes all but the largest black-color connected components in the image

Note: It does not remove small white components, as done in the 
RemoveNonMaxComponents() function in dagmatcher/ImageProcessing.cpp
*/
void RemoveNonMaxComponents(FIELD<int>* pSkelField)
{
	FIELD<int> labeledSkelField(pSkelField->dimX(),pSkelField->dimY());

	ConnectedComponents cc(10);

	int n = cc.connected(pSkelField->data(),
		labeledSkelField.data(),
		pSkelField->dimX(),
		pSkelField->dimY(),
		std::equal_to<int>(),
		true);

	if(n <= 2)
		return; // nothing to do

	//remove all the non max skeleton components
	IntVector blackLbls(n, 0);
	int x, y, i, maxBlackLbl = 0, maxBlackSz = 0;

	// Count number of pixels in each "foreground color" component
	for(x = 0; x < labeledSkelField.dimX(); x++)
		for(y = 0; y < labeledSkelField.dimY(); y++)
			if(pSkelField->value(x,y) == 1) //foreground pixel
				blackLbls[int(labeledSkelField.value(x,y))]++;

	// Find largest foreground color component
	for(i = 0; i < n; i++)
	{
		if(blackLbls[i] > maxBlackSz)
		{
			maxBlackSz = blackLbls[i];
			maxBlackLbl = i;
		}
	}
	// Remove "small" foreground (black) components
	for(x = 0; x < labeledSkelField.dimX(); x++)
		for(y = 0; y < labeledSkelField.dimY(); y++)
		{
			i = int(labeledSkelField.value(x,y));

			if(pSkelField->value(x,y) == 1 && i != maxBlackLbl) //background pixel
				pSkelField->value(x,y) = 0; //turn it into a background pixel
		}
}

/*!
b_f is the input fields, i.e., the original image
*/
AFMMSkeleton* Skeleton::get_DDSl_pointer(float do_sk_lev, float do_sh_len, 
										 FLAGS* b_flags, FIELD<float>* b_dt_b, 
										 FIELD<float>* b_cnt0, FIELD<float>* b_grad, 
										 FIELD<float>* b_f, int N, int nBndrySmoothIter)
{     
	FLAGS* flags   = new FLAGS(*b_flags);
	FIELD<float>* dt_b = new FIELD<float>(*b_dt_b);
	FIELD<float>* cnt0 = new FIELD<float>(*b_cnt0);
	FIELD<float>* grad = new FIELD<float>(*b_grad);

	//do_sk_lev should at least be 2 otherwise we are passing something around that looks
	//more like an object then a skeleton
	if(do_sk_lev < 2)
		do_sk_lev = 2;    

	comp_grad(flags,cnt0,grad); 
	postprocess2(grad,do_sk_lev);

	delete flags;

	// 8/Nov/2006. Diego: We can assume that "small white spots" have been removed already
	RemoveSmallBackgroundComponents(grad);

	ThinningMethod thm(grad,dt_b,-0.9f);       //Make boundary-skel 1-pixel-thin by DT-order thinning

	FIELD<int>* thin_skel = thm.thin();

	// Sometimes it may happen that after thinning we end up with a disconnected skeleton
	RemoveNonMaxComponents(thin_skel);

	// We have a thin skeleton here. To transform it into the McGill code's 
	// skeleton graph we will need to determine which points are end
	// points of the branches
	const int nDimY = b_f->dimY(), nDimX = b_f->dimX();

	ASSERT(nBndrySmoothIter >= 0 && nBndrySmoothIter <= 5);

	sg::ShapeMaker sm(nDimX, nDimY, nBndrySmoothIter, false /*no init*/);

	int x,y;

	for (y=0; y < nDimY; y++)
		for (x=0; x < nDimX; x++)
			sm(x,y) = b_f->value(x,y) == 0;

	// The shape is being made here
	sg::ShapeBoundary* pShape = sm.getShape();

	double xmin, xmax, ymin, ymax; 

	pShape->getBounds(&xmin, &xmax, &ymin, &ymax);

	if(xmin >= xmax || ymin >= ymax)
	{
		xmin = 0;
		ymin = 0;
		xmax = thin_skel->dimX();
		ymax = thin_skel->dimY();
	}

	// Create divarray
	sg::DivArr da(thin_skel->dimX(), thin_skel->dimY());
	sg::DivArr::iterator it = da.begin();

	for(y = 0; y < thin_skel->dimY(); y++)
	{
		for(x = 0; x < thin_skel->dimX(); x++, it++)
		{
			if(thin_skel->value(x,y) == 0)
				//backgroundpixel has color 1
			{
				*it = sg::DivPt(sg::Point(x,y), DIV_MAP_MAX_VAL);
				it->col = 1;
				it->dist = dt_b->value(x,y);
			}
			else if(thin_skel->value(x,y) == 1)
				//skeleton pixel has color 0 if just "foreground" has color 2 if endpoint
			{
				*it = sg::DivPt(sg::Point(x,y), 1);
				//if(SkelPointIsEndPoint(x,y,thin_skel))
				//	dp.col = 2;
				//else //no endpoint
				it->col = 0;
				it->dist = dt_b->value(x,y);
			}
			else
				ShowError("Cannot convert AFMM skeleton to McGill's format");
		}
	}

	// Create an AFMMSkeleton, which owns the shape
	AFMMSkeleton* pDDS = computeAFMMkeleton(da, pShape);

	pDDS->SetDimensions(xmin, xmax, ymin, ymax);

	// Let every node and edge node its index along
	// the node/edge vectors of the graph
	pDDS->SetNodeAndEdgeIndices();

	delete dt_b;
	delete cnt0;
	delete grad;
	delete thin_skel;

	return pDDS;
}





void Skeleton::calculate_errors(float do_sk_lev, float do_sh_len, FLAGS* b_flags_b,FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,FIELD<float>* b_grad,FIELD<float>* b_skel, FIELD<float>* b_f,int i,int j,int N)
{
	std::cerr << "----------------- iteration " << do_sk_lev << " ----------------\n\n";      

	DARRAY<std::pair<int ,std::pair<int,double> > > differentLikelihoods;
	std::pair<int,double> branchesLikelihood;
	std::pair<int ,std::pair<int,double> > thresholdAndtheRest;

	FLAGS* flags_b = new FLAGS(*b_flags_b);
	FLAGS* flags   = new FLAGS(*b_flags);
	FIELD<float>* dt_b = new FIELD<float>(*b_dt_b);
	FIELD<float>* cnt0 = new FIELD<float>(*b_cnt0);
	FIELD<float>* grad = new FIELD<float>(*b_grad);
	FIELD<float>* skel = new FIELD<float>(*b_skel);
	FIELD<float>* f    = new FIELD<float>(*b_f);
	comp_grad(flags,cnt0,grad);  
	postprocess2(grad,do_sk_lev);
	FIELD<float>* grad2 = new FIELD<float>(*grad);

	//do_sh_len now specifies the max value of structural simplification we are interested in.

	ThinningMethod thm(grad2,dt_b,-0.9f);       //Make boundary-skel 1-pixel-thin by DT-order thinning
	FIELD<int>* thin_skel = thm.thin();    
	TreeSkel* tsk = new TreeSkel(thin_skel);       //Reduce 1-pixel-thin skel to graph structure
	tsk->AssignRadius(*dt_b);
	tsk->GetShockGraphParams(dMinSlope, dMinError, dMaxAccelChg);

	tsk->GetCorrectRadii(dt_b);
	tsk->render(*grad2,1);             //Convert (render) simplified skel to image

	//      char filename[100];
	//      int nSimps = 0;
	//      sprintf(filename,"%f, %d, skel_simp",do_sk_lev,nSimps);
	//      writePPM(grad2,filename,1);

	int number_branch = 0;
	for(i=0;i<tsk->branches.Count();i++)//get the number of branches after the simplification
		number_branch++;
	//current_terms.number_branches = number_branch;
	branchesLikelihood.first = number_branch;
	delete thin_skel;
	delete f; 
	delete flags; 
	delete cnt0;
	f     = new FIELD<float>(*grad2);            //*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
	flags = new FLAGS(*f,*dt_b,-0.9f); 
	FIELD<float>* s_infl = 0;
	s_infl = inflate_obj(f,flags,N,0,0);           //Inflate skeleton till every point reaches its DT_s value.
	delete f; delete flags;              //(this gives us the simplified object)
	FIELD<float>* s_infl2 = new FIELD<float>(*s_infl);   //---------------------------------------------
	for(i=0;i<s_infl->dimX();i++)            //Compute a mask = the inflated skeleton = the simplified object
		for(j=0;j<s_infl->dimY();j++)
		{
			s_infl->value(i,j)  = (float) ((s_infl->value(i,j) < 0) ? INFINITY : -1);
			s_infl2->value(i,j) = (float) ((s_infl->value(i,j) == -1) ? INFINITY : -1);
		}
		FLAGS* flags_s = new FLAGS(*s_infl,-1);

		//      sprintf(filename,"%f, %d, obj_simpl",do_sk_lev,nSimps);
		//      writePPM(flags_s,filename,1);
		flags = new FLAGS(*flags_s);     //*** 3. COMPUTE DT OF INFLATED (SIMPLIFIED) OBJECT
		FIELD<float>* dt_s = 0;              //(needed for comparing simplified and original objects)
		dt_s = inflate_obj(s_infl,flags,N,0,INFINITY); //This DT is computed BOTH inside and outside the object
		delete s_infl; delete flags;             //First, compute dt_s = DT outside object
		flags = new FLAGS(*s_infl2,-1);          //Next, compute dt_s2 = DT inside object
		FIELD<float>* dt_s2 = 0;
		dt_s2 = inflate_obj(s_infl2,flags,N,0,INFINITY);
		for(i=0;i<dt_s->dimX();i++)              //Finally, compute dt_s = DT inside and outside the object
			for(j=0;j<dt_s->dimY();j++)
				if (dt_s->value(i,j)<=0)
					dt_s->value(i,j) = dt_s2->value(i,j);
		delete s_infl2; delete flags; delete dt_s2;
		for(i=0;i<flags_b->dimX();i++)           //*** 4. COMPUTE DT OF INITIAL OBJECT BOTH INSIDE AND OUTSIDE
			for(j=0;j<flags_b->dimY();j++)            //(we've computed it inside, now compute it outside too)
				if (flags_b->alive(i,j)) flags_b->value(i,j) = FLAGS::FAR_AWAY;
				else if (flags_b->faraway(i,j)) flags_b->value(i,j) = FLAGS::ALIVE;
				flags = new FLAGS(*flags_b);             //The DT of the initial obj is now computed outside, in the
				FIELD<float>* dt_b2 = 0;             //same field (dt_b) as it was computed inside. After this,
				dt_b2 = inflate_obj(dt_b,flags,N,0,INFINITY);      //dt_b will hold the complete DT of the initial obj.
				delete dt_b; delete flags; dt_b = dt_b2;
				//*** 4. COMPUTE DISTANCE BETWEEN ORIGINAL AND SIMPLIFIED OBJECTS
				float d1 = 0;
				likelihood_term_ed(flags_b, dt_b, flags_s, dt_s, d1);
				std::cerr << "Likelihood term Ed = " << d1 << " and number of branches = " << branchesLikelihood.first << "  structural length simplified:  0" << std::endl;
				branchesLikelihood.second = d1;

				double dProcessedLength = 0;

				thresholdAndtheRest.first = int(dProcessedLength);
				thresholdAndtheRest.second = branchesLikelihood;

				differentLikelihoods.Add(thresholdAndtheRest);

				std::pair<bool,double> resultSimplification;
				if(do_sh_len == 0) //means we are only interested in the boundary simplification
				{
					current_terms.number_branches = differentLikelihoods[0].second.first; //branches
					current_terms.likelihood_term = (float)differentLikelihoods[0].second.second; //likelihoodterm
					simplif_pair.first = int(do_sk_lev);
					simplif_pair.second = 0;
					error_terms[simplif_pair] = current_terms;
				}
				if(do_sh_len != 0)
				{
					resultSimplification = tsk->performNextSimplification();
					//      nSimps++;
					if(resultSimplification.first == true)
						dProcessedLength += resultSimplification.second;
				}

				delete dt_s;
				delete flags_b;
				delete flags_s;
				delete dt_b;
				dt_b = NULL;
				//dt_b2, aka, dt_b is being deleted at the begining of next while loop

				//as long as there still is an inside branch that we can remove
				//and we also still have structural simplification to do, because we didn't
				//reach the 2nd simplification threshold yet (do_sh_len)
				while(resultSimplification.first == true && dProcessedLength < do_sh_len)
				{

					flags_b = new FLAGS(*b_flags_b);
					dt_b = new FIELD<float>(*b_dt_b);
					tsk->GetCorrectRadii(dt_b);
					tsk->render(*grad2,1);             //Convert (render) simplified skel to image
					// 		sprintf(filename,"%f, %d, skel_simp",do_sk_lev,nSimps);
					//      		writePPM(grad2,filename,1);
					int number_branch = 0;
					for(i=0;i<tsk->branches.Count();i++)//get the number of branches after the simplification
						number_branch++;
					//current_terms.number_branches = number_branch;
					branchesLikelihood.first = number_branch;
					//delete f; delete flags; delete cnt0;
					f     = new FIELD<float>(*grad2);            //*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
					flags = new FLAGS(*f,*dt_b,-0.9f); 
					FIELD<float>* s_infl = 0;
					s_infl = inflate_obj(f,flags,N,0,0);           //Inflate skeleton till every point reaches its DT_s value.
					delete f; delete flags;              //(this gives us the simplified object)
					FIELD<float>* s_infl2 = new FIELD<float>(*s_infl);   //---------------------------------------------
					for(i=0;i<s_infl->dimX();i++)            //Compute a mask = the inflated skeleton = the simplified object
						for(j=0;j<s_infl->dimY();j++)
						{
							s_infl->value(i,j)  = (float) ((s_infl->value(i,j) < 0) ? INFINITY : -1);
							s_infl2->value(i,j) = (float) ((s_infl->value(i,j) == -1) ? INFINITY : -1);
						}
						FLAGS* flags_s = new FLAGS(*s_infl,-1);

						// 		sprintf(filename,"%f %d obj_simpl",do_sk_lev,nSimps);
						//      		writePPM(flags_s,filename,1);
						flags = new FLAGS(*flags_s);     //*** 3. COMPUTE DT OF INFLATED (SIMPLIFIED) OBJECT
						FIELD<float>* dt_s = 0;              //(needed for comparing simplified and original objects)
						dt_s = inflate_obj(s_infl,flags,N,0,INFINITY); //This DT is computed BOTH inside and outside the object
						delete s_infl; delete flags;             //First, compute dt_s = DT outside object
						flags = new FLAGS(*s_infl2,-1);          //Next, compute dt_s2 = DT inside object
						FIELD<float>* dt_s2 = 0;
						dt_s2 = inflate_obj(s_infl2,flags,N,0,INFINITY);
						for(i=0;i<dt_s->dimX();i++)              //Finally, compute dt_s = DT inside and outside the object
							for(j=0;j<dt_s->dimY();j++)
								if (dt_s->value(i,j)<=0)
									dt_s->value(i,j) = dt_s2->value(i,j);
						delete s_infl2; delete flags; delete dt_s2;
						for(i=0;i<flags_b->dimX();i++)           //*** 4. COMPUTE DT OF INITIAL OBJECT BOTH INSIDE AND OUTSIDE
							for(j=0;j<flags_b->dimY();j++)            //(we've computed it inside, now compute it outside too)
								if (flags_b->alive(i,j)) flags_b->value(i,j) = FLAGS::FAR_AWAY;
								else if (flags_b->faraway(i,j)) flags_b->value(i,j) = FLAGS::ALIVE;
								flags = new FLAGS(*flags_b);             //The DT of the initial obj is now computed outside, in the
								// 		sprintf(filename,"%f %d obj_init",do_sk_lev,nSimps);
								//      		writePPM(flags,filename,1);
								FIELD<float>* dt_b2 = 0;             //same field (dt_b) as it was computed inside. After this,
								dt_b2 = inflate_obj(dt_b,flags,N,0,INFINITY);      //dt_b will hold the complete DT of the initial obj.

								delete dt_b; delete flags; dt_b = dt_b2;
								//*** 4. COMPUTE DISTANCE BETWEEN ORIGINAL AND SIMPLIFIED OBJECTS
								d1 = 0;
								likelihood_term_ed(flags_b, dt_b, flags_s, dt_s, d1);
								std::cerr << "Likelihood term Ed = " << d1 << " and number of branches = " << branchesLikelihood.first << "  structural length simplified: "<< dProcessedLength << std::endl;
								branchesLikelihood.second = d1;
								thresholdAndtheRest.first = int(dProcessedLength);
								thresholdAndtheRest.second = branchesLikelihood;

								differentLikelihoods.Add(thresholdAndtheRest);

								//try next simplification step
								resultSimplification = tsk->performNextSimplification();
								// 		nSimps++;
								if(resultSimplification.first == true)
									dProcessedLength += resultSimplification.second;

								delete dt_s;
								delete flags_b;
								delete flags_s;
								delete dt_b;
				}

				//create all the error terms for this threshold of do_sk_lev using 
				//the different values we stored during the simplification steps
				for(j = 0,i = 0; i < do_sh_len ; i+=5)
				{
					if(j+1 < differentLikelihoods.Count()) //if we can still compare wiht the next dProcessedLength value
					{
						if(i < differentLikelihoods[j+1].first) //we can still use the current value of the thresholdAndtheRest
						{
							current_terms.number_branches = differentLikelihoods[j].second.first; //branches
							current_terms.likelihood_term = (float)differentLikelihoods[j].second.second; //likelihoodterm
							simplif_pair.first = int(do_sk_lev);
							simplif_pair.second = i;
							error_terms[simplif_pair] = current_terms;
						}
						else //we need to move on to the next one. This is possible because j+1 < thresholdAndtheRest.Count()
						{
							current_terms.number_branches = differentLikelihoods[j+1].second.first; //branches
							current_terms.likelihood_term = (float)differentLikelihoods[j+1].second.second; //likelihoodterm
							simplif_pair.first = int(do_sk_lev);
							simplif_pair.second = i;
							error_terms[simplif_pair] = current_terms;
							j++;
						}
					}
					else //there were no more simplifications performed, so we will stick with the one we are dealing wiht right now
					{
						current_terms.number_branches = differentLikelihoods[j].second.first; //branches
						current_terms.likelihood_term = (float)differentLikelihoods[j].second.second; //likelihoodterm
						simplif_pair.first = int(do_sk_lev);
						simplif_pair.second = i;
						error_terms[simplif_pair] = current_terms;
					}
				}
				/*         
				current_terms.number_branches = number_branch;
				simplif_pair.first = int(do_sk_lev);          //Threshold of first simplification
				simplif_pair.second = int(do_sh_len);         //Threshold of second simplification
				error_terms[simplif_pair] = current_terms;*/
				delete tsk;
				delete skel;
				delete grad;
				delete grad2;
				std::cerr << std::endl;

}


/*
This function will set all the points of the given branch to zero in the 
given field. The only point that won't be set to zero is the end point
that is shared with other branches, since removing this branch
would not remove that particular endpoint
*/
void Skeleton::setBranchPointsToZero(TreeSkel::TreeBranch* pTBRemoved, FIELD<float>* pField)
{
	ASSERT((pTBRemoved->end1->type == 1 && pTBRemoved->end2->type != 1) ||
		(pTBRemoved->end1->type != 1 && pTBRemoved->end2->type == 1));

	//Remove the entire branch in the same way it was rendered
	int i;
	TreeSkel::TreePoint* prev = pTBRemoved->end1;

	for(i = 0; i < pTBRemoved->points.Count(); i++)
	{
		TreeSkel::TreePoint* p = pTBRemoved->points[i];
		setLineToZero(pField, prev, p);
		prev = p;
	}

	setLineToZero(pField, prev, pTBRemoved->end2);

	//put the end point that shouldn't be removed back in the skeleton
	if(pTBRemoved->end1->type != 1)
		pField->value(pTBRemoved->end1->i,pTBRemoved->end1->j) = 1;
	else
		pField->value(pTBRemoved->end2->i,pTBRemoved->end2->j) = 1;
}


/*
Set a "line" from 1 point to its neighbor to 0
*/
void Skeleton::setLineToZero(FIELD<float>* pField, TreeSkel::TreePoint* prev, TreeSkel::TreePoint* p)
{
	float l  = sqrt((prev->x - p->x)*(prev->x - p->x) 
		+ (prev->y - p->y)*(prev->y - p->y));

	float dt = 1 / l;

	for(float t = 0; t <= 1; t += dt)
	{
		float x = (1-t) * prev->x + t * p->x;
		float y = (1-t) * prev->y + t * p->y;
		pField->value(int(x), int(y)) = 0;
	}
}

void Skeleton::GetDTByInflation(FIELD<float>* grad2, FIELD<float>* dt_b_b, FLAGS*& flags_s, FIELD<float>*& dt_s)
{
	int i,j;
	FIELD<float>* f = new FIELD<float>(*grad2);            	//*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
	FLAGS* flags = new FLAGS(*f,*dt_b_b,-0.9f); 
	FIELD<float>* s_infl = 0;
	s_infl = inflate_obj(f,flags,10000000/*N*/,0,0);           //Inflate skeleton till every point reaches its DT_s value.
	delete f; 
	delete flags;              				//(this gives us the simplified object)
	FIELD<float>* s_infl2 = new FIELD<float>(*s_infl);   	//---------------------------------------------
	for(i=0;i<s_infl->dimX();i++)            		//Compute a mask = the inflated skeleton = the simplified object
		for(j=0;j<s_infl->dimY();j++)
		{
			s_infl->value(i,j)  = (float)((s_infl->value(i,j) < 0) ? INFINITY : -1);
			s_infl2->value(i,j) = (float)((s_infl->value(i,j) == -1) ? INFINITY : -1);
		}
		flags_s = new FLAGS(*s_infl,-1);
		//	writePPM(flags_s,"inflatedWith1BranchMissing.ppm",1);
		flags = new FLAGS(*flags_s);     			//*** 3. COMPUTE DT OF INFLATED (SIMPLIFIED) OBJECT
		dt_s = 0;    		          			//(needed for comparing simplified and original objects)
		dt_s = inflate_obj(s_infl,flags,10000000/*N*/,0,INFINITY); //This DT is computed BOTH inside and outside the object
		delete s_infl; 
		delete flags;             				//First, compute dt_s = DT outside object
		flags = new FLAGS(*s_infl2,-1);          		//Next, compute dt_s2 = DT inside object
		FIELD<float>* dt_s2 = 0;
		dt_s2 = inflate_obj(s_infl2,flags,10000000/*N*/,0,INFINITY);
		for(i=0;i<dt_s->dimX();i++)              		//Finally, compute dt_s = DT inside and outside the object
			for(j=0;j<dt_s->dimY();j++)
				if (dt_s->value(i,j)<=0)
					dt_s->value(i,j) = dt_s2->value(i,j);
		delete s_infl2; 
		delete flags; 
		delete dt_s2;
}

void Skeleton::GetDTByInflation(FIELD<float>* grad2, FIELD<float>* dt_b_b, FLAGS*& flags_s)
{
	int i,j;
	FIELD<float>* f = new FIELD<float>(*grad2);            	//*** 2. COMPUTE SIMPLFIED-OBJECT BY INFLATING SIMPLIFIED-SKELETON
	FLAGS* flags = new FLAGS(*f,*dt_b_b,-0.9f); 
	FIELD<float>* s_infl = 0;

	s_infl = inflate_obj(f,flags,10000000/*N*/,0,0);           //Inflate skeleton till every point reaches its DT_s value.

	delete f; 
	delete flags;              				//(this gives us the simplified object)

	for(i=0;i<s_infl->dimX();i++)            		//Compute a mask = the inflated skeleton = the simplified object
		for(j=0;j<s_infl->dimY();j++)
			s_infl->value(i,j)  = (float)((s_infl->value(i,j) < 0) ? INFINITY : -1);

	flags_s = new FLAGS(*s_infl,-1);

	delete s_infl;
}

/*
This function will calculate the reconstruction error of a given
skeleton matching it against the initial object
*/
double Skeleton::GetReconstructionError(FIELD<float>* grad2, FIELD<float>* dt_b_b, 
										/*FIELD<float>* pCompleteDTInitial, FLAGS* pRealInitialFlags,*/ ShapeDiff& sd)
{
	//FIELD<float>* dt_s;
	FLAGS* flags_s;
	float d1 = 0;

	//GetDTByInflation(grad2, dt_b_b, flags_s, dt_s);
	GetDTByInflation(grad2, dt_b_b, flags_s);

	d1 = (float)sd.ComputeError(*flags_s);

	//likelihood_term_ed(pRealInitialFlags, pCompleteDTInitial, flags_s, dt_s, d1);

	//std::cerr << "\nError = " << d1 << std::endl;

	//delete dt_s;

	delete flags_s;

	return d1;
}


double Skeleton::getOptimumStructural(DARRAY< LengthBrnchError > CompleteDimension2)
{
	LengthBrnchError result = CompleteDimension2[0];
	double dMinError = (result.nBrnch * params.nBrnchWeightStr) + (params.nRecErrorWeightStr * result.dError), dCurrent = 0, dLength = 0;
	int i;

	for(i = 1; i < CompleteDimension2.Count(); i++)
	{
		dCurrent = (CompleteDimension2[i].nBrnch * params.nBrnchWeightStr) + (params.nRecErrorWeightStr * CompleteDimension2[i].dError);
		if(dMinError > dCurrent)
		{
			dMinError = dCurrent;
			dLength = CompleteDimension2[i].dLength;
		}
	}

	std::cerr << "CHOSEN SIMPLIFICATION LENGTH: " << dLength << std::endl;

	return dLength;
}




/*
Returns the optimum boundary simplification option given the weighting factors
*/
TauBrnchErrorIndices Skeleton::getOptimumBoundary(DARRAY< TauBrnchErrorIndices > CompleteDimension)
{
	TauBrnchErrorIndices result = CompleteDimension[0];
	double dMinError = (result.nBrnch * params.nBrnchWeightBnd) + (params.nRecErrorWeightBnd * result.dError), dCurrent = 0;
	//	std::cerr << std::endl << "Reconstruction error : " << result.dError << " branches: " << result.nBrnch << " weight for branches: " << params.nBrnchWeightBnd;
	// 	std::cerr << "\n-----Number of branches " << result.nBrnch << " rec error " << result.dError << " Total error: " << dMinError << std::endl;
	int i;

	//check until 1 from end of array (0 was taken as initial minimum)
	for(i = 1; i < CompleteDimension.Count(); i++)
	{
		dCurrent = (CompleteDimension[i].nBrnch * params.nBrnchWeightBnd) + (params.nRecErrorWeightBnd * CompleteDimension[i].dError);
		//		std::cerr << std::endl << "Reconstruction error : " << CompleteDimension[i].dError << " branches: " << CompleteDimension[i].nBrnch << " weight for branches: " << params.nBrnchWeightBnd;
		//		std::cerr << std::endl << "Reconstruction error : " << CompleteDimension[i].dError;
		if( dMinError > dCurrent )
		{
			result = CompleteDimension[i];
			dMinError = dCurrent;
			// 			std::cerr << "ERROR LOWERED TO " << i << std::endl;
			// 			std::cerr << "\n-----Number of branches " << CompleteDimension[i].nBrnch << " rec error " << CompleteDimension[i].dError << " Total error: " << dMinError << std::endl;
		}
	}

	//ASSERT(false)

	return result;
}

std::pair< DARRAY< int >, double > Skeleton::calculate_errors_new_boundary(float do_sk_lev, FLAGS* b_flags_b,FLAGS* b_flags,FIELD<float>* b_dt_b,FIELD<float>* b_cnt0,FIELD<float>* b_grad,FIELD<float>* b_skel, FIELD<float>* b_f,int i,int j,int N)
{
	std::cerr << "----------------- Optimizing boundary ----------------\n";      

	FLAGS* flags_b = new FLAGS(*b_flags_b);
	FLAGS* flags   = new FLAGS(*b_flags);
	FIELD<float>* dt_b = new FIELD<float>(*b_dt_b);
	FIELD<float>* cnt0 = new FIELD<float>(*b_cnt0);
	FIELD<float>* grad = new FIELD<float>(*b_grad);
	FIELD<float>* skel = new FIELD<float>(*b_skel);
	FIELD<float>* f    = new FIELD<float>(*b_f);
	comp_grad(flags,cnt0,grad);
	//Threshold the field on a fixed value namely 20  
	postprocess2(grad,SKEL_TAU/*do_sk_lev*/);
	FIELD<float>* grad2 = new FIELD<float>(*grad);

	//do_sh_len now specifies the max value of structural simplification we are interested in.
	//we first turn the grad2 field into a graph structure
	//then when we have the graph structure we can acces information
	//about inside and outside branches allowing us to remove individual
	//branches from the graph 

	ThinningMethod thm(grad2,dt_b,-0.9f);       //Make boundary-skel 1-pixel-thin by DT-order thinning
	FIELD<int>* thin_skel = thm.thin();    


	TreeSkel* tsk = new TreeSkel(thin_skel);       //Reduce 1-pixel-thin skel to graph structure
	tsk->AssignRadius(*dt_b);
	tsk->GetShockGraphParams(dMinSlope, dMinError, dMaxAccelChg);
	tsk->GetCorrectRadii(dt_b);
	tsk->render(*grad2,1);             //Convert (render) simplified skel to image

	//###############################################################################################
	//######################## FIRST PART; BOUNDARY SIMPLIFICATION ##################################
	//###############################################################################################

	//Calculate the reconstruction error for our initial
	//threshold which is fixed on 20
	//FIELD<float>* pAlteredSkel = new FIELD<float>(*grad2);
	FIELD<float>* pAlteredSkel = new FIELD<float>(*b_grad);
	comp_grad(flags,cnt0,pAlteredSkel);
	postprocess2(pAlteredSkel,2/*do_sk_lev*/);
	FIELD<float>* pAlteredDt = new FIELD<float>(*dt_b);
	//---------------------------------
	FIELD<float>* pCompleteDTInitial;
	FLAGS* pRealInitialFlags;

	GetDTByInflation(pAlteredSkel, pAlteredDt, pRealInitialFlags, pCompleteDTInitial);
	delete pAlteredSkel;
	delete pAlteredDt;

	pAlteredSkel = new FIELD<float>(*grad2);
	pAlteredDt = new FIELD<float>(*dt_b);

	ShapeDiff sd(*pAlteredSkel, pRealInitialFlags, pCompleteDTInitial);

	double dRecError = GetReconstructionError(pAlteredSkel, pAlteredDt, /*pCompleteDTInitial, pRealInitialFlags,*/ sd);
	// 	std::cerr << "Reconstruction error initial: " << dRecError << std::endl;
	delete pAlteredSkel;
	delete pAlteredDt;

	DARRAY< int > allIndices;
	DARRAY< TauBrnchErrorIndices > CompleteDimension;
	TauBrnchErrorIndices currentTerms;
	currentTerms.nBrnch = tsk->branches.Count();
	currentTerms.dError = dRecError;
	currentTerms.dTau = 0;
	currentTerms.indices = allIndices;
	CompleteDimension.Add(currentTerms);

	//double dInitRecError = dRecError;

	//Now we have the grad2 field once again, no simplification done,
	//but we also have a graph structure of the skeleton, namely
	//tsk, which we will use to remove individual outside branche
	//DARRAY< BrnchIndexError > ErrorPerBranch;
	std::vector< BrnchIndexError > ErrorPerBranch;
	BrnchIndexError current;
	if(tsk->branches.Count() > 1)
	{
		for(i = 0; i < tsk->branches.Count(); i++)
		{
			//for all outside branches
			//if(tsk->branches[i]->external())
			if((tsk->branches[i]->end1->type == 1 && tsk->branches[i]->end2->type != 1) ||
				(tsk->branches[i]->end1->type != 1 && tsk->branches[i]->end2->type == 1))
			{
				FIELD<float>* pAlteredSkel = new FIELD<float>(*grad2);
				FIELD<float>* dt = new FIELD<float>(*dt_b);
				setBranchPointsToZero(tsk->branches[i], pAlteredSkel);
				dRecError = GetReconstructionError(pAlteredSkel, dt, /*pCompleteDTInitial, pRealInitialFlags,*/ sd);
				//due to our error approx. we sometimes find no error for removing a branch. Clearly this is due
				//to our approx. and we have to account for that.
				if(dRecError < APROX_ERROR_FIX) dRecError = APROX_ERROR_FIX;
				//   				std::cerr << "Reconstruction error : " << dRecError << " for branch: " << *tsk->branches[i] <<std::endl;
				current.nIndex = i;
				current.dError = dRecError;
				//ErrorPerBranch.Add(current);
				ErrorPerBranch.push_back(current);
				delete pAlteredSkel;
				delete dt;
			}
		}
	}

	// Sort branches based on their reconstruction error
	std::sort(ErrorPerBranch.begin(), ErrorPerBranch.end());

	int nBranches = tsk->branches.Count();
	double dErr, dSumError = 0;
	allIndices.Init(0);

	//determine the number of branches and the total amount of error
	//given a certain Tau as upperbound for the amout of error allowed
	//per removal of 1 branch
	for(unsigned int iSpec = 0; iSpec < ErrorPerBranch.size(); iSpec++)
	{
		dErr = ErrorPerBranch[iSpec].dError;

		// If multiple branches have the same error we must remove them all together
		do {	
			dSumError += dErr;
			nBranches--;
			allIndices.AddOrd(ErrorPerBranch[iSpec].nIndex);
			iSpec++;
		} while (iSpec < ErrorPerBranch.size() && dErr == ErrorPerBranch[iSpec].dError);

		iSpec--; // must compensate for the ++ operation within the loop

		currentTerms.nBrnch = nBranches;
		currentTerms.dError = dSumError;
		currentTerms.dTau = dErr;
		currentTerms.indices = allIndices;
		CompleteDimension.Add(currentTerms);

		//		DBG_MSG2("Num branches and tau = ", nBranches, dErr)
	}

	// 	for(double iSpec = 0; iSpec < params.dTauUpperBound; iSpec += params.dTauStep /*0.0001*/)
	// 	{
	// 		allIndices.Init(0);
	// 		nBranches = tsk->branches.Count();
	// 		dSumError = 0;
	// 		for(j = 0; j < ErrorPerBranch.Count(); j++)
	// 		{
	// 			if(ErrorPerBranch[j].dError <= iSpec)
	// 			{
	// 				nBranches--;
	// 				dSumError += ErrorPerBranch[j].dError;
	// 				allIndices.AddOrd(ErrorPerBranch[j].nIndex);
	// 			}
	// 		}
	// 		currentTerms.nBrnch = nBranches;
	// 		currentTerms.dError = dSumError + dInitRecError;
	// 		currentTerms.dTau = iSpec;
	// 		currentTerms.indices = allIndices;
	// 		CompleteDimension.Add(currentTerms);
	// 	}

	// 	for(i = 0; i < CompleteDimension.Count(); i++)
	// 	{
	// 		double error = CompleteDimension[i].nBrnch * 3 + 300 * CompleteDimension[i].dError;
	// 		std::cerr << "Iteration " << i << " error " << error << 
	// 			" branches: " <<CompleteDimension[i].nBrnch << std::endl;
	// 	}

	//READY TO CALCULATE OPTIMUM!!
	//make sure to return the "currentTerms" struct at the chosen optimum so we know what to remove etc.
	currentTerms = getOptimumBoundary(CompleteDimension);
	//REMOVE THE BRANCHES FROM THE AttributedGraph THAT THE OPTIMUM SUGGESTED
	//for(i = 0; i < currentTerms.indices.Count(); i++)
	//{
	std::cerr << "Going to remove outside branches" << std::endl;
	tsk->RemoveOutsideBranch(currentTerms.indices);
	//}
	//BOUNDARY SIMPLIFIED SKELETON IS NOW DONE


	delete thin_skel;
	delete f; 
	delete flags; 
	delete cnt0;

	std::cerr << "----------------- Optimizing structure ----------------\n" << std::endl;  

	//###############################################################################################
	//######################## SECOND PART; STRUCTURAL SIMPLIFICATION ###############################
	//###############################################################################################

	//empty array we used before
	CompleteDimension.Init(0);

	//We need to calculate the initial error we are dealing with because of the boundary simplification
	//This initial error should be substracted from all the errors we find next

	//########################## ADDED JANUARI 09 ########################################################   

	dt_b = new FIELD<float>(*b_dt_b);
	tsk->GetCorrectRadii(dt_b);
	tsk->render(*grad2,1);             //Convert (render) simplified skel to image
	dRecError = GetReconstructionError(grad2, dt_b, /*pCompleteDTInitial, pRealInitialFlags,*/ sd);
	double dErrorAfterBndry = dRecError;
	std::cerr << "Reconstruction error after the boundary simplification: " << dErrorAfterBndry << std::endl;
	//delete dt_b;

	//####################################################################################################   

	DARRAY< LengthBrnchError > CompleteDimension2;
	LengthBrnchError currentStruc;

	//structural simplification starts with the values we ended up chosing for the boundary simplification
	currentStruc.nBrnch = currentTerms.nBrnch;
	currentStruc.dError = 0;//currentTerms.dError;
	currentStruc.dLength = 0;

	CompleteDimension2.Add(currentStruc);

	std::pair<bool,double> resultSimplification;

	bool bInsideBranchRemoved = false;
	double dProcessedLength = 0;
	resultSimplification = tsk->performNextSimplification();
	if(resultSimplification.first == true)
	{	
		bInsideBranchRemoved = true;
		dProcessedLength += resultSimplification.second;
	}

	delete flags_b;
	delete dt_b;

	while(bInsideBranchRemoved == true)
	{
		dt_b = new FIELD<float>(*b_dt_b);
		tsk->GetCorrectRadii(dt_b);
		tsk->render(*grad2,1);             //Convert (render) simplified skel to image
		int number_branch = 0;
		for(i = 0; i < tsk->branches.Count(); i++)//get the number of branches after the simplification
			number_branch++;

		currentStruc.nBrnch = number_branch;
		dRecError = GetReconstructionError(grad2, dt_b, /*pCompleteDTInitial, pRealInitialFlags,*/ sd);
		//due to our error approx. we sometimes find no error for removing a branch. Clearly this is due
		//to our approx. and we have to account for that.
		if(dRecError < APROX_ERROR_FIX) dRecError = APROX_ERROR_FIX;
		dRecError -= dErrorAfterBndry;
		std::cerr << "Reconstruction error : " << dRecError << std::endl;
		currentStruc.dError = dRecError;
		currentStruc.dLength = dProcessedLength;
		CompleteDimension2.Add(currentStruc);

		delete dt_b;
		//try next simplification step
		resultSimplification = tsk->performNextSimplification();
		if(resultSimplification.first == true)
		{
			bInsideBranchRemoved = true;
			dProcessedLength += resultSimplification.second;
		}
		else //resultSimplification.first == false
			bInsideBranchRemoved = false;
	}

	// 	dt_b = new FIELD<float>(*b_dt_b);
	//      	tsk->GetCorrectRadii(dt_b);
	//      	tsk->render(*grad2,1);             //Convert (render) simplified skel to image
	//      	int number_branch = 0;
	//      	for(i = 0; i < tsk->branches.Count(); i++)//get the number of branches after the simplification
	//      		 number_branch++;
	//      		
	// 	currentStruc.nBrnch = number_branch;
	// 	dRecError = getReconstructionError(grad2, dt_b, pCompleteDTInitial, pRealInitialFlags, sd);
	// 	std::cerr << "Reconstruction error : " << dRecError << std::endl;
	// 	currentStruc.dError = dRecError;
	// 	currentStruc.dLength = dProcessedLength;
	// 	CompleteDimension2.Add(currentStruc);

	// 	delete dt_b;


	for(i = 0; i < CompleteDimension2.Count(); i++)
		std::cerr << CompleteDimension2[i].dError << std::endl;

	double dOptimumProcLength = getOptimumStructural(CompleteDimension2);

	// 	ASSERT(false);
	//      	//create all the error terms for this threshold of do_sk_lev using 
	//      	//the different values we stored during the simplification steps
	//      	for(j = 0,i = 0; i < do_sh_len ; i+=5)
	//      	{
	//      		if(j+1 < CompleteDimension2.Count()) //if we can still compare wiht the next dProcessedLength value
	// 		{
	// 			if(i < CompleteDimension2[j+1].dLength) //we can still use the current value of the thresholdAndtheRest
	// 			{
	// 				current_terms.number_branches = differentLikelihoods[j].second.first; //branches
	// 				current_terms.likelihood_term = differentLikelihoods[j].second.second; //likelihoodterm
	// 				simplif_pair.first = int(do_sk_lev);
	// 				simplif_pair.second = i;
	// 				error_terms[simplif_pair] = current_terms;
	// 			}
	// 			else //we need to move on to the next one. This is possible because j+1 < thresholdAndtheRest.Count()
	// 			{
	// 				current_terms.number_branches = differentLikelihoods[j+1].second.first; //branches
	// 				current_terms.likelihood_term = differentLikelihoods[j+1].second.second; //likelihoodterm
	// 				simplif_pair.first = int(do_sk_lev);
	// 				simplif_pair.second = i;
	// 				error_terms[simplif_pair] = current_terms;
	// 				j++;
	// 			}
	// 		}
	// 		else //there were no more simplifications performed, so we will stick with the one we are dealing wiht right now
	// 		{
	// 			current_terms.number_branches = differentLikelihoods[j].second.first; //branches
	// 			current_terms.likelihood_term = differentLikelihoods[j].second.second; //likelihoodterm
	// 			simplif_pair.first = int(do_sk_lev);
	// 			simplif_pair.second = i;
	// 			error_terms[simplif_pair] = current_terms;
	// 		}
	//      	}

	std::pair< DARRAY< int >, double > BndStrucResults;
	BndStrucResults.first = currentTerms.indices;
	BndStrucResults.second = dOptimumProcLength;

	delete pCompleteDTInitial;
	delete pRealInitialFlags;

	delete tsk;
	delete skel;
	delete grad;
	delete grad2;
	std::cerr << std::endl;   
	return BndStrucResults;
}


