/*
Andrew ID : xinghaoz
Name : Xinghao Zhou
Email : xinghaoz@andrew.cmu.edu
    
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


/* Basic constants and macros.
 * Some are come form textbook.c.
 */
#define WSIZE			4       /* Word and header/footer size (bytes) */ 
#define DSIZE			8       /* Double word size (bytes) */
#define HEADSIZE		4		/* Header size */
#define HNF				8		/* Header And Footer size */
#define ALIGNMENT		8		/* Alignment size */
#define S_BLOCK_SIZE	16
#define M_BLOCK_SIZE	8
#define BLKSIZE 24
#define CHUNKSIZE (1 << 8)


/* Find the Max/Min of the given two numbers */
#define MAX(x, y) ((x)>(y)? (x) : (y))
#define MIN(x, y) ((x)>(y)? (y) : (x))

/*Make the block to meet with the standard alignment requirements*/
#define ALIGN_SIZE(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val)) 

/*Read the size and allocated fields from address p*/
#define SIZE(p) (GET(p) & (~0x7))
#define ALLOC(p) (GET(p) & (0x1))
#define PREVALLOC(p) (GET(p) & (0x2))
#define PREV_E_FREE(p) (GET(p) & (0x4))

/*Given block pointer bp, read the size and allocated fields*/
#define GET_SIZE(bp) ((GET(HEAD(bp)))&~0x7)
#define GET_ALLOC(bp) ((GET(HEAD(bp)))&0x1)
#define GET_PREVALLOC(bp) ((GET(HEAD(bp)))&0x2)
#define GET_PREV_E_FREE(bp) ((GET(HEAD(bp)))&0x4)

#define SET_PREVALLOC(bp) (GET(HEAD(bp)) |= 0x2)
#define RESET_PREVALLOC(bp) (GET(HEAD(bp)) &= ~0x2)
#define SET_PREV_E_FREE(bp) (GET(HEAD(bp)) |= 0x4)
#define RESET_PREV_E_FREE(bp) (GET(HEAD(bp)) &= ~0x4)
/*Given pointer p at the second word of the data structure, compute addresses
of its HEAD,LEFT,RIGHT,PARENT,BROTHER and FOOT pointer*/
#define HEAD(p) ((void *)(p) - WSIZE)
#define LEFT(p) ((void *)(p))
#define RIGHT(p) ((void *)(p) + WSIZE)
#define PARENT(p) ((void *)(p) + 2 * WSIZE)
#define BROTHER(p) ((void *)(p) + 3 * WSIZE)
#define FOOT(p) ((void *)(p) + SIZE(HEAD(p)) - DSIZE)


/*Get the LEFT,RIGHT,PARENT,BROTHER and FOOT pointer of the block to which bp points*/
#define GET_LEFT(bp) ((long)GET(LEFT(bp))|(0x800000000))
#define GET_RIGHT(bp) ((long)GET(RIGHT(bp))|(0x800000000))
#define GET_PARENT(bp) ((long)GET(PARENT(bp))|(0x800000000))
#define GET_BROTHER(bp) ((long)GET(BROTHER(bp))|(0x800000000))

/*Define value to each character in the block bp points to.*/
#define PUT_LEFT(bp, val) (PUT(LEFT(bp), ((unsigned int)(long)val)))
#define PUT_RIGHT(bp, val) (PUT(RIGHT(bp), ((unsigned int)(long)val)))
#define PUT_PARENT(bp, val) (PUT(PARENT(bp), ((unsigned int)(long)val)))
#define PUT_BROTHER(bp, val) (PUT(BROTHER(bp), ((unsigned int)(long)val)))


////////////////////////////////////////////////////////////////////
#define HEADER_SIZE 4			/* Header size 8 bytes */
#define OVERHEAD    8			/* Overhead = header + footer = 8 bytes */
#define MIN_SIZE	24			/* Minimux size 24 bytes */
#define LESS_THAN_PREVIOUS		0 /* Indicator used in the insertion of tree */
#define LARGER_THAN_PREVIOUS	1 /* Indicator used in the insertion of tree */

#define MM_NULL ((void *)0x800000000)	// Set the NULL to be heap bottom

