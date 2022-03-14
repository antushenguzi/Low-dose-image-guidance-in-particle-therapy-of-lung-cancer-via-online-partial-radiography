#include"convertstring.h"
std::wstring StringToWString( const std::string &s)
{
    std::wstring wsTmp(s.begin(), s.end());
    return wsTmp;
}

bool MakeMyDirectory(const std::string &strFilePathDirctory)
{
    std::wstring wDirectoryName = StringToWString(strFilePathDirctory);
    LPCTSTR lpwdir = wDirectoryName.c_str();
    return CreateDirectory(lpwdir, NULL);
}

