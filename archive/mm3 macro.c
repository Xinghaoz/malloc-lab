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
#define WSIZE       4       /* Word and header/footer size (bytes) */ 
#define DSIZE       8       /* Double word size (bytes) */
#define CHUNKSIZE  (1<<12)  /* Extend heap by this amount (bytes) */  

#define MAX(x, y) ((x) > (y)? (x) : (y)) 
#define MIN(x, y) ((x) > (y)? (y) : (x)) 

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8
#define HEADER 8

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
 *	7		6		5		4		3		2		1		0
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

/* Given block ptr bp, compute address of its header and footer */
#define HEADER_ADDR(bp)       ((char *)(bp) - WSIZE)                      
#define FOOTER_ADDR(bp)       ((char *)(bp) + THIS_SIZE - DSIZE) 

 /* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_ADDR(bp)  ((char *)(bp) + THIS_SIZE(bp)) 
#define PREV_ADDR(bp)  ((char *)(bp) - PREV_SIZE(bp)) 
  


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
    if ((heap_listp = mem_sbrk(4*WSIZE)) == (void *)-1) 
        return -1;
    PUT(heap_listp, 0);                          /* Alignment padding */
    PUT(heap_listp + (1*WSIZE), PACK(DSIZE, 1)); /* Prologue header */ 
    PUT(heap_listp + (2*WSIZE), PACK(DSIZE, 1)); /* Prologue footer */ 
    PUT(heap_listp + (3*WSIZE), PACK(0, 1));     /* Epilogue header */
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
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
static void *extend_heap(size_t words) 
{
    char *bp;
    size_t size;

    /* Allocate an even number of words to maintain alignment */
    size = (words % 2) ? (words+1) * WSIZE : words * WSIZE; 
    if ((long)(bp = mem_sbrk(size)) == -1)  
        return NULL;                                        

    /* Initialize free block header/footer and the epilogue header */
    PUT(HDRP(bp), PACK(size, 0));         /* Free block header */   
    PUT(FTRP(bp), PACK(size, 0));         /* Free block footer */   
    PUT(HDRP(NEXT_BLKP(bp)), PACK(0, 1)); /* New epilogue header */ 

    /* Coalesce if the previous block was free */
    return coalesce(bp);                                          
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
	if(verbose<0) {
		return;
	}
	if(verbose==1) {
//		print_heap();
	}
	else if(verbose==2) {
		printf("Now, print the tree in detail:\n");
//		print_tree(free_tree_rt);
	}
}

/*
void print_heap()
{
	void * p=heap_listp;
	printf("Now, print the heap in detail:\n");
	printf("heap size : %d\n",(int)mem_heapsize());

	// print the information of the whole heap. 
	while(GET_SIZE(p)!=0){
		printf("address bias:  %d\n",(int)(p-HEAP));                     
		printf("size        :  %d\n",(int)GET_SIZE(p));
		printf("alloated    :  %d\n",(int)GET_ALLOC(p));

		// To see if the coalescing is wrong.
		if(GET_ALLOC(p)==0&&(GET_ALLOC(GET_PREV(p))==0||GET_ALLOC(GET_NEXT(p))==0)){
			printf("coalesce error!!!\n");                          
			pause();
		}
		else 
			printf("coalesce correct\n");

		p=GET_NEXT(p);
	}
}
*/