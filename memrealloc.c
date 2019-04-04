#include "memrealloc.h"

/** realloc 'ptr' of 'nelem' of 'psz' from 'oldsz' to 'newsz' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_fixed (void *ptr, size_t psz, size_t oldsz, size_t newsz)
{   void *memptr = realloc ((char *)ptr, newsz * psz);
    if (!memptr) {
        perror ("realloc(): virtual memory exhausted.");
        exit (EXIT_FAILURE);
    }   /* zero new memory (optional) */
    if (newsz > oldsz)
        memset ((char *)memptr + oldsz * psz, 0, (newsz - oldsz) * psz);
    return memptr;
}

/** realloc 'ptr' of 'nelem' of 'psz' to 'nelem + inc' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_inc (void *ptr, size_t psz, size_t *nelem, size_t inc)
{   void *memptr = realloc ((char *)ptr, (*nelem + inc) * psz);
    if (!memptr) {
        perror ("realloc(): virtual memory exhausted.");
        exit (EXIT_FAILURE);
    }   /* zero new memory (optional) */
    memset ((char *)memptr + *nelem * psz, 0, inc * psz);
    *nelem += inc;
    return memptr;
}

/** realloc 'ptr' of 'nelem' of 'psz' to 'nelem * 2' of 'psz'.
 *  returns pointer to reallocated block of memory with new
 *  memory initialized to 0/NULL. return must be assigned to
 *  original pointer in caller.
 */
void *xrealloc_x2 (void *ptr, size_t psz, size_t *nelem)
{   void *memptr = realloc ((char *)ptr, *nelem * 2 * psz);
    if (!memptr) {
        perror ("realloc(): virtual memory exhausted.");
        exit (EXIT_FAILURE);
        /* return NULL; */
    }   /* zero new memory (optional) */
    memset ((char *)memptr + *nelem * psz, 0, *nelem * psz);
    *nelem *= 2;
    return memptr;
}

