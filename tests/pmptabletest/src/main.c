#include "pmpt.h"

void init_pmptable() {
  asm volatile("csrw pmpcfg2, %0" : : "r"((long)24<<(8*7))); 
  // enable_pmp_TOR(0, 0x0, FIRST_AREA_END, 0, PMP_R | PMP_W | PMP_X);

  // all allow
  // csr_set_num(PMPCFG_BASE, 0x1fUL << (0*8));

  //////////////////////////////////////////
  // init first area: 0x0 -> 0x90000000, by tor mode pmp table
  csr_set_num(PMPCFG_BASE, 0x2fUL << (2*8));  // set pmpcfg2 tor and table mode
  csr_write_num(PMPADDR_BASE + 1, 0x0 >> 2);
  csr_write_num(PMPADDR_BASE + 2, FIRST_AREA_END >> 2);
  csr_write_num(PMPADDR_BASE + 3, FIRST_PMPT_BASE >> 12);
  // set all root pte perm to 0x1111
  uint64_t *root = (uint64_t *)FIRST_PMPT_BASE;
  for (uint64_t i = 0; i < 512; i++) { // 0x480 times
    root[i] = 0xf;
  }

  //////////////////////////////////////////
  // init second area: 0x90000000 -> 0x400000000
  csr_set_num(PMPCFG_BASE, 0x28UL << (7*8));  // set pmpcfg7 napot and table mode
  csr_write_num(PMPADDR_BASE + 6, FIRST_AREA_END >> 2);
  csr_write_num(PMPADDR_BASE + 7, MAX_ADDR >> 2);   //tor: 2^(27+3) B, 0x90000000 -> 0xD0000000
  csr_write_num(PMPADDR_BASE + 8, SECONE_PMPT_BASE >> 12);

  // asm volatile("sfence.vma");
}

void init_cte() {
  _cte_init(simple_trap);
  irq_handler_reg(EXCEPTION_STORE_ACCESS_FAULT, &pmp_store_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_ACCESS_FAULT, &pmp_load_fault_handler);
  irq_handler_reg(EXCEPTION_INST_ACCESS_FAULT, &pmp_instr_fault_handler);
}

/**
 * mainargs:  s(Simple Test, include amo test)
 *            t(Simple Test with sv39, include amo test)
 *            r(Rand Test)
 *            a(Rand Test, very slow)
 * default: r
 */
int main(const char *args) {
  // set random perm
  srand(2);
  // printf("_bss: 0x%lx end: 0x%lx\n", _stack_top, end);

  if (args[0] == 's') {
    for (int i = 0; i < TEST_MAX_NUM; i++) {
      int p = (rand()*32)>>15;
      // printf("%x\n", p);
      add_simple_test(p);
    }
    asm volatile("sfence.vma");
    init_cte();

    printf("start simple test\n");
    start_simple_tests(-1, false);
    printf("rwx test pass\n");
    start_simple_tests(-1, true);
    printf("amo test pass\n");

  } else if (args[0] == 'a') {
    init_rand_test(-1, -1);
    asm volatile("sfence.vma");
    init_cte();

    printf("start rand test (large)\n");
    start_rand_test(-1);

  } else if (args[0] == 't') {
    for (int i = 0; i < TEST_MAX_NUM; i++) {
      // int p = (rand()*32)>>15;
      // printf("%x\n", p);
      add_simple_test(rand()&0xf);
      // add_simple_test(0);
    }
    sv39_init();
    asm volatile("sfence.vma");
    init_cte();

    printf("start simple test\n");
    start_simple_tests(-1, false);
    printf("rwx test pass\n");
    start_simple_tests(-1, true);
    printf("amo test pass\n");

  } else {
    init_rand_test(8, 15);
    asm volatile("sfence.vma");
    init_cte();

    printf("start rand test (fast)\n");
    start_rand_test(15);
  }

  printf("PASS!\n");
  return 0;
}