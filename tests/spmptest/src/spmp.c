#include <spmptest.h>

#define __ARCH_RISCV64_XS 1

#define EXCEPTION_U_ECALL 8
#define EXCEPTION_S_ECALL 9
#define EXCEPTION_INSTR_SPMP_FAULT 16
#define EXCEPTION_LOAD_SPMP_FAULT 17
#define EXCEPTION_STORE_SPMP_FAULT 18

inline int inst_is_compressed(uint64_t addr){
  uint8_t byte = *(uint8_t*)addr;
  return (byte & 0x3) != 0x3; 
}

volatile int result_blackhole = 0;

uint8_t test_priv[SPMP_COUNT];

void record(char type) {
  uint64_t mtval;
  asm volatile("csrr %0, mtval":"=r"(mtval));
  // printf("[%c] fault, mtval=0x%llx\n", type, mtval);
  uint8_t index = (mtval & INDEX_MASK) >> 12;

  uint8_t tmp = test_priv[index];
  switch (type) {
    case 'r': tmp += 1; break;
    case 'w': tmp += 2; break;
    case 'x': tmp += 4; break;
  }
  test_priv[index] = tmp;
}

_Context* spmp_load_fault_handler(_Event* ev, _Context *c) {
  record('r');
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  return c;
}

_Context* spmp_store_fault_handler(_Event* ev, _Context *c) {
  record('w');
  // skip the inst that triggered the exception
  c->sepc = inst_is_compressed(c->sepc) ? c->sepc + 2: c->sepc + 4; 
  return c;
}

_Context* spmp_instr_fault_handler(_Event* ev, _Context *c) {
  record('x');
  // ret PC from a6
  c->sepc = c->gpr[16];
  return c;
}

void test_entry();

_Context* s2m(_Event* ev, _Context *c) {
  printf("to M mode\n");
  test_entry();
  c->sepc = 0;
  return c;
}

void init_spmp_handler() {
  irq_handler_reg(EXCEPTION_STORE_SPMP_FAULT, &spmp_store_fault_handler);
  irq_handler_reg(EXCEPTION_LOAD_SPMP_FAULT, &spmp_load_fault_handler);
  irq_handler_reg(EXCEPTION_INSTR_SPMP_FAULT, &spmp_instr_fault_handler);
  irq_handler_reg(EXCEPTION_U_ECALL, &s2m);
  irq_handler_reg(EXCEPTION_S_ECALL, &s2m);
}

// for test
void spmp_read_test(uint64_t addr) {
  volatile int *d = (int *)addr;
  result_blackhole = (*d);
}

void spmp_write_test(uint64_t addr) {
  volatile uint32_t *a = (uint32_t *)addr;
  *a = 0x00010001;    // write NOP
}

void spmp_instr_test(uint64_t addr) {
  asm volatile(
    "jalr a6, 0(a0);"
  );
  // asm volatile(
  //   "nop;"
  //   "nop;"
  //   "nop;"
  //   "ret"
  // );
}
