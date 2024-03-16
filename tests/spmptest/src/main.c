#include <spmptest.h>

void disable_timer();
static void* sv39_pgalloc(size_t pg_size);
static void sv39_pgfree(void *ptr);
void test_entry();
_Context *simple_trap(_Event, _Context *);

// for S mode sum=1
#define RANGE_LEN(start, len) RANGE((start), (start + len))

static _Area segments[] = {		 // Kernel memory mappings
#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS)
  RANGE_LEN(0x80000000, 0x8000000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
  // RANGE_LEN(0xc0000000, 0x100000),  // page table test allocates from this position
#elif defined(__ARCH_RISCV64_XS_NHV3) || defined(__ARCH_RISCV64_XS_NHV3_FLASH)
  RANGE_LEN(0x1000000000, 0x800000),	// PMEM
  RANGE_LEN(0x37000000,   0x1000),		// uart
  // RANGE_LEN(0x1040000000, 0x100000),  // page table test allocates from this position
#endif
};

#if defined(__ARCH_RISCV64_NOOP) || defined(__ARCH_RISCV64_XS)
static char *sv39_alloc_base = (char *)(0xc0000000);
#elif defined(__ARCH_RISCV64_XS_NHV3) || defined(__ARCH_RISCV64_XS_NHV3_FLASH)
static char *sv39_alloc_base = (char *)(0x1040000000);
#endif

static uintptr_t sv39_alloced_size = 0;
static int flag = 0;  // count flag 1,2,3->modeU,modeS,modeS(sum=1)


int main(const char *args) {
  _cte_init(simple_trap);
  
  csr_set(SPMP_SWITCH, 0x1); // open spmp
  printf("SPMP_SWITCH is on\n");

  init_spmp_handler();
  asm volatile("ecall");
  _halt(1);
}



static void* sv39_pgalloc(size_t pg_size) {
  assert(pg_size == 0x1000);
  printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base + sv39_alloced_size);
  sv39_alloced_size += pg_size;
  return ret;
}

static void sv39_pgfree(void *ptr) {
  return ;
}



void test_entry() {
  printf("test entry\n");
  disable_timer();
  flag++;
  
  if (flag == 1) {
    spmp_test_init_modeU();
    printf("start sPMP U mode (sum=0/1) test\n");

    MRET(1 << 3, spmp_test_modeU);
  } else if (flag == 2) {
    spmp_test_init_modeS();
    printf("start sPMP S mode (sum=0) test\n");

    MRET(1 << 3|1 << 11, spmp_test_modeS);
  } else if (flag == 3) {
    spmp_test_init_modeS();
    _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 2);
    
    printf("start sPMP S mode (sum=1) test\n");
    MRET(1 << 3|1 << 11|1 << 18, spmp_test_modeS_SUM);
  } else {
    _halt(1);
  }
  printf("quite test_entry\n");
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
