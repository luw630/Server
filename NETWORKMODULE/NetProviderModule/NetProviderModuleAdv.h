#pragma once
#include <new_net/netserveradv.h>

typedef unsigned __int64 QWORD;

// Context的使用规则：
// Context是上层给Dnid附加的额外指针数据，有利于快速的定位Dnid对应的实体对象
// 但是需要注意的是：Context的生成和释放规则，以及何时能够安全使用网络消息传回的Context

// （3）安全使用回传的Context：
// 从为Dnid附加Context成功开始到清除Dnid附加的Context为止，该Dnid获取的网络消息中必然附带着设置的Context
// 一般情况下只要操作上层不主动释放Context就可以安全使用它
// 如果上层主动释放Context，那么，不论是否主动清除Dnid附加的Context，都有可能导致访问无效
// 因为在这个时候就算主动清除Dnid附加的Context，但是在这个时刻之前，网络底层仍然有可能生成新的带有现在的Context的数据包
// 当主动清除Dnid附加的Context之后，继续处理网络消息，这时候的Context已经被释放掉了，所以这样做是不安全的！
// 所以想要主动清除一个Context对象的时候，只能清除Dnid附加的Context，当收到该Dnid的断开消息时才能释放Context对象

// （2）Context的清除&释放：
// 就像（3）当中所述的那样，无法主动释放以被附加的Context对象！！！
// 所以安全的释放方式为，收到该Dnid的断开消息时，释放Context对象，当然，你也可以继续的保留该Context
// 同时有一个特殊的情况，就是在退出的时候，可能无法完全的处理所有的网络消息
// 其中也包含了Dnid的断开消息，所以可能导致Context对象无法被释放
// 为了处理这个问题，所有的Context必须在上层有一个管理列表，当退出时主动释放遗留下的对象

// （1）Context的生成&附加：
// 这个可以比较随意的生成，无论何时都可以，但是需要注意的是：
// 一定要处理附加Context到Dnid时失败的情况，因为这时候的Dnid已经无效，无法由底层驱动Context的释放
// 同时需要注意以上的（3）（2）两点，不要主动释放Context，保留Context列表。

// 最后说明两个特殊情况：
// A）：为一个Dnid附加多个Context，这样是不行的，而且尽量不要这样做，
// 一个是因为只能保存一个Context，附加新的Context就破坏了（3）（2）规则！！！
// 再一个是因为创建封包时当前的Context会立即生效，可能会导致无法判断的时序问题
// B）：为多个Dnid附加同一个Context，这样是可以，但是必须自己统计附加次数的计数器！
// 确保在处理所有Dnid的断开消息之前，不会释放掉该Context！！！

// pEntry 必须是 int __thiscall Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, void *pContext); 类型的函数
// 或者 pEntry 是 int __callback Entry(unsigned __int64 qwDnid, LINKSTATUS enumStatus, void *pMsg, void *pContext); 类型的函数

//模块说明：
//基于 NetServer的一个模块，由CNetModule继承而来，负责处理server端的数据
//Initialize 作为初始化函数
class CNetProviderModuleAdv : 
    public CNetServerAdv
{
public:
    // 流程控制函数
    template < typename op >
    int Execution( op _entry )
    {
        SNetServerPacketAdv *pPacket = NULL;

        extern DWORD dwNetRunLimit;
        DWORD segTime = timeGetTime() + dwNetRunLimit;

        int count = 0;

        while ( (int)( segTime - timeGetTime() ) > 0 )
        {
            if ( GetMessage( &pPacket ) == FALSE )
                break;

            bool ret = _entry( pPacket->m_dnidClient, 
                pPacket->m_enumStatus, pPacket->m_pvBuffer, pPacket->m_wLength, pPacket->m_pContext );

            count ++;
 
            if ( pPacket->m_pvBuffer && ret )
                delete pPacket->m_pvBuffer;

            delete pPacket;
        }

        return count;
    }

    int     Destroy(void);

    // 构造析构
    CNetProviderModuleAdv(BOOL bUseVerify = FALSE);
    ~CNetProviderModuleAdv(void);
}; 

