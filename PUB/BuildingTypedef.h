#pragma once

#include "networkmodule/itemtypedef.h"

/*/ 
����ϵͳ������
    �µĶ������ͣ��ɵ�ԭ�ж������ģʽ���ۺϳ�Ϊ����ϵͳ��
    �������Ż�ӵ�����¼����ص㣺
    1�����
    2����������㣨��̺���ڸ���ʱ����������ߣ�
    3����θı��ͼ�赲���ԣ����߲��ı䣨ʵ��̨�ף�Χǽ���ſ򣬳���ڡ�������
    4���Ƿ��Կɽ����������ʽ����
    5������4���趨������������Ƿ�����ƻ����;öȡ�

    ���������ͬ�����ƣ�
    ���������������ͬ����ʽ��ͬ��
    ������������Ϊ��ȫ�ֱ�ţ�XY���꣬�������ţ��;ö�
    ��ʣ�µ����ݸ��ݲ�������ͬ����Ϣ��С������16���ֽ�����
/*/

// ������Ļ�������
struct SBuildingBaseData
{
    WORD index;         // ������������ţ�ֱ���Ա�ŵĴ�С���ж�ͼ�㣬1w����Ϊ��̺�㣬1~2wΪ����㣬1~3wΪ������������
    WORD graph;         // ���������ͼƬ���

    QWORD tileMask;     // 8x8�赲��Ϣ��

    // ������ͼƬ�����������Ӧ�����赲����
    BYTE centerX, centerY; 

    BYTE buildType;     // �������ͣ���ʱ���Զ���������ʱ���������ĳɳ�����Ҫ����Ͷ������������ģ�
    BYTE buildSpeed;    // �����ٶȣ���λΪ���ӣ�
    BYTE material;      // ���󽨲����ͣ�ͬʱҲ�����˸ò��ʵĿ�������������������ֻ�趨���Ĵ����ͣ�������ͬ�����͵Ĳ��ʻᵼ�½��������ĸı䣩
    BYTE maintenance;   // �ɳ�ֵ�ٶȣ�Ϊ�����ٶȵı��ʣ�10λ�ģ�

    DWORD maxHP;        // �;ö����ֵ��

    union
    {
        struct Defender
        {
            BYTE type;      // ������������
            BYTE range;     // ��������
            BYTE atkSpeed;  // ����Ƶ��
            BYTE reserve;   // ����
            WORD damage;    // ���������ɵ��˺�ֵ
        };

        struct Scripter
        {
            DWORD scriptId; // ����ű�
        };

        struct Decker
        {
        };
    };
};

// ���ڱ���Ľ��������� 40BYTE
struct SBuildingSaveData : 
    public SItemBase    // ������������Ϊ��������
{
    // ---------- 8byte -----------
    // WORD wIndex;         ��������
    // BYTE size  : 6;      ��Ҫ��������ݴ�С!
    // BYTE flags : 2;      ������״̬�������������У�����У�ͣ�ã�
    // QWORD uniqueId;      Ψһ��ʶ��,��������������!

    // ---------- 8byte -----------
    WORD curX, curY;        // �����������ڵ�λ��
    DWORD curHP;            // ��ǰ�;öȡ�

    // ---------- 12byte -----------
    WORD scriptValue[6];    // 6����������!

    // ---------- 12byte -----------
    BYTE ownerType : 2;     // ӵ�������ͣ����⡢���ˡ����ɡ�������
    BYTE openType : 1;      // �Ƿ���⿪�ţ�δ���š�������
    char owner[11];         // ӵ��������!
};

// ������Ϸ�еĽ��������ݽṹ
struct SBuildingData :
    public SBuildingBaseData,
    public SBuildingSaveData
{
};
