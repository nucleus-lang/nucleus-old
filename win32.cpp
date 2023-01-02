#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <Windows.h>

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