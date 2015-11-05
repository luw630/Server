BOOL TraceInfoDirectly(LPCSTR szFileName, LPCSTR szInfo);       //文本方式
BOOL TraceInfo(LPCSTR szFileName, LPCSTR szFormat, ...);        //文本方式
BOOL TraceData(LPCSTR szFileName, LPVOID pData, DWORD dwSize);  //二进制方式

BOOL TraceInfo_C(LPCSTR szFileName, LPCSTR szFormat, ...);        //文本方式
BOOL TraceMsgInfo_C(LPCSTR szFileName, LPCSTR szFormat, ...);	  //密聊记录
BOOL TraceInfoDirectly_C(LPCSTR szFileName, LPCSTR szInfo);       //文本方式
