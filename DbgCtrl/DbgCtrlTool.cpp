#include <Windows.h>
#include <Iphlpapi.h>
#include <set>
#include <ComLib/ComLib.h>
#include <ComStatic/ComStatic.h>
#include "DbgCtrlTool.h"

#pragma comment(lib, "Iphlpapi.lib")

using namespace std;

static unsigned short _GetMinUnUsedPort() {
    static char *s_buffer = new char[1024 * 8];
    static DWORD s_size = 1024 * 8;
    unsigned short minPort = 6500;

    DWORD size = s_size;
    DWORD result = GetTcpTable((PMIB_TCPTABLE)s_buffer, &size, FALSE);
    if ((result == ERROR_INSUFFICIENT_BUFFER) && size >  s_size)
    {
        delete []s_buffer;
        s_size = size + 4096;
        s_buffer = new char[s_size];

        size = s_size;
        if (NO_ERROR != GetTcpTable((PMIB_TCPTABLE)s_buffer, &size, FALSE))
        {
            return minPort;
        }
    }

    PMIB_TCPTABLE ptr = (PMIB_TCPTABLE)s_buffer;
    set<int> portSet;
    for (int i = 0 ; i < (int)ptr->dwNumEntries ; i++)
    {
        if (ptr->table->dwLocalPort >= 6500 && ptr->table->dwLocalPort < 6700)
        {
            portSet.insert(ptr->table->dwLocalPort);
        }
    }

    while (portSet.end() != portSet.find(minPort)) {
        minPort++;

        if (minPort == 6700)
        {
            break;
        }
    }
    return minPort;
}

unsigned short CalPortFormUnique(const wstring &unique) {
    DWORD port = RegGetDWORDFromRegW(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str(), 0);
    if (port)
    {
        return (unsigned short)port;
    }

    port = _GetMinUnUsedPort();
    RegSetDWORDValueW(HKEY_LOCAL_MACHINE, REG_VDEBUG_CACHE, unique.c_str(), port);
    return (unsigned short)port;
}