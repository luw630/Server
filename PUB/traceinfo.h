BOOL TraceInfoDirectly(LPCSTR szFileName, LPCSTR szInfo);       //�ı���ʽ
BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);        //�ı���ʽ
BOOL TraceData(LPCSTR szFileName, LPVOID pData, DWORD dwSize);  //�����Ʒ�ʽ

BOOL TraceInfo_C(LPCSTR szFileName, LPCSTR szFormat, ...);        //�ı���ʽ
BOOL TraceMsgInfo_C(LPCSTR szFileName, LPCSTR szFormat, ...);	  //���ļ�¼
BOOL TraceInfoDirectly_C(LPCSTR szFileName, LPCSTR szInfo);       //�ı���ʽ
