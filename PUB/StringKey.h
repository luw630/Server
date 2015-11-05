#pragma once

#include "new_net/MD5Lib.H"

typedef struct { __int32 i32[4]; } DQWORD;

#define ZeroDQWord( dqw ) ((dqw).i32[0] = (dqw).i32[1] = (dqw).i32[2] = (dqw).i32[3] = 0)
#define IsZeroDQWord( dqw ) ((dqw).i32[0] == 0 && (dqw).i32[1] == 0 && (dqw).i32[2] == 0 && (dqw).i32[3] == 0)

extern "C" DWORD _GetCrc32(LPVOID buf, DWORD size);

namespace dwt
{

template <class _Key>
    class StringKey_CRC32
    {
    public:
        StringKey_CRC32()
        {
            memset(_key, 0, sizeof(_Key));
            _crc32 = 0;
        }

        StringKey_CRC32(LPCSTR Key)
        {
            memset(_key, 0, sizeof(_Key));
            strcpy(_key, Key, sizeof(_Key));
            _crc32 = 0;
        }

        bool operator < (const StringKey_CRC32<_Key> &key) const
        {
            return CRC32() < key.CRC32();
        }

        bool operator == (const StringKey_CRC32<_Key> &key) const
        {
            return CRC32 == key.CRC32();
        }

        operator LPSTR ()
        {
            return _key;
        }

        unsigned __int32 CRC32() const 
        {
            if (_crc32 != 0)
                return _crc32;

            *(LPDWORD)&_crc32 = _GetCrc32((LPVOID)_key, sizeof(_Key));
            return _crc32;
        }

        __int8 _key[sizeof(_Key)];

        unsigned __int32 _crc32;
    };

    //以下这个是专门用来处理以数据流来作为关键字的对象
    class LongStringKey_CRC32
    {
    public:
        LongStringKey_CRC32()
        {
            _str = NULL;
            _len = 0;
            _crc32 = 0;
        }

        LongStringKey_CRC32(LPCSTR szKey)
        {
            _crc32 = 0;

            _len = (DWORD)(strlen(szKey) + 1);
            _str = new char[_len + sizeof(DQWORD) * 2];

            DQWORD *GuardArray = (DQWORD *)_str;
            ZeroDQWord(*GuardArray);

            GuardArray = (DQWORD *)(_str + _len);
            ZeroDQWord(*GuardArray);

            memcpy(_str + sizeof(DQWORD), szKey, _len);
        }

        ~LongStringKey_CRC32()
        {
            ReleaseBuffer();
        }

        bool operator < (const LongStringKey_CRC32 &key) const
        {
            return CRC32() < key.CRC32();
        }

        bool operator == (const LongStringKey_CRC32 &key) const
        {
            return CRC32() == key.CRC32();
        }

        operator LPSTR ()
        {
            return _str + sizeof(DQWORD);
        }

        DWORD CRC32() const
        {
            if (_crc32 != 0) 
                return _crc32;

            *(LPDWORD)&_crc32 = _GetCrc32(_str, _len);
            return _crc32;
        }

        void ReleaseBuffer()
        {
            if (_str == NULL)
                return;

            DQWORD *GuardArray = (DQWORD *)_str;
            if (!IsZeroDQWord(*GuardArray))
            {
                // overflow !!!
            }

            GuardArray = (DQWORD *)(_str + _len);
            if (!IsZeroDQWord(*GuardArray))
            {
                // overflow !!!
            }

            delete _str;

            _str = NULL;
            _len = 0;
            _crc32 = 0;
        }

    private:
        LPSTR _str;
        DWORD _len;
        DWORD _crc32;
    };

    class LongStringKey_MD5
    {
    public:
        LongStringKey_MD5()
        {
            _str = NULL;
            _len = 0;
            ZeroDQWord(_md5);
        }

        LongStringKey_MD5(LPCSTR szKey)
        {
            ZeroDQWord(_md5);

            _len = (DWORD)(strlen(szKey) + 1);
            _str = new char[_len + sizeof(DQWORD) * 2];

            DQWORD *GuardArray = (DQWORD *)_str;
            ZeroDQWord(*GuardArray);

            GuardArray = (DQWORD *)(_str + _len);
            ZeroDQWord(*GuardArray);

            memcpy(_str + sizeof(DQWORD), szKey, _len);
        }

        ~LongStringKey_MD5()
        {
            ReleaseBuffer();
        }

        bool operator < (const LongStringKey_MD5 &key) const
        {
            return CompareMD5(MD5(), key.MD5()) < 0;
        }

        bool operator == (const LongStringKey_MD5 &key) const
        {
            return CompareMD5(MD5(), key.MD5()) == 0;
        }

        operator LPSTR ()
        {
            return _str + sizeof(DQWORD);
        }

        static int CompareMD5(const DQWORD &src, const DQWORD &dest)
        {
            return memcmp(&src, &dest, sizeof(DQWORD));
        }

        const DQWORD &MD5() const
        {
            if (!IsZeroDQWord(_md5))
                return _md5;

            HASH_MD5((unsigned char *)_str, _len, (unsigned char *)&_md5);
            return _md5;
        }

        void ReleaseBuffer()
        {
            if (_str == NULL)
                return;

            DQWORD *GuardArray = (DQWORD *)_str;
            if (!IsZeroDQWord(*GuardArray))
            {
                // overflow !!!
            }

            GuardArray = (DQWORD *)(_str + _len);
            if (!IsZeroDQWord(*GuardArray))
            {
                // overflow !!!
            }

            delete _str;

            _str = NULL;
            _len = 0;
            ZeroDQWord(_md5);
        }

    private:
        LPSTR _str;
        DWORD _len;
        DQWORD _md5;
    };
};