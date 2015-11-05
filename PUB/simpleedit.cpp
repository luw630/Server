#include <windows.h>
#include "simpleedit.h"

#define ENDL '\r'

// *
// extended edit function
bool edit_init(unsigned __int32 &len, 
               unsigned __int32 &pos, 
               unsigned __int32 &status, 
               const unsigned __int32 size, 
               __int8 *pbuffer)
{
    if (IsBadReadPtr(pbuffer, size))
        return false;

    len = 0;
    pos = 0;
    status = 0;

    memset(pbuffer, 0, size);

    return true;
}

int edit_segline(const unsigned __int32 len, 
                 const unsigned __int32 pos,
                 const unsigned __int32 size, 
                 const __int8 *pbuffer)
{
    if (len >= size) return false;
    if (pos > len) return false;

    const unsigned __int8 *pcheck = (const unsigned __int8 *)&pbuffer[pos];

    while (true)
    {
        if (pcheck <= (const unsigned __int8 *)pbuffer)
            return 0;

        if (pcheck[-1] == ENDL)
            break;

        pcheck--;
    }

    return (int)(pcheck - (const unsigned __int8 *)pbuffer);
}

int edit_endline(const unsigned __int32 len, 
                 const unsigned __int32 pos,
                 const unsigned __int32 size, 
                 const __int8 *pbuffer)
{
    if (len >= size) return false;
    if (pos > len) return false;

    const unsigned __int8 *pcheck = (const unsigned __int8 *)&pbuffer[pos];

    while (true)
    {
        if (pcheck >= (const unsigned __int8 *)&pbuffer[len])
            return len;

        if (*pcheck == ENDL)
            break;

        pcheck++;
    }

    return (int)(pcheck - (const unsigned __int8 *)pbuffer);
}

int edit_length(const unsigned __int32 size, 
                const __int8 *pbuffer, 
                const unsigned __int32 seg, 
                const unsigned __int32 end)
{
    return 0;
}

int edit_curlength(const unsigned __int32 len, 
                   const unsigned __int32 pos,
                   const unsigned __int32 size, 
                   const __int8 *pbuffer)
{
    return edit_endline(len, pos, size, pbuffer) - edit_segline(len, pos, size, pbuffer);
}

bool edit_copy(unsigned __int32 &len, 
               unsigned __int32 &pos, 
               const unsigned __int32 size, 
               __int8 *pbuffer, 
               unsigned __int32 seg, 
               unsigned __int32 end, 
               unsigned __int32 rsize, 
               __int8 *prbuffer)
{
    return 0;
}

bool edit_paste(unsigned __int32 &len, 
                unsigned __int32 &pos, 
                const unsigned __int32 size, 
                __int8 *pbuffer, 
                unsigned __int32 seg, 
                unsigned __int32 end, 
                unsigned __int32 rsize, 
                __int8 *prbuffer)
{
    if (len >= size) return false;
    if (pos > len) return false;
    if (seg != pos && end != pos) return false;

    if (seg > end) 
    { 
        unsigned __int32 tmp = end; 
        end = seg; 
        seg = tmp; 
    }

    if (edit_iswordcode(len, seg, size, pbuffer) == EDIT_ISWORDCODECENTER)
        seg--;

    if (edit_iswordcode(len, end, size, pbuffer) == EDIT_ISWORDCODECENTER)
        end++;

    pos = seg;

    if (seg > end) return false;
    if (seg > len) return false;
    if (end > len) return false;

    // comput rbuffer
    unsigned int count = 0;
    for (const unsigned __int8 *pcheck = (const unsigned __int8 *)prbuffer; 
        pcheck < (const unsigned __int8 *)&prbuffer[rsize]; pcheck++)
    {
        bool is_wchar = false;
        if (*pcheck < 0x80)
        {
            if (*pcheck >= 0x20)
                count++;
            else if (*pcheck == '\r')
                count++;
        }
        else
        {
            if (pcheck + 2 < (const unsigned __int8 *)&prbuffer[rsize])
                count += 2;
            pcheck++;
            is_wchar = true;
        }

        if (len - (end - seg) + count > size - (is_wchar ? 2 : 1)) 
        {
            rsize = (unsigned int)(pcheck - (const unsigned __int8 *)prbuffer + 1) + 1; // 最后一个是\0
            break;
        }
    }

    if (len - (end - seg) + count >= size) 
        return false;

    pos = seg + count;

    // resize text info
    if (count != end - seg)
        memmove(&pbuffer[seg + count], &pbuffer[end], len-end);

    unsigned __int8 *pdest = (unsigned __int8 *)&pbuffer[seg];
    for (const unsigned __int8 *psrc = (const unsigned __int8 *)prbuffer; 
        psrc < (const unsigned __int8 *)&prbuffer[rsize]; psrc++)
    {
        if (*psrc < 0x80)
        {
            if (*psrc >= 0x20)
                *pdest++ = *psrc;
            else if (*psrc == '\r')
                *pdest++ = ENDL;

            continue;
        }

        if (psrc + 2 < (const unsigned __int8 *)&prbuffer[rsize])
        {
            *(__int16 *)pdest = *(__int16 *)psrc;
            pdest += 2;
        }

        psrc++;
    }

    len = len - (end - seg) + count;

    pbuffer[len] = 0;

    return true;
}

