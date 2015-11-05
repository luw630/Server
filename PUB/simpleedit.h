#pragma once

// information
// in this simple-edit component, all '\r' and '\n' (normal string) changed to '\0' (special string)
// and the '\0' (terminate character) changed to a external 32bit value 
// so ...
// use edit_paste() to convert normal byte string to '\0'-string and paste it into simple-edit
// use edit_copy() to convert '\0'-string to normal byte string and copy it into you prepared buffer
// use edit_check() to check 'pos' (function parameter) assigned position character is (terminate / endline / ascii / wordcode / wordcodecenter)
// in wordcodecenter case, the 'pos' maybe is wrong position
// hehe ... enjoy it

#define EDIT_ISTERMINATE        0
#define EDIT_ISENDL             1
#define EDIT_ISASCII            2
#define EDIT_ISWORDCODE         3
#define EDIT_ISWORDCODECENTER   4

#define EDIT_FLAG_WAIT_SECOND_WORDCODE_BYTE     (0x01)
#define EDIT_FLAG_SUPPORT_ENTER                 (0x01 << 8)
#define EDIT_FLAG_SUPPORT_TABLE                 (0x02 << 8)

#define EDIT_IS_STATE(_state, _flag) (_state & _flag)
#define EDIT_SET_STATE(_state, _flag) (_state |= _flag)
#define EDIT_CLEAR_STATE(_state, _flag) (_state &= ~_flag)

#define EDIT_SET_STATE_HIWORD(_state, _word) (_state &= 0xffff, _state |= ((unsigned __int32)_word << 16))
#define EDIT_GET_STATE_HIWORD(_state) (unsigned __int16)(_state >> 16)

// extended simple-edit function
bool edit_init(unsigned __int32 &len, 
               unsigned __int32 &pos, 
               unsigned __int32 &status, 
               const unsigned __int32 size, 
               __int8 *pbuffer);

int edit_iswordcode(const unsigned __int32 len, 
                    const unsigned __int32 pos, 
                    const unsigned __int32 size, 
                    const __int8 *pbuffer);

bool edit_paste(unsigned __int32 &len, 
                unsigned __int32 &pos, 
                const unsigned __int32 size, 
                __int8 *pbuffer, 
                unsigned __int32 seg, 
                unsigned __int32 end, 
                unsigned __int32 rsize, 
                __int8 *prbuffer);

bool edit_char(unsigned __int32 &len, 
               unsigned __int32 &pos, 
               const unsigned __int32 size, 
               __int8 *pbuffer, 
               __int8 ch, 
               unsigned __int32 &status);

bool ConverPos2RC(const unsigned __int32 len, 
                  const unsigned __int32 pos, 
                  const unsigned __int32 size, 
                  const __int8 *pbuffer, 
                  unsigned __int32 &row,
                  unsigned __int32 &col,
                  const unsigned __int32 row_max,
                  const unsigned __int32 col_max);

bool ConverRC2Pos(const unsigned __int32 len, 
                  unsigned __int32 &pos, 
                  const unsigned __int32 size, 
                  const __int8 *pbuffer, 
                  const unsigned __int32 row,
                  const unsigned __int32 col,
                  const unsigned __int32 row_max,
                  const unsigned __int32 col_max);

int edit_curlength(const unsigned __int32 len, 
                   const unsigned __int32 pos,
                   const unsigned __int32 size, 
                   const __int8 *pbuffer);
