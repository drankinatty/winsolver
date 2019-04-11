#include "mtrx_t.h"
#include "memrealloc.h"

/** debug - print all pointer values for mtrx_t->mtrx & mtrx_t->mtrx[i] */
void prnptrs (mtrx_t *m)
{
    size_t i;

    printf ("m->mtrx: %p\n", (void*)m->mtrx);
    for (i = 0; i < m->rowmax; i++)
        printf ("  m->mtrx[%2zu]: %p\n", i, (void*)m->mtrx[i]);
    putchar ('\n');
}

/** allocate/initialize struct T and (m) pointers, set rowmax/colmax */
mtrx_t *mtrx_create_ptrs (const size_t m)
{
    mtrx_t *ms = calloc (1, sizeof *ms);        /* calloc struct */
    if (!ms) {
        fprintf (stderr, "%s() error: calloc-m.\n", __func__);
        return NULL;
    }

    ms->mtrx = calloc (m, sizeof *ms->mtrx);    /* calloc pointers */
    if (!ms->mtrx) {    /* validate allocation  */
        fprintf (stderr, "%s() error: malloc m->mtrx failed.\n",
                __func__);
        free (ms);
        return NULL;
    }

    ms->rowmax = m;
    ms->colmax = COLSZ;

    return ms;
}

/** allocate vector of n elements */
T *vect_calloc (const size_t n)
{
    T *vect = calloc (n, sizeof *vect);     /* calloc vector */

    if (!vect) {
        perror ("calloc-vector");
        fprintf (stderr, "%s() error: calloc-vect failed.\n",
                __func__);
        return NULL;
    }

    return vect;
}

/** allocate matrix of (m x n) */
T **mtrx_calloc (const size_t m, const size_t n)
{
    register size_t i = 0;
    T **mtrx = calloc (m, sizeof *mtrx);    /* calloc pointers */
    if (!mtrx) {
        perror ("calloc-pointers");
        fprintf (stderr, "%s() error: calloc mtrx failed.\n",
                __func__);
        return NULL;
    }

    /* allocate m rows of n elements */
    for (i = 0; i < m; i++) {
        if (!(mtrx[i] = calloc (n, sizeof *mtrx[i]))) {
            perror ("calloc-mtrx[i]");
            fprintf (stderr, "%s() error: calloc-mtrx[%zu] failed.\n",
                    __func__, i);
            while (i--)         /* free all rows allocated */
                free (mtrx[i]);
            free (mtrx);        /* free pointers */
            return NULL;
        }
    }

    return mtrx;
}

/** allocate/initialize struct T and (m x n) matrix, set rowmax/colmax */
mtrx_t *mtrx_create_fixed (const size_t m, const size_t n)
{
    mtrx_t *ms = calloc (1, sizeof *ms);             /* calloc struct */
    if (!ms) {
        fprintf (stderr, "%s() error: calloc-m.\n", __func__);
        return NULL;
    }

    if (!(ms->mtrx = mtrx_calloc (m, n))) { /* allocate/validate matrix */
        free (ms);
        return NULL;
    }

    ms->rowmax = m;     /* set rowmax/colmax to current allocation size */
    ms->colmax = n;

    return ms;          /* return matrix struct */
}

/** realloc matrix pointers to twice current */
T **mtrx_realloc_ptrs (mtrx_t *m)
{
    if (!m) {
        fprintf (stderr, "%s() error: struct parameter NULL.\n",
                __func__);
        return NULL;
    }
    /* realloc m->mtrx to 2 x m->rowmax, zero new ptrs, set m->rowmax */
    m->mtrx = xrealloc_x2 (m->mtrx, sizeof *m->mtrx, &m->rowmax);

    return m->mtrx;
}

/** realloc matrix_fixed, which = 0 realloc ptrs, 1 column storage.
 *  custom realloc used as no need to zero new pointers (they will be
 *  allocated) and row realloc simply saves the function call overhead
 *  for each row realloc.
 */
T **mtrx_realloc_fixed (mtrx_t *m, const int which)
{
    register size_t i;

    if (!m) {
        fprintf (stderr, "%s() error: struct parameter NULL.\n",
                __func__);
        return NULL;
    }
    if (which == ROWOP) {   /* reallocate pointers & new rows */
        void *tmp = realloc (m->mtrx, 2 * m->rowmax * sizeof *m->mtrx);
        if (!tmp) {
            fprintf (stderr, "%s() error: ptr realloc failed.\n",
                    __func__);
            exit (EXIT_FAILURE);
        }
        m->mtrx = tmp;      /* assign new block to m->mtrx */
        for (i = 0; i < m->rowmax; i++) { /* allocate rows */
            m->mtrx[i + m->rowmax] = calloc (m->colmax, sizeof **m->mtrx);
            if (!m->mtrx[i + m->rowmax]) {
                fprintf (stderr, "%s() error: ptr realloc failed.\n",
                        __func__);
                exit (EXIT_FAILURE);
            }
        }
        m->rowmax *= 2; /* incriment m->rowmax */
        return m->mtrx;
    }

    /* reallocate row storage for all allocated rows */
    for (i = 0; i < m->rowmax; i++) {
        void *tmp = realloc (m->mtrx[i],
                            2 * m->colmax * sizeof *m->mtrx[i]);
        if (!tmp) {
            fprintf (stderr, "%s() error: realloc failed row[%zu].\n",
                    __func__, i);
            exit (EXIT_FAILURE);
        }
        m->mtrx[i] = tmp;     /* assign new block to (m->mtrx)[i] */
        memset (m->mtrx[i] + m->colmax, 0,     /* zero new memory */
                m->colmax * sizeof *m->mtrx[i]);
    }
    m->colmax *= 2;     /* increment colmax only if all succeed */

    return m->mtrx;
}

