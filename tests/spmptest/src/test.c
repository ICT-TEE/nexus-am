#include <spmptest.h>


static uint64_t addrs[SPMP_COUNT];
// U mode Exception map, for S,R,W,X <- 0 to 15, 
static uint8_t modeU_priv[SPMP_COUNT] = { // 0xwr,
  0, 4, 1, 3, 1, 5, 3, 1, // switch 7,15
  0, 0, 4, 4, 0, 0, 0, 7
};

static uint8_t modeS_priv[SPMP_COUNT] = { // 0xwr,
  0, 0, 3, 3, 0, 0, 0, 0,
  0, 4, 4, 5, 1, 5, 3, 1
};

static uint8_t modeS_SUM_priv[SPMP_COUNT] = { // 0xwr,
  0, 0, 3, 3, 1, 1, 3, 3,
  0, 4, 4, 5, 1, 5, 3, 1
};
static uint8_t modeU_priv_PMP[SPMP_COUNT] = { // 0xwr,
  7, 6, 5, 1, 5, 4, 1, 5, // switch 7,15
  7, 7, 6, 6, 7, 7, 7, 0
};

static uint8_t modeS_priv_PMP[SPMP_COUNT] = { // 0xwr,
  7, 7, 1, 1, 7, 7, 7, 7,
  7, 6, 6, 4, 5, 4, 1, 0
};

static uint8_t modeS_SUM_priv_PMP[SPMP_COUNT] = { // 0xwr,
  7, 7, 1, 1, 5, 5, 1, 1,
  7, 6, 6, 4, 5, 4, 1, 0
};
extern uint8_t test_priv[SPMP_COUNT];
extern uint8_t test_priv_PMP[PMP_COUNT];

void init_instr_mem(uint64_t addr) {
  uint32_t nop[3] = {0x00010001, 0x00010001, 0x00080067};
  ((uint32_t*)addr)[0] = nop[0];
  ((uint32_t*)addr)[1] = nop[1];
  ((uint32_t*)addr)[2] = nop[2];
}
void pmp_test_init_modeU(){
  for (int i = 0; i < PMP_COUNT; i++) {
    test_priv_PMP[i] = 0;
  }
  clean_pmp_all();
  clean_spmp_all();
  enable_spmp(SPMP_COUNT-1, 0x0, 0x100000000, 0, 7);
  for (int i = 0; i < PMP_COUNT; i++) {
    
    addrs[i] = TEST_BASE + (i*0x1000);
    init_instr_mem(addrs[i]);
    if(i!=PMP_COUNT-1){
    enable_pmp(i, addrs[i], 0x1000, 0, modeU_priv[i]);
    }
  }
}
void pmp_test_init_modeS_sum0(){
  for (int i = 0; i < PMP_COUNT; i++) {
    test_priv_PMP[i] = 0;
  }
  clean_pmp_all();
  clean_spmp_all();
  //enable_spmp(SPMP_COUNT-1, 0x0, 0x100000000, 0, 7);
  for (int i = 0; i < PMP_COUNT; i++) {
    
    addrs[i] = TEST_BASE + (i*0x1000);
    init_instr_mem(addrs[i]);
    if(i!=PMP_COUNT-1){
    enable_pmp(i, addrs[i], 0x1000, 0, modeS_priv[i]);
    } 
  }
}
void pmp_test_init_modeS_sum1(){
  for (int i = 0; i < PMP_COUNT; i++) {
    test_priv_PMP[i] = 0;
  }
  clean_pmp_all();
  clean_spmp_all();
  //enable_spmp(SPMP_COUNT-1, 0x0, 0x100000000, 0, 7);
  for (int i = 0; i < PMP_COUNT; i++) {
    
    addrs[i] = TEST_BASE + (i*0x1000);
    init_instr_mem(addrs[i]);
    if(i!=PMP_COUNT-1){
    enable_pmp(i, addrs[i], 0x1000, 0, modeS_SUM_priv[i]);
    }
  }
}
void spmp_test_init_modeU() {
  for (int i = 0; i < SPMP_COUNT; i++) {
    test_priv[i] = 0;
  }
  clean_spmp_all();
  enable_spmp(7, TEST_BASE+0x7000, 0x1000, 1, 7);
  enable_spmp(SPMP_COUNT-1, 0x0, 0x100000000, 0, 7);

  for (int i = 0; i < SPMP_COUNT; i++) {
    uint8_t r = (i >> 2) & 1;
    uint8_t w = (i >> 1) & 1;
    uint8_t x = i & 1;

    addrs[i] = TEST_BASE + (i*0x1000);
    init_instr_mem(addrs[i]);

    if (i!=7 && i!=SPMP_COUNT-1) {
      enable_spmp(i, addrs[i], 0x1000, i >> 3, (x<<2)|(w<<1)|(r));
    }
  }
}

