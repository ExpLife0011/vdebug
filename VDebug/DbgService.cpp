#include "DbgService.h"
#include <Shlwapi.h>
#include <ComLib/ComLib.h>

DbgCtrlService::DbgCtrlService() {
}

DbgCtrlService::~DbgCtrlService() {
}

DbgCtrlService *DbgCtrlService::GetInstance() {
    static DbgCtrlService *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new DbgCtrlService();
    }
    return s_ptr;
}

bool DbgCtrlService::InitCtrlService() {
    srand(GetTickCount());
    m_unique = FormatW(
        L"%04x-%04x%04x-%04x",
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff,
        rand() % 0xffff
        );

    m_pCtrlService = DbgServiceBase::GetInstance();
    m_pCtrlService->InitDbgService(m_unique.c_str());
    return true;
}