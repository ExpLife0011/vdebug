#ifndef GLOBALDEF_H_H_
#define GLOBALDEF_H_H_

#define SFV_SERVICE_NAME           L"DbgService"
#define SFV_SERVICE_DISPLAY_NAME   L"DbgService"
#define SFV_SERVICE_DESCRIPTION    L"DbgService·þÎñ"

#define SFV_NOTIFY_NAME L"Global\\{784BC5BC-25D1-4861-8FED-38CFF9428877}"

#define PATH_SERVICE_CACHE L"software\\vdebug\\runner"
#define RUNNER_EVENT32  (L"runner32_%ls")
#define RUNNER_EVENT64  (L"runner64_%ls")
#define SERVICE_EVENT   (L"service_%ls")

#if WIN64 || _WIN64
#define REG_VDEBUG_CACHE    L"SoftWare\WOW6432Node\\vdebug\\config\\dbgport"
#else
#define REG_VDEBUG_CACHE    L"SoftWare\\vdebug\\config\\dbgport"
#endif
#endif //GLOBALDEF_H_H_