#include <stdio.h>

void b();

void a() {b();}

void b() {a();}

int
main(int argc, char **argv) {
  a();
  return 0;
}


