/*
 * list/string_dict.c
 *
 * dictionary of strings
 *
 * Sven Wachsmuth, 16.7.2003
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "string_dict.h"

lsPt_t *strDict_splitLine(lsPt_t *pWords, lsChar_t *line, char delim)
{
  lsChar_t word;
  int i;
  if (!line) return (pWords);
  if (!pWords)
    pWords = lsPt_Nil();

  for (lsChar_cpyPt(lsChar_nil(&word),line);
       (i = lsChar_getFstIndex(&word,delim)) >= 0;
       lsChar_dropPt(&word,i+1)) {
    lsPt_add(pWords, LS_ITEMS(&word));
  }
  return (pWords);
}

strDict_t *strDict_Nil(void)
{
  return (strDict_nil(NULL));
}

strDict_t *strDict_nil(strDict_t *dict)
{
  if (!dict)
    dict = (strDict_t *) rs_malloc(sizeof(strDict_t),"strDict_t");
  
  lsChar_nil(&dict->cWords);
  lsInt_nil(&dict->iWords);
  lsInt_nil(&dict->iWords_sorted);
  lsInt_nil(&dict->isorted2index);

  return (dict);
}

void strDict_free(strDict_t *dict)
{
  if (!dict) return;
  lsChar_free(&dict->cWords);
  lsInt_free(&dict->iWords);
  lsInt_free(&dict->iWords_sorted);
  lsInt_free(&dict->isorted2index);
}

void strDict_Free(strDict_t *dict)
{
  if (!dict) return;
  strDict_free(dict);
  free(dict);
}

strDict_t *strDict_cpy(strDict_t *copy, strDict_t *dict)
{
  if (!dict) return (copy);
  if (!copy)
    copy = strDict_Nil();

  lsChar_cpy(&copy->cWords, &dict->cWords);
  lsInt_cpy(&copy->iWords, &dict->iWords);
  lsInt_cpy(&copy->iWords_sorted, &dict->iWords_sorted);
  lsInt_cpy(&copy->isorted2index, &dict->isorted2index);

  return (copy);
}

strDict_t *strDict_Cpy(strDict_t *dict)
{
  return (strDict_cpy(NULL,dict));
}

strDict_t *strDict_clear(strDict_t *dict)
{
  if (!dict) return (dict);
  
  lsChar_setNil(&dict->cWords);
  lsInt_setNil(&dict->iWords_sorted);
  lsInt_setNil(&dict->iWords);
  lsInt_setNil(&dict->isorted2index);
  
  return (dict);
}

int _strDict_checkWord(int *iWord, strDict_t *dict, char *pWord)
{
  lsChar_t word;
  int index, isorted, _iWord;
  pairPt_t arg;
  if (!dict || !pWord || !*pWord) return (-1);

  lsChar_setPt(lsChar_nil(&word), strlen(pWord)+1, pWord);
  _iWord = LS_N_CHECK(&(dict->cWords));
  /*fprintf(stderr,"_strDict_checkWord: iWord: %d\n",_iWord);*/

  lsChar_cat(&(dict->cWords), &word);

  isorted = lsInt_bsearchLtFunc_2(&dict->iWords_sorted, _iWord, 
				  (lsInt_cmp_2_t *) lsChar_cmpIndexPt,
				  pairPt_set(&arg, &dict->cWords, strcmp));
  /*fprintf(stderr,"_strDict_checkWord: isorted: %d\n",isorted);*/

  if (iWord) (*iWord) = _iWord;
  return (isorted);
}

int strDict_word2index(strDict_t *dict, char *pWord)
{
    triplePt_t arg;
    int isorted
	= lsInt_bsearchLtFunc_2(&dict->iWords_sorted, 0, 
				(lsInt_cmp_2_t *) lsChar_cmpValue2IndexPt,
				triplePt_set(&arg,&dict->cWords,strcmp,pWord));
    return (isorted >= 0) ? LS_GET(&dict->isorted2index,isorted) : -1;
}

int strDict_insert(strDict_t *dict, char *pWord)
{
  int isorted, index, iWord;
  if (!dict || !pWord || !*pWord) return (-1);

  isorted = _strDict_checkWord(&iWord, dict, pWord);

  if (isorted < 0) {
    isorted = -(isorted+1);

    /*fprintf(stderr,"strDict_insert: isorted: %d\n",isorted);*/

    lsInt_insert(&dict->iWords_sorted, isorted, iWord, -1);
    index = LS_N(&dict->iWords);
    lsInt_add(&dict->iWords, iWord);
    lsInt_insert(&dict->isorted2index, isorted, index, -1);

  } else {
    LS_N(&dict->cWords) = iWord;
    index = LS_GET(&dict->isorted2index,isorted);
  }
  return (index);
}

strDict_t *strDict_insertLine(lsInt_t *indices, strDict_t *dict, char *line)
{
  char *pWord, *pLine;
  if (!line) return (dict);
  if (!dict)
    dict = strDict_Nil();

  if (indices) lsInt_setNil(indices);

  for (pWord=line; (pLine=strpbrk(pWord," \t\n,;.!?")); pWord = pLine+1) {
    int index;
    char t = (*pLine);
    (*pLine) = '\0';

    index = strDict_insert(dict, pWord);
    /*fprintf(stderr,"strDict_insertLine: pWord: '%s' [%d]\n",pWord,index);*/
    
    if (indices && index >= 0) lsInt_add(indices,index);
    (*pLine) = t;
  }
  if (*pWord) {
    int index;
    /*fprintf(stderr,"strDict_insertLine: pWord: '%s'\n",pWord);*/
    index = strDict_insert(dict, pWord);
    if (indices) lsInt_add(indices,index);
  }
  return (dict);
}
    

int strDict_getIndex(strDict_t *dict, char *pWord)
{
  int isorted, iWord, index=-1;

  if (!dict || !pWord || !*pWord) return (-1);

  isorted = _strDict_checkWord(&iWord, dict, pWord);

  LS_N(&dict->cWords) = iWord;
  if (isorted >= 0)
    index = LS_GET(&dict->isorted2index,isorted);

  return (index);
}


char *strDict_getWord(strDict_t *dict, int index)
{
  int iWord;
  if (!dict) return (NULL);

  iWord = LS_GET_CHECK(&dict->iWords,index,-1);

  return (iWord >= 0) ? &LS_GET(&dict->cWords,iWord) : NULL;
}

int strDict_size(strDict_t *dict)
{
  return (dict) ? LS_N(&dict->iWords) : 0;
}

