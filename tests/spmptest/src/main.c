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
void* sv39_pgalloc(size_t pg_size) {
  assert(pg_size == 0x1000);
  printf("sv39 pgalloc called\n");
  void *ret = (void *)(sv39_alloc_base + sv39_alloced_size);
  sv39_alloced_size += pg_size;
  return ret;
}

void sv39_pgfree(void *ptr) {
  return ;
}

int main(const char *args) {
  _cte_init(simple_trap);
  init_spmp_handler();
  disable_timer();
  switch (args[0]) {
    case 'u':
      spmp_test_init_modeU();
      printf("start sPMP U mode (sum=0/1) test\n");
      asm volatile("csrs mstatus, %0; csrw mepc, %1; mret;"
        : : "r"(1 << 3|0 << 11), "r"(&spmp_test_modeU));
      break;
    case 's':
      spmp_test_init_modeS();
      printf("start sPMP S mode (sum=0) test\n");
      asm volatile("csrs mstatus, %0; csrw mepc, %1; mret;"
        : : "r"(1 << 3|1 << 11), "r"(&spmp_test_modeS));
      break;
    case 'v':
      spmp_test_init_modeS();
      _vme_init_custom(sv39_pgalloc, sv39_pgfree, segments, 2);
      printf("start sPMP S mode (sum=1) test\n");
      asm volatile("csrs mstatus, %0; csrw mepc, %1; mret;"
        : : "r"(1 << 3|1 << 11|1 << 18), "r"(&spmp_test_modeS_SUM));
      break;
    default:
      printf("Usage: make run mainargs=*\n");
      printf("  u: sPMP mode U test\n");
      printf("  s: sPMP mode S test (sum=0)\n");
      printf("  v: sPMP mode S test (sum=1)\n");
      break;
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
