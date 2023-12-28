%macro mtrand_add_eax_mod_mtsize 2
    add eax, %1
    cmp eax, MT_SIZE
    jb mtrand_%2
    sub eax, MT_SIZE
    mtrand_%2:
%endmacro

%macro put_fabs_mask 1
    pcmpeqw %1, %1
    psrld %1, 1
%endmacro

%macro put_ones 1
    pcmpeqw %1, %1
    pslld %1, 25
    psrld %1, 2
    psrldq %1, 4
%endmacro

%macro abs_eax 0
    cdq
    xor eax, edx
    sub eax, edx
%endmacro

; Receives a tag to jump in case no hit happens, and places result in xmm6
;
; Expects xmm0:xmm1 to be the ray, xmm2 and xmm3 to be t_min and t_max, and
; xmm4 and xmm5 to be the plane origin and normal respectively
%macro plane_hit_macro 1
    ; Check if plane is perpendicular to ray direction
    vdpps xmm6, xmm1, xmm5, 0xF1
    put_fabs_mask xmm7
    andps xmm6, xmm7
    comiss xmm6, [eps]
    jb %1

    ; Check if t is in allowed range
    vsubps xmm6, xmm4, xmm0
    dpps xmm6, xmm5, 0xF1
    vdpps xmm7, xmm1, xmm5, 0xF1
    divss xmm6, xmm7
    comiss xmm6, xmm2
    jb %1
    comiss xmm3, xmm6
    jb %1
%endmacro

; res = v1 x v2
; Uses tmp1 and tmp2 as temp variables
%macro vec_cross 5 ; vec_cross(res, v1, v2, tmp1, tmp2)
    ; cross product
    vshufps %4, %2, %2, 0b11001001
    vshufps %5, %3, %3, 0b11010010
    vmulps %1, %4, %5
    vshufps %4, %2, %2, 0b11010010
    vshufps %5, %3, %3, 0b11001001
    mulps %4, %5
    subps %1, %4 ; %1 = %2 x %3
%endmacro

; Skips the current triangle is p (expected in xmm8) is not left of (p2-p1)
; Also uses xmm10-14 as temp variables
%macro is_left_of 2; is_left_of(p1, p2)
    vsubps xmm10, %2, %1
    vsubps xmm11, xmm8, %1
    vec_cross xmm12, xmm10, xmm11, xmm13, xmm14
    dpps xmm12, xmm9, 0xF1
    xorps xmm10, xmm10
    comiss xmm12, xmm10
    jbe triangle_loop_next
%endmacro

; Save hit record info
%macro save_hit 3 ; save_hit(TAG, src, hit_point)
    movss [rsi+HITRECORD_T_OFF], xmm3
    mov rdx, [%2+%1_MATERIAL_OFF]
    mov [rsi+HITRECORD_MATERIAL_OFF], rdx
    vdpps xmm8, xmm1, xmm5, 0xF1
    xorps xmm7, xmm7
    xor dl, dl
    comiss xmm8, xmm7

    jb %1_front_face ; IF (front_face)
    vsubps xmm5, xmm7, xmm5
    %1_front_face: ; ELSE
    inc dl

    movups [rsi+HITRECORD_NORMAL_OFF], xmm5
    mov [rsi+HITRECORD_FFACE_OFF], dl
    movups [rsi+HITRECORD_P_OFF], %3
%endmacro
