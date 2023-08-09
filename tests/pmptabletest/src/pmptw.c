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

// get rwx permission by addr, addr must in second area
uint8_t get_table_perm(uint64_t addr) {
  if (addr < TEST_BASE || addr >= MAX_ADDR) {
    printf("wrong addr\n");
    return 0;
  }
  uint8_t perm = 0;

  uint64_t offset = addr - TEST_BASE;
  uint64_t off1 = (offset >> 25) & 0x1ff; // root offset
  uint64_t off0 = (offset >> 16) & 0x1ff; // leaf offset
  uint8_t page_index = (offset >> 12) & 0xf;  // page index

  uint64_t root_pte = *((uint64_t *)(TEST_PMPT_BASE + (off1 << 3)));
  // printf("root pte addr: 0x%lx, 0x%lx\n", TEST_PMPT_BASE + (off1 << 3), root_pte);
  // hit leaf pte
  if ((root_pte & 0xf) == 1) {
    uint8_t *leaf_pte = (uint8_t *)(((root_pte >> 5) << 12) + (off0 << 3));
    int idx = page_index/2;
    bool at_high = page_index%2;
    if (at_high) {
      perm = leaf_pte[idx] >> 4;
    } else {
      perm = leaf_pte[idx] & 0xf;
    }
    // printf("hit leaf pte: 0x%lx\n", (uint64_t)leaf_pte);
  // hit root pte
  } else if ((root_pte & 0x1) == 1) {
    perm = (root_pte >> 1) & 0xf;
    // printf("hit root pte\n");
  }
  perm = (perm & 0b11) == 0b10 ? perm & 0b1100 : perm;

  return perm;
}
