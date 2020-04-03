#define VERSION "1.16"
#include <stdint.h>

/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
/* private part of the path and curve data structures */
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef POTRACELIB_H
#define POTRACELIB_H
/* this file defines the API for the core Potrace library. For a more
   detailed description of the API, see potracelib.pdf */
#ifdef __cplusplus
extern "C" {
#endif
/* ---------------------------------------------------------------------- */
/* tracing parameters */
/* turn policies */
#define POTRACE_TURNPOLICY_BLACK 0
#define POTRACE_TURNPOLICY_WHITE 1
#define POTRACE_TURNPOLICY_LEFT 2
#define POTRACE_TURNPOLICY_RIGHT 3
#define POTRACE_TURNPOLICY_MINORITY 4
#define POTRACE_TURNPOLICY_MAJORITY 5
#define POTRACE_TURNPOLICY_RANDOM 6
/* structure to hold progress bar callback data */
struct potrace_progress_s {
  void (*callback)(double progress, void *privdata); /* callback fn */
  void *data;          /* callback function's private data */
  double min, max;     /* desired range of progress, e.g. 0.0 to 1.0 */
  double epsilon;      /* granularity: can skip smaller increments */
};
typedef struct potrace_progress_s potrace_progress_t;
/* structure to hold tracing parameters */
struct potrace_param_s {
  int turdsize;        /* area of largest path to be ignored */
  int turnpolicy;      /* resolves ambiguous turns in path decomposition */
  double alphamax;     /* corner threshold */
  int opticurve;       /* use curve optimization? */
  double opttolerance; /* curve optimization tolerance */
  potrace_progress_t progress; /* progress callback function */
};
typedef struct potrace_param_s potrace_param_t;
/* ---------------------------------------------------------------------- */
/* bitmaps */
/* native word size */
typedef unsigned long potrace_word;
/* Internal bitmap format. The n-th scanline starts at scanline(n) =
   (map + n*dy). Raster data is stored as a sequence of potrace_words
   (NOT bytes). The leftmost bit of scanline n is the most significant
   bit of scanline(n)[0]. */
struct potrace_bitmap_s {
  int w, h;              /* width and height, in pixels */
  int dy;                /* words per scanline (not bytes) */
  potrace_word *map;     /* raw data, dy*h words */
};
typedef struct potrace_bitmap_s potrace_bitmap_t;
/* ---------------------------------------------------------------------- */
/* curves */
/* point */
struct potrace_dpoint_s {
  double x, y;
};
typedef struct potrace_dpoint_s potrace_dpoint_t;
/* segment tags */
#define POTRACE_CURVETO 1
#define POTRACE_CORNER 2
/* closed curve segment */
struct potrace_curve_s {
  int n;                    /* number of segments */
  int *tag;                 /* tag[n]: POTRACE_CURVETO or POTRACE_CORNER */
  potrace_dpoint_t (*c)[3]; /* c[n][3]: control points. 
			       c[n][0] is unused for tag[n]=POTRACE_CORNER */
};
typedef struct potrace_curve_s potrace_curve_t;
/* Linked list of signed curve segments. Also carries a tree structure. */
struct potrace_path_s {
  int area;                         /* area of the bitmap path */
  int sign;                         /* '+' or '-', depending on orientation */
  potrace_curve_t curve;            /* this path's vector data */
  struct potrace_path_s *next;      /* linked list structure */
  struct potrace_path_s *childlist; /* tree structure */
  struct potrace_path_s *sibling;   /* tree structure */
  struct potrace_privpath_s *priv;  /* private state */
};
typedef struct potrace_path_s potrace_path_t;  
/* ---------------------------------------------------------------------- */
/* Potrace state */
#define POTRACE_STATUS_OK         0
#define POTRACE_STATUS_INCOMPLETE 1
struct potrace_state_s {
  int status;                       
  potrace_path_t *plist;            /* vector data */
  struct potrace_privstate_s *priv; /* private state */
};
typedef struct potrace_state_s potrace_state_t;
/* ---------------------------------------------------------------------- */
/* API functions */
/* get default parameters */
potrace_param_t *potrace_param_default(void);
/* free parameter set */
void potrace_param_free(potrace_param_t *p);
/* trace a bitmap */
potrace_state_t *potrace_trace(const potrace_param_t *param, 
			       const potrace_bitmap_t *bm);
/* free a Potrace state */
void potrace_state_free(potrace_state_t *st);
/* return a static plain text version string identifying this version
   of potracelib */
const char *potrace_version(void);
#ifdef  __cplusplus
} /* end of extern "C" */
#endif
#endif /* POTRACELIB_H */
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef _PS_LISTS_H
#define _PS_LISTS_H
/* here we define some general list macros. Because they are macros,
   they should work on any datatype with a "->next" component. Some of
   them use a "hook". If elt and list are of type t* then hook is of
   type t**. A hook stands for an insertion point in the list, i.e.,
   either before the first element, or between two elements, or after
   the last element. If an operation "sets the hook" for an element,
   then the hook is set to just before the element. One can insert
   something at a hook. One can also unlink at a hook: this means,
   unlink the element just after the hook. By "to unlink", we mean the
   element is removed from the list, but not deleted. Thus, it and its
   components still need to be freed. */
/* Note: these macros are somewhat experimental. Only the ones that
   are actually *used* have been tested. So be careful to test any
   that you use. Looking at the output of the preprocessor, "gcc -E"
   (possibly piped though "indent"), might help too. Also: these
   macros define some internal (local) variables that start with
   "_". */
/* we enclose macro definitions whose body consists of more than one
   statement in MACRO_BEGIN and MACRO_END, rather than '{' and '}'.  The
   reason is that we want to be able to use the macro in a context
   such as "if (...) macro(...); else ...". If we didn't use this obscure
   trick, we'd have to omit the ";" in such cases. */
#define MACRO_BEGIN do {
#define MACRO_END   } while (0)
/* ---------------------------------------------------------------------- */
/* macros for singly-linked lists */
/* traverse list. At the end, elt is set to NULL. */
#define list_forall(elt, list)   for (elt=list; elt!=NULL; elt=elt->next)
/* set elt to the first element of list satisfying boolean condition
   c, or NULL if not found */
#define list_find(elt, list, c) \
  MACRO_BEGIN list_forall(elt, list) if (c) break; MACRO_END
/* like forall, except also set hook for elt. */
#define list_forall2(elt, list, hook) \
  for (elt=list, hook=&list; elt!=NULL; hook=&elt->next, elt=elt->next)
/* same as list_find, except also set hook for elt. */
#define list_find2(elt, list, c, hook) \
  MACRO_BEGIN list_forall2(elt, list, hook) if (c) break; MACRO_END
/* same, except only use hook. */
#define _list_forall_hook(list, hook) \
  for (hook=&list; *hook!=NULL; hook=&(*hook)->next)
/* same, except only use hook. Note: c may only refer to *hook, not elt. */
#define _list_find_hook(list, c, hook) \
  MACRO_BEGIN _list_forall_hook(list, hook) if (c) break; MACRO_END
/* insert element after hook */
#define list_insert_athook(elt, hook) \
  MACRO_BEGIN elt->next = *hook; *hook = elt; MACRO_END
/* insert element before hook */
#define list_insert_beforehook(elt, hook) \
  MACRO_BEGIN elt->next = *hook; *hook = elt; hook=&elt->next; MACRO_END
/* unlink element after hook, let elt be unlinked element, or NULL.
   hook remains. */
#define list_unlink_athook(list, elt, hook) \
  MACRO_BEGIN \
  elt = hook ? *hook : NULL; if (elt) { *hook = elt->next; elt->next = NULL; }\
  MACRO_END
/* unlink the specific element, if it is in the list. Otherwise, set
   elt to NULL */
#define list_unlink(listtype, list, elt)      \
  MACRO_BEGIN  	       	       	       	      \
  listtype **_hook;			      \
  _list_find_hook(list, *_hook==elt, _hook);  \
  list_unlink_athook(list, elt, _hook);	      \
  MACRO_END
/* prepend elt to list */
#define list_prepend(list, elt) \
  MACRO_BEGIN elt->next = list; list = elt; MACRO_END
/* append elt to list. */
#define list_append(listtype, list, elt)     \
  MACRO_BEGIN                                \
  listtype **_hook;                          \
  _list_forall_hook(list, _hook) {}          \
  list_insert_athook(elt, _hook);            \
  MACRO_END
/* unlink the first element that satisfies the condition. */
#define list_unlink_cond(listtype, list, elt, c)     \
  MACRO_BEGIN                                        \
  listtype **_hook;			  	     \
  list_find2(elt, list, c, _hook);                   \
  list_unlink_athook(list, elt, _hook);              \
  MACRO_END
/* let elt be the nth element of the list, starting to count from 0.
   Return NULL if out of bounds.   */
#define list_nth(elt, list, n)                                \
  MACRO_BEGIN                                                 \
  int _x;  /* only evaluate n once */                         \
  for (_x=(n), elt=list; _x && elt; _x--, elt=elt->next) {}   \
  MACRO_END
/* let elt be the nth element of the list, starting to count from 0.
   Return NULL if out of bounds.   */
#define list_nth_hook(elt, list, n, hook)                     \
  MACRO_BEGIN                                                 \
  int _x;  /* only evaluate n once */                         \
  for (_x=(n), elt=list, hook=&list; _x && elt; _x--, hook=&elt->next, elt=elt->next) {}   \
  MACRO_END
/* set n to the length of the list */
#define list_length(listtype, list, n)                   \
  MACRO_BEGIN          	       	       	       	       	 \
  listtype *_elt;   			 		 \
  n=0;					 		 \
  list_forall(_elt, list) 		 		 \
    n++;				 		 \
  MACRO_END
/* set n to the index of the first element satisfying cond, or -1 if
   none found. Also set elt to the element, or NULL if none found. */
#define list_index(list, n, elt, c)                      \
  MACRO_BEGIN				 		 \
  n=0;					 		 \
  list_forall(elt, list) {		 		 \
    if (c) break;			 		 \
    n++;				 		 \
  }					 		 \
  if (!elt)				 		 \
    n=-1;				 		 \
  MACRO_END
/* set n to the number of elements in the list that satisfy condition c */
#define list_count(list, n, elt, c)                      \
  MACRO_BEGIN				 		 \
  n=0;					 		 \
  list_forall(elt, list) {		 		 \
    if (c) n++;				 		 \
  }                                                      \
  MACRO_END
/* let elt be each element of the list, unlinked. At the end, set list=NULL. */
#define list_forall_unlink(elt, list) \
  for (elt=list; elt ? (list=elt->next, elt->next=NULL), 1 : 0; elt=list)
/* reverse a list (efficient) */
#define list_reverse(listtype, list)            \
  MACRO_BEGIN				 	\
  listtype *_list1=NULL, *elt;			\
  list_forall_unlink(elt, list) 		\
    list_prepend(_list1, elt);			\
  list = _list1;				\
  MACRO_END
/* insert the element ELT just before the first element TMP of the
   list for which COND holds. Here COND must be a condition of ELT and
   TMP.  Typical usage is to insert an element into an ordered list:
   for instance, list_insert_ordered(listtype, list, elt, tmp,
   elt->size <= tmp->size).  Note: if we give a "less than or equal"
   condition, the new element will be inserted just before a sequence
   of equal elements. If we give a "less than" condition, the new
   element will be inserted just after a list of equal elements.
   Note: it is much more efficient to construct a list with
   list_prepend and then order it with list_merge_sort, than to
   construct it with list_insert_ordered. */
#define list_insert_ordered(listtype, list, elt, tmp, cond) \
  MACRO_BEGIN                                               \
  listtype **_hook;                                         \
  _list_find_hook(list, (tmp=*_hook, (cond)), _hook);       \
  list_insert_athook(elt, _hook);                           \
  MACRO_END
