#ifndef __rf_processor_H
#define __rf_processor_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

void rfProcessCommand (char *Data, uint8_t Length);
void rfProcessError (char *Data, uint8_t Length);
  
#endif /* __rf_processor_H */
