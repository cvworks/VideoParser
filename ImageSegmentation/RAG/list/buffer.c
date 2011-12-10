/*
 * buffer.c
 *
 * Buffer zum zwischenzeiltichen manipulieren von Daten
 *
 * Sven Wachsmuth, 4.8.00
 */
#include <rs/memory.h>
#include <rs/messages.h>
#include "buffer.h"

lsBuffer_t *lsBuffer_create(lsBuffer_Copy_t *_copy, lsBuffer_Free_t *_free)
{
  return (lsBuffer_init(NULL, _copy, _free));
}

lsBuffer_t *lsBuffer_init(lsBuffer_t *b, 
			  lsBuffer_Copy_t *_copy, lsBuffer_Free_t *_free)
{
  int err;

  if (!b)
    b = rs_malloc(sizeof(lsBuffer_t),"lsBuffer");

  lsPt_nil(&b->new);
  lsPt_nil(&b->orig);
  lsPt_nil(&b->copy);
  lsInt_nil(&b->status);
  b->update = 1;

  if ((err = pthread_mutex_init(&b->mutexNew, NULL)) != 0) {
    rs_error("lsBuffer_init: error initializing mutex: %d.", err);
  }
  if ((err = pthread_mutex_init(&b->mutexOrig, NULL)) != 0) {
    rs_error("lsBuffer_init: error initializing mutex: %d.", err);
  }
  if ((err = pthread_mutex_init(&b->mutexCopy, NULL)) != 0) {
    rs_error("lsBuffer_init: error initializing mutex: %d.", err);
  }
  if (!_copy || !_free)
    rs_error("lsBuffer_init: no copy or free function specified.");

  b->_copy = _copy;
  b->_free = _free;
  
  return (b);
}

void lsBuffer_destroy(lsBuffer_t *b)
{
  if (!b) return;

  lsBuffer_free(b);
  rs_free(b);
}

void lsBuffer_free(lsBuffer_t *b)
{
  if (!b) return;

  if (b->_free) {
    int i;
    
    LS_FORALL_ITEMS(&b->new,i) {
      void *obj  = LS_GET(&b->new,i);
      void *_obj = LS_GET(&b->orig,i);

      /** ACHTUNG evtl. gleicher Pointer in orig wie in new ? */
      if (obj != _obj) (*b->_free)(obj);
    }
    lsPt_map(&b->orig,(lsPt_map_t *) b->_free);
    lsPt_map(&b->copy,(lsPt_map_t *) b->_free);
  }
  lsPt_free(&b->new);
  lsPt_free(&b->orig);
  lsPt_free(&b->copy);
  lsInt_free(&b->status);

  pthread_mutex_destroy(&b->mutexNew);
  pthread_mutex_destroy(&b->mutexOrig);
  pthread_mutex_destroy(&b->mutexCopy);
}

/*
 * Funktionen auf der `set'-Seite (new) 
 */

lsBuffer_t *lsBuffer_setUpdate(lsBuffer_t *b)
{ 
  if (!b) return(b);
  b->update = 1;
  return (b);
}

lsBuffer_t *lsBuffer_unsetUpdate(lsBuffer_t *b)
{
  if (!b) return(b);
  b->update = 0;
  return (b);
}

lsBuffer_t *lsBuffer_lock4set(lsBuffer_t *b)
{
  if (!b) return;
  pthread_mutex_lock(&b->mutexNew);
  return (b);
}

lsBuffer_t *lsBuffer_unlock4set(lsBuffer_t *b)
{
  if (!b) return;
  pthread_mutex_unlock(&b->mutexNew);
  return (b);
}

/*
 * lsBuffer_set: setzt neuen Eintrag in Liste `new' und gibt einen
 *   eventuell nicht mehr im Buffer (`new' oder `orig') enthaltenen Eintrag 
 *   zurueck.
 */