/* sort the given list, according to the comparison condition.
   Typical usage is list_sort(listtype, list, a, b, a->size <
   b->size).  Note: if we give "less than or equal" condition, each
   segment of equal elements will be reversed in order. If we give a
   "less than" condition, each segment of equal elements will retain
   the original order. The latter is slower but sometimes
   prettier. Average running time: n*n/2. */
#define list_sort(listtype, list, a, b, cond)            \
  MACRO_BEGIN                                            \
  listtype *_newlist=NULL;                               \
  list_forall_unlink(a, list)                            \
    list_insert_ordered(listtype, _newlist, a, b, cond); \
  list = _newlist;                                       \
  MACRO_END
/* a much faster sort algorithm (merge sort, n log n worst case). It
   is required that the list type has an additional, unused next1
   component. Note there is no curious reversal of order of equal
   elements as for list_sort. */
#define list_mergesort(listtype, list, a, b, cond)              \
  MACRO_BEGIN						        \
  listtype *_elt, **_hook1;				    	\
							    	\
  for (_elt=list; _elt; _elt=_elt->next1) {			\
    _elt->next1 = _elt->next;				    	\
    _elt->next = NULL;					    	\
  }							    	\
  do {			                               	    	\
    _hook1 = &(list);				    	    	\
    while ((a = *_hook1) != NULL && (b = a->next1) != NULL ) {  \
      _elt = b->next1;					    	\
      _list_merge_cond(listtype, a, b, cond, *_hook1);      	\
      _hook1 = &((*_hook1)->next1);			    	\
      *_hook1 = _elt;				            	\
    }							    	\
  } while (_hook1 != &(list));                                 	\
  MACRO_END
/* merge two sorted lists. Store result at &result */
#define _list_merge_cond(listtype, a, b, cond, result)   \
  MACRO_BEGIN                                            \
  listtype **_hook;					 \
  _hook = &(result);					 \
  while (1) {                                            \
     if (a==NULL) {					 \
       *_hook = b;					 \
       break;						 \
     } else if (b==NULL) {				 \
       *_hook = a;					 \
       break;						 \
     } else if (cond) {					 \
       *_hook = a;					 \
       _hook = &(a->next);				 \
       a = a->next;					 \
     } else {						 \
       *_hook = b;					 \
       _hook = &(b->next);				 \
       b = b->next;					 \
     }							 \
  }							 \
  MACRO_END
/* ---------------------------------------------------------------------- */
/* macros for doubly-linked lists */
#define dlist_append(head, end, elt)                    \
  MACRO_BEGIN  	       	       	       	       	       	 \
  elt->prev = end;					 \
  elt->next = NULL;					 \
  if (end) {						 \
    end->next = elt;					 \
  } else {  						 \
    head = elt;						 \
  }	    						 \
  end = elt;						 \
  MACRO_END
/* let elt be each element of the list, unlinked. At the end, set list=NULL. */
#define dlist_forall_unlink(elt, head, end) \
  for (elt=head; elt ? (head=elt->next, elt->next=NULL, elt->prev=NULL), 1 : (end=NULL, 0); elt=head)
/* unlink the first element of the list */
#define dlist_unlink_first(head, end, elt)               \
  MACRO_BEGIN				       	       	 \
  elt = head;						 \
  if (head) {						 \
    head = head->next;					 \
    if (head) {						 \
      head->prev = NULL;				 \
    } else {						 \
      end = NULL;					 \
    }    						 \
    elt->prev = NULL;					 \
    elt->next = NULL;					 \
  }							 \
  MACRO_END
#endif /* _PS_LISTS_H */
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef CURVE_H
#define CURVE_H
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
/* This header file collects some general-purpose macros (and static
   inline functions) that are used in various places. */
#ifndef AUXILIARY_H
#define AUXILIARY_H
#ifdef HAVE_CONFIG_H
#endif
/* ---------------------------------------------------------------------- */
/* point arithmetic */
struct point_s {
  long x;
  long y;
};
typedef struct point_s point_t;
typedef potrace_dpoint_t dpoint_t;
/* convert point_t to dpoint_t */
static inline dpoint_t dpoint(point_t p) {
  dpoint_t res;
  res.x = p.x;
  res.y = p.y;
  return res;
}
/* range over the straight line segment [a,b] when lambda ranges over [0,1] */
static inline dpoint_t interval(double lambda, dpoint_t a, dpoint_t b) {
  dpoint_t res;
  res.x = a.x + lambda * (b.x - a.x);
  res.y = a.y + lambda * (b.y - a.y);
  return res;
}
/* ---------------------------------------------------------------------- */
/* some useful macros. Note: the "mod" macro works correctly for
   negative a. Also note that the test for a>=n, while redundant,
   speeds up the mod function by 70% in the average case (significant
   since the program spends about 16% of its time here - or 40%
   without the test). The "floordiv" macro returns the largest integer
   <= a/n, and again this works correctly for negative a, as long as
   a,n are integers and n>0. */
/* integer arithmetic */
static inline int mod(int a, int n) {
  return a>=n ? a%n : a>=0 ? a : n-1-(-1-a)%n;
}
static inline int floordiv(int a, int n) {
  return a>=0 ? a/n : -1-(-1-a)/n;
}
/* Note: the following work for integers and other numeric types. */
#undef sign
#undef abs
#undef min
#undef max
#undef sq
#undef cu
#define sign(x) ((x)>0 ? 1 : (x)<0 ? -1 : 0)
#define abs(a) ((a)>0 ? (a) : -(a))
#define min(a,b) ((a)<(b) ? (a) : (b))
#define max(a,b) ((a)>(b) ? (a) : (b))
#define sq(a) ((a)*(a))
#define cu(a) ((a)*(a)*(a))
#endif /* AUXILIARY_H */
/* vertex is c[1] for tag=POTRACE_CORNER, and the intersection of
   .c[-1][2]..c[0] and c[1]..c[2] for tag=POTRACE_CURVETO. alpha is only
   defined for tag=POTRACE_CURVETO and is the alpha parameter of the curve:
   .c[-1][2]..c[0] = alpha*(.c[-1][2]..vertex), and
   c[2]..c[1] = alpha*(c[2]..vertex).
   Beta is so that (.beta[i])[.vertex[i],.vertex[i+1]] = .c[i][2].
*/
struct privcurve_s {
  int n;            /* number of segments */
  int *tag;         /* tag[n]: POTRACE_CORNER or POTRACE_CURVETO */
  dpoint_t (*c)[3]; /* c[n][i]: control points. 
		       c[n][0] is unused for tag[n]=POTRACE_CORNER */
  /* the remainder of this structure is special to privcurve, and is
     used in EPS debug output and special EPS "short coding". These
     fields are valid only if "alphacurve" is set. */
  int alphacurve;   /* have the following fields been initialized? */
  dpoint_t *vertex; /* for POTRACE_CORNER, this equals c[1] */
  double *alpha;    /* only for POTRACE_CURVETO */
  double *alpha0;   /* "uncropped" alpha parameter - for debug output only */
  double *beta;
};
typedef struct privcurve_s privcurve_t;
struct sums_s {
  double x;
  double y;
  double x2;
  double xy;
  double y2;
};
typedef struct sums_s sums_t;
/* the path structure is filled in with information about a given path
   as it is accumulated and passed through the different stages of the
   Potrace algorithm. Backends only need to read the fcurve and fm
   fields of this data structure, but debugging backends may read
   other fields. */
struct potrace_privpath_s {
  int len;
  point_t *pt;     /* pt[len]: path as extracted from bitmap */
  int *lon;        /* lon[len]: (i,lon[i]) = longest straight line from i */
  int x0, y0;      /* origin for sums */
  sums_t *sums;    /* sums[len+1]: cache for fast summing */
  int m;           /* length of optimal polygon */
  int *po;         /* po[m]: optimal polygon */
  privcurve_t curve;   /* curve[m]: array of curve elements */
  privcurve_t ocurve;  /* ocurve[om]: array of curve elements */
  privcurve_t *fcurve;  /* final curve: this points to either curve or
		       ocurve. Do not free this separately. */
};
typedef struct potrace_privpath_s potrace_privpath_t;
/* shorter names */
typedef potrace_privpath_t privpath_t;
typedef potrace_path_t path_t;
path_t *path_new(void);
void path_free(path_t *p);
void pathlist_free(path_t *plist);
int privcurve_init(privcurve_t *curve, int n);
void privcurve_to_curve(privcurve_t *pc, potrace_curve_t *c);
#endif /* CURVE_H */
#define SAFE_CALLOC(var, n, typ) \
  if ((var = (typ *)calloc(n, sizeof(typ))) == NULL) goto calloc_error 
/* ---------------------------------------------------------------------- */
/* allocate and free path objects */
path_t *path_new(void) {
  path_t *p = NULL;
  privpath_t *priv = NULL;
  SAFE_CALLOC(p, 1, path_t);
  memset(p, 0, sizeof(path_t));
  SAFE_CALLOC(priv, 1, privpath_t);
  memset(priv, 0, sizeof(privpath_t));
  p->priv = priv;
  return p;
 calloc_error:
  free(p);
  free(priv);
  return NULL;
}
/* free the members of the given curve structure. Leave errno unchanged. */
static void privcurve_free_members(privcurve_t *curve) {
  free(curve->tag);
  free(curve->c);
  free(curve->vertex);
  free(curve->alpha);
  free(curve->alpha0);
  free(curve->beta);
}
/* free a path. Leave errno untouched. */
void path_free(path_t *p) {
  if (p) {
    if (p->priv) {
      free(p->priv->pt);
      free(p->priv->lon);
      free(p->priv->sums);
      free(p->priv->po);
      privcurve_free_members(&p->priv->curve);
      privcurve_free_members(&p->priv->ocurve);
    }
    free(p->priv);
    /* do not free p->fcurve ! */
  }
  free(p);
}  
/* free a pathlist, leaving errno untouched. */
void pathlist_free(path_t *plist) {
  path_t *p;
  list_forall_unlink(p, plist) {
    path_free(p);
  }
}
/* ---------------------------------------------------------------------- */
/* initialize and finalize curve structures */
typedef dpoint_t dpoint3_t[3];
/* initialize the members of the given curve structure to size m.
   Return 0 on success, 1 on error with errno set. */
int privcurve_init(privcurve_t *curve, int n) {
  memset(curve, 0, sizeof(privcurve_t));
  curve->n = n;
  SAFE_CALLOC(curve->tag, n, int);
  SAFE_CALLOC(curve->c, n, dpoint3_t);
  SAFE_CALLOC(curve->vertex, n, dpoint_t);
  SAFE_CALLOC(curve->alpha, n, double);
  SAFE_CALLOC(curve->alpha0, n, double);
  SAFE_CALLOC(curve->beta, n, double);
  return 0;
 calloc_error:
  free(curve->tag);
  free(curve->c);
  free(curve->vertex);
  free(curve->alpha);
  free(curve->alpha0);
  free(curve->beta);
  return 1;
}
/* copy private to public curve structure */
void privcurve_to_curve(privcurve_t *pc, potrace_curve_t *c) {
  c->n = pc->n;
  c->tag = pc->tag;
  c->c = pc->c;
}
    
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
/* transform jaggy paths into smooth curves */
#ifdef HAVE_CONFIG_H
#endif
#include <math.h>
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef TRACE_H
#define TRACE_H
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
/* operations on potrace_progress_t objects, which are defined in
   potracelib.h. Note: the code attempts to minimize runtime overhead
   when no progress monitoring was requested. It also tries to
   minimize excessive progress calculations beneath the "epsilon"
   threshold. */
