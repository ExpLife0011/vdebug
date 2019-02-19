#include <list>
#include "mstring.h"
#include "PrintFormater.h"
#include "StrUtil.h"

using namespace std;

PrintFormater::PrintFormater() {
    m_space = 2;
}

PrintFormater::~PrintFormater() {
}

void PrintFormater::SetSpaceCount(int count) {
    m_space = count;
}

bool PrintFormater::Reset() {
    m_matrix1.clear();
    m_matrix2.clear();
    return true;
}

bool PrintFormater::SetRule(const mstring &type) {
    list<mstring> result = SplitStrA(type, ";");
    m_rule.clear();
    for (list<mstring>::const_iterator it = result.begin() ; it != result.end() ; it++)
    {
        m_rule.push_back(atoi(it->c_str()));
    }
    return true;
}

PrintFormater &PrintFormater::operator << (const mstring &data) {
    if (data.empty())
    {
        m_matrix1.push_back(" ");
    } else {
        m_matrix1.push_back(data);
    }
    return *this;
}

PrintFormater &PrintFormater::operator << (PrintFormatStat stat) {
    if (stat == line_end)
    {
        m_matrix2.push_back(m_matrix1);
        m_matrix1.clear();
    } else if (stat == space)
    {
        m_matrix1.push_back(" ");
    }
    return *this;
}

bool PrintFormater::StartSession(const mstring &type) {
    return true;
}

bool PrintFormater::EndSession() {
    return true;
}

mstring PrintFormater::GetResult() {
    mstring result;
    vector<mstring> resultSet = GetResultSet();
    for (vector<mstring>::const_iterator it = resultSet.begin() ; it != resultSet.end() ; it++)
    {
        result += *it;
        result += "\r\n";
    }
    return result;
}

vector<mstring> PrintFormater::GetResultSet() {
    vector<mstring> result;

    int nodeSize = (int)m_matrix2[0].size();
    int lineSize = (int)m_matrix2.size();

    //init format rule
    while ((int)m_rule.size() < nodeSize) {
        m_rule.push_back(0);
    }
    int i = 0;
    int j = 0;
    for (i = 0 ; i < nodeSize ; i++)
    {
        int maxSize = 0;
        for (j = 0 ; j < lineSize ; j++)
        {
            if ((int)m_matrix2[j][i].size() > maxSize)
            {
                maxSize = m_matrix2[j][i].size();
            }
        }

        if (m_rule[i] == 0)
        {
            m_rule[i] = maxSize;
        }
    }

    result.clear();
    for (i = 0 ; i < lineSize ; i++)
    {
        mstring tmp;
        for (j = 0 ; j < nodeSize ; j++)
        {
            string node = m_matrix2[i][j];
            if ((int)node.size() < m_rule[j])
            {
                if (j != (nodeSize - 1))
                {
                    int count = m_rule[j] - node.size();
                    while (count > 0) {
                        node += " ";
                        count--;
                    }
                }

                tmp += node;
            } else {
                tmp += node;
            }

            if (j != (nodeSize - 1))
            {
                for (int k = 0 ; k < m_space ; k++)
                {
                    tmp += " ";
                }
            }
        }
        result.push_back(tmp);
    }
    return result;
}