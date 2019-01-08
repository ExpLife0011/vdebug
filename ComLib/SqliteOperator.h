//sqlite3·â×°²ã 2019 01 05
#ifndef SQLITEOPT_COMSTATIC_H_H_
#define SQLITEOPT_COMSTATIC_H_H_
#include <Windows.h>
#include <map>
#include <list>
#include <ComStatic/mstring.h>
#include <ComLib/sqlite3.h>

#pragma warning(disable:4251)

class SqliteException {
public:
    SqliteException(const std::mstring &err) {
        mErr = err;
    }

    virtual ~SqliteException() {}

    const char *what() {
        return mErr.c_str();
    }

private:
    std::mstring mErr;
};

class _declspec(dllexport) SqliteIterator {
    friend class SqliteResult;
private:
    SqliteIterator();

public:
    SqliteIterator(std::map<std::mstring, std::mstring> data);
    virtual ~SqliteIterator();
    std::mstring GetValue(const std::mstring &name);
    SqliteIterator *GetNext();

private:
    std::map<std::mstring, std::mstring> mCurData;
    SqliteIterator *mNextPtr;
};

class _declspec(dllexport) SqliteResult {
    friend class SqliteOperator;
public:
    bool IsValid();
    bool IsEmpty();
    SqliteIterator *begin();
    SqliteIterator *end();

    int GetSize();
    SqliteIterator *GetNode(int);
    SqliteIterator *GetFirst();
    SqliteIterator *GetLast();
private:
    bool Clear();
    bool Push(const std::map<std::mstring, std::mstring> &data);

private:
    std::list<SqliteIterator *> mData;
};

class _declspec(dllexport) SqliteOperator {
public:
    SqliteOperator();
    SqliteOperator(const std::mstring &filePath);
    virtual ~SqliteOperator();

    bool Open(const std::mstring &filePath);
    bool IsOpen();
    void Close();
    const SqliteResult &Select(const std::mstring &sql);
    bool Update(const std::mstring &sql);
    bool Delete(const std::mstring &sql);
    bool Insert(const std::mstring &sql);
    bool Exec(const std::mstring &sql);
    bool TransBegin();
    bool TransSubmit();
    std::mstring GetError();

private:
    static int SelectCallback(void *data, int argc, char **argv, char **name);

private:
    bool mInit;
    sqlite3 *mDb;
    std::mstring mDbPath;
    SqliteResult mResult;
    std::mstring mError;
};
#endif //SQLITEOPT_COMSTATIC_H_H_