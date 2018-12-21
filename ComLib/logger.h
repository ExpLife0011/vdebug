#ifndef LOGGER_COMLIB_H_H_
#define LOGGER_COMLIB_H_H_
#include "global.h"

class Logger {
public:
    virtual void LoggerInit(const wchar_t *format, ...) = 0;
    virtual void LoggerPrint(const wchar_t *format, ...) = 0;
    virtual void LoggerInfo(const wchar_t *format, ...) = 0;
    virtual void LoggerWarn(const wchar_t *format, ...) = 0;
    virtual void LoggerErr(const wchar_t *format, ...) = 0;
};

static void _LoggerCommon(int level, const char *file, int line, const wchar_t *format, ...) {
}

#define LOGGER_PRINT(f, ...)      _LoggerCommon(0, __FILE__, __LINE__, f, ##__VA_ARGS__)
#define LOGGER_INFO(f, ...)       _LoggerCommon(1, __FILE__, __LINE__, f, ##__VA_ARGS__)
#define LOGGER_WARN(f, ...)       _LoggerCommon(2, __FILE__, __LINE__, f, ##__VA_ARGS__)
#define LOGGER_ERROR(f, ...)      _LoggerCommon(3, __FILE__, __LINE__, f, ##__VA_ARGS__)
Logger *GetLogger();
#endif //LOGGER_COMLIB_H_H_