/** parse_dbl_array from any buffer regardless of format.
 *  uses strtod to parse all values from buf regardless of intervening
 *  characters. array initially sized to *nelem, reallocates as needed,
 *  a final realloc sizes the array to the number of elements, *nelem
 *  updated to final number of elements. returns allocated array on
 *  success, NULL otherwise. user responsible for freeing non-NULL return.
 */
T *parse_dbl_array (char *buf, size_t *nelem)
{
    T *array = NULL;
    char *nptr = buf, *endptr;
    size_t n = 0;

    if (!*nelem)
        *nelem = COLSZ;

    if (!(array = calloc (*nelem, sizeof *array))) {
        perror ("calloc-array");
        exit (EXIT_FAILURE);
    }

    for (;;) {
        errno = 0;          /* reset errno before each conversion */
        if (n == *nelem)    /* check if realloc required */
            array = xrealloc_x2 (array, sizeof *array, nelem);
        /* skip any non-digit characters
         *
         * TODO: update to check digit after "-." (e.g. "-.4")
         */
        while (*nptr && ((*nptr != '-'  && *nptr != '+' && *nptr != '.' &&
                    (*nptr < '0' || '9' < *nptr)) ||
                    ((*nptr == '-' || *nptr == '+' || *nptr == '.') &&
                    (*(nptr+1) != '.' && (*(nptr+1) < '0' || '9' < *(nptr+1))))))
            nptr++;

        if (!*nptr) /* check if at end of buf */
            break;
        array[n] = strtod (nptr, &endptr);  /* call strtod */
        /* validate strod conversion */
        if (nptr == endptr) {   /* no digits converted */
            fputs ("error: no digits converted.\n", stderr);
            break;
        }
        else if (errno) {   /* error in conversion under/overflow */
            fputs ("error: in conversion.\n", stderr);
            break;
        }
        n++;    /* increment element count */

        /* skip delimiters/move pointer to next digit
         * (avoids unnecessary realloc if no values remain)
         */
        while (*endptr && *endptr != '-' && *endptr != '+' && *endptr != '.'
                && (*endptr < '0' || *endptr > '9'))
            endptr++;
        if (!*endptr)       /* break if end of string */
            break;
        nptr = endptr;      /* update pointer to end pointer */
    }
    if (!n) {           /* validate elements stored */
        free (array);
        return NULL;
    }
    if (n < *nelem) {   /* realloc to fit nrows, set rowmax */
        array = xrealloc_fixed (array, sizeof *array, *nelem, n);
        *nelem = n;     /* update rowmax */
    }

    return array;
}

/** read (m x n) matrix from file stream into dynamically sized T *mtrx.
 *  pointers are reallocated x2 as needed, row storage is reallocated
 *  x2 as required to store all values in 1st row of data, then row storage
 *  is reallocate to shrink row allocation to exact number of columns and
 *  T->colmax is set. following read of all data, pointer storage is reduced
 *  to T->nrows and T->rowmax is set.
 */
mtrx_t *mtrx_read_alloc (FILE *fp)
{
    char buf[MAXC];                         /* buffer for line */
    mtrx_t *m = mtrx_create_ptrs (ROWSZ);   /* allocate ROWSIZE pointers */
    if (!m)                                 /* validate */
        return NULL;

    while (fgets (buf, MAXC, fp)) {         /* read each line */
        size_t col = 0;

        if (m->rows == m->rowmax)   /* check if realloc needed */
            mtrx_realloc_ptrs (m);

        /* create array of doubles from buf */
        m->mtrx[m->rows] = parse_dbl_array (buf, &m->colmax);
        if (!m->mtrx[m->rows]) {    /* validate */
            fprintf (stderr, "%s() error: no values for mtrx[%zu][%zu].\n",
                    __func__, m->rows, m->cols);
            exit (EXIT_FAILURE);
        }
        col = m->colmax;

        if (!m->rows)               /* if first row */
            m->cols = col;          /* set number of columns */
        else if (col != m->cols)    /* check all rows have cols values */
            fprintf (stderr, "%s() error: column mismatch row[%zu]\n",
                    __func__, m->rows);
        m->rows++;  /* increment row count */
    }

    if (!m->rows) {
        free (m->mtrx);
        free (m);
        return (m = NULL);
    }

    if (m->rows < m->rowmax) {  /* realloc to fit nrows, set rowmax */
        m->mtrx = xrealloc_fixed (m->mtrx, sizeof *m->mtrx, m->rowmax, m->rows);
        m->rowmax = m->rows;    /* update rowmax */
    }

    return m;   /* return filled and exactly sized matrix struct */
}

