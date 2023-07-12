#ifndef __PMPT_H__
#define __PMPT_H__

#include <am.h>
#include <klib.h>
#include <csr.h>
#include <xsextra.h>

void csr_set_num(int csr_num, unsigned long val);
void csr_write_num(int csr_num, unsigned long val);

#define EXCEPTION_INST_ACCESS_FAULT 1
#define EXCEPTION_LOAD_ACCESS_FAULT 5
#define EXCEPTION_STORE_ACCESS_FAULT 7

#define NORMA_PAGE 0x1000UL
#define SUPER_PAGE 0x2000000UL

#define MAX_ADDR            0x400000000 // 16GB
#define FIRST_AREA_END      0x90000000
#define SECOND_AREA_END     MAX_ADDR
#define FIRST_PMPT_BASE     0x80010000  // ->0x80020000, max page number: 15
#define SECONE_PMPT_BASE    0x80020000  // ->0x90000000, max page number: 65504
// test in second area, use second pmpt base, only use front 670 page;
#define TEST_BASE           FIRST_AREA_END
#define TEST_PMPT_BASE      SECONE_PMPT_BASE

#define TEST_MAX_NUM            128

// rand addr, rand perm
#define RAND_PAGE_NUM           8
#define RAND_TEST_POINT         12

// init
inline int inst_is_compressed(uint64_t addr){
  uint8_t byte = *(uint8_t*)addr;
  return (byte & 0x3) != 0x3; 
}

_Context *simple_trap(_Event ev, _Context *ctx);
void init_instr_mem(uint64_t addr);
_Context* pmp_load_fault_handler(_Event* ev, _Context *c);
_Context* pmp_store_fault_handler(_Event* ev, _Context *c);
_Context* pmp_instr_fault_handler(_Event* ev, _Context *c);

// main
void* get_table_addr(uint64_t addr, int level);
void* alloc_test_page(int perm, bool super_page);
uint8_t get_table_perm(uint64_t addr);

// lib
uint8_t get_current_perm();
void clean_current_perm();
void pmp_rwx_test(uint64_t addr) __attribute__((noinline));

// simple test
void add_simple_test(uint8_t p);
void start_simple_tests(int idx);

// random test
void init_rand_test();
void start_rand_test(int n);

#endif
