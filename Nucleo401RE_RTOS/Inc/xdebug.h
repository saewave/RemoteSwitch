
#define _DEBUG

#ifdef _DEBUG

  #define dxprintf printf

#else
  #define dxprintf(...)
#endif
