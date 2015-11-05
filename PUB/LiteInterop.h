/*
#pragma once

#include <atlstr.h>

#define ALIGN_BYTE_4    2
#define ALIGN_BYTE_8    3
#define ALIGN(X, bit) ((((X - 1) >> bit) + 1) << bit)

#define DeclareXcpt(XcptName, u32ErrCode, szInfo) class XcptName : public XcptBase { \
    public : XcptName() : XcptBase(u32ErrCode) {} LPCSTR GetErrInfo() { return szInfo; } };

namespace LiteInterop
{
    typedef void    *lpv;

    typedef char    s8;
    typedef short   s16;
    typedef long    s32;
    typedef unsigned char   u8;
    typedef unsigned short  u16;
    typedef unsigned long   u32;

    struct XcptBase
    {
        virtual LPCSTR GetErrInfo() = 0;

        XcptBase(u32 u32Error) : u32ErrCode(u32Error) {}

        u32 u32ErrCode;
    };

    DeclareXcpt(NullPointer,    0x00000000, "非法使用空指针")
    DeclareXcpt(FullStack,      0x00000001, "缓冲区已满")
    DeclareXcpt(InvalidConvert, 0x00000002, "无效的转换")
    DeclareXcpt(NotInitialize,  0x00000003, "使用的对象没有初始化")
    DeclareXcpt(CurrentIsEnd,   0x00000004, "当前已经是结尾了")

    enum VARIANT_TYPE
    {
        VT_EMPTY = 0,
        VT_NULL = 1,

        VT_U8 = 3,
        VT_S8 = 4,

        VT_U16 = 5,
        VT_S16 = 6,

        VT_U32 = 7,
        VT_S32 = 8,

        VT_BOX = 0x50,

        VT_VARIANT_TYPE = 0x60,
        VT_RPC_OP = 0x61,

        VT_LPCPTR = 0x80,
        VT_LPCSTR = 0x81,
        VT_LPCBSTR = 0x82,
    };

    class CLiteVariant
    {
    public:
        CLiteVariant()
        {
            u8DataType = LiteInterop::VT_EMPTY;
            u16DataSize = 0;
            _lpRefVal = 0;
        }

        CLiteVariant(u8 v8, u16 v16, u32 v32)
        {
            u8DataType = v8;
            u16DataSize = v16;
            _u32Val = v32;
        }

        CLiteVariant(LPCSTR v)
        {
            u8DataType = LiteInterop::VT_LPCSTR;
            u16DataSize = (u16)(strlen(v) + 1);
            _lpStrVal = v;
        }

        CLiteVariant(u32 v)
        {
            u8DataType = LiteInterop::VT_U32;
            u16DataSize = sizeof(v);
            _u32Val = v;
        }

        CLiteVariant(s32 v)
        {
            u8DataType = LiteInterop::VT_S32;
            u16DataSize = sizeof(v);
            _s32Val = v;
        }

        CLiteVariant(LPVOID v, u16 u32Size)
        {
            u8DataType = LiteInterop::VT_LPCPTR;
            u16DataSize = u32Size;
            _lpRefVal = v;
        }

        CLiteVariant(const _variant_t &variant)
        {
            switch (variant.vt)
            {
            case VT_EMPTY:
                u8DataType = VT_EMPTY;
                u16DataSize = 0;
                _u32Val = 0;
                break;

            case VT_NULL:
                u8DataType = VT_NULL;
                u16DataSize = 0;
                _u32Val = 0;
                break;

            case VT_BSTR:
                u8DataType = VT_LPCBSTR;
                u16DataSize = 0;
                _lpBstrVal = variant.bstrVal;
                break;

            case VT_UI1:
            case VT_UI2:
            case VT_UI4:
                u8DataType = VT_U32;
                u16DataSize = 4;
                _u32Val = variant.ulVal;
                break;

            case VT_I1:
            case VT_I2:
            case VT_I4:
                u8DataType = VT_S32;
                u16DataSize = 4;
                _s32Val = variant.lVal;
                break;

            default:
                throw InvalidConvert();
            }
        }

    public:
        operator u32 ()
        {
            if ((u8DataType != VT_U32) && (u8DataType != VT_S32))
                throw InvalidConvert();

            return _u32Val;
        }

        operator s32 ()
        {
            if ((u8DataType != VT_U32) && (u8DataType != VT_S32))
                throw InvalidConvert();

            return _s32Val;
        }

        operator LPCSTR ()
        {
            if (u8DataType == VT_NULL)
                return NULL;

            if (u8DataType != VT_LPCSTR)
                throw InvalidConvert();

            return _lpStrVal;
        }

        operator _bstr_t ()
        {
            if (u8DataType == VT_LPCSTR)
                return _lpStrVal;

            if (u8DataType != VT_LPCBSTR)
                return _lpBstrVal;
                
            throw InvalidConvert();
        }

        operator const _variant_t ()
        {
            _variant_t ret;

            switch (u8DataType)
            {
            case VT_LPCSTR:
                ret = _lpStrVal;
                break;

            case VT_LPCBSTR:
                ret = _lpBstrVal;
                break;

            case VT_U32:
                ret = _u32Val;
                break;

            case VT_S32:
                ret = _s32Val;
                break;

            default:
                throw InvalidConvert();
            }

            return ret;
        }

    public:
        u8  u8DataType;
        u8  u8Reserved;
        u16 u16DataSize;

        union 
        {
            u8 _u8Val;
            s8 _s8Val;
            u16 _u16Val;
            s16 _s16Val;
            u32 _u32Val;
            s32 _s32Val;
            float _fltVal;
            LPCVOID _lpRefVal;
            LPCSTR _lpStrVal;
            BSTR _lpBstrVal;
        };
    };

    class CLiteInterop
    {
    public:
        CLiteInterop()
        {
            u32InteropMaxSize = 0;
            lpInteropData = NULL;
        }

    public:
        CLiteInterop(u32 u32Size)
        {
            u32InteropMaxSize = 0;
            lpInteropData = NULL;

            Alloc(u32Size);
        }

    public:
        CLiteInterop(CLiteInterop *lpLiop)
        {
            u32InteropMaxSize = 0;
            lpInteropData = lpLiop;

            lpInteropData->u32InteropMaxSize = lpInteropData->u32InteropCurSize;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

        CLiteInterop(lpv lpData, u32 i32Size)
        {
            u32InteropMaxSize = sizeof(CLiteInterop);
            lpInteropData = (CLiteInterop *)(new u8[32 + i32Size] + 32);
            memcpy(lpInteropData, lpData, i32Size);

            lpInteropData->u32InteropMaxSize = i32Size;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

    public:
        ~CLiteInterop() 
        {
            if (u32InteropMaxSize == 0)
                return;

            delete (((u8 *) lpInteropData) - 32);
        }

    public:
        void Alloc(u32 u32Size)
        {
            if ((lpInteropData != 0) || (u32InteropMaxSize != 0))
                return;

            lpInteropData = (CLiteInterop *)(new u8[32 + sizeof(CLiteInterop) + u32Size] + 32);
            u32InteropMaxSize = sizeof(CLiteInterop) + u32Size;
            lpInteropData->u32InteropMaxSize = 0;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

        u32 GetValidSize()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            return lpInteropData->u32InteropCurSize;
        }

        lpv GetValidAddr()
        {
            return lpInteropData;
        }

        lpv AddHeader(lpv lpHeader, u32 &u32Size)
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32Size > 32)
                throw FullStack();

            u32 aligned = ALIGN(u32Size, ALIGN_BYTE_4);

            u8 *lpu8 = ((u8 *) lpInteropData) - aligned;
            memcpy(lpu8, lpHeader, u32Size);

            u32Size = aligned + lpInteropData->u32InteropCurSize;

            return lpu8;
        }

        void Rewind()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

    public:
        void PushLiteVariant(CLiteVariant &Liop)
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32InteropMaxSize == 0)
                throw NotInitialize();

            u8 *lpu8 = (u8 *)lpInteropData;

            lpu8 += lpInteropData->u32InteropCurSize;

            if (lpInteropData->u32InteropCurSize + sizeof(CLiteVariant) > u32InteropMaxSize)
                throw FullStack();

            lpInteropData->u32InteropCurSize += sizeof(CLiteVariant);
            memcpy(lpu8, &Liop, sizeof(CLiteVariant));

            if (Liop.u8DataType == LiteInterop::VT_LPCBSTR)
            {
                CLiteVariant *lpLvar = (CLiteVariant *)lpu8;

                lpu8 += sizeof(CLiteVariant);

                // convert bstr to ansi-str
                CString temp = lpLvar->_lpBstrVal;

                lpLvar->u8DataType = LiteInterop::VT_LPCSTR;
                lpLvar->u16DataSize = temp.GetLength() + 1;

                if (lpInteropData->u32InteropCurSize + lpLvar->u16DataSize > u32InteropMaxSize)
                    throw FullStack();

                lpLvar->_lpRefVal = lpu8;

                lpInteropData->u32InteropCurSize += lpLvar->u16DataSize;
                memcpy(lpu8, temp.GetBuffer(), lpLvar->u16DataSize);
            }
            else if (Liop.u8DataType >= LiteInterop::VT_LPCPTR)
            {
                CLiteVariant *lpLvar = (CLiteVariant *)lpu8;

                lpu8 += sizeof(CLiteVariant);

                if (lpInteropData->u32InteropCurSize + lpLvar->u16DataSize > u32InteropMaxSize)
                    throw FullStack();

                lpLvar->_lpRefVal = lpu8;

                lpInteropData->u32InteropCurSize += lpLvar->u16DataSize;
                memcpy(lpu8, Liop._lpRefVal, lpLvar->u16DataSize);
            }
        }

        operator CLiteVariant & ()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32InteropMaxSize > sizeof(CLiteVariant))
                throw CurrentIsEnd();

            u8 *lpu8 = (u8 *)lpInteropData;

            lpu8 += lpInteropData->u32InteropCurSize;

            if (lpInteropData->u32InteropCurSize + sizeof(CLiteVariant) > lpInteropData->u32InteropMaxSize)
                throw FullStack();

            lpInteropData->u32InteropCurSize += sizeof(CLiteVariant);
            CLiteVariant &Liop = (CLiteVariant &)*lpu8;

            if (Liop.u8DataType >= LiteInterop::VT_LPCPTR)
            {
                lpu8 += sizeof(CLiteVariant);
                Liop._lpRefVal = lpu8;

                if (lpInteropData->u32InteropCurSize + Liop.u16DataSize > lpInteropData->u32InteropMaxSize)
                    throw FullStack();

                lpInteropData->u32InteropCurSize += Liop.u16DataSize;
            }

            return Liop;
        }

    public:
        CLiteInterop & operator () (LPCSTR lpStr)
        {
            PushLiteVariant( CLiteVariant(lpStr) );
            return *this;
        }

        CLiteInterop & operator () (u32 u32Val)
        {
            PushLiteVariant( CLiteVariant(u32Val) );
            return *this;
        }

        CLiteInterop & operator () (s32 s32Val)
        {
            PushLiteVariant( CLiteVariant(s32Val) );
            return *this;
        }

        CLiteInterop & operator () (int iVal)
        {
            PushLiteVariant( CLiteVariant((s32)iVal) );
            return *this;
        }

        CLiteInterop & operator () (LPVOID v, u16 u32Size)
        {
            PushLiteVariant( CLiteVariant(v, u32Size) );
            return *this;
        }

        CLiteInterop & operator [] (u32 opType)
        {
            PushLiteVariant( CLiteVariant(VT_RPC_OP, 4, opType) );
            return *this;
        }

    public:
        u32 u32InteropMaxSize;

        union
        {
            CLiteInterop *lpInteropData;
            u32 u32InteropCurSize;
        };
    };
};
*/
/////////////////////////////////////////////////////////
#pragma once

