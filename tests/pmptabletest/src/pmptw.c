#include "pmpt.h"

// get table addr by current addr and level
void* get_table_addr(uint64_t addr, int level) {
  assert(level == 0 || level == 1);
  assert(addr >= TEST_BASE);

  uint64_t offset = addr - TEST_BASE;
  uint64_t ta = 0;
  uint64_t root_pte_addr = TEST_PMPT_BASE + (((offset >> 25) & 0x1ff) << 3);
  if (level == 0) {
    ta = root_pte_addr;
  } else {
    uint64_t root_pte = *((uint64_t *)root_pte_addr);
    // must has leaf pte
    // printf("root pte: 0x%lx\n", root_pte);
    assert((root_pte & 0xf) == 1);
    ta = (((root_pte >> 5) << 12) + (((offset >> 16) & 0x1ff) << 3));
  }
  // printf("get_table_addr: 0x%lx\n", ta);

  return (void *)ta;
}

void check(char type) {
  printf("error: %c\n", type);
  return;
}