void *lsBuffer_set(lsBuffer_t *b, int index, void *obj)
{
  void *_obj;
  if (!b) return (obj);

  /** Setze neues Objekt bei new-Liste ... */
  _obj  = lsPt_setIndex(&b->new,index,obj,NULL);
  if (_obj) {
    void *__obj = LS_GET_CHECK(&b->orig,index,NULL);
    /** ... und gebe altes Objekt aus new-Liste zurueck, 
	falls nicht in orig-Liste */
    if (_obj == __obj) _obj = NULL;
  }
  return (_obj);
}

/*
 * lsBuffer_setLock: setzt neuen Eintrag in der waehrenddessen blockierten
 *   Liste `new' und gibt einen eventuell nicht mehr im Buffer (`new' oder 
 *   `orig') enthaltenen Eintrag zurueck.
 */
void *lsBuffer_setLock(lsBuffer_t *b, int index, void *obj)
{
  void *_obj;
  if (!b) return (obj);

  pthread_mutex_lock(&b->mutexNew);
  _obj = lsBuffer_set(b,index,obj);
  pthread_mutex_unlock(&b->mutexNew);
  return (_obj);
}

lsBuffer_t *_lsBuffer_lsSet(lsBuffer_t *b,lsPt_t *objs,lsBuffer_Index_t *Index,
			    int free /* Speicherfreigabe alter Elemente ? */)
{
  int i;
  if (!b) return (b);

  pthread_mutex_lock(&b->mutexNew);

  if (free) {
    /** alte new-Eintraege loeschen, die nicht in der orig-Liste stehen */
    LS_FORALL_ITEMS(&b->new,i) {
      void *_obj;
      if (!(_obj = LS_GET(&b->new,i))) continue;
      if (_obj == LS_GET_CHECK(&b->orig,i,NULL)) continue;
      
      (*b->_free)(_obj);
    }
  }
  lsPt_setNil(&b->new);

  /** setze neue Daten */
  LS_FORALL_ITEMS(objs,i) {
    void *obj = LS_GET(objs,i);
    lsBuffer_set(b, (Index) ? (*Index)(obj) : i, obj);
  }
  pthread_mutex_unlock(&b->mutexNew);
  return (b);
}
  
lsBuffer_t *_lsBuffer_clean(lsBuffer_t *b, 
			    int free /* Speicherfreigabe alter Elemente ? */)
{
  if (!b) return (b);
  pthread_mutex_lock(&b->mutexNew);
  if (free) {
    int i;
    /** alte new-Eintraege loeschen, die nicht in der orig-Liste stehen */
    LS_FORALL_ITEMS(&b->new,i) {
      void *_obj;
      if (!(_obj = LS_GET(&b->new,i))) continue;
      if (_obj == LS_GET_CHECK(&b->orig,i,NULL)) continue;
      
      (*b->_free)(_obj);
    }
  }
  lsPt_setNil(&b->new);
  pthread_mutex_unlock(&b->mutexNew);
}

/*
 * Funktionen auf `orig'
 */

/*
 * lsBuffer_reset: setzt die Kopie in `copy' auf `orig' zurueck.
 *   Falls `_update==1' wird vorher das `orig'-Element durch das 
 *   `new'-Element ersetzt. Ein eventuell nicht mehr enthaltenes 
 *   `orig'-Element wird zurueckgegeben.
 */
void *lsBuffer_reset(lsBuffer_t *b, int index, int _update)
{
  void *_obj, *__obj = NULL;
  if (!b) return (NULL);
  if (_update) {
    _obj = LS_GET_CHECK(&b->new,index,NULL);
    if (_obj || LS_EXISTS(&b->orig, index)) {
      __obj = lsPt_setIndex(&b->orig,index,_obj,NULL);
      /** Falls identischer Eintrag in `new' und `orig', keine Rueckgabe */
      if (__obj == _obj) __obj = NULL;
    }
  } else
    _obj = LS_GET_CHECK(&b->orig,index,NULL);

  if (_obj || LS_EXISTS(&b->copy,index)) {
    void *_obj_ = lsPt_setIndex(&b->copy, index, 
				(_obj) ? (*b->_copy)(_obj) : NULL, NULL);
    lsInt_setIndex(&b->status,index,(_obj) ? lsBuffer_state : 0x0, 0x0);
    /** alte Kopie aus `copy' freigeben */
    if (_obj_) (*b->_free)(_obj_);
  }
  return (__obj);
}

