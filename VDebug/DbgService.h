#ifndef DBGSERVICE_VDEBUG_H_H_
#define DBGSERVICE_VDEBUG_H_H_
#include <Windows.h>
#include <ComStatic/ComStatic.h>
#include <DbgCtrl/DbgCtrl.h>

class DbgCtrlService {
private:
    DbgCtrlService();

public:
    static DbgCtrlService *GetInstance();
    virtual ~DbgCtrlService();

    bool InitCtrlService();

private:
    std::ustring m_unique;
    DbgServiceBase *m_pCtrlService;
};
#endif //DBGSERVICE_VDEBUG_H_H_