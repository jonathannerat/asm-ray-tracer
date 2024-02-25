; MT RNG Seeder
; mtsrand(int seed[edi])
; ret & args: rdi
mtsrand: ;{{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    mov [g_aiMT], edi

    mov ebx, 1 ; FOR (i = 0; i < MT_SIZE; i++) {{{
    mtsrand_loop:
        mov r12d, edi
        shr r12d, 30
        xor r12d, edi
        imul r12d, r12d, 0x6c078965
        lea edi, [r12d+ebx]
        mov [g_aiMT+ebx*4], edi
        inc ebx
        cmp ebx, MT_SIZE
        jl mtsrand_loop ;}}}

    mov dword [g_iMTI], 0

    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; MT RNG
; int[eax] mtrand()
; ret & args: rax
mtrand: ;{{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13

    cmp dword [g_iMTI], 0 ; IF (g_iMTI == 0) {{{
    jne mtrand_skip_twist

    ; twist
    xor eax, eax ; i = 0
    mtrand_loop: ; FOR (i in [0, ..., MT_SIZE) ) {{{
        mov ebx, eax
        mov r12d, [g_aiMT+eax*4]

        mtrand_add_eax_mod_mtsize 1, l1

        mov r13d, [g_aiMT+eax*4]

        and r12d, 0x80000000
        and r13d, 0x7fffffff
        or r12d, r13d
        mov r13d, r12d
        shr r13d, 1

        test r12d, 1 ; IF (y%2==1) {{{
        jz mtrand_even_y
        xor r13d, 0x9908b0df
        mtrand_even_y: ;}}}

        mtrand_add_eax_mod_mtsize 396, l2 ; should be 397, but we already added 1 before
        mov r12d, [g_aiMT+eax*4]
        xor r13d, r12d
        mov eax, ebx
        mov [g_aiMT+rax*4], r13d

        inc eax
        cmp eax, MT_SIZE
        jl mtrand_loop ;}}}

    mtrand_skip_twist: ;}}}

    mov eax, [g_iMTI]
    mov eax, [g_aiMT+eax*4]
    mov ebx, eax
    shr ebx, 11
    xor eax, ebx ; y ^ (y >> 11)

    mov ebx, eax
    shl ebx, 7
    and ebx, 0x9d2c5680
    xor eax, ebx ; y ^ ((y << 7) & (0x9d2c5680));

    mov ebx, eax
    shl ebx, 15
    and ebx, 0xefc60000
    xor eax, ebx ; y = y ^ ((y << 15) & (0xefc60000));

    mov ebx, eax
    shr ebx, 18
    xor ebx, eax ; y = y ^ (y >> 18);

    mov eax, [g_iMTI]
    mtrand_add_eax_mod_mtsize 1, l3
    mov [g_iMTI], eax
    mov eax, ebx

    pop r13
    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; Random float in [0, 1)
; float[xmm0] mtfrand()
; ret & args: xmm0
; also uses: rax, rdx
mtfrand: ;{{{
    push rbp
    mov rbp, rsp
    sub rsp, 0x10
    movaps [rsp], xmm1

    call mtrand
    abs_eax
    cvtsi2ss xmm0, eax

    xor eax, eax
    not eax
    shr eax, 1
    cvtsi2ss xmm1, eax
    divss xmm0, xmm1

    movaps xmm1, [rsp]
    add rsp, 0x10
    pop rbp
    ret ;}}}

; Random vector with components between [-1, 1]
; float[xmm0] mtfrand_vec()
; ret & args: xmm0
; also uses: rax, rdx
mtfrand_vec: ;{{{
    push rbp
    mov rbp, rsp
    sub rsp, 0x20

    call mtrand
    abs_eax
    mov [rsp+0x10], eax

    call mtrand
    abs_eax
    mov [rsp+0x14], eax

    call mtrand
    abs_eax
    mov [rsp+0x18], eax

    mov dword [rsp+0x1C], 0 ; [rsp+0x10] = {mtrand(), mtrand(), mtrand(), 0}

    cvtdq2ps xmm0, [rsp+0x10]
    put_fabs_mask xmm1 ; xmm1 = {0x7f...f * 4}

    movaps [rsp+0x10], xmm1   ; [rsp+0x10] = {MAX_INT32 * 4}
    cvtdq2ps xmm1, [rsp+0x10] ; xmm1 = {(float) MAX_INT32 * 4}
    divps xmm0, xmm1 ; xmm0 /= xmm1

    ; xmm0 = rand in [0, 1]
    addps xmm0, xmm0 ; xmm0 = rand in [0, 2]
    put_ones xmm1
    subps xmm0, xmm1 ; xmm0 = rand in [-1, 1]

    ; split random vec in 2 xmm registers to comply with C standard
    vshufps xmm1, xmm0, xmm0, 0b11111110
    shufps xmm0, xmm0, 0b11110100

    add rsp, 0x20
    pop rbp
    ret ;}}}
