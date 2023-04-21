#include <spmptest.h>

void spmp_test();
_Context *simple_trap(_Event, _Context *);

int main(const char *args) {
    _cte_init(simple_trap);
    spmp_test();
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
