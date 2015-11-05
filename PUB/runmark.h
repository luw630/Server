#pragma once

#define CALLMARK _asm call MarkPoint

#define MAXPATHLEN  0xffff

static DWORD RunPath[1 + MAXPATHLEN] = {0};

static __declspec(naked) void MarkPoint()
{
    _asm 
    {
        lea edx, RunPath
        mov ecx, [edx]
        cmp ecx, MAXPATHLEN
        jg _error_point
        
        inc ecx
        cmp ecx, MAXPATHLEN
        jle _markpoint
        
        mov ecx, 1
        
    _markpoint:
    
        mov [edx], ecx
        
        shl ecx, 2
        add edx, ecx
        
        mov eax, [esp]
        mov [edx], eax
        
        jmp _end 
        
    _error_point:
        int 3

    _end:
        ret
    }
}
