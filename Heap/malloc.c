// Katia Lopez
// Heap Assignment

#include <assert.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>

#define ALIGN4(s)         (((((s) - 1) >> 2) << 2) + 4)
#define BLOCK_DATA(b)      ((b) + 1)
#define BLOCK_HEADER(ptr)   ((struct _block *)(ptr) - 1)


/* Implement three additional heap management strategies:
Next Fit, Worst Fit, Best Fit (First Fit has already been implemented for you) */

/*
Counters exist in the code for tracking of the following events:
-Number of times the user calls malloc successfully
-Number of times the user calls free successfully
-Number of times we reuse an existing block
-Number of times we request a new block
-Number blocks in free list
-Total amount of memory requested
-Maximum size of the heap
*/
static int atexit_registered = 0;
static int num_mallocs       = 0;
static int num_frees         = 0;
static int num_reuses        = 0;
static int num_grows         = 0;
//static int num_blocks        = 0;
static int num_requested     = 0;
static int max_heap          = 0;

/*
 *  \brief printStatistics
 *
 *  \param none
 *
 *  Prints the heap statistics upon process exit.  Registered
 *  via atexit()
 *
 *  \return none
 */
void printStatistics( void )
{
  printf("\nheap management statistics\n");
  printf("mallocs:\t%d\n", num_mallocs );
  printf("frees:\t\t%d\n", num_frees );
  printf("reuses:\t\t%d\n", num_reuses );
  printf("grows:\t\t%d\n", num_grows );
  printf("blocks:\t\t%d\n", num_frees - num_reuses );
  //printf("blocks:\t\t%d\n", num_blocks );
  printf("requested:\t%d\n", num_requested );
  printf("max heap:\t%d\n", max_heap );
}

struct _block
{
   size_t  size;         /* Size of the allocated _block of memory in bytes */
   struct _block *prev;  /* Pointer to the previous _block of allocated memory   */
   struct _block *next;  /* Pointer to the next _block of allocated memory   */
   bool   free;          /* Is this _block free?                     */
   char   padding[3];
};


struct _block *heapList = NULL; /* Free list to track the _blocks available */
struct _block *last_reuse = NULL; // List to use for next fit


/*
 * \brief findFreeBlock
 *
 * \param last pointer to the linked list of free _blocks
 * \param size size of the _block needed in bytes
 *
 * \return a _block that fits the request or NULL if no free _block matches
 *
 * \Implement Next Fit
 * \Implement Best Fit
 * \Implement Worst Fit
 */
struct _block *findFreeBlock(struct _block **last, size_t size)
{
    struct _block *curr = heapList;

#if defined FIT && FIT == 0
   // FIRST FIT
   while (curr && !(curr->free && curr->size >= size))
   {
      *last = curr;
      curr  = curr->next;
   }
#endif


#if defined BEST && BEST == 0
   // BEST FIT
   // Pick the block that has the least space left over
   int minSpace = INT_MAX;

   // Need to store the block where it fits best
   struct _block *winner = NULL;

   // Traverse the entire list to find best fit
   while(curr)
   {
       if(curr->free && curr->size >= size)
       {
           if(curr->size - size < minSpace)
           {
               winner = curr;
               minSpace = curr->size - size;
           }
       }
       curr = curr->next;
   }
   curr = winner;
#endif


#if defined WORST && WORST == 0
   // WORST FIT
   // Pick the block that has the most space left over
   int maxSpace = 0;

   // Need to store the block where it fits worst
   struct _block *winner = NULL;

   // Keep track of same blocks so we pick first one in the list
   int sameBlock = 0;

   // Traverse the entire list to find worst fit
   while(curr != NULL)
   {
       if ((curr->free) && (curr->size >= size))
       {
           if(maxSpace==0 && sameBlock==0)
           {
               if (curr->size - size >= maxSpace)
                {
                    winner = curr;
                    maxSpace = curr->size - size;

                    //increment counter so if we find another block of same size, we don't pick it
                    sameBlock++;
                }
           }
           else
           {
               if (curr->size - size > maxSpace)
                {
                    winner = curr;
                    maxSpace = curr->size - size;
                }
           }
       }
       curr = curr->next;
   }
   curr = winner;
#endif


#if defined NEXT && NEXT == 0
   // NEXT FIT
   // Optimization of First Fit
   while (last_reuse && !(last_reuse->size >= size && last_reuse->free))
   {
       *last = last_reuse;
       last_reuse = last_reuse->next;
   }

