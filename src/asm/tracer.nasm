; vi: ft=nasm ts=4 sw=4 et fdm=marker
global tracer_asm

%include "offsets.nasm"
%include "macros.nasm"
%define MT_SIZE 624
%define MT_SEED 0x20221218 ;    

%macro mtrand_add_eax_mod_mtsize 2
    add eax, %1
    cmp eax, MT_SIZE
    jb mtrand_%2
    sub eax, MT_SIZE
    mtrand_%2:
%endmacro

%macro fabs_mask 1
    pcmpeqw %1, %1
    psrld %1, 1
%endmacro

section .data

; 32-bit absolute value mask
rand_descale:  dd 0x2FFFFFFD
eps:  dd 1.0e-4
inf:  dd 0x7F800000
half: dd 0.5e0
halfdegtorad: dd 0x3C0EFA35

; Mersenne Twister
g_aiMT: times MT_SIZE dd 0 ; MT array
g_iMTI: dd 0 ; MT index
mt_size: dw MT_SIZE

section .text

; tracer_asm(Camera, Output, List *, Color *);
tracer_asm:
    push rbp
    mov rbp, rsp
    sub rsp, 0x48

    mov rdi, MT_SEED
    call mtsrand

    ; Camera *c = $rbp+0x10
    mov rbx, rbp
    add rbx, 0x10
    mov [rsp], rbx ; save Camera pointer

    ; output.width= $rdi>>32
    ; output.height= $edi
    ; output.spp= $rsi>>32
    ; output.depth= $esi
    mov [rsp+0x10], edi ; save Output height
    shr rdi, 0x20
    mov [rsp+0x14], edi ; save Output width
    mov [rsp+0x18], esi ; save Output spp
    shr rsi, 0x20
    mov [rsp+0x1C], esi ; save Output depth

    ; List *l = $rdx
    mov [rsp+0x20], rdx

    ; Color *f = $rcx
    mov [rsp+0x28], rcx

    height_loop: ; {{{
    xor edi, edi ; j = 0

    width_loop: ; {{{
    xor esi, esi ; i = 0

    spp_loop: ; {{{
    xor edx, edx ; k = 0

    pxor xmm0, xmm0
    movaps [rsp+0x30], xmm0 ; pixel = V3(0)
    rdrand rax

    cmp edx, [rsp+0x18]
    jl spp_loop ;}}}

    cmp esi, [rsp+0x14]
    jl width_loop ;}}}

    cmp edi, [rsp+0x10]
    jl height_loop ;}}}

    add rsp, 0x48
    pop rbp
    ret

mtsrand:
    push rbp
    mov rbp, rsp
    push rax
    push rdx

    mov [g_aiMT], edi

    mov edx, 1 ; FOR (i = 0; i < MT_SIZE; i++) {{{
    mtsrand_loop:
        mov eax, edi
        shr eax, 30
        xor eax, edi
        imul eax, eax, 0x6c078965
        lea edi, [eax+edx]
        mov [g_aiMT+edx*4], edi
        inc edx
        cmp edx, MT_SIZE
        jl mtsrand_loop ; }}}

    mov dword [g_iMTI], 0

    pop rdx
    pop rax
    pop rbp
    ret

mtrand:
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
        mtrand_even_y: ; }}}

        mtrand_add_eax_mod_mtsize 396, l2 ; should be 397, but we already added 1 before
        mov r12d, [g_aiMT+eax*4]
        xor r13d, r12d
        mov eax, ebx
        mov [g_aiMT+rax*4], r13d

        inc eax
        cmp eax, MT_SIZE
        jl mtrand_loop ; }}}

    mtrand_skip_twist: ; }}}

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
    ret

mtfrand_vec:
    push rbp
    mov rbp, rsp
    push rax
    sub rsp, 0x20
    movaps [rsp], xmm1

    call mtrand
    mov [rsp+0x10], eax
    call mtrand
    mov [rsp+0x14], eax
    call mtrand
    mov [rsp+0x18], eax
    call mtrand
    mov [rsp+0x1C], eax
    cvtdq2ps xmm0, [rsp+0x10]
    fabs_mask xmm1
    andps xmm0, xmm1

    movaps [rsp+0x10], xmm1
    cvtdq2ps xmm1, [rsp+0x10]
    divps xmm0, xmm1

    movaps xmm1, [rsp]
    add rsp, 0x20
    pop rax
    pop rbp
    ret
