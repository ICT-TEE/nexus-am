#include "pmpt.h"

int page_alloc_num = 1; // already have root table
uint64_t test_base = TEST_BASE;

// normal page size 4kB
void* alloc_norma_page(int perm) {
  bool need_new_leaf_table = (test_base & (SUPER_PAGE-1)) == 0;
  uint64_t page_start = test_base;
  int page_index = (page_start >> 12) & 0xf;
  // add base
  test_base += NORMA_PAGE;

  if (need_new_leaf_table) {
    printf("alloc new leaf table\n");
    // use new page
    uint64_t ppn = TEST_PMPT_BASE + (page_alloc_num * NORMA_PAGE);
    assert(ppn < FIRST_AREA_END);
    page_alloc_num += 1;
    // write root pte
    uint64_t* root_pte = (uint64_t *)get_table_addr(page_start, 0);
    *root_pte = ((ppn >> 12) << 5) | 0x1;
  }

  // write perm
  uint8_t* leaf_pte = (uint8_t *)get_table_addr(page_start, 1);
  int idx = page_index/2;
  bool at_high = page_index%2;
  if (at_high) {
    leaf_pte[idx] = (leaf_pte[idx] & 0xf) | (perm << 4);
  } else {
    leaf_pte[idx] = (leaf_pte[idx] & 0xf0) | perm;
  }

  return (void *)(page_start);
}

// super page size 32MB
void* alloc_super_page(int perm) {
  uint64_t offset = test_base & (SUPER_PAGE-1);
  uint64_t page_start = offset == 0 ? test_base : (test_base & (~(SUPER_PAGE-1))) + SUPER_PAGE;
  // add base
  test_base = page_start + SUPER_PAGE;

  // write perm
  uint64_t* root_pte = (uint64_t *)get_table_addr(page_start, 0);
  *root_pte = (perm << 1)+1;
  // printf("write root pte 0x%lx: %x\n", (uint64_t)root_pte, perm);

  page_alloc_num += 1;
  return (void *)(page_start);
}

void* alloc_test_page(int perm, bool super_page) {
  void* addr = 0;
  if(super_page) {
    addr = alloc_super_page(perm);
  } else {
    addr = alloc_norma_page(perm);
  }

  // asm volatile("sfence.vma");
  return addr;
}
