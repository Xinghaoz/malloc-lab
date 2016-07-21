/*
 *
 * This source code contains the functions act as 
 * malloc, free, realloc and calloc.
 * Detail information is in the header of each function.
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

/* When debugging, change DEBUG1 to DEBUG */
/* Note: It is EXTREMELY slow in the DEBUG mode */
#define DEBUG1

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
#define WSIZE				4	/* Word and header/footer size (bytes) */ 
#define DSIZE				8	/* Double word size (bytes) */
#define TSIZE				12	/* Triple word size (bytes) */
#define ALIGNMENT			8	/* Alignment size */
#define CHUNKSIZE			(1 << 8)
#define HEADER_SIZE			4	/* Header size 8 bytes */
#define OVERHEAD			8	/* Overhead = header + footer = 8 bytes */
#define MIN_SIZE			24	/* Minimux size 24 bytes */
#define LESS_THAN_PREVIOUS	 0	/* Indicator used in the insertion of tree */
#define LARGER_THAN_PREVIOUS 1	/* Indicator used in the insertion of tree */
#define IS_ROOT				 2	/* Indicator used in the insertion of tree */

#define MM_NULL ((void *)0x800000000)		/* Heap bottom */

/* Find the Max of the given two numbers */
#define MAX(x, y) ((x) > (y) ? (x) : (y))

/*Make the block to meet with the standard alignment requirements*/
#define ALIGN_SIZE(size) (((size) + (ALIGNMENT-1)) & ~0x7)

/* Pack a size and allocated bit into a word */
#define PACK(size, alloc)  ((size) | (alloc)) 

/* Read and write a word at address p */
#define GET(p)       (*(unsigned int *)(p))            
#define PUT(p, val)  (*(unsigned int *)(p) = (val)) 

#define MM_NULL ((void *)0x800000000)	/* Set the NULL to be heap bottom */

/* Header structure:
 *	31...7	6		5		4		3		2		1		0
 *	|-----------THIS_SIZE-----------|  IS_PREV_FREE 
 *												IS_PREV_ALLOC
 *														IS_ALLOC
 * IS_ALLOC:		1 if THIS block is allocated.
 * IS_PREV_ALLOC:	1 if the PREVIOUS block is allocated.
 * IS_PREV_8_BYTE:	1 if the PREVIOUS block is 8 byte free block.
 * THIS_SIZE:		Size of THIS block. 
 */
#define IS_ALLOC(bp)		(GET(HEADER_ADDR(bp)) & 0x1)
#define IS_PREV_ALLOC(bp)	(GET(HEADER_ADDR(bp)) & 0x2)
#define IS_PREV_8_BYTE(bp)	(GET(HEADER_ADDR(bp)) & 0x4)
#define THIS_SIZE(bp)		(GET(HEADER_ADDR(bp)) & ~(0x7))
#define PREV_SIZE(bp)		(THIS_SIZE(PREV_ADDR(bp)))
#define NEXT_SIZE(bp)		(THIS_SIZE(NEXT_ADDR(bp)))

/* Set and reset the IS_PREV_ALLOC and IS_PREV_8_BYTE parameters */
#define SET_IS_PREV_ALLOC(bp)	(GET(HEADER_ADDR(bp)) |= 0x2)
#define RESET_IS_PREV_ALLOC(bp) (GET(HEADER_ADDR(bp)) &= ~0x2)
#define SET_IS_PREV_8_SIZE(bp) (GET(HEADER_ADDR(bp)) |= 0x4)
#define RESET_IS_PREV_8_SIZE(bp) (GET(HEADER_ADDR(bp)) &= ~0x4)

