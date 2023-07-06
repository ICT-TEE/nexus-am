#include "pmpt.h"

uint8_t test_page_perm[TEST_MAX_NUM];
uint64_t test_addr[TEST_MAX_NUM];
uint16_t test_num = 0;

bool r_fault = false;
bool w_fault = false;
bool x_fault = false;

volatile int result_blackhole = 0;
void pmp_rwx_test(uint64_t addr) __attribute__((noinline));

// add tests, p < 32, perm = p&0xf; super_page = p >> 4;
void add_tests(uint8_t p) {
  assert(p < 32);
  test_page_perm[test_num] = p;
  test_addr[test_num] = (uint64_t)alloc_test_page(p&0xf, p>>4);
  init_instr_mem(test_addr[test_num]);

  test_num += 1;
}

// access test_addr, compare test_page_perm
void start_tests(int idx) {
  int len = idx<0 ? test_num : idx+1;
  int i = idx<0? 0 : idx;
  for (; i < len; i++) {
    // clean
    r_fault = false;
    w_fault = false;
    x_fault = false;

    printf("test addr: 0x%lx\n", test_addr[i]);
    pmp_rwx_test(test_addr[i]);
    // compare
    if (r_fault == (test_page_perm[i] & 0x1)||
        w_fault == ((test_page_perm[i] >> 1) & 0x1)||
        x_fault == ((test_page_perm[i] >> 2) & 0x1)) {
      printf("wrong: idx: %d, addr: 0x%lx\n", i, test_addr[i]);
      printf("xwr: %d,%d,%d <> %d\n", !x_fault, !w_fault, !r_fault, test_page_perm[i]&0x7);
      uint64_t *root_addr = (uint64_t *)get_table_addr(test_addr[i], 0);
      printf("root pte addr: 0x%lx, data: 0x%lx\n", root_addr, *root_addr);
      _halt(1);
    }
  }
}

_Context *simple_trap(_Event ev, _Context *ctx) {
  switch(ev.event) {
    case _EVENT_IRQ_TIMER:
      printf("t"); break;
    case _EVENT_IRQ_IODEV:
      printf("d"); read_key(); break;
    case _EVENT_YIELD:
      printf("y"); break;
  }
  return ctx;
}

void init_instr_mem(uint64_t addr) {
  uint32_t nop[3] = {0x00010001, 0x00010001, 0x00008082};
  ((uint32_t*)addr)[0] = nop[0];
  ((uint32_t*)addr)[1] = nop[1];
  ((uint32_t*)addr)[2] = nop[2];
}

_Context* pmp_load_fault_handler(_Event* ev, _Context *c) {
  // printf("r fault, sepc %lx\n", c->sepc);
  r_fault = true;
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  return c;
}

_Context* pmp_store_fault_handler(_Event* ev, _Context *c) {
  // uint64_t stval;
  // asm volatile("csrr %0, stval":"=r"(stval));
  // printf("w fault, stval=0x%llx\n", stval);
  // printf("w fault, sepc %lx\n", c->sepc);
  // write ret
  // *(uint32_t *)stval = 0x00008082;
  w_fault = true;
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  return c;
}

_Context* pmp_instr_fault_handler(_Event* ev, _Context *c) {
  // uint64_t stval;
  // asm volatile("csrr %0, stval":"=r"(stval));
  // printf("x fault, stval=0x%llx\n", stval);
  // printf("x fault, sepc %lx\n", c->sepc);
  x_fault = true;
  // ret PC from a6
  c->sepc = c->gpr[16];
  return c;
}

// for test
void pmp_read_test(uint64_t addr) {
  //printf("start read test");
  volatile int *d = (int *)addr;
  result_blackhole = (*d);
}

void pmp_write_test(uint64_t addr) {
  //printf("start write test");
  volatile uint32_t *a = (uint32_t *)addr;
  *a = 0x00008082;    // write ret
}

void pmp_instr_test(uint64_t addr) {
  asm volatile(
    "jalr a6, 0(a0);"
  );
}

void pmp_rwx_test(uint64_t addr) {
  pmp_read_test(addr);
  pmp_write_test(addr);
  pmp_instr_test(addr);
}
