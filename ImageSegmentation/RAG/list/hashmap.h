/*
    hashmap.h

    Last update: 6/02/04

    An implemantation of a hashmap for storing PairLong_t's.
    The first long is used as the key, the second as the value.

    Frank Frenser
    Applied Computer Science, University of Bielefeld
    E-Mail: ffrenser@techfak.uni-bielefeld.de

    30/6/04 Sven Wachsmuth: Adapted to liblist.a
*/
#ifndef LS_HASHMAP_H
#define LS_HASHMAP_H

#include "list.h"
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ls_hashmap_t {
	int hash;					/*The size of the hash-index*/
	lsPairLong_t* hashPT;				/*Array of lists of elements*/
	int size;					/*Number of elements stored in the hashmap*/
	int (*hashfuncPT)(long int key,int hash);	/*Function to compute the hash-index of a key*/
} ls_hashmap_t;

#define ls_hashmap_size(hm) ((hm)->size)


ls_hashmap_t* 
ls_hashmap_init (ls_hashmap_t *hm, int hash, 
		 int (*hashfuncPT)(long int key,int hash));
ls_hashmap_t* 
ls_hashmap_create (int hash, 
		   int (*hashfuncPT)(long int key,int hash));
/*Creates a new hashmap. 'hash' is the initialized size of the hash-index. The second parameter must be a function
to compute the hash-index. It gets the original key and the size of the hash-index as paremeters and returns the computed hash-index*/

ls_hashmap_t *ls_hashmap_initSTD (ls_hashmap_t *hm);
ls_hashmap_t *ls_hashmap_createSTD (void);
/*Also creates a hashmap, but with a standard size and hash-function*/

ls_hashmap_t *ls_hashmap_clear (ls_hashmap_t *hm);
ls_hashmap_t *ls_hashmap_copy (ls_hashmap_t* target,
			       ls_hashmap_t* source);
ls_hashmap_t *ls_hashmap_dup (ls_hashmap_t *source);

int ls_hashmap_put (ls_hashmap_t* hm, 
		    long int key, 
		    long int value);
/*Adds a PairLong_t build from 'key' and 'value' to the hashmap 'hm'. 'key' acts as the hash key.*/

int ls_hashmap_delete (ls_hashmap_t * hm,long int key);
/*Deletes the first element with the 'key' from 'hm' onetimes. returns 1 if delete was succesfull and 0 otherwise*/

int ls_hashmap_deleteAll (ls_hashmap_t * hm, long int key);
/*As function above, but deletes all elemts with that key*/


int ls_hashmap_exists (ls_hashmap_t* hm, 
		       long int key); 
int ls_hashmap_get (ls_hashmap_t* hm, 
		    long int key, 
		    pairLong_t * output);
long int ls_hashmap_getValue(ls_hashmap_t* hm, 
			     long int key, long int undef);
/*Searches for element with key 'pix' in hashmap 'hm' and writes it to 'output'. Returns 1 if element was found and 0 otherwise*/

int ls_hashmap_reorganize (ls_hashmap_t* hm);

void ls_hashmap_destroy(ls_hashmap_t *hm);
void ls_hashmap_free(ls_hashmap_t *hm);
/*Destroys the hashmap 'hm' and returns memory.*/

#ifdef __cplusplus
}
#endif

#endif
