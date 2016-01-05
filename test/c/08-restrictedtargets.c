#include <stdio.h>


void a(int i) {}

void b(short i, short j) {}

void c(short i, short j) {}

void d() {}


void
hook(void (*funptr)(short, short)) {
  funptr(1,2);
}

