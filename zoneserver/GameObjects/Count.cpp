#include "StdAfx.h"
#include "count.h"
#include <string>

#include "BaseObject.h"
#include "GlobalFunctions.h"

CCount g_Count;
///////////////////////////////////////////////////////////////////////
extern int rfalse(char, char, const char*, ...);

//-----------------------------------------------------------

void CCount::ResetCount(eCountType type)
{
    switch(type) {
        case CCT_MONEYDATA:
            ZeroMemory(&m_stMoneyData, sizeof(_MONEYDATA));
            break;
            
        case CCT_PLAYERDATA:
            ZeroMemory(&m_stPlayerData, sizeof(_PLAYERDATA));
            break;

        case CTT_ALL:
            ZeroMemory(&m_stMoneyData, sizeof(_MONEYDATA));
            ZeroMemory(&m_stPlayerData, sizeof(_PLAYERDATA));
            break;

        default:
            return;
    }

}

void CCount::StartTime(eCountType countType)
{
    switch(countType) {
        case CCT_MONEYDATA: {
                if(m_dwCountMoneyTime != 0)
                    return;

                m_dwCountMoneyTime = GetTickCount();
            }break;

        case CCT_PLAYERDATA: {
                if(m_dwCountPlayerTime != 0)
                    return;

                m_dwCountPlayerTime = GetTickCount();

            }break;

        case CTT_ALL: {
                m_dwCountMoneyTime = GetTickCount();
                m_dwCountPlayerTime = GetTickCount();

            }break;

        default:
            return;

    }
}

void CCount::UpdateMoneyDataCount(int nTime) 
{

    if((m_dwCountMoneyTime + 1000 * 60 * nTime) <=  GetTickCount()) {
        m_dwCountMoneyTime = 0;
        StartTime(CCT_MONEYDATA);
        PrintMoneyDataCount();
        ResetCount(CCT_MONEYDATA);
    }

}

void CCount::UpdatePlayerDataCount(_PLAYERDATA *pData, const char*filename, int nTime,  int nSaveType)
{
	if (!pData || !filename)
	{
		rfalse(4, 1, "Count.cpp - UpdatePlayerDataCount() - !pData || !filename");
		return;
	}

    if((m_dwCountPlayerTime + 1000 * 60 * nTime) <=  GetTickCount()) {
        m_dwCountPlayerTime = 0;
        StartTime(CCT_PLAYERDATA);
        PrintPlayerDataCount(pData, filename, nSaveType);
        ResetCount(CCT_PLAYERDATA);
    }
     
}

void CCount::SetMoneyCount(eMoneyCType type, int nNum)
{
    switch(type) {
        case CT_GROUND: 
                m_stMoneyData.dwGroundMoney += nNum;
                break;

        case CT_BUY:
                m_stMoneyData.dwBuyMoney += nNum;
                break;

        case CT_SALE:
                m_stMoneyData.dwSaleMoney += nNum;
                break;

        case CT_EXCHANGE:
                m_stMoneyData.dwExchangeMoney += nNum;
                break;

        case CT_GETFROMNPC:
                m_stMoneyData.dwGetMoneyFromNpc += nNum;
                break;

        case CT_SUBFROMNPC:
                m_stMoneyData.dwSubMoneyFromNpc += nNum;
                break;

        default:
            return;

    }
}

void CCount::SetUpdateTime(eCountType type, int nTime)
{
    switch(type)  {
        case CCT_MONEYDATA:
            m_wUpdateMoneyTime = nTime;
            break;
        case CCT_PLAYERDATA:
            m_wUpdatePlayerTime = nTime;
            break;
        case CTT_ALL:
            m_wUpdateMoneyTime = nTime;
            m_wUpdatePlayerTime = nTime;
            break;

        default:
            return;

    }

}

int  CCount::GetUpdateTime(eCountType type)
{
    int ret = 0;

    switch(type)  {
        case CCT_MONEYDATA:
            ret = m_wUpdateMoneyTime;
            break;
        case CCT_PLAYERDATA:
            ret = m_wUpdatePlayerTime;
            break;

        default:
            return 0;       

    }

    return ret;

}

void CCount::AddPlayerToList(std::string name)
{
    m_PlayerList.push_back(name);
}

bool CCount::DelPlayerFromList(std::string name, bool Really)
{
    if(m_PlayerList.size() == 0)
        return false;

    std::string check_str = "";
    std::list<std::string>::iterator it = m_PlayerList.begin();
    for (; it != m_PlayerList.end(); ) {
        check_str = *it;
        if(strcmp(check_str.c_str(), name.c_str()) == 0) {
            if(Really) {
                m_PlayerList.erase(it);
            }
            return true;
        }

        it++;
    }

    return false;
}

void CCount::SetPlayerDataCount(_PLAYERDATA *data) 
{
	if (!data)
	{
		rfalse(4, 1, "Count.cpp - SetPlayerDataCount() - !data");
		return;
	}

    memcpy(&m_stPlayerData, data, sizeof(_PLAYERDATA));  
}

void CCount::GetPlayerEquipCount(_PLAYEREQUIPDATA *data)
{
	if (!data)
	{
		rfalse(4, 1, "Count.cpp - GetPlayerEquipCount() - !data");
		return;
	}

    memcpy(&m_stPlayerData.stEquipData, data, sizeof(_PLAYEREQUIPDATA));
}

