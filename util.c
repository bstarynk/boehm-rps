/****************************************************************
 * file boehm-rps/main.c
 * SPDX-License-Identifier: GPL-3.0-or-later
 *
 * Description: This file is part of the Reflective Persistent System
 *      (Boehm variant) It contains several utility functions
 *
 * Author(s): Basile Starynkevitch, 92340 Bourg-la-Reine, France
 *      <basile@starynkevitch.net>
 *
 *      © Copyright (C) 2026 Basile Starynkevitch
 *
 ****/
#include "boehmrps.h"

static const char _bprs_util_shortgit[] = BPRS_SHORTGIT;


static const char _bprs_b36_digits[] = "0123456789ABCDEFGHIJKLMNOPQRTSUVWXYZ";
#define BPRS_INT_MAGIC 68140917	/* 0x40fbf75 */
#define BPRS_MASK_INT 0x7fef
struct bprs_int_st
{
  Bprs_Kind i_kind;
  int16_t i_gap;
  int32_t i_magic;
  int64_t i_val;
};

#define BPRS_DOUBLE_MAGIC 171454729	/* 0xa383109 */
#define BPRS_MASK_DOUBLE 0xffff
struct bprs_double_st
{
  Bprs_Kind d_kind;
  int16_t d_gap;
  int32_t d_magic;
  double d_val;
};


#define BPRS_STRING_MAGIC 654689243	/* 0x2705c3db */
#define BPRS_MASK_STRING 0x3ffd
struct bprs_string_st
{
  Bprs_Kind s_kind;
  int16_t s_gap;
  int32_t s_magic;
  uint32_t s_hash;
  uint32_t s_bytlen;
  uint32_t s_utflen;
  char s_val[];
};

const char *
bprs_shortgit_util (void)
{
  return _bprs_util_shortgit;
}				/* end bprs_shortgit_util */


bool
bprs_uint64_to_b36 (uint64_t i, char *buf, int bufsiz)
{
  char locbuf[48];
  if (!buf)
    return false;
  if (i == 0)
    {
      if (bufsiz <= 2)
	return false;;
      buf[0] = '0';
      buf[1] = (char) 0;
      return true;
    };
  memset (locbuf, 0, sizeof (locbuf));
  char *p = locbuf;
  uint64_t n = i;
  while (n != 0)
    {
      unsigned c = n % 36;
      n = n / 36;
      *p = _bprs_b36_digits[c];
      p++;
    };
  char *w = buf;
  if (p - locbuf + 1 > bufsiz)
    return false;
  for (char *b = p - 1; b >= locbuf; b--)
    *(w++) = *b;
  *w = 0;
  assert (strlen (buf) > 0 && (int) strlen (buf) < bufsiz);
  return true;
}				/* end bprs_uint64_to_b36 */

uint64_t
bprs_b36_to_uint64 (const char *s, const char **pend, bool *pok)
{
  unsigned long res = 0;
  char *end = NULL;
  if (!s)
    goto fail;
  res = strtoul (s, &end, 36);
  if (end && end > s)
    {
      if (pend)
	*pend = end;
      if (pok)
	*pok = true;
      return res;
    };
fail:
  if (pok)
    *pok = false;
  if (pend)
    *pend = s;
  return 0;
}				/* end bprs_b36_to_ulong */

Bprs_Int *
bprs_make_int (int64_t i)
{
  Bprs_Int *p = GC_MALLOC_ATOMIC (sizeof (Bprs_Int));
  p->i_kind = BprsKind_Int;
  p->i_gap = (intptr_t) p & BPRS_MASK_INT;
  p->i_magic = BPRS_INT_MAGIC;
  p->i_val = i;
  return p;
}				/* end bprs_make_int */






int64_t
bprs_get_int (const Bprs_Int *p, bool *pok)
{
  if (!p || p->i_kind != BprsKind_Int
      || p->i_magic != BPRS_INT_MAGIC
      || p->i_gap != ((intptr_t) p & BPRS_MASK_INT))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  return p->i_val;
}				/* end bprs_get_int */

uint32_t
bprs_hash_int (const Bprs_Int *p, bool *pok)
{
  if (!p || p->i_kind != BprsKind_Int
      || p->i_magic != BPRS_INT_MAGIC || p->i_gap != ((intptr_t) p & 0xffff))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  uint32_t h = (uint32_t) p->i_val;
  if (h == 0)
    h = ((p->i_val % 45233) & 0xfffff) + 2;
  assert (h != 0);
  return h;
}				/* end bprs_hash_int */


Bprs_Double *
bprs_make_double (double d)
{
  Bprs_Double *p = GC_MALLOC_ATOMIC (sizeof (Bprs_Double));
  p->d_kind = BprsKind_Double;
  p->d_gap = (intptr_t) p & BPRS_MASK_DOUBLE;
  p->d_magic = BPRS_DOUBLE_MAGIC;
  p->d_val = d;
  return p;
}				/* end bprs_make_double */

double
bprs_get_double (const Bprs_Double *p, bool *pok)
{
  if (!p || p->d_kind != BprsKind_Double
      || p->d_magic != BPRS_DOUBLE_MAGIC
      || p->d_gap != ((intptr_t) p & BPRS_MASK_DOUBLE))
    {
      if (pok)
	*pok = false;
      return __builtin_nan ("");
    };
  if (pok)
    *pok = true;
  return p->d_val;
}				/* end bprs_get_double */