/* Given block ptr bp, get the address of each parameters */
#define HEADER_ADDR(bp)			((char *)(bp) - WSIZE)                      
#define FOOTER_ADDR(bp)			((char *)(bp) + THIS_SIZE(bp) - DSIZE) 
#define LEFT_ADDR(bp)			((char *)(bp))
#define RIGHT_ADDR(bp)			((char *)(bp) + WSIZE)
#define PARENT_ADDR(bp)			((char *)(bp) + DSIZE)
#define BROTHER_ADDR(bp)		((char *)(bp) + TSIZE)
#define GET_LEFT(bp)	((char *)((long)GET(LEFT_ADDR(bp))|(0x800000000)))
#define GET_RIGHT(bp)	((char *)((long)GET(RIGHT_ADDR(bp))|(0x800000000)))
#define GET_PARENT(bp)	((char *)((long)GET(PARENT_ADDR(bp))|(0x800000000)))
#define GET_BROTHER(bp)	((char *)((long)GET(BROTHER_ADDR(bp))|(0x800000000)))

/* Given block ptr bp, compute address of next and previous blocks */
#define NEXT_ADDR(bp)  ((char *)(bp) + THIS_SIZE(bp))
#define PREV_ADDR(bp) (IS_PREV_8_BYTE(bp) ? ((char *)bp - DSIZE): \
		((char *)(bp) - (GET(((char *)(bp) - DSIZE)) & ~(0x7))))

/* Meaning: left child "point to", right child "point to", etc. */
#define LEFT_PTR(bp, addr)	 (PUT(LEFT_ADDR(bp), ((unsigned int)(long)addr)))
#define RIGHT_PTR(bp, addr)	 (PUT(RIGHT_ADDR(bp), ((unsigned int)(long)addr)))
#define PARENT_PTR(bp, add)  (PUT(PARENT_ADDR(bp), ((unsigned int)(long)add)))
#define BROTHER_PTR(bp, add) (PUT(BROTHER_ADDR(bp), ((unsigned int)(long)add)))


/* static functions */
static void *coalesce ( void *bp );
static void *extend_heap ( size_t size );
static void place ( void *ptr, size_t asize );
static void insert_node ( void *bp );
static void delete_node ( void *bp );
static void *find_fit ( size_t asize );
void print_heap(int version);
int tree_checker(void *root);
void print_tree();
void print_list_8();
void print_list_16();

/* Function prototypes for internal helper routines */
static void *heap_listp;	/* Pointer to first block */
static void *root;			/* Pointer to Binary Search Tree*/
static void *size_8_list;	/* Pointer to 8 bytes free list */
static void *size_16_list;  /* Pointer to 16 bytes free list */
int current_chuck = 1 << 8; /* Extend chuck size */

/*
 * mm_init - Initialize the memory manager 
 *	0	  4		8	  12    16    20
 * ----------------------------------------------
 * 0/0 | 0/0 | 0/0 | 8/1 | 8/1 | 0/3 | 
 * ----------------------------------------------
 *							¡ü
 *						heap_listp
 */
int mm_init(void)
{
    /* create the initial empty heap */
    if ((heap_listp = mem_sbrk(6 * WSIZE)) == (void *)-1 )
        return -1;
    PUT(heap_listp + (2 * WSIZE), 0 );				/* Alignment padding */	
    PUT(heap_listp + (3 * WSIZE), PACK(DSIZE, 1) ); /* Prologue header */ 
    PUT(heap_listp + (4 * WSIZE), PACK(DSIZE, 1) ); /* Prologue footer */
    PUT(heap_listp + (5 * WSIZE), PACK(0, 3) );		/* Epilogue header */
    heap_listp += (4 * WSIZE);

	
    root = MM_NULL;
    size_16_list = MM_NULL;
    size_8_list = MM_NULL;
    /* Extend the empty heap with a free block of CHUNKSIZE bytes */
	if (extend_heap(OVERHEAD) == NULL) {
		return -1;
	}
	return 0;
}

/* 
 * malloc - Allocate a block with at least size bytes of payload 
 */
void *malloc(size_t size)
{
    size_t asize; /* Adjusted block size */
    void *bp;

    if (heap_listp == 0){
        mm_init();
    }
    /* Ignore spurious requests */
    if( size <= 0 ) {
        return NULL;
	}

    /* We need to count the header and footer as overhead and align it */
    asize = ALIGN_SIZE(size + HEADER_SIZE);
    /* Find the proper free block */
    if((bp = find_fit(asize)) == MM_NULL) {
        extend_heap(asize);
        if((bp = find_fit(asize)) == MM_NULL) {
            return NULL;
		}
    }
	/* Mark the new malloc block as alloated */
    place(bp, asize);
	#ifdef DEBUG
	mm_checkheap(1);	
	#endif
    return bp;
}