#ifndef PROGRESS_H
#define PROGRESS_H
/* structure to hold progress bar callback data */
struct progress_s {
  void (*callback)(double progress, void *privdata); /* callback fn */
  void *data;          /* callback function's private data */
  double min, max;     /* desired range of progress, e.g. 0.0 to 1.0 */
  double epsilon;      /* granularity: can skip smaller increments */
  double b;            /* upper limit of subrange in superrange units */
  double d_prev;       /* previous value of d */
};
typedef struct progress_s progress_t;
/* notify given progress object of current progress. Note that d is
   given in the 0.0-1.0 range, which will be scaled and translated to
   the progress object's range. */
static inline void progress_update(double d, progress_t *prog) {
  double d_scaled;
  if (prog != NULL && prog->callback != NULL) {
    d_scaled = prog->min * (1-d) + prog->max * d;
    if (d == 1.0 || d_scaled >= prog->d_prev + prog->epsilon) {
      prog->callback(prog->min * (1-d) + prog->max * d, prog->data);
      prog->d_prev = d_scaled;
    }
  }
}
/* start a subrange of the given progress object. The range is
   narrowed to [a..b], relative to 0.0-1.0 coordinates. If new range
   is below granularity threshold, disable further subdivisions. */
static inline void progress_subrange_start(double a, double b, const progress_t *prog, progress_t *sub) {
  double min, max;
  if (prog == NULL || prog->callback == NULL) {
    sub->callback = NULL;
    return;
  }
  min = prog->min * (1-a) + prog->max * a;
  max = prog->min * (1-b) + prog->max * b;
  if (max - min < prog->epsilon) {
    sub->callback = NULL;    /* no further progress info in subrange */
    sub->b = b;
    return;
  }
  sub->callback = prog->callback;
  sub->data = prog->data;
  sub->epsilon = prog->epsilon;
  sub->min = min;
  sub->max = max;
  sub->d_prev = prog->d_prev;
  return;
}
static inline void progress_subrange_end(progress_t *prog, progress_t *sub) {
  if (prog != NULL && prog->callback != NULL) {
    if (sub->callback == NULL) {
      progress_update(sub->b, prog);
    } else {
      prog->d_prev = sub->d_prev;
    }
  }    
}
#endif /* PROGRESS_H */
int process_path(path_t *plist, const potrace_param_t *param, progress_t *progress);
#endif /* TRACE_H */
#define INFTY 10000000	/* it suffices that this is longer than any
			   path; it need not be really infinite */
#define COS179 -0.999847695156	 /* the cosine of 179 degrees */
/* ---------------------------------------------------------------------- */
#define SAFE_CALLOC(var, n, typ) \
  if ((var = (typ *)calloc(n, sizeof(typ))) == NULL) goto calloc_error 
/* ---------------------------------------------------------------------- */
/* auxiliary functions */
/* return a direction that is 90 degrees counterclockwise from p2-p0,
   but then restricted to one of the major wind directions (n, nw, w, etc) */
static inline point_t dorth_infty(dpoint_t p0, dpoint_t p2) {
  point_t r;
  
  r.y = sign(p2.x-p0.x);
  r.x = -sign(p2.y-p0.y);
  return r;
}
/* return (p1-p0)x(p2-p0), the area of the parallelogram */
static inline double dpara(dpoint_t p0, dpoint_t p1, dpoint_t p2) {
  double x1, y1, x2, y2;
  x1 = p1.x-p0.x;
  y1 = p1.y-p0.y;
  x2 = p2.x-p0.x;
  y2 = p2.y-p0.y;
  return x1*y2 - x2*y1;
}
/* ddenom/dpara have the property that the square of radius 1 centered
   at p1 intersects the line p0p2 iff |dpara(p0,p1,p2)| <= ddenom(p0,p2) */
static inline double ddenom(dpoint_t p0, dpoint_t p2) {
  point_t r = dorth_infty(p0, p2);
  return r.y*(p2.x-p0.x) - r.x*(p2.y-p0.y);
}
/* return 1 if a <= b < c < a, in a cyclic sense (mod n) */
static inline int cyclic(int a, int b, int c) {
  if (a<=c) {
    return (a<=b && b<c);
  } else {
    return (a<=b || b<c);
  }
}
/* determine the center and slope of the line i..j. Assume i<j. Needs
   "sum" components of p to be set. */
static void pointslope(privpath_t *pp, int i, int j, dpoint_t *ctr, dpoint_t *dir) {
  /* assume i<j */
  int n = pp->len;
  sums_t *sums = pp->sums;
  double x, y, x2, xy, y2;
  double k;
  double a, b, c, lambda2, l;
  int r=0; /* rotations from i to j */
  while (j>=n) {
    j-=n;
    r+=1;
  }
  while (i>=n) {
    i-=n;
    r-=1;
  }
  while (j<0) {
    j+=n;
    r-=1;
  }
  while (i<0) {
    i+=n;
    r+=1;
  }
  
  x = sums[j+1].x-sums[i].x+r*sums[n].x;
  y = sums[j+1].y-sums[i].y+r*sums[n].y;
  x2 = sums[j+1].x2-sums[i].x2+r*sums[n].x2;
  xy = sums[j+1].xy-sums[i].xy+r*sums[n].xy;
  y2 = sums[j+1].y2-sums[i].y2+r*sums[n].y2;
  k = j+1-i+r*n;
  
  ctr->x = x/k;
  ctr->y = y/k;
  a = (x2-(double)x*x/k)/k;
  b = (xy-(double)x*y/k)/k;
  c = (y2-(double)y*y/k)/k;
  
  lambda2 = (a+c+sqrt((a-c)*(a-c)+4*b*b))/2; /* larger e.value */
  /* now find e.vector for lambda2 */
  a -= lambda2;
  c -= lambda2;
  if (fabs(a) >= fabs(c)) {
    l = sqrt(a*a+b*b);
    if (l!=0) {
      dir->x = -b/l;
      dir->y = a/l;
    }
  } else {
    l = sqrt(c*c+b*b);
    if (l!=0) {
      dir->x = -c/l;
      dir->y = b/l;
    }
  }
  if (l==0) {
    dir->x = dir->y = 0;   /* sometimes this can happen when k=4:
			      the two eigenvalues coincide */
  }
}
/* the type of (affine) quadratic forms, represented as symmetric 3x3
   matrices.  The value of the quadratic form at a vector (x,y) is v^t
   Q v, where v = (x,y,1)^t. */
typedef double quadform_t[3][3];
/* Apply quadratic form Q to vector w = (w.x,w.y) */
static inline double quadform(quadform_t Q, dpoint_t w) {
  double v[3];
  int i, j;
  double sum;
  v[0] = w.x;
  v[1] = w.y;
  v[2] = 1;
  sum = 0.0;
  for (i=0; i<3; i++) {
    for (j=0; j<3; j++) {
      sum += v[i] * Q[i][j] * v[j];
    }
  }
  return sum;
}
/* calculate p1 x p2 */
static inline int xprod(point_t p1, point_t p2) {
  return p1.x*p2.y - p1.y*p2.x;
}
/* calculate (p1-p0)x(p3-p2) */
static inline double cprod(dpoint_t p0, dpoint_t p1, dpoint_t p2, dpoint_t p3) {
  double x1, y1, x2, y2;
  x1 = p1.x - p0.x;
  y1 = p1.y - p0.y;
  x2 = p3.x - p2.x;
  y2 = p3.y - p2.y;
  return x1*y2 - x2*y1;
}
/* calculate (p1-p0)*(p2-p0) */
static inline double iprod(dpoint_t p0, dpoint_t p1, dpoint_t p2) {
  double x1, y1, x2, y2;
  x1 = p1.x - p0.x;
  y1 = p1.y - p0.y;
  x2 = p2.x - p0.x;
  y2 = p2.y - p0.y;
  return x1*x2 + y1*y2;
}
/* calculate (p1-p0)*(p3-p2) */
static inline double iprod1(dpoint_t p0, dpoint_t p1, dpoint_t p2, dpoint_t p3) {
  double x1, y1, x2, y2;
  x1 = p1.x - p0.x;
  y1 = p1.y - p0.y;
  x2 = p3.x - p2.x;
  y2 = p3.y - p2.y;
  return x1*x2 + y1*y2;
}
/* calculate distance between two points */
static inline double ddist(dpoint_t p, dpoint_t q) {
  return sqrt(sq(p.x-q.x)+sq(p.y-q.y));
}
/* calculate point of a bezier curve */
static inline dpoint_t bezier(double t, dpoint_t p0, dpoint_t p1, dpoint_t p2, dpoint_t p3) {
  double s = 1-t;
  dpoint_t res;
  /* Note: a good optimizing compiler (such as gcc-3) reduces the
     following to 16 multiplications, using common subexpression
     elimination. */
  res.x = s*s*s*p0.x + 3*(s*s*t)*p1.x + 3*(t*t*s)*p2.x + t*t*t*p3.x;
  res.y = s*s*s*p0.y + 3*(s*s*t)*p1.y + 3*(t*t*s)*p2.y + t*t*t*p3.y;
  return res;
}
/* calculate the point t in [0..1] on the (convex) bezier curve
   (p0,p1,p2,p3) which is tangent to q1-q0. Return -1.0 if there is no
   solution in [0..1]. */
static double tangent(dpoint_t p0, dpoint_t p1, dpoint_t p2, dpoint_t p3, dpoint_t q0, dpoint_t q1) {
  double A, B, C;   /* (1-t)^2 A + 2(1-t)t B + t^2 C = 0 */
  double a, b, c;   /* a t^2 + b t + c = 0 */
  double d, s, r1, r2;
  A = cprod(p0, p1, q0, q1);
  B = cprod(p1, p2, q0, q1);
  C = cprod(p2, p3, q0, q1);
  a = A - 2*B + C;
  b = -2*A + 2*B;
  c = A;
  
  d = b*b - 4*a*c;
  if (a==0 || d<0) {
    return -1.0;
  }
  s = sqrt(d);
  r1 = (-b + s) / (2 * a);
  r2 = (-b - s) / (2 * a);
  if (r1 >= 0 && r1 <= 1) {
    return r1;
  } else if (r2 >= 0 && r2 <= 1) {
    return r2;
  } else {
    return -1.0;
  }
}
/* ---------------------------------------------------------------------- */
/* Preparation: fill in the sum* fields of a path (used for later
   rapid summing). Return 0 on success, 1 with errno set on
   failure. */
static int calc_sums(privpath_t *pp) {
  int i, x, y;
  int n = pp->len;
  SAFE_CALLOC(pp->sums, pp->len+1, sums_t);
  /* origin */
  pp->x0 = pp->pt[0].x;
  pp->y0 = pp->pt[0].y;
  /* preparatory computation for later fast summing */
  pp->sums[0].x2 = pp->sums[0].xy = pp->sums[0].y2 = pp->sums[0].x = pp->sums[0].y = 0;
  for (i=0; i<n; i++) {
    x = pp->pt[i].x - pp->x0;
    y = pp->pt[i].y - pp->y0;
    pp->sums[i+1].x = pp->sums[i].x + x;
    pp->sums[i+1].y = pp->sums[i].y + y;
    pp->sums[i+1].x2 = pp->sums[i].x2 + (double)x*x;
    pp->sums[i+1].xy = pp->sums[i].xy + (double)x*y;
    pp->sums[i+1].y2 = pp->sums[i].y2 + (double)y*y;
  }
  return 0;  
 calloc_error:
  return 1;
}
/* ---------------------------------------------------------------------- */
/* Stage 1: determine the straight subpaths (Sec. 2.2.1). Fill in the
   "lon" component of a path object (based on pt/len).	For each i,
   lon[i] is the furthest index such that a straight line can be drawn
   from i to lon[i]. Return 1 on error with errno set, else 0. */
/* this algorithm depends on the fact that the existence of straight
   subpaths is a triplewise property. I.e., there exists a straight
   line through squares i0,...,in iff there exists a straight line
   through i,j,k, for all i0<=i<j<k<=in. (Proof?) */
