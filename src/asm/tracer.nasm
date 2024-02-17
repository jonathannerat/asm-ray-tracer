; vi: ft=nasm ts=4 sw=4 et fdm=marker
global tracer_asm
global mtsrand
global mtfrand

%include "constants.nasm"
%include "macros.nasm"

section .data

; 32-bit absolute value mask
eps:  times 3 dd 1.0e-4
zero: dd 0.0e0
inf:  dd 0x7F800000

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

    xor r12d, r12d ; j = 0
    height_loop: ;{{{

        ; error on j==19 && i == 111 && k == (24;30)

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

%assign RAYCOLOR_HR_OFF 0
%assign RAYCOLOR_RAY_OFF HITRECORD_SIZE ; 48
%assign RAYCOLOR_SCATTERED_OFF RAYCOLOR_RAY_OFF+RAY_SIZE ; 80
%assign RAYCOLOR_ATTENUATION_OFF RAYCOLOR_SCATTERED_OFF+RAY_SIZE ; 112
%assign RAYCOLOR_EMITTED_OFF RAYCOLOR_ATTENUATION_OFF+VEC3_SIZE ;128
%assign RAYCOLOR_WORLD_OFF RAYCOLOR_EMITTED_OFF+VEC3_SIZE ; 144
%assign RAYCOLOR_DEPTH_OFF RAYCOLOR_WORLD_OFF+POINTER_SIZE ; 152
%assign RAYCOLOR_SIZE RAYCOLOR_DEPTH_OFF+INT_SIZE+4 ; 156 + 4 (padding) = 160 bytes


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

%assign SURFACE_TYPE_PLANE     0
%assign SURFACE_TYPE_SPHERE    1
%assign SURFACE_TYPE_LIST      2
%assign SURFACE_TYPE_AABOX     3
%assign SURFACE_TYPE_TRIANGLE  4
%assign SURFACE_TYPE_KDTREE    5

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
    mov r14, [rdi+LIST_SURFACES_OFF]

    ;FOR j = 0 to plane count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_PLANE]
    xor r13d, r13d ; j = 0
    jmp plane_loop_cond
    plane_loop:
        mov r15, [r14+rbx*8]
        movups xmm4, [r15+PLANE_ORIGIN_OFF]
        movups xmm5, [r15+PLANE_NORMAL_OFF]

        plane_hit_macro plane_loop_next

        ; PLANE HIT
        or byte [rsp+0x10], 1
        movss xmm3, xmm6

        shufps xmm6, xmm6, 0
        mulps xmm6, xmm1
        addps xmm6, xmm0
        save_hit PLANE, r15, xmm6

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
        mulps xmm6, xmm1
        addps xmm6, xmm0
        vsubps xmm7, xmm6, xmm4
        shufps xmm5, xmm5, 0
        vdivps xmm5, xmm7, xmm5

        save_hit SPHERE, r15, xmm6

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

    ;FOR j = 0 to aabox count {{{
    mov [rsp], rdi

    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_AABOX]
    xor r13d, r13d ; j = 0
    jmp aabox_loop_cond
    aabox_loop:
        mov rdi, [r14+rbx*8]
        call aabox_hit
        or [rsp+0x10], al

        inc rbx
        inc r13d
        aabox_loop_cond:
        cmp r13d, r12d
        jl aabox_loop

    mov rdi, [rsp] ;}}}

    ;FOR j = 0 to triangle count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_TRIANGLE]
    xor r13d, r13d ; j = 0
    jmp triangle_loop_cond
    triangle_loop:
        mov r15, [r14+rbx*8]
        movups xmm4, [r15+TRIANGLE_P1_OFF]
        movups xmm5, [r15+TRIANGLE_P2_OFF]
        movups xmm6, [r15+TRIANGLE_P3_OFF]

        ; triangle plane normal
        vsubps xmm7, xmm5, xmm4
        vsubps xmm8, xmm6, xmm4
        vec_cross xmm9, xmm7, xmm8, xmm10, xmm11
        vdpps xmm10, xmm9, xmm9, 0xF1
        rsqrtss xmm10, xmm10
        shufps xmm10, xmm10, 0
        mulps xmm9, xmm10

        ; ray is perpendicular to triangle plane?
        vdpps xmm7, xmm1, xmm9, 0xF1
        put_fabs_mask xmm8
        andps xmm7, xmm8
        comiss xmm7, [eps]
        jb triangle_loop_next

        ; contact is in allowed range?
        vsubps xmm7, xmm4, xmm0
        dpps xmm7, xmm9, 0xF1
        vdpps xmm8, xmm1, xmm9, 0xF1
        divss xmm7, xmm8
        comiss xmm7, xmm2
        jb triangle_loop_next
        comiss xmm3, xmm7
        jb triangle_loop_next

        ; xmm8 = ray_at(xmm0:xmm1, xmm7)
        shufps xmm7, xmm7, 0
        vmulps xmm8, xmm7, xmm1
        addps xmm8, xmm0

        ; check that xmm8 is left of each edge
        is_left_of xmm4, xmm5
        is_left_of xmm5, xmm6
        is_left_of xmm6, xmm4

        ; triangle hit
        or byte [rsp+0x10], 1
        movss xmm3, xmm7

        movaps xmm5, xmm9 ; move normal to xmm5
        movaps xmm6, xmm8 ; move point to xmm6
        save_hit TRIANGLE, r15, xmm6

        triangle_loop_next:
        inc rbx
        inc r13d
        triangle_loop_cond:
        cmp r13d, r12d
        jl triangle_loop ;}}}

    ;FOR j = 0 to kdtree count {{{
    mov r12d, [rdi+LIST_COUNTS_OFF+4*SURFACE_TYPE_KDTREE]
    xor r13d, r13d ; j = 0
    jmp kdtree_loop_cond
    kdtree_loop:
        sub rsp, 0x50
        mov [rsp+HITRECORD_SIZE], rdi
        mov [rsp+HITRECORD_SIZE+8], rsi
        mov rdi, [r14+rbx*8]
        mov rdi, [rdi+KDTREE_ROOT_OFF]
        mov rsi, rsp

        call kdtree_node_hit

        mov rdi, [rsp+HITRECORD_SIZE]
        mov rsi, [rsp+HITRECORD_SIZE+8]
        add rsp, 0x50

        test al, al
        jz kdtree_loop_next

        or [rsp+0x10], al
        movss xmm3, [rsp-0x50+HITRECORD_T_OFF]

        ; copy all 48 bytes of the HitRecord
        movaps xmm4, [rsp-0x50]
        movaps [rsi], xmm4
        movaps xmm4, [rsp-0x40]
        movaps [rsi+0x10], xmm4
        movaps xmm4, [rsp-0x30]
        movaps [rsi+0x20], xmm4

        kdtree_loop_next:
        inc rbx
        inc r13d
        kdtree_loop_cond:
        cmp r13d, r12d
        jl kdtree_loop ;}}}

    mov al, [rsp+0x10]

    add rsp, 0x20
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; Check if ray hits any surface on the node
; bool[ax] kdtree_node_hit(KDTreeNode *l[rdi], Ray r[xmm0:xmm1], real t_min[xmm2], real t_max xmm3, HitRecord *hit[rsi])
; ret & args: rax, rdi, rsi, xmm0-xmm3
kdtree_node_hit: ; {{{
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push rdx
    sub rsp, 2*HITRECORD_SIZE+0x10

    mov r12, rdi
    mov r13, rsi
    movaps [rsp+2*HITRECORD_SIZE], xmm3

    mov rdi, [rdi+SURFACE_BBOX_OFF]
    mov rsi, rsp
    call aabox_hit

    mov dl, al
    xor al, al
    test dl, dl
    jz kdtree_node_ret

    mov rdi, [r12+KDTREE_NODE_OBJECTS_OFF]
    movaps xmm3, [rsp+2*HITRECORD_SIZE]

    test rdi, rdi
    jz kdtree_node_recursive

    ; Call list_hit on node objects, and return
    mov rsi, r13
    call list_hit
    jmp kdtree_node_ret

    ; Recursively call kdtree_node_hit on each leaf
    kdtree_node_recursive:
    mov rdi, [r12+KDTREE_NODE_LEFT_OFF]
    mov rsi, rsp
    call kdtree_node_hit
    mov dl, al

    mov rdi, [r12+KDTREE_NODE_RIGHT_OFF]
    lea rsi, [rsp+HITRECORD_SIZE]
    call kdtree_node_hit

    ; dl == left_hit, al == right_hit
    mov rbx, rsp ; rbx = left_rec

    test dl, al
    jz kdtree_node_check_right ; IF (left_hit && right_hit) {{{
        movss xmm3, [rsp+HITRECORD_T_OFF]
        comiss xmm3, [rsp+HITRECORD_SIZE+HITRECORD_T_OFF]
        jb kdtree_node_save_hit ; left.t < right.t ? save hit as is
        ; else, keep going and eventually add HITRECORD_SIZE to rbx, so we use right_rec
    ;}}}

    kdtree_node_check_right:
    test al, al
    jz kdtree_node_save_hit
    add rbx, HITRECORD_SIZE

    kdtree_node_save_hit:
    movups xmm3, [rbx]
    movups [r13], xmm3
    movups xmm3, [rbx+0x10]
    movups [r13+0x10], xmm3
    movups xmm3, [rbx+0x20]
    movups [r13+0x20], xmm3

    or al, dl

    kdtree_node_ret:
    movaps xmm3, [rsp+2*HITRECORD_SIZE]

    add rsp, 2*HITRECORD_SIZE+0x10
    pop rdx
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret ;}}}

