#ifndef __rf_processor_H
#define __rf_processor_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void rfProcessCommand (uint8_t *Data, uint8_t Length);
void rfProcessError (uint8_t *Data, uint8_t Length);
  
#endif /* __rf_processor_H */
