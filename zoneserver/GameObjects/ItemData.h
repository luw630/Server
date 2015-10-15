#pragma once

#include "networkmodule\itemtypedef.h"

#define MakeLifeTime(m) ((m) * 60000)
#define MAX_ITEMSCOPE   65535

#include <list>
using namespace std;

class CItemData
{
public:
	static int LoadItemData( LPCSTR filename, bool crcInit = true, BOOL refresh = FALSE );        // �������е��ߵ���������
	static int LoadSpecialItemData( LPCSTR filename, BOOL refresh = FALSE );                      // ��ȡ���������Ե���Ʒ
	/*static SpecialItem* GetSpecialItem( DWORD id );*/
    /*static const SItemData *GetItemData( int index );                       // ��ȡָ�����ߵ���������*/
	static int LoadSpecialItemAttribute( LPCSTR filename, BOOL refresh = FALSE );                      // ��ȡ���������Ե���Ʒ
	/*static SGemAtt* GetSpecialItemAttribute( DWORD id );*/
    // ����&���&ɾ�����ߵ���������
    // itemData == NULL �ָ�Ĭ������
    // itemData.wItemID == index ��������
    // itemData.wItemID == -1 ɾ��������
    /*static BOOL UpdateItemData( int index, const SItemData *itemData = NULL, BOOL fullUpdate = false ); */

    // ��Ҫ���ڿͻ��˵���������Ч�Լ�⣡
    /*static std::pair< DWORD, const SItemData* >* GetRawData( int index );  */
};

