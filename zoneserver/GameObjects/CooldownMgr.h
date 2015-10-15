#pragma once

#include "EventManager.h"

// 思路：
// 自定义类型分类（字符串key），来区分冷却器类型

// 关联型冷却的做法：
// 比如A系列技能公共冷却时间1s，A-a技能冷却时间2s
// 那么在A-a技能启动后，A系列所有技能都启动冷却1s，同时A-a技能启动冷却时间max（1s,2s）
// 所以反过来，在服务器上，需要启动2个冷却器（A系列公共冷却器，A-a独立技能冷却器）
// 在服务器收到技能启动请求时，首先判断当前系列公共冷却时间是否还存在，再判断当前技能独立冷却时间是否还存在

// 关于冷却器的保存：
// 部分事件的冷却时间会存在于较长时间，比如魔兽里炉石之类的东西
// 但是，除了这类型的事件，存在一些需要保存的长时间的BUFF，比如长时间中毒，死亡复活惩罚等。。。
// 所以，为了保证逻辑的一致性，冷却器就不考虑保存了

// 炉石之类的道具冷却可以有2种做法：
// 1）脚本类道具可以自己在道具数据区，记录上次使用的时间，用来计算冷却剩余时间
// 2）使用BUFF来代替道具冷却时间（在这里推荐使用第一种方式）

// 冷却器的事件处理，基于事件管理器
// 如不需回调（只用于使用前检测逻辑，那就不需要将这个冷却器放入事件管理器）（owner为NULL）
// 如需要回调（外部传入派生后的自定义冷却器，则需要用事件管理器来激活事件）（owner为实际值）
struct Cooler : EventMgr::EventBase {
    // 注意，由于普通版本不会加入事件管理，所以不需要考虑冷却器的销毁和事件中断问题
    // 而派生类必定为加入事件管理，所以，销毁逻辑要做正确处理
    // 同时事件中断需要调用CooldownMgr::Deactive，而不是EventMgr::ResetEvent ！！！
    std::string type;
    DWORD segment, offset;
    class CooldownMgr *owner;
};

class CooldownMgr {
    // 冷却器在运行过程中不会销毁（因为冷却器类型也是有限的，同时它本身就不占用多少空间）
    typedef std::map< std::string, Cooler* > COOLDOWNMAP;
    COOLDOWNMAP coolerMap;

public:
    // 启动一个冷却器，可以用外部的冷却器逻辑来代替默认的冷却器，如果目标冷却器还在冷却中，则函数失败返回
    // 注意，注册冷却器只能注册一次，假如使用了自定义冷却器，那就不能再修改为其他的了！
    // 这样做主要是为了保证代码逻辑简单！
    BOOL Active( std::string &type, DWORD remain, Cooler *cooler = NULL ); 

    // 检测指定类型的冷却器的剩余时间（单位毫秒）
    DWORD Check( std::string &type ); 

    // 强制关闭一个冷却器
    BOOL Deactive( std::string &type ); 
};