void CCount::PrintMoneyDataCount(int type)
{
    rfalse(type, 1, "-----------------------------\r\n%s", GetStringTime());
    rfalse(type, 1, "������ʰ��Ǯ��: %d", m_stMoneyData.dwGroundMoney);
    rfalse(type, 1, "���׵�Ǯ��: %d", m_stMoneyData.dwExchangeMoney);
    rfalse(type, 1, "����NPC����Ǯ��: %d", m_stMoneyData.dwSaleMoney);
    rfalse(type, 1, "��NPC������������Ǯ��: %d", m_stMoneyData.dwBuyMoney);
    rfalse(type, 1, "��NPC�õ���Ǯ��: %d", m_stMoneyData.dwGetMoneyFromNpc);
    rfalse(type, 1, "��NPC��ȥ��Ǯ��: %d", m_stMoneyData.dwSubMoneyFromNpc);
}

void CCount::PrintPlayerDataCount(_PLAYERDATA *pData, int type)
{
    rfalse(type, 1, "���������-------------------\r\n%s", GetStringTime());
    rfalse(type, 1, "�ȼ�: %d" ,  pData->dwLevel); 
    rfalse(type, 1, "����: %d" , pData->dwExper); 
    rfalse(type, 1, "��Ǯ: %d" , pData->dwMoney); 
    rfalse(type, 1, "�ֿ��Ǯ: %d" , pData->dwStoreMoney); 
    rfalse(type, 1, "��̫��: %d" , pData->dwEN);
    rfalse(type, 1, "������: %d" , pData->dwIN);
    rfalse(type, 1, "��̫��: %d" , pData->dwST); 
    rfalse(type, 1, "������: %d" , pData->dwAG); 
    rfalse(type, 1, "װ��������: %d" , pData->stEquipData.wEquipAGI); 
    rfalse(type, 1, "װ���书������: %d" , pData->stEquipData.wEquipDAM); 
    rfalse(type, 1, "װ��������: %d" , pData->stEquipData.wEquipDEF); 
    rfalse(type, 1, "װ���ڹ�������: %d" , pData->stEquipData.wEquipPOW); 

    int i = 0;
    for(; i < 12; i++) {
        if(pData->stSkill[i].byLevel == 0)
            continue;

            rfalse(type, 1, "�书���: %d, �书�ȼ�: %d, �书������: %d",  pData->stSkill[i].wTypeID , pData->stSkill[i].byLevel ,pData->stSkill[i].dwProficiency ); 
            
    }
}

void CCount::PrintPlayerDataCount(_PLAYERDATA *pData, const char*filename,  int nSaveType)
{
    m_outFile.open(FormatString("CountData\\%s.txt", filename), nSaveType);
    if(m_outFile.fail()) 
        return;

    m_outFile << "���������-------------------\r\n" << GetStringTime() << std::endl;

    m_outFile << "��Ϊ" << pData->dwLevel << std::endl; 
    m_outFile << "����: " << pData->dwExper << std::endl; 
    m_outFile << "��Ǯ: " << pData->dwMoney << std::endl; 
    m_outFile << "�ֿ��Ǯ: " << pData->dwStoreMoney << std::endl; 
    m_outFile << "��̫��: " << pData->dwEN << std::endl;
    m_outFile << "������: " << pData->dwIN << std::endl;
    m_outFile << "��̫��: " << pData->dwST << std::endl; 
    m_outFile << "������: " << pData->dwAG << std::endl; 
    m_outFile << "װ��������: " << pData->stEquipData.wEquipAGI << std::endl; 
    m_outFile << "װ���书������: " << pData->stEquipData.wEquipDAM << std::endl; 
    m_outFile << "װ��������: " << pData->stEquipData.wEquipDEF << std::endl; 
    m_outFile << "װ���ڹ�������: " << pData->stEquipData.wEquipPOW << std::endl; 

    int i = 0;
    for(; i < 12; i++) {
        if(pData->stSkill[i].byLevel == 0)
            continue;

        m_outFile << "�书���: " << (DWORD)pData->stSkill[i].wTypeID << "   �书�ȼ�: " << (DWORD)pData->stSkill[i].byLevel << "   �书������: " << pData->stSkill[i].dwProficiency << std::endl; 
    }

    m_outFile.close();        
}

bool CCount::SavePlayerList(const char*filename)
{
	if (!filename)
	{
		rfalse(4, 1, "Count.cpp - SavePlayerList() - !filename");
		return false;
	}

    if(m_PlayerList.size() == 0)
        return false;

    std::ofstream playerfile;

    playerfile.open(filename);
    if(playerfile.fail())
        return false;

    std::string check = "";
    std::list<std::string>::iterator it = m_PlayerList.begin();
    for (; it != m_PlayerList.end(); ) {
        check = *it;
        playerfile << check.c_str() << std::endl;

        it++;
    }

    return true;

}

bool CCount::FillPlayerList(const char*filename)
{
//     dwt::ifstream readfile(filename);
//     if(!readfile.is_open())
//         return false;
// 
//     dwt::stringkey<char [1024]> fillstring;
// 
//     while(!readfile.eof()) {  
//         readfile.getline(fillstring, 1024);
//         if(fillstring != "") 
//         {
//             std::string str = fillstring;
//             m_PlayerList.push_back(str);
//         }
//     }
        
    return true;

}
