#include "rf_cmd_exec.h"
#include "rf_cmd.h"
#include "xdebug.h"

void rfCmdExec(uint8_t *Data, uint8_t Length) {
  dxprintf("*** CMD:\n");
  for(int i = 0; i < Length; i++) {
    dxprintf("%d ", Data[i]);
  }
  dxprintf("\n");
}

void rfInternalCallback(uint8_t *Data, uint8_t Size) {
  SendCommandToHub(rfCMD_R_DATA, Data, Size);
}
  
void rfStartup(void) {
  dxputs("rfStartup\n");
}
