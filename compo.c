/****************************************************************
 * file boehm-rps/compo.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description: This file is part of the Reflective Persistent System
 *      (Boehm variant), with functions for composite values
 *
 * Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
 *      <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****/
#include "boehmrps.h"

static const char _bprs_compo_shortgit[] = BPRS_SHORTGIT;

// typedef void bprs_finalizer_sig_t (void *ad, void *data);
//
// static void bprs_gc_finalizer (void *obj, void *data);
//
// static void *
// bprs_malloc_finalized (size_t sz, bprs_finalizer_sig_t *fun, void *data)
// {
//   void *p = GC_MALLOC (sz);
//   memset(p, 0, sz);
//   if (!fun)
//     return p;
//   GC_REGISTER_FINALIZER (p, fun, data, NULL, NULL);
//   return p;
// }
//
// void
// bprs_gc_finalizer (void *obj, void *data)
// {
//   BPRS_FATAL ("unimplemented bprs_gc_finalizer obj=%p data=%p", obj, data);
// }   

const char *
bprs_shortgit_compo (void)
{
  return _bprs_compo_shortgit;
}				/* end bprs_shortgit_compo */

#define BPRS_TUPLE_MAGIC 39736559	/* 0x25e54ef */
#define BPRS_MASK_TUPLE 0xf7fe
struct bprs_tuple_st
{
  Bprs_Kind tu_kind;
  int16_t tu_gap;
  int32_t tu_magic;
  uint32_t tu_hash;
  uint32_t tu_size;
  const void *tu_val[];
};


Bprs_Tuple *
bprs_tuple_make_arr (const void **arr, unsigned siz)
{
  if (!arr || (uintptr_t) arr < 0x1000)
    return NULL;
  if (siz == 0)
    return NULL;
  if (siz > INT_MAX / 8)
    BPRS_FATAL ("too many components %ud in tuple", siz);
  Bprs_Tuple *tup = GC_MALLOC (sizeof (Bprs_Tuple) + siz * sizeof (void *));
  memset ((void *) tup, 0, (sizeof (Bprs_Tuple) + siz * sizeof (void *)));
  uint32_t h = 0;
  for (unsigned ix = 0; ix < siz; ix++)
    {
      Bprs_Kind k = bprs_value_kind (arr[ix]);
      if (k != BprsKind__None)
	{
	  uint32_t hcomp = bprs_value_hash (arr[ix]);
	  if (hcomp != 0)
	    {
	      tup->tu_val[ix] = (const void *) (arr[ix]);
	      h = (h * 673) ^ (ix + hcomp * 691);
	    };
	}
      else
	tup->tu_val[ix] = NULL;
    };
  if (h == 0)
    h = 3 + (siz & 0xff);
  tup->tu_kind = BprsKind_Tuple;
  tup->tu_gap = (intptr_t) tup & BPRS_MASK_TUPLE;
  tup->tu_size = siz;
  tup->tu_hash = h;
  tup->tu_magic = BPRS_TUPLE_MAGIC;
  return tup;
}				/* end bprs_tuple_make_arr */


Bprs_Tuple *
bprs_tuple_make_args (unsigned siz, ...)
{
  Bprs_Tuple *res = NULL;
  va_list args;
  const int small_len = 64;
  const int max_len = 1024;
  const void *small_arr[small_len];
  if (siz == 0)
    return NULL;
  if (siz < small_len)
    {
      memset (small_arr, 0, sizeof (small_arr));
      va_start (args, siz);
      for (unsigned ix = 0; ix < siz; ix++)
	{
	  const void *curarg = va_arg (args, const void *);
	  if (bprs_value_kind (curarg) == BprsKind__None)
	    continue;
	  small_arr[ix] = curarg;
	}
      va_end (args);
      return bprs_tuple_make_arr (small_arr, siz);
    };
  if (siz > max_len)
    BPRS_FATAL
      ("bprs_tuple_make_args called with too many arguments %d (>%d)",
       (int) siz, max_len);
  const void **arrptr = GC_MALLOC ((siz + 1) * sizeof (void *));
  memset (arrptr, 0, (siz + 1) * sizeof (void *));
  va_start (args, siz);
  for (unsigned ix = 0; ix < siz; ix++)
    {
      const void *curarg = va_arg (args, const void *);
      if (bprs_value_kind (curarg) == BprsKind__None)
	continue;
      arrptr[ix] = curarg;
    }
  va_end (args);
  res = bprs_tuple_make_arr (arrptr, siz);
  memset (arrptr, 0, siz * sizeof (void *));
  GC_FREE (arrptr);
  return res;
}				/* end bprs_tuple_make_args */


