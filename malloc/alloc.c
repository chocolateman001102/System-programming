/**
 * malloc
 * CS 341 - Fall 2022
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct _metadata_entry_t {
  unsigned int size;     // The size of the memory block.
  unsigned int free;  // 0 if the block is free; 1 if the block is used.
  void* next;
  void* prev;
  void* freenext;
  void* freeprev;
} metadata_t;

static metadata_t *Head = NULL;
static metadata_t *Tail = NULL;
static metadata_t *FreeHead = NULL;
static metadata_t *FreeTail = NULL;

metadata_t *blockspliting(metadata_t *temp, size_t size);

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    void *ptr = malloc(num * size);
    memset(ptr, 0, num*size);
    return ptr;
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    if (Head == NULL) {
        Head = sbrk(0);
        Tail = sbrk(0);
        metadata_t *meta_data = sbrk(sizeof(metadata_t));
        meta_data->size = size;
        meta_data->free = 0;
        meta_data->prev = NULL;
        meta_data->next = NULL;
        void *ptr = sbrk(size);
        return ptr;
    }
    metadata_t *temp = FreeHead;


    while (temp) {
            if (temp->size >= size) {
                if((temp->size - size) >= sizeof(metadata_t)){
                    temp = blockspliting(temp, size);
                    void* ptr0 =  ((void*)temp + sizeof(metadata_t));
                   return ptr0;
                }
                temp->free = 0;
                void* ptr = (void*)temp + sizeof(metadata_t);
                if(temp != FreeHead && temp!= FreeTail) {

                    ((metadata_t*)(temp -> freenext)) -> freeprev = temp->freeprev;

                    ((metadata_t*)(temp-> freeprev)) -> freenext = temp->freenext;

                    temp -> freenext = NULL;
                    temp -> freeprev = NULL;
                } else if (temp ->freeprev == NULL) {
                    if(temp ->freenext != NULL) {
                        FreeHead = temp->freenext;
                        ((metadata_t*)temp->freenext)->freeprev = NULL;
                        temp -> freeprev = NULL;
                        temp -> freenext = NULL;
                    } else {
                        FreeHead = NULL;
                        FreeTail = NULL;
                        temp -> freeprev = NULL;
                        temp -> freenext = NULL;
                    }
                } else if(temp->freenext == NULL){
                    if(temp -> freeprev != NULL) {
                        FreeTail = temp ->freeprev;
                        ((metadata_t*)(temp->freeprev)) -> freenext = NULL;
                        temp -> freeprev = NULL;
                        temp -> freenext = NULL;
                    }else {
                        FreeHead = NULL;
                        FreeTail = NULL;
                        temp -> freeprev = NULL;
                        temp -> freenext = NULL;
                    }
                }
                return ptr;
                }
        temp = temp->freenext;
    }
    metadata_t *meta_data = sbrk(sizeof(metadata_t));
    meta_data->size = size;
    meta_data->free = 0;
    meta_data->prev = Tail;
    meta_data->next = NULL;
    void *ptr = sbrk(size);
    Tail->next = meta_data;
    Tail = meta_data;
    return ptr;
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    if (ptr == NULL) {
        return;
    }
    metadata_t *meta_data = ptr - sizeof(metadata_t);
    if (meta_data->free == 0) {
        meta_data->free = 1;
    if (FreeHead == NULL) {
        FreeHead = meta_data;
        FreeTail = meta_data;
        meta_data->freenext = NULL;
        meta_data->freeprev = NULL;
    }else {
        FreeTail-> freenext = meta_data;
        meta_data -> freeprev = FreeTail;
        meta_data ->freenext = NULL;
        FreeTail = meta_data;
    }
    }
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    if (ptr == NULL) {
      return malloc(size);
    }
    if (size == 0) {
      free(ptr);
      return NULL;
    }
    metadata_t *meta_data = ptr - sizeof(metadata_t);
    if (meta_data->size >= size) {
      return ptr;
    }
    void *new_block = malloc(size);
    memcpy(new_block, ptr, meta_data->size);
    free(ptr);
    return new_block;
}

metadata_t *blockspliting(metadata_t *temp, size_t size) {

    metadata_t *new_entry = (void*)temp + sizeof(metadata_t) + size;
    new_entry->free = 1;
    new_entry->size = temp->size - size - sizeof(metadata_t);
    new_entry->next = temp;


    if(temp->freeprev) {
        ((metadata_t*)temp->freeprev)->freenext = new_entry;
    } else {
        FreeHead = new_entry;
    } if(temp->freenext) {
        ((metadata_t*)temp->freenext)->freeprev = new_entry;
    } else {
        FreeTail = new_entry;
    }
    new_entry->freeprev = temp->freeprev;
    new_entry->freenext = temp->freenext;
    temp->freeprev = NULL;
    temp->freenext = NULL;    
    temp->free = 0;


    if (temp->prev) {
      ((metadata_t*)temp->prev)->next = new_entry;
    } else {
      Head = new_entry;
    }
    new_entry->prev = temp->prev;
    temp->size = size;
    temp->prev = new_entry;

    return temp;
}