/* 
 * free - Free a block 
 */
void free(void *bp)
{
    if(bp == NULL) {
		return;
	}
    size_t size = THIS_SIZE(bp);

    PUT(HEADER_ADDR(bp),PACK(size,IS_PREV_8_BYTE(bp) | IS_PREV_ALLOC(bp) | 0));
    PUT(FOOTER_ADDR(bp),PACK(size,IS_PREV_8_BYTE(bp) | IS_PREV_ALLOC(bp) | 0));

	if (!IS_ALLOC(bp) && (!IS_PREV_ALLOC(bp) || !IS_ALLOC(NEXT_ADDR(bp)))) {
		bp = coalesce(bp);
	}
    insert_node(bp);
	#ifdef DEBUG
	mm_checkheap(2);	
	#endif
}

/*
 * realloc - realloc according to the size.
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

    /* if ptr is NULL, the call is equivalent to malloc(size); */
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

void *calloc (size_t nmemb, size_t size) {
	size_t bytes = nmemb * size;
	void *newptr;
	
	newptr = malloc(bytes);
	memset(newptr, 0, bytes);
	
	return newptr;
}

/*
 * mm_checkheap
 * Evaluate the heap in every aspects.
 * If there is anything wrong,
 * print the related information and stop the program.
 * Otherwise it will run quitely and print nothing.
	 
 * version 1 is for debugging malloc.
 * version 2 is for debugging free.
 * version 3 is for debugging Binary Search Tree.
 * version 4 is for debugging 8-byte free block list.
 * version 5 is for debugging 16-byte free block list.
 */
void mm_checkheap(int version) {
	void *bp = heap_listp;

	/* Check the whole heap
	 * Whenever there is anything wrong, stop the program and
	 * Print the heap information - use for malloc
	 * Note: It is extremely slow when debugging.
	 */
	if (version == 1) {
		while (THIS_SIZE(bp) != 0) {
			int check1 = IS_ALLOC(bp);
			int check2 = IS_ALLOC(NEXT_ADDR(bp));
			if (!(check1 || check2)) {
				print_heap(1);
				exit(0);
			}
			bp = NEXT_ADDR(bp);
		}

	/* Check the whole heap
	 * Whenever there is anything wrong, stop the program and
	 * Print the heap information - use for free 
	 * Note: It is extremely slow when debugging.
	 */
	} else if (version == 2) {
		while (THIS_SIZE(bp) != 0) {
			int check1 = IS_ALLOC(bp);
			int check2 = IS_ALLOC(NEXT_ADDR(bp));
			if (!(check1 || check2)) {
				print_heap(1);
				exit(0);
			}
			bp = NEXT_ADDR(bp);
		}

	/* Check the Binary Search Tree.
	 * Whenever there is anything wrong, stop the program and
	 * print the information of the Binary Search Tree */
	} else if (version == 3) {
		if (tree_checker(root)) {
			printf("An error occur in the tree!!!");
			print_tree(root);
			exit(0);
		}

	/* Check the 8-byte free block list.
	 * Whenever there is anything wrong, stop the program and
	 * print the information of the 8-byte free block list */
	} else if (version == 4) {
		bp = size_8_list;
		while (MM_NULL != bp) {
			int check = IS_ALLOC(bp);
				if (check != 0) {
					printf("An error occur in the 8_byte_list!!!");
					print_list_8();
					exit(0);
				}
			bp = GET_LEFT(bp);
		}

	/* Check the 16-byte free block list.
	 * Whenever there is anything wrong, stop the program and
	 * print the information of the 16-byte free block list */
	} else if (version == 5) {
		bp = size_16_list;
		while (MM_NULL != bp) {
			int check = IS_ALLOC(bp);
				if (check != 0) {
					printf("An error occur in the 16_byte_list!!!");
					print_list_16();
					exit(0);
				}
			bp = GET_RIGHT(bp);
		}
	}
}

