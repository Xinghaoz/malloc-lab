/*
 * mm.c
 * This source code contains the functions act as 
 * malloc, free, realloc and calloc.
 *
 * Andrew ID : xinghaoz
 * Name : Xinghao Zhou
 * Email : xinghaoz@andrew.cmu.edu
 *
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

/* Basic constants and macros */
#define WSIZE       4		/* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define TSIZE		12		/* Triple word size (bytes) */
#define CHUNKSIZE  (1 << 12)  /* Extend heap by this amount (bytes) */  
#define MIN_SIZE	24		/* The minimum block size is 24 bytes */
#define ALIGNMENT 8			/* Double word (8) alignment */
#define HEADER_SIZE 8			/* Header size 8 bytes */

#define MAX(x, y) ((x) > (y) ? (x) : (y)) 



/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val))   

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(p) (((size_t)(p) + (ALIGNMENT-1)) & ~0x7)
#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))
#define SIZE_PTR(p)  ((size_t*)(((char*)(p)) - SIZE_T_SIZE))

/* Header structure:
 *	31..7	6		5		4		3		2		1		0
 *	|-----------THIS_SIZE-----------|  IS_PREV_FREE 
 *												IS_PREV_ALLOC
 *														IS_ALLOC
 * IS_ALLOC:		1 if THIS block is allocated.
 * IS_PREV_ALLOC:	1 if the PREVIOUS block is allocated.
 * IS_PREV_FREE:	1 if the PREVIOUS block is free.
 * THIS_SIZE:		Size of THIS block. 
 */
/* Given the pointer bp, get the above parameters */
//#define IS_ALLOC(bp)		((*(unsigned int *)(bp)) & 0x1)
#define IS_ALLOC(bp)		(GET(HEADER_ADDR(bp)) & 0x1)
#define IS_PREV_ALLOC(bp)	(GET(HEADER_ADDR(bp)) & 0x2)
#define IS_PREV_FREE(bp)	(GET(HEADER_ADDR(bp)) & 0x4)
#define THIS_SIZE(bp)		(GET(HEADER_ADDR(bp)) & ~(0x7))
#define PREV_SIZE(bp)		(GET(((char *)(bp) - DSIZE)) & ~(0x7))
#define NEXT_SIZE(bp)		(THIS_SIZE(NEXT_ADDR(bp)))

/* Given block ptr bp, compute address of its header and footer */
#define HEADER_ADDR(bp)			((char *)(bp) - WSIZE)                      
#define FOOTER_ADDR(bp)			((char *)(bp) + THIS_SIZE(bp) - DSIZE) 
#define LEFT_ADDR(bp)			((char *)(bp))
#define RIGHT_ADDR(bp)			((char *)(bp) + WSIZE)
#define PARENT_ADDR(bp)			((char *)(bp) + DSIZE)
#define BROTHER_ADDR(bp)		((char *)(bp) + TSIZE)

 /* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_ADDR(bp)  ((char *)(bp) + THIS_SIZE(bp)) 
#define PREV_ADDR(bp)  ((char *)(bp) - PREV_SIZE(bp)) 
  
#define MM_NULL ((void *)0x800000000)	// Set the NULL to be heap bottom

/* Global variables */
static char *heap_listp = 0;  /* Pointer to first block */  


/* Function declaration */
static void *extend_heap(size_t words);
static void place(void *bp, size_t asize);
static void *find_fit(size_t asize);
static void *coalesce(void *bp);
void print_heap();
void mm_checkheap(int verbose);


/*
 * Initialize: return -1 on error, 0 on success.
 *	0	  4		8	 12
 * -----------------------
 * 0/0 | 8/1 | 8/1 | 0/1 |
 * -----------------------
 *				¡ü
 *			heap_listp
 */
int mm_init(void) {
    /* Create the initial empty heap */
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) {
        return -1;
	}
    PUT(heap_listp, 0);								/* Alignment padding */
    PUT(heap_listp + (1 * WSIZE), PACK(DSIZE, 1));	/* Prologue header */ 
    PUT(heap_listp + (2 * WSIZE), PACK(DSIZE, 1));	/* Prologue footer */ 
    PUT(heap_listp + (3 * WSIZE), PACK(0, 1));		/* Epilogue header */
    heap_listp += (2*WSIZE);                     

    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
    if (extend_heap(CHUNKSIZE/WSIZE) == NULL) {
        return -1;
	}
    return 0;
}



/*
 * malloc
 */
void *malloc (size_t size) 
{
    size_t asize; /* adjusted block size */
    void *bp;
    /* Ignore spurious requests */
    if( size <= 0 ) {
        return NULL;
	}
    /* Adjust block size to include HNF and alignment requirements. */
    asize = ALIGN_SIZE(size + HEADER_SIZE);
    /* Search the free list for a fit */
    if((bp = find_fit(asize)) == MM_NULL ) {
        extend_heap(asize);
		/* ///////// To be optimized ///////// */
        if((bp = find_fit(asize)) == MM_NULL ) {
            return NULL;
		}
    }
    place(bp, asize);
    return bp;
}