#include <atlstr.h>

const int ALIGN_BYTE_4      2
const int ALIGN_BYTE_8      3
const int ALIGN( X, bit ) ( ( ( (X - 1) >> bit ) + 1 ) << bit )

#define DeclareXcpt(XcptName, u32ErrCode, szInfo) class XcptName : public XcptBase { \
    public : XcptName() : XcptBase(u32ErrCode) {} LPCSTR GetErrInfo() { return szInfo; } };

namespace LiteInterop
{
    typedef void            *lpv;

    typedef char            s8;
    typedef short           s16;
    typedef long            s32;
    typedef unsigned char   u8;
    typedef unsigned short  u16;
    typedef unsigned long   u32;

    struct XcptBase
    {
        virtual LPCSTR GetErrInfo() = 0;

        XcptBase(u32 u32Error) : u32ErrCode(u32Error) {}

        u32 u32ErrCode;
    };

    DeclareXcpt( NullPointer,    0x00000000, "非法使用空指针"       )
    DeclareXcpt( FullStack,      0x00000001, "缓冲区已满"           )
    DeclareXcpt( InvalidConvert, 0x00000002, "无效的转换"           )
    DeclareXcpt( NotInitialize,  0x00000003, "使用的对象没有初始化" )
    DeclareXcpt( CurrentIsEnd,   0x00000004, "当前已经是结尾了"     )

