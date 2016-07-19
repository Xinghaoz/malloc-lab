/*
 ******************************************************************************
 * mm.c
 * Name: Siyu Jiang
 * Andrew ID: siyuj
 *  ************************************************************************  *
 *                               DOCUMENTATION                                *
 *                                                                            *
 *  ** STRUCTURE. **                                                          *
 *                                                                            *
 *  Both allocated and free blocks share the same header structure.           *
 *  HEADER: 8-byte, aligned to 8th byte of an 16-byte aligned heap, where     *
 *          - The lowest order bit is 1 when the block is allocated, and      *
 *            0 otherwise.                                                    *
 *          - The whole 8-byte value with the least significant bit set to 0  *
 *            represents the size of the block as a size_t                    *
 *            The size of a block includes the header and footer.             *
 *  FOOTER: 8-byte, aligned to 0th byte of an 16-byte aligned heap. It        *
 *          contains the exact copy of the block's header.                    *
 *  The minimum blocksize is 32 bytes.                                        *
 *                                                                            *
 *  Allocated blocks contain the following:                                   *
 *  HEADER, as defined above.                                                 *
 *  PAYLOAD: Memory allocated for program to store information.               *
 *  FOOTER, as defined above.                                                 *
 *  The size of an allocated block is exactly PAYLOAD + HEADER + FOOTER.      *
 *                                                                            *
 *  Free blocks contain the following:                                        *
 *  HEADER, as defined above.                                                 *
 *  FOOTER, as defined above.                                                 *
 *  The size of an unallocated block is at least 32 bytes.                    *
 *                                                                            *
 *  Block Visualization.                                                      *
 *                    block     block+8          block+size-8   block+size    *
 *  Allocated blocks:   |  HEADER  |  ... PAYLOAD ...  |  FOOTER  |           *
 *                                                                            *
 *                    block     block+8          block+size-8   block+size    *
 *  Unallocated blocks: |  HEADER  |  ... (empty) ...  |  FOOTER  |           *
 *                                                                            *
 *  ************************************************************************  *
 *  ** INITIALIZATION. **                                                     *
 *                                                                            *
 *  The following visualization reflects the beginning of the heap.           *
 *      start            start+8           start+16                           *
 *  INIT: | PROLOGUE_FOOTER | EPILOGUE_HEADER |                               *
 *  PROLOGUE_FOOTER: 8-byte footer, as defined above, that simulates the      *
 *                    end of an allocated block. Also serves as padding.      *
 *  EPILOGUE_HEADER: 8-byte block indicating the end of the heap.             *
 *                   It simulates the beginning of an allocated block         *
 *                   The epilogue header is moved when the heap is extended.  *
 *                                                                            *
 *  ************************************************************************  *
 *  ** BLOCK ALLOCATION. **                                                   *
 *                                                                            *
 *  Upon memory request of size S, a block of size S + dsize, rounded up to   *
 *  16 bytes, is allocated on the heap, where dsize is 2*8 = 16.              *
 *  Selecting the block for allocation is performed by finding the first      *
 *  block that can fit the content based on a first-fit or next-fit search    *
 *  policy.                                                                   *
 *  First-fit: The search starts from the beginning of the heap pointed by    *
 *             heap_listp. It sequentially goes through each block in the     *
 *             implicit free list, towards the end of the heap, until either  *
 *             - A sufficiently-large unallocated block is found, or          *
 *             - The end of the implicit free list is reached, which occurs   *
 *               when no sufficiently-large unallocated block is available.   *
 *  Next-fit:  The search starts from the most recently allocated block in    *
 *             the implicit free list. It sequentially goes through each      *
 *             block to the end of the implicit free list, towards the end    *
 *             of the heap until either                                       *
 *             - A sufficiently-large unallocated block is found, or          *
 *             - The end of the implicit free list is reached.                *
 *             In the case that the end of the implicit free list is reached, *
 *             the search continues from the beginning of the heap pointed    *
 *             by heap_listp, sequentially going through each block in the    *
 *             implicit free list, until either                               *
 *             - A sufficiently-large unallocated block is found, or          *
 *             - The most recently allocated block in the implicit free list  *
 *               is reached, which occurs when no sufficiently-large          *
 *               unallocated block is available.                              *
 *  In case that a sufficiently-large unallocated block is found, then        *
 *  that block will be used for allocation. Otherwise--that is, when no       *
 *  sufficiently-large unallocated block is found--then more unallocated      *
 *  memory of size chunksize or requested size, whichever is larger, is       *
 *  requested through mem_sbrk, and the search is redone.                     *
 *                                                                            *
 *  ************************************************************************  *
 *  ** ADVICE FOR STUDENTS. **                                                *
 *  Step 0: Please read the writeup!                                          *
 *  Write your heap checker. Write your heap checker. Write. Heap. checker.   *
 *  Good luck, and have fun!                                                  *
 *                                                                            *
 ******************************************************************************
 */