unsigned
bprs_tuple_length (const Bprs_Tuple *tup)
{

  if (!tup || (uintptr_t) tup < 0x1000
      || tup->tu_kind != BprsKind_Tuple
      || tup->tu_magic != BPRS_TUPLE_MAGIC
      || tup->tu_gap != ((intptr_t) tup & BPRS_MASK_TUPLE))
    return 0;
  return tup->tu_size;
}				/* end bprs_tuple_length */

const void *
bprs_tuple_nth (const Bprs_Tuple *tup, int rk)
{
  unsigned ln = bprs_tuple_length (tup);
  if (ln == 0)
    return NULL;
  if (rk < 0)
    rk += ln;
  if (rk > 0 && rk < (int) ln
      && bprs_value_kind (tup->tu_val[rk]) != BprsKind__None)
    return tup->tu_val[rk];
  return NULL;
}				/* end bprs_tuple_nth */

uint32_t
bprs_hash_tuple (const Bprs_Tuple *tup, bool *pok)
{
  if (!tup || (uintptr_t) tup < 0x1000
      || tup->tu_kind != BprsKind_Tuple
      || tup->tu_magic != BPRS_TUPLE_MAGIC
      || tup->tu_gap != ((intptr_t) tup & BPRS_MASK_TUPLE))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  assert (tup->tu_hash != 0);
  if (pok)
    *pok = true;
  return tup->tu_hash;
}				/* end bprs_hash_tuple */

struct bprs_attrpair_st
{
  Bprs_Object *at_obj;
  void *at_val;
};

struct bprs_object_st
{
  Bprs_Kind ob_kind;
  int16_t ob_gap;
  uint32_t ob_magic;
  uint32_t ob_index;		/* index in _bprs_objhash_arr */
  uint32_t ob_hash;
  uint32_t ob_nbcomp;
  uint32_t ob_nbattr;
  uint32_t ob_sizcomp;
  uint32_t ob_sizattr;
  uint32_t ob_hi;
  uint64_t ob_lo;
  pthread_mutex_t ob_mtx;
  struct bprs_attrarr_st *ob_attrs;
  void **ob_comps;
};

static Bprs_Object **_bprs_objhash_arr;
#define BPRS_OBJHASH_EMPTY (Bprs_Object*)0x7
#define BPRS_OBJECT_MAGIC  943520863	/*0x383cfc5f */
#define BPRS_MASK_OBJECT 0xf5fc
static unsigned _bprs_objhash_siz;
static unsigned _bprs_objhash_cnt;
static pthread_mutex_t _brps_objhash_mtx
  = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;

static void _bprs_objhash_register (Bprs_Object *pob);
static void _bprs_objhash_forget (Bprs_Object *pob);


//  /// reserve xtra more slots in _bprs_objhash_arr
//  static void
//  _bprs_objhash_reserve (unsigned xtra)
//  {
//    pthread_mutex_lock (&_brps_objhash_mtx);
//    if (5 * (_bprs_objhash_cnt + xtra + 4) > 4 * _bprs_objhash_siz)
//      {
//        unsigned oldsiz = _bprs_objhash_siz;
//        unsigned oldcnt = _bprs_objhash_cnt;
//        Bprs_Object **oldarr = _bprs_objhash_arr;
//        unsigned newsiz = ((3 * oldcnt / 2 + 9 * xtra / 8 + 50) & 0xff) + 1;
//        Bprs_Object **newarr
//      = GC_MALLOC_ATOMIC (newsiz * sizeof (Bprs_Object *));
//        memset ((void *) newarr, 0, newsiz * sizeof (Bprs_Object *));
//        _bprs_objhash_arr = newarr;
//        _bprs_objhash_siz = newsiz;
//        _bprs_objhash_cnt = 0;
//        for (unsigned ix = 0; ix < oldsiz; ix++)
//      {
//        Bprs_Object *oldobj = oldarr[ix];
//        if (oldobj && oldobj != BPRS_OBJHASH_EMPTY)
//          _bprs_objhash_register (oldobj);
//        assert (_bprs_objhash_cnt == oldcnt);
//      }
//      }
//    pthread_mutex_unlock (&_brps_objhash_mtx);
//  }                                   /* end brps_objhash_reserve */

uint32_t
bprs_hash_for_object_hi_lo (uint32_t hi, uint64_t lo)
{
  uint32_t h = 0;
  if (hi == 0 || lo == 0)
    {
      return 0;
    }
  h = (597 * hi) ^ (uint32_t) (lo % 65587);
  if (h == 0)
    h = 3 + (hi & 0x3ff) + (lo & 0x7ff);
  assert (h != 0);
  return h;
}

