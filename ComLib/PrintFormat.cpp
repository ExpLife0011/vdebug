#include "PrintFormat.h"
#include "ComUtil.h"

using namespace std;

SyntaxFormater::SyntaxFormater() {
}

SyntaxFormater::~SyntaxFormater() {
}

bool SyntaxFormater::InitRule(const char *type, const char *rule) {
    return true;
}

bool SyntaxFormater::Reset() {
    m_matrix1.clear();
    m_matrix2.clear();
    return true;
}

bool SyntaxFormater::SetRule(const char *type) {
    return true;
}

PrintFormater &SyntaxFormater::operator << (const char *data) {
    m_matrix1.push_back(data);
    return *this;
}

PrintFormater &SyntaxFormater::operator << (PrintFormatStat stat) {
    if (stat == line_end)
    {
        m_matrix2.push_back(m_matrix1);
        m_matrix1.clear();
    }
    return *this;
}

bool SyntaxFormater::StartSession(const char *type) {
    return true;
}

bool SyntaxFormater::EndSession() {
    return true;
}

const char *SyntaxFormater::GetResult() {
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
        for (j = 0 ; j < (int)m_matrix2[0].size() ; j++)
        {
            string node = m_matrix2[i][j];
            if ((int)node.size() < rule[j])
            {
                int count = rule[j] - node.size();
                while (count > 0) {
                    node += " ";
                    count--;
                }
                s_result += node;
            } else {
                s_result += node;
            }

            for (int k = 0 ; k < ms_space ; k++)
            {
                s_result += " ";
            }
        }
        s_result += "\n";
    }
    return s_result.c_str();
}

VOID WINAPI RundllFun(HWND hwnd, HINSTANCE hinst, LPSTR cmd, int show) {
    SyntaxFormater *p = new SyntaxFormater();
    SyntaxFormater &ff = *p;
    ff << "ÄãºÃ£¬ÊÀ½ç" << "1111" << "222" << line_end;
    ff << "11"         << "aa"   << "bb"  << line_end;
    string dd = p->GetResult();
    OutputDebugStringA(dd.c_str());
    MessageBoxA(0, 0, 0, 0);
}