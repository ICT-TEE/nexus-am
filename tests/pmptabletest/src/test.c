#include "pmpt.h"

uint8_t test_page_perm[TEST_MAX_NUM];
uint64_t test_addr[TEST_MAX_NUM];
uint16_t test_num = 0;

uint64_t rand_test_point[RAND_TEST_POINT];

// add tests, p < 32, perm = p&0xf; super_page = p >> 4;
void add_simple_test(uint8_t p) {
  assert(p < 32);
  test_page_perm[test_num] = p;
  test_addr[test_num] = (uint64_t)alloc_test_page(p&0xf, p>>4);
  init_instr_mem(test_addr[test_num]);

  test_num += 1;
}

// access test_addr, compare test_page_perm
void start_simple_tests(int idx) {
  int len = idx<0 ? test_num : idx+1;
  int i = idx<0? 0 : idx;
  for (; i < len; i++) {
    // clean
    clean_current_perm();

    // printf("test addr: 0x%lx\n", test_addr[i]);
    pmp_rwx_test(test_addr[i]);
    // compare
    if (get_current_perm() != (test_page_perm[i]&0x7)) {
      printf("wrong: idx: %d, addr: 0x%lx\n", i, test_addr[i]);
      printf("xwr: shuold be: 0x%x but: 0x%x\n", test_page_perm[i]&0x7, get_current_perm());
      uint64_t *root_addr = (uint64_t *)get_table_addr(test_addr[i], 0);
      printf("root pte addr: 0x%lx, data: 0x%lx\n", root_addr, *root_addr);
      _halt(1);
    }
  }
}

uint64_t generate_rand_root_pte() {
  uint64_t ppn = TEST_PMPT_BASE + NORMA_PAGE;
  uint64_t rand_num = (rand() << 15) | rand();
  uint64_t offset = (rand_num * ((1<<9)*(RAND_PAGE_NUM-1)))>>30;
  ppn += offset << 3;
  // printf("ppn: 0x%llx\n", ppn);
  // 1/2 open leaf pte
  uint64_t pte = (rand() & 0x1) ? ppn >> (12-5) : ((ppn >> 12)<<5)|0x1;
  return pte;
}

uint64_t generate_rand_leaf_pte() {
  uint64_t pte = 0;
  for (int i=0; i<(64/15+1); i++) {
    uint64_t rand_num = rand();
    pte = pte | (rand_num << (i*15));
  }
  return pte;
}

void init_rand_test() {
  for (int i = 0; i < RAND_TEST_POINT; i++) {
    uint64_t rand_num = (rand()<<15)|rand();
    rand_test_point[i] = TEST_BASE + ((rand_num % 0x370000) << 12);
    printf("point: 0x%llx\n", rand_test_point[i]);
    init_instr_mem(rand_test_point[i]);
  }

  // set random mem
  uint64_t *root_base = (uint64_t *)TEST_PMPT_BASE;
  for (int i = 0; i < (1<<9); i++) {
    root_base[i] = generate_rand_root_pte();
    // printf("root pte data: 0x%llx\n", root_base[i]);
  }

  for (int i = 0; i < RAND_PAGE_NUM-1; i++) {
    uint64_t *leaf_base = (uint64_t *)(TEST_PMPT_BASE + NORMA_PAGE);
    for (int j = 0; j < (1<<9); j++) {
      leaf_base[j] = generate_rand_leaf_pte();
      // printf("leaf pte data: 0x%llx\n", leaf_base[j]);
    }
  }
}

void start_rand_test(int n) {
  for (int i = 0; i < n && i < RAND_TEST_POINT; i++) {
    // clean
    clean_current_perm();

    pmp_rwx_test(rand_test_point[i]);
    uint8_t perm = get_table_perm(rand_test_point[i]);

    if (get_current_perm() != (perm&0x7)) {
      printf("wrong: idx: %d, addr: 0x%lx\n", i, rand_test_point[i]);
      printf("xwr: shuold be: 0x%x but: 0x%x\n", perm&0x7, get_current_perm());

      uint64_t *root_addr = (uint64_t *)get_table_addr(rand_test_point[i], 0);
      printf("root pte addr: 0x%lx, data: 0x%lx\n", root_addr, *root_addr);
      _halt(1);
    }
  }
}
