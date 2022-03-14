#ifndef CONVERTSTRING_H
#define CONVERTSTRING_H

#include<string>
#include<Windows.h>//此处必须添加windows.h，否则无法识别CreateDirectory函数和LPCTSTR

std::wstring StringToWString( const std::string &s);//将string转换为wstring类，便于利用大恒接口写文件
bool MakeMyDirectory(const std::string& strFilePathDirctory);//创建文件夹，方便保存文件，

#endif // CONVERTSTRING_H

