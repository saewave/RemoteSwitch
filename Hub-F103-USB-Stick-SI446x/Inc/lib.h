#ifndef __lib_H
#define __lib_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "F103RE_Peripheral.h"

unsigned int htoi (uint8_t *ptr, uint8_t Length);
void FWWriteToFlash(uint8_t *cData);
void FWPrepareFlash(void);
void FWCheckFlash(void);

#endif /* __lib_H */
