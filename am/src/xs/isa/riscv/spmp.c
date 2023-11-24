#include <xs.h>
#include <csr.h>
#include <klib.h>

// use pmp for spmp

unsigned long csr_read_num(int csr_num);
void csr_write_num(int csr_num, unsigned long val);
void csr_set_num(int csr_num, unsigned long val);
void csr_clear_num(int csr_num, unsigned long val);

void init_spmp() {
  // set all addr registers to disable possible access fault
  for (int i = 0; i < SPMP_COUNT; i++) {
    csr_write_num(SPMPADDR_BASE + i, -1L);
  }
  // set PMP to access all memory in S-mode
  // asm volatile("csrw pmpaddr8, %0" : : "r"(-1));
  // the last pmp pair is used to enable all access (in current case is pmp15)
  asm volatile("csrw 0x1a2, %0" : : "r"((long)0x98<<(8*7))); 

  asm volatile("sfence.vma");
}

void enable_spmp(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, uint8_t s, uint8_t permission) {
  // by default using NAPOT
  assert((pmp_size & (pmp_size - 1)) == 0); // must be power of 2
  assert(pmp_reg < SPMP_COUNT);
  if (pmp_size != 4) {
    // adjust pmp addr according to pmp size
    uintptr_t append = (pmp_size >> 1) - 1;
    pmp_addr |= append;
  }
  csr_write_num(SPMPADDR_BASE + pmp_reg, pmp_addr >> 2);

  uintptr_t set_content = permission | (s << 7) | (pmp_size == 4 ? 2 : 3) << 3;
  uintptr_t cfg_offset = pmp_reg > 7 ? 2 : 0;
  uintptr_t cfg_shift = pmp_reg & 0x7;
  // printf("addr %llx, cfg offset %d, shift %d, set_content %d\n", pmp_addr, cfg_offset, cfg_shift, set_content);
  csr_set_num(SPMPCFG_BASE + cfg_offset, set_content << (cfg_shift * 8));
  asm volatile("sfence.vma");
}

void enable_spmp_TOR(uintptr_t pmp_reg, uintptr_t pmp_addr, uintptr_t pmp_size, bool s, uint8_t permission) {
    // similar interface but different implementation
    // pmp reg and pmp reg + 1 will be used
    assert(pmp_reg < SPMP_COUNT);
    csr_write_num(SPMPADDR_BASE + pmp_reg, (pmp_addr + pmp_size) >> 2);
    if (pmp_reg) {
      csr_write_num(SPMPADDR_BASE + pmp_reg - 1, pmp_addr >> 2);
    }
    // printf("finished writing ADDR\n");
    uintptr_t set_content = permission | (s << 7) | (1 << 3);
    uintptr_t cfg_offset = pmp_reg > 7 ? 2 : 0;
    uintptr_t cfg_shift = pmp_reg & 0x7;
    // printf("addr %llx, cfg offset %d, shift %d, set_content %d\n", pmp_addr, cfg_offset, cfg_shift, set_content);
    csr_set_num(SPMPCFG_BASE + cfg_offset, set_content << (cfg_shift * 8));
    // printf("finished writing CFG\n");
    asm volatile("sfence.vma");
}

// void disable_spmp(uintptr_t pmp_reg) {
//   // just set pmp addr to max
//   csr_write_num(SPMPADDR_BASE + pmp_reg, -1L);
//   asm volatile("sfence.vma");
// }

void clean_spmp_all() {
  csr_write_num(SPMPCFG_BASE, 0L);
  csr_write_num(SPMPCFG_BASE + 2, 0L);
}
