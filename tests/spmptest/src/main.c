#include <spmptest.h>

void disable_timer();

_Context *simple_trap(_Event, _Context *);

// for S mode sum=1
#define RANGE_LEN(start, len) RANGE((start), (start + len))

static _Area segments[] = {      // Kernel memory mappings
  RANGE_LEN(0x80000000, 0x100000), // PMEM
  RANGE_LEN(0x40600000, 0x1000),    // uart
};

static char *sv39_alloc_base = (char *)(0xc0000000UL);
static uintptr_t sv39_alloced_size = 0;
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

// count flag 1,2,3->modeU,modeS,modeS(sum=1)
static int flag = 0;
#define MRET(MSTATUS, MEPC) asm volatile(       \
      "csrw mstatus, %0; csrw mepc, %1; mret;"  \
      : : "r"(MSTATUS), "r"(MEPC))

void test_entry() {
  disable_timer();
  flag++;
  if (flag == 1) {
    pmp_test_init_modeU();
    printf("start PMP U mode (sum=0/1) test\n");

    MRET(1 << 3, pmp_test_modeU);
  } else if (flag == 2) {
    spmp_test_init_modeU();
    printf("start SPMP U mode (sum=0/1) test\n");

    MRET(1 << 3, spmp_test_modeU);
  } else if (flag == 3) {
    pmp_test_init_modeS_sum0();
    printf("start PMP S mode (sum=0) test\n");

    MRET(1 << 3|1 << 11, pmp_test_modeS);
  } else if (flag == 4) {
    spmp_test_init_modeS();
    printf("start sPMP S mode (sum=0) test\n");

    MRET(1 << 3|1 << 11, spmp_test_modeS);
  } else if (flag == 5) {
    pmp_test_init_modeS_sum1();
    _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 2);
    
    printf("start PMP S mode (sum=1) test\n");
    MRET(1 << 3|1 << 11|1 << 18, pmp_test_modeS_SUM);
  } else if (flag == 6) {
    spmp_test_init_modeS();
    _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 2);
    
    printf("start sPMP S mode (sum=1) test\n");
    MRET(1 << 3|1 << 11|1 << 18, spmp_test_modeS_SUM);
  } else {
    _halt(1);
  }
}

int main(const char *args) {
  _cte_init(simple_trap);
  init_pmp_handler();
  init_spmp_handler();
  // printf("ecall\n");
  asm volatile("ecall;");
  _halt(1);
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