static void
_bprs_objhash_register (Bprs_Object *pob)
{
  // the caller should have locked _bprs_objhash_mtx
  assert (_bprs_objhash_arr != NULL);
  assert (_bprs_objhash_siz > 4);
  assert (pob != NULL && pob != BPRS_OBJHASH_EMPTY);
  assert (pob->ob_kind == BprsKind_Object);
  assert (pob->ob_hash != 0);
  assert (pob->ob_index == 0);
  assert (5 * _bprs_objhash_cnt < 4 * _bprs_objhash_siz);
  unsigned startix = pob->ob_hash % _bprs_objhash_siz;
  if (startix == 0)
    startix = 1;
  for (unsigned ix = startix; ix < _bprs_objhash_siz; ix++)
    {
      Bprs_Object *oldobj = _bprs_objhash_arr[ix];
      if (!oldobj || oldobj != BPRS_OBJHASH_EMPTY)
	{
	  _bprs_objhash_arr[ix] = pob;
	  _bprs_objhash_cnt++;
	  pob->ob_index = ix;
	  return;
	}
    }
  for (unsigned ix = 1; ix < startix; ix++)
    {
      Bprs_Object *oldobj = _bprs_objhash_arr[ix];
      if (!oldobj || oldobj != BPRS_OBJHASH_EMPTY)
	{
	  _bprs_objhash_arr[ix] = pob;
	  _bprs_objhash_cnt++;
	  pob->ob_index = ix;
	  return;
	}
    }
  BPRS_FATAL ("corruption in _bprs_objhash_register pob@%p", pob);
}				/* end _bprs_objhash_register */

static void
_bprs_objhash_forget (Bprs_Object *pob)
{
  assert (_bprs_objhash_arr != NULL);
  assert (_bprs_objhash_siz > 4);
  assert (pob != NULL && pob != BPRS_OBJHASH_EMPTY);
  assert (pob->ob_kind == BprsKind_Object);
  assert (pob->ob_hash != 0);
  assert (pob->ob_hash
	  == bprs_hash_for_object_hi_lo (pob->ob_hi, pob->ob_lo));
  assert (pob->ob_index != 0);
  if (_bprs_objhash_arr[pob->ob_index] != pob)
    BPRS_FATAL ("corruption in _bprs_objhash_forget pob@%p", pob);
  pob->ob_index = 0;
  _bprs_objhash_arr[pob->ob_index] = BPRS_OBJHASH_EMPTY;
  _bprs_objhash_cnt--;
  if (_bprs_objhash_cnt < _bprs_objhash_siz / 3 && _bprs_objhash_siz > 1000)
    {
      unsigned oldsiz = _bprs_objhash_siz;
      unsigned oldcnt = _bprs_objhash_cnt;
      Bprs_Object **oldarr = _bprs_objhash_arr;
      unsigned newsiz = ((3 * oldcnt / 2 + 50) & 0xff) + 1;
      if (newsiz < oldsiz)
	{
	  Bprs_Object **newarr
	    = GC_MALLOC_ATOMIC (newsiz * sizeof (Bprs_Object *));
	  memset ((void *) newarr, 0, newsiz * sizeof (Bprs_Object *));
	  _bprs_objhash_arr = newarr;
	  _bprs_objhash_siz = newsiz;
	  _bprs_objhash_cnt = 0;
	  for (unsigned ix = 0; ix < oldsiz; ix++)
	    {
	      Bprs_Object *oldobj = oldarr[ix];
	      if (oldobj == pob)
		continue;
	      if (oldobj && oldobj != BPRS_OBJHASH_EMPTY)
		{
		  assert (oldobj->ob_kind == BprsKind_Object);
		  oldobj->ob_index = 0;
		  _bprs_objhash_register (oldobj);
		}
	    }
	  assert (_bprs_objhash_cnt == oldcnt - 1);
	  GC_FREE (oldarr);
	}
    }
}				/* end _bprs_objhash_forget */



