/*
 * mm.c
 * This source code contains the functions act as 
 * malloc, free, realloc and calloc.
 *
 * Andrew ID : xinghaoz
 * Name : Xinghao Zhou
 * Email : xinghaoz@andrew.cmu.edu
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "mm.h"
#include "memlib.h"

/* If you want debugging output, use the following macro.  When you hand
 * in, remove the #define DEBUG line. */
#define DEBUG
#ifdef DEBUG
# define dbg_printf(...) printf(__VA_ARGS__)
#else
# define dbg_printf(...)
#endif

/* do not change the following! */
#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#endif /* def DRIVER */

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define HEADER 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define SIZE_PTR(p)  ((size_t*)(((char*)(p)) - SIZE_T_SIZE))

/* Header structure:
 *	7		6		5		4		3		2		1		0
 *	|-----------THIS_SIZE-----------|  IS_PREV_FREE 
 *												IS_PREV_ALLOC
 *														IS_ALLOC
 * IS_ALLOC:		1 if THIS block is allocated.
 * IS_PREV_ALLOC:	1 if the PREVIOUS block is allocated.
 * IS_PREV_FREE:	1 if the PREVIOUS block is free.
 * THIS_SIZE:		Size of THIS block. 
 */
#define IS_ALLOC		((*(unsigned int *)(p)) & 0x1)
#define IS_PREV_ALLOC	((*(unsigned int *)(p)) & 0x2)
#define IS_PREV_FREE	((*(unsigned int *)(p)) & 0x4)
#define THIS_SIZE		((*(unsigned int *)(p)) & ~(0x7))



/*
 * Initialize: return -1 on error, 0 on success.
 */
int mm_init(void) {
    return 0;
}

/*
 * malloc
 */
void *malloc (size_t size) {
//	int newsize = ALIGN(size + SIZE_T_SIZE);
//	unsigned char *p = mem_sbrk(newsize);
    //dbg_printf("malloc %u => %p\n", size, p);
	size_t total_size = ALIGN_SIZE(size + HEADER);
    if ((long)p < 0)
        return NULL;
    else {
        p += SIZE_T_SIZE;
        *SIZE_PTR(p) = size;
        return p;
	}
}

/*
 * free
 */
void free (void *ptr) {
    if(!ptr) return;
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) {
    return NULL;
}

/*
 * calloc - you may want to look at mm-naive.c
 * This function is not tested by mdriver, but it is
 * needed to run the traces.
 */
void *calloc (size_t nmemb, size_t size) {
    return NULL;
}


/*
 * Return whether the pointer is in the heap.
 * May be useful for debugging.
 */
static int in_heap(const void *p) {
    return p <= mem_heap_hi() && p >= mem_heap_lo();
}

/*
 * Return whether the pointer is aligned.
 * May be useful for debugging.
 */
static int aligned(const void *p) {
    return (size_t)ALIGN(p) == (size_t)p;
}

/*
 * mm_checkheap
 */
void mm_checkheap(int lineno) {
}
