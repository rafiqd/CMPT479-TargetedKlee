#include <stdio.h>

void a() {a();}

int
main(int argc, char **argv) {
  a();
  return 0;
}


