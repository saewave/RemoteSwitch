#include "F103RE_Peripheral.h"
#include "xdebug.h"
#include "stdio.h"
#include "core.h"

#ifdef SAE_ENABLE_DEBUG
void xprintf(                 /* Put a formatted string to the default device */
             char *fmt, /* Pointer to the format string */
             ...              /* Optional arguments */
             )
{
    va_list arp;
    char Buf[SAE_OUTPUT_DATA_LENGTH];
    va_start(arp, fmt);
    vsnprintf(Buf, SAE_OUTPUT_DATA_LENGTH, fmt, arp);
    va_end(arp);
    uint8_t i;
    for (i = 0; i < SAE_OUTPUT_DATA_LENGTH; i++) {
        if (Buf[i] == 0) {
            break;
        }
    }
    if (i == SAE_OUTPUT_DATA_LENGTH)
        Buf[SAE_OUTPUT_DATA_LENGTH-1] = 0;
    QueueResponse(Buf, OUSART1);
}
#endif