bool edit_char(unsigned __int32 &len, 
               unsigned __int32 &pos, 
               const unsigned __int32 size, 
               __int8 *pbuffer, 
               __int8 ch,
               unsigned __int32 &status)
{
    if (len >= size) return false;
    if (pos > len) return false;

    if (EDIT_IS_STATE(status, EDIT_FLAG_WAIT_SECOND_WORDCODE_BYTE))
        goto _input_second_wordcode_byte;

    switch (ch)
    {
    case VK_RETURN: 
        if (EDIT_IS_STATE(status, EDIT_FLAG_SUPPORT_ENTER))
        if (len < size-1)
        {
            if (pos < len)
                memmove(&pbuffer[pos+1], &pbuffer[pos], len-pos);
            pbuffer[pos++] = ENDL;
            len++;
        }
        break;

    case VK_TAB:
        if (EDIT_IS_STATE(status, EDIT_FLAG_SUPPORT_TABLE))
        if (len < size-1)
        {
            if (pos < len)
                memmove(&pbuffer[pos+1], &pbuffer[pos], len-pos);
            pbuffer[pos++] = ' ';
            len++;
        }
        break;

    case VK_BACK:
        switch (edit_iswordcode(len, pos-1, size, pbuffer))
        {
        case EDIT_ISTERMINATE:
            return true;

        case EDIT_ISENDL:
        case EDIT_ISASCII:
            if (pos >= 1)
            if (len >= 1) // this is special verify
            {
                if (pos < len)
                    memmove(&pbuffer[pos-1], &pbuffer[pos], len-pos);
                pbuffer[len -= 1] = 0;
                pos--;
            }
            break;

        case EDIT_ISWORDCODECENTER:
            if (pos >= 2)
            if (len >= 2) // this is special verify
            {
                if (pos < len)
                    memmove(&pbuffer[pos-2], &pbuffer[pos], len-pos);
                pbuffer[len -= 2] = 0;
                pos -= 2;
            }
            break;

        case EDIT_ISWORDCODE: // at this, the cursor position is wrong
            if (pos >= 1)
            if (len >= 2) // this is special verify
            {
                if (pos < len-1)
                    memmove(&pbuffer[pos-1], &pbuffer[pos+1], len-1-pos);
                pbuffer[len -= 2] = 0;
                pos--;
            }
            break;
        }
        break;

    default:
        {
            if ((unsigned __int8)ch < 0x20) // only visible character
                return false;

            switch (edit_iswordcode(len, pos, size, pbuffer))
            {
            case EDIT_ISTERMINATE:
                return true;

            default:
                if (edit_iswordcode(0, 0, 1, &ch) == EDIT_ISWORDCODE)
                {
                    EDIT_SET_STATE(status, EDIT_FLAG_WAIT_SECOND_WORDCODE_BYTE);
                    EDIT_SET_STATE_HIWORD(status, ch);
                }
                else
                {
                    if (len < size-1)
                    {
                        if (pos < len)
                            memmove( &pbuffer[pos+1], &pbuffer[pos], len-pos);
                        pbuffer[pos++] = ch;
                        len++;
                    }
                }
                break;

_input_second_wordcode_byte:
                if (len < size-2)
                {
                    if (pos < len)
                        memmove(&pbuffer[pos+2], &pbuffer[pos], len-pos);
                    pbuffer[pos++] = (unsigned __int8)EDIT_GET_STATE_HIWORD(status);
                    pbuffer[pos++] = ch;
                    len += 2;
                }

                EDIT_CLEAR_STATE(status, EDIT_FLAG_WAIT_SECOND_WORDCODE_BYTE);
            }
        }
    }

    return true;
}