/* Do not change the following! */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "mm.h"
#include "memlib.h"

#ifdef DRIVER
/* create aliases for driver tests */
#define malloc mm_malloc
#define free mm_free
#define realloc mm_realloc
#define calloc mm_calloc
#define memset mem_memset
#define memcpy mem_memcpy
#endif /* def DRIVER */

/* You can change anything from here onward */

/*
 * If DEBUG is defined, enable printing on dbg_printf and contracts.
 * Debugging macros, with names beginning "dbg_" are allowed.
 * You may not define any other macros having arguments.
 */
// #define DEBUG // uncomment this line to enable debugging

#ifdef DEBUG
/* When debugging is enabled, these form aliases to useful functions */
#define dbg_printf(...) printf(__VA_ARGS__)
#define dbg_requires(...) assert(__VA_ARGS__)
#define dbg_assert(...) assert(__VA_ARGS__)
#define dbg_ensures(...) assert(__VA_ARGS__)
#else
/* When debugging is disnabled, no code gets generated for these */
#define dbg_printf(...)
#define dbg_requires(...)
#define dbg_assert(...)
#define dbg_ensures(...)
#endif


/* Basic constants */
typedef uint64_t word_t; // 8 bytes
static const size_t wsize = sizeof(word_t);   // word, header, footer size (bytes)
static const size_t dsize = 2*wsize;          // double word size (bytes)
static const size_t min_block_size = 2*dsize; // Minimum block size
static const size_t chunksize = (1 << 16);    // requires (chunksize % 16 == 0)

typedef struct block
{
    /* Header contains size + allocation flag */
    word_t header;
    /*
     * We don't know how big the payload will be.  Declaring it as an
     * array of size 0 allows computing its starting address using
     * pointer notation.
     */
    char payload[0];
    /*
     * We can't declare the footer as part of the struct, since its starting
     * position is unknown
     */
} block_t;


/* Global variables */
/* Pointer to first block */
//static block_t *heap_listp = NULL;
static block_t *free_listp = NULL;



/* Function prototypes for internal helper routines */
static block_t *extend_heap(size_t size);
static void place(block_t *block, size_t asize);
static block_t *find_fit(size_t asize);
static block_t *coalesce(block_t *block);

static size_t max(size_t x, size_t y);
static size_t round_up(size_t size, size_t n);
static word_t pack(size_t size, bool alloc);

static size_t extract_size(word_t header);
static size_t get_size(block_t *block);
static size_t get_payload_size(block_t *block);

static bool extract_alloc(word_t header);
static bool get_alloc(block_t *block);

static void write_header(block_t *block, size_t size, bool alloc);
static void write_footer(block_t *block, size_t size, bool alloc);

static block_t *payload_to_header(void *bp);
static void *header_to_payload(block_t *block);

static block_t *find_next(block_t *block);
static word_t *find_prev_footer(block_t *block);
static block_t *find_prev(block_t *block);

bool mm_checkheap(int lineno);

/*
 *  Self-defined functions
 */