/*
 * lsBuffer_reset: setzt die Kopie in `copy' auf `orig' zurueck.
 *   Falls `_update==1' wird vorher das `orig'-Element durch das 
 *   `new'-Element ersetzt. Ein eventuell nicht mehr enthaltenes 
 *   `orig'-Element wird zurueckgegeben.
 */
void *lsBuffer_resetLock(lsBuffer_t *b, int index, int _update)
{
  void *_obj, *__obj = NULL;
  if (!b) return (NULL);

  /** Zugriffe auf `new' (_update==1) und `orig'-Liste blockieren */
  if (_update)
    pthread_mutex_lock(&b->mutexNew);
  pthread_mutex_lock(&b->mutexOrig);

  if (_update) {
    _obj = LS_GET_CHECK(&b->new,index,NULL);
    if (_obj || LS_EXISTS(&b->orig, index)) {
      __obj = lsPt_setIndex(&b->orig,index,_obj,NULL);
      /** Falls identischer Eintrag in `new' und `orig', keine Rueckgabe */
      if (__obj == _obj) __obj = NULL;
    }
    /** Zugriff auf `new'-Liste wieder freigeben */
    pthread_mutex_unlock(&b->mutexNew);
  } else
    _obj = LS_GET_CHECK(&b->orig,index,NULL);

  if (_obj || LS_EXISTS(&b->copy,index)) {
    void *_obj_;
    /** Zugriff auf `copy'-Liste blockieren */
    pthread_mutex_lock(&b->mutexCopy);
    _obj_ = lsPt_setIndex(&b->copy, index, 
			  (_obj) ? (*b->_copy)(_obj):NULL,NULL);
    lsInt_setIndex(&b->status,index,(_obj) ? lsBuffer_state:0x0,0x0);

    /** Zugriff auf `copy'-Liste wieder freigeben */
    pthread_mutex_unlock(&b->mutexCopy);
    /** alte Kopie aus `copy' freigeben */
    if (_obj_) (*b->_free)(_obj_);
  }
  /** Zugriff auf `orig'-Liste wieder freigeben */
  pthread_mutex_unlock(&b->mutexOrig);

  return (__obj);
}

lsBuffer_t *_lsBuffer_resetAll(lsBuffer_t *b, int free)
{
  int i,n;
  int _update = b->update;
  if (!b) return (b);
  if (_update)
    pthread_mutex_lock(&b->mutexNew);
  pthread_mutex_lock(&b->mutexOrig);
  pthread_mutex_lock(&b->mutexCopy);
  n = ((_update && LS_N(&b->new) > LS_N(&b->orig)) ? 
       LS_N(&b->new) : LS_N(&b->orig)); 
  for (i=0; i < n; i++) {
    void *_obj = lsBuffer_reset(b,i,_update);
    if (free && _obj) (*b->_free)(_obj);
  }
  pthread_mutex_unlock(&b->mutexCopy);
  pthread_mutex_unlock(&b->mutexOrig);
  if (_update)
    pthread_mutex_unlock(&b->mutexNew);
  return (b);
}

lsBuffer_t *lsBuffer_lock4reset(lsBuffer_t *b, int _update)
{
  if (!b) return (b);
  
  if (_update)
    pthread_mutex_lock(&b->mutexNew);
  pthread_mutex_lock(&b->mutexOrig);
  pthread_mutex_lock(&b->mutexCopy);
  return (b);
}

lsBuffer_t *lsBuffer_unlock4reset(lsBuffer_t *b, int _update)
{
  if (!b) return(b);
  if (_update)
    pthread_mutex_unlock(&b->mutexNew);
  pthread_mutex_unlock(&b->mutexOrig);
  pthread_mutex_unlock(&b->mutexCopy);
  return (b);
}

/*
 * Funktionen auf der `get'-Seite (copy) 
 */