/* this implementation of calc_lon is O(n^2). It replaces an older
   O(n^3) version. A "constraint" means that future points must
   satisfy xprod(constraint[0], cur) >= 0 and xprod(constraint[1],
   cur) <= 0. */
/* Remark for Potrace 1.1: the current implementation of calc_lon is
   more complex than the implementation found in Potrace 1.0, but it
   is considerably faster. The introduction of the "nc" data structure
   means that we only have to test the constraints for "corner"
   points. On a typical input file, this speeds up the calc_lon
   function by a factor of 31.2, thereby decreasing its time share
   within the overall Potrace algorithm from 72.6% to 7.82%, and
   speeding up the overall algorithm by a factor of 3.36. On another
   input file, calc_lon was sped up by a factor of 6.7, decreasing its
   time share from 51.4% to 13.61%, and speeding up the overall
   algorithm by a factor of 1.78. In any case, the savings are
   substantial. */
/* returns 0 on success, 1 on error with errno set */
static int calc_lon(privpath_t *pp) {
  point_t *pt = pp->pt;
  int n = pp->len;
  int i, j, k, k1;
  int ct[4], dir;
  point_t constraint[2];
  point_t cur;
  point_t off;
  int *pivk = NULL;  /* pivk[n] */
  int *nc = NULL;    /* nc[n]: next corner */
  point_t dk;  /* direction of k-k1 */
  int a, b, c, d;
  SAFE_CALLOC(pivk, n, int);
  SAFE_CALLOC(nc, n, int);
  /* initialize the nc data structure. Point from each point to the
     furthest future point to which it is connected by a vertical or
     horizontal segment. We take advantage of the fact that there is
     always a direction change at 0 (due to the path decomposition
     algorithm). But even if this were not so, there is no harm, as
     in practice, correctness does not depend on the word "furthest"
     above.  */
  k = 0;
  for (i=n-1; i>=0; i--) {
    if (pt[i].x != pt[k].x && pt[i].y != pt[k].y) {
      k = i+1;  /* necessarily i<n-1 in this case */
    }
    nc[i] = k;
  }
  SAFE_CALLOC(pp->lon, n, int);
  /* determine pivot points: for each i, let pivk[i] be the furthest k
     such that all j with i<j<k lie on a line connecting i,k. */
  
  for (i=n-1; i>=0; i--) {
    ct[0] = ct[1] = ct[2] = ct[3] = 0;
    /* keep track of "directions" that have occurred */
    dir = (3+3*(pt[mod(i+1,n)].x-pt[i].x)+(pt[mod(i+1,n)].y-pt[i].y))/2;
    ct[dir]++;
    constraint[0].x = 0;
    constraint[0].y = 0;
    constraint[1].x = 0;
    constraint[1].y = 0;
    /* find the next k such that no straight line from i to k */
    k = nc[i];
    k1 = i;
    while (1) {
      
      dir = (3+3*sign(pt[k].x-pt[k1].x)+sign(pt[k].y-pt[k1].y))/2;
      ct[dir]++;
      /* if all four "directions" have occurred, cut this path */
      if (ct[0] && ct[1] && ct[2] && ct[3]) {
	pivk[i] = k1;
	goto foundk;
      }
      cur.x = pt[k].x - pt[i].x;
      cur.y = pt[k].y - pt[i].y;
      /* see if current constraint is violated */
      if (xprod(constraint[0], cur) < 0 || xprod(constraint[1], cur) > 0) {
	goto constraint_viol;
      }
      /* else, update constraint */
      if (abs(cur.x) <= 1 && abs(cur.y) <= 1) {
	/* no constraint */
      } else {
	off.x = cur.x + ((cur.y>=0 && (cur.y>0 || cur.x<0)) ? 1 : -1);
	off.y = cur.y + ((cur.x<=0 && (cur.x<0 || cur.y<0)) ? 1 : -1);
	if (xprod(constraint[0], off) >= 0) {
	  constraint[0] = off;
	}
	off.x = cur.x + ((cur.y<=0 && (cur.y<0 || cur.x<0)) ? 1 : -1);
	off.y = cur.y + ((cur.x>=0 && (cur.x>0 || cur.y<0)) ? 1 : -1);
	if (xprod(constraint[1], off) <= 0) {
	  constraint[1] = off;
	}
      }	
      k1 = k;
      k = nc[k1];
      if (!cyclic(k,i,k1)) {
	break;
      }
    }
  constraint_viol:
    /* k1 was the last "corner" satisfying the current constraint, and
       k is the first one violating it. We now need to find the last
       point along k1..k which satisfied the constraint. */
    dk.x = sign(pt[k].x-pt[k1].x);
    dk.y = sign(pt[k].y-pt[k1].y);
    cur.x = pt[k1].x - pt[i].x;
    cur.y = pt[k1].y - pt[i].y;
    /* find largest integer j such that xprod(constraint[0], cur+j*dk)
       >= 0 and xprod(constraint[1], cur+j*dk) <= 0. Use bilinearity
       of xprod. */
    a = xprod(constraint[0], cur);
    b = xprod(constraint[0], dk);
    c = xprod(constraint[1], cur);
    d = xprod(constraint[1], dk);
    /* find largest integer j such that a+j*b>=0 and c+j*d<=0. This
       can be solved with integer arithmetic. */
    j = INFTY;
    if (b<0) {
      j = floordiv(a,-b);
    }
    if (d>0) {
      j = min(j, floordiv(-c,d));
    }
    pivk[i] = mod(k1+j,n);
  foundk:
    ;
  } /* for i */
  /* clean up: for each i, let lon[i] be the largest k such that for
     all i' with i<=i'<k, i'<k<=pivk[i']. */
  j=pivk[n-1];
  pp->lon[n-1]=j;
  for (i=n-2; i>=0; i--) {
    if (cyclic(i+1,pivk[i],j)) {
      j=pivk[i];
    }
    pp->lon[i]=j;
  }
  for (i=n-1; cyclic(mod(i+1,n),j,pp->lon[i]); i--) {
    pp->lon[i] = j;
  }
  free(pivk);
  free(nc);
  return 0;
 calloc_error:
  free(pivk);
  free(nc);
  return 1;
}
/* ---------------------------------------------------------------------- */
/* Stage 2: calculate the optimal polygon (Sec. 2.2.2-2.2.4). */ 
/* Auxiliary function: calculate the penalty of an edge from i to j in
   the given path. This needs the "lon" and "sum*" data. */
static double penalty3(privpath_t *pp, int i, int j) {
  int n = pp->len;
  point_t *pt = pp->pt;
  sums_t *sums = pp->sums;
  /* assume 0<=i<j<=n  */
  double x, y, x2, xy, y2;
  double k;
  double a, b, c, s;
  double px, py, ex, ey;
  int r = 0; /* rotations from i to j */
  if (j>=n) {
    j -= n;
    r = 1;
  }
  
  /* critical inner loop: the "if" gives a 4.6 percent speedup */
  if (r == 0) {
    x = sums[j+1].x - sums[i].x;
    y = sums[j+1].y - sums[i].y;
    x2 = sums[j+1].x2 - sums[i].x2;
    xy = sums[j+1].xy - sums[i].xy;
    y2 = sums[j+1].y2 - sums[i].y2;
    k = j+1 - i;
  } else {
    x = sums[j+1].x - sums[i].x + sums[n].x;
    y = sums[j+1].y - sums[i].y + sums[n].y;
    x2 = sums[j+1].x2 - sums[i].x2 + sums[n].x2;
    xy = sums[j+1].xy - sums[i].xy + sums[n].xy;
    y2 = sums[j+1].y2 - sums[i].y2 + sums[n].y2;
    k = j+1 - i + n;
  } 
  px = (pt[i].x + pt[j].x) / 2.0 - pt[0].x;
  py = (pt[i].y + pt[j].y) / 2.0 - pt[0].y;
  ey = (pt[j].x - pt[i].x);
  ex = -(pt[j].y - pt[i].y);
  a = ((x2 - 2*x*px) / k + px*px);
  b = ((xy - x*py - y*px) / k + px*py);
  c = ((y2 - 2*y*py) / k + py*py);
  
  s = ex*ex*a + 2*ex*ey*b + ey*ey*c;
  return sqrt(s);
}
/* find the optimal polygon. Fill in the m and po components. Return 1
   on failure with errno set, else 0. Non-cyclic version: assumes i=0
   is in the polygon. Fixme: implement cyclic version. */
static int bestpolygon(privpath_t *pp)
{
  int i, j, m, k;     
  int n = pp->len;
  double *pen = NULL; /* pen[n+1]: penalty vector */
  int *prev = NULL;   /* prev[n+1]: best path pointer vector */
  int *clip0 = NULL;  /* clip0[n]: longest segment pointer, non-cyclic */
  int *clip1 = NULL;  /* clip1[n+1]: backwards segment pointer, non-cyclic */
  int *seg0 = NULL;    /* seg0[m+1]: forward segment bounds, m<=n */
  int *seg1 = NULL;   /* seg1[m+1]: backward segment bounds, m<=n */
  double thispen;
  double best;
  int c;
  SAFE_CALLOC(pen, n+1, double);
  SAFE_CALLOC(prev, n+1, int);
  SAFE_CALLOC(clip0, n, int);
  SAFE_CALLOC(clip1, n+1, int);
  SAFE_CALLOC(seg0, n+1, int);
  SAFE_CALLOC(seg1, n+1, int);
  /* calculate clipped paths */
  for (i=0; i<n; i++) {
    c = mod(pp->lon[mod(i-1,n)]-1,n);
    if (c == i) {
      c = mod(i+1,n);
    }
    if (c < i) {
      clip0[i] = n;
    } else {
      clip0[i] = c;
    }
  }
  /* calculate backwards path clipping, non-cyclic. j <= clip0[i] iff
     clip1[j] <= i, for i,j=0..n. */
  j = 1;
  for (i=0; i<n; i++) {
    while (j <= clip0[i]) {
      clip1[j] = i;
      j++;
    }
  }
  /* calculate seg0[j] = longest path from 0 with j segments */
  i = 0;
  for (j=0; i<n; j++) {
    seg0[j] = i;
    i = clip0[i];
  }
  seg0[j] = n;
  m = j;
  /* calculate seg1[j] = longest path to n with m-j segments */
  i = n;
  for (j=m; j>0; j--) {
    seg1[j] = i;
    i = clip1[i];
  }
  seg1[0] = 0;
  /* now find the shortest path with m segments, based on penalty3 */
  /* note: the outer 2 loops jointly have at most n iterations, thus
     the worst-case behavior here is quadratic. In practice, it is
     close to linear since the inner loop tends to be short. */
  pen[0]=0;
  for (j=1; j<=m; j++) {
    for (i=seg1[j]; i<=seg0[j]; i++) {
      best = -1;
      for (k=seg0[j-1]; k>=clip1[i]; k--) {
	thispen = penalty3(pp, k, i) + pen[k];
	if (best < 0 || thispen < best) {
	  prev[i] = k;
	  best = thispen;
	}
      }
      pen[i] = best;
    }
  }
  pp->m = m;
  SAFE_CALLOC(pp->po, m, int);
  /* read off shortest path */
  for (i=n, j=m-1; i>0; j--) {
    i = prev[i];
    pp->po[j] = i;
  }
  free(pen);
  free(prev);
  free(clip0);
  free(clip1);
  free(seg0);
  free(seg1);
  return 0;
  
 calloc_error:
  free(pen);
  free(prev);
  free(clip0);
  free(clip1);
  free(seg0);
  free(seg1);
  return 1;
}
/* ---------------------------------------------------------------------- */
/* Stage 3: vertex adjustment (Sec. 2.3.1). */
/* Adjust vertices of optimal polygon: calculate the intersection of
   the two "optimal" line segments, then move it into the unit square
   if it lies outside. Return 1 with errno set on error; 0 on
   success. */