#define IS_ALLOC(bp)		(GET(HEADER_ADDR(bp)) & 0x1)
#define IS_PREV_ALLOC(bp)	(GET(HEADER_ADDR(bp)) & 0x2)
#define IS_PREV_FREE(bp)	(GET(HEADER_ADDR(bp)) & 0x4)
#define THIS_SIZE(bp)		(GET(HEADER_ADDR(bp)) & ~(0x7))
//#define PREV_SIZE(bp)		(GET(((char *)(bp) - DSIZE)) & ~(0x7))
#define PREV_SIZE(bp)		(THIS_SIZE(PREV_ADDR(bp)))
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
//#define PREV_ADDR(bp)  ((char *)(bp) - PREV_SIZE(bp)) 
#define PREV_ADDR(bp) (IS_PREV_FREE(bp) ? ((char *)bp - DSIZE): ((char *)(bp) - (GET(((char *)(bp) - DSIZE)) & ~(0x7))))
//#define GET_PREV(bp) ( GET_PREV_E_FREE(bp) ? ((void *)bp - M_BLOCK_SIZE ): ((void *)(bp) - SIZE((void *)(bp) - DSIZE)) )

/* Store the address into the nodes */
#define LEFT_PTR(bp, add)		(PUT(LEFT(bp), ((unsigned int)(long)add)))
#define RIGHT_PTR(bp, add)		(PUT(RIGHT(bp), ((unsigned int)(long)add)))
#define PARENT_PTR(bp, add)		(PUT(PARENT(bp), ((unsigned int)(long)add)))
#define BROTHER_PTR(bp, add)	(PUT(BROTHER(bp), ((unsigned int)(long)add)))

#define MM_NULL ((void *)0x800000000)	// Set the NULL to be heap bottom
#define HEAP_BUTTOM ((void *)0x800000000) // Heap buttom

////////////////////////////////////////////////////////////////////


//All functions and global variables used in the program:

/* static functions */
static void *coalesce ( void *bp );
static void *extend_heap ( size_t size );
static void place ( void *ptr, size_t asize );
static void insert_node ( void *bp );
static void delete_node ( void *bp );
static void *find_fit ( size_t asize );
static void printblock(void *bp);
static void checkblock(void *bp);
static void checktree(void * temp);
static void checkminilist(void * bp); 
static void checksmalllist(void * bp);
static void *bst_search(void * root, size_t size);
void print_heap();

/* Global variables */
static void *heap_list_ptr;//head block node of all heap blocks
static void *root;//root of the BST
static void *small_list_ptr;//head of the 16-bit list
static void *mini_list_ptr;//head of the 8-byte list

/*
MM_INIT
similar with the code in textbook
*/

int mm_init(void)
{
    /* create the initial empty heap */
    if ((heap_list_ptr = mem_sbrk(6 * WSIZE)) == (void *)-1 )
        return -1;
    PUT(heap_list_ptr + (2 * WSIZE), 0 ); // alignment padding
    PUT(heap_list_ptr + (3 * WSIZE), PACK(DSIZE, 1) ); // prologue header
    PUT(heap_list_ptr + (4 * WSIZE), PACK(DSIZE, 1) ); // prologue footer
    PUT(heap_list_ptr + (5 * WSIZE), PACK(0, 3) ); // epilogue header
    heap_list_ptr += (4 * WSIZE);
    /*init the global variables*/
    root = MM_NULL;
    small_list_ptr = MM_NULL;
    mini_list_ptr = MM_NULL;
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
//	extend_heap(CHUNKSIZE);
	if (extend_heap(OVERHEAD) == NULL) {
		return -1;
	}
	return 0;
}
/*EXTEND_HEAP

extend_heap extends the heap with a new free block. It is invoked when the
heap is initialized or malloc or realloc is unable to find a suitable fit.

before we extend the heap, check if the last block is free, if yes, extend the
heap for size - GET_SIZE(prev) bits, At the end of the function, we call the coalesce
function to merge the two free blocks and return the block pointer to the
merged blocks.*/

