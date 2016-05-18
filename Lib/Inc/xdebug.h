
#define _DEBUG

#ifdef _DEBUG

  #include "xprintf.h"
  
  #define dxdev_out xdev_out
  #define dxputs xputs
  #define dxprintf xprintf

#else
  #define dxdev_out(...)
  #define dxputs(...)
  #define dxprintf(...)
#endif