static int adjust_vertices(privpath_t *pp) {
  int m = pp->m;
  int *po = pp->po;
  int n = pp->len;
  point_t *pt = pp->pt;
  int x0 = pp->x0;
  int y0 = pp->y0;
  dpoint_t *ctr = NULL;      /* ctr[m] */
  dpoint_t *dir = NULL;      /* dir[m] */
  quadform_t *q = NULL;      /* q[m] */
  double v[3];
  double d;
  int i, j, k, l;
  dpoint_t s;
  int r;
  SAFE_CALLOC(ctr, m, dpoint_t);
  SAFE_CALLOC(dir, m, dpoint_t);
  SAFE_CALLOC(q, m, quadform_t);
  r = privcurve_init(&pp->curve, m);
  if (r) {
    goto calloc_error;
  }
  
  /* calculate "optimal" point-slope representation for each line
     segment */
  for (i=0; i<m; i++) {
    j = po[mod(i+1,m)];
    j = mod(j-po[i],n)+po[i];
    pointslope(pp, po[i], j, &ctr[i], &dir[i]);
  }
  /* represent each line segment as a singular quadratic form; the
     distance of a point (x,y) from the line segment will be
     (x,y,1)Q(x,y,1)^t, where Q=q[i]. */
  for (i=0; i<m; i++) {
    d = sq(dir[i].x) + sq(dir[i].y);
    if (d == 0.0) {
      for (j=0; j<3; j++) {
	for (k=0; k<3; k++) {
	  q[i][j][k] = 0;
	}
      }
    } else {
      v[0] = dir[i].y;
      v[1] = -dir[i].x;
      v[2] = - v[1] * ctr[i].y - v[0] * ctr[i].x;
      for (l=0; l<3; l++) {
	for (k=0; k<3; k++) {
	  q[i][l][k] = v[l] * v[k] / d;
	}
      }
    }
  }
  /* now calculate the "intersections" of consecutive segments.
     Instead of using the actual intersection, we find the point
     within a given unit square which minimizes the square distance to
     the two lines. */
  for (i=0; i<m; i++) {
    quadform_t Q;
    dpoint_t w;
    double dx, dy;
    double det;
    double min, cand; /* minimum and candidate for minimum of quad. form */
    double xmin, ymin;	/* coordinates of minimum */
    int z;
    /* let s be the vertex, in coordinates relative to x0/y0 */
    s.x = pt[po[i]].x-x0;
    s.y = pt[po[i]].y-y0;
    /* intersect segments i-1 and i */
    j = mod(i-1,m);
    /* add quadratic forms */
    for (l=0; l<3; l++) {
      for (k=0; k<3; k++) {
	Q[l][k] = q[j][l][k] + q[i][l][k];
      }
    }
    
    while(1) {
      /* minimize the quadratic form Q on the unit square */
      /* find intersection */
#ifdef HAVE_GCC_LOOP_BUG
      /* work around gcc bug #12243 */
      free(NULL);
#endif
      
      det = Q[0][0]*Q[1][1] - Q[0][1]*Q[1][0];
      if (det != 0.0) {
	w.x = (-Q[0][2]*Q[1][1] + Q[1][2]*Q[0][1]) / det;
	w.y = ( Q[0][2]*Q[1][0] - Q[1][2]*Q[0][0]) / det;
	break;
      }
      /* matrix is singular - lines are parallel. Add another,
	 orthogonal axis, through the center of the unit square */
      if (Q[0][0]>Q[1][1]) {
	v[0] = -Q[0][1];
	v[1] = Q[0][0];
      } else if (Q[1][1]) {
	v[0] = -Q[1][1];
	v[1] = Q[1][0];
      } else {
	v[0] = 1;
	v[1] = 0;
      }
      d = sq(v[0]) + sq(v[1]);
      v[2] = - v[1] * s.y - v[0] * s.x;
      for (l=0; l<3; l++) {
	for (k=0; k<3; k++) {
	  Q[l][k] += v[l] * v[k] / d;
	}
      }
    }
    dx = fabs(w.x-s.x);
    dy = fabs(w.y-s.y);
    if (dx <= .5 && dy <= .5) {
      pp->curve.vertex[i].x = w.x+x0;
      pp->curve.vertex[i].y = w.y+y0;
      continue;
    }
    /* the minimum was not in the unit square; now minimize quadratic
       on boundary of square */
    min = quadform(Q, s);
    xmin = s.x;
    ymin = s.y;
    if (Q[0][0] == 0.0) {
      goto fixx;
    }
    for (z=0; z<2; z++) {   /* value of the y-coordinate */
      w.y = s.y-0.5+z;
      w.x = - (Q[0][1] * w.y + Q[0][2]) / Q[0][0];
      dx = fabs(w.x-s.x);
      cand = quadform(Q, w);
      if (dx <= .5 && cand < min) {
	min = cand;
	xmin = w.x;
	ymin = w.y;
      }
    }
  fixx:
    if (Q[1][1] == 0.0) {
      goto corners;
    }
    for (z=0; z<2; z++) {   /* value of the x-coordinate */
      w.x = s.x-0.5+z;
      w.y = - (Q[1][0] * w.x + Q[1][2]) / Q[1][1];
      dy = fabs(w.y-s.y);
      cand = quadform(Q, w);
      if (dy <= .5 && cand < min) {
	min = cand;
	xmin = w.x;
	ymin = w.y;
      }
    }
  corners:
    /* check four corners */
    for (l=0; l<2; l++) {
      for (k=0; k<2; k++) {
	w.x = s.x-0.5+l;
	w.y = s.y-0.5+k;
	cand = quadform(Q, w);
	if (cand < min) {
	  min = cand;
	  xmin = w.x;
	  ymin = w.y;
	}
      }
    }
    pp->curve.vertex[i].x = xmin + x0;
    pp->curve.vertex[i].y = ymin + y0;
    continue;
  }
  free(ctr);
  free(dir);
  free(q);
  return 0;
 calloc_error:
  free(ctr);
  free(dir);
  free(q);
  return 1;
}
/* ---------------------------------------------------------------------- */
/* Stage 4: smoothing and corner analysis (Sec. 2.3.3) */
/* reverse orientation of a path */
static void reverse(privcurve_t *curve) {
  int m = curve->n;
  int i, j;
  dpoint_t tmp;
  for (i=0, j=m-1; i<j; i++, j--) {
    tmp = curve->vertex[i];
    curve->vertex[i] = curve->vertex[j];
    curve->vertex[j] = tmp;
  }
}
/* Always succeeds */
static void smooth(privcurve_t *curve, double alphamax) {
  int m = curve->n;
  int i, j, k;
  double dd, denom, alpha;
  dpoint_t p2, p3, p4;
  /* examine each vertex and find its best fit */
  for (i=0; i<m; i++) {
    j = mod(i+1, m);
    k = mod(i+2, m);
    p4 = interval(1/2.0, curve->vertex[k], curve->vertex[j]);
    denom = ddenom(curve->vertex[i], curve->vertex[k]);
    if (denom != 0.0) {
      dd = dpara(curve->vertex[i], curve->vertex[j], curve->vertex[k]) / denom;
      dd = fabs(dd);
      alpha = dd>1 ? (1 - 1.0/dd) : 0;
      alpha = alpha / 0.75;
    } else {
      alpha = 4/3.0;
    }
    curve->alpha0[j] = alpha;	 /* remember "original" value of alpha */
    if (alpha >= alphamax) {  /* pointed corner */
      curve->tag[j] = POTRACE_CORNER;
      curve->c[j][1] = curve->vertex[j];
      curve->c[j][2] = p4;
    } else {
      if (alpha < 0.55) {
	alpha = 0.55;
      } else if (alpha > 1) {
	alpha = 1;
      }
      p2 = interval(.5+.5*alpha, curve->vertex[i], curve->vertex[j]);
      p3 = interval(.5+.5*alpha, curve->vertex[k], curve->vertex[j]);
      curve->tag[j] = POTRACE_CURVETO;
      curve->c[j][0] = p2;
      curve->c[j][1] = p3;
      curve->c[j][2] = p4;
    }
    curve->alpha[j] = alpha;	/* store the "cropped" value of alpha */
    curve->beta[j] = 0.5;
  }
  curve->alphacurve = 1;
  return;
}
/* ---------------------------------------------------------------------- */
/* Stage 5: Curve optimization (Sec. 2.4) */
/* a private type for the result of opti_penalty */
struct opti_s {
  double pen;	   /* penalty */
  dpoint_t c[2];   /* curve parameters */
  double t, s;	   /* curve parameters */
  double alpha;	   /* curve parameter */
};
typedef struct opti_s opti_t;
/* calculate best fit from i+.5 to j+.5.  Assume i<j (cyclically).
   Return 0 and set badness and parameters (alpha, beta), if
   possible. Return 1 if impossible. */
static int opti_penalty(privpath_t *pp, int i, int j, opti_t *res, double opttolerance, int *convc, double *areac) {
  int m = pp->curve.n;
  int k, k1, k2, conv, i1;
  double area, alpha, d, d1, d2;
  dpoint_t p0, p1, p2, p3, pt;
  double A, R, A1, A2, A3, A4;
  double s, t;
  /* check convexity, corner-freeness, and maximum bend < 179 degrees */
  if (i==j) {  /* sanity - a full loop can never be an opticurve */
    return 1;
  }
  k = i;
  i1 = mod(i+1, m);
  k1 = mod(k+1, m);
  conv = convc[k1];
  if (conv == 0) {
    return 1;
  }
  d = ddist(pp->curve.vertex[i], pp->curve.vertex[i1]);
  for (k=k1; k!=j; k=k1) {
    k1 = mod(k+1, m);
    k2 = mod(k+2, m);
    if (convc[k1] != conv) {
      return 1;
    }
    if (sign(cprod(pp->curve.vertex[i], pp->curve.vertex[i1], pp->curve.vertex[k1], pp->curve.vertex[k2])) != conv) {
      return 1;
    }
    if (iprod1(pp->curve.vertex[i], pp->curve.vertex[i1], pp->curve.vertex[k1], pp->curve.vertex[k2]) < d * ddist(pp->curve.vertex[k1], pp->curve.vertex[k2]) * COS179) {
      return 1;
    }
  }
  /* the curve we're working in: */
  p0 = pp->curve.c[mod(i,m)][2];
  p1 = pp->curve.vertex[mod(i+1,m)];
  p2 = pp->curve.vertex[mod(j,m)];
  p3 = pp->curve.c[mod(j,m)][2];
  /* determine its area */
  area = areac[j] - areac[i];
  area -= dpara(pp->curve.vertex[0], pp->curve.c[i][2], pp->curve.c[j][2])/2;
  if (i>=j) {
    area += areac[m];
  }
  /* find intersection o of p0p1 and p2p3. Let t,s such that o =
     interval(t,p0,p1) = interval(s,p3,p2). Let A be the area of the
     triangle (p0,o,p3). */
  A1 = dpara(p0, p1, p2);
  A2 = dpara(p0, p1, p3);
  A3 = dpara(p0, p2, p3);
  /* A4 = dpara(p1, p2, p3); */
  A4 = A1+A3-A2;    
  
  if (A2 == A1) {  /* this should never happen */
    return 1;
  }
  t = A3/(A3-A4);
  s = A2/(A2-A1);
  A = A2 * t / 2.0;
  
  if (A == 0.0) {  /* this should never happen */
    return 1;
  }
  R = area / A;	 /* relative area */
  alpha = 2 - sqrt(4 - R / 0.3);  /* overall alpha for p0-o-p3 curve */
  res->c[0] = interval(t * alpha, p0, p1);
  res->c[1] = interval(s * alpha, p3, p2);
  res->alpha = alpha;
  res->t = t;
  res->s = s;
  p1 = res->c[0];
  p2 = res->c[1];  /* the proposed curve is now (p0,p1,p2,p3) */
  res->pen = 0;
  /* calculate penalty */
  /* check tangency with edges */
  for (k=mod(i+1,m); k!=j; k=k1) {
    k1 = mod(k+1,m);
    t = tangent(p0, p1, p2, p3, pp->curve.vertex[k], pp->curve.vertex[k1]);
    if (t<-.5) {
      return 1;
    }
    pt = bezier(t, p0, p1, p2, p3);
    d = ddist(pp->curve.vertex[k], pp->curve.vertex[k1]);
    if (d == 0.0) {  /* this should never happen */
      return 1;
    }
    d1 = dpara(pp->curve.vertex[k], pp->curve.vertex[k1], pt) / d;
    if (fabs(d1) > opttolerance) {
      return 1;
    }
    if (iprod(pp->curve.vertex[k], pp->curve.vertex[k1], pt) < 0 || iprod(pp->curve.vertex[k1], pp->curve.vertex[k], pt) < 0) {
      return 1;
    }
    res->pen += sq(d1);
  }
  /* check corners */
  for (k=i; k!=j; k=k1) {
    k1 = mod(k+1,m);
    t = tangent(p0, p1, p2, p3, pp->curve.c[k][2], pp->curve.c[k1][2]);
    if (t<-.5) {
      return 1;
    }
    pt = bezier(t, p0, p1, p2, p3);
    d = ddist(pp->curve.c[k][2], pp->curve.c[k1][2]);
    if (d == 0.0) {  /* this should never happen */
      return 1;
    }
    d1 = dpara(pp->curve.c[k][2], pp->curve.c[k1][2], pt) / d;
    d2 = dpara(pp->curve.c[k][2], pp->curve.c[k1][2], pp->curve.vertex[k1]) / d;
    d2 *= 0.75 * pp->curve.alpha[k1];
    if (d2 < 0) {
      d1 = -d1;
      d2 = -d2;
    }
    if (d1 < d2 - opttolerance) {
      return 1;
    }
    if (d1 < d2) {
      res->pen += sq(d1 - d2);
    }
  }
  return 0;
}
/* optimize the path p, replacing sequences of Bezier segments by a
   single segment when possible. Return 0 on success, 1 with errno set
   on failure. */
