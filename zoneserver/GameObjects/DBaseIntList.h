#ifndef     _BaseIntList_H_
#define     _BaseIntList_H_

/****************
�ļ���ʽ��
1���ı��ļ�
2����һ�������������������ݵ�����
3���ļ��в�Ҫ����κ���������
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