mtrx_t *mtrx_read_alloc_buf (char *textviewbuf)
{
    char buf[MAXC];                         /* buffer for line */
    char *p = textviewbuf;
    size_t off = 0;
    int used;
    mtrx_t *m = mtrx_create_ptrs (ROWSZ);   /* allocate ROWSIZE pointers */
    if (!m)                                 /* validate */
        return NULL;

    while (sscanf (p + off, "%[^\n]%n", buf, &used) == 1) {  /* parse each line */
        size_t col = 0;

        while (isspace (p[off + used]))
            used++;
        off += used;

        if (m->rows == m->rowmax)   /* check if realloc needed */
            mtrx_realloc_ptrs (m);

        /* create array of doubles from buf */
        m->mtrx[m->rows] = parse_dbl_array (buf, &m->colmax);
        if (!m->mtrx[m->rows]) {    /* validate */
            fprintf (stderr, "%s() error: no values for mtrx[%zu][%zu].\n",
                    __func__, m->rows, m->cols);
            exit (EXIT_FAILURE);
        }
        col = m->colmax;

        if (!m->rows)               /* if first row */
            m->cols = col;          /* set number of columns */
        else if (col != m->cols)    /* check all rows have cols values */
            fprintf (stderr, "%s() error: column mismatch row[%zu]\n",
                    __func__, m->rows);
        m->rows++;  /* increment row count */
    }

    if (!m->rows) {
        free (m->mtrx);
        free (m);
        return (m = NULL);
    }

    if (m->rows < m->rowmax) {  /* realloc to fit nrows, set rowmax */
        m->mtrx = xrealloc_fixed (m->mtrx, sizeof *m->mtrx, m->rowmax, m->rows);
        m->rowmax = m->rows;    /* update rowmax */
    }

    return m;   /* return filled and exactly sized matrix struct */
}

/** matr_read_fixed, alloc/read (m x n) matrix from file stream.
 *  reallocate as required, but should not be required if correct initial
 *  size given. matrix is resized to exact number of rows and pointers
 *  before return.
 */
mtrx_t *mtrx_read_fixed (FILE *fp, const size_t rows, const size_t cols)
{
    char buf[MAXC];
    mtrx_t *m = mtrx_create_fixed (rows, cols); /* create m/alloc m->mtrx */
    if (!m)
        return NULL;

    while (fgets (buf, MAXC, fp)) {         /* read each line */
        char *nptr = buf, *endptr = buf;
        size_t col = 0;

        if (m->rows == m->rowmax)           /* check ptr realloc req'd */
            mtrx_realloc_fixed (m, ROWOP);  /* custom realloc ptrs & rows */

        /* NOTE cannot use parse_dbl_array () due to mtrx_realloc_fixed ()
         * (added skip of intervening characters) TODO - refactor.
         */
        for (;;) {          /* loop extracting all values in line */
            T v;            /* value of type T */
            errno = 0;      /* reset errno before conversion */
            /* skip any non-digit characters */
            while (*nptr && ((*nptr != '-'  && *nptr != '+' && *nptr != '.' &&
                        (*nptr < '0' || '9' < *nptr)) ||
                        ((*nptr == '-' || *nptr == '+' || *nptr == '.') &&
                        (*(nptr+1) < '0' || '9' < *(nptr+1)))))
                nptr++;

            if (!*nptr) /* check if at end of buf */
                break;
            v = strtod (nptr, &endptr);     /* convert with strtod */
            if (nptr == endptr) {   /* validate digits converted */
                fprintf (stderr, "%s() error: no digits for mtrx[%zu][%zu].\n",
                        __func__, m->rows, m->cols);
                return m;
            }
            else if (errno) {   /* check over/underflow in conversion */
                fprintf (stderr, "%s() error: failed conversion mtrx[%zu][%zu].\n",
                        __func__, m->rows, m->cols);
                return m;
            }
            if (col == m->colmax)           /* check row realloc req'd */
                mtrx_realloc_fixed (m, COLOP);  /* custom row realloc */

            m->mtrx[m->rows][col++] = v;    /* assing value to row/col */

            /* skip delimiters/move pointer to next digit
             * (avoids unnecessary realloc if no values remain)
             */
            while (*endptr && *endptr != '-' && *endptr != '+' && *endptr != '.'
                    && (*endptr < '0' || *endptr > '9'))
                endptr++;
            if (!*endptr)       /* break if end of string */
                break;
            nptr = endptr;      /* update nptr from endptr */
        }
        if (!m->rows) {             /* set m->cols from 1st row */
            m->cols = col;
            if (m->cols < m->colmax) {  /* realloc to fit, set colmax */
                m->mtrx[m->rows] = xrealloc_fixed (m->mtrx[m->rows],
                                    sizeof **m->mtrx, m->colmax, m->cols);
                m->colmax = m->cols;    /* update colmax */
            }
        }
        else if (col != m->cols)    /* verify subsequent rows match */
            fprintf (stderr, "%s() error: column mismatch row[%zu]\n",
                    __func__, m->rows);
        m->rows++;  /* increment row count */
    }
    if (m->rows < m->rowmax) {  /* realloc to fit nrows, set rowmax */
        register size_t i;
        for (i = m->rows; i < m->rowmax; i++)
            free (m->mtrx[i]);
        m->mtrx = xrealloc_fixed (m->mtrx, sizeof *m->mtrx, m->rowmax, m->rows);
        m->rowmax = m->rows;    /* update rowmax */
    }

    return m;
}

