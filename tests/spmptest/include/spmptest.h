#ifndef __AMUNIT_H__
#define __AMUNIT_H__

#include <am.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include <csr.h>

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS)
#define TEST_MAX    0x100000000
#define TEST_BASE   0x80000000
#elif defined(__ARCH_RISCV64_XS_NHV3) || defined(__ARCH_RISCV64_XS_NHV3_FLASH)
#define TEST_MAX    0x10000000000
#define TEST_BASE   0x1000010000
#endif
#define INDEX_MASK  0x0000f000

void init_spmp();
void enable_spmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t s, uint8_t permission);
void enable_spmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool s, uint8_t permission);
void clean_spmp_all();

void init_spmp_handler();

// void spmp_write_test(uint64_t addr);
// void spmp_read_test(uint64_t addr);
// void spmp_instr_test(uint64_t addr);
void spmp_rwx_test(uint64_t addr) __attribute__((noinline));

void spmp_test_init_modeU();
void spmp_test_init_modeS();

void spmp_test_modeU();
void spmp_test_modeS();
void spmp_test_modeS_SUM();


#define MRET(MSTATUS, MEPC) asm volatile(   \
      "csrw mstatus, %0;" \
      "csrw mepc, %1;"  \
      " mret;"  \
      : : "r"(MSTATUS), "r"(MEPC));


#endif
