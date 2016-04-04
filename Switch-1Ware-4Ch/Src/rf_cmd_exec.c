#include "rf_cmd_exec.h"
#include <stdlib.h>

void rfCmdExec(uint8_t *Data, uint8_t Length) {
  printf("*** CMD:\n");
  for(int i = 0; i < Length; i++) {
    printf("%d ", Data[i]);
  }
  printf("\n");
}
