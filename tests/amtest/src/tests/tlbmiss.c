#include <amtest.h>

#define RANGE_LEN(start, len) RANGE((start), (start + len))
#define N 8

static _Area segments[] = {      // Kernel memory mappings
  RANGE_LEN(0x80000000, 0x100000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
};

// static char *sv39_alloc_base = (char *)(0xc0000000UL);
// static uintptr_t sv39_alloced_size = 0;
void* sv39_pgalloc(size_t pg_size);

void sv39_pgfree(void *ptr);

void init_instr_mem(uint64_t addr) {
  uint32_t nop[3] = {0x00010001, 0x00010001, 0x00008082};
  ((uint32_t*)addr)[0] = nop[0];
  ((uint32_t*)addr)[1] = nop[1];
  ((uint32_t*)addr)[2] = nop[2];
}

void (*en)();

void tlbmiss() {
  _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 2);
  asm volatile("sfence.vma");
  uint64_t entry = 0x800afff0;
  // int n = 8*4*2;
  for (int i=0;i<N;i++) {
    init_instr_mem(entry+(0x2000*i));
  }
  printf("test start\n");
  for (int i=0;i<N;i++) {
    en = (void *)(entry+(0x2000*i));
    (*en)();
  }
    // asm volatile("jal ra, %0"::"i"(0x800000000));
}
