#include <stdio.h>
#include <stdarg.h>

void a(int i, ...) {}

void b(int i, ...) {}

void c(int i, int b, ...) {}

void d(char i, ...) {}

int
hrm(void (*funptr)(int i, ...)) {
  funptr(1, 2, 3, 4);
  funptr(2, 'h', 'i', 't', 'h', 'e', 'r', 'e');
  return 0;
}


int main() {
  hrm(c);
  return 0;
}