    // 变量类型！
    enum VARIANT_TYPE
    {
        VT_EMPTY        = 0,        // 完全的空类型，变量未初始化
        VT_NULL         = 1,        // 空类型，表示没有数据

        VT_DWORD        = 0x10,     // 4字节无符号数据
        VT_INTEGER      = 0x11,     // 32位带符号整数
        VT_FLOAT        = 0x12,     // 单精度浮点数

        VT_QWORD        = 0x20,     // 8字节无符号数据
        VT_LARGINTEGER  = 0x21,     // 64位带符号大整数
        VT_DOUBLE       = 0x22,     // 双精度浮点数

        VT_POINTER      = 0x40,     // 指针类型数据
        VT_STRING       = 0x41,     // 标准字符串
        VT_BSTR         = 0x42,     // BSTR字符串
        VT_UTF8         = 0x43,     // UTF8字符串

        VT_RPC_OP       = 0xff,     // 特殊控制字符
    };

    class CLiteVariant
    {
    public:
        CLiteVariant()
        {
            u8DataType = LiteInterop::VT_EMPTY;
            u16DataSize = 0;
            _lpRefVal = 0;
        }

        CLiteVariant( u8 v8, u16 v16, u32 v32 )
        {
            u8DataType = v8;
            u16DataSize = v16;
            _u32Val = v32;
        }