static block_t *get_prev(block_t *block);
static block_t *get_next(block_t *block);
static void write_prev(block_t *block, block_t *next);
static void write_next(block_t *block, block_t *next);
static void insert(block_t *block);
static void delete_node(block_t *block);
static void connect_two(block_t *node1, block_t *node2);

/*
 * mm_init: initializes the heap; it is run once when heap_start == NULL.
 *          prior to any extend_heap operation, this is the heap:
 *              start            start+8           start+16
 *          INIT: | PROLOGUE_FOOTER | EPILOGUE_HEADER |
 * heap_listp ends up pointing to the epilogue header.
 */
bool mm_init(void)
{
    // Create the initial empty heap
    word_t *start = (word_t *)(mem_sbrk(2*wsize));

    if (start == (void *)-1)
    {
        return false;
    }

    start[0] = pack(0, true); // Prologue footer
    start[1] = pack(0, true); // Epilogue header
    // Heap starts with first block header (epilogue)

    // Remember!
    free_listp = NULL;

    // Extend the empty heap with a free block of chunksize bytes
    if (extend_heap(chunksize) == NULL)
    {
        return false;
    }

    return true;
}

/*
 * malloc: allocates a block with size at least (size + dsize), rounded up to
 *         the nearest 16 bytes, with a minimum of 2*dsize. Seeks a
 *         sufficiently-large unallocated block on the heap to be allocated.
 *         If no such block is found, extends heap by the maximum between
 *         chunksize and (size + dsize) rounded up to the nearest 16 bytes,
 *         and then attempts to allocate all, or a part of, that memory.
 *         Returns NULL on failure, otherwise returns a pointer to such block.
 *         The allocated block will not be used for further allocations until
 *         freed.
 */
void *malloc(size_t size)
{
    size_t asize;      // Adjusted block size
    size_t extendsize; // Amount to extend heap if no fit is found
    block_t *block;
    void *bp = NULL;

    if (free_listp == NULL) // Initialize heap if it isn't initialized
    {
        mm_init();
    }

/*    if (size == 0) // Ignore spurious request
    {
	dbg_printf("Malloc(%zd) --> %p\n", size, bp);
        return bp;
    }
*/
    // Adjust block size to include overhead and to meet alignment requirements
    asize = round_up(size, dsize) + dsize;
    //printf("asize = %ld\n", (long)asize);

    // Search the free list for a fit
    //mm_checkheap(0);
    block = find_fit(asize);

    // If no fit is found, request more memory, and then and place the block
    if (block == NULL)
    {
        extendsize = max(asize, chunksize);
        block = extend_heap(extendsize);
/*        if (block == NULL) // extend_heap returns an error
        {
	    dbg_printf("Malloc(%zd) --> %p\n", size, bp);
            return bp;
        }
*/
    }

    place(block, asize);
    bp = header_to_payload(block);
//  dbg_printf("Malloc(%zd) --> %p\n", size, bp);
  //printf("Malloc(%lx) --> %p\n", asize, bp);
  //mm_checkheap(0);
    return bp;
}

/*
 * free: Frees the block such that it is no longer allocated while still
 *       maintaining its size. Block will be available for use on malloc.
 */
void free(void *bp)
{
    if (bp == NULL)
    {
        return;
    }

    block_t *block = payload_to_header(bp);
    size_t size = get_size(block);

    write_header(block, size, false);
    write_footer(block, size, false);

    block = coalesce(block);

    // -------
    //insert(block);
  //printf("Completed free(%p)\n", bp);
  //mm_checkheap(0);
//  dbg_printf("Completed free(%p)\n", bp);
}

/*
 * realloc: returns a pointer to an allocated region of at least size bytes:
 *          if ptrv is NULL, then call malloc(size);
 *          if size == 0, then call free(ptr) and returns NULL;
 *          else allocates new region of memory, copies old data to new memory,
 *          and then free old block. Returns old block if realloc fails or
 *          returns new pointer on success.
 */