; Hit an AABox
; bool[ax] aabox_hit(AABox *a[rdi], Ray r[xmm0:xmm1], real t_min[xmm2], real t_max xmm3, HitRecord *hit[rsi])
; ret & args: rax, rdi, rsi, xmm0-xmm3
aabox_hit: ;{{{
    push rbp
    mov rbp, rsp
    push rcx
    push rdx

    xor al, al

    xor cx, cx
    aabox_sides_loop: ;{{{
        mov rdx, [rdi+AABOX_SIDES_OFF]
        mov rdx, [rdx+rcx*8]
        movups xmm4, [rdx+PLANE_ORIGIN_OFF]
        movups xmm5, [rdx+PLANE_NORMAL_OFF]

        plane_hit_macro aabox_sides_next

        vshufps xmm4, xmm6, xmm6, 0
        mulps xmm4, xmm1
        addps xmm4, xmm0

        movups xmm7, [rdi+AABOX_PMIN_OFF]
        movups xmm8, [eps]
        subps xmm7, xmm8
        vminps xmm9, xmm7, xmm4 ; xmm9 = packed_min(xmm7, xmm4)
        ptest xmm9, xmm7 ; CF = 1 <==> xmm9 == xmm7 <==> xmm7 <= xmm4
        jnc aabox_sides_next ; CF == 0 ==> point is not inside box

        addps xmm8, [rdi+AABOX_PMAX_OFF]
        vminps xmm9, xmm8, xmm4 ; xmm9 = packed_min(xmm8, xmm6)
        ptest xmm9, xmm4 ; CF = 1 <==> xmm9 == xmm4 <==> xmm4 <= xmm8
        jnc aabox_sides_next ; CF == 0 ==> point is not inside box

        ; aabox hit
        or al, 1
        movss xmm3, xmm6

        save_hit AABOX, rdi, xmm4

        aabox_sides_next:
        inc cx
        cmp cx, 6
        jl aabox_sides_loop ;}}}

    pop rdx
    pop rcx
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
    jnz material_scatter_check_dielectric

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

    ; DIELECTRIC {{{
    material_scatter_check_dielectric:
    dec ebx
    jnz material_scatter_done

    movups xmm2, [rdi+MATERIAL_COLOR_OFF]
    movups xmm3, [rsi+HITRECORD_P_OFF]
    movss xmm5, [rdi+MATERIAL_ALPHA_OFF]

    mov al, [rsi+HITRECORD_FFACE_OFF] ; IF (front face) {{{
    test al, al
    jz skip_invert
    rcpss xmm5, xmm5
    skip_invert: ;}}}

    ; normaliza r->direction
    vdpps xmm6, xmm1, xmm1, 0xF1
    rsqrtss xmm6, xmm6
    shufps xmm6, xmm6, 0
    mulps xmm6, xmm1 ; unit_dir

    ; calculate cos(theta) of angle between ray dir and surface normal
    movups xmm7, [rsi+HITRECORD_NORMAL_OFF]
    xorps xmm8, xmm8
    subps xmm8, xmm6
    dpps xmm8, xmm7, 0xF1
    put_ones xmm9 ; ones
    minss xmm8, xmm9 ; cos_theta

    ; calculate sin(theta)
    vmulss xmm10, xmm8, xmm8
    vsubss xmm10, xmm9, xmm10
    sqrtss xmm10, xmm10 ; sin_theta

    ; check if refraction is posible
    mulss xmm10, xmm5
    comiss xmm10, xmm9
    ja dielectric_reflect ; cannot refract, so we reflect

    ; Schlick's approximation for reflactance
    vsubss xmm10, xmm9, xmm5
    vaddss xmm11, xmm9, xmm5
    divss xmm10, xmm11
    mulss xmm10, xmm10 ; r0 ** 2
    vsubss xmm11, xmm9, xmm8
    movss xmm12, xmm11 ; (1-cos)
    mulss xmm11, xmm11 ; (1-cos) ** 2
    mulss xmm11, xmm11 ; (1-cos) ** 4
    mulss xmm11, xmm12 ; (1-cos) ** 5
    vsubss xmm12, xmm9, xmm10
    mulss xmm12, xmm11
    addss xmm12, xmm10

    movaps [rsp], xmm0
    call mtfrand
    movss xmm10, xmm0
    movaps xmm0, [rsp]
    comiss xmm12, xmm10
    ja dielectric_reflect

    ; refract
    shufps xmm8, xmm8, 0
    mulps xmm8, xmm7
    addps xmm8, xmm6
    shufps xmm5, xmm5, 0
    mulps xmm8, xmm5 ; r_out_perp
    vdpps xmm10, xmm8, xmm8, 0xF1
    subss xmm9, xmm10
    put_fabs_mask xmm10
    andps xmm9, xmm10
    sqrtss xmm9, xmm9
    shufps xmm9, xmm9, 0
    mulps xmm9, xmm7 ; r_out_par
    vsubps xmm4, xmm8, xmm9
    jmp material_scatter_done

    dielectric_reflect:
    vdpps xmm10, xmm7, xmm6, 0xF1
    addss xmm10, xmm10
    shufps xmm10, xmm10, 0
    mulps xmm10, xmm7
    vsubps xmm4, xmm6, xmm10
    ;}}}

    material_scatter_done:
    mov al, 1
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
