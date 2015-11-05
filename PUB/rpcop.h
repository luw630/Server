#pragma once

enum RPC_OP
{
    OP_NULL_ = 0,
    OP_REG_VARIANT = 1,
    OP_ASSIGN = 2,
    OP_RESET = 3,

    OP_PREPARE_STOREDPROC = 10,
    OP_BEGIN_PARAMS = 11,
    OP_END_PARAMS = 12,
    OP_CALL_STOREDPROC = 13,

    OP_INIT_RETBOX = 100,
    OP_BOX_PARAM = 101,
    OP_BOX_VARIANT = 102,
    OP_RETURN_BOX = 103,
    OP_BEGIN_ERRHANDLE = 104,
    OP_END_ERRHANDLE = 105,
    OP_TRAVERSE_RECORDSET   = 106,      // [参数1：有限循环次数]遍历数据集中的数据
    OP_CHECK_TRAVERSE_LOOP  = 107,      // [无参数1]遍历循环退出条件：没有后续数据，设定的循环次数结束
    OP_BOX_FIELD            = 108,      // [参数1：字段名/字段序号]将字段值装入返回箱
	OP_TABLE_DATA			= 109,		// 通过RPC操作返回数据表，结果集中以特殊结构存放单行数据的所有列。

    OP_RPC_END = 255
};

enum PARAM_DIR
{
    DIR_IN = 1,
    DIR_OUT = 2,
    DIR_INOUT = 3,
    DIR_RETURN = 4
};