void spmp_test_init_modeS() {
  for (int i = 0; i < SPMP_COUNT; i++) {
    test_priv[i] = 0;
  }
  clean_spmp_all();
  for (int i = 0; i < SPMP_COUNT; i++) {
    uint8_t r = (i >> 2) & 1;
    uint8_t w = (i >> 1) & 1;
    uint8_t x = i & 1;

    addrs[i] = TEST_BASE + (i*0x1000);
    enable_spmp(i, addrs[i], 0x1000, i >> 3, (x<<2)|(w<<1)|(r));
    init_instr_mem(addrs[i]);
  }
}

void spmp_test_main(uint8_t* compare) {
  for (int i = 0; i < SPMP_COUNT; i++) {
    spmp_read_test( addrs[i]);
    spmp_write_test(addrs[i]);
    spmp_instr_test(addrs[i]);
  }
  bool wrong = 0;
  for (int i = 0; i < SPMP_COUNT; i++) {
    if (compare[i] != ((~test_priv[i])&7)) {
      printf("[SRWX] %d:\tshould be %d, but %d\n", i, compare[i], (~test_priv[i])&7);
      wrong = 1;
    }
  }

  if (wrong) {_halt(1);}
  printf("sPMP test pass\n");
  // _halt(0);
}
void pmp_test_main(uint8_t* compare) {
  for (int i = 0; i < SPMP_COUNT; i++) {
    spmp_read_test( addrs[i]);
    spmp_write_test(addrs[i]);
    spmp_instr_test(addrs[i]);
  }
  bool wrong = 0;
  for (int i = 0; i < SPMP_COUNT; i++) {
    if (compare[i] != ((test_priv_PMP[i])&7)) {
      printf("pmp[SRWX] %d:\tshould be %d, but %d\n", i, compare[i], (test_priv_PMP[i])&7);
      wrong = 1;
    }
  }

  if (wrong) {_halt(1);}
  printf("PMP test pass\n");
  // _halt(0);
}
void pmp_test_modeU() { pmp_test_main(modeU_priv_PMP); asm volatile("ecall;"); }
void pmp_test_modeS() { pmp_test_main(modeS_priv_PMP); asm volatile("ecall;"); }
void pmp_test_modeS_SUM() { pmp_test_main(modeS_SUM_priv_PMP); asm volatile("ecall;"); }

void spmp_test_modeU() { spmp_test_main(modeU_priv); asm volatile("ecall;"); }
void spmp_test_modeS() { spmp_test_main(modeS_priv); asm volatile("ecall;"); }
void spmp_test_modeS_SUM() { spmp_test_main(modeS_SUM_priv); _halt(0); }

/*
void spmp_test() {
  irq_handler_reg(EXCEPTION_STORE_ACCESS_FAULT, &spmp_store_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_ACCESS_FAULT, &spmp_load_fault_handler);
  irq_handler_reg(EXCEPTION_INSTR_PAGE_FAULT, &spmp_instr_fault_handler);
  printf("start spmp test\n");
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV32_NOOP) || defined(__ARCH_RISCV64_XS)
  spmp_instr_test(0xa0000000UL, true);
  spmp_read_test(0x90000040UL, true);
  // Case: store to address protected by spmp
  spmp_write_test(0x90000040UL, true);

  // Case: store to normal cacheable address
  spmp_write_test(0xa0000000UL, false);

  // Case: store to address protected by spmp tor
  spmp_write_test(0xb0000040UL, true);

  // Case: load from address protected by spmp
  spmp_read_test(0x90000040UL, true);

  // Case: load from address protected by spmp tor
  spmp_read_test(0xb0000040UL, true);

  // Case: store to address protected by spmp (use spmpcfg2)
  spmp_write_test(0xb0010000UL, true);

  // Case: lr from address protected by spmp
  spmp_load_should_fault = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "lr.d s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check(0);
  printf("line %d passed\n", __LINE__);

  // Case: sc to address protected by spmp
  spmp_store_should_fault = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "sc.d s5, s5, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check(0);
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by spmp
  spmp_store_should_fault = 1;
  asm volatile(
    "li s4, 0xb0000040;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check(0);
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by spmp (w,!r)
  spmp_store_should_fault = 1;
  asm volatile(
    "li s4, 0xb0008000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check(0);
  printf("line %d passed\n", __LINE__);

  // Case: amo to address protected by spmp (!w,r)
  spmp_store_should_fault = 1;
  asm volatile(
    "li s4, 0xb0004000;"
    "amoadd.d s5, s6, (s4);"
    :
    :
    :"s4","s5","s6"
  );
  result_check(0);
  printf("line %d passed\n", __LINE__);

#elif defined(__ARCH_RISCV64_XS_SOUTHLAKE) || defined(__ARCH_RISCV64_XS_SOUTHLAKE_FLASH)
  // TODO: update spmp test for southlake
  spmp_store_should_fault = 0;
  int *b = (int *)(0x2030000000UL);
  *b = 1; // should not trigger a fault
  result_check(0);

  spmp_store_should_fault = 1;
  volatile int *a = (int *)(0x2010000040UL);
  *a = 1; // should trigger a fault
  result_check(0);
#else
  // invalid arch
  printf("invalid arch\n");
  _halt(1);
#endif
}
*/