/*
 * free
 */
void free (void *bp) {
    if (bp == 0) {
        return;
	}

    size_t size = THIS_SIZE(bp);
    if (heap_listp == 0){
        mm_init();
    }

    PUT(HEADER_ADDR(bp), PACK(size, 0));
    PUT(FOOTER_ADDR(bp), PACK(size, 0));
    coalesce(bp);
}

/*
 * realloc - you may want to look at mm-naive.c
 */
void *realloc(void *oldptr, size_t size) 
{
    size_t oldsize;
    void *newptr;

    /* If size == 0 then this is just free, and we return NULL. */
    if(size == 0) {
        mm_free(oldptr);
        return 0;
    }

    /* If oldptr is NULL, then this is just malloc. */
    if(oldptr == NULL) {
        return mm_malloc(size);
    }

    newptr = mm_malloc(size);

    /* If realloc() fails the original block is left untouched  */
    if(!newptr) {
        return 0;
    }

    /* Copy the old data. */
    oldsize = THIS_SIZE(oldptr);
    if(size < oldsize) {
		oldsize = size;
	}
    memcpy(newptr, oldptr, oldsize);

    /* Free the old block. */
    mm_free(oldptr);

    return newptr;
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
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) 
{
    char *bp;
	/* round up to even */
    size_t size = (((words + 1) >> 1 ) << 1) * WSIZE;

    /* Allocate an even number of words to maintain alignment */
//	size = (words % 2) ? (words+1) * WSIZE : words * WSIZE;	

    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        

    /* Initialize free block header/footer and the epilogue header */
    PUT(HEADER_ADDR(bp), PACK(size, 0));         /* Free block header */   
    PUT(FOOTER_ADDR(bp), PACK(size, 0));         /* Free block footer */   
    PUT(HEADER_ADDR(NEXT_ADDR(bp)), PACK(0, 1)); /* New epilogue header */ 

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          
}

/*
 * coalesce - Boundary tag coalescing. Return ptr to coalesced block
 */
static void *coalesce(void *bp) 
{
    size_t prev_alloc = IS_ALLOC(PREV_ADDR(bp));
    size_t next_alloc = IS_ALLOC(NEXT_ADDR(bp));
    size_t size = THIS_SIZE(bp);

    if (prev_alloc && next_alloc) {            /* Case 1 */
        return bp;
    }

    else if (prev_alloc && !next_alloc) {      /* Case 2 */
        size += NEXT_SIZE(bp);
        PUT(HEADER_ADDR(bp), PACK(size, 0));
		///////// This footer or next footer? /////////
        PUT(FOOTER_ADDR(bp), PACK(size, 0));
		// PUT(FOOTER_ADDR(NEXT_ADDR(bp)), PACK(size, 0));
    }

    else if (!prev_alloc && next_alloc) {      /* Case 3 */
        size += PREV_SIZE(bp);
        PUT(FOOTER_ADDR(bp), PACK(size, 0));
        PUT(HEADER_ADDR(PREV_ADDR(bp)), PACK(size, 0));
        bp = PREV_ADDR(bp);
    }

    else {                                     /* Case 4 */
        size += NEXT_SIZE(bp) + PREV_SIZE(bp);
        PUT(HEADER_ADDR(PREV_ADDR(bp)), PACK(size, 0));
        PUT(FOOTER_ADDR(NEXT_ADDR(bp)), PACK(size, 0));
        bp = PREV_ADDR(bp);
    }

    return bp;
}


/* 
 * find_fit - Find a fit for a block with asize bytes 
 */
static void *find_fit(size_t asize)
{
    /* First-fit search */
    void *bp;

    for (bp = heap_listp; THIS_SIZE(bp) > 0; bp = NEXT_ADDR(bp)) {
        if (!(IS_ALLOC(bp)) && (asize <= THIS_SIZE(bp))) {
            return bp;
        }
    }
    return NULL; /* No fit */
}

/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp, size_t asize)
{
    size_t csize = THIS_SIZE(bp);   

    if ((csize - asize) >= (2 * DSIZE)) { 
        PUT(HEADER_ADDR(bp), PACK(asize, 1));
        PUT(FOOTER_ADDR(bp), PACK(asize, 1));
        bp = NEXT_ADDR(bp);
        PUT(HEADER_ADDR(bp), PACK(csize - asize, 0));
        PUT(FOOTER_ADDR(bp), PACK(csize - asize, 0));
    }
    else { 
        PUT(HEADER_ADDR(bp), PACK(csize, 1));
        PUT(FOOTER_ADDR(bp), PACK(csize, 1));
    }
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
	lineno = lineno;
}
