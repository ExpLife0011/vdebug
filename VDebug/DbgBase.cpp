#include "DbgBase.h"
#include <set>

bool CDbgBase::IsRegister(const ustring &wstr) const
{
    static set<ustring> *s_ptr = NULL;
    if (!s_ptr)
    {
        s_ptr = new set<ustring>();
        //x86
        s_ptr->insert(L"eax"), s_ptr->insert(L"ebx");
        s_ptr->insert(L"ecx"), s_ptr->insert(L"edx");
        s_ptr->insert(L"esi"), s_ptr->insert(L"edi");
        s_ptr->insert(L"esp"), s_ptr->insert(L"ebp");
        //x64
        s_ptr->insert(L"rax"), s_ptr->insert(L"rbx");
        s_ptr->insert(L"rcx"), s_ptr->insert(L"rdx");
        s_ptr->insert(L"rsi"), s_ptr->insert(L"rdi");
        s_ptr->insert(L"r8"), s_ptr->insert(L"r9");
        s_ptr->insert(L"r10"), s_ptr->insert(L"r11");
        s_ptr->insert(L"r12"), s_ptr->insert(L"r13");
        s_ptr->insert(L"r14"), s_ptr->insert(L"r15");

        s_ptr->insert(L"cs"), s_ptr->insert(L"ss");
        s_ptr->insert(L"ds"), s_ptr->insert(L"es");
        s_ptr->insert(L"fs"), s_ptr->insert(L"gs");
    }
    ustring wstrLower(wstr);
    wstrLower.trim();
    wstrLower.makelower();
    return (s_ptr->end() != s_ptr->find(wstrLower));
}