static int opticurve(privpath_t *pp, double opttolerance) {
  int m = pp->curve.n;
  int *pt = NULL;     /* pt[m+1] */
  double *pen = NULL; /* pen[m+1] */
  int *len = NULL;    /* len[m+1] */
  opti_t *opt = NULL; /* opt[m+1] */
  int om;
  int i,j,r;
  opti_t o;
  dpoint_t p0;
  int i1;
  double area;
  double alpha;
  double *s = NULL;
  double *t = NULL;
  int *convc = NULL; /* conv[m]: pre-computed convexities */
  double *areac = NULL; /* cumarea[m+1]: cache for fast area computation */
  SAFE_CALLOC(pt, m+1, int);
  SAFE_CALLOC(pen, m+1, double);
  SAFE_CALLOC(len, m+1, int);
  SAFE_CALLOC(opt, m+1, opti_t);
  SAFE_CALLOC(convc, m, int);
  SAFE_CALLOC(areac, m+1, double);
  /* pre-calculate convexity: +1 = right turn, -1 = left turn, 0 = corner */
  for (i=0; i<m; i++) {
    if (pp->curve.tag[i] == POTRACE_CURVETO) {
      convc[i] = sign(dpara(pp->curve.vertex[mod(i-1,m)], pp->curve.vertex[i], pp->curve.vertex[mod(i+1,m)]));
    } else {
      convc[i] = 0;
    }
  }
  /* pre-calculate areas */
  area = 0.0;
  areac[0] = 0.0;
  p0 = pp->curve.vertex[0];
  for (i=0; i<m; i++) {
    i1 = mod(i+1, m);
    if (pp->curve.tag[i1] == POTRACE_CURVETO) {
      alpha = pp->curve.alpha[i1];
      area += 0.3*alpha*(4-alpha)*dpara(pp->curve.c[i][2], pp->curve.vertex[i1], pp->curve.c[i1][2])/2;
      area += dpara(p0, pp->curve.c[i][2], pp->curve.c[i1][2])/2;
    }
    areac[i+1] = area;
  }
  pt[0] = -1;
  pen[0] = 0;
  len[0] = 0;
  /* Fixme: we always start from a fixed point -- should find the best
     curve cyclically */
  for (j=1; j<=m; j++) {
    /* calculate best path from 0 to j */
    pt[j] = j-1;
    pen[j] = pen[j-1];
    len[j] = len[j-1]+1;
    for (i=j-2; i>=0; i--) {
      r = opti_penalty(pp, i, mod(j,m), &o, opttolerance, convc, areac);
      if (r) {
	break;
      }
      if (len[j] > len[i]+1 || (len[j] == len[i]+1 && pen[j] > pen[i] + o.pen)) {
	pt[j] = i;
	pen[j] = pen[i] + o.pen;
	len[j] = len[i] + 1;
	opt[j] = o;
      }
    }
  }
  om = len[m];
  r = privcurve_init(&pp->ocurve, om);
  if (r) {
    goto calloc_error;
  }
  SAFE_CALLOC(s, om, double);
  SAFE_CALLOC(t, om, double);
  j = m;
  for (i=om-1; i>=0; i--) {
    if (pt[j]==j-1) {
      pp->ocurve.tag[i]     = pp->curve.tag[mod(j,m)];
      pp->ocurve.c[i][0]    = pp->curve.c[mod(j,m)][0];
      pp->ocurve.c[i][1]    = pp->curve.c[mod(j,m)][1];
      pp->ocurve.c[i][2]    = pp->curve.c[mod(j,m)][2];
      pp->ocurve.vertex[i]  = pp->curve.vertex[mod(j,m)];
      pp->ocurve.alpha[i]   = pp->curve.alpha[mod(j,m)];
      pp->ocurve.alpha0[i]  = pp->curve.alpha0[mod(j,m)];
      pp->ocurve.beta[i]    = pp->curve.beta[mod(j,m)];
      s[i] = t[i] = 1.0;
    } else {
      pp->ocurve.tag[i] = POTRACE_CURVETO;
      pp->ocurve.c[i][0] = opt[j].c[0];
      pp->ocurve.c[i][1] = opt[j].c[1];
      pp->ocurve.c[i][2] = pp->curve.c[mod(j,m)][2];
      pp->ocurve.vertex[i] = interval(opt[j].s, pp->curve.c[mod(j,m)][2], pp->curve.vertex[mod(j,m)]);
      pp->ocurve.alpha[i] = opt[j].alpha;
      pp->ocurve.alpha0[i] = opt[j].alpha;
      s[i] = opt[j].s;
      t[i] = opt[j].t;
    }
    j = pt[j];
  }
  /* calculate beta parameters */
  for (i=0; i<om; i++) {
    i1 = mod(i+1,om);
    pp->ocurve.beta[i] = s[i] / (s[i] + t[i1]);
  }
  pp->ocurve.alphacurve = 1;
  free(pt);
  free(pen);
  free(len);
  free(opt);
  free(s);
  free(t);
  free(convc);
  free(areac);
  return 0;
 calloc_error:
  free(pt);
  free(pen);
  free(len);
  free(opt);
  free(s);
  free(t);
  free(convc);
  free(areac);
  return 1;
}
/* ---------------------------------------------------------------------- */
#define TRY(x) if (x) goto try_error
/* return 0 on success, 1 on error with errno set. */
int process_path(path_t *plist, const potrace_param_t *param, progress_t *progress) {
  path_t *p;
  double nn = 0, cn = 0;
  if (progress->callback) {
    /* precompute task size for progress estimates */
    nn = 0;
    list_forall (p, plist) {
      nn += p->priv->len;
    }
    cn = 0;
  }
  
  /* call downstream function with each path */
  list_forall (p, plist) {
    TRY(calc_sums(p->priv));
    TRY(calc_lon(p->priv));
    TRY(bestpolygon(p->priv));
    TRY(adjust_vertices(p->priv));
    if (p->sign == '-') {   /* reverse orientation of negative paths */
      reverse(&p->priv->curve);
    }
    smooth(&p->priv->curve, param->alphamax);
    if (param->opticurve) {
      TRY(opticurve(p->priv, param->opttolerance));
      p->priv->fcurve = &p->priv->ocurve;
    } else {
      p->priv->fcurve = &p->priv->curve;
    }
    privcurve_to_curve(p->priv->fcurve, &p->curve);
    if (progress->callback) {
      cn += p->priv->len;
      progress_update(cn/nn, progress);
    }
  }
  progress_update(1.0, progress);
  return 0;
 try_error:
  return 1;
}
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifdef HAVE_CONFIG_H
#endif
#include <limits.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef BITMAP_H
#define BITMAP_H
#include <errno.h>
#include <stddef.h>
/* The bitmap type is defined in potracelib.h */
/* The present file defines some convenient macros and static inline
   functions for accessing bitmaps. Since they only produce inline
   code, they can be conveniently shared by the library and frontends,
   if desired */
/* ---------------------------------------------------------------------- */
/* some measurements */
#define BM_WORDSIZE ((int)sizeof(potrace_word))
#define BM_WORDBITS (8*BM_WORDSIZE)
#define BM_HIBIT (((potrace_word)1)<<(BM_WORDBITS-1))
#define BM_ALLBITS (~(potrace_word)0)
/* macros for accessing pixel at index (x,y). U* macros omit the
   bounds check. */
#define bm_scanline(bm, y) ((bm)->map + (ptrdiff_t)(y)*(ptrdiff_t)(bm)->dy)
#define bm_index(bm, x, y) (&bm_scanline(bm, y)[(x)/BM_WORDBITS])
#define bm_mask(x) (BM_HIBIT >> ((x) & (BM_WORDBITS-1)))
#define bm_range(x, a) ((int)(x) >= 0 && (int)(x) < (a))
#define bm_safe(bm, x, y) (bm_range(x, (bm)->w) && bm_range(y, (bm)->h))
#define BM_UGET(bm, x, y) ((*bm_index(bm, x, y) & bm_mask(x)) != 0)
#define BM_USET(bm, x, y) (*bm_index(bm, x, y) |= bm_mask(x))
#define BM_UCLR(bm, x, y) (*bm_index(bm, x, y) &= ~bm_mask(x))
#define BM_UINV(bm, x, y) (*bm_index(bm, x, y) ^= bm_mask(x))
#define BM_UPUT(bm, x, y, b) ((b) ? BM_USET(bm, x, y) : BM_UCLR(bm, x, y))
#define BM_GET(bm, x, y) (bm_safe(bm, x, y) ? BM_UGET(bm, x, y) : 0)
#define BM_SET(bm, x, y) (bm_safe(bm, x, y) ? BM_USET(bm, x, y) : 0)
#define BM_CLR(bm, x, y) (bm_safe(bm, x, y) ? BM_UCLR(bm, x, y) : 0)
#define BM_INV(bm, x, y) (bm_safe(bm, x, y) ? BM_UINV(bm, x, y) : 0)
#define BM_PUT(bm, x, y, b) (bm_safe(bm, x, y) ? BM_UPUT(bm, x, y, b) : 0)
/* calculate the size, in bytes, required for the data area of a
   bitmap of the given dy and h. Assume h >= 0. Return -1 if the size
   does not fit into the ptrdiff_t type. */
static inline ptrdiff_t getsize(int dy, int h) {
  ptrdiff_t size;
  if (dy < 0) {
    dy = -dy;
  }
  
  size = (ptrdiff_t)dy * (ptrdiff_t)h * (ptrdiff_t)BM_WORDSIZE;
  /* check for overflow error */
  if (size < 0 || (h != 0 && dy != 0 && size / h / dy != BM_WORDSIZE)) {
    return -1;
  }
  return size;
}
/* return the size, in bytes, of the data area of the bitmap. Return
   -1 if the size does not fit into the ptrdiff_t type; however, this
   cannot happen if the bitmap is well-formed, i.e., if created with
   bm_new or bm_dup. */