void *realloc(void *ptr, size_t size)
{
    block_t *block = payload_to_header(ptr);
    size_t copysize;
    void *newptr;

    // If size == 0, then free block and return NULL
    if (size == 0)
    {
        free(ptr);
        return NULL;
    }

    // If ptr is NULL, then equivalent to malloc
    if (ptr == NULL)
    {
        return malloc(size);
    }

    // Otherwise, proceed with reallocation
    newptr = malloc(size);
    // If malloc fails, the original block is left untouched
    if (!newptr)
    {
        return NULL;
    }

    // Copy the old data
    copysize = get_payload_size(block); // gets size of old payload
    if(size < copysize)
    {
        copysize = size;
    }
    memcpy(newptr, ptr, copysize);

    // Free the old block
    free(ptr);

    return newptr;
}

/*
 * calloc: Allocates a block with size at least (elements * size + dsize)
 *         through malloc, then initializes all bits in allocated memory to 0.
 *         Returns NULL on failure.
 */
void *calloc(size_t nmemb, size_t size)
{
    void *bp;
    size_t asize = nmemb * size;

    if (asize/nmemb != size)
	// Multiplication overflowed
      return NULL;

    bp = malloc(asize);
    if (bp == NULL)
    {
        return NULL;
    }
    // Initialize all bits to 0
    memset(bp, 0, asize);

    return bp;
}

/******** The remaining content below are helper and debug routines ********/

/*
 * extend_heap: Extends the heap with the requested number of bytes, and
 *              recreates epilogue header. Returns a pointer to the result of
 *              coalescing the newly-created block with previous free block, if
 *              applicable, or NULL in failure.
 */
static block_t *extend_heap(size_t size)
{
    void *bp;

    // Allocate an even number of words to maintain alignment
    size = round_up(size, dsize);
    if ((bp = mem_sbrk(size)) == (void *)-1)
    {
      return NULL;
    }

    // Initialize free block header/footer
    block_t *block = payload_to_header(bp);
    write_header(block, size, false);
    write_footer(block, size, false);
    // Create new epilogue header
    block_t *block_next = find_next(block);
    write_header(block_next, 0, true);

    // Coalesce in case the previous block was free
    block = coalesce(block);

    // Insert new free block -------
   //insert(block);
  //printf("EXTEND: block = %lx\n", (long)block);
  //mm_checkheap(0);
    return block;
}

/* Coalesce: Coalesces current block with previous and next blocks if
 *           either or both are unallocated; otherwise the block is not
 *           modified. Then, insert coalesced block into the segregated list.
 *           Returns pointer to the coalesced block. After coalescing, the
 *           immediate contiguous previous and next blocks must be allocated.
 */
static block_t *coalesce(block_t * block)
{
    block_t *block_next = find_next(block);
    block_t *block_prev = find_prev(block);

    bool prev_alloc = extract_alloc(*(find_prev_footer(block)));
    bool next_alloc = get_alloc(block_next);
    size_t size = get_size(block);

    if (prev_alloc && next_alloc)              // Case 1
    {
    	insert(block);
        return block;
    }

    else if (prev_alloc && !next_alloc)        // Case 2
    {
        delete_node(block_next);
        size += get_size(block_next);
        write_header(block, size, false);
        write_footer(block, size, false);
        //delete(block_next);
    }

    else if (!prev_alloc && next_alloc)        // Case 3
    {
        delete_node(block_prev);
        size += get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block, size, false);
        block = block_prev;
        //delete(block_prev);
    }

    else                                        // Case 4
    {
        size += get_size(block_next) + get_size(block_prev);
        write_header(block_prev, size, false);
        write_footer(block_next, size, false);

        block = block_prev;
        delete_node(block_prev);
        delete_node(block_next);
    }

    insert(block);
    return block;
}

/*
 * place: Places block with size of asize at the start of bp. If the remaining
 *        size is at least the minimum block size, then split the block to the
 *        the allocated block and the remaining block as free, which is then
 *        inserted into the segregated list. Requires that the block is
 *        initially unallocated.
 */
