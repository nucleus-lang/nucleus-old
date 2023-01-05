#ifdef _WIN32
#define DLLEXPORT __declspec(dllexport)
#else
#define DLLEXPORT
#endif

#include <Windows.h>

//extern "C" DLLEXPORT HANDLE Win32GetStdHandle(int getHandle)
//{
//  return GetStdHandle(getHandle);
//}
//
//extern "C" DLLEXPORT int Win32WriteFile(HANDLE console, const char* str)
//{
//    DWORD dwBytesWritten{};
//    WriteFile(console, str, strlen(str), &dwBytesWritten, 0);
//
//    return 0;
//}

extern "C" DLLEXPORT int printint(int i)
{
	printf("%i\n", i);
	return 0;
}

//extern "C" DLLEXPORT int printstring(char* str)
//{
//	printf(str);
//	return 0;
//}