static inline ptrdiff_t bm_size(const potrace_bitmap_t *bm) {
  return getsize(bm->dy, bm->h);
}
/* calculate the base address of the bitmap data. Assume that the
   bitmap is well-formed, i.e., its size fits into the ptrdiff_t type.
   This is the case if created with bm_new or bm_dup. The base address
   may differ from bm->map if dy is negative */
static inline potrace_word *bm_base(const potrace_bitmap_t *bm) {
  int dy = bm->dy;
  if (dy >= 0 || bm->h == 0) {
    return bm->map;
  } else {
    return bm_scanline(bm, bm->h - 1);
  }  
}
/* free the given bitmap. Leaves errno untouched. */
static inline void bm_free(potrace_bitmap_t *bm) {
  if (bm && bm->map) {
    free(bm_base(bm));
  }
  free(bm);
}
/* return new bitmap initialized to 0. NULL with errno on error.
   Assumes w, h >= 0. */
static inline potrace_bitmap_t *bm_new(int w, int h) {
  potrace_bitmap_t *bm;
  int dy = w == 0 ? 0 : (w - 1) / BM_WORDBITS + 1;
  ptrdiff_t size;
  size = getsize(dy, h);
  if (size < 0) {
    errno = ENOMEM;
    return NULL;
  }
  if (size == 0) {
    size = BM_WORDSIZE; /* make sure calloc() doesn't return NULL */
  } 
  bm = (potrace_bitmap_t *) malloc(sizeof(potrace_bitmap_t));
  if (!bm) {
    return NULL;
  }
  bm->w = w;
  bm->h = h;
  bm->dy = dy;
  bm->map = (potrace_word *) calloc(1, size);
  if (!bm->map) {
    free(bm);
    return NULL;
  }
  return bm;
}
/* clear the given bitmap. Set all bits to c. Assumes a well-formed
   bitmap. */
static inline void bm_clear(potrace_bitmap_t *bm, int c) {
  /* Note: if the bitmap was created with bm_new, then it is
     guaranteed that size will fit into the ptrdiff_t type. */
  ptrdiff_t size = bm_size(bm);
  memset(bm_base(bm), c ? -1 : 0, size);
}
/* duplicate the given bitmap. Return NULL on error with errno
   set. Assumes a well-formed bitmap. */
static inline potrace_bitmap_t *bm_dup(const potrace_bitmap_t *bm) {
  potrace_bitmap_t *bm1 = bm_new(bm->w, bm->h);
  int y;
  
  if (!bm1) {
    return NULL;
  }
  for (y=0; y < bm->h; y++) {
    memcpy(bm_scanline(bm1, y), bm_scanline(bm, y), (size_t)bm1->dy * (size_t)BM_WORDSIZE);
  }
  return bm1;
}
/* invert the given bitmap. */
static inline void bm_invert(potrace_bitmap_t *bm) {
  int dy = bm->dy;
  int y;
  int i;
  potrace_word *p;
  if (dy < 0) {
    dy = -dy;
  }
  
  for (y=0; y < bm->h; y++) {
    p = bm_scanline(bm, y);
    for (i=0; i < dy; i++) {
      p[i] ^= BM_ALLBITS;
    }
  }
}
/* turn the given bitmap upside down. This does not move the bitmap
   data or change the bm_base() address. */
static inline void bm_flip(potrace_bitmap_t *bm) {
  int dy = bm->dy;
  if (bm->h == 0 || bm->h == 1) {
    return;
  }
  
  bm->map = bm_scanline(bm, bm->h - 1);
  bm->dy = -dy;
}
/* resize the bitmap to the given new height. The bitmap data remains
   bottom-aligned (truncated at the top) when dy >= 0 and top-aligned
   (truncated at the bottom) when dy < 0. Return 0 on success, or 1 on
   error with errno set. If the new height is <= the old one, no error
   should occur. If the new height is larger, the additional bitmap
   data is *not* initialized. */
static inline int bm_resize(potrace_bitmap_t *bm, int h) {
  int dy = bm->dy;
  ptrdiff_t newsize;
  potrace_word *newmap;
  if (dy < 0) {
    bm_flip(bm);
  }
  
  newsize = getsize(dy, h);
  if (newsize < 0) {
    errno = ENOMEM;
    goto error;
  }
  if (newsize == 0) {
    newsize = BM_WORDSIZE; /* make sure realloc() doesn't return NULL */
  }
  
  newmap = (potrace_word *)realloc(bm->map, newsize);
  if (newmap == NULL) {
    goto error;
  }
  bm->map = newmap;
  bm->h = h;
  if (dy < 0) {
    bm_flip(bm);
  }
  return 0;
  
 error:
  if (dy < 0) {
    bm_flip(bm);
  }
  return 1;  
}
#endif /* BITMAP_H */
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifndef DECOMPOSE_H
#define DECOMPOSE_H
int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param, progress_t *progress);
#endif /* DECOMPOSE_H */
/* ---------------------------------------------------------------------- */
/* deterministically and efficiently hash (x,y) into a pseudo-random bit */
static inline int detrand(int x, int y) {
  unsigned int z;
  static const unsigned char t[256] = { 
    /* non-linear sequence: constant term of inverse in GF(8), 
       mod x^8+x^4+x^3+x+1 */
    0, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 
    0, 1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 
    0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 
    1, 0, 1, 1, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 
    0, 0, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 0, 
    0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 1, 1, 1, 0, 1, 0, 
    0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 
    0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 1, 1, 0, 0, 1, 1, 0, 0, 1, 
    1, 0, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 
    0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 
    1, 1, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 
  };
  /* 0x04b3e375 and 0x05a8ef93 are chosen to contain every possible
     5-bit sequence */
  z = ((0x04b3e375 * x) ^ y) * 0x05a8ef93;
  z = t[z & 0xff] ^ t[(z>>8) & 0xff] ^ t[(z>>16) & 0xff] ^ t[(z>>24) & 0xff];
  return z;
}
/* ---------------------------------------------------------------------- */
/* auxiliary bitmap manipulations */
/* set the excess padding to 0 */
static void bm_clearexcess(potrace_bitmap_t *bm) {
  potrace_word mask;
  int y;
  if (bm->w % BM_WORDBITS != 0) {
    mask = BM_ALLBITS << (BM_WORDBITS - (bm->w % BM_WORDBITS));
    for (y=0; y<bm->h; y++) {
      *bm_index(bm, bm->w, y) &= mask;
    }
  }
}
struct bbox_s {
  int x0, x1, y0, y1;    /* bounding box */
};
typedef struct bbox_s bbox_t;
/* clear the bm, assuming the bounding box is set correctly (faster
   than clearing the whole bitmap) */
static void clear_bm_with_bbox(potrace_bitmap_t *bm, bbox_t *bbox) {
  int imin = (bbox->x0 / BM_WORDBITS);
  int imax = ((bbox->x1 + BM_WORDBITS-1) / BM_WORDBITS);
  int i, y;
  for (y=bbox->y0; y<bbox->y1; y++) {
    for (i=imin; i<imax; i++) {
      bm_scanline(bm, y)[i] = 0;
    }
  }
}
/* ---------------------------------------------------------------------- */
/* auxiliary functions */
/* return the "majority" value of bitmap bm at intersection (x,y). We
   assume that the bitmap is balanced at "radius" 1.  */
static int majority(potrace_bitmap_t *bm, int x, int y) {
  int i, a, ct;
  for (i=2; i<5; i++) { /* check at "radius" i */
    ct = 0;
    for (a=-i+1; a<=i-1; a++) {
      ct += BM_GET(bm, x+a, y+i-1) ? 1 : -1;
      ct += BM_GET(bm, x+i-1, y+a-1) ? 1 : -1;
      ct += BM_GET(bm, x+a-1, y-i) ? 1 : -1;
      ct += BM_GET(bm, x-i, y+a) ? 1 : -1;
    }
    if (ct>0) {
      return 1;
    } else if (ct<0) {
      return 0;
    }
  }
  return 0;
}
/* ---------------------------------------------------------------------- */
/* decompose image into paths */
/* efficiently invert bits [x,infty) and [xa,infty) in line y. Here xa
   must be a multiple of BM_WORDBITS. */
static void xor_to_ref(potrace_bitmap_t *bm, int x, int y, int xa) {
  int xhi = x & -BM_WORDBITS;
  int xlo = x & (BM_WORDBITS-1);  /* = x % BM_WORDBITS */
  int i;
  
  if (xhi<xa) {
    for (i = xhi; i < xa; i+=BM_WORDBITS) {
      *bm_index(bm, i, y) ^= BM_ALLBITS;
    }
  } else {
    for (i = xa; i < xhi; i+=BM_WORDBITS) {
      *bm_index(bm, i, y) ^= BM_ALLBITS;
    }
  }
  /* note: the following "if" is needed because x86 treats a<<b as
     a<<(b&31). I spent hours looking for this bug. */
  if (xlo) {
    *bm_index(bm, xhi, y) ^= (BM_ALLBITS << (BM_WORDBITS - xlo));
  }
}
/* a path is represented as an array of points, which are thought to
   lie on the corners of pixels (not on their centers). The path point
   (x,y) is the lower left corner of the pixel (x,y). Paths are
   represented by the len/pt components of a path_t object (which
   also stores other information about the path) */
/* xor the given pixmap with the interior of the given path. Note: the
   path must be within the dimensions of the pixmap. */
static void xor_path(potrace_bitmap_t *bm, path_t *p) {
  int xa, x, y, k, y1;
  if (p->priv->len <= 0) {  /* a path of length 0 is silly, but legal */
    return;
  }
  y1 = p->priv->pt[p->priv->len-1].y;
  xa = p->priv->pt[0].x & -BM_WORDBITS;
  for (k=0; k<p->priv->len; k++) {
    x = p->priv->pt[k].x;
    y = p->priv->pt[k].y;
    if (y != y1) {
      /* efficiently invert the rectangle [x,xa] x [y,y1] */
      xor_to_ref(bm, x, min(y,y1), xa);
      y1 = y;
    }
  }
}
/* Find the bounding box of a given path. Path is assumed to be of
   non-zero length. */
static void setbbox_path(bbox_t *bbox, path_t *p) {
  int x, y;
  int k;
  bbox->y0 = INT_MAX;
  bbox->y1 = 0;
  bbox->x0 = INT_MAX;
  bbox->x1 = 0;
  for (k=0; k<p->priv->len; k++) {
    x = p->priv->pt[k].x;
    y = p->priv->pt[k].y;
    if (x < bbox->x0) {
      bbox->x0 = x;
    }
    if (x > bbox->x1) {
      bbox->x1 = x;
    }
    if (y < bbox->y0) {
      bbox->y0 = y;
    }
    if (y > bbox->y1) {
      bbox->y1 = y;
    }
  }
}
/* compute a path in the given pixmap, separating black from white.
   Start path at the point (x0,x1), which must be an upper left corner
   of the path. Also compute the area enclosed by the path. Return a
   new path_t object, or NULL on error (note that a legitimate path
   cannot have length 0). Sign is required for correct interpretation
   of turnpolicies. */