static void place(block_t *block, size_t asize)
{
    size_t csize = get_size(block);

    //block_t *prev_block = get_prev(block);
    //block_t *next_block = get_next(block);

    delete_node(block);

    if ((csize - asize) >= min_block_size)
    {
        block_t *block_next;
        write_header(block, asize, true);
        write_footer(block, asize, true);

        block_next = find_next(block);
        write_header(block_next, csize-asize, false);
        write_footer(block_next, csize-asize, false);
        // printf("1: Insert to free list: prev<%lx> mid<%lx> next<%lx>\n", (long)prev_block, (long)block_next, (long)next_block);
        insert(block_next); // Insert the newly splitting block to free list
    }

    else
    {
        write_header(block, csize, true);
        write_footer(block, csize, true);
    }
}

/*
 * find_fit: Looks for a free block with at least asize bytes.
 *           Returns NULL if none found
 */
static block_t *find_fit(size_t asize)
{
    // First-fit search
    if (get_size(free_listp) >= asize){
    	return free_listp;
    }

    block_t *block=free_listp;

    while ((block=get_next(block)) != 0)
    {
        // printf("finding");
        if (get_size(block) >= asize)
        {
            return block;
        }
        //printf("[%lx] [%lx]\n", (long)block, (long)get_next(block));
    }
    return NULL;

}

/*
 * max: returns x if x > y, and y otherwise.
 */
static size_t max(size_t x, size_t y)
{
    return (x > y) ? x : y;
}


/*
 * round_up: Rounds size up to next multiple of n
 */
static size_t round_up(size_t size, size_t n)
{
    return (n * ((size + (n-1)) / n));
}

/*
 * pack: returns a header reflecting a specified size and its alloc status.
 *       If the block is allocated, the lowest bit is set to 1, and 0 otherwise.
 */
static word_t pack(size_t size, bool alloc)
{
    return alloc ? (size | 1) : size;
}


/*
 * extract_size: returns the size of a given header value based on the header
 *               specification above.
 */
static size_t extract_size(word_t word)
{
    return (word & ~(word_t) 0xF);
}

/*
 * get_size: returns the size of a given block by clearing the lowest 4 bits
 *           (as the heap is 16-byte aligned).
 */
static size_t get_size(block_t *block)
{
    return extract_size(block->header);
}

/*
 * get_payload_size: returns the payload size of a given block, equal to
 *                   the entire block size minus the header and footer sizes.
 */
static word_t get_payload_size(block_t *block)
{
    size_t asize = get_size(block);
    return asize - dsize;
}

/*
 * extract_alloc: returns the allocation status of a given header value based
 *                on the header specification above.
 */
static bool extract_alloc(word_t word)
{
    return (bool)(word & 0x1);
}

/*
 * get_alloc: returns true when the block is allocated based on the
 *            block header's lowest bit, and false otherwise.
 */
static bool get_alloc(block_t *block)
{
    return extract_alloc(block->header);
}

/*
 * write_header: given a block and its size and allocation status,
 *               writes an appropriate value to the block header.
 */
static void write_header(block_t *block, size_t size, bool alloc)
{
    block->header = pack(size, alloc);
}


/*
 * write_footer: given a block and its size and allocation status,
 *               writes an appropriate value to the block footer by first
 *               computing the position of the footer.
 */
static void write_footer(block_t *block, size_t size, bool alloc)
{
    word_t *footerp = (word_t *)((block->payload) + get_size(block) - dsize);
    *footerp = pack(size, alloc);
}


/*
 * find_next: returns the next consecutive block on the heap by adding the
 *            size of the block.
 */
static block_t *find_next(block_t *block)
{
    return (block_t *)(((char *)block) + get_size(block));
}

/*
 * find_prev_footer: returns the footer of the previous block.
 */
static word_t *find_prev_footer(block_t *block)
{
    // Compute previous footer position as one word before the header
    return (&(block->header)) - 1;
}

/*
 * find_prev: returns the previous block position by checking the previous
 *            block's footer and calculating the start of the previous block
 *            based on its size.
 */