/** print a (m x n) matrix */
void mtrx_prn (const mtrx_t *m, int width)
{
    register size_t i, j;

    for (i = 0; i < m->rows; i++) {
        for (j = 0; j < m->cols; j++)
            printf (" % *g", width, m->mtrx[i][j]);
        putchar ('\n');
    }
}

/** print a (m x m+1) system of linear equations with separtor */
void mtrx_sys_prn (const mtrx_t *m, int width)
{
    register size_t i, j;

    if (m->rows >= m->cols) {
        fprintf (stderr, "%s() error - not a linear system matrix, "
                "(rows >= cols).\n", __func__);
        return;
    }

    for (i = 0; i < m->rows; i++) {
        for (j = 0; j < m->cols - 1; j++)
            printf (" %*g", width, m->mtrx[i][j]);
        printf ("  | %*g\n", width, m->mtrx[i][m->cols - 1]);
    }
}

/** print a (m x m) matrix from (m x n) with n > m */
void mtrx_prn_sq (const mtrx_t *m, int width)
{
    register size_t i, j;

    if (m->rows > m->cols) {
        fprintf (stderr, "%s() error - square of m->rows not possible, "
                "(rows > cols).\n", __func__);
        return;
    }

    for (i = 0; i < m->rows; i++) {
        for (j = 0; j < m->rows; j++)
            printf (" % *.3f", width, m->mtrx[i][j]);
        putchar ('\n');
    }
}

/** copy a struct matrix */
mtrx_t *mtrx_copy (const mtrx_t *ma)
{
    register size_t i, j;

    mtrx_t *result = mtrx_create_fixed (ma->rows, ma->cols);
    if (!result)        /* allocate/validate stuct & (m x n) */
        return NULL;

    result->rowmax = result->rows = ma->rows;   /* set retuls rows/cols */
    result->colmax = result->cols = ma->cols;

    /* copy mtrx contents */
    for (i = 0; i < ma->rows; i++)
        for (j = 0; j < ma->cols; j++)
            result->mtrx[i][j] = ma->mtrx[i][j];

    return result;
}

/** free a (m x n) matrix and struct for m->nrows pointers */
void mtrx_free (mtrx_t *m)
{
    register size_t i;

    for (i = 0; i < m->rows; i++)
        free (m->mtrx[i]);
    free (m->mtrx);
    free (m);
}

void arr2d_free (T **m, const size_t n)
{
    register size_t i;

    for (i = 0; i < n; i++)
        free (m[i]);
    free (m);
}

void arr_prn (T * const *a, const size_t m, const size_t n, const int wdth)
{
    register size_t i, j;

    for (i = 0; i < m; i++) {
        for (j = 0; j < n; j++)
            printf (" % *g", wdth, a[i][j]);
        putchar ('\n');
    }
}

void v_prn (const T *v, const size_t n, const int wdth)
{
    register size_t i;

    for (i = 0; i < n; i++)
        printf (" % *g", wdth, v[i]);

    putchar ('\n');
}

/* check all elements of v are zero (within T_FP_MIN/MAX) */
int v_is_zero_fp (const T *v, const size_t n)
{
    int is_zero = 1;
    register size_t i;

    for (i = 0; i < n; i++)
        if (v[i] < T_FP_MIN || T_FP_MAX < v[i]) {
            is_zero = 0;
            break;
        }

    return is_zero;
}

/** helper funcitons to incorporate in wrappers handling mtrx_t */
/* copy two (m x n) matricies */
T **m_copy (T * const *m_a, const size_t m, const size_t n)
{
    T **result = mtrx_calloc (m, n);
    register size_t i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            result [i][j] = m_a [i][j];

    return result;
}

/* add two (m x n) matricies together, return new result */
T **m_add (T * const *m_a, T * const *m_b, const size_t m, const size_t n)
{
    T **result = mtrx_calloc (m, n);
    register size_t i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            result [i][j] = m_a [i][j] + m_b [i][j];

    return result;
}