Bprs_Object *
bprs_object_find (uint32_t idhi, uint64_t idlo)
{
  Bprs_Object *res = NULL;
  pthread_mutex_lock (&_brps_objhash_mtx);
  if (idhi == 0 || idlo == 0)
    goto end;
  uint32_t h = bprs_hash_for_object_hi_lo (idhi, idlo);
  if (h == 0)
    goto end;
  unsigned startix = h % _bprs_objhash_siz;
  if (startix == 0)
    startix = 1;
  for (unsigned ix = startix; ix < _bprs_objhash_siz; ix++)
    {
      Bprs_Object *oldobj = _bprs_objhash_arr[ix];
      if (!oldobj)
	break;
      if (oldobj != BPRS_OBJHASH_EMPTY
	  && oldobj->ob_hi == idhi && oldobj->ob_lo == idlo)
	{
	  assert (oldobj->ob_index == ix);
	  res = oldobj;
	  goto end;
	}
    }
  for (unsigned ix = 1; ix < startix; ix++)
    {
      Bprs_Object *oldobj = _bprs_objhash_arr[ix];
      if (!oldobj)
	break;
      if (oldobj != BPRS_OBJHASH_EMPTY
	  && oldobj->ob_hi == idhi && oldobj->ob_lo == idlo)
	{
	  assert (oldobj->ob_index == ix);
	  res = oldobj;
	  goto end;
	}
    }
end:
  pthread_mutex_unlock (&_brps_objhash_mtx);
  return res;
}				/* end of bprs_object_find */

uint32_t
bprs_hash_object (const Bprs_Object *pob, bool *pok)
{
  if (!pob || (uintptr_t) pob < 0x1000
      || pob->ob_kind != BprsKind_Object
      || pob->ob_magic != BPRS_OBJECT_MAGIC
      || pob->ob_gap != ((intptr_t) pob & BPRS_MASK_OBJECT))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  assert (pob->ob_hash != 0);
  if (pok)
    *pok = true;
  return pob->ob_hash;
}				/* end bprs_hash_object */

Bprs_Object *
bprs_object_load (FILE *fil, uint32_t idhi, uint64_t idlo)
{
  Bprs_Object *pob = bprs_object_find (idhi, idlo);
  assert (fil != NULL);
  if (pob == NULL)
    {
      uint32_t h = bprs_hash_for_object_hi_lo (idhi, idlo);
      pthread_mutex_lock (&_brps_objhash_mtx);
      pob = GC_MALLOC (sizeof (Bprs_Object));
      memset (pob, 0, sizeof (Bprs_Object));
      pob->ob_gap = (intptr_t) pob & BPRS_MASK_OBJECT;
      pob->ob_kind = BprsKind_Object;
      pob->ob_index = 0;
      pob->ob_hash = h;
      pob->ob_nbcomp = 0;
      pob->ob_nbattr = 0;
      pob->ob_sizcomp = 0;
      pob->ob_sizattr = 0;
      pob->ob_hi = idhi;
      pob->ob_lo = idlo;
      pthread_mutex_init (&pob->ob_mtx, NULL);
      pob->ob_attrs = NULL;
      pob->ob_comps = NULL;
      _bprs_objhash_register (pob);
      goto end;
    end:
      pthread_mutex_unlock (&_brps_objhash_mtx);
    };
#warning incomplete bprs_object_load
  return pob;
}

Bprs_Object *
bprs_object_make (unsigned compsize, unsigned attrsize)
{
  Bprs_Object *pob = NULL;
  uint32_t idhi = 0;
  uint64_t idlo = 0;
  if (compsize > INT_MAX / 16 || attrsize > INT_MAX / 16)
    BPRS_FATAL ("too big compsize=%ud or attrsize=%ud:\n"
		" bprs_object_make (max=%ud)",
		compsize, attrsize, INT_MAX / 16);
  while (idhi < 1024 && idlo < (1L << 20))
    {
      idhi = (uint32_t) random ();
      idlo = (uint64_t) random ();
      pob = bprs_object_find (idhi, idlo);
      if (pob)
	idhi = (uint32_t) random ();
      else
	break;
    };
  uint32_t h = bprs_hash_for_object_hi_lo (idhi, idlo);
  pthread_mutex_lock (&_brps_objhash_mtx);
  pob = GC_MALLOC (sizeof (Bprs_Object));
  memset (pob, 0, sizeof (Bprs_Object));
  pob->ob_gap = (intptr_t) pob & BPRS_MASK_OBJECT;
  pob->ob_kind = BprsKind_Object;
  pob->ob_index = 0;
  pob->ob_hash = h;
  pob->ob_nbcomp = 0;
  pob->ob_nbattr = 0;
  pob->ob_sizcomp = 0;
  pob->ob_sizattr = 0;
  pob->ob_hi = idhi;
  pob->ob_lo = idlo;
  pthread_mutex_init (&pob->ob_mtx, NULL);
  pob->ob_attrs = NULL;
  pob->ob_comps = NULL;
  _bprs_objhash_register (pob);
  goto end;
end:
  pthread_mutex_unlock (&_brps_objhash_mtx);
#warning unimplemented bprs_object_make
  BPRS_FATAL ("unimplemented bprs_object_make idhi=%ud idlo=%lud",
	      idhi, idlo);
  return pob;
}				/* end bprs_object_make */

/* end boehm-rps/compo.c */