static path_t *findpath(potrace_bitmap_t *bm, int x0, int y0, int sign, int turnpolicy) {
  int x, y, dirx, diry, len, size;
  uint64_t area;
  int c, d, tmp;
  point_t *pt, *pt1;
  path_t *p = NULL;
  x = x0;
  y = y0;
  dirx = 0;
  diry = -1;
  len = size = 0;
  pt = NULL;
  area = 0;
  
  while (1) {
    /* add point to path */
    if (len>=size) {
      size += 100;
      size = (int)(1.3 * size);
      pt1 = (point_t *)realloc(pt, size * sizeof(point_t));
      if (!pt1) {
	goto error;
      }
      pt = pt1;
    }
    pt[len].x = x;
    pt[len].y = y;
    len++;
    
    /* move to next point */
    x += dirx;
    y += diry;
    area += x*diry;
    
    /* path complete? */
    if (x==x0 && y==y0) {
      break;
    }
    
    /* determine next direction */
    c = BM_GET(bm, x + (dirx+diry-1)/2, y + (diry-dirx-1)/2);
    d = BM_GET(bm, x + (dirx-diry-1)/2, y + (diry+dirx-1)/2);
    
    if (c && !d) {               /* ambiguous turn */
      if (turnpolicy == POTRACE_TURNPOLICY_RIGHT
	  || (turnpolicy == POTRACE_TURNPOLICY_BLACK && sign == '+')
	  || (turnpolicy == POTRACE_TURNPOLICY_WHITE && sign == '-')
	  || (turnpolicy == POTRACE_TURNPOLICY_RANDOM && detrand(x,y))
	  || (turnpolicy == POTRACE_TURNPOLICY_MAJORITY && majority(bm, x, y))
	  || (turnpolicy == POTRACE_TURNPOLICY_MINORITY && !majority(bm, x, y))) {
	tmp = dirx;              /* right turn */
	dirx = diry;
	diry = -tmp;
      } else {
	tmp = dirx;              /* left turn */
	dirx = -diry;
	diry = tmp;
      }
    } else if (c) {              /* right turn */
      tmp = dirx;
      dirx = diry;
      diry = -tmp;
    } else if (!d) {             /* left turn */
      tmp = dirx;
      dirx = -diry;
      diry = tmp;
    }
  } /* while this path */
  /* allocate new path object */
  p = path_new();
  if (!p) {
    goto error;
  }
  p->priv->pt = pt;
  p->priv->len = len;
  p->area = area <= INT_MAX ? area : INT_MAX; /* avoid overflow */
  p->sign = sign;
  return p;
 
 error:
   free(pt);
   return NULL; 
}
/* Give a tree structure to the given path list, based on "insideness"
   testing. I.e., path A is considered "below" path B if it is inside
   path B. The input pathlist is assumed to be ordered so that "outer"
   paths occur before "inner" paths. The tree structure is stored in
   the "childlist" and "sibling" components of the path_t
   structure. The linked list structure is also changed so that
   negative path components are listed immediately after their
   positive parent.  Note: some backends may ignore the tree
   structure, others may use it e.g. to group path components. We
   assume that in the input, point 0 of each path is an "upper left"
   corner of the path, as returned by bm_to_pathlist. This makes it
   easy to find an "interior" point. The bm argument should be a
   bitmap of the correct size (large enough to hold all the paths),
   and will be used as scratch space. Return 0 on success or -1 on
   error with errno set. */
static void pathlist_to_tree(path_t *plist, potrace_bitmap_t *bm) {
  path_t *p, *p1;
  path_t *heap, *heap1;
  path_t *cur;
  path_t *head;
  path_t **plist_hook;          /* for fast appending to linked list */
  path_t **hook_in, **hook_out; /* for fast appending to linked list */
  bbox_t bbox;
  
  bm_clear(bm, 0);
  /* save original "next" pointers */
  list_forall(p, plist) {
    p->sibling = p->next;
    p->childlist = NULL;
  }
  
  heap = plist;
  /* the heap holds a list of lists of paths. Use "childlist" field
     for outer list, "next" field for inner list. Each of the sublists
     is to be turned into a tree. This code is messy, but it is
     actually fast. Each path is rendered exactly once. We use the
     heap to get a tail recursive algorithm: the heap holds a list of
     pathlists which still need to be transformed. */
  while (heap) {
    /* unlink first sublist */
    cur = heap;
    heap = heap->childlist;
    cur->childlist = NULL;
  
    /* unlink first path */
    head = cur;
    cur = cur->next;
    head->next = NULL;
    /* render path */
    xor_path(bm, head);
    setbbox_path(&bbox, head);
    /* now do insideness test for each element of cur; append it to
       head->childlist if it's inside head, else append it to
       head->next. */
    hook_in=&head->childlist;
    hook_out=&head->next;
    list_forall_unlink(p, cur) {
      if (p->priv->pt[0].y <= bbox.y0) {
	list_insert_beforehook(p, hook_out);
	/* append the remainder of the list to hook_out */
	*hook_out = cur;
	break;
      }
      if (BM_GET(bm, p->priv->pt[0].x, p->priv->pt[0].y-1)) {
	list_insert_beforehook(p, hook_in);
      } else {
	list_insert_beforehook(p, hook_out);
      }
    }
    /* clear bm */
    clear_bm_with_bbox(bm, &bbox);
    /* now schedule head->childlist and head->next for further
       processing */
    if (head->next) {
      head->next->childlist = heap;
      heap = head->next;
    }
    if (head->childlist) {
      head->childlist->childlist = heap;
      heap = head->childlist;
    }
  }
  
  /* copy sibling structure from "next" to "sibling" component */
  p = plist;
  while (p) {
    p1 = p->sibling;
    p->sibling = p->next;
    p = p1;
  }
  /* reconstruct a new linked list ("next") structure from tree
     ("childlist", "sibling") structure. This code is slightly messy,
     because we use a heap to make it tail recursive: the heap
     contains a list of childlists which still need to be
     processed. */
  heap = plist;
  if (heap) {
    heap->next = NULL;  /* heap is a linked list of childlists */
  }
  plist = NULL;
  plist_hook = &plist;
  while (heap) {
    heap1 = heap->next;
    for (p=heap; p; p=p->sibling) {
      /* p is a positive path */
      /* append to linked list */
      list_insert_beforehook(p, plist_hook);
      
      /* go through its children */
      for (p1=p->childlist; p1; p1=p1->sibling) {
	/* append to linked list */
	list_insert_beforehook(p1, plist_hook);
	/* append its childlist to heap, if non-empty */
	if (p1->childlist) {
	  list_append(path_t, heap1, p1->childlist);
	}
      }
    }
    heap = heap1;
  }
  return;
}
/* find the next set pixel in a row <= y. Pixels are searched first
   left-to-right, then top-down. In other words, (x,y)<(x',y') if y>y'
   or y=y' and x<x'. If found, return 0 and store pixel in
   (*xp,*yp). Else return 1. Note that this function assumes that
   excess bytes have been cleared with bm_clearexcess. */
static int findnext(potrace_bitmap_t *bm, int *xp, int *yp) {
  int x;
  int y;
  int x0;
  x0 = (*xp) & ~(BM_WORDBITS-1);
  for (y=*yp; y>=0; y--) {
    for (x=x0; x<bm->w && x>=0; x+=(unsigned)BM_WORDBITS) {
      if (*bm_index(bm, x, y)) {
	while (!BM_GET(bm, x, y)) {
	  x++;
	}
	/* found */
	*xp = x;
	*yp = y;
	return 0;
      }
    }
    x0 = 0;
  }
  /* not found */
  return 1;
}
/* Decompose the given bitmap into paths. Returns a linked list of
   path_t objects with the fields len, pt, area, sign filled
   in. Returns 0 on success with plistp set, or -1 on error with errno
   set. */
int bm_to_pathlist(const potrace_bitmap_t *bm, path_t **plistp, const potrace_param_t *param, progress_t *progress) {
  int x;
  int y;
  path_t *p;
  path_t *plist = NULL;  /* linked list of path objects */
  path_t **plist_hook = &plist;  /* used to speed up appending to linked list */
  potrace_bitmap_t *bm1 = NULL;
  int sign;
  bm1 = bm_dup(bm);
  if (!bm1) {
    goto error;
  }
  /* be sure the byte padding on the right is set to 0, as the fast
     pixel search below relies on it */
  bm_clearexcess(bm1);
  /* iterate through components */
  x = 0;
  y = bm1->h - 1;
  while (findnext(bm1, &x, &y) == 0) { 
    /* calculate the sign by looking at the original */
    sign = BM_GET(bm, x, y) ? '+' : '-';
    /* calculate the path */
    p = findpath(bm1, x, y+1, sign, param->turnpolicy);
    if (p==NULL) {
      goto error;
    }
    /* update buffered image */
    xor_path(bm1, p);
    /* if it's a turd, eliminate it, else append it to the list */
    if (p->area <= param->turdsize) {
      path_free(p);
    } else {
      list_insert_beforehook(p, plist_hook);
    }
    if (bm1->h > 0) { /* to be sure */
      progress_update(1-y/(double)bm1->h, progress);
    }
  }
  pathlist_to_tree(plist, bm1);
  bm_free(bm1);
  *plistp = plist;
  progress_update(1.0, progress);
  return 0;
 error:
  bm_free(bm1);
  list_forall_unlink(p, plist) {
    path_free(p);
  }
  return -1;
}
/* Copyright (C) 2001-2019 Peter Selinger.
   This file is part of Potrace. It is free software and it is covered
   by the GNU General Public License. See the file COPYING for details. */
#ifdef HAVE_CONFIG_H
#endif
/* default parameters */
static const potrace_param_t param_default = {
  2,                             /* turdsize */
  POTRACE_TURNPOLICY_MINORITY,   /* turnpolicy */
  1.0,                           /* alphamax */
  1,                             /* opticurve */
  0.2,                           /* opttolerance */
  {
    NULL,                        /* callback function */
    NULL,                        /* callback data */
    0.0, 1.0,                    /* progress range */
    0.0,                         /* granularity */
  },
};
/* Return a fresh copy of the set of default parameters, or NULL on
   failure with errno set. */
potrace_param_t *potrace_param_default(void) {
  potrace_param_t *p;
  p = (potrace_param_t *) malloc(sizeof(potrace_param_t));
  if (!p) {
    return NULL;
  }
  memcpy(p, &param_default, sizeof(potrace_param_t));
  return p;
}
/* On success, returns a Potrace state st with st->status ==
   POTRACE_STATUS_OK. On failure, returns NULL if no Potrace state
   could be created (with errno set), or returns an incomplete Potrace
   state (with st->status == POTRACE_STATUS_INCOMPLETE, and with errno
   set). Complete or incomplete Potrace state can be freed with
   potrace_state_free(). */
potrace_state_t *potrace_trace(const potrace_param_t *param, const potrace_bitmap_t *bm) {
  int r;
  path_t *plist = NULL;
  potrace_state_t *st;
  progress_t prog;
  progress_t subprog;
  
  /* prepare private progress bar state */
  prog.callback = param->progress.callback;
  prog.data = param->progress.data;
  prog.min = param->progress.min;
  prog.max = param->progress.max;
  prog.epsilon = param->progress.epsilon;
  prog.d_prev = param->progress.min;
  /* allocate state object */
  st = (potrace_state_t *)malloc(sizeof(potrace_state_t));
  if (!st) {
    return NULL;
  }
  progress_subrange_start(0.0, 0.1, &prog, &subprog);
  /* process the image */
  r = bm_to_pathlist(bm, &plist, param, &subprog);
  if (r) {
    free(st);
    return NULL;
  }
  st->status = POTRACE_STATUS_OK;
  st->plist = plist;
  st->priv = NULL;  /* private state currently unused */
  progress_subrange_end(&prog, &subprog);
  progress_subrange_start(0.1, 1.0, &prog, &subprog);
  /* partial success. */
  r = process_path(plist, param, &subprog);
  if (r) {
    st->status = POTRACE_STATUS_INCOMPLETE;
  }
  progress_subrange_end(&prog, &subprog);
  return st;
}
/* free a Potrace state, without disturbing errno. */
void potrace_state_free(potrace_state_t *st) {
  pathlist_free(st->plist);
  free(st);
}
/* free a parameter list, without disturbing errno. */
void potrace_param_free(potrace_param_t *p) {
  free(p);
}
const char *potrace_version(void) {
  return "potracelib " VERSION "";
}
