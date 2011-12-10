/*#include "treeskel.h"
#include "dqueue.h"
#include "genrl.h"
#include "../dagmatcher/BasicUtils.h"
//#include "../dagmatcher/SmartArray.h"

#define _STANDARD_
#define use_namespace 
#include <newmatap.h>                // need matrix applications
#include <newmatio.h>                // need matrix output routines

#define Matrix NEWMAT::Matrix

// Matrix A(3,2); 
// Real a[] = { 11,12,21,22,31,33 }; 
// A << a;

//typedef SmartMatrix<int> Matrix;
*/


void FixField(FIELD<int>* s)
{
	int i, j;
	int pat1[] = {0, 0, 1, 
			    0, 1, 1, 
			    0, 0, 1};
	int sol1[] = {0, 0, 1, 0, 0, 1, 0, 0, 1};
	//Matrix patMat, solMat;
	SkelRepair sr;
	
	sr.Add(pat1, sol1, 3, 3);
}