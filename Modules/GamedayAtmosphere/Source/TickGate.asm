option casemap:none
EXTERN CrowdTick:PROC
EXTERN TickOriginal:QWORD
.code
CrowdTickGate PROC
    pushfq
    push rax
    push rcx
    push rdx
    push r8
    push r9
    push r10
    push r11
    sub rsp, 088h
    movdqu [rsp+020h], xmm0
    movdqu [rsp+030h], xmm1
    movdqu [rsp+040h], xmm2
    movdqu [rsp+050h], xmm3
    movdqu [rsp+060h], xmm4
    movdqu [rsp+070h], xmm5
    call CrowdTick
    movdqu xmm0, [rsp+020h]
    movdqu xmm1, [rsp+030h]
    movdqu xmm2, [rsp+040h]
    movdqu xmm3, [rsp+050h]
    movdqu xmm4, [rsp+060h]
    movdqu xmm5, [rsp+070h]
    add rsp, 088h
    pop r11
    pop r10
    pop r9
    pop r8
    pop rdx
    pop rcx
    pop rax
    popfq
    jmp qword ptr [TickOriginal]
CrowdTickGate ENDP
END
