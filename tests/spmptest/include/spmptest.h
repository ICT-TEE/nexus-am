#ifndef __AMUNIT_H__
#define __AMUNIT_H__

#include <am.h>
#include <xsextra.h>
#include <klib.h>
#include <klib-macros.h>

#include <csr.h>
#include <xsextra.h>

#define TEST_BASE  0x80010000
#define INDEX_MASK 0x0000f000

void init_spmp();
void init_pmp();
void enable_pmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t lock, uint8_t permission);
void enable_pmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool lock, uint8_t permission);
void enable_spmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t s, uint8_t permission);
void enable_spmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool s, uint8_t permission);
void clean_spmp_all();
void clean_pmp_all();

void init_pmp_handler();
void init_spmp_handler();

void spmp_write_test(uint64_t addr);
void spmp_read_test(uint64_t addr);
void spmp_instr_test(uint64_t addr);

void pmp_test_init_modeU();
void pmp_test_init_modeS_sum0();
void pmp_test_init_modeS_sum1();

void spmp_test_init_modeU();
void spmp_test_init_modeS();

void pmp_test_modeU();
void pmp_test_modeS();
void pmp_test_modeS_SUM();

void spmp_test_modeU();
void spmp_test_modeS();
void spmp_test_modeS_SUM();

#endif
