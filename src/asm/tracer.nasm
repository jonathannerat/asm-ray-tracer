; vi: ft=nasm ts=4 sw=4 et fdm=marker
global tracer_asm

%include "constants.nasm"
%include "macros.nasm"

section .data

; 32-bit absolute value mask
rand_descale:  dd 0x2FFFFFFD
eps:  times 3 dd 1.0e-4
zero: dd 0.0e0
inf:  dd 0x7F800000
half: dd 0.5e0
halfdegtorad: dd 0x3C0EFA35

; Mersenne Twister
g_aiMT: times MT_SIZE dd 0 ; MT array
g_iMTI: dd 0 ; MT index
mt_size: dw MT_SIZE

section .text

%include "mt.nasm"

; Ray Tracer
; tracer_asm(Camera c[rbp+0x10], Output o[rdi:rsi], List *world[rdx], Color *buf[rcx]);
tracer_asm: ;{{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 0x50

    ; PREAMBLE {{{
    ; Camera *c = $rbp+0x10

    ; output.width= $rdi>>32
    ; output.height= $edi
    ; output.spp= $rsi>>32
    ; output.depth= $esi
    mov [rsp+0x10], edi ; save Output height
    shr rdi, 32
    mov [rsp+0x14], edi ; save Output width
    mov [rsp+0x18], esi ; save Output spp
    shr rsi, 32
    mov [rsp+0x1C], esi ; save Output depth

    ; List *l = $rdx
    mov [rsp+0x20], rdx

    ; Color *f = $rcx
    mov [rsp+0x28], rcx
    ;}}}

    ; seed rng
    mov edi, MT_SEED
    call mtsrand

    xor r12d, r12d ; j = 0
    height_loop: ;{{{

        xor r13d, r13d ; i = 0
        width_loop: ;{{{
            xorps xmm0, xmm0
            movaps [rsp+0x30], xmm0 ; pixel = (0, 0, 0)

            xor r14d, r14d ; k = 0
            spp_loop: ;{{{
                lea rdi, [rbp+0x10]
                lea rsi, [rsp+0x10]
                mov rdx, r13
                mov rcx, r12
                call camera_get_ray

                mov rdi, [rsp+0x20]
                mov esi, [rsp+0x1C]
                call ray_color
                addps xmm0, [rsp+0x30]
                movaps [rsp+0x30], xmm0

                inc r14d
                cmp r14d, [rsp+0x18]
                jl spp_loop ;}}}

            mov rbx, [rsp+0x28] ; fb
            mov r14d, r12d
            imul r14d, [rsp+0x14]
            add r14d, r13d
            add r14d, r14d ; multiply offset by 2
            movups [rbx+r14*8], xmm0 ; then by 8, so that we offset by 16

            inc r13d
            cmp r13d, [rsp+0x14]
            jl width_loop ;}}}

        inc r12d
        cmp r12d, [rsp+0x10]
        jl height_loop ;}}}

    add rsp, 0x50
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; Random vec with |x| <= 1
; Vec3[xmm0] rand_vec_in_unit()
; also uses: rax, rdx
rand_vec_in_unit: ;{{{
    push rbp
    mov rbp, rsp
    sub rsp, 0x20
    movaps [rsp], xmm1
    movaps [rsp+0x10], xmm2

    rviu_loop:
        call mtfrand_vec
        addps xmm0, xmm0
        put_ones xmm1
        subps xmm0, xmm1
        vdpps xmm2, xmm0, xmm0, 0xF1
        comiss xmm2, xmm1
        ja rviu_loop

    movaps xmm2, [rsp+0x10]
    movaps xmm1, [rsp]
    add rsp, 0x20
    pop rbp
    ret ;}}}

; Get random Camera ray for pixel
; Vec[xmm0](orig) Vec[xmm1](dir) camera_get_ray(Camera *rdi, Output *rsi, int rdx[i], int rcx[j])
; ret & args: xmm0, xmm1, rdi, rsi, rdx, rcx
; also uses: rax
camera_get_ray:  ;{{{
    push rbp
    mov rbp, rsp
    sub rsp, 0x30
    movaps [rsp], xmm2
    movaps [rsp+0x10], xmm3
    movaps [rsp+0x20], xmm4

    ; s offset in width direction
    cvtsi2ss xmm1, [rsi+OUTPUT_WIDTH_OFF]
    cvtsi2ss xmm2, [rsi+OUTPUT_HEIGHT_OFF]
    mov rsi, rdx
    call mtfrand
    cvtsi2ss xmm3, rsi
    addss xmm0, xmm3
    divss xmm0, xmm1
    vshufps xmm1, xmm0, xmm0, 0
    ; xmm1 = (s, s, s, s)

    ; t offset in width direction
    call mtfrand
    cvtsi2ss xmm3, rcx
    addss xmm0, xmm3
    divss xmm0, xmm2
    vshufps xmm2, xmm0, xmm0, 0
    ; xmm2 = (t, t, t, t)

    call rand_vec_in_unit
    movups xmm3, [rdi+CAMERA_LR_OFF]
    shufps xmm3, xmm3, 0
    mulps xmm0, xmm3 ; xmm0 = rd
    vshufps xmm3, xmm0, xmm0, 0 ; xmm3 = (rd.x * 4)
    mulps xmm3, [rdi+CAMERA_U_OFF] ; xmm3 = c->u * rd.x
    vshufps xmm4, xmm0, xmm0, 0b01010101 ; xmm4 = (rd.y * 4)
    mulps xmm4, [rdi+CAMERA_V_OFF] ; xmm3 = c->v * rd.y
    addps xmm3, xmm4 ; xmm3 = offset

    movups xmm0, [rdi+CAMERA_ORIGIN_OFF]
    addps xmm0, xmm3 ; ray origin
    mulps xmm1, [rdi+CAMERA_HORIZ_OFF]
    mulps xmm2, [rdi+CAMERA_VERTI_OFF]
    addps xmm1, xmm2
    addps xmm1, [rdi+CAMERA_BLCORN_OFF]
    subps xmm1, xmm0 ; ray direction

    movaps xmm4, [rsp+0x20]
    movaps xmm3, [rsp+0x10]
    movaps xmm2, [rsp]
    add rsp, 0x30
    pop rbp
    ret ;}}}

%define RAYCOLOR_HR_OFF 0
%define RAYCOLOR_RAY_OFF HITRECORD_SIZE ; 48
%define RAYCOLOR_SCATTERED_OFF RAYCOLOR_RAY_OFF+RAY_SIZE ; 80
%define RAYCOLOR_ATTENUATION_OFF RAYCOLOR_SCATTERED_OFF+RAY_SIZE ; 112
%define RAYCOLOR_EMITTED_OFF RAYCOLOR_ATTENUATION_OFF+VEC3_SIZE ;128
%define RAYCOLOR_WORLD_OFF RAYCOLOR_EMITTED_OFF+VEC3_SIZE ; 144
%define RAYCOLOR_DEPTH_OFF RAYCOLOR_WORLD_OFF+POINTER_SIZE ; 152
%define RAYCOLOR_SIZE RAYCOLOR_DEPTH_OFF+INT_SIZE+4 ; 156 + 4 (padding) = 160 bytes


; Calculate color for a ray
; Vec[xmm0] ray_color(List *world[rdi], Ray r[xmm0:xmm1], uint depth[rsi])
; ret & args: xmm0, xmm1, rdi, rsi
; also uses: rax
ray_color: ;{{{
    push rbp
    mov rbp, rsp
    sub rsp, RAYCOLOR_SIZE+0x40

    ;PREAMBLE {{{
    mov [rsp+RAYCOLOR_WORLD_OFF], rdi
    mov [rsp+RAYCOLOR_DEPTH_OFF], esi
    movaps [rsp+RAYCOLOR_RAY_OFF], xmm0
    movaps [rsp+RAYCOLOR_RAY_OFF+0x10], xmm1
    movaps [rsp+RAYCOLOR_SIZE], xmm2
    movaps [rsp+RAYCOLOR_SIZE+0x10], xmm3
    movaps [rsp+RAYCOLOR_SIZE+0x20], xmm4
    movaps [rsp+RAYCOLOR_SIZE+0x30], xmm6
    ;}}}

    test esi, esi
    jz ray_color_ret_zero

    mov rsi, rsp
    movss xmm2, [eps]
    movss xmm3, [inf]
    call list_hit

    test al, al
    jz ray_color_ret_zero

    mov rdi, [rsp+HITRECORD_MATERIAL_OFF]
    call material_emitted
    movaps xmm6, xmm0

    mov rdi, [rsp+HITRECORD_MATERIAL_OFF]
    mov rsi, rsp
    movaps xmm0, [rsp+RAYCOLOR_RAY_OFF]
    movaps xmm1, [rsp+RAYCOLOR_RAY_OFF+0x10]
    call material_scatter

    test al, al
    jz ray_color_ret_emitted

    mov rdi, [rsp+RAYCOLOR_WORLD_OFF]
    movaps xmm0, xmm3
    movaps xmm1, xmm4
    mov esi, [rsp+RAYCOLOR_DEPTH_OFF]
    dec esi
    call ray_color

    mulps xmm0, xmm2
    addps xmm0, xmm6
    jmp ray_color_ret

    ray_color_ret_emitted:
    movaps xmm0, xmm6
    jmp ray_color_ret

    ray_color_ret_zero:
    xorps xmm0, xmm0

    ray_color_ret:
    movaps xmm2, [rsp+RAYCOLOR_SIZE]
    movaps xmm3, [rsp+RAYCOLOR_SIZE+0x10]
    movaps xmm4, [rsp+RAYCOLOR_SIZE+0x20]
    movaps xmm6, [rsp+RAYCOLOR_SIZE+0x30]
    add rsp, RAYCOLOR_SIZE+0x40
    pop rbp
    ret ;}}}

%define SURFACE_TYPE_PLANE  0
%define SURFACE_TYPE_SPHERE 1
%define SURFACE_TYPE_LIST   2
%define SURFACE_TYPE_AARECT 3
%define SURFACE_TYPE_AABOX  4

; Check if ray hits any surface on the list
; bool[ax] list_hit(List *l[rdi], Ray r[xmm0:xmm1], real t_min[xmm2], real t_max xmm3, HitRecord *hit[rsi])
; ret & args: rax, rdi, rsi, xmm0-xmm3
list_hit: ;{{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, 0x20

    ;PREAMBLE {{{
    mov [rsp], rdi
    mov [rsp+0x08], rsi
    mov byte [rsp+0x10], 0
    ;}}}

    xor rbx, rbx ; i = 0

    ;FOR j = 0 to plane count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_PLANE]
    mov r14, [rdi+LIST_SURFACES_OFF]
    xor r13d, r13d ; j = 0
    jmp plane_loop_cond
    plane_loop:
        mov r15, [r14+rbx*8]
        movups xmm4, [r15+PLANE_ORIGIN_OFF]
        movups xmm5, [r15+PLANE_NORMAL_OFF]

        plane_hit_macro plane_loop_next

        ; PLANE HIT
        or byte [rsp+0x10], 1
        movaps xmm3, xmm6

        movss [rsi+HITRECORD_T_OFF], xmm6
        mov rdx, [r15+PLANE_MATERIAL_OFF]
        mov [rsi+HITRECORD_MATERIAL_OFF], rdx
        vdpps xmm8, xmm1, xmm5, 0xF1
        xorps xmm7, xmm7
        xor dl, dl
        comiss xmm8, xmm7

        jb plane_front_face ; IF (front_face)
        vsubps xmm5, xmm7, xmm5
        plane_front_face: ; ELSE
        inc dl

        movups [rsi+HITRECORD_NORMAL_OFF], xmm5
        mov [rsi+HITRECORD_FFACE_OFF], dl
        shufps xmm6, xmm6, 0
        mulps xmm6, xmm1
        addps xmm6, xmm0
        movups [rsi+HITRECORD_P_OFF], xmm6

        plane_loop_next:
        inc rbx
        inc r13d
        plane_loop_cond:
        cmp r13d, r12d
        jl plane_loop ;}}}

    ;FOR j = 0 to sphere count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_SPHERE]
    xor r13d, r13d ; j = 0
    jmp sphere_loop_cond
    sphere_loop:
        mov r15, [r14+rbx*8]
        movups xmm4, [r15+SPHERE_CENTER_OFF]
        movss xmm5, [r15+SPHERE_RADIUS_OFF]

        vsubps xmm6, xmm0, xmm4 ; oc
        vdpps xmm7, xmm1, xmm1, 0xF1 ; a
        vdpps xmm8, xmm6, xmm1, 0xF1 ; hb
        vdpps xmm9, xmm6, xmm6, 0xF1
        vmulss xmm6, xmm5, xmm5
        subss xmm9, xmm6
        vmulss xmm6, xmm8, xmm8
        mulss xmm9, xmm7
        vsubss xmm9, xmm6, xmm9 ; discriminant

        xorps xmm6, xmm6
        comiss xmm9, xmm6
        jb sphere_loop_next ; discriminant < 0

        sqrtss xmm9, xmm9
        vsubss xmm8, xmm6, xmm8 ; -hb
        vsubss xmm6, xmm8, xmm9
        divss xmm6, xmm7 ; (-hb - sqrtd) / a
        addss xmm8, xmm9
        vdivss xmm7, xmm8, xmm7 ; (-hb + sqrtd) / a

        xorps xmm10, xmm10
        vsubss xmm8, xmm6, xmm2
        vsubss xmm9, xmm3, xmm6
        mulss xmm8, xmm9
        comiss xmm8, xmm10
        jae sphere_loop_hit

        movss xmm6, xmm7
        vsubss xmm8, xmm7, xmm2
        vsubss xmm9, xmm3, xmm7
        mulss xmm8, xmm9
        comiss xmm8, xmm10
        jae sphere_loop_hit
        jmp sphere_loop_next

        sphere_loop_hit:
        or byte [rsp+0x10], 1
        movss xmm3, xmm6

        shufps xmm6, xmm6, 0
        vmulps xmm7, xmm6, xmm1
        addps xmm7, xmm0
        subps xmm7, xmm4
        shufps xmm5, xmm5, 0
        vdivps xmm5, xmm7, xmm5
        movss [rsi+HITRECORD_T_OFF], xmm6
        mov rdx, [r15+SPHERE_MATERIAL_OFF]
        mov [rsi+HITRECORD_MATERIAL_OFF], rdx
        vdpps xmm8, xmm1, xmm5, 0xF1
        xorps xmm7, xmm7
        xor dl, dl
        comiss xmm8, xmm7

        jb sphere_front_face ; IF (front_face)
        vsubps xmm5, xmm7, xmm5
        sphere_front_face: ; ELSE
        inc dl

        movups [rsi+HITRECORD_NORMAL_OFF], xmm5
        mov [rsi+HITRECORD_FFACE_OFF], dl
        shufps xmm6, xmm6, 0
        vmulps xmm5, xmm1, xmm6
        vaddps xmm6, xmm5, xmm0
        movups [rsi+HITRECORD_P_OFF], xmm6

        sphere_loop_next:
        inc rbx
        inc r13d
        sphere_loop_cond:
        cmp r13d, r12d
        jl sphere_loop ;}}}

    ;FOR j = 0 to list count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_LIST]
    xor r13d, r13d ; j = 0
    jmp list_loop_cond
    list_loop:
        sub rsp, 0x50
        mov [rsp+HITRECORD_SIZE], rdi
        mov [rsp+HITRECORD_SIZE+8], rsi
        mov rdi, [r14+rbx*8]
        mov rsi, rsp

        call list_hit

        mov rdi, [rsp+HITRECORD_SIZE]
        mov rsi, [rsp+HITRECORD_SIZE+8]
        add rsp, 0x50

        test al, al
        jz list_loop_next

        or [rsp+0x10], al
        movss xmm3, [rsp-0x50+HITRECORD_T_OFF]

        ; copy all 48 bytes of the HitRecord
        movaps xmm4, [rsp-0x50]
        movaps [rsi], xmm4
        movaps xmm4, [rsp-0x40]
        movaps [rsi+0x10], xmm4
        movaps xmm4, [rsp-0x30]
        movaps [rsi+0x20], xmm4

        list_loop_next:
        inc rbx
        inc r13d
        list_loop_cond:
        cmp r13d, r12d
        jl list_loop ;}}}

    ;FOR j = 0 to aarect count {{{}}}

    ;FOR j = 0 to aabox count {{{
    sub rsp, 0x20
    mov [rsp], rdi
    mov [rsp+0x08], rsi
    mov [rsp+0x10], rcx

    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_AABOX]
    xor r13d, r13d ; j = 0
    jmp aabox_loop_cond
    aabox_loop:

        mov r15, [r14+rbx*8]
        mov rdi, [r15+AABOX_SIDES_OFF]

        xor cx, cx
        aabox_sides_loop: ;{{{
            mov rsi, [rdi+rcx*8]
            movups xmm4, [rsi+PLANE_ORIGIN_OFF]
            movups xmm5, [rsi+PLANE_NORMAL_OFF]

            plane_hit_macro aabox_sides_next

            vshufps xmm4, xmm6, xmm6, 0
            mulps xmm4, xmm1
            addps xmm4, xmm0

            movups xmm7, [r15+AABOX_PMIN_OFF]
            movups xmm8, [eps]
            subps xmm7, xmm8
            vminps xmm9, xmm7, xmm4 ; xmm9 = packed_min(xmm7, xmm4)
            ptest xmm9, xmm7 ; CF = 1 <==> xmm9 == xmm7 <==> xmm7 <= xmm4
            jnc aabox_sides_next ; CF == 0 ==> point is not inside box

            addps xmm8, [r15+AABOX_PMAX_OFF]
            vminps xmm9, xmm8, xmm4 ; xmm9 = packed_min(xmm8, xmm6)
            ptest xmm9, xmm4 ; CF = 1 <==> xmm9 == xmm4 <==> xmm4 <= xmm8
            jnc aabox_sides_next ; CF == 0 ==> point is not inside box

            ; aabox hit
            or byte [rsp+0x20+0x10], 1
            movaps xmm3, xmm6

            mov rsi, [rsp+0x08]
            movss [rsi+HITRECORD_T_OFF], xmm6
            mov rdx, [r15+AABOX_MATERIAL_OFF]
            mov [rsi+HITRECORD_MATERIAL_OFF], rdx
            vdpps xmm8, xmm1, xmm5, 0xF1
            xorps xmm7, xmm7
            xor dl, dl
            comiss xmm8, xmm7

            jb aabox_side_front_face ; IF (front_face)
            vsubps xmm5, xmm7, xmm5
            aabox_side_front_face: ; ELSE
            inc dl

            movups [rsi+HITRECORD_NORMAL_OFF], xmm5
            mov [rsi+HITRECORD_FFACE_OFF], dl
            movups [rsi+HITRECORD_P_OFF], xmm4

            aabox_sides_next:
            inc cx
            cmp cx, 6
            jl aabox_sides_loop ;}}}

        aabox_loop_next:
        inc rbx
        inc r13d
        aabox_loop_cond:
        cmp r13d, r12d
        jl aabox_loop

    mov rdi, [rsp]
    mov rsi, [rsp+0x08]
    mov rcx, [rsp+0x10]
    add rsp, 0x20 ;}}}


    mov al, [rsp+0x10]

    add rsp, 0x20
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; Color[xmm0] material_emitted(Material *self[rdi])
; ret & args: rdi, xmm0
; also uses: rsi
material_emitted: ; {{{
    push rbp
    mov rbp, rsp

    xorps xmm0, xmm0
    mov esi, [rdi+MATERIAL_TYPE_OFF]
    cmp esi, 3
    jne material_emitted_ret

    movups xmm0, [rdi+MATERIAL_COLOR_OFF]

    material_emitted_ret:
    pop rbp
    ret ;}}}

