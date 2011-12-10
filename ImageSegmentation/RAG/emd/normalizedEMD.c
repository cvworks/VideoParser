#include <stdlib.h>
#include <stdio.h>
#include "libemd.h"

void emd_signature_free(emd_signature_t *sig)
{
    if (!sig) return;
    if (sig->Features) free(sig->Features);
    if (sig->Weights) free (sig->Weights);
}

void emd_signature_destroy(emd_signature_t *sig)
{
    if (!sig) return;
    emd_signature_free(sig);
    free(sig);
}

float emd_normalized(emd_signature_t *Signature1, emd_signature_t *Signature2,
		     float (*Dist)(emd_feature_t *, emd_feature_t *),
		     emd_flow_t *Flow, int *FlowSize){
	
    float *Weigths;
    float sw1,sw2,nf;
    int itr;
    Weigths = Signature1->Weights;
    sw1 =0.0;
    for (itr=0;itr<Signature1->n;itr++){
	sw1+=Weigths[itr];
    }
    Weigths = Signature2->Weights;
    sw2=0.0;
    for (itr=0;itr<Signature2->n;itr++){
	sw2+=Weigths[itr];
    }
    if (sw1 == 0.0 || sw2 == 0.0) return (INFINITY);

    fprintf(stderr,"emd_normalized: weights %g %g.\n", sw1, sw2);

    if (sw1>sw2){
	Weigths = Signature1->Weights;
	nf = sw1/sw2;
	for (itr=0;itr<Signature1->n;itr++){
	    Weigths[itr]= Weigths[itr]/nf;
	}
    }
    if (sw1<sw2){
	Weigths = Signature2->Weights;
	nf = sw2/sw1;
	for (itr=0;itr<Signature2->n;itr++){
	    Weigths[itr]= Weigths[itr]/nf;
	}
    }
    
    return emd(Signature1,Signature2,Dist, Flow,FlowSize);
}