uint32_t
bprs_hash_double (const Bprs_Double *p, bool *pok)
{
  uint32_t h = 0;
  if (!p || p->d_kind != BprsKind_Double
      || p->d_magic != BPRS_DOUBLE_MAGIC
      || p->d_gap != ((intptr_t) p & BPRS_MASK_DOUBLE))
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = true;
  if (isnan (p->d_val))
    return 17;
  else if (isinf (p->d_val))
    return 45413;
  h = (uint32_t) round (log (fabs (p->d_val) + 0.0001) * 65.637);
  if (h == 0)
    h = 17;
  return h;
}				/* end bprs_hash_double */

uint32_t
bprs_hash_c_string (const char *s)
{
  if (!s || (uintptr_t) s < 0x100)
    return 0;
  unsigned h = 0;
  const uint8_t *us = (const uint8_t *) s;
  if (!s)
    return 0;
  unsigned bytlen = strlen (s);
  int cnt = 0;
  while (us)
    {
      ucs4_t uc = 0;
      int l = u8_mbtoucr (&uc, us, s + bytlen - (char *) us);
      if (l < 0)
	BPRS_FATAL ("corrupted UTF8 string %s", s);
      cnt++;
      us += l;
      if (cnt % 2 == 0)
	h = ((h * 443) ^ (uc * 347)) + (cnt & 0xff);
      else
	h = ((h * 311) ^ (uc * 359 + cnt % 17));
    };
  if (h == 0)
    h = bytlen + 1;
  return h;
}

Bprs_String *
bprs_make_string (const char *s)
{
  uint32_t bytlen = 0;
  uint32_t utflen = 0;
  if (!s || ((uintptr_t) s < 0X1000))
    return NULL;
  if (!s[0])
    return NULL;
  uint32_t hs = bprs_hash_c_string (s);
  if (!hs)
    return NULL;
  bytlen = strlen (s);
  utflen = u8_strlen ((uint8_t *) s);
  if (bytlen > INT_MAX / 4 || utflen > INT_MAX / 4)
    BPRS_FATAL ("too long string of %ud bytes and %ud UTF8 chars",
		bytlen, utflen);
  size_t alsiz = ((sizeof (Bprs_String) + bytlen + 2) | 0xf) + 1;
  Bprs_String *ps = GC_MALLOC_ATOMIC (alsiz);
  ps->s_kind = BprsKind_String;
  ps->s_gap = (intptr_t) ps & BPRS_MASK_STRING;
  ps->s_magic = BPRS_STRING_MAGIC;
  ps->s_hash = hs;
  ps->s_bytlen = bytlen;
  ps->s_utflen = utflen;
  strncpy (ps->s_val, s, bytlen);
  return ps;
}				/* end bprs_make_string */

const char *
bprs_get_string (const Bprs_String *ps, bool *pok)
{
  if (!ps || ((uintptr_t) ps < 0X1000)
      || ps->s_kind != BprsKind_String
      || ps->s_gap != ((intptr_t) ps & BPRS_MASK_STRING)
      || ps->s_magic != BPRS_STRING_MAGIC)
    {
      if (pok)
	*pok = false;
      return NULL;
    };
  assert (ps->s_hash == bprs_hash_c_string (ps->s_val));
  if (pok)
    *pok = true;
  return ps->s_val;
}				/* end bprs_get_string */

uint32_t
bprs_hash_string (const Bprs_String *ps, bool *pok)
{
  if (!ps || ((uintptr_t) ps < 0X1000)
      || ps->s_kind != BprsKind_String
      || ps->s_gap != ((intptr_t) ps & BPRS_MASK_STRING)
      || ps->s_magic != BPRS_STRING_MAGIC)
    {
      if (pok)
	*pok = false;
      return 0;
    };
  assert (ps->s_hash == bprs_hash_c_string (ps->s_val));
  if (pok)
    *pok = true;
  return ps->s_hash;
}				/* end bprs_hash_string */

uint32_t
bprs_byte_len_string (const Bprs_String *ps, bool *pok)
{
  if (!ps || ((uintptr_t) ps < 0X1000)
      || ps->s_kind != BprsKind_String
      || ps->s_gap != ((intptr_t) ps & BPRS_MASK_STRING)
      || ps->s_magic != BPRS_STRING_MAGIC)
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = false;
  return ps->s_bytlen;
}				/* end bprs_byte_len_string */


uint32_t
bprs_utf_len_string (const Bprs_String *ps, bool *pok)
{
  if (!ps || ((uintptr_t) ps < 0X1000)
      || ps->s_kind != BprsKind_String
      || ps->s_gap != ((intptr_t) ps & BPRS_MASK_STRING)
      || ps->s_magic != BPRS_STRING_MAGIC)
    {
      if (pok)
	*pok = false;
      return 0;
    };
  if (pok)
    *pok = false;
  return ps->s_utflen;
}				/* end bprs_utf_len_string */


void
bprs_warning_at (const char *srcfil, int lin, const char *fun,
		 const char *fmt, ...)
{
  va_list arg;
  fprintf (stderr, "%s:%d <%s> WARNING\n", srcfil, lin, fun);
  va_start (arg, fmt);
  vfprintf (stderr, fmt, arg);
  va_end (arg);
  fputc ('\n', stderr);
  fflush (stderr);
  fflush (NULL);		/* can be a GDB breakpoint */
}				/* end bprs_warning_at */

void
bprs_fatal_at (const char *srcfil, int lin, const char *fun,
	       const char *fmt, ...)
{
  va_list args;
  va_start (args, fmt);
  fprintf (stderr, "%s:%d <%s> FATAL\n", srcfil, lin, fun);
  vfprintf (stderr, fmt, args);
  va_end (args);
  fputc ('\n', stderr);
  fflush (stderr);
  va_end (args);
  fflush (NULL);		/* can be a GDB breakpoint */
  abort ();
}				/* end bprs_fatal_at */

/* end boehm-rps/util.c */
