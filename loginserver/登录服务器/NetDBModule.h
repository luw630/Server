#pragma once

#include "networkmodule\netconsumermodule\netconsumermodule.h"

class CNetDBModule
{
public:
    int Execution(void);	            // �������������׽��ֲ��������ݵĽ����뷢�ͣ���Ӧ�����ĵ���
    void Destroy();

    bool Connect( LPCSTR ip, LPCSTR port );
    bool Entry( LPCVOID data, size_t size );

public:
    void RecvGetCharacterMsg    ( struct SAGetCharacterMsg *pMsg     );
    void RecvCreateCharacterMsg ( struct SACreatePlayerMsg *pMsg     );
    void RecvDelCharacterMsg    ( struct SADeleteCharacterMsg *pMsg  );
    void RecvGetCharacterListMsg( struct SAGetCharacterListMsg *pMsg );
    void RecvCheckCharacterMsg  ( struct SACheckPlayerDBMsg *pMsg    );
    void RecvCheckWarehouseMsg  ( struct SACheckWarehouseMsg *pMsg   );
    void RecvUpdatePlayerMsg    ( struct SAUpdatePlayerMsg * pMsg    );
    void RecvGetAccountByNameMsg( struct SAGetAccountByName *pMsg    );
	void RecvGetRankListMsg		( struct SADBGETRankList *pMsg		 );
	 //void RecvSavePlayerConfigMsg( struct SQSavePlayerConfig *pMsg    );
public:
    CNetDBModule(void);
    ~CNetDBModule(void);

public:
    CNetConsumerModule client;

private:
    std::string dbServerIP;
    std::string dbServerPort;
};
