#include "PrintFormater.h"

using namespace std;

PrintFormater::PrintFormater() {
}

PrintFormater::~PrintFormater() {
}

bool PrintFormater::InitRule(const mstring &type, const mstring &rule) {
    return true;
}

bool PrintFormater::Reset() {
    m_matrix1.clear();
    m_matrix2.clear();
    return true;
}

bool PrintFormater::SetRule(const mstring &type) {
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

const char *PrintFormater::GetResult() {
    static string s_result;
    vector<int> rule;
    int i = 0;
    int j = 0;
    for (i = 0 ; i < (int)m_matrix2[0].size() ; i++)
    {
        int tmp = 0;
        for (j = 0 ; j < (int)m_matrix2.size() ; j++)
        {
            if ((int)m_matrix2[j][i].size() > tmp)
            {
                tmp = m_matrix2[j][i].size();
            }
        }
        rule.push_back(tmp);
    }

    s_result.clear();
    for (i = 0 ; i < (int)m_matrix2.size() ; i++)
    {
        int border = (int)m_matrix2[0].size();
        for (j = 0 ; j < border ; j++)
        {
            string node = m_matrix2[i][j];
            if ((int)node.size() < rule[j])
            {
                if (j != (border - 1))
                {
                    int count = rule[j] - node.size();
                    while (count > 0) {
                        node += " ";
                        count--;
                    }
                }

                s_result += node;
            } else {
                s_result += node;
            }

            if (j != (border - 1))
            {
                for (int k = 0 ; k < ms_space ; k++)
                {
                    s_result += " ";
                }
            }
        }
        s_result += "\n";
    }
    return s_result.c_str();
}