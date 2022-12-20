#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <iostream>

extern "C" DLLEXPORT double putchard(double X) {
  fputc((char)X, stderr);
  return 0;
}

extern "C" DLLEXPORT double printd(double X) {
  fprintf(stderr, "%f\n", X);
  return 0;
}

extern "C" DLLEXPORT double HelloWorld()
{
  printf("Hello World!\n");
  return 0;
}

extern "C" DLLEXPORT char printchar(char X)
{
  std::cout << X;
  return X;
}

extern "C" DLLEXPORT int print(const char* X)
{
  printf(X);
  return 0;
}

extern "C" DLLEXPORT int printi(int X)
{
  printf("%i", X);
  return 0;
}