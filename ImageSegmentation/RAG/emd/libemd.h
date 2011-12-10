#ifndef _EMD_H
#define _EMD_H

#ifdef __cplusplus
extern "C" {
#endif

/*
    emd.h

    Last update: 3/24/98

    An implementation of the Earth Movers Distance.
    Based of the solution for the Transportation problem as described in
    "Introduction to Mathematical Programming" by F. S. Hillier and 
    G. J. Lieberman, McGraw-Hill, 1990.

    Copyright (C) 1998 Yossi Rubner
    Computer Science Department, Stanford University
    E-Mail: rubner@cs.stanford.edu   URL: http://vision.stanford.edu/~rubner
*/


/* DEFINITIONS */
#define MAX_SIG_SIZE   500
#define MAX_ITERATIONS 500
#define INFINITY       1e20
#define EPSILON        1e-6


/*****************************************************************************/
/* feature_t SHOULD BE MODIFIED BY THE USER TO REFLECT THE FEATURE TYPE      */
//typedef int feature_t;
/*****************************************************************************/

typedef struct {
int X,Y,Z;
}emd_feature_t;

typedef struct
{
  int n;                /* Number of features in the signature */
  emd_feature_t *Features;  /* Pointer to the features vector */
  float *Weights;       /* Pointer to the weights of the features */
} emd_signature_t;


typedef struct
{
  int from;             /* Feature number in signature 1 */
  int to;               /* Feature number in signature 2 */
  float amount;         /* Amount of flow from "from" to "to" */
} emd_flow_t;


float emd(emd_signature_t *Signature1, emd_signature_t *Signature2,
	  float (*func)(emd_feature_t *, emd_feature_t *),
	  emd_flow_t *Flow, int *FlowSize);

float emd_normalized(emd_signature_t *Signature1, emd_signature_t *Signature2,
	  float (*Dist)(emd_feature_t *, emd_feature_t *),
	  emd_flow_t *Flow, int *FlowSize);

void emd_signature_free(emd_signature_t *sig);
void emd_signature_destroy(emd_signature_t *sig);

#ifdef __cplusplus
}
#endif


#endif
