#include "stdafx.h"

BOOL DirExist(const char *pszDirName)
{
    WIN32_FIND_DATA fileinfo;
    char _szDir[_MAX_PATH];
    strcpy(_szDir, pszDirName);
    int nLen = (int)strlen(_szDir);
    if( (_szDir[nLen-1] == '\\') || (_szDir[nLen-1] == '/') )
    {
        _szDir[nLen-1] = '\0';
    }
    HANDLE hFind = ::FindFirstFile(_szDir, &fileinfo);

    if (hFind == INVALID_HANDLE_VALUE)
    {
        return false;
    }
    if( fileinfo.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY )
    {
        ::FindClose(hFind);
        return true;
    }
    ::FindClose(hFind);
    return false;
}

// 创建目录，包含子目录，可以创建多级子目录
BOOL CreateDir(const char *pszDirName)
{
    BOOL bRet = false;
    char _szDir[_MAX_PATH];
    char _szTmp[_MAX_DIR];
    int nLen = 0;
    int idx ;
    if( (DirExist(pszDirName)) )
        return true;
    strcpy(_szDir, pszDirName);
    nLen = (int)strlen(_szDir);
    if( _szDir[nLen-1] == '\\' || _szDir[nLen-1] == '/' )
    {
        _szDir[nLen-1] = '\0';
    }
    nLen = (int)strlen(_szDir);
    memset(_szTmp, 0, _MAX_DIR);
    char _str[2];
    for(idx = 0; idx < nLen; idx++)
    {
        if(_szDir[idx] != '\\' && _szDir[idx] != '/')
        {
            _str[0] = _szDir[idx];
            _str[1] = 0;
            strcat(_szTmp, _str);
        }
        else
        {
            CreateDirectory(_szTmp, NULL);
            _str[0] = _szDir[idx];
            _str[1] = 0;
            strcat(_szTmp, _str);
        }

        if( idx == nLen-1 )
            CreateDirectory(_szTmp, NULL);
    }

    if( DirExist(_szTmp) )
        return true;

    return false;
}