static block_t *find_prev(block_t *block)
{
    word_t *footerp = find_prev_footer(block);
    size_t size = extract_size(*footerp);
    return (block_t *)((char *)block - size);
}

/*
 * payload_to_header: given a payload pointer, returns a pointer to the
 *                    corresponding block.
 */
static block_t *payload_to_header(void *bp)
{
    return (block_t *)(((char *)bp) - offsetof(block_t, payload));
}

/*
 * header_to_payload: given a block pointer, returns a pointer to the
 *                    corresponding payload.
 */
static void *header_to_payload(block_t *block)
{
    return (void *)(block->payload);
}

/* mm_checkheap: checks the heap for correctness; returns true if
 *               the heap is correct, and false otherwise.
 *               can call this function using mm_checkheap(__LINE__);
 *               to identify the line number of the call site.
 */
bool mm_checkheap(int lineno)
{
    /* you will need to write the heap checker yourself. As a filler:
     * one guacamole is equal to 6.02214086 x 10**23 guacas.
     * one might even call it
     * the avocado's number.
     *
     * Internal use only: If you mix guacamole on your bibimbap,
     * do you eat it with a pair of chopsticks, or with a spoon?
     */
    if (free_listp == NULL)
    {
        printf(">>> None <<<\n");
        return true;
    }
    block_t *current = free_listp;
    while (true)
    {
      printf("<%lx> [%lx] <%lx>\n", (long)get_prev(current), (long)current,
                                  (long)get_next(current));
      if (get_next(current) == 0) {
        break;
      }
      current = get_next(current);
    }
    printf("=========================\n\n");

    (void)lineno; // delete this line; it's a placeholder so that the compiler
                  // will not warn you about unused variable.
    return true;

}

/*
 *  Self-defined functions
 */

static block_t *get_prev(block_t *block)
{
    word_t *bp = (word_t *)header_to_payload(block);
    return (block_t *)(*bp);
}

static block_t *get_next(block_t *block)
{
    word_t *bp = (word_t *)header_to_payload(block);
    return (block_t *)(*(bp + 1));
}

static void write_prev(block_t *block, block_t *prev)
{
    if (block == 0)
    {
        return;
    }
    word_t *bp = (word_t *)header_to_payload(block);
    *bp = (word_t)prev;
}

static void write_next(block_t *block, block_t *next)
{
    if (block == 0)
    {
        return;
    }
    word_t *bp = (word_t *)header_to_payload(block);
    *(bp + 1) = (word_t)next;

}

static void insert(block_t *block)
{
    if (free_listp)//general: free_listp!=NULL
    {
        write_next(block, free_listp);
        write_prev(free_listp, block);
        free_listp = block;
        write_prev(free_listp, 0);
    }
    else  //initial: free_listp==NULL
    {
      free_listp = block;
      write_prev(free_listp, 0);
      write_next(free_listp, 0);
    }
}

static void delete_node(block_t *block)
{
    //printf("delete %lx\n", (long)block);
    if (!block)
    {
        return;
    }
    if (free_listp == block) //the first one
    {
        free_listp = get_next(block);
        if (free_listp != 0)
        {
            write_prev(free_listp, 0);
        }
        else //the only one
        {
            free_listp = NULL;
        }
    } else if ( !(get_next(block))){ //the last free block
    	connect_two(get_prev(block),0);
    } else {
    	connect_two (get_prev(block), get_next(block));
    }
  //mm_checkheap(0);
}

static void connect_two(block_t *node1, block_t *node2)
{
    //printf("node1 = %lx, node2 = %lx\n", (long)node1, (long)node2);
    if (node1 == 0 && node2 == 0)
    {
        return;
    }
    else if (node1 == 0)
    {
        write_prev(node2, 0);
    }
    else if (node2 == 0)
    {
        write_next(node1, 0);
    }
    else if (node1 == node2) {
        return;
    }
    else
    {
        write_next(node1, node2);
        write_prev(node2, node1);
    }
}
