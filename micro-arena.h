///////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// micro-arena.h
// =============
//
// Header only memory allocator in C99 (first fit). It implements
// the common memory function from stdlib.h:
//
//       void *micro_arena_malloc(MicroArena *ma, size_t size);
//       void micro_arena_free(MicroArena *ma, void * ptr);
//       void *micro_arena_calloc(MicroArena *ma, size_t nmemb, size_t size);
//       void *micro_arena_realloc(MicroArena *ma, void * ptr, size_t size);
//       void *micro_arena_reallocarray(MicroArena *ma, void * ptr, size_t nmemb, size_t size);
//
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o
//
//
// Example
// -------
//
//
//     MicroArena ma;
//     micro_arena_init(&ma);
//
//     int array_len = 10;
//     int* mem = micro_arena_malloc(&ma, sizeof(int) * array_len);
//     assert(mem != NULL);  
//
//     micro_arena_free(&ma, mem);
//
//
// Usage
// -----
//
// Do this:
//
//   #define MICRO_ARENA_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define MICRO_ARENA_IMPLEMENTATION
//   #include "micro-arena.h"
//
// You can tune the library by #defining certain values. See the
// "Config" comments under "Configuration" below.
//
//
// Code
// ----
//
// The official git repository of micro-arena.h is hosted at:
//
//     https://github.com/San7o/micro-arena.h
//
// This is part of a bigger collection of header-only C99 libraries
// called "micro-headers", contributions are welcome:
//
//     https://github.com/San7o/micro-headers
//

#ifndef MICRO_ARENA
#define MICRO_ARENA

#define MICRO_ARENA_MAJOR 0
#define MICRO_ARENA_MINOR 1
#define MICRO_ARENA_VERSION \
  ((MICRO_ARENA_MAJOR << 8) | MICRO_ARENA_MINOR)

