#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <pthread.h>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <future>
#include <vector>
#include <execution>
#include <Windows.h>

extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

std::vector<unsigned char> buffer;

extern "C" DLLEXPORT int bprint(const char* str, ...)
{
  for(int i = 0; i < strlen(str); i++)
  {
    buffer.push_back(str[i]);
  }

  return 0;
}

extern "C" DLLEXPORT int bprintfinish(...)
{
  va_list vl;
  va_start( vl, &buffer[0] );
  fwrite(&buffer[0], buffer.size(), 1, stdout); 
  va_end(vl);
  buffer.clear();
  return 0;
}

extern "C" DLLEXPORT int print(const char* str, ...)
{
  va_list vl;
  va_start( vl, str );
  fwrite(str, strlen(str), 1, stdout); 
  va_end(vl);
  return 0;
}

extern "C" DLLEXPORT int printi(unsigned int X)
{
  printf("%i", X);
  return 0;
}

extern "C" DLLEXPORT HANDLE Win32GetStdHandle(int getHandle)
{
  return GetStdHandle(getHandle);
}

extern "C" DLLEXPORT int Win32WriteFile(HANDLE console, const char* str)
{
    DWORD dwBytesWritten{};
    WriteFile(console, str, strlen(str), &dwBytesWritten, 0);

    return 0;
}