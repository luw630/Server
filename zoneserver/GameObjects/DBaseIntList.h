#ifndef     _BaseIntList_H_
#define     _BaseIntList_H_

/****************
文件格式：
1、文本文件
2、第一个数据是整个后面数据的数量
3、文件中不要添加任何其他符号
***************************/


class   CBaseIntList
{
protected:
    int             m_iCount;
    int             *m_pData;
public:
    CBaseIntList(void);
    ~CBaseIntList(void);

    BOOL            LoadDataFromFile(char *fileName);
    BOOL            SaveDataToFile(char *fileName);

    void            ClearData(void);
    int             GetCount(void)          {   return  m_iCount;                           }
    int             *GetData(void)          {   return  m_pData;                            }
    int             SetData( int *pData, int iCount );
};

#endif  _BaseIntList_H_