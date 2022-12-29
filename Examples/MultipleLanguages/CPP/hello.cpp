#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <iostream>

extern "C" DLLEXPORT double HelloFromCPP()
{
  printf("Hello! I'm C++!\n");
  return 0;
}