#include "pmpt.h"

#define RANGE_LEN(start, len) RANGE((start), (start + len))

static _Area segments[] = {      // Kernel memory mappings
  RANGE_LEN(0x80000000, 0x100000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
  RANGE_LEN(FIRST_PMPT_BASE, 0x1000),
  RANGE_LEN(SECONE_PMPT_BASE, 0x10000),
  RANGE_LEN(TEST_BASE, 0x100000),    // test
};

static char *sv39_alloc_base = (char *)(SV39_PAGE_BASE);
static uintptr_t sv39_alloced_size = 0;
static void* sv39_pgalloc(size_t pg_size) {
  assert(pg_size == 0x1000);
  printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base + sv39_alloced_size);
  sv39_alloced_size += pg_size;
  return ret;
}

static void sv39_pgfree(void *ptr) {
  return ;
}

void sv39_init() {
  _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 5);
}
