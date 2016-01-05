#include <stdio.h>

void a() {}

void b() {a();}

void c() {}

void d() {c();}

int
main(int argc, char **argv) {
  return 0;
}


