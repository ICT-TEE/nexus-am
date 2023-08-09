#include "pmpt.h"

bool r_fault = false;
bool w_fault = false;
bool x_fault = false;

volatile int result_blackhole = 0;

uint8_t get_current_perm() {
    uint8_t r = !r_fault;
    uint8_t w = !w_fault;
    uint8_t x = !x_fault;
    return (x<<2)|(w<<1)|r;
}

void clean_current_perm() {
    r_fault = false;
    w_fault = false;
    x_fault = false;
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
  *((uint32_t*)addr) = 0x00080067;
  // uint32_t nop[3] = {0x00010001, 0x00010001, 0x00008082};
  // ((uint32_t*)addr)[0] = nop[0];
  // ((uint32_t*)addr)[1] = nop[1];
  // ((uint32_t*)addr)[2] = nop[2];
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
inline void pmp_read_test(uint64_t addr) {
  //printf("start read test");
  volatile int *d = (int *)addr;
  result_blackhole = (*d);
}

inline void pmp_write_test(uint64_t addr) {
  //printf("start write test");
  volatile uint32_t *a = (uint32_t *)addr;
  *a = 0x00080067;    // write ret
}

inline void pmp_instr_test(uint64_t addr) {
  asm volatile(
    "jalr a6, 0(a0);"
    :::"a6"
  );
}

inline void pmp_amo_lr_test(uint64_t addr) {
  asm volatile(
    "lr.d s5, (a0);"
    :::"s5","s6"
  );
}

inline void pmp_amo_sc_test(uint64_t addr) {
  asm volatile(
    "li s5, 0x00080067;"
    "sc.d s5, s5, (a0);"
    :::"s4","s5","s6"
  );
}

inline void pmp_amo_write_test(uint64_t addr) {
  asm volatile(
    "li s6, 0;"
    "amoadd.d s5, s6, (a0);"
    :::"s4","s5","s6"
  );
}

void pmp_rwx_test(uint64_t addr) {
  pmp_read_test(addr);
  pmp_write_test(addr);
  pmp_instr_test(addr);
}

void pmp_amo_test(uint64_t addr) {
  // pmp_read_test(addr);
  pmp_amo_lr_test(addr);
  pmp_amo_sc_test(addr);
  // pmp_amo_write_test(addr);
  pmp_instr_test(addr);
}