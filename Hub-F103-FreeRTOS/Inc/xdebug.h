#include <stdarg.h>
#include "config.h"

#ifdef SAE_ENABLE_DEBUG
  #define dxprintf xprintf

void xprintf(                 /* Put a formatted string to the default device */
             char *fmt,       /* Pointer to the format string */
             ...              /* Optional arguments */
             );

#else
  #define dxprintf(...)
#endif
