/*
    hashmap.c

    Last update: 6/08/04

    An implemantation of a hashmap for storing PairLong_t's.
    The first long is used as the key, the second as the value.

    Frank Frenser
    Applied Computer Science, University of Bielefeld
    E-Mail: ffrenser@techfak.uni-bielefeld.de

    30/6/04 Sven Wachsmuth: Adapted to liblist.a
*/
#include "tuple.h" 
#include "hashmap.h"

#define LS_HASH_STD (29)
#define LS_HASH_REORG_STD (120)

/* local functions */

int ls_hashfunction (long int key, int hash );
lsPairLong_t *ls_hashmap_getListEntry (ls_hashmap_t* hm, 
				       long int key,
				       int *index);

/* function implementations */

int ls_hashmap_delete (ls_hashmap_t * hm,long int key)
{
    int i;
    lsPairLong_t* list = ls_hashmap_getListEntry(hm, key, &i);
    if (list) {
	lsPairLong_delSwap(list,i);
	(hm->size)--;
	return (1);
    } else
	return (0);
}

int ls_hashmap_deleteAll (ls_hashmap_t * hm, long int key)
{
    int i=0;
    while (ls_hashmap_delete(hm,key)) i++;
    return i;
}


ls_hashmap_t* 
ls_hashmap_create (int hash, 
		   int (*hashfuncPT)(long int key,int hash))
{
    return (ls_hashmap_init(NULL, hash, hashfuncPT));
}
    
ls_hashmap_t* 
ls_hashmap_init (ls_hashmap_t *hm,
		 int hash, 
		 int (*hashfuncPT)(long int key,int hash))
{
    int i;
    if (!hm)
	hm = (ls_hashmap_t*) malloc(sizeof(ls_hashmap_t));

    hm->hash = hash;
    hm->size = 0;
    hm->hashPT = (lsPairLong_t*) calloc (hash, sizeof(lsPairLong_t));
    hm->hashfuncPT = hashfuncPT;
    for (i=0;i <hash;i++){
	lsPairLong_nil(&hm->hashPT[i]);
    }
    return hm;
}

ls_hashmap_t *ls_hashmap_createSTD (void)
{
    return (ls_hashmap_initSTD(NULL));
}

ls_hashmap_t *ls_hashmap_initSTD (ls_hashmap_t *hm)
{
    return (ls_hashmap_init(hm,LS_HASH_STD,ls_hashfunction));
}

ls_hashmap_t *ls_hashmap_clear (ls_hashmap_t *hm)
{
    int i;
    if (!hm) return (NULL);
    for (i=0; i < hm->hash; i++) {
	lsPairLong_setNil(&hm->hashPT[i]);
    }
    hm->size = 0;
    
    return (hm);
}

ls_hashmap_t *ls_hashmap_dup (ls_hashmap_t *source)
{
    int i;
    ls_hashmap_t *target;
    if (!source) return (NULL);
    
    target = ls_hashmap_create(source->hash, source->hashfuncPT);
    for (i=0; i < source->hash; i++) {
	lsPairLong_cpy(&source->hashPT[i], &target->hashPT[i]);
    }
    target->size = source->size;

    return (target);
}

ls_hashmap_t *ls_hashmap_copy (ls_hashmap_t* target,
			       ls_hashmap_t* source)
{
    int i ;

    if (!target)
	return ls_hashmap_dup (source);
    else
	ls_hashmap_clear(target);

    if (!source)
	return (target);

    for (i=0;i< source->hash;i++){
	lsPairLong_t* old_list;
	int j;
	old_list = &source->hashPT[i];
	LS_FORALL_ITEMS(old_list,j) {
	    ls_hashmap_put (target, pairLong_getFst(&LS_GET(old_list,j),0) ,pairLong_getSnd(&LS_GET(old_list,j),0));
	}
    }
    return target;
}


int ls_hashmap_reorganize (ls_hashmap_t* hm)
{
    if (((hm->hash*100)/(hm->size+1))<=LS_HASH_REORG_STD){
	ls_hashmap_t hm_new;
	int i;
	ls_hashmap_init (&hm_new, hm->hash*2-1,hm->hashfuncPT);
	for (i=0;i<hm->hash;i++){
	    lsPairLong_t* old_list;
	    int j;
	    old_list = &hm->hashPT[i];
	    LS_FORALL_ITEMS(old_list,j) {
		ls_hashmap_put (&hm_new, 
					tplFst(&LS_GET(old_list,j)),
					tplSnd(&LS_GET(old_list,j)));
	    }
	}
	ls_hashmap_free (hm);
	memcpy(hm, &hm_new, sizeof(ls_hashmap_t));
	return 1;
    }
    return 0;
}

int ls_hashmap_put (ls_hashmap_t* hm, 
			    long int key, long int value)
{
    pairLong_t *input;
    long int hmindex;
    if (!hm) return (0);
    //ls_hashmap_reorganize (hm);
    hmindex = ((hm->hashfuncPT)) (key,hm->hash);
    input = lsPairLong_getNewItem(&(hm->hashPT[hmindex]));
    pairLong_set(input,key,value);
    
    hm->size++;
    return (hm->size);
}

int ls_hashmap_exists (ls_hashmap_t* hm, long int key)
{
    return ls_hashmap_get (hm, key, NULL);
}

lsPairLong_t *
ls_hashmap_getListEntry(ls_hashmap_t* hm, 
				long int key,
				int *index)
{
    int i;
    lsPairLong_t* list;
    int hmindex;
    if (!hm) return (NULL);

    hmindex = (*hm->hashfuncPT)(key, hm->hash);
    list = &(hm->hashPT [hmindex]);
    LS_FORALL_ITEMS(list,i) {
	if (tplFst(&LS_GET(list,i)) == key){
	    if (index)
		*index = i;
	    return list;
	}
    }
    return NULL;
}
    
int ls_hashmap_get (ls_hashmap_t* hm, 
		    long int key, 
		    pairLong_t * output)
{
    int i;
    lsPairLong_t* list = ls_hashmap_getListEntry(hm, key, &i);

    if (!list) 
	return (0);
    if (output)
	*output = LS_GET(list,i);
    return (1);
}

long int ls_hashmap_getValue(ls_hashmap_t* hm, 
			     long int key, long int undef)
{
    int i;
    lsPairLong_t* list = ls_hashmap_getListEntry(hm, key, &i);
    return (list) ? tplSnd(&LS_GET(list,i)) : undef;
}

int ls_hashfunction (long int key, int hash ){
	return (key%hash);
}

void ls_hashmap_free(ls_hashmap_t *hm)
{
    int i;
    if (!hm) return;
    for (i=0;i<hm->hash;i++){
	lsPairLong_free(&(hm->hashPT[i]));
    }
    free(hm->hashPT);
}

void ls_hashmap_destroy(ls_hashmap_t *hm)
{
    if (!hm) return;
    ls_hashmap_free(hm);
    free(hm);
}