        CLiteVariant( LPCSTR v )
        {
            u8DataType = LiteInterop::VT_STRING;
            u16DataSize = (u16)(strlen(v) + 1);
            _lpStrVal = v;
        }

        template < typename v_type, 
            int v_size = sizeof( static_cast< v_type > ),
            bool is_signed = ( static_cast< v_type >( -1 ) < 0 ) > 
            CLiteVariant( v_type ) 
        { 
            #error
        };

        template < typename v_type, 4, false > 
            CLiteVariant( v_type v )
        {
            u8DataType = LiteInterop::VT_DWORD;
            u16DataSize = 4;
            _u32Val = v;
        }

        CLiteVariant( int v )
        {
            u8DataType = LiteInterop::VT_INT32;
            u16DataSize = sizeof(v);
            _u32Val = v;
        }

        CLiteVariant( DWORD v )
        {
            u8DataType = LiteInterop::VT_DWORD;
            u16DataSize = sizeof( v );
            _u32Val = v;
        }

        CLiteVariant(s32 v)
        {
            u8DataType = LiteInterop::VT_S32;
            u16DataSize = sizeof(v);
            _s32Val = v;
        }

        CLiteVariant(LPVOID v, u16 u32Size)
        {
            u8DataType = LiteInterop::VT_LPCPTR;
            u16DataSize = u32Size;
            _lpRefVal = v;
        }

        CLiteVariant(const _variant_t &variant)
        {
            switch (variant.vt)
            {
            case VT_EMPTY:
                u8DataType = VT_EMPTY;
                u16DataSize = 0;
                _u32Val = 0;
                break;

            case VT_NULL:
                u8DataType = VT_NULL;
                u16DataSize = 0;
                _u32Val = 0;
                break;

            case VT_BSTR:
                u8DataType = VT_LPCBSTR;
                u16DataSize = 0;
                _lpBstrVal = variant.bstrVal;
                break;

            case VT_UI1:
            case VT_UI2:
            case VT_UI4:
                u8DataType = VT_U32;
                u16DataSize = 4;
                _u32Val = variant.ulVal;
                break;

            case VT_I1:
            case VT_I2:
            case VT_I4:
                u8DataType = VT_S32;
                u16DataSize = 4;
                _s32Val = variant.lVal;
                break;

            default:
                throw InvalidConvert();
            }
        }

