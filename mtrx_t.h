#ifndef __mtrx_t_h__
#define __mtrx_t_h__  1

#if defined (_WIN32) || defined (_WIN64)
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <math.h>

typedef double T;       /* must set T typedef for type */

typedef struct {        /* matrix struct */
    size_t rows, cols, rowmax, colmax;
    T **mtrx;
} mtrx_t;

typedef struct {        /* vector struct (not fully implemented) */
    size_t nelem, nmax;
    T *vect;
} vect_t;

#define ROWSZ 8             /* initial allocation size */
#define COLSZ 8

#define MAXC 2048           /* max chars per-row to read */

#define T_FP_MAX  1.0e-20   /* floating-point values considered zero */
#define T_FP_MIN -1.0e-20

enum { ROWOP, COLOP };  /* row operation/column operation consts */

/* __func__ is not defined in pre C99 so provide fallback */
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

/** allocate vector of n elements */
T *vect_calloc (const size_t n);
/** allocate matrix of (m x n) */
T **mtrx_calloc (const size_t m, const size_t n);
/** allocate/initialize struct T and (m) pointers, set rowmax/colmax */
mtrx_t *mtrx_create_ptrs (const size_t m);
/** allocate/initialize struct T and (m x n) matrix, set rowmax/colmax */
mtrx_t *mtrx_create_fixed (const size_t m, const size_t n);
/** realloc matrix pointers to twice current */
T **mtrx_realloc_ptrs (mtrx_t *m);
/** realloc matrix_fixed, which = 0 realloc ptrs, 1 column storage.
 *  custom realloc used as no need to zero new pointers (they will be
 *  allocated) and row realloc simply saves the function call overhead
 *  for each row realloc.
 */
T **mtrx_realloc_fixed (mtrx_t *m, const int which);
/** read (m x n) matrix from file stream into dynamically sized T *mtrx.
 *  pointers are reallocated x2 as needed, row storage is reallocated
 *  x2 as required to store all values in 1st row of data, then row storage
 *  is reallocate to shrink row allocation to exact number of columns and
 *  T->colmax is set. following read of all data, pointer storage is reduced
 *  to T->nrows and T->rowmax is set.
 */
mtrx_t *mtrx_read_alloc (FILE *fp);
mtrx_t *mtrx_read_alloc_buf (char *textviewbuf);
/** matr_read_fixed, alloc/read (m x n) matrix from file stream.
 *  reallocate as required, but should not be required if correct initial
 *  size given. matrix is resized to exact number of rows and pointers
 *  before return.
 */
mtrx_t *mtrx_read_fixed (FILE *fp, const size_t rows, const size_t cols);
/** output matrix to stdout with/pad */
void mtrx_prn (const mtrx_t *m, int width);
/** print a (m x m+1) system of linear equations with separtor */
void mtrx_sys_prn (const mtrx_t *m, int width);
/** print a (m x m) matrix from (m x n) with n > m */
void mtrx_prn_sq (const mtrx_t *m, int width);
/** copy a struct matrix */
mtrx_t *mtrx_copy (const mtrx_t *ma);
/** free a (m x n) matrix and struct for m->nrows pointers */
void mtrx_free (mtrx_t *m);

/** add two struct matricies */
mtrx_t *mtrx_add (const mtrx_t *ma, const mtrx_t *mb);
/** subtract two struct matricies */
mtrx_t *mtrx_sub (const mtrx_t *ma, const mtrx_t *mb);
/** multiply two struct matricies */
mtrx_t *mtrx_mult (const mtrx_t *ma, const mtrx_t *mb);
/** transpose matrix */
mtrx_t *mtrx_trans (const mtrx_t *m);

/** solve system of equations */
T *mtrx_solv (const mtrx_t *m, const T *v);
/** mtrx_solv_cmb - solves system of eq where m contains solution vect.
 *  solves system where m contains the solution vector as the last column
 *  in the form {m} = {m'}[v] where {m'} is the coefficient matrix and [v]
 *  the solution vector. it is a wrapper for mtrx_solv() above. returns
 *  a vector containing the unique solution or NULL if {m'} is singular
 *  or the solutions are infinite or trivial.
 */
T *mtrx_solv_cmb (mtrx_t *m);
/** Guass-Jordan elimination with full pivoting.
 *  'a' is coefficient matrix with constant vector as last col.
 *  on return 'a' contains matrix inverse, last col contains
 *  solution vector.
 */
void mtrx_solv_gaussj (T **a, const size_t n);
void mtrx_solv_gaussj_v (T **a, T *v, const size_t n);
/** mtrx_solv_gaussj_inv returns mtrx_t containing inverse + solution.
 *  wrapper preserving original mtrx_t, allocating a copy before calling
 *  mtrx_solv_gaussj to create inverse of m and solution vector in newly
 *  allocted mtrx_t. user is responsible for freeing return.
 */
mtrx_t *mtrx_solv_gaussj_inv (const mtrx_t *m);
/** mtrx_get_sol_v return solution vector from inverse+solution mtrx_t.
 *  m must be a square matrix + constant vector as final column. returns
 *  allocated solution vector on success, NULL otherwise.
 */
T *mtrx_get_sol_v (const mtrx_t *m);

/** qsort compare ascending for matrix */
int mtrx_compare_rows_asc (const void *a, const void *b);

#endif