/* 
 * The remaining routines are internal helper routines 
 */

/* 
 * extend_heap - Extend heap with free block and return its block pointer
 */
void *extend_heap(size_t input_size)
{
    void *bp;
	int size = input_size;

	/* Optimization */
	char *epilogue_block = (char *)mem_heap_hi() + 1;

	/* If last block is free, 
	 * we can extend less block in order to increase the utilization.
	 */
	if (!IS_PREV_ALLOC(epilogue_block)) {     
		size -= THIS_SIZE(PREV_ADDR(epilogue_block));		
	}
	if (size <= 0) {
		return NULL;
	}
    
	/* CHUKSIZE is a tricky part */
	size = MAX(CHUNKSIZE, size);

    if ((long)(bp = mem_sbrk(size)) == -1) {
        return NULL;
	}
    /* Initialize free block header/footer and the epilogue header */
    PUT(HEADER_ADDR(bp),PACK(size,IS_PREV_8_BYTE(bp) | IS_PREV_ALLOC(bp) | 0));
    PUT(FOOTER_ADDR(bp),PACK(size,IS_PREV_8_BYTE(bp) | IS_PREV_ALLOC(bp) | 0));
	/* Set new epilogue header */
    PUT(HEADER_ADDR(NEXT_ADDR(bp)), PACK(0, 1));
    
	/* We should insert the new extend block as a new node */
    insert_node(coalesce(bp));
    return bp;         
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
	/* Case 2: Previous block is allocated, and next block is free */
	else if (prev_alloc && !next_alloc) {
	    size += NEXT_SIZE(bp);
	    delete_node(NEXT_ADDR(bp));
		PUT(HEADER_ADDR(bp), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		PUT(FOOTER_ADDR(bp), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		return bp;
	}
	/* Case 3: Previous block is free, and next block is allocated */
	else if (!prev_alloc && next_alloc) { 
		size += PREV_SIZE(bp);
	    char * prev = (char *)PREV_ADDR(bp);
	    delete_node(prev);
		PUT(HEADER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		PUT(FOOTER_ADDR(prev), PACK(size, IS_PREV_ALLOC(bp) | 0x0));
		return prev;
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


/* 
 * place - Place block of asize bytes at start of free block bp 
 *         and split if remainder would be at least minimum block size
 */
static void place(void *bp,size_t input_size)
{
	size_t block_size = THIS_SIZE(bp);

	/* Once the block is malloc, we should remove it from free list */
	delete_node(bp);
	PUT(HEADER_ADDR(bp), PACK(input_size, IS_PREV_ALLOC(bp) | 0x1));

	/* If the remaining block is larger than OVERHEAD size (8 bytes),
	 * we should split it and set it as free block.
	 */
	if ((block_size - input_size) >= OVERHEAD) {
		bp = NEXT_ADDR(bp);
		/* Set the splited block as free block and 
		 * set its previous block as allocated block.
		 * We should also insert the splited block into free list.
		 */
		PUT(HEADER_ADDR(bp), PACK(block_size - input_size, 0x2));
		PUT(FOOTER_ADDR(bp), PACK(block_size - input_size, 0x2));		
		insert_node(bp);
	}

}

/* 
 * find_fit - Find a fit for a block with asize bytes 
 * There are 3 parts:
 * 1. If the search size is 8 bytes, search from the 8-byte free list.
 * 2. If the search size is 16 bytes, search from the 16-byte free list.
 * 3. Else, search from the Binary Search Tree.
 */
static void *find_fit(size_t size)
{
	/* If the size is 8 bytes or 16 bytes,
	   return the lists respectively.
	 */
    if (size <= 8 && size_8_list != MM_NULL) {
		return size_8_list;
	} else if(size <= 16 && size_16_list != MM_NULL) {
		return size_16_list;
	}

	/* BST search */	
	void *fit = MM_NULL;
	void *current_node = root;
	while (current_node != MM_NULL) {
		if (size == THIS_SIZE(current_node)) {
			return current_node;
		} else if (size < THIS_SIZE(current_node)) {
			fit = current_node;
			current_node = GET_LEFT(current_node);
		}
		else {
			current_node = GET_RIGHT(current_node);
		}
	}
	return fit;
	
}

/* 
 * insert_node - 
 * when we free/extend/splited a block,
 * we have to insert the free block into the free list.
 * There are 3 parts:
 * 1. If the insert block size is 8 bytes, insert it to the 8-byte free list.
 * 2. If the insert block size is 16 bytes, insert it to the 16-byte free list.
 * 3. Else, we find the most fit part in the Binary Search Tree.
 */
inline static void insert_node(void *bp)
{
	/* when we insert a node,
	 * we should notify its next block.
	 */
    RESET_IS_PREV_ALLOC(NEXT_ADDR(bp));
    size_t insert_size = THIS_SIZE(bp);

	/* Insert as the FIRST node of the list */
    if (insert_size == 8){
        SET_IS_PREV_8_SIZE(NEXT_ADDR(bp));
        LEFT_PTR(bp, size_8_list);
        size_8_list = bp;
		#ifdef DEBUG
		mm_checkheap(4);	
		#endif
		return;
    }    

	/*	Insert as the FIRST node of the list
	 *	bp -> right = list;
	 *	list -> left = bp;
	 *	list = bp;
	 *	bp -> left = null;
	 */
    if (insert_size == 16) {
		RIGHT_PTR(bp, size_16_list);
		LEFT_PTR(size_16_list, bp);
        size_16_list = bp;
		LEFT_PTR(bp, MM_NULL);
		#ifdef DEBUG
		mm_checkheap(5);	
		#endif
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
	void *current_parent = MM_NULL;
	void *current_node = root;
	int sign = IS_ROOT;

	/* Initilize the insert node */
	LEFT_PTR(bp, MM_NULL);
	RIGHT_PTR(bp, MM_NULL);
	BROTHER_PTR(bp, MM_NULL);

	while (1) {
	    if (MM_NULL == current_node){
			PARENT_PTR(bp, current_parent);
            break;
        }
		else if (insert_size == THIS_SIZE(current_node)){
		    PUT(LEFT_ADDR(bp), GET(LEFT_ADDR(current_node)));
            PUT(RIGHT_ADDR(bp), GET(RIGHT_ADDR(current_node)));
            PARENT_PTR(GET_LEFT(current_node), bp);
            PARENT_PTR(GET_RIGHT(current_node), bp);
		    PARENT_PTR(bp, current_parent);
			/* Change the order will cause infinite loop */
			BROTHER_PTR(bp, current_node);
            LEFT_PTR(current_node, bp);
		    break;
		}
		else if (insert_size < THIS_SIZE(current_node)) {
			current_parent = current_node;
			current_node = GET_LEFT(current_node);
			sign = LESS_THAN_PREVIOUS;
		}
		else {
			current_parent = current_node;
			current_node = GET_RIGHT(current_node);
			sign = LARGER_THAN_PREVIOUS;
		}
	}
	if (sign == IS_ROOT) {
		root = bp;
		#ifdef DEBUG
		mm_checkheap(3);	
		#endif
		return;
	} else if (sign == LESS_THAN_PREVIOUS) {
		LEFT_PTR(current_parent, bp);
		#ifdef DEBUG
		mm_checkheap(3);	
		#endif
		return;
	} else if (sign == LARGER_THAN_PREVIOUS) {
		RIGHT_PTR(current_parent, bp);
		#ifdef DEBUG
		mm_checkheap(3);	
		#endif
		return;
	}

}

/* 
 * delete_node - 
 * when we allocate a block,
 * we have to delete the free block into the free list.
 * There are 3 parts:
 * 1. If the delete size is 8 bytes, remove it from the 8-byte free list.
 * 2. If the delete size is 16 bytes, remove it from the 16-byte free list.
 * 3. Else, remove it from the Binary Search Tree.
 */
inline static void delete_node(void *bp)
{
	/* when we delete a node,
	 * we should notify its next block.
	 */
	SET_IS_PREV_ALLOC(NEXT_ADDR(bp));
    size_t delete_size = THIS_SIZE(bp);

    if (8 == delete_size) {
        RESET_IS_PREV_8_SIZE(NEXT_ADDR(bp));
        char *current_node = size_8_list;
        if (current_node == bp) {
            size_8_list = GET_LEFT(bp);
			#ifdef DEBUG
			mm_checkheap(4);	
			#endif
            return;
        }
        while(current_node != MM_NULL) {
            if (GET_LEFT(current_node) == bp) {
				break;
			}
            current_node = GET_LEFT(current_node); 
        } 
		#ifdef DEBUG
		mm_checkheap(4);	
		#endif
        LEFT_PTR(current_node, GET_LEFT(bp));
		return;
    }     
	/*
	 *	bp -> left -> right = bp -> right;
	 *	bp -> right -> left = bp -> left;
	 */
    if (delete_size == 16) {		
		if (bp == size_16_list) {
            size_16_list = GET_RIGHT(bp); 
		}		
		RIGHT_PTR(GET_LEFT(bp), GET_RIGHT(bp));
		LEFT_PTR(GET_RIGHT(bp), GET_LEFT(bp));
		#ifdef DEBUG
		mm_checkheap(5);	
		#endif		
        return;
    }    
    if(delete_size < MIN_SIZE) {
		return;
	}


    /*	If the deleting node has two more brothers 
	 *	bp -> bro -> left = bp -> left;
	 *	bp -> left -> bro = bp -> bro;
	 */

    if (GET_BROTHER(GET_LEFT(bp)) == bp) {
        LEFT_PTR(GET_BROTHER(bp), GET_LEFT(bp));
		BROTHER_PTR(GET_LEFT(bp), GET_BROTHER(bp));
    }
	/* If the deleting node doesn't have brothers */
	else if(GET_BROTHER(bp) == MM_NULL) {
		/* If the deleting node is the root */
		if (bp == root) {
			/* If bp doesn't have right child */
			if (GET_RIGHT(bp) == MM_NULL) {
				root = GET_LEFT(bp);
				PARENT_PTR(root, MM_NULL);
			/* If bp doesn't have left child */
			} else if (GET_LEFT(bp) == MM_NULL) {
				root = GET_RIGHT(bp);
				PARENT_PTR(root, MM_NULL);
			}
			/* If bp has both left child and right child,
			 * search the BIGEST one in its LEFT child.
			 * Then exchange the one and bp.
			 * In fact, we can also find the SMALEST one in its RIGHT child.
			 * However, because we link the brothers as left,
			 * it might cost more time to search the right children.
			 */
			else {
				if (GET_RIGHT(GET_LEFT(bp)) == MM_NULL) {
					root = GET_LEFT(bp);
					RIGHT_PTR(root, GET_RIGHT(bp));
					PARENT_PTR(GET_RIGHT(bp), root);
					PARENT_PTR(root, MM_NULL);
				} else {
					char *find_bigest = GET_LEFT(bp);
					while(GET_RIGHT(find_bigest) != MM_NULL) {
						find_bigest = GET_RIGHT(find_bigest);
					}
					char *bp_left = GET_LEFT(bp);
					char *bp_right = GET_RIGHT(bp);
					char *bigest_left = GET_LEFT(find_bigest);
					char *bigest_parent = GET_PARENT(find_bigest);
					root = find_bigest;
					PARENT_PTR(root, MM_NULL);
					RIGHT_PTR(root, bp_right);
					PARENT_PTR(bp_right, root);
					LEFT_PTR(root, bp_left);
					PARENT_PTR(bp_left, root);
					RIGHT_PTR(bigest_parent, bigest_left);
					PARENT_PTR(bigest_left, bigest_parent);
				}
			}
		}
		/* bp is not the root */
		else {
			/* If bp doesn't have left child */
			if (GET_LEFT(bp) == MM_NULL) {
				if (GET_LEFT(GET_PARENT(bp)) == bp) {
					LEFT_PTR(GET_PARENT(bp), GET_RIGHT(bp));
				} else {
					RIGHT_PTR(GET_PARENT(bp), GET_RIGHT(bp));
				}
                PARENT_PTR(GET_RIGHT(bp), GET_PARENT(bp));

			/* If bp doesn't have right child */
			} else if (GET_RIGHT(bp) == MM_NULL) {
				if (GET_LEFT(GET_PARENT(bp)) == bp) {
					LEFT_PTR(GET_PARENT(bp), GET_LEFT(bp));
				} else {
					RIGHT_PTR(GET_PARENT(bp), GET_LEFT(bp));
				}
                PARENT_PTR(GET_LEFT(bp), GET_PARENT(bp));

			/* If bp has both left child and right child,
			 * search the BIGEST one in its LEFT child.
			 * Then exchange the one and bp.
			 * In fact, we can also find the SMALEST one in its RIGHT child.
			 * However, because we link the brothers as left,
			 * it might cost more time to search the right children.
			 */
			} else {
				if (GET_RIGHT(GET_LEFT(bp)) == MM_NULL) {
					if (GET_LEFT(GET_PARENT(bp)) == bp) {
						LEFT_PTR(GET_PARENT(bp), GET_LEFT(bp));
					} else {
						RIGHT_PTR(GET_PARENT(bp), GET_LEFT(bp));
					}
					PARENT_PTR(GET_LEFT(bp), GET_PARENT(bp));
					RIGHT_PTR(GET_LEFT(bp), GET_RIGHT(bp));
					PARENT_PTR(GET_RIGHT(bp), GET_LEFT(bp));
				} else {
					char *bigest = GET_LEFT(bp);
					while (GET_RIGHT(bigest) != MM_NULL) {
						bigest = GET_RIGHT(bigest);
					}
					char *bigest_left = GET_LEFT(bigest);
					char *bigest_parent = GET_PARENT(bigest);
					char *bp_left = GET_LEFT(bp);
					char *bp_right = GET_RIGHT(bp);
					char *bp_parent = GET_PARENT(bp);

					if (GET_LEFT(bp_parent) == bp) {
						LEFT_PTR(bp_parent, bigest);
					} else {
						RIGHT_PTR(bp_parent, bigest);
					}
					PARENT_PTR(bigest, bp_parent);
					LEFT_PTR(bigest, bp_left);
					RIGHT_PTR(bigest, bp_right);
					PARENT_PTR(bp_left, bigest);
					PARENT_PTR(bp_right, bigest);
					RIGHT_PTR(bigest_parent, bigest_left);
					PARENT_PTR(bigest_left, bigest_parent);
				}
			}
		}
	}
	/* If the node is the FIRST brother */
    else {
	    char *temp = GET_BROTHER(bp);
		if (bp == root) {
			root = temp;
			PARENT_PTR(temp, MM_NULL);
		} else {		
		    if (GET_LEFT(GET_PARENT(bp)) == bp)
				LEFT_PTR(GET_PARENT(bp), temp);
			else {
				RIGHT_PTR(GET_PARENT(bp), temp);
			}
			PARENT_PTR(temp, GET_PARENT(bp));
		}
		
		LEFT_PTR(temp, GET_LEFT(bp));
        RIGHT_PTR(temp, GET_RIGHT(bp));
		PARENT_PTR(GET_LEFT(bp), temp);
	    PARENT_PTR(GET_RIGHT(bp), temp);
	}
	#ifdef DEBUG
	mm_checkheap(3);	
	#endif
}

/* 
 * print_heap - 
 * print the all information of the whole heap.
 */
void print_heap(int version)
{
	void * bp = heap_listp;
	/* print ALL the information of the whole heap. */
	if (1 == version) {
		printf("----------malloc-----------\n");
		while (THIS_SIZE(bp) != 0) {		
			int offset = (int)(bp - MM_NULL);
			int this_size = THIS_SIZE(bp);
			printf("address bias:  %d / %d%d%d\n",
				offset, (int)IS_PREV_8_BYTE(bp), 
				(int)IS_PREV_ALLOC(bp), (int)IS_ALLOC(bp));               
			printf("size        :  %d\n",(int)THIS_SIZE(bp));
			printf("next	    :  %d / %d\n",
				this_size + offset, (int)mem_heapsize());
			long this_addr = (long)(bp);
			long prev_addr = (long)(PREV_ADDR(bp));
			long next_addr = (long)(NEXT_ADDR(bp));
			printf("this address: %lx  prev address: %lx  next address: %lx\n",
				this_addr, prev_addr, next_addr);

			bp = NEXT_ADDR(bp);
		}
		printf("-------------------------\n\n");
	} else if (2 == version) {
		printf("-----------free----------\n");
		while (THIS_SIZE(bp) != 0) {
			int offset = (int)(bp - MM_NULL);
			int this_size = THIS_SIZE(bp);
			printf("address bias:  %d / %d%d%d\n",
				offset, (int)IS_PREV_8_BYTE(bp), 
				(int)IS_PREV_ALLOC(bp), (int)IS_ALLOC(bp));               
			printf("size        :  %d\n",(int)THIS_SIZE(bp));
			printf("next	    :  %d / %d\n",
				this_size + offset, (int)mem_heapsize());
			long this_addr = (long)(bp);
			long prev_addr = (long)(PREV_ADDR(bp));
			long next_addr = (long)(NEXT_ADDR(bp));
			printf("this address: %lx  prev address: %lx  next address: %lx\n",
				this_addr, prev_addr, next_addr);

			bp = NEXT_ADDR(bp);
		}
		printf("-------------------------\n\n");
	}
}

/* 
 * tree_checker - 
 * Use recursion to check the tree.
 * If all the node in the tree is correct, return 0;
 * If any of the node is incorrect, return 1.
 */
int tree_checker(void *root) {
	if (MM_NULL == root) {
		return 0;
	}
	int check = IS_ALLOC(root);

	if (1 == check) {
		return 1;
	}

	tree_checker(GET_LEFT(root));
	tree_checker(GET_RIGHT(root));

	return 0;
}

/* 
 * print_tree - 
 * Use recursion to print the information of the whole tree.
 */
void print_tree(void *root) {
	if (MM_NULL == root) {
		return;
	}

	print_tree(GET_LEFT(root));
	print_tree(GET_RIGHT(root));

	long left = (long)(GET_LEFT(root));
	long right = (long)(GET_RIGHT(root));
	long parent = (long)(GET_PARENT(root));


	printf("THIS: %lx \tLEFT: %lx \tPARENT: %lx \tRIGHT: %lx\n",
		(long)root, left, parent, right);
}

/* 
 * print_list_8 - 
 * Print the heap infomation of the 8-byte free list.
 */
void print_list_8() {
	void *bp = size_8_list;
	printf("Heap size :  %d\t", (int)mem_heapsize());
	while (MM_NULL != bp) {
		long this_addr = (long)(bp);
		int sign1 = IS_ALLOC(bp);
		int sign2 = IS_PREV_ALLOC(bp);
		int sign3 = IS_PREV_8_BYTE(bp);
		int this_size = THIS_SIZE(bp);
		printf("address: %lx /%d%d%d/%d---> ", 
			this_addr, sign3, sign2, sign1, this_size);
		bp = GET_LEFT(bp);
	}
	printf("\n");
}

/* 
 * print_list_16 - 
 * Print the heap infomation of the 16-byte free list.
 */
void print_list_16() {
	void *bp = size_16_list;
	printf("Heap size :  %d\t", (int)mem_heapsize());
	while (MM_NULL != bp) {
		long this_addr = (long)(bp);
		int sign1 = IS_ALLOC(bp);
		int sign2 = IS_PREV_ALLOC(bp);
		int sign3 = IS_PREV_8_BYTE(bp);
		int this_size = THIS_SIZE(bp);
		printf("address: %lx /%d%d%d/%d---> ", 
			this_addr, sign3, sign2, sign1, this_size);
		bp = GET_LEFT(bp);
	}
	printf("\n");
}

