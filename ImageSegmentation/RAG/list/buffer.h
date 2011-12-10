/*
 * buffer.h
 *
 * Buffer zum zwischenzeiltichen manipulieren von Daten
 *
 * Sven Wachsmuth, 4.8.00
 */
#ifndef LS_BUFFER_H
#define LS_BUFFER_H

#include <pthread.h>
#include "list.h"

#define lsBuffer_active 0x1
#define lsBuffer_state  (lsBuffer_active)

#define lsBuffer_isActive(s) ((s) & lsBuffer_active)
#define lsBuffer_setActive(s) ((s) = (s) | lsBuffer_active)
#define lsBuffer_notActive(s) ((s) = (s) & ~lsBuffer_active)
#define lsBuffer_xorActive(s) ((s) = (s) ^ lsBuffer_active)

/** Typen fuer Argument-Funktionen */
typedef void *(lsBuffer_Copy_t)(void *);
typedef void  (lsBuffer_Free_t)(void *);
typedef int   (lsBuffer_Index_t)(void *);

typedef struct {
  lsPt_t  new;     /* Daten aus letztem set-Commando */
  lsPt_t  orig;    /* Daten aus new bei letztem (re)set-Commando (update==1) */
  lsPt_t  copy;    /* Kopien von orig bei letztem reset-Commando */
  lsInt_t status;  /* Status von copy-Eintraegen (active) */
  int     update;  /* update-Flag */

  /** Mutexe muessen IMMER in der Reihenfolge New,Orig,Copy gelockt werden */
  pthread_mutex_t mutexNew;
  pthread_mutex_t mutexOrig;
  pthread_mutex_t mutexCopy;

  lsBuffer_Copy_t *_copy;
  lsBuffer_Free_t *_free;
} lsBuffer_t;

lsBuffer_t *lsBuffer_create(lsBuffer_Copy_t *_copy, lsBuffer_Free_t *_free);
lsBuffer_t *lsBuffer_init(lsBuffer_t *b,
			  lsBuffer_Copy_t *_copy, lsBuffer_Free_t *_free);
void        lsBuffer_destroy(lsBuffer_t *b);
void        lsBuffer_free(lsBuffer_t *b);

/** Funktionen auf der `set'-Seite (new) */
#define     lsBuffer_update(b) ((b)->update)
lsBuffer_t *lsBuffer_setUpdate(lsBuffer_t *b);
lsBuffer_t *lsBuffer_unsetUpdate(lsBuffer_t *b);
lsBuffer_t *lsBuffer_lock4set(lsBuffer_t *b);
lsBuffer_t *lsBuffer_unlock4set(lsBuffer_t *b);
void       *lsBuffer_set(lsBuffer_t *b, int index, void *obj);
void        *lsBuffer_setLock(lsBuffer_t *b, int index, void *obj);
lsBuffer_t *_lsBuffer_lsSet(lsBuffer_t *b,lsPt_t *objs,lsBuffer_Index_t *Index,
			    int free);
#define     lsBuffer_lsSet(b,objs,Index) _lsBuffer_lsSet(b,objs,Index,1)
#define     lsBuffer_lsSetPt(b,objs,Index) _lsBuffer_lsSet(b,objs,Index,0)
lsBuffer_t *_lsBuffer_clean(lsBuffer_t *b, int free);
#define     lsBuffer_clean(b) _lsBuffer_clean(b,1)
#define     lsBuffer_cleanPt(b) _lsBuffer_clean(b,0)

/** Funktionen auf `orig' */
void       *lsBuffer_reset(lsBuffer_t *b, int index, int _update);
void       *lsBuffer_resetLock(lsBuffer_t *b, int index, int _update);
lsBuffer_t *_lsBuffer_resetAll(lsBuffer_t *b, int free);
#define     lsBuffer_resetAll(b) _lsBuffer_resetAll(b,1)
#define     lsBuffer_resetAllPt(b) _lsBuffer_resetAll(b,0)
lsBuffer_t *lsBuffer_lock4reset(lsBuffer_t *b, int _update);
lsBuffer_t *lsBuffer_unlock4reset(lsBuffer_t *b, int _update);

/** Funktionen auf der `get'-Seite (copy) */
#define     lsBuffer_getActivity(b,index) \
              ((b) ? lsBuffer_isActive(LS_GET_CHECK(&(b)->status,index,0)):0) 
int         lsBuffer_flipActive(lsBuffer_t *b, int index);
int         lsBuffer_activate(lsBuffer_t *b, int index);
int         lsBuffer_deactivate(lsBuffer_t *b, int index);
lsBuffer_t *lsBuffer_activateAll(lsBuffer_t *b);
lsBuffer_t *lsBuffer_deactivateAll(lsBuffer_t *b);
void       *lsBuffer_get(lsBuffer_t *b, int index);
lsPt_t     *lsBuffer_lsGet(lsPt_t *ls, lsBuffer_t *b);
void       *lsBuffer_getCopyLock(lsBuffer_t *b, int index);
void       *lsBuffer_setCopyUnlock(lsBuffer_t *b, int index, void *obj);
lsBuffer_t *lsBuffer_lock4get(lsBuffer_t *b);
lsBuffer_t *lsBuffer_unlock4get(lsBuffer_t *b);

#define LSBUFFER_FORALL_ITEMS(b,i) \
  LS_FORALL_ITEMS(&(b)->copy,i) if (LS_GET(&(b)->copy,i))
#define LSBUFFER_GET(b,i) \
  LS_GET(&(b)->copy,i)

#endif
