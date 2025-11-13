// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define MICRO_ARENA_MULTITHREADED
#define MICRO_ARENA_DEBUG
#define MICRO_ARENA_IMPLEMENTATION
#include "micro-arena.h"

#include <assert.h>
#include <stdio.h>

int main(void)
{
  MicroArena ma;
  micro_arena_init(&ma);
  micro_arena_debug_print(&ma);
  assert(ma.free_chunks.len == 1);
  assert(ma.used_chunks.len == 0);

  int array_len = 10;
  int* mem1 = micro_arena_malloc(&ma, sizeof(int) * array_len);
  assert(mem1 != NULL);
  assert(ma.free_chunks.len == 1);
  assert(ma.used_chunks.len == 1);
  
  char* mem2 = micro_arena_malloc(&ma, sizeof(char) * array_len);
  assert(mem2 != NULL);
  assert(ma.free_chunks.len == 1);
  assert(ma.used_chunks.len == 2);
  
  void* mem3 = micro_arena_malloc(&ma, 69);
  assert(mem3 != NULL);
  assert(ma.free_chunks.len == 1);
  assert(ma.used_chunks.len == 3);
  
  micro_arena_debug_print(&ma);
  
  micro_arena_free(&ma, mem1);
  assert(ma.free_chunks.len == 2);
  assert(ma.used_chunks.len == 2);

  micro_arena_free(&ma, mem3);
  assert(ma.free_chunks.len == 2);
  assert(ma.used_chunks.len == 1);
  

  micro_arena_free(&ma, mem2);
  assert(ma.free_chunks.len == 1);
  assert(ma.used_chunks.len == 0);
  
  micro_arena_debug_print(&ma);
  
  return 0;
}
