#ifndef _memrealloc_h_
#define _memrealloc_h_  1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/** realloc 'ptr' of 'nelem' of 'psz' from 'oldsz' to 'newsz' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_fixed (void *ptr, size_t psz, size_t oldsz, size_t newsz);

/** realloc 'ptr' of 'nelem' of 'psz' to 'nelem + inc' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_inc (void *ptr, size_t psz, size_t *nelem, size_t inc);

/** realloc 'ptr' of 'nelem' of 'psz' to 'nelem * 2' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_x2 (void *ptr, size_t psz, size_t *nelem);

#endif