void *extend_heap(size_t input_size)
{
    void *bp;
	int size = input_size;

	/* Optimization */
	char *epilogue_block = (char *)mem_heap_hi() + 1;

	/* If last block is free, 
	 * we can extend less block in order to increase the utilization.
	 */
//	if (!IS_PREV_ALLOC(epilogue_block) && (unsigned int)size < THIS_SIZE(PREV_ADDR(epilogue_block))) {   
	if (!IS_PREV_ALLOC(epilogue_block)) {     
		size -= THIS_SIZE(PREV_ADDR(epilogue_block));		
	}
	if(size <= 0) {
		return NULL;
	}
    
	/* CHUKSIZE is a tricky part */
    size = MAX(CHUNKSIZE, size);

    if((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
	}
    /* Initialize free block header/footer and the epilogue header */
    PUT(HEADER_ADDR(bp), PACK(size, IS_PREV_FREE(bp) | IS_PREV_ALLOC(bp) | 0));
    PUT(FOOTER_ADDR(bp), PACK(size, IS_PREV_FREE(bp) | IS_PREV_ALLOC(bp) | 0));
	/* Set new epilogue header */
    PUT(HEADER_ADDR(NEXT_ADDR(bp)), PACK(0, 1));
    
	/* We should insert the new extend block as a new node */
    insert_node(coalesce(bp));
    return bp;         
}

/*
MALLOC

After initializing, we use malloc to allocate a block by incrementing the brk
pointer. We always allocate a block whose size is a multiple of the alignment.
The function below is very likely to the function described on the textbook.
The behaviors of the function are:

1. Checking the spurious requests.
2. Adjusting block size to include alignment requirements.
3. Searching the BST for a fit.
4. Placing the block into its fit.

At the end of the function, I looked in the traces and found the best strategy to
meet the performance evaluation principle of the project, so I added the if
sentence after it.*/

void *malloc(size_t size)
{
    size_t asize; /* adjusted block size */
    void *bp;
    /* Ignore spurious requests */
    if( size <= 0 ) {
        return NULL;
	}
    /* We need to count the header and footer as overhead and align it */
    asize = ALIGN_SIZE(size + HEADER_SIZE);
    /* Search the free list for a fit */
    if((bp = find_fit(asize)) == MM_NULL) {
        extend_heap(asize);
		/* ///////// To be optimized ///////// */
        if((bp = find_fit(asize)) == MM_NULL) {
            return NULL;
		}
    }
	/* Mark the new malloc block as alloated */
    place(bp, asize);
	mm_checkheap(1);
    return bp;
}

/*
 * free
 * The function of free is quite easy.
 * Just set the free sign bit of header and footer to 0.
 * And check if the previous block or next block are free.
 * If so, coalesce these blocks.
 */
void free(void *bp)
{
    if(bp == NULL) {
		return;
	}
    size_t size = THIS_SIZE(bp);

    PUT(HEADER_ADDR(bp), PACK(size, IS_PREV_FREE(bp) | IS_PREV_ALLOC(bp) | 0));
    PUT(FOOTER_ADDR(bp), PACK(size, IS_PREV_FREE(bp) | IS_PREV_ALLOC(bp) | 0));

	if (!IS_ALLOC(bp) && (!IS_PREV_ALLOC(bp) || !IS_ALLOC(NEXT_ADDR(bp)))) {
		bp = coalesce(bp);
	}
    insert_node(bp);
	mm_checkheap(2);
}

/*
REALLOC

The behavior of the function is listed below:
1.When ptr==HEAP_NIL, call malloc(size), when size==0, free ptr.
2.When size>0, compare new size and old size and then adopt relative
strategies.:
    (1) if newsize < oldsize, check if the size of the rest space,
        if > HNF, divide the block into 2 pieces
    (2) else, check if the next space is free, if so, coalesce the block and 
        the next block
*/
void *realloc(void *ptr, size_t size)
{
    if( ptr==NULL ) return malloc(size);
    if( size == 0 ){
        free(ptr);
        return NULL;
    }
    if( size > 0 ){
        size_t oldsize = THIS_SIZE( ptr );
        size_t newsize = ALIGN_SIZE( size + HEADSIZE );
        if( newsize <= oldsize ){ /* newsize is less than oldsize */
            if( GET_ALLOC(NEXT_ADDR(ptr) ) ){
            /* the next block is allocated */
                if( (oldsize-newsize) >= HNF ){
                /* the remainder is greater than BLKSIZE */
                    size_t sign = 1 | GET_PREVALLOC(ptr) | GET_PREV_E_FREE(ptr);
                    PUT(HEADER_ADDR(ptr), PACK(newsize,sign) );
                    //this pointer points to extra space
                    void *temp = NEXT_ADDR(ptr);
                    PUT(HEADER_ADDR(temp), PACK(oldsize-newsize,2) );
                    PUT(FOOTER_ADDR(temp), PACK(oldsize-newsize,2) );
                    insert_node(coalesce(temp));
                }
                return ptr;
            }
            else{ /* the next block is free */
                size_t csize = oldsize + GET_SIZE(NEXT_ADDR(ptr) );
                delete_node(NEXT_ADDR(ptr));
                size_t sign = 1 | GET_PREVALLOC(ptr) | GET_PREV_E_FREE(ptr);
                PUT(HEADER_ADDR(ptr), PACK(newsize,sign) );
                void *temp = NEXT_ADDR(ptr);
                PUT(HEADER_ADDR(temp), PACK(csize-newsize,2) );
                PUT(FOOTER_ADDR(temp), PACK(csize-newsize,2) );
                insert_node( coalesce(temp) );
                return ptr;
            }
        }
        else{ /* newsize is greater than oldsize */
            void * next = NEXT_ADDR(ptr);
            size_t next_alloc = GET_ALLOC(next);
            size_t next_size = THIS_SIZE(next);
               
            if (next_size == 0 || GET(NEXT_ADDR(next) ) == 0) {
                extend_heap( newsize - oldsize );
            }

            size_t csize;
            // the next block is free and the addition of the two blocks no less than the newsize
            if( !next_alloc && ((csize = oldsize + next_size) >= newsize) ){
                delete_node(NEXT_ADDR(ptr));
                if((csize-newsize) >= HNF){
                    size_t sign = 1 | GET_PREVALLOC(ptr) | GET_PREV_E_FREE(ptr);
                    PUT(HEADER_ADDR(ptr), PACK(newsize,sign) );
                    void *temp=NEXT_ADDR(ptr);
                    PUT(HEADER_ADDR(temp), PACK(csize-newsize,2) );
                    PUT(FOOTER_ADDR(temp), PACK(csize-newsize,2) );
                    insert_node( coalesce(temp) );
				}else{
				    size_t sign = 1 | GET_PREVALLOC(ptr) | GET_PREV_E_FREE(ptr);
					PUT(HEADER_ADDR(ptr),PACK(csize,sign) );
                    SET_PREVALLOC(NEXT_ADDR(ptr) );
				}
				return ptr;
			}
			else{
				void *newptr;
				if((newptr=find_fit(newsize))==MM_NULL){        
					extend_heap( newsize );
					if((newptr = find_fit(newsize)) == MM_NULL)
                         return NULL;
				}
				place( newptr, newsize );
				/*copy content from memory*/
				memcpy(newptr, ptr, oldsize - HEADSIZE);
				free(ptr);
				return newptr;
			}
		}
	}
    else return NULL;
}

/*
 *	coalesce - Boundary tag coalescing. Return ptr to coalesced block.
 */
static void *coalesce(void *bp)
{	
	size_t prev_alloc = IS_PREV_ALLOC(bp);
    size_t next_alloc = IS_ALLOC(NEXT_ADDR(bp));
    size_t size = THIS_SIZE(bp);
	
	/* Case 1: Previous block and next block are both free, do nothing */
	if (prev_alloc && next_alloc){		
        return bp;
	}		
	/* Case 2: Previous block is free, and next block is allocated */
	else if (!prev_alloc && next_alloc) { 
		size += PREV_SIZE(bp);
	    char * prev = (char *)PREV_ADDR(bp);
	    delete_node(prev);
		PUT(HEADER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		PUT(FOOTER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		return prev;
	}
	/* Case 3: Previous block is allocated, and next block is free */
	else if (prev_alloc && !next_alloc) {
	    size += NEXT_SIZE(bp);
	    delete_node(NEXT_ADDR(bp));
		PUT(HEADER_ADDR(bp), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		PUT(FOOTER_ADDR(bp), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		return bp;
	}
	/* Case 4: Previous block and next block are both free */
	else {
		size += PREV_SIZE(bp) + NEXT_SIZE(bp);
	    char * prev = (char *)PREV_ADDR(bp);
	    char * next = (char *)NEXT_ADDR(bp);
		
		delete_node(prev);
		delete_node(next);
		
		PUT(HEADER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		PUT(FOOTER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		return prev;
	}
}

/*PLACE

place is to place the requested block.
If the remainder of the block after slitting would be greater than or equal to the
minimum block size, then we go ahead and split the block. We should realize
that we need to place the new allocated block before moving to the next block.
It is very likely to the operation on realloc.*/

static void place(void *bp,size_t input_size)
{
	size_t block_size = THIS_SIZE(bp);
	delete_node(bp);
	if((block_size - input_size) >= OVERHEAD) {
		PUT(HEADER_ADDR(bp), PACK(input_size, IS_PREV_ALLOC(bp) | 0x1));

		bp = NEXT_ADDR(bp);
		PUT(HEADER_ADDR(bp), PACK(block_size - input_size, 0x2));
		PUT(FOOTER_ADDR(bp), PACK(block_size - input_size, 0x2));
		
		// insert_node(coalesce(temp));
		insert_node(bp);
	}
	else{
		PUT(HEADER_ADDR(bp), PACK(block_size, IS_PREV_ALLOC(bp) | 0x1));
	}
}

/*find_fit

find_fit performs a fit search. Our basic principles for BEST FIT strategies is

8-byte list :
    if asize < 8 , check the 8-byte list, if the list is not empty, return the head of the list

16-bit list :
    if asize < 16 , check the 16-byte list, if the list is not empty, return the head of the list
    
BINARY TREE :
    1.We must eventually find a fit block after searching the binary free tree.
    2.We must choose the least size free block compared with the requested size.
      So we should initially move toward left and go on. When the block in the left is
      not big enough to support the block, move right.
    3.If the block is so big that every node cannot fit it (till the rightmost), extend the
       heap and put the block in the rightmost of the tree.
*/

static void *find_fit(size_t size)
{
	/* ///////// To be optimized ///////// */	
    if (size <= 8 && mini_list_ptr != MM_NULL) {
		return mini_list_ptr;
	} else if(size <= 16 && small_list_ptr != MM_NULL) {
		return small_list_ptr;
	}
	/* ///////// To be optimized ///////// */

	/* BST search */
	
	void *fit = MM_NULL;
	void *current_node = root;
	while(current_node != MM_NULL) {
		if (size == THIS_SIZE(current_node)) {
			return current_node;
		} else if (size < THIS_SIZE(current_node)) {
			fit = current_node;
			current_node = (char *)GET_LEFT(current_node);
		}
		else
			current_node = (char *)GET_RIGHT(current_node);
	}
	return fit;
	
//	return bst_search(root, size);
}

static void *bst_search(void * root, size_t size) {
	if (NULL == root) {
		return MM_NULL;
	}

	if (size == THIS_SIZE(root)) {
		return root;
	} else if (size < THIS_SIZE(root)) {
		return bst_search((void *)GET_LEFT(root), size);
	} else {
		return bst_search((void *)GET_RIGHT(root), size);
	}
}

/*
INSERT_NODE
principles:
    if the block size = 8; insert it to the head of the 8-byte list
    if the block size = 16; insert it to the head of the 16-bit list
    if the block size >= 24; insert it to BST
        if the block size is less than the size of the block on the node
           insert it to the node's left child node
        if the block size is greater than the size of the block on the node
           insert it to the node's right child node
        if the block size equals to the size of the block on the node
           insert it to the node's position and set the node as its brother
*/

inline static void insert_node(void *bp)
{
    RESET_PREVALLOC(NEXT_ADDR(bp));
    size_t insert_size = THIS_SIZE(bp);
    if(insert_size == 8){
        SET_PREV_E_FREE(NEXT_ADDR(bp));
        LEFT_PTR(bp, mini_list_ptr);
        mini_list_ptr = bp;
    }    
    if(insert_size == 16) {
        LEFT_PTR(bp, MM_NULL);
        RIGHT_PTR(bp, small_list_ptr);
        LEFT_PTR(small_list_ptr, bp);
        small_list_ptr = bp;
        return;
    }
    if(insert_size < MIN_SIZE) {
		return;
	}
	/* If it is the first node */
	if (root == MM_NULL){
		root = bp;
		LEFT_PTR(bp, MM_NULL);
        RIGHT_PTR(bp, MM_NULL);
		PARENT_PTR(bp, MM_NULL);
		BROTHER_PTR(bp, MM_NULL);
		return;
	}

	/* Instert */
	/*
	void *current_node = root;
	void *fit = MM_NULL;
	while(current_node != MM_NULL){
		if(insert_size <= THIS_SIZE(current_node)){
			fit = current_node;
			current_node = (void *)GET_LEFT(current_node);
		}
		else
			current_node = (void *)GET_RIGHT(current_node);
	}
	return fit;
	*/

	
	void *current_parent = MM_NULL;
	void *current_node = root;
	int sign = -1;

	while(1) {
	    if (MM_NULL == current_node){
            LEFT_PTR(bp, MM_NULL);
            RIGHT_PTR(bp, MM_NULL);
            PARENT_PTR(bp, current_parent);
            BROTHER_PTR(bp, MM_NULL);
            break;
        }
		/* If there is a node in the BST has the size as the insert size,
		 * add the insert node to it brothers.
		 */
		if (insert_size == THIS_SIZE(current_node)){
		    PUT(LEFT_ADDR(bp), GET(LEFT_ADDR(current_node)));
            PUT(RIGHT_ADDR(bp), GET(RIGHT_ADDR(current_node)));
            PARENT_PTR(GET_LEFT(current_node), bp);
            PARENT_PTR(GET_RIGHT(current_node), bp);
		    PARENT_PTR(bp, current_parent);
			BROTHER_PTR(bp, current_node);
            LEFT_PTR(current_node, bp);
		    break;
		}
		else if (insert_size < THIS_SIZE(current_node)) {
			current_parent = current_node;
			current_node = (char *)GET_LEFT(current_node);
			sign = LESS_THAN_PREVIOUS;
		}
		else{
			current_parent = current_node;
			current_node = (char *)GET_RIGHT(current_node);
			sign = LARGER_THAN_PREVIOUS;
		}
	}
	if(sign == -1) {
		root = bp;
	} else if (sign == LESS_THAN_PREVIOUS) {
		PUT_LEFT(current_parent, bp);
	} else if(sign == LARGER_THAN_PREVIOUS) {
		PUT_RIGHT(current_parent, bp);
	}
	
}

/*DELETE_NODE

delete_node is to delete a free block from the free-block binary tree. It also has
many possibilities.

*/

inline static void delete_node(void *bp)
{
    SET_PREVALLOC(NEXT_ADDR(bp));
    size_t bpsize = THIS_SIZE(bp);
    if(bpsize == 8) {//if the block size = 8, remove it from the 8-byte list, O(n)
        RESET_PREV_E_FREE(NEXT_ADDR(bp));
        char * temp = mini_list_ptr;
        if(temp == bp){
            mini_list_ptr = (char *)GET_LEFT(bp);
            return;
        }
        while(temp != MM_NULL){
            if((char *)GET_LEFT(temp) == bp) break;
            temp = (char *)GET_LEFT(temp); 
        } 
        PUT_LEFT(temp, (char *)GET_LEFT(bp));
    }     
    if (bpsize == 16) {//if the block size = 16, remove it from the 16-bit list, O(1)
        char * bpL = (char *)GET_LEFT(bp);
        char * bpR = (char *)GET_RIGHT(bp);
        
        if(bp == small_list_ptr)
            small_list_ptr = bpR;    
        
        PUT_RIGHT( bpL, bpR);
        PUT_LEFT( bpR, bpL);
        return;
    }    
    if(bpsize < BLKSIZE) {
		return;
	}


    /* Case that the block is one of the following ones in the node. */
    if(((char *)GET_LEFT(bp) != MM_NULL) && ((char *)GET_BROTHER(GET_LEFT(bp)) == bp)){
        PUT_BROTHER(GET_LEFT(bp), GET_BROTHER(bp));
        PUT_LEFT(GET_BROTHER(bp), GET_LEFT(bp));
    }
	/* Case that the block is the only one in the node
       we are all familar with the BST delete node operation
       but in the heap, it seems a little complex */
	else if( (char *)GET_BROTHER(bp) == MM_NULL ){
		if( bp == root ){/* the node is the root */
			if( (char *)GET_RIGHT(bp) == MM_NULL ){/* no right child */
				root = (char *)GET_LEFT(bp);
				if( root != MM_NULL )
				    PUT_PARENT( root, MM_NULL );
			}
			else{/* it has a right child */
				char *temp = (char *)GET_RIGHT(bp);
				while( (char *)GET_LEFT(temp) != MM_NULL )
                    temp = (char *)GET_LEFT(temp);
				char *rootL = (char *)GET_LEFT(bp);
				char *rootR = (char *)GET_RIGHT(bp);
				char *tempR = (char *)GET_RIGHT(temp);
				char *tempP = (char *)GET_PARENT(temp);
				root = temp;
				PUT_PARENT( root, MM_NULL);
				PUT_LEFT( root, rootL );
				PUT_PARENT( rootL, root );
				if( root != rootR ){
					PUT_RIGHT( root, rootR );
					PUT_PARENT( rootR, root );
					PUT_LEFT( tempP, tempR );
					PUT_PARENT( tempR, tempP );
				}
			}
		}
		else{/* the node is not the root */
			if( (char *)GET_RIGHT(bp) == MM_NULL){/* no right child */
				if( (char *)GET_LEFT( GET_PARENT( bp ) ) == bp )
					PUT_LEFT( GET_PARENT(bp), GET_LEFT(bp) );
				else
					PUT_RIGHT( GET_PARENT(bp), GET_LEFT(bp) );
                PUT_PARENT( GET_LEFT(bp), GET_PARENT(bp) );
			}else{/* it has a right child */
				char *temp = (char *)GET_RIGHT(bp);
				while( (char *)GET_LEFT(temp) != MM_NULL)
				    temp = (char *)GET_LEFT(temp);
				char *bpL = (char *)GET_LEFT(bp);
				char *bpR = (char *)GET_RIGHT(bp);
				char *bpP = (char *)GET_PARENT(bp);
				char *tempR = (char *)GET_RIGHT(temp);
				char *tempP = (char *)GET_PARENT(temp);
				
				if( (char *)GET_LEFT( bpP ) == bp )
					PUT_LEFT( bpP, temp );
				else
					PUT_RIGHT( bpP, temp );
			    PUT_PARENT( temp, bpP );

				PUT_LEFT( temp, bpL );
				PUT_PARENT( bpL, temp );
				if( temp != bpR ){
					PUT_RIGHT( temp, bpR );
					PUT_PARENT( bpR, temp );
					PUT_LEFT( tempP, tempR );
					PUT_PARENT( tempR, tempP );
				}
			}
		}
	}
	/* Case that the block is first one in the node. */
    else{
	    char *temp = (char *)GET_BROTHER(bp);
		if( bp == root ){/* the node is the root */
			root = temp;
			PUT_PARENT( temp, MM_NULL);
		}else{/* the node is not the root */		
		    if( (char *)GET_LEFT(GET_PARENT(bp)) == bp )
				PUT_LEFT( GET_PARENT(bp), temp );
			else
				PUT_RIGHT( GET_PARENT(bp), temp );
			PUT_PARENT( temp, GET_PARENT(bp) );
		}
		PUT(LEFT(temp), GET(LEFT(bp)));
        PUT(RIGHT(temp), GET(RIGHT(bp)));
		PUT_PARENT( GET_LEFT(bp), temp );
	    PUT_PARENT( GET_RIGHT(bp), temp );
	}
}

/* 
 * mm_checkheap - Check the heap for consistency 
     options:
         verbose = 0: only print the checkblock message
         verbose = 1: print the heap block
         verbose = 2: print the BST nodes in mid-brother-left-right order
         verbose = 3: print the 8-byte list(minilist) and the 16-bit list(smalllist)
 */
 /*
void mm_checkheap(int verbose) 
{
    char *bp = heap_list_ptr;

    if (verbose == 1)
        printf("Heap (%p):\n", heap_list_ptr);

    if ((SIZE(heap_list_ptr) != DSIZE) || !ALLOC(heap_list_ptr))
        printf("Bad prologue header\n");
        
    if (verbose == 1)
        printblock(heap_list_ptr);
    for (bp = GET_NEXT(bp); GET_SIZE(bp) > 0; bp = GET_NEXT(bp)) {
        if (verbose == 1) printblock(bp);
        checkblock(bp);
    }

    if (verbose == 1) printblock(bp);
    if ((GET_SIZE(bp) != 0) || !(GET_ALLOC(bp)))
        printf("Bad epilogue header\n");
    if (verbose == 2){
        printf("=============tree=============\n");
        checktree(root);
    }
    if (verbose == 3){
        printf("=========mini list(size = 8)==========\n");
        checkminilist(mini_list_ptr);
        printf("=========short list(size = 16)==========\n");
        checksmalllist(small_list_ptr);
    }    
}
*/
/*print the information of a block, including the address, size and allocated bit*/
static void printblock(void *bp) 
{
    size_t hsize;

    hsize = SIZE(HEAD(bp)); 

    if (hsize == 0) {
        printf("%p: EOL\n", bp);
        return;
    }

    printf("%p--prev_e_free bit[%d] prevalloc bit[%d] allocated bit[%d] size[%d]\n",
     bp, !!(int)GET_PREV_E_FREE(bp), !!(int)GET_PREVALLOC(bp), (int)GET_ALLOC(bp), (int)hsize); 
}

/*check if the block is doubleword aligned*/
static void checkblock(void *bp) 
{
    if ((size_t)bp % 8)
    printf("Error: %p is not doubleword aligned\n", bp);
}

/*check a free block in the BST
    print the block and check the next node in the list including the brother node
    Then check the left child node and the right child node
*/
static void checktree(void * temp){
    if(temp != MM_NULL){
        printblock(temp);
        printf("\tparent:%p;\tbrother:%p;\n\tleft:%p;\tright:%p\n",
         (void *)GET_PARENT(temp), (void *)GET_BROTHER(temp), (void *)GET_LEFT(temp), (void *)GET_RIGHT(temp));
        void * temp1 = (void *)GET_BROTHER(temp);
        while(temp1 != MM_NULL){
            printblock(temp1);
            printf("\tparent:%p;\tbrother:%p;\n\tleft:%p;\tright:%p\n",
             (void *)GET_PARENT(temp1), (void *)GET_BROTHER(temp1), (void *)GET_LEFT(temp1), (void *)GET_RIGHT(temp1));
            temp1 = (void *)GET_BROTHER(temp1);
        }
        checktree((void *)GET_LEFT(temp));
        checktree((void *)GET_RIGHT(temp));
    }
}     

/*check a 16-bit free block in the small list
    print the block and check the next node in the list
*/
static void checksmalllist(void * bp){
    if(bp != MM_NULL){
        printblock(bp);
        printf("\tleft:%p;\tright:%p\n", (void *)GET_LEFT(bp), (void *)GET_RIGHT(bp));
        checksmalllist((void *)GET_RIGHT(bp));
    }
}

/*check a 16-bit free block in the mini list
    print the block and check the next node in the list
*/
static void checkminilist(void * bp){
    if(bp != MM_NULL){
        printblock(bp);
        printf("\tleft:%p\n", (void *)GET_LEFT(bp));
        checkminilist((void *)GET_LEFT(bp));
    }
}

/*
*int mm_traverse(void * root) {
	int result[100];

	if (NULL == root) {
		return result;
	}

	int left[]  = mm_traverse(
}
*/

void mm_checkheap(int verbose) {
	
	if (verbose<0) {
		return;
	}
	if (verbose==1) {
//		print_heap(1);
	}
	else if(verbose==2) {
//		print_heap(2);
	}
}


void print_heap(int version)
{
	void * bp = heap_list_ptr;
	// print the information of the whole heap. 
	if (1 == version) {
		printf("----------malloc-----------\n");
		while(1){		
			printf("11");
			int offset = (int)(bp - HEAP_BUTTOM);
			int this_size = THIS_SIZE(bp);
			printf("address bias:  %d / %d%d%d\n",offset, (int)IS_PREV_FREE(bp), (int)IS_PREV_ALLOC(bp), (int)IS_ALLOC(bp));                     
			printf("size        :  %d\n",(int)THIS_SIZE(bp));
			printf("next	    :  %d / %d\n",this_size + offset, (int)mem_heapsize());
//			printf(" /// \n");		
//			int aa = IS_ALLOC(bp);
//			int bb = IS_ALLOC(PREV_ADDR(bp));
//			int cc = IS_ALLOC(NEXT_ADDR(bp));
//			printf("this: %d\n", aa);
//			printf("prev: %d\n", bb);
//			printf("next: %d\n", cc);
			/*
			if(IS_ALLOC(bp) == 0 && (IS_ALLOC(PREV_ADDR(bp)) == 0 || IS_ALLOC(NEXT_ADDR(bp)) == 0)) {
				if ((long)(bp) != (long)(PREV_ADDR(bp))) {
					printf("coalesce error!!!\n"); 
					int a = IS_ALLOC(bp);
					int b = IS_ALLOC(PREV_ADDR(bp));
					int c = IS_ALLOC(NEXT_ADDR(bp));
					long d = (long)bp;
					long e = (long)(PREV_ADDR(bp));
					long f = (long)(NEXT_ADDR(bp));
					printf("this: %d  prev: %d  next: %d\n", a, b, c);
					printf("this address: %lx  prev address: %lx  next address: %lx", d, e, f);
					pause();
				}
			}
			else {
		//		printf("coalesce correct\n");
			}
			*/			
//			printf("~~~\n" );
			long d = (long)bp;
			long e = (long)(PREV_ADDR(bp));
			long f = (long)(NEXT_ADDR(bp));
			long g = (long)mem_heap_hi();
			printf("this address: %lx  prev address: %lx  next address: %lx\n", d, e, f);
//			printf("this size = %d\n", this_size);
//			printf("this size + offset = %d\n", this_size + offset);
//			printf("heap_high = %lx\n", g);
			if (f < g) {
				bp = NEXT_ADDR(bp);

			//#define NEXT_ADDR(bp)	((char *)(bp) + THIS_SIZE(bp)) 

			} else {
				printf("break\n");
				break;
			}
		}
		printf("-------------------------\n\n");
	} else if (2 == version) {
		printf("-----------free----------\n");
		while(THIS_SIZE(bp) != 0){
			
			printf("address bias:  %d / %d\n",(int)(bp - HEAP_BUTTOM), (int)IS_ALLOC(bp));                     
			printf("size        :  %d\n",(int)THIS_SIZE(bp));
			printf("next        :  %d\n",(int)THIS_SIZE(bp) + (int)(bp - HEAP_BUTTOM));
			printf(" /// \n");
			if(IS_ALLOC(bp) == 0 && (IS_ALLOC(PREV_ADDR(bp)) == 0 || IS_ALLOC(NEXT_ADDR(bp)) == 0)) {
				if ((long)(bp) != (long)(PREV_ADDR(bp))) {
					printf("coalesce error!!!\n"); 
					int a = IS_ALLOC(bp);
					int b = IS_ALLOC(PREV_ADDR(bp));
					int c = IS_ALLOC(NEXT_ADDR(bp));
					long d = (long)bp;
					long e = (long)(PREV_ADDR(bp));
					long f = (long)(NEXT_ADDR(bp));
					printf("this: %d  prev: %d  next: %d\n", a, b, c);
					printf("this address: %lx  prev address: %lx  next address: %lx", d, e, f);
					pause();

				}
			}
			else {
		//		printf("coalesce correct\n");
			}
			bp = NEXT_ADDR(bp);
		}
		printf("-------------------------\n\n");
	}
}