/* subtract two (m x n) matricies together, return new result */
T **m_sub (T * const *m_a, T * const *m_b, const size_t m, const size_t n)
{
    T **result = mtrx_calloc (m, n);
    register size_t i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            result [i][j] = m_a [i][j] - m_b [i][j];

    return result;
}

/* if _a is (m x n) and _b is (n x p), the product is an (m x p) matrix */
T **m_mult (T * const *m_a, T * const *m_b,
            const size_t m, const size_t n, const size_t p)
{
    T **result = mtrx_calloc (m, p);
    register size_t i, j, k;

    for (i = 0; i < m; i++)
        for (j = 0; j < p; j++)
            for (k = 0; k < n; k++)
                result [i][j] += m_a [i][k] * m_b [k][j];

    return result;
}

/* transpose matrix, dynamically allocate result */
T **m_trans (T * const *m_a, size_t m, size_t n)
{
    T **result = mtrx_calloc (n, m);
    register size_t i, j;

    for (i = 0; i < m; i++)
        for (j = 0; j < n; j++)
            result [j][i] = m_a [i][j];

    return result;
}

/*  Transpose square matrix in place  */
void m_trans_sq (T **m_a, size_t n)
{
    register size_t i, j;
    T tmp;

    for (i = 1; i < n; i++) {
        for (j = 0; j < i; j++) {
            tmp = m_a [i][j];
            m_a[i][j] = m_a[j][i];
            m_a[j][i] = tmp;
        }
    }
}

/* cofactor matrix for (3 x 3) from matrix of minors */
T **m_cofx (T * const *m, size_t n)
{
    T **cofx = mtrx_calloc (n, n);
    register size_t i, j;

    /* fill cofactor from matrix of minors */
    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++) {
            int cof_1 = (i + 1) % n;
            int cof_2 = (j + 1) % n;
            int cof_3 = (i + 2) % n;
            int cof_4 = (j + 2) % n;

            cofx[i][j]= (m[cof_1][cof_2] * m[cof_3][cof_4]) -
                        (m[cof_1][cof_4] * m[cof_3][cof_2]);
        }

    return cofx;
}

/* determinate  for (3 x 3) from matrix and cofactor matrix */
T m_det (T * const *m, T * const *cofx, const size_t n)
{
    T det = 0;
    register size_t i;

    for (i = 0; i < n; i++)
        det += m[i][0] * cofx[i][0];

    return det;
}

/* consistency check vector for (3 x 3) from adjugate and determinate */
T *m_chk (T * const *adj, const T *v, const size_t n)
{
    T *chk = calloc (n, sizeof *chk);
    register size_t i, j;

    if (!chk) { /* allocate/validate consistency check vector */
        fprintf (stderr, "%s() error: virtual memory exhausted.\n",
            __func__);
        return NULL;
    }

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
           chk[i] += adj[i][j] * v[j];

    return chk;
}

/* inverse matrix from determinate */
T **m_inv (T * const *m, const T det, const size_t n)
{
    T **inv = mtrx_calloc (n, n);
    register size_t i, j;

    if (!inv)
        return NULL;

    for (i = 0; i < n; i++)
        for (j = 0; j < n; j++)
           inv[i][j] = m[i][j] / det;

    return inv;
}

/** add two struct matricies */
mtrx_t *mtrx_add (const mtrx_t *ma, const mtrx_t *mb)
{
    mtrx_t *result = NULL;

    if (ma->rows != mb->rows || ma->cols != mb->cols) {
        fprintf (stderr, "%s() error: unequal dimensions, (%zu x %zu) != "
                "(%zu x %zu)\n", __func__, ma->rows, ma->cols,
                mb->rows, mb->cols);
        return NULL;
    }

    /* alloccate/validate sturct */
    if (!(result = calloc (1, sizeof *result))) {
        fprintf (stderr, "%s() error: calloc-result.\n", __func__);
        return NULL;
    }

    result->rowmax = result->rows = ma->rows;   /* set retuls rows/cols */
    result->colmax = result->cols = ma->cols;

    /* add ma->mtrx and mb->mtrx assigning newly allocated result */
    result->mtrx = m_add (ma->mtrx, mb->mtrx, ma->rows, ma->cols);

    return result;
}

/** subtract two struct matricies */
mtrx_t *mtrx_sub (const mtrx_t *ma, const mtrx_t *mb)
{
    mtrx_t *result = NULL;

    if (ma->rows != mb->rows || ma->cols != mb->cols) {
        fprintf (stderr, "%s() error: unequal dimensions, (%zu x %zu) != "
                "(%zu x %zu)\n", __func__, ma->rows, ma->cols,
                mb->rows, mb->cols);
        return NULL;
    }

    /* alloccate/validate sturct */
    if (!(result = calloc (1, sizeof *result))) {
        fprintf (stderr, "%s() error: calloc-result.\n", __func__);
        return NULL;
    }

    result->rowmax = result->rows = ma->rows;   /* set retuls rows/cols */
    result->colmax = result->cols = ma->cols;

    /* add ma->mtrx and mb->mtrx assigning newly allocated result */
    result->mtrx = m_sub (ma->mtrx, mb->mtrx, ma->rows, ma->cols);

    return result;
}