;bool[ax] material_scatter(Material *self[rdi], Ray r[xmm0:xmm1], HitRecord *hit[rsi], Color *attenuation[xmm2], Ray *scattered[xmm3:xmm4]) {
; ret & args: rax, rdi, rsi, xmm0-xmm4
material_scatter: ;{{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12

    xor al, al
    mov ebx, [rdi+MATERIAL_TYPE_OFF]
    cmp ebx, 3
    je material_scatter_skip

    sub rsp, 0x50
    movaps [rsp+0x10], xmm5
    movaps [rsp+0x20], xmm6
    movaps [rsp+0x30], xmm7
    movaps [rsp+0x40], xmm8

    movups xmm6, [rsi+HITRECORD_NORMAL_OFF]

    ; LAMBERTIAN {{{
    test ebx, ebx
    jnz material_scatter_check_metal

    movaps [rsp], xmm0
    call rand_vec_in_unit
    movaps xmm5, xmm0
    movaps xmm0, [rsp]

    addps xmm5, xmm6

    ; check if scatter_dir is near zero
    put_fabs_mask xmm7
    andps xmm7, xmm5 ; fabs(scatter_dir)
    movups xmm8, [eps]
    vminps xmm8, xmm7, xmm8 ; xmm8 = min(fabs(scatter_dir), (eps,eps,eps,0))
    ptest xmm7, xmm8 ; set CF if xmm7 = xmm8 (i.e. fabs(scatter_dir) <= (eps,...))
    jnc not_near_zero ; if CF=0, scatter is not near zero
    movaps xmm5, xmm6
    not_near_zero:

    movups xmm2, [rdi+MATERIAL_COLOR_OFF]
    movups xmm3, [rsi+HITRECORD_P_OFF]
    movaps xmm4, xmm5

    mov al, 1
    ;}}}

    ; METAL {{{
    material_scatter_check_metal:
    dec ebx
    test ebx, ebx
    jnz material_scatter_done

    vdpps xmm5, xmm1, xmm1, 0xF1
    rsqrtss xmm5, xmm5
    shufps xmm5, xmm5, 0
    mulps xmm5, xmm1
    vdpps xmm7, xmm5, xmm6, 0xF1
    addss xmm7, xmm7
    shufps xmm7, xmm7, 0
    mulps xmm7, xmm6
    vsubps xmm7, xmm5, xmm7 ; xmm7 = reflected

    vdpps xmm5, xmm7, xmm6, 0xF1
    xorps xmm8, xmm8
    comiss xmm5, xmm8
    jbe material_scatter_done

    movaps [rsp], xmm0
    call rand_vec_in_unit
    movaps xmm5, xmm0
    movaps xmm0, [rsp]

    movss xmm4, [rdi+MATERIAL_ALPHA_OFF]
    shufps xmm4, xmm4, 0
    mulps xmm4, xmm5
    addps xmm4, xmm7

    movups xmm2, [rdi+MATERIAL_COLOR_OFF]
    movups xmm3, [rsi+HITRECORD_P_OFF]
    mov al, 1
    ;}}}

    material_scatter_done:
    movaps xmm5, [rsp+0x10]
    movaps xmm6, [rsp+0x20]
    movaps xmm7, [rsp+0x30]
    movaps xmm8, [rsp+0x40]
    add rsp, 0x50

    material_scatter_skip:
    pop r12
    pop rbx
    pop rbp
    ret ;}}}
