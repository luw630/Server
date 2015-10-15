#pragma once

#pragma comment (lib, "desenc.lib")

extern "C" void _set_key_encipher();

extern "C" void _des_encrypt(BYTE *block);

#ifdef USE_CRC

#pragma comment (lib, "crc32.lib")

extern "C" DWORD _GetCrc32(LPVOID buf, DWORD size);

extern "C" DWORD _get_dval(DWORD val);

#endif

class cDirectNetEncryption : public iDirectNetCryption
{
public:
	cDirectNetEncryption()
    {
	    _set_key_encipher();
    }

#ifdef USE_CRC

    UINT32 CRC32_compute(/* [in] */PVOID pvBuffer, DWORD dwLength) 
    { 
        return _GetCrc32(pvBuffer, dwLength); 
    }

#endif

    void DES_encrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength)
    {
	    BYTE *tmp = (BYTE *)pvBuffer;
	    for(long i=0,j=dwLength>>3; i<j; ++i,tmp+=8) 
            _des_encrypt(tmp);
    }
};