    public:
        operator u32 ()
        {
            if ((u8DataType != VT_U32) && (u8DataType != VT_S32))
                throw InvalidConvert();

            return _u32Val;
        }

        operator s32 ()
        {
            if ((u8DataType != VT_U32) && (u8DataType != VT_S32))
                throw InvalidConvert();

            return _s32Val;
        }

        operator LPCSTR ()
        {
            if (u8DataType == VT_NULL)
                return NULL;

            if (u8DataType != VT_LPCSTR)
                throw InvalidConvert();

            return _lpStrVal;
        }

        operator _bstr_t ()
        {
            if (u8DataType == VT_LPCSTR)
                return _lpStrVal;

            if (u8DataType != VT_LPCBSTR)
                return _lpBstrVal;
                
            throw InvalidConvert();
        }

        operator const _variant_t ()
        {
            _variant_t ret;

            switch (u8DataType)
            {
            case VT_LPCSTR:
                ret = _lpStrVal;
                break;

            case VT_LPCBSTR:
                ret = _lpBstrVal;
                break;

            case VT_U32:
                ret = _u32Val;
                break;

            case VT_S32:
                ret = _s32Val;
                break;

            default:
                throw InvalidConvert();
            }

            return ret;
        }

    public:
        u8  u8DataType;
        u8  u8Reserved;
        u16 u16DataSize;

        union 
        {
            u8 _u8Val;
            s8 _s8Val;
            u16 _u16Val;
            s16 _s16Val;
            u32 _u32Val;
            s32 _s32Val;
            float _fltVal;
            LPCVOID _lpRefVal;
            LPCSTR _lpStrVal;
            BSTR _lpBstrVal;
        };
    };

    class CLiteInterop
    {
    public:
        CLiteInterop()
        {
            u32InteropMaxSize = 0;
            lpInteropData = NULL;
        }

    public:
        CLiteInterop(u32 u32Size)
        {
            u32InteropMaxSize = 0;
            lpInteropData = NULL;

            Alloc(u32Size);
        }

    public:
        CLiteInterop(CLiteInterop *lpLiop)
        {
            u32InteropMaxSize = 0;
            lpInteropData = lpLiop;

            lpInteropData->u32InteropMaxSize = lpInteropData->u32InteropCurSize;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

        CLiteInterop(lpv lpData, u32 i32Size)
        {
            u32InteropMaxSize = sizeof(CLiteInterop);
            lpInteropData = (CLiteInterop *)(new u8[32 + i32Size] + 32);
            memcpy(lpInteropData, lpData, i32Size);

            lpInteropData->u32InteropMaxSize = i32Size;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

    public:
        ~CLiteInterop() 
        {
            if (u32InteropMaxSize == 0)
                return;

            delete (((u8 *) lpInteropData) - 32);
        }

    public:
        void Alloc(u32 u32Size)
        {
            if ((lpInteropData != 0) || (u32InteropMaxSize != 0))
                return;

            lpInteropData = (CLiteInterop *)(new u8[32 + sizeof(CLiteInterop) + u32Size] + 32);
            u32InteropMaxSize = sizeof(CLiteInterop) + u32Size;
            lpInteropData->u32InteropMaxSize = 0;
            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

        u32 GetValidSize()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            return lpInteropData->u32InteropCurSize;
        }

        lpv GetValidAddr()
        {
            return lpInteropData;
        }

        lpv AddHeader(lpv lpHeader, u32 &u32Size)
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32Size > 32)
                throw FullStack();

            u32 aligned = ALIGN(u32Size, ALIGN_BYTE_4);

            u8 *lpu8 = ((u8 *) lpInteropData) - aligned;
            memcpy(lpu8, lpHeader, u32Size);

            u32Size = aligned + lpInteropData->u32InteropCurSize;

            return lpu8;
        }

        void Rewind()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            lpInteropData->u32InteropCurSize = sizeof(CLiteInterop);
        }

