#include "SqliteOperator.h"

using namespace std;

SqliteIterator::SqliteIterator(std::map<std::mstring, std::mstring> data) {
    mCurData = data;
    mNextPtr = NULL;
}

SqliteIterator::~SqliteIterator() {
}

mstring SqliteIterator::GetValue(const mstring &name) {
    map<mstring, mstring>::const_iterator it = mCurData.find(name);
    if (mCurData.end() != it)
    {
        return it->second;
    }
    return "";
}

SqliteIterator *SqliteIterator::GetNext() {
    return mNextPtr;
}

bool SqliteResult::IsValid() {
    return !mData.empty();
}

bool SqliteResult::IsEmpty() {
    return mData.empty();
}

SqliteIterator *SqliteResult::begin() {
    if (mData.empty())
    {
        return NULL;
    }

    return *mData.begin();
}

SqliteIterator *SqliteResult::end() {
    return NULL;
}

bool SqliteResult::Clear() {
    for (list<SqliteIterator *>::iterator it = mData.begin() ; it != mData.end() ; it++)
    {
        delete (*it);
    }
    mData.clear();
    return true;
}

bool SqliteResult::Push(const std::map<std::mstring, std::mstring> &data) {
    SqliteIterator *ptr = new SqliteIterator(data);
    list<SqliteIterator *>::reverse_iterator endNode = mData.rbegin();
    if (endNode != mData.rend())
    {
        (*endNode)->mNextPtr = ptr;
    }
    mData.push_back(ptr);

    return true;
}

SqliteOperator::SqliteOperator() {
    mInit = false;
    mDb = NULL;
}

SqliteOperator::~SqliteOperator() {
    Close();
}

int SqliteOperator::SelectCallback(void *data, int argc, char **argv, char **name) {
    SqliteOperator *ptr = (SqliteOperator *)data;
    map<mstring, mstring> line;
    for (int i = 0 ; i < argc ; i++)
    {
        line.insert(make_pair(name[i], argv[i]));
    }

    ptr->mResult.Push(line);
    return 0;
}

bool SqliteOperator::OpenDbFile(const std::mstring &filePath) {
    mInit = (0 == sqlite3_open(filePath.c_str(), &mDb));
    return mInit;
}

void SqliteOperator::Close() {
    if (mDb)
    {
        sqlite3_close(mDb);
        mDb = NULL;
    }
}

const SqliteResult &SqliteOperator::Select(const std::mstring &sql) {
    mResult.Clear();
    char *err = NULL;
    sqlite3_exec(mDb, sql.c_str(), SelectCallback, this, &err);
    return mResult;
}

bool SqliteOperator::Update(const std::mstring &sql) {
    Exec(sql.c_str());
    return true;
}

bool SqliteOperator::Delete(const std::mstring &sql) {
    Exec(sql.c_str());
    return true;
}

bool SqliteOperator::Insert(const std::mstring &sql) {
    Exec(sql.c_str());
    return true;
}

bool SqliteOperator::Exec(const std::mstring &sql) {
    char *err = NULL;
    sqlite3_exec(mDb, sql.c_str(), NULL, NULL, &err);

    mError = err;
    return true;
}

mstring SqliteOperator::GetError() {
    return mError;
}

bool SqliteOperator::TransBegin() {
    return true;
}

bool SqliteOperator::TransSubmit() {
    return true;
}