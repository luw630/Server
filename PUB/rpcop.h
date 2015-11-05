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
    OP_TRAVERSE_RECORDSET   = 106,      // [����1������ѭ������]�������ݼ��е�����
    OP_CHECK_TRAVERSE_LOOP  = 107,      // [�޲���1]����ѭ���˳�������û�к������ݣ��趨��ѭ����������
    OP_BOX_FIELD            = 108,      // [����1���ֶ���/�ֶ����]���ֶ�ֵװ�뷵����
	OP_TABLE_DATA			= 109,		// ͨ��RPC�����������ݱ��������������ṹ��ŵ������ݵ������С�

    OP_RPC_END = 255
};

enum PARAM_DIR
{
    DIR_IN = 1,
    DIR_OUT = 2,
    DIR_INOUT = 3,
    DIR_RETURN = 4
};