    public:
        void PushLiteVariant(CLiteVariant &Liop)
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32InteropMaxSize == 0)
                throw NotInitialize();

            u8 *lpu8 = (u8 *)lpInteropData;

            lpu8 += lpInteropData->u32InteropCurSize;

            if (lpInteropData->u32InteropCurSize + sizeof(CLiteVariant) > u32InteropMaxSize)
                throw FullStack();

            lpInteropData->u32InteropCurSize += sizeof(CLiteVariant);
            memcpy(lpu8, &Liop, sizeof(CLiteVariant));

            if (Liop.u8DataType == LiteInterop::VT_LPCBSTR)
            {
                CLiteVariant *lpLvar = (CLiteVariant *)lpu8;

                lpu8 += sizeof(CLiteVariant);

                // convert bstr to ansi-str
                CString temp = lpLvar->_lpBstrVal;

                lpLvar->u8DataType = LiteInterop::VT_LPCSTR;
                lpLvar->u16DataSize = temp.GetLength() + 1;

                if (lpInteropData->u32InteropCurSize + lpLvar->u16DataSize > u32InteropMaxSize)
                    throw FullStack();

                lpLvar->_lpRefVal = lpu8;

                lpInteropData->u32InteropCurSize += lpLvar->u16DataSize;
                memcpy(lpu8, temp.GetBuffer(), lpLvar->u16DataSize);
            }
            else if (Liop.u8DataType >= LiteInterop::VT_LPCPTR)
            {
                CLiteVariant *lpLvar = (CLiteVariant *)lpu8;

                lpu8 += sizeof(CLiteVariant);

                if (lpInteropData->u32InteropCurSize + lpLvar->u16DataSize > u32InteropMaxSize)
                    throw FullStack();

                lpLvar->_lpRefVal = lpu8;

                lpInteropData->u32InteropCurSize += lpLvar->u16DataSize;
                memcpy(lpu8, Liop._lpRefVal, lpLvar->u16DataSize);
            }
        }

        operator CLiteVariant & ()
        {
            if (lpInteropData == NULL)
                throw NullPointer();

            if (u32InteropMaxSize > sizeof(CLiteVariant))
                throw CurrentIsEnd();

            u8 *lpu8 = (u8 *)lpInteropData;

            lpu8 += lpInteropData->u32InteropCurSize;

            if (lpInteropData->u32InteropCurSize + sizeof(CLiteVariant) > lpInteropData->u32InteropMaxSize)
                throw FullStack();

            lpInteropData->u32InteropCurSize += sizeof(CLiteVariant);
            CLiteVariant &Liop = (CLiteVariant &)*lpu8;

            if (Liop.u8DataType >= LiteInterop::VT_LPCPTR)
            {
                lpu8 += sizeof(CLiteVariant);
                Liop._lpRefVal = lpu8;

                if (lpInteropData->u32InteropCurSize + Liop.u16DataSize > lpInteropData->u32InteropMaxSize)
                    throw FullStack();

                lpInteropData->u32InteropCurSize += Liop.u16DataSize;
            }

            return Liop;
        }

    public:
        CLiteInterop & operator () (LPCSTR lpStr)
        {
            PushLiteVariant( CLiteVariant(lpStr) );
            return *this;
        }

        CLiteInterop & operator () (u32 u32Val)
        {
            PushLiteVariant( CLiteVariant(u32Val) );
            return *this;
        }

        CLiteInterop & operator () (s32 s32Val)
        {
            PushLiteVariant( CLiteVariant(s32Val) );
            return *this;
        }

        CLiteInterop & operator () (int iVal)
        {
            PushLiteVariant( CLiteVariant((s32)iVal) );
            return *this;
        }

        CLiteInterop & operator () (LPVOID v, u16 u32Size)
        {
            PushLiteVariant( CLiteVariant(v, u32Size) );
            return *this;
        }

        CLiteInterop & operator [] (u32 opType)
        {
            PushLiteVariant( CLiteVariant(VT_RPC_OP, 4, opType) );
            return *this;
        }

    public:
        u32 u32InteropMaxSize;

        union
        {
            CLiteInterop *lpInteropData;
            u32 u32InteropCurSize;
        };
    };
};
