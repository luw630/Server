#pragma once

namespace WND_CUSTOM_OP
{

    enum 
    {
        OP_BEGIN = 0,           // 数据流启动标记！

        OP_END = 1,             // 域结束标记，用于结束多种类型的域定义

        OP_OCX = 4,             // 创建控键，作用于当前域内，第一个参数即为控键类型，后续可存在多个参数，以(attrib=value)表现

        OP_RPC_END = 255
    };

}