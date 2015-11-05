#pragma once

#pragma comment (lib, "desdec.lib")

extern "C" void _des_decrypt(BYTE *block);

extern "C" void _set_key_decipher();

#ifdef USE_CRC

#pragma comment (lib, "crc32.lib")

extern "C" DWORD _GetCrc32(LPVOID buf, DWORD size);

extern "C" DWORD _get_dval(DWORD val);

#endif

#ifdef EXTRA

extern "C" void _set_key_enc(BYTE *_key);
extern "C" void _set_key_dec(BYTE *_key);

#endif

class cDirectNetDecryption : public iDirectNetCryption
{
public:
	cDirectNetDecryption()
    {
	    _set_key_decipher();
    }

#ifdef USE_CRC

    UINT32 CRC32_compute(/* [in] */PVOID pvBuffer, DWORD dwLength) 
    { 
        return _GetCrc32(pvBuffer, dwLength); 
    }

#endif

    void DES_decrypt(/* [in|out] */PVOID pvBuffer, DWORD dwLength)
    {
	    BYTE *tmp = (BYTE *)pvBuffer;
	    for(long i=0,j=dwLength>>3; i<j; ++i,tmp+=8) 
            _des_decrypt(tmp);
    }
};

