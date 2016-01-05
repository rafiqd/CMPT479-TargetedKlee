#include <stdio.h>


void a() {}

void b(int i) {}

void c(short i, short j) {}


void
hook(void (*funptr)()) {
  funptr();
  funptr(1);
  funptr(1,2);
}

extern void (*foo)();
extern void (*bar)();

int
main() {
  foo = a;
  bar = c;
  hook(b);
  return 0;
}


