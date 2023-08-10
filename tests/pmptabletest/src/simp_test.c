#include "pmpt.h"

int page_alloc_num = 1; // already have root table
uint64_t test_base = TEST_BASE;

uint8_t test_page_perm[TEST_MAX_NUM];
uint64_t test_addr[TEST_MAX_NUM];
uint16_t test_num = 0;

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

void* alloc_super_page(int perm) {
  uint64_t offset = test_base & (SUPER_PAGE-1);
  uint64_t page_start = offset == 0 ? test_base : (test_base & (~(SUPER_PAGE-1))) + SUPER_PAGE;
  // add base
  test_base = page_start + SUPER_PAGE;

  // write perm
  uint64_t* root_pte = (uint64_t *)get_table_addr(page_start, 0);
  *root_pte = (perm&0x7) == 0 ? 0 : (perm << 1)+1;
  // printf("write root pte 0x%lx: %x\n", (uint64_t)root_pte, perm);

  page_alloc_num += 1;
  return (void *)(page_start);
}

/**
 * alloc test page
 * perm: cwxr
 * super_page: if need alloc super_page
 */
void* alloc_test_page(int perm, bool super_page) {
  void* addr = 0;
  if(super_page) {
    addr = alloc_super_page(perm); // super page size 32MB
  } else {
    addr = alloc_norma_page(perm); // normal page size 4kB
  }

  // asm volatile("sfence.vma");
  return addr;
}

/**
 * add simple tests, p < 32
 * perm = p&0xf; super_page = p >> 4;
 */
void add_simple_test(uint8_t p) {
  assert(p < 32);
  test_page_perm[test_num] = (p & 0b11) == 0b10 ? p & 0b11100 : p;
  test_addr[test_num] = (uint64_t)alloc_test_page(p&0xf, p>>4);
  init_instr_mem(test_addr[test_num]);

  test_num += 1;
}

/**
 * access test_addr, compare test_page_perm
 * idx: idx < 0: test all
 *      idx >= 0: only test idx
 */
void start_simple_tests(int idx, bool amo) {
  int len = idx<0 ? test_num : idx+1;
  int i = idx<0? 0 : idx;
  for (; i < len; i++) {
    // clean
    clean_current_perm();

    // printf("test addr: 0x%lx\n", test_addr[i]);
    if (amo) {
      pmp_amo_test(test_addr[i]);
    } else {
      pmp_rwx_test(test_addr[i]);
    }
    // compare
    if (get_current_perm() != (test_page_perm[i]&0x7)) {
      printf("wrong: idx: %d, addr: 0x%lx\n", i, test_addr[i]);
      printf("xwr: shuold be: 0x%x but: 0x%x\n", test_page_perm[i]&0x7, get_current_perm());
      uint64_t *root_addr = (uint64_t *)get_table_addr(test_addr[i], 0);
      printf("root pte addr: 0x%lx, data: 0x%lx\n", root_addr, *root_addr);
      if ((*root_addr & 0xf) == 1) {
        printf("hit leaf pte: 0x%lx\n", *(uint64_t *)get_table_addr(test_addr[i], 1));
      }
      _halt(1);
    }
  }
}