int lsBuffer_flipActive(lsBuffer_t *b, int index)
{
  /** Bezieht sich immer auf aktuelle Kopie */
  if (!b) return (-1);
  if (LS_GET_CHECK(&b->copy,index,NULL)) {
    return (lsBuffer_isActive(lsBuffer_xorActive(LS_GET(&b->status,index))));
  }
  return (-1);
}
  
int lsBuffer_activate(lsBuffer_t *b, int index)
{
  /** Bezieht sich immer auf aktuelle Kopie */
  if (!b) return (-1);
  if (LS_GET_CHECK(&b->copy,index,NULL)) {
    return (lsBuffer_isActive(lsBuffer_setActive(LS_GET(&b->status,index))));
  }
  return (-1);
}
  
int lsBuffer_deactivate(lsBuffer_t *b, int index)
{
  /** Bezieht sich immer auf aktuelle Kopie */
  if (!b) return (-1);
  if (LS_GET_CHECK(&b->copy,index,NULL)) {
    return (lsBuffer_isActive(lsBuffer_notActive(LS_GET(&b->status,index))));
  }
  return (-1);
}
  
lsBuffer_t *lsBuffer_activateAll(lsBuffer_t *b)
{
  int i;
  if (!b) return (b);
  LS_FORALL_ITEMS(&b->copy,i) {
    lsBuffer_activate(b,i);
  }
  return (b);
}
 
lsBuffer_t *lsBuffer_deactivateAll(lsBuffer_t *b)
{
  int i;
  if (!b) return (b);
  LS_FORALL_ITEMS(&b->copy,i) {
    lsBuffer_deactivate(b,i);
  }
  return (b);
}

void *lsBuffer_get(lsBuffer_t *b, int index)
{
  void *_obj;
  int active;
  if (!b) return (NULL);
  _obj = LS_GET_CHECK(&b->copy,index,NULL);
  active = lsBuffer_isActive(LS_GET_CHECK(&b->status,index,0x0));
  return ((_obj && active) ? b->_copy(_obj) : NULL);
}

lsPt_t *lsBuffer_lsGet(lsPt_t *ls, lsBuffer_t *b)
{
  int i;
  if (!ls)
    ls = lsPt_Nil();
  else
    lsPt_setNil(ls); /** ACHTUNG: evtl.e Elemente werden nicht freigegeben */
  
  if (!b) return (ls);

  pthread_mutex_lock(&b->mutexCopy);
  LS_FORALL_ITEMS(&b->copy,i) {
    void *_obj = LS_GET(&b->copy,i);
    int active = lsBuffer_isActive(LS_GET(&b->status,i));
    if (!_obj || !active) continue;
    lsPt_add(ls,b->_copy(_obj));
  }
  pthread_mutex_unlock(&b->mutexCopy);
  return (ls);
}

void *lsBuffer_getCopyLock(lsBuffer_t *b, int index)
{
  if (!b) return (NULL);
  lsBuffer_unsetUpdate(b);
  pthread_mutex_lock(&b->mutexCopy);
  return (LS_GET_CHECK(&b->copy,index,NULL));
}

void *lsBuffer_setCopyUnlock(lsBuffer_t *b, int index, void *obj)
{
  void *_obj;
  if (!b) {
    rs_warning("lsBuffer_setCopyUnlock: no copy data for updating.");
    return (obj);
  }
  _obj = LS_GET_CHECK(&b->copy,index,NULL);
  if (_obj) {
    LS_GET(&b->copy,index) = obj;
  } else {
    _obj = obj;
    rs_warning("lsBuffer_setCopyUnlock: no copy data for updating.");
  }
  pthread_mutex_unlock(&b->mutexCopy);
  return (_obj);
}
  
lsBuffer_t *lsBuffer_lock4get(lsBuffer_t *b)
{
  if (!b) return (b);
  pthread_mutex_lock(&b->mutexCopy);
  return (b);
}

lsBuffer_t *lsBuffer_unlock4get(lsBuffer_t *b)
{
  if (!b) return (b);
  pthread_mutex_unlock(&b->mutexCopy);
  return (b);
}