extern "C"
{
    int _edit_iswordcode(const unsigned __int32 len, 
                        const unsigned __int32 pos, 
                        const unsigned __int32 size, 
                        const __int8 *pbuffer);

    bool _ConverPos2RC(const unsigned __int32 len, 
                    const unsigned __int32 pos, 
                    const unsigned __int32 size, 
                    const __int8 *pbuffer, 
                    unsigned __int32 &col,
                    unsigned __int32 &row,
                    const unsigned __int32 col_max,
                    const unsigned __int32 row_max);

    bool _ConverRC2Pos(const unsigned __int32 len, 
                    unsigned __int32 &pos, 
                    const unsigned __int32 size, 
                    const __int8 *pbuffer, 
                    const unsigned __int32 _row,
                    const unsigned __int32 _col,
                    const unsigned __int32 row_max,
                    const unsigned __int32 col_max);

    int _edit_iswordcode(const unsigned __int32 len, 
                        const unsigned __int32 pos, 
                        const unsigned __int32 size, 
                        const __int8 *pbuffer)
    {
        const unsigned __int8 *pcheck = NULL;
        int ret = EDIT_ISTERMINATE;

        if (len >= size) 
            return EDIT_ISTERMINATE;

        if (pos > len) 
            return EDIT_ISTERMINATE;

        pcheck = (const unsigned __int8 *)&pbuffer[pos];

        if (*pcheck == ENDL)
            return EDIT_ISENDL;

        for (; pcheck > (const unsigned __int8 *)pbuffer; pcheck--)
            if (*pcheck == ENDL)
                break;

        for (; pcheck <= (const unsigned __int8 *)&pbuffer[pos]; pcheck++)
        {
            if (*pcheck < 0x80)
            {
                ret = EDIT_ISASCII;
                continue;
            }

            ret = EDIT_ISWORDCODE;
            pcheck++;
        }

        if (ret == EDIT_ISWORDCODE)
        if (pcheck == (const unsigned __int8 *)&pbuffer[pos+1])
            return EDIT_ISWORDCODECENTER;

        return ret;
    }

    bool _ConverPos2RC( UINT len, UINT pos, UINT size, LPCSTR pbuffer, UINT &col, UINT &row, UINT col_max, UINT row_max )
    {
        const unsigned __int8 *pcheck = NULL;

        row = row_max;
        col = col_max;

        if (len >= size) return false;
        if (pos > len) return false;

        pcheck = (const unsigned __int8 *)pbuffer;

        row = col = 0;

        for (; pcheck < (const unsigned __int8 *)&pbuffer[pos]; pcheck++)
        {
            if (col > col_max - ((*pcheck < 0x80) ? 1 : 2))
            {
                if (row == row_max-1)
                    break;

                col = 0, row++;

                if (*pcheck == ENDL) // 用软回车的时候忽略掉硬回车
                    continue;
            }

            if (*pcheck < 0x80)
            {
                col++;

                // 硬回车
                if (*pcheck == ENDL)
                {
                    if (row == row_max-1)
                        break;

                    col = 0, row++;
                }
                continue;
            }

            col += 2;
            pcheck++;
        }

        if (col != 0)
        if (_edit_iswordcode(len, pos, size, pbuffer) == EDIT_ISWORDCODECENTER)
            col--;

        return true;
    }

    bool _ConverRC2Pos(const unsigned __int32 len, 
                    unsigned __int32 &pos, 
                    const unsigned __int32 size, 
                    const __int8 *pbuffer, 
                    const unsigned __int32 _row,
                    const unsigned __int32 _col,
                    const unsigned __int32 row_max,
                    const unsigned __int32 col_max)
    {
        const unsigned __int8 *pcheck = NULL;

        if (len >= size) return false;
        if (pos > len) return false;

        pcheck = (const unsigned __int8 *)pbuffer;

        unsigned __int32 row = 0, col = 0;
        pos = 0;

        for (; pcheck < (const unsigned __int8 *)&pbuffer[len]; pcheck++)
        {
            if (col > col_max - ((*pcheck < 0x80) ? 1 : 2))
            {
                if (row == row_max-1)
                    break;

                col = 0, row++;

                if (*pcheck == ENDL) // 用软回车的时候忽略掉硬回车
                    continue;
            }

            if (row == _row)
            {
                if (col >= _col)
                    break;
            }
            else if (row > _row)
                break;

            if (*pcheck < 0x80)
            {
                col++;
                pos++;

                if (*pcheck == ENDL)
                {
                    if (row == row_max-1)
                        break;

                    col = 0, row++;
                }
                continue;
            }

            col += 2;
            pos += 2;
            pcheck++;
        }

        if (col != 0)
        if (_edit_iswordcode(len, pos, size, pbuffer) == EDIT_ISWORDCODECENTER)
            col--;

        return true;
    }

    void CTS( class CDFace *_this, RECT *rc )
    {
        RECT r;
        if ( CDFace *parent = *( CDFace** )( ( LPBYTE )_this + 0x118 ) )
	    {
            r.left = *( long* )( ( LPBYTE )parent + 0x13c );
            r.top = *( long* )( ( LPBYTE )parent + 0x140 );
            CTS( parent, &r );
		    rc->left += r.left;
		    rc->right += r.left;
		    rc->top += r.top;
		    rc->bottom += r.top;
	    }
    }

}

int edit_iswordcode(const unsigned __int32 len, 
                const unsigned __int32 pos, 
                const unsigned __int32 size, 
                const __int8 *pbuffer)
{
    return _edit_iswordcode( len, pos, size, pbuffer );
}

bool ConverRC2Pos(const unsigned __int32 len, 
                unsigned __int32 &pos, 
                const unsigned __int32 size, 
                const __int8 *pbuffer, 
                const unsigned __int32 _row,
                const unsigned __int32 _col,
                const unsigned __int32 row_max,
                const unsigned __int32 col_max)
{
    return _ConverRC2Pos( len, pos, size, pbuffer, _row, _col, row_max, col_max );
}

bool ConverPos2RC(const unsigned __int32 len, 
                const unsigned __int32 pos, 
                const unsigned __int32 size, 
                const __int8 *pbuffer, 
                unsigned __int32 &col,
                unsigned __int32 &row,
                const unsigned __int32 col_max,
                const unsigned __int32 row_max)
{
    return _ConverPos2RC( len, pos, size, pbuffer, col, row, col_max, row_max );
}