/** multiply two struct matricies */
mtrx_t *mtrx_mult (const mtrx_t *ma, const mtrx_t *mb)
{
    mtrx_t *result = NULL;

    if (ma->cols != mb->rows) {
        fprintf (stderr, "%s() error: unequal dimensions, (%zu x %zu) != "
                "(%zu x %zu)\n [not (m x n) * (n x p) => (m x p)]\n",
                 __func__, ma->rows, ma->cols, mb->rows, mb->cols);
        return NULL;
    }

    /* alloccate/validate sturct */
    if (!(result = calloc (1, sizeof *result))) {
        fprintf (stderr, "%s() error: calloc-result.\n", __func__);
        return NULL;
    }

    result->rowmax = result->rows = mb->rows;   /* set retuls rows/cols */
    result->colmax = result->cols = mb->cols;

    /* add ma->mtrx and mb->mtrx assigning newly allocated result */
    result->mtrx = m_mult (ma->mtrx, mb->mtrx, ma->rows, ma->cols, mb->cols);

    return result;
}

/** transpose matrix */
mtrx_t *mtrx_trans (const mtrx_t *m)
{
    mtrx_t *result = NULL;

    /* alloccate/validate sturct */
    if (!(result = calloc (1, sizeof *result))) {
        fprintf (stderr, "%s() error: calloc-result.\n", __func__);
        return NULL;
    }

    result->rowmax = result->rows = m->cols;    /* set retuls rows/cols */
    result->colmax = result->cols = m->rows;

    /* add ma->mtrx and mb->mtrx assigning newly allocated result */
    result->mtrx = m_trans (m->mtrx, m->rows, m->cols);

    return result;
}

/** solve system of equations - FIXME cofactor for N not 3x3
 *  (just use src-c/tmp/gauss_jordan_chk_10x10_mod.c) and change
 *  to accept mtrx_t *m + solution vect.
 */
T *mtrx_solv (const mtrx_t *m, const T *v)
{
    T **cofx = NULL,        /* cofactor matrix (adjugate as transposed) */
        **inv = NULL;       /* inverse of adjugate matrix */
    T *chk = NULL,          /* consistency check vector */
        *sol = NULL;        /* solution vector */
    T det = 0;              /* determinate */
    register size_t i, j;   /* loop vars */

    if (m->rows != m->cols) {   /* validate m is a square matrix */
        fprintf (stderr, "%s() error: maxtix not square (%zu x %zu).\n",
                __func__, m->rows, m->cols);
        return NULL;
    }

    /* allocate/validate solution vector */
    if (!(sol = calloc (m->rows, sizeof *sol))) {
        fprintf (stderr, "%s() error: memory exhausted 'sol'.\n", __func__);
        return NULL;
    }

    /* form cofactor matrix */
    if (!(cofx = m_cofx (m->mtrx, m->rows)))
        return NULL;

    /* find determinate */
    det = m_det (m->mtrx, cofx, m->rows);
#ifdef DEBUG
        puts ("\ncoefficient matrix:\n");
        arr_prn (m->mtrx, m->rows, m->rows, 5);

        puts ("\ncofactor matrix:\n");
        arr_prn (cofx, m->rows, m->rows, 5);

        printf ("\ndeterminant: %.2f\n", det);
#endif
    /* form adjugate with in-place transpose of cofactor */
    m_trans_sq (cofx, m->rows);
#ifdef DEBUG
        puts ("\nadjugate matrix:\n");
        arr_prn (cofx, m->rows, m->rows, 5);
#endif
    /* form consistency check vector */
    if (!(chk = m_chk (cofx, v, m->rows))) {
        arr2d_free (cofx, m->rows);
        return NULL;
    }
#ifdef DEBUG
        puts ("\nconsistency check vector:\n");
        v_prn (chk, m->rows, 5);
        putchar ('\n');
#endif
    if (det == 0) { /* eliminate infinite and no solution cases */
        if (v_is_zero_fp (chk, m->rows))    /* consistency vect zero */
            printf ("The system is consistent - infinite solutions.\n");
        else
            printf ("The system has no solution.\n");

        arr2d_free (cofx, m->rows);
        free (chk);

        return NULL;
    }

    /* form inverse of adjugate matrix */
    if (!(inv = m_inv (cofx, det, m->rows))) {
        arr2d_free (cofx, m->rows);
        free (chk);
        return NULL;
    }

#ifdef DEBUG
        puts ("\ninverse of adjugate;\n");
        arr_prn (inv, m->rows, m->rows, 5);
#endif
    /* compute solution vector by inverse */
    for (i = 0; i < m->rows; i++)
        for (j = 0; j < m->rows; j++)
            sol[i] += inv[i][j] * v[j];

    arr2d_free (inv, m->rows);
    arr2d_free (cofx, m->rows);
    free (chk);

    /* check if solution vector is zero - trivial solution */
    if (v_is_zero_fp (sol, m->rows)) {
        printf ("The system has a trivial solution.\n");
        free (sol);
        return NULL;
    }

    return sol; /* return unique solution vector */
}

