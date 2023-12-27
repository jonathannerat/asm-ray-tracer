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

%macro vec_scale 3 ; vec_scale(v, s, tmp) = (v.x*s, v.y*s, v.z*s, _)
    shufps %3, %2, 0b00000000
    mulps %1, %3
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
