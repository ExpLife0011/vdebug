#include <Windows.h>
#include <DbgCtrl/DbgCtrl.h>

#pragma comment(lib, "Dbghelp.lib")

#if _WIN64 || WIN64
#pragma comment(lib, "capstone/capstone_x64.lib")
#else
#pragma comment(lib, "capstone/capstone_x86.lib")
#endif

int WINAPI WinMain(HINSTANCE hT, HINSTANCE hP, LPSTR szCmdLine, int iShow)
{
    DbgServiceBase *p = DbgServiceBase::GetInstance();
    return 0;
}