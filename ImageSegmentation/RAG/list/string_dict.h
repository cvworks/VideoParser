/*
 * list/string_dict.h
 *
 * dictionary of strings
 *
 * Sven Wachsmuth, 16.7.2003
 */
#ifndef _LS_STRDICT_T
#define _LS_STRDICT_T

#ifdef __cplusplus
extern "C" {
#endif

#include "list.h"

typedef struct strDict {
  lsChar_t cWords;
  lsInt_t  iWords_sorted;
  lsInt_t  iWords;
  lsInt_t  isorted2index;
} strDict_t;

strDict_t *strDict_Nil(void);
strDict_t *strDict_nil(strDict_t *dict);
void       strDict_free(strDict_t *dict);
void       strDict_Free(strDict_t *dict);

strDict_t *strDict_cpy(strDict_t *copy, strDict_t *dict);
strDict_t *strDict_Cpy(strDict_t *dict);

strDict_t *strDict_clear(strDict_t *dict);
int        strDict_insert(strDict_t *dict, char *pWord);
strDict_t *strDict_insertLine(lsInt_t *indices, strDict_t *dict, char *line);
int        strDict_getIndex(strDict_t *dict, char *pWord);
char *     strDict_getWord(strDict_t *dict, int index);
int        strDict_size(strDict_t *dict);
int        strDict_word2index(strDict_t *dict, char *pWord);

lsPt_t    *strDict_splitLine(lsPt_t *pWords, lsChar_t *line, char delim);

#ifdef __cplusplus
}
#endif

#endif
