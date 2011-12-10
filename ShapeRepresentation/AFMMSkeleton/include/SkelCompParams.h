#ifndef _SKEL_COMP_PARAMS_H_
#define _SKEL_COMP_PARAMS_H_

#include "field.h"

struct SkelCompParams
{
	const char* pInputfile;     //<! Set either the file name (this field) or pInputField
	FIELD<float>* pInputField;  //<! Set either the a field pointer (this field) or pInputfile
	float fThresSimp1;
	float fThresSimp2;
	bool bOnlyBound;
	bool bOnlyStruc;
	bool bDo2DField;
	float fSimp1UpperBound;
	float fSimp1StepSize;
	float fSimp2UpperBound;
	float fSimp2StepSize;
	bool bGiveSimpleSkel;
	double dMinSlope;
	double dMinError;
	double dMaxAccelChg;
	int nBrnchWeightBnd;	//=1
	int nBrnchWeightStr;	//=1
	int nRecErrorWeightBnd;	//=250
	int nRecErrorWeightStr;	//=400
	int nBndrySmoothIter;
	
	/*!
			@TODO remove the parameters that not used anymore, which is most of them

			1 char* input file name
			2 float& start threshold boundary simplification
			3 float& start threshold structural simplification
			4 int& (0 or 1) indicating not to do a 1D boundary search, or to actually do it
			5 int& (0 or 1) indicating not to do a 1D structural search, or to actually do it
			6 float& upperbound for threshold boundary simplification
			7 float& upperbound for threshold structural simplification
			8 float& stepsize for the threshold for boundary simplification
			9 float& stepsize for the threshold for structural simplification
			10 bool give a unsimpified skeleton or not
			Params in sgparams: {dSkTreshold}
			sk_upper, sk_step, sh_upper, sh_step, compDerivSkel
	*/
	SkelCompParams()
	{
		pInputfile = NULL;
		pInputField = NULL;
		nBrnchWeightBnd = 1;
		nBrnchWeightStr = 1;
		nRecErrorWeightBnd = 500;
		nRecErrorWeightStr = 500;
		fThresSimp2 = 20;

		// These params are not used any more, but double check before removing them
		nBndrySmoothIter = 5; // no need to smooth anymore, so this param can go
		fThresSimp1 = 0;
		bOnlyBound = false;
		bOnlyStruc = false;
		bDo2DField = false;
		fSimp1UpperBound = 71;
		fSimp1StepSize = 5;
		fSimp2UpperBound = 101;
		fSimp2StepSize = 5;
		bGiveSimpleSkel = false;
		dMinSlope = 0.05;
		dMinError = 2.0;
		dMaxAccelChg = 0.5;
	}
};
		
#endif //_SKEL_COMP_PARAMS_H_
