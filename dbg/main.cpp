#include <Windows.h>
#include <Shlwapi.h>
#include <DbgCtrl/DbgCtrl.h>
#include <ComStatic/ComStatic.h>
#include <ComLib/ComLib.h>
#include <runner/runner.h>

using namespace std;

#pragma comment(lib, "Dbghelp.lib")
#if _WIN64 || WIN64
#pragma comment(lib, "capstone/capstone_x64.lib")
#else
#pragma comment(lib, "capstone/capstone_x86.lib")
#endif

static void _KeepAlive(const ustring &unique) {
    while (TRUE) {
        HANDLE service = OpenEventW(EVENT_ALL_ACCESS, FALSE, FormatW(SERVICE_EVENT, unique.c_str()).c_str());

        if (!service)
        {
            break;
        }

        CloseHandle(service);
        Sleep(5000);
    }
}

int WINAPI WinMain(HINSTANCE hT, HINSTANCE hP, LPSTR szCmdLine, int iShow)
{
    int count = 0;
    LPWSTR *args = CommandLineToArgvW(GetCommandLineW(), &count);

    if (count != 2)
    {
        LocalFree(args);
        return 0;
    }

    ustring cmd = args[1];
    LocalFree(args);
    if (!cmd.startwith(L"runner"))
    {
        return 0;
    }

    WCHAR path[256];
    GetModuleFileNameW(NULL, path, 256);
#if WIN64 || _WIN64
    PathAppendW(path, L"..\\..\\ComLib64.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\mq64.dll");
    LoadLibraryW(path);
#else
    PathAppendW(path, L"..\\..\\ComLib32.dll");
    LoadLibraryW(path);
    PathAppendW(path, L"..\\mq32.dll");
    LoadLibraryW(path);
#endif

    size_t pos = cmd.rfind('_');
    ustring unique = cmd.substr(pos + 1, cmd.size() - pos - 1);

#if _WIN64 || WIN64
#else
#endif
    _KeepAlive(unique);
    return 0;
}