#ifdef __cplusplus
extern "C" {
#endif

//
// Configuration
//

// Config: Prefix for all functions
// For function inlining, set this to `static inline` and then define
// the implementation in all the files
#ifndef MICRO_ARENA_DEF
  #define MICRO_ARENA_DEF extern
#endif

// Config: Size of arena buffer allocated on the stack.
#define MICRO_ARENA_STACK_MEM_SIZE 4096

// Config: Include an example program, see the end of the header
// #define MICRO_ARENA_EXAMPLE_MAIN

// Config: Maximum number of pointers. A limit is required since
//         we need to keep track of what pointers were allocated and
//         what parts of memory are free.
#ifndef MICRO_ARENA_MAX_NUM_CHUNKS
  #define MICRO_ARENA_MAX_NUM_CHUNKS 1024
#endif

// Config: Include debug functions
// #define MICRO_ARENA_DEBUG

//
// Types
//

#include <stddef.h>
#include <stdbool.h>

typedef struct {
  char* start;
  size_t size;
} MicroArenaChunk;

typedef struct {
  MicroArenaChunk chunks[MICRO_ARENA_MAX_NUM_CHUNKS];
  size_t len;
} MicroArenaChunkList;

typedef struct {
  char mem[MICRO_ARENA_STACK_MEM_SIZE];
  MicroArenaChunkList free_chunks;
  MicroArenaChunkList used_chunks;
} MicroArena;

//
// Function declarations
//

// O(1)
MICRO_ARENA_DEF void micro_arena_init(MicroArena *ma);
// First fit. O(ma->free_chunks.len)
MICRO_ARENA_DEF void *micro_arena_malloc(MicroArena *ma, size_t size);
// O(max(ma->free_chunks.len, ma->used_chunks.len)
MICRO_ARENA_DEF void micro_arena_free(MicroArena *ma, void *ptr);
MICRO_ARENA_DEF void *micro_arena_calloc(MicroArena *ma, size_t nmemb, size_t size);
MICRO_ARENA_DEF void *micro_arena_realloc(MicroArena *ma, void *ptr, size_t size);
MICRO_ARENA_DEF void *micro_arena_reallocarray(MicroArena *ma, void *ptr,
                                               size_t nmemb, size_t size);

// O(1)
MICRO_ARENA_DEF MicroArenaChunk*
micro_arena_chunk_list_add(MicroArenaChunkList *chunk_list,
                           void* start, size_t size);
// O(chunk_list->len) = O(MICRO_ARENA_MAX_NUM_CHUNKS)
// May invalidate previous pointers to its items
MICRO_ARENA_DEF void
micro_arena_chunk_list_remove(MicroArenaChunkList *chunk_list,
                              void* start);
MICRO_ARENA_DEF MicroArenaChunk*
micro_arena_chunk_list_get(MicroArenaChunkList *chunk_list,
                           void* start);
// O(1)
MICRO_ARENA_DEF void
micro_arena_chunk_list_reset(MicroArenaChunkList *chunk_list);

#ifdef MICRO_ARENA_DEBUG

MICRO_ARENA_DEF void
micro_arena_debug_print(MicroArena *ma);
  
MICRO_ARENA_DEF void
micro_arena_debug_print_chunk_list(MicroArenaChunkList *chunk_list);

#endif // MICRO_ARENA_DEBUG
  
MICRO_ARENA_DEF int micro_arena_major(void);
MICRO_ARENA_DEF int micro_arena_minor(void);
MICRO_ARENA_DEF int micro_arena_version(void);
  
//
// Implementation
//
  
#ifdef MICRO_ARENA_IMPLEMENTATION

#ifdef MICRO_ARENA_DEBUG
#include <stdio.h>
#endif

MICRO_ARENA_DEF void micro_arena_init(MicroArena *ma)
{
  if (!ma)
    return;
  micro_arena_chunk_list_reset(&ma->free_chunks);
  micro_arena_chunk_list_reset(&ma->used_chunks);
  micro_arena_chunk_list_add(&ma->free_chunks, &ma->mem,
                             MICRO_ARENA_STACK_MEM_SIZE);
  return;
}

MICRO_ARENA_DEF void *micro_arena_malloc(MicroArena *ma, size_t size)
{
  #ifdef MICRO_ARENA_DEBUG
  printf("DEBUG: micro_arena_malloc: called with size %ld\n", size);
  #endif

  if (!ma)
    return NULL;

  for (size_t i = 0; i < ma->free_chunks.len; ++i)
  {
    if (ma->free_chunks.chunks[i].size < size)
      continue;

    MicroArenaChunk *used_chunk =
      micro_arena_chunk_list_add(&ma->used_chunks,
                                 ma->free_chunks.chunks[i].start,
                                 size);
    if (!used_chunk)
      return NULL;
    ma->free_chunks.chunks[i].start = ma->free_chunks.chunks[i].start + size;
    ma->free_chunks.chunks[i].size = ma->free_chunks.chunks[i].size - size;
    
    return (void*)used_chunk->start;
  }
  
  return NULL;
}

MICRO_ARENA_DEF void micro_arena_free(MicroArena *ma, void *ptr)
{
  #ifdef MICRO_ARENA_DEBUG
  printf("DEBUG: micro_arena_free: called with ptr %p\n", ptr);
  #endif

  if (!ma)
    return;
  
  MicroArenaChunk *used_chunk =
    micro_arena_chunk_list_get(&ma->used_chunks, ptr);
  if (!used_chunk)
    return;

  #ifdef MICRO_ARENA_DEBUG
  printf("DEBUG: micro_arena_free: used chunk start = %p, size = %ld\n",
         used_chunk->start,
         used_chunk->size);
  #endif

  MicroArenaChunk *free_chunk_after = NULL;
  MicroArenaChunk *free_chunk_before = NULL;
  for (size_t i = 0; i < ma->free_chunks.len; ++i)
  {
    if (ma->free_chunks.chunks[i].start ==
        used_chunk->start + used_chunk->size)
      free_chunk_after = &ma->free_chunks.chunks[i];
    if (used_chunk->start ==
        ma->free_chunks.chunks[i].start + ma->free_chunks.chunks[i].size)
      free_chunk_before = &ma->free_chunks.chunks[i];
  }

  if (free_chunk_before && free_chunk_after)
  {
    #ifdef MICRO_ARENA_DEBUG
    printf("DEBUG: micro_arena_free: free chunks were found before and after ptr\n");
    #endif
    free_chunk_before->size += used_chunk->size + free_chunk_after->size;
    micro_arena_chunk_list_remove(&ma->used_chunks, used_chunk->start);
    micro_arena_chunk_list_remove(&ma->free_chunks, free_chunk_before->start);
    return;
  }
  if (free_chunk_before)
  {
    #ifdef MICRO_ARENA_DEBUG
    printf("DEBUG: micro_arena_free: free chunks were found before ptr\n");
    #endif
    free_chunk_before->size += used_chunk->size;
    micro_arena_chunk_list_remove(&ma->used_chunks, used_chunk->start);
    return;
  }
  if (free_chunk_after)
  {
    #ifdef MICRO_ARENA_DEBUG
    printf("DEBUG: micro_arena_free: free chunks were found after ptr\n");
    #endif
    free_chunk_after->start = used_chunk->start;
    free_chunk_after->size += used_chunk->size;
    micro_arena_chunk_list_remove(&ma->used_chunks, used_chunk->start);
    return;
  }

  #ifdef MICRO_ARENA_DEBUG
  printf("DEBUG: micro_arena_free: no free chunks found before or after prt\n");
  #endif

  micro_arena_chunk_list_add(&ma->free_chunks,
                             used_chunk->start,
                             used_chunk->size);
  micro_arena_chunk_list_remove(&ma->used_chunks, used_chunk->start);
  return;
}

MICRO_ARENA_DEF void *micro_arena_calloc(MicroArena *ma, size_t nmemb, size_t size)
{
  if (!ma)
    return NULL;

  char* mem = micro_arena_malloc(ma, size * nmemb);
  if (!mem)
    return NULL;

  for (size_t i = 0; i < nmemb * size; ++i)
    mem[i] = 0;

  return mem;
}

MICRO_ARENA_DEF void *micro_arena_realloc(MicroArena *ma, void *ptr, size_t size)
{
  if (!ma)
    return NULL;

  if (ptr == NULL)
    return micro_arena_malloc(ma, size);

  MicroArenaChunk *used_chunk =
    micro_arena_chunk_list_get(&ma->used_chunks, ptr);
  if (!used_chunk)
    return NULL;
  
  char* mem = micro_arena_malloc(ma, size);
  if (!mem)
    return NULL;

  size_t min_size = (used_chunk->size < size) ? used_chunk->size : size;
  for (size_t i = 0; i < min_size; ++i)
    mem[i] = ((char*)ptr)[i];

  micro_arena_free(ma, ptr);
  return mem;
}

MICRO_ARENA_DEF void *micro_arena_reallocarray(MicroArena *ma, void *ptr,
                                               size_t nmemb, size_t size)
{
  return micro_arena_realloc(ma, ptr, nmemb * size);
}

MICRO_ARENA_DEF MicroArenaChunk*
micro_arena_chunk_list_add(MicroArenaChunkList *chunk_list,
                           void* start, size_t size)
{
  if (!chunk_list || chunk_list->len + 1 >= MICRO_ARENA_MAX_NUM_CHUNKS)
    return NULL;

  chunk_list->chunks[chunk_list->len] = (MicroArenaChunk){
    .start = start,
    .size = size,
  };
  chunk_list->len++;
  return &chunk_list->chunks[chunk_list->len - 1];
}

MICRO_ARENA_DEF void
micro_arena_chunk_list_remove(MicroArenaChunkList *chunk_list,
                              void* start)
{
  if (!chunk_list)
    return;
  
  size_t i = 0;
  while (i < chunk_list->len)
  {
    if (chunk_list->chunks[i].start == start)
      break;
    ++i;
  }
  if (i >= chunk_list->len)
    return;
  
  for (i = i+1; i < chunk_list->len; ++i)
  {
    chunk_list->chunks[i-1] = chunk_list->chunks[i];
  }
  chunk_list->len--;
  return;
}

MICRO_ARENA_DEF MicroArenaChunk*
micro_arena_chunk_list_get(MicroArenaChunkList *chunk_list,
                           void* start)
{
  if (!chunk_list)
    return NULL;
  
  for (size_t i = 0; i < chunk_list->len; ++i)
    if (chunk_list->chunks[i].start == start)
      return &chunk_list->chunks[i];

  return NULL;
}

  
MICRO_ARENA_DEF void
micro_arena_chunk_list_reset(MicroArenaChunkList *chunk_list)
{
  if (!chunk_list)
    return;
  chunk_list->len = 0;
  return;
}

#ifdef MICRO_ARENA_DEBUG

MICRO_ARENA_DEF void
micro_arena_debug_print(MicroArena *ma)
{
  printf("// MicroArena debug -----------------------------------------//\n");
  if (!ma)
  {
    printf("MicroArena is NULL\n");
    return;
  }
  
  printf("Free ");
  micro_arena_debug_print_chunk_list(&ma->free_chunks);
  printf("Used ");
  micro_arena_debug_print_chunk_list(&ma->used_chunks);

  printf("// ----------------------------------------------------------//\n");
  return;
}

MICRO_ARENA_DEF void
micro_arena_debug_print_chunk_list(MicroArenaChunkList* chunk_list)
{
  if (!chunk_list)
  {
    printf("ChunkList is NULL\n");
    return;
  }

  printf("Chunk list len: %ld\n", chunk_list->len);
  for (size_t i = 0; i < chunk_list->len; ++i)
    printf("- start = %p, size = %ld\n",
           chunk_list->chunks[i].start,
           chunk_list->chunks[i].size);
}

#else

#define micro_arena_debug_print(...)
#define micro_arena_debug_print_chunk_list(...)

#endif // MICRO_ARENA_DEBUG

MICRO_ARENA_DEF int micro_arena_major(void)
{
  return MICRO_ARENA_MAJOR;
}

MICRO_ARENA_DEF int micro_arena_minor(void)
{
  return MICRO_ARENA_MINOR;
}

MICRO_ARENA_DEF int micro_arena_version(void)
{
  return MICRO_ARENA_VERSION;
}

#endif // MICRO_ARENA_IMPLEMENTATION

//
// Examples
//
  
#ifdef MICRO_ARENA_EXAMPLE_MAIN

// #define MICRO_ARENA_IMPLEMENTATION
// #include "micro-arena.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
  MicroArena ma;
  micro_arena_init(&ma);

  int array_len = 10;
  int* mem = micro_arena_malloc(&ma, sizeof(int) * array_len);
  assert(mem != NULL);  

  micro_arena_free(&ma, mem);
  return 0;
}
  
#endif // MICRO_ARENA_EXAMPLE_MAIN
  
#ifdef __cplusplus
}
#endif

#endif // MICRO_ARENA