   // if we reach end of list, reset back to beginning
   if (!last_reuse) last_reuse=curr;

   // run previous loop again
   while (last_reuse && !(last_reuse->size >= size && last_reuse->free))
   {
       *last = last_reuse;
       last_reuse = last_reuse->next;
   }

   curr = last_reuse;
#endif

   return curr;
}


/*
 * \brief growheap
 *
 * Given a requested size of memory, use sbrk() to dynamically
 * increase the data segment of the calling process.  Updates
 * the free list with the newly allocated memory.
 *
 * \param last tail of the free _block list
 * \param size size in bytes to request from the OS
 *
 * \return returns the newly allocated _block of NULL if failed
 */
struct _block *growHeap(struct _block *last, size_t size)
{
   /* Request more space from OS */
   struct _block *curr = (struct _block *)sbrk(0);
   struct _block *prev = (struct _block *)sbrk(sizeof(struct _block) + size);

   // keep track of total amount of memory requested
   num_requested += size;

   // max heap is the total size of heap which includes struct _blocks
   max_heap += sizeof(struct _block) + size;

   assert(curr == prev);

   /* OS allocation failed */
   if (curr == (struct _block *)-1)
   {
      return NULL;
   }

   /* Update heapList if not set */
   if (heapList == NULL)
   {
      heapList = curr;
   }

   /* Attach new _block to prev _block */
   if (last)
   {
      last->next = curr;
   }

   /* Update _block metadata */
   curr->size = size;
   curr->next = NULL;
   curr->free = false;

   return curr;
}


/*
 * \brief malloc
 *
 * finds a free _block of heap memory for the calling process.
 * if there is no free _block that satisfies the request then grows the
 * heap and returns a new _block
 *
 * \param size size of the requested memory in bytes
 *
 * \return returns the requested memory allocation to the calling process
 * or NULL if failed
 */
void *malloc(size_t size)
{

   if( atexit_registered == 0 )
   {
      atexit_registered = 1;
      atexit( printStatistics );
   }

   /* Align to multiple of 4 */
   size = ALIGN4(size);

   /* Handle 0 size */
   if (size == 0)
   {
      return NULL;
   }

   /* Look for free _block */
   struct _block *last = heapList;
   struct _block *next = findFreeBlock(&last, size);


   // If we find a free block, that means we will reuse a block
   if (next != NULL)
   {
       num_reuses++;
       num_requested += size; //don't grow heap but keep track of memory requested

       // decrement number of blocks in free list
       //num_blocks--;
   }

   /* Could not find free _block, so grow heap */
   if (next == NULL)
   {
      next = growHeap(last, size);
      num_grows++; //increment times we request a new block
   }

   /* Could not find free _block or grow heap, so just return NULL */
   if (next == NULL)
   {
      return NULL;
   }

   /* Mark _block as in use */
   next->free = false;

   // increment malloc counter
   num_mallocs++;

   /* Return data address associated with _block */
   return BLOCK_DATA(next);
}


// Implement calloc using malloc and memset
void *calloc(size_t nmemb, size_t size)
{
    void *ptr = malloc(nmemb*size);
    memset(ptr, 0, nmemb*size);
    return ptr;
}

// Implement realloc
void *realloc(void *ptr, size_t size)
{
    struct _block *curr = BLOCK_HEADER(ptr);
    void *newptr = malloc(size); //malloc the new pointer
    memcpy(newptr, ptr, curr->size);
    free(ptr); //free pointer
    return newptr;
}


/*
 * \brief free
 *
 * frees the memory _block pointed to by pointer. if the _block is adjacent
 * to another _block then coalesces (combines) them
 *
 * \param ptr the heap memory to free
 *
 * \return none
 */
void free(void *ptr)
{
   if (ptr == NULL)
   {
      return;
   }

   /* Make _block as free */
   struct _block *curr = BLOCK_HEADER(ptr);
   assert(curr->free == 0);
   curr->free = true;

   // increment free counter
   num_frees++;

   // increment number of blocks in free list
   //num_blocks++;
}

/* vim: set expandtab sts=3 sw=3 ts=6 ft=cpp: --------------------------------*/
