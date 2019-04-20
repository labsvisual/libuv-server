#include <stdio.h>
#include <uv.h>

int main() {
  int err;

  double uptime;
  err = uv_uptime(&uptime);
  printf("Uptime: %f", uptime);

  size_t resident_set_memory;

  return 0;
}

