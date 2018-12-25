#include <Windows.h>
#include "ServiceRunner.h"

int WINAPI WinMain(HINSTANCE m, HINSTANCE p, LPSTR cmd, int show)
{
    OutputDebugStringA("runner:WinMain");
    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);

    if (count == 2 && 0 == lstrcmpiW(args[1], L"-service"))
    {
        ServiceRunner::GetInstance()->InitServiceRunner();
    }
    LocalFree(args);
    return 0;
}