/** mtrx_solv_cmb - solves system of eq where m contains solution vect.
 *  solves system where m contains the solution vector as the last column
 *  in the form {m} = {m'}[v] where {m'} is the coefficient matrix and [v]
 *  the solution vector. it is a wrapper for mtrx_solv() above. returns
 *  a vector containing the unique solution or NULL if {m'} is singular
 *  or the solutions are infinite or trivial.
 */
T *mtrx_solv_cmb (mtrx_t *m)
{
    mtrx_t *mc = NULL;  /* matrix of coefficients */
    T *v = NULL,        /* vector of constants */
        *sol = NULL;    /* unique solution vector */
    register size_t i;

    /* create matrix struct for mc */
    if (!(mc = calloc (1, sizeof *mc)))
        return NULL;

    /* create/allocaate/copy coefficient maxtrix */
    if (!(mc->mtrx = m_copy (m->mtrx, m->rows, m->cols - 1)))
        return NULL;

    mc->rows = m->rows;     /* set rows/cols */
    mc->cols = m->cols - 1;

    if (!(v = calloc (m->rows, sizeof *v))) {
        fprintf (stderr, "%s() error: calloc-v\n", __func__);
        return NULL;
    }

    for (i = 0; i < m->rows; i++)
        v[i] = m->mtrx[i][m->cols-1];

    sol = mtrx_solv (mc, v);

    mtrx_free (mc);
    free (v);

    return sol;
}

static void SWAP (T *a, T *b)
{
    T tmp = *a;
    *a = *b;
    *b = tmp;
}

/** Guass-Jordan elimination with full pivoting.
 *  'a' is coefficient matrix with constant vector as last col.
 *  on return 'a' contains matrix inverse, last col contains
 *  solution vector.
 */
void mtrx_solv_gaussj (T **a, const size_t n)
{   /* bookkeeping arrays for pivot */
    int *indxc = calloc (n, sizeof *indxc),
        *indxr = calloc (n, sizeof *indxr),
        *ipiv  = calloc (n, sizeof *ipiv);
    T big, dum, pivinv;
    size_t i, icol = 0, irow = 0, j, k, l, ll;

    for (j = 0; j < n; j++) ipiv[j] = 0;
    for (i = 0; i < n; i++) {
        big = 0.0;
        for (j = 0; j < n; j++)
            if (ipiv[j] != 1)
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs (a[j][k]) >= big) {
                            big = fabs (a[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                }
        ipiv[icol]++;

        if (irow != icol)   /* transpose row/col */
            for (l = 0; l < n + 1; l++) SWAP(&a[irow][l], &a[icol][l]);

        indxr[i] = irow;
        indxc[i] = icol;

        if (a[icol][icol] == 0.0) {
            /* nrerror ("gaussj: Singular Matrix"); */
            fprintf (stderr, "guassj() error: singular matrix.\n");
            goto gaussjdone;
        }

        pivinv = 1.0 / a[icol][icol];
        a[icol][icol] = 1.0;

        for (l = 0; l < n + 1; l++) a[icol][l] *= pivinv;

        for (ll = 0; ll < n; ll++)
            if (ll != icol) {
                dum = a[ll][icol];
                a[ll][icol] = 0.0;
                for (l = 0; l < n + 1; l++) a[ll][l] -= a[icol][l] * dum;
            }
    }

    l = n;
    while (l--) {
        if (indxr[l] != indxc[l])
            for (k = 0; k < n; k++)
                SWAP(&a[k][indxr[l]],&a[k][indxc[l]]);
    }
    gaussjdone:;

    free (ipiv);
    free (indxr);
    free (indxc);
}

/** same taking a (n x n) and v. TODO preserve T * const * a by making a copy
 *  allocating for the inverse and returing the inverse while letting the
 *  solution vector be available to the caller through the updated v.
 */
void mtrx_solv_gaussj_v (T **a, T *v, const size_t n)
{   /* bookkeeping arrays for pivot */
    int *indxc = calloc (n, sizeof *indxc),
        *indxr = calloc (n, sizeof *indxr),
        *ipiv  = calloc (n, sizeof *ipiv);
    T big, dum, pivinv;
    size_t i, icol = 0, irow = 0, j, k, l, ll;

    for (j = 0; j < n; j++) ipiv[j] = 0;
    for (i = 0; i < n; i++) {
        big = 0.0;
        for (j = 0; j < n; j++)
            if (ipiv[j] != 1)
                for (k = 0; k < n; k++) {
                    if (ipiv[k] == 0) {
                        if (fabs (a[j][k]) >= big) {
                            big = fabs (a[j][k]);
                            irow = j;
                            icol = k;
                        }
                    }
                }
        ipiv[icol]++;

        if (irow != icol) {  /* transpose row/col */
            for (l = 0; l < n; l++) SWAP(&a[irow][l], &a[icol][l]);
            SWAP (&v[irow], &v[icol]);
        }
        indxr[i] = irow;
        indxc[i] = icol;

        if (a[icol][icol] == 0.0) {
            /* nrerror ("gaussj: Singular Matrix"); */
            fprintf (stderr, "guassj() error: singular matrix.\n");
            goto gaussjdone;
        }

        pivinv = 1.0 / a[icol][icol];
        a[icol][icol] = 1.0;

        for (l = 0; l < n; l++) a[icol][l] *= pivinv;
        v[icol] *= pivinv;

        for (ll = 0; ll < n; ll++)
            if (ll != icol) {
                dum = a[ll][icol];
                a[ll][icol] = 0.0;
                for (l = 0; l < n; l++) a[ll][l] -= a[icol][l] * dum;
                v[ll] -= v[icol] * dum;
            }
    }

    l = n;
    while (l--) {
        if (indxr[l] != indxc[l])
            for (k = 0; k < n; k++)
                SWAP(&a[k][indxr[l]],&a[k][indxc[l]]);
    }
    gaussjdone:;

    free (ipiv);
    free (indxr);
    free (indxc);
}

/** mtrx_solv_gaussj_inv returns mtrx_t containing inverse + solution.
 *  wrapper preserving original mtrx_t, allocating a copy before calling
 *  mtrx_solv_gaussj to create inverse of m and solution vector in newly
 *  allocted mtrx_t. user is responsible for freeing return.
 */
mtrx_t *mtrx_solv_gaussj_inv (const mtrx_t *m)
{
    mtrx_t *invsol = NULL;

    if (m->cols <= m->rows) {
        fprintf (stderr, "%s() error: invalid mtrx_t size (cols <= rows)\n",
                __func__);
        return NULL;
    }

    if (!(invsol = mtrx_copy (m)))
        return NULL;

    mtrx_solv_gaussj (invsol->mtrx, invsol->rows);

    return invsol;
}

/** mtrx_get_sol_v return solution vector from inverse+solution mtrx_t.
 *  m must be a square matrix + constant vector as final column. returns
 *  allocated solution vector on success, NULL otherwise.
 */
T *mtrx_get_sol_v (const mtrx_t *m)
{
    T *sol = NULL;
    register size_t i;

    if (m->cols <= m->rows) {
        fprintf (stderr, "%s() error: invalid mtrx_t size (cols <= rows)\n",
                __func__);
        return NULL;
    }

    if (!(sol = calloc (m->rows, sizeof *sol))) {
        fprintf (stderr, "%s() error: memory exhausted calloc-sol.\n",
                __func__);
        return NULL;
    }

    for (i = 0; i < m->rows; i++)
        sol[i] = m->mtrx[i][m->rows];

    return sol;
}

/** qsort compare ascending for matrix (1st value only)
 *  must write a wrapper to sort by rows by col values,
 *  and then sort by 1st value.
 */
int mtrx_compare_rows_asc (const void *a, const void *b)
{
    /* (a > b) - (a < b) */
    return (**(T * const *)a > **(T * const *)b) -
            (**(T * const *)a < **(T * const *)b);
}


//                 void *tmp = realloc (m->mtrx[m->rows],
//                                         2 * m->colmax * sizeof **m->mtrx);
//                 if (!tmp) { /* validate */
//                     perror ("realloc-m->mtrx[m->rows]");
//                     exit (EXIT_FAILURE);
//                 }
//                 m->mtrx[m->rows] = tmp;     /* assigne new block of memory */
//                 memset (m->mtrx[m->rows] + m->colmax, 0, /* zero new memory */
//                         m->colmax * sizeof *m->mtrx[m->rows]);
//                 m->colmax *= 2;             /* increment colmax */

//                 void *tmp = realloc (m->mtrx[m->rows],
//                                         m->cols * sizeof **m->mtrx);
//                 m->colmax = m->cols;    /* update colmax */
//                 if (!tmp) { /* validate allocation */
//                     perror ("realloc-m->mtrx[m->rows]_to_fit");
//                     exit (EXIT_FAILURE);
//                 }
//                 m->mtrx[m->rows] = tmp; /* assign new block of memory */

// mtrx_realloc_ptrs (T *m)
//     void *tmp = realloc (m->mtrx, 2 * m->rowmax * sizeof *m->mtrx);
//     if (!tmp) {
//         fprintf (stderr, "%s() error: ptr realloc failed.\n",
//                 __func__);
//         exit (EXIT_FAILURE);
//     }
//     m->mtrx = tmp;      /* assign new block to m->mtrx */
//     memset (m->mtrx + m->rowmax, 0,      /* zero new pointers */
//             m->rowmax * sizeof *m->mtrx);
//     m->rowmax *= 2; /* incriment m->rowmax */
