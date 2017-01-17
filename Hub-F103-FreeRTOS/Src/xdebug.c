#include "F103RE_Peripheral.h"
#include "xdebug.h"
#include "freertos.h"
#include "stdio.h"

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
    QueueResponse(Buf, OUSART2);
}
#endif
