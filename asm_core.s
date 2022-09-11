; GLOBALS & EXTERNS
global vec3_add
global vec3_prod
global vec3_unscale
global vec3_sub
global vec3_norm2
global vec3_rnd_unit_sphere

global box_hit
global plane_hit
global sphere_hit
global triangle_hit

global lambertian_scatter
global metal_scatter
global dielectric_scatter

global camera_init
global camera_get_ray
global scene_render
global ray_color

extern list_get
extern tanf
extern rand
extern printf

; OFFSETS
%define REAL_SIZE 4 ; float
%define VEC3_SIZE (REAL_SIZE * 4)

%define RAY_ORIG_OFFS   0
%define RAY_DIR_OFFS    VEC3_SIZE
%define RAY_SIZE        (RAY_DIR_OFFS+VEC3_SIZE)

%define HITTABLE_HIT_OFFS        0
%define HITTABLE_DESTROY_OFFS    8
%define HITTABLE_BBOX_OFFS      16
%define HITTABLE_REFP_OFFS      24
%define HITTABLE_SIZE           (HITTABLE_REFP_OFFS+VEC3_SIZE)

%define RECORD_P_OFFS       0
%define RECORD_NORMAL_OFFS  VEC3_SIZE
%define RECORD_SM_OFFS      (RECORD_NORMAL_OFFS+VEC3_SIZE)
%define RECORD_T_OFFS       (RECORD_SM_OFFS+8)
%define RECORD_FFACE_OFFS   (RECORD_T_OFFS+REAL_SIZE)
%define RECORD_SIZE         48 ; size is aligned to largest property

%define LIST_HITTABLE_OFFS   0
%define LIST_BBOX_OFFS       HITTABLE_SIZE
%define LIST_SM_OFFS         (LIST_BBOX_OFFS+8)
%define LIST_REFPSUM_OFFS    (LIST_SM_OFFS+8)
%define LIST_LIST_OFFS       (LIST_REFPSUM_OFFS+VEC3_SIZE)
%define LIST_SIZE_OFFS       (LIST_LIST_OFFS+8)
%define LIST_CAP_OFFS        (LIST_SIZE_OFFS+4)
%define LIST_SIZE            (LIST_CAP_OFFS+4)

%define BOX_HITTABLE_OFFS   0
%define BOX_SM_OFFS         HITTABLE_SIZE
%define BOX_CBACK_OFFS      BOX_SM_OFFS+8
%define BOX_CFRONT_OFFS     (BOX_CBACK_OFFS+VEC3_SIZE)
%define BOX_FACES_OFFS      (BOX_CFRONT_OFFS+VEC3_SIZE)

%define PLANE_HITTABLE_OFFS 0
%define PLANE_SM_OFFS       HITTABLE_SIZE
%define PLANE_ORIGIN_OFFS   (PLANE_SM_OFFS+8)
%define PLANE_NORMAL_OFFS   (PLANE_ORIGIN_OFFS+VEC3_SIZE)
%define PLANE_SIZE          (PLANE_NORMAL_OFFS+VEC3_SIZE)

%define SPHERE_HITTABLE_OFFS   0
%define SPHERE_BBOX_OFFS       HITTABLE_SIZE
%define SPHERE_SM_OFFS         SPHERE_BBOX_OFFS+8
%define SPHERE_CENTER_OFFS     (SPHERE_SM_OFFS+8)
%define SPHERE_RADIUS_OFFS     (SPHERE_CENTER_OFFS+VEC3_SIZE)
%define SPHERE_SIZE            (SPHERE_RADIUS_OFFS+REAL_SIZE)

%define TRIANGLE_HITTABLE_OFFS   0
%define TRIANGLE_BBOX_OFFS       HITTABLE_SIZE
%define TRIANGLE_SM_OFFS         TRIANGLE_BBOX_OFFS+8
%define TRIANGLE_P1_OFFS         (TRIANGLE_SM_OFFS+8)
%define TRIANGLE_P2_OFFS         (TRIANGLE_P1_OFFS+VEC3_SIZE)
%define TRIANGLE_P3_OFFS         (TRIANGLE_P2_OFFS+VEC3_SIZE)
%define TRIANGLE_SIZE            (TRIANGLE_P3_OFFS+VEC3_SIZE)

%define MATERIAL_SCATTER_OFFS 0
%define MATERIAL_EMITTED_OFFS 8
%define MATERIAL_SIZE         (2*8)

%define MATALL_MATERIAL_OFFS 0
%define MATALL_ALBEDO_OFFS   MATERIAL_SIZE
%define MATALL_ALPHA_OFFS    (MATALL_ALBEDO_OFFS+VEC3_SIZE)

%define CAMERA_ORIGIN_OFFS 0
%define CAMERA_BLCORN_OFFS (CAMERA_ORIGIN_OFFS+VEC3_SIZE)
%define CAMERA_HORIZ_OFFS  (CAMERA_BLCORN_OFFS+VEC3_SIZE)
%define CAMERA_VERTI_OFFS  (CAMERA_HORIZ_OFFS+VEC3_SIZE)
%define CAMERA_U_OFFS      (CAMERA_VERTI_OFFS+VEC3_SIZE)
%define CAMERA_V_OFFS      (CAMERA_U_OFFS+VEC3_SIZE)
%define CAMERA_W_OFFS      (CAMERA_V_OFFS+VEC3_SIZE)
%define CAMERA_LR_OFFS     (CAMERA_W_OFFS+VEC3_SIZE)
%define CAMERA_SIZE        (CAMERA_LR_OFFS+REAL_SIZE)

%define SCENE_CAMERA_OFFS 0
%define SCENE_OUTPUT_OFFS CAMERA_SIZE
%define SCENE_WORLD_OFFS  (SCENE_OUTPUT_OFFS+OUTPUT_SIZE+4)
%define SCENE_SIZE (SCENE_WORLD_OFFS+8)

%define OUTPUT_WIDTH_OFFS 0
%define OUTPUT_HEIGHT_OFFS 4
%define OUTPUT_SPP_OFFS 8
%define OUTPUT_DEPTH_OFFS 12
%define OUTPUT_SIZE 16

%macro v3p_scale 3 ; v1, t, v2 -> v1=t*v2
	vshufps %2, %2, 0b00000000
	vmulps %1, %2, %3
%endmacro

%macro v3p_cross 3 ; v1 = v2 x v3 ; v2 = (x2,y2,z2,_), v3 = (x3,y3,z3,_)
	vshufps xmm6, %2, %2, 0b11001001                        ; (y2,z2,x2)
	vshufps xmm7, %3, %3, 0b11010010                        ; (z3,x3,y3)
	vmulps %1, xmm6, xmm7 ; v1 = (y2*z3, z2*x3, x2*y3)
	vshufps xmm6, %2, %2, 0b11010010                        ; (z2,x2,y2)
	vshufps xmm7, %3, %3, 0b11001001                        ; (y3,z3,x3)
	mulps xmm6, xmm7    ; (z2*y3, x2*z3, y2*x3)
	subps %1, xmm6   ; v1 = (y2*z3-z2*y3, z2*x3-x2*z3, (x2*y3-y2*x3))
%endmacro

; v3p_reflect: return, v, n
%macro v3p_reflect 3
	vdpps %1, %2, %3, 0xF1
	addss %1, %1
	shufps %1, %1, 0
	mulps %1, %3
	vsubps %1, %2, %1
%endmacro

%macro v3p_normalized  2
	vdpps %2, %1, %1, 0xF1 ; norm2
	rsqrtss %2, %2        ; 1/sqrt(norm2)
	shufps %2, %2, 0 ;normalize
	mulps %1, %2
%endmacro

; ray_at: return, ray_pointer, t
%macro ray_at 3
    movups %1, [%2+RAY_DIR_OFFS]
	shufps %3, %3, 0
	mulps %1, %3
    addps %1, [%2+RAY_ORIG_OFFS]
%endmacro

%macro rnd 1
	call rand
	pxor %1, %1
	cvtsi2ss %1, eax
	mulss %1, [rand_descale]
%endmacro

%macro fabs_mask 1
    pcmpeqw %1, %1
    psrld %1, 1
%endmacro

; set_face_normal record, ray, normal, tag
; kills: xmm0, xmm1, r8
%macro set_face_normal 4
    mov r8, rax

    xor eax, eax
    movups xmm0, [%2+RAY_DIR_OFFS]
    dpps xmm0, %3, 0xF1
    pxor xmm1, xmm1
    comiss xmm0, xmm1 ; dot < 0?
    jae .set_face_normal_%4_no_ff

    movaps xmm1, %3
    inc eax
    jmp .set_face_normal_%4_return

    .set_face_normal_%4_no_ff:
    subps xmm1, %3

    .set_face_normal_%4_return:
    mov [%1+RECORD_FFACE_OFFS], al
    movups [%1+RECORD_NORMAL_OFFS], xmm1

    mov rax, r8
%endmacro

section .data

; 32-bit absolute value mask 
rand_descale:  dd 0x2FFFFFFD
eps:  dd 1.0e-4
inf:  dd 0x7F800000
half: dd 0.5e0
halfdegtorad: dd 0x3C0EFA35

section .text

vec3_add:
    addps xmm0, xmm2
    addps xmm1, xmm3
    ret

vec3_prod:
    mulps xmm0, xmm2
    mulps xmm1, xmm3
    ret

vec3_unscale:
    shufps xmm2, xmm2, 0b00000000
    divps xmm0, xmm2
    divps xmm1, xmm2
    ret

vec3_sub:
    subps xmm0, xmm2
    subps xmm1, xmm3
    ret

vec3_norm2:
    dpps xmm1, xmm1, 0b00010001
    dpps xmm0, xmm0, 0b00110001
    addss xmm0, xmm1
    ret

vec3_rnd_unit_sphere:
    sub rsp, 0x18

    .vru_loop:
    call rand
    mov [rsp], eax

    call rand
    mov [rsp+0x04], eax

    call rand
    mov [rsp+0x08], eax

    xor eax, eax
    mov [rsp+0x0C], eax

    cvtdq2ps xmm0, [rsp]

    movss xmm1, [rand_descale]
    shufps xmm1, xmm1, 0x40
    mulps xmm0, xmm1

    addps xmm0, xmm0

    pcmpeqw xmm2, xmm2
    pslld xmm2, 25
    psrld xmm2, 2 ; (one,one,one,one)
    psrldq xmm2, 4
    subps xmm0, xmm2

    vdpps xmm1, xmm0, xmm0, 0xF1
    comiss xmm1, xmm2 ; one
    jae .vru_loop

    add rsp, 0x18
    ret

; Hittable Functions
box_hit:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    sub rsp, RECORD_SIZE+0x40

    xor al, al

    movaps [rsp], xmm0 ; t_min
    movaps [rsp+0x10], xmm1 ; t_max
    mov [rsp+0x20], rdx ; rec
    mov [rsp+0x28], rdi ; self
    mov [rsp+0x30], al ; hit_anything = false

    mov r12, [rdi+BOX_FACES_OFFS]
    mov r12, [r12+LIST_LIST_OFFS] ; self->faces->list
    mov r13, rsi ; ray
    xor r14, r14

    .bh_face_loop:
        ; Hittable *h = list_get(self->faces, i)
        mov rdi, [r12+r14*8]

        ; h->hit(h, ray, t_min, t_max, tmp)
        mov rsi, r13
        movaps xmm0, [rsp]
        movaps xmm1, [rsp+0x10]
        lea rdx, [rsp+0x40]
        call [rdi+BOX_HITTABLE_OFFS+HITTABLE_HIT_OFFS]
        test al, al
        jz .bh_face_loop_continue ; no hit

        ; box_is_inside(self, tmp.p)
        mov rdi, [rsp+0x28]
        movaps xmm0, [rsp+0x40+RECORD_P_OFFS]

        ; check if it's inside
        movups xmm1, [rdi+BOX_CBACK_OFFS]
        movss xmm2, [eps]
        shufps xmm2, xmm2, 0b01000000
        subps xmm1, xmm2
        cmpleps xmm1, xmm0 ; self->cback <= p (coord by coord)

        movups xmm3, [rdi+BOX_CFRONT_OFFS]
        addps xmm3, xmm2
        cmpnltps xmm3, xmm0  ; self->cfront >= p (coord by coord)

        andps xmm1, xmm3
        pcmpeqw xmm2, xmm2 ; fill xmm2 with ones
        ptest xmm1, xmm2 ; if xmm1 ANDN xmm2 === 0...0, then xmm2 is all ones and CF=1

        jnc .bh_face_loop_continue ; if CF=0, it's not inside

        mov byte [rsp+0x30], 1 ; hit_anything = true
        lea rdi, [rsp+0x40]
        movss xmm0, [rdi+RECORD_T_OFFS] ; xmm0 = tmp->t
        movss [rsp+0x10], xmm0 ; t_max = xmm0

        ; *rec = tmp
        movaps xmm0, [rdi+RECORD_P_OFFS]
        movaps xmm1, [rdi+RECORD_NORMAL_OFFS]
        mov r8, [rdi+RECORD_SM_OFFS]
        mov r9, [rdi+RECORD_T_OFFS]

        mov rdx, [rsp+0x20] ; rec
        movups [rdx+RECORD_P_OFFS], xmm0
        movups [rdx+RECORD_NORMAL_OFFS], xmm1
        mov [rdx+RECORD_SM_OFFS], r8
        mov [rdx+RECORD_T_OFFS], r9

        .bh_face_loop_continue:
        inc r14 ; i++
        cmp r14, 6 ; i < self->size
        jl .bh_face_loop

    mov al, [rsp+0x30]

    add rsp, RECORD_SIZE+0x40
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

plane_hit:
    push rbp
    mov rbp, rsp

    xor eax, eax; false

    ; check if ray direction is perpendicular to plane normal
    movups xmm2, [rsi+RAY_DIR_OFFS] ; ray->direction
    movups xmm3, [rdi+PLANE_NORMAL_OFFS] ; self->normal
    vdpps xmm4, xmm2, xmm3, 0xF1
    fabs_mask xmm5
    andps xmm4, xmm5
    comiss xmm4, [eps] ; xmm4 < EPS
    jbe .ph_return ; if it is, return false (no plane hit)

    movups xmm4, [rdi+PLANE_ORIGIN_OFFS] ; self->origin
    subps xmm4, [rsi+RAY_ORIG_OFFS] ; - r->origin
    dpps xmm4, xmm3, 0xF1
    dpps xmm2, xmm3, 0xF1
    divss xmm4, xmm2 ; xmm4 = t
    comiss xmm4, xmm0
    jb .ph_return  ; t < t_min
    comiss xmm1, xmm4
    jb .ph_return  ; t_max < t

    movss [rdx+RECORD_T_OFFS], xmm4 ; record->t = t
    mov r8, [rdi+PLANE_SM_OFFS]
    mov [rdx+RECORD_SM_OFFS], r8 ; record->sm = plane->sm

    ray_at xmm0, rsi, xmm4
    movups [rdx+RECORD_P_OFFS], xmm0 ; record->p = ray_at(ray, t)

    set_face_normal rdx, rsi, xmm3, plane

    inc eax ; return true

    .ph_return:
    pop rbp
    ret

sphere_hit:
    push rbp
    mov rbp, rsp

    xor eax, eax

    movups xmm2, [rsi+RAY_ORIG_OFFS]      ; r->origin
    movups xmm3, [rdi+SPHERE_CENTER_OFFS] ; center
    subps  xmm2, xmm3               ; oc = center to ray origin
    movups xmm4, [rsi+RAY_DIR_OFFS] ; r->direction
    vdpps xmm5, xmm4, xmm4, 0xF1     ; a = norm2(r->direction)
    vdpps xmm6, xmm2, xmm4, 0xF1     ; hb = half b
    xorps xmm9, xmm9
    vsubss xmm6, xmm9, xmm6          ; -hb
    dpps xmm2, xmm2, 0xF1           ; norm2(oc)
    movss xmm8, [rdi+SPHERE_RADIUS_OFFS] ; radius
    vmulss xmm7, xmm8, xmm8          ; radius^2
    subss xmm2, xmm7                ; c = norm2(oc) - radius^2
    mulss xmm2, xmm5                ; c * a
    vmulss xmm7, xmm6, xmm6          ; hb^2 = (-hb) * (-hb)
    vsubss xmm2, xmm7, xmm2 ; discriminant

    pxor xmm9, xmm9
    comiss xmm2, xmm9
    jb .sphere_hit_return

    sqrtss xmm2, xmm2 ; sqrt(discriminant)
    vsubss xmm7, xmm6, xmm2
    divss xmm7, xmm5  ; root = (-hb - sqrtd) / a
    comiss xmm7, xmm0
    jb .sphere_hit_check_other_root ; root < t_min
    comiss xmm1, xmm7
    jae .sphere_hit_did_hit ; t_max >= root

    .sphere_hit_check_other_root:
    vaddss xmm7, xmm6, xmm2
    divss xmm7, xmm5  ; root = (-hb + sqrtd) / a
    comiss xmm7, xmm0 ; root < t_min
    jb .sphere_hit_return
    comiss xmm1, xmm7 ; t_max < root
    jb .sphere_hit_return

    .sphere_hit_did_hit:
    inc eax
    movss [rdx+RECORD_T_OFFS], xmm7
    mov r8, [rdi+SPHERE_SM_OFFS]
    mov [rdx+RECORD_SM_OFFS], r8

    ray_at xmm2, rsi, xmm7
    movups [rdx+RECORD_P_OFFS], xmm2

    shufps xmm8, xmm8, 0b00000000
    subps xmm2, xmm3
    divps xmm2, xmm8 ; outward_normal
    set_face_normal rdx, rsi, xmm2, sphere

    .sphere_hit_return:
    pop rbp
    ret

triangle_hit:
    push rbp
    mov rbp, rsp

    xor eax, eax

    movups xmm2, [rdi+TRIANGLE_P1_OFFS]
    movups xmm3, [rdi+TRIANGLE_P2_OFFS]
    movups xmm4, [rdi+TRIANGLE_P3_OFFS]
    vsubps xmm8, xmm3, xmm2
    vsubps xmm9, xmm4, xmm2
    v3p_cross xmm5, xmm8, xmm9 ; xmm5 = normal

    ; perpendicular(normal, r->direction)
    movups xmm6, [rsi+RAY_DIR_OFFS] ; r->direction
    vdpps xmm7, xmm5, xmm6, 0xF1
    fabs_mask xmm8
    andps xmm7, xmm8
    comiss xmm7, [eps] ; xmm7 < EPS
    jb .th_return

    ; check if ray intersection is outside of t_range
    vsubps xmm8, xmm2, [rsi+RAY_ORIG_OFFS]
    dpps xmm8, xmm5, 0xF1 ; t_nom
    dpps xmm6, xmm5, 0xF1 ; t_denom
    divss xmm8, xmm6 ; t
    comiss xmm8, xmm0
    jb .th_return
    comiss xmm1, xmm8
    jb .th_return

    mov rcx, [rdi+TRIANGLE_SM_OFFS] ; save sm, since we'll overwrite rdi

    ray_at xmm0, rsi, xmm8

    ; first boundary check
    vsubps xmm9, xmm3, xmm2 ; (p2 - p1)
    vsubps xmm10, xmm0, xmm2 ; (p - p1)
    v3p_cross xmm1, xmm9, xmm10
    dpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
    pxor xmm9, xmm9
    comiss xmm1, xmm9
    jbe .th_return ; dot(...) <= 0

    ;second boundary check
    vsubps xmm9, xmm4, xmm3 ; (p3 - p2)
    vsubps xmm10, xmm0, xmm3 ; (p - p2)
    v3p_cross xmm1, xmm9, xmm10
    dpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
    pxor xmm9, xmm9
    comiss xmm1, xmm9
    jbe .th_return ; dot(...) <= 0

    ;third boundary check
    vsubps xmm9, xmm2, xmm4 ; (p1 - p3)
    vsubps xmm10, xmm0, xmm4 ; (p - p3)
    v3p_cross xmm1, xmm9, xmm10
    dpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
    pxor xmm9, xmm9
    comiss xmm1, xmm9
    jbe .th_return ; dot(...) <= 0

    ; triangle hit
    inc eax
    movss [rdx+RECORD_T_OFFS], xmm8
    movups [rdx+RECORD_P_OFFS], xmm0
    mov [rdx+RECORD_SM_OFFS], rcx
    set_face_normal rdx, rsi, xmm5, triangle

    .th_return:
    pop rbp
    ret

lambertian_scatter:
    push rbp
    mov rbp, rsp
    sub rsp, 0x10

    ; *attenuation = self->albedo
    movups xmm0, [rdi+MATALL_ALBEDO_OFFS]
    movups [rcx], xmm0

    mov [rsp], rdx
    mov [rsp+0x08], r8

    call vec3_rnd_unit_sphere

    mov rdx, [rsp]
    mov r8, [rsp+0x08]

    xor eax, eax
    inc eax

    movups xmm1, [rdx+RECORD_NORMAL_OFFS] ; rec->normal
    addps xmm0, xmm1 ; scatter_dir

    ; check if scatter_dir is near zero
    fabs_mask xmm2
    andps xmm2, xmm0 ; fabs(scatter_dir)

    movss xmm3, [eps]
    shufps xmm3, xmm3, 0b00000000
    vminps xmm5, xmm2, xmm3 ; xmm5 = min(fabs(scatter_dir), (eps,eps,eps,eps))

    ptest xmm5, xmm2 ; set CF if xmm5 = xmm2 (i.e. fabs(scatter_dir) < (eps,...))
    jnc .ls_not_near_zero ; if CF=0, scatter is not near zero

    movaps xmm0, xmm1 ; scatter_dir = rec->normal

    .ls_not_near_zero:
    ; *scattered = Ray(p, scatter_dir)
    movups xmm1, [rdx+RECORD_P_OFFS]
    movups [r8+RAY_ORIG_OFFS], xmm1
    movups [r8+RAY_DIR_OFFS], xmm0

    add rsp, 0x10
    pop rbp
    ret

metal_scatter:
	push rbp
	mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14

	; *attenuation = self->albedo
	movups xmm0, [rdi+MATALL_ALBEDO_OFFS]
	movups [rcx], xmm0

	; scattered->orig = rec->p
	movups xmm0, [rdx+RECORD_P_OFFS]
	movups [r8+RAY_ORIG_OFFS], xmm0

    mov rbx, rdi
    mov r12, rsi
    mov r13, rdx
    mov r14, r8

	call vec3_rnd_unit_sphere

    mov rdi, rbx
    mov rsi, r12
    mov rdx, r13
    mov r8, r14

    ; reflected
	movups xmm3, [rsi+RAY_DIR_OFFS]
	v3p_normalized xmm3, xmm2
	movups xmm1, [rdx+RECORD_NORMAL_OFFS]
	v3p_reflect xmm4, xmm3, xmm1

	; scattered->dir = reflected + self->fuzz * vec3_rnd_unit_sphere()
	movss xmm3, [rdi+MATALL_ALPHA_OFFS]
	shufps xmm3, xmm3, 0x80
	mulps xmm0, xmm3
	addps xmm0, xmm4
	movups [r8+RAY_DIR_OFFS], xmm0

	xor rax, rax

	dpps xmm1, xmm4, 0xF1
    pxor xmm2, xmm2
	comiss xmm1, xmm2

	jbe .ms_return ; dot(...) <= 0
	inc rax
	
	.ms_return:
    pop r14
    pop r13
    pop r12
    pop rbx
	pop rbp
	ret

dielectric_scatter:
	push rbp
	mov rbp, rsp

    push rdi
    push rsi
    push rdx
    push rcx
    push r8
    sub rsp, 8
	rnd xmm0
    add rsp, 8
    pop r8
    pop rcx
    pop rdx
    pop rsi
    pop rdi

	movups xmm1, [rdi+MATALL_ALPHA_OFFS] ; ref_ratio = self->ir
	mov al, [rdx+RECORD_FFACE_OFFS]
	test al, al
	jz .ds_not_fface

	rcpss xmm1, xmm1

	.ds_not_fface:
	movups xmm2, [rsi+RAY_DIR_OFFS]
	v3p_normalized xmm2, xmm3 ; unit_dir

	pxor xmm3, xmm3
	vsubps xmm5, xmm3, xmm2 ; -unit_dir
	movups xmm9, [rdx+RECORD_NORMAL_OFFS] ; rec->normal
	dpps xmm5, xmm9, 0xF1

    pcmpeqw xmm3, xmm3
    pslld xmm3, 25
    psrld xmm3, 2 ; xmm3 = (1,1,1,1)

	minss xmm5, xmm3 ; cos_theta = min(dot(-unit_dir, rec->normal), 1)
	vmulss xmm6, xmm5, xmm5
	vsubss xmm6, xmm3, xmm6
	sqrtss xmm6, xmm6 ; sin_theta
	mulss xmm6, xmm1

	comiss xmm6, xmm3 ; ref_ratio * sin_theta > 1
	ja .ds_do_reflect ; cannot refract

	; Use Schlick's approximation for reflectance.
	vsubss xmm6, xmm3, xmm1
	vaddss xmm7, xmm3, xmm1
	divss xmm6, xmm7 ; r0
	mulss xmm6, xmm6 ; r0 *= r0

	vsubss xmm5, xmm3, xmm5 ; (1-cos_theta)
	movss xmm8, xmm5
	mulss xmm5, xmm5 ; (1-cos_theta)^2
	mulss xmm5, xmm5 ; (1-cos_theta)^4
	mulss xmm5, xmm8 ; (1-cos_theta)^5
	vsubss xmm7, xmm3, xmm6 ; (1-r0)
	mulss xmm7, xmm5
	addss xmm6, xmm7

	comiss xmm6, xmm0
	jbe .ds_do_refract ; reflectance <= rnd

	.ds_do_reflect:
	v3p_reflect xmm0, xmm2, xmm9
	jmp .ds_return

	.ds_do_refract:
	pxor xmm4, xmm4
	subps xmm4, xmm2
	dpps xmm4, xmm9, 0xF1
	minss xmm4, xmm3 ; cos_theta
	shufps xmm4, xmm4, 0
	vmulps xmm5, xmm4, xmm9
	addps xmm5, xmm2
	shufps xmm1, xmm1, 0
	mulps xmm5, xmm1 ; r_out_perp

	vdpps xmm6, xmm5, xmm5, 0xF1
	vsubss xmm6, xmm3, xmm6
    fabs_mask xmm8
	andps xmm6, xmm8
	sqrtss xmm6, xmm6
	shufps xmm6, xmm6, 0
	mulps xmm6, xmm9
	vsubps xmm0, xmm5, xmm6 ; sub instead of add, because we use sqrt instead of -sqrt

	.ds_return:
	; *attenuation = self->albedo
	movups xmm1, [rdi+MATALL_ALBEDO_OFFS]
	movups [rcx], xmm1

	movups xmm1, [rdx+RECORD_P_OFFS]
	movups [r8+RAY_ORIG_OFFS], xmm1
	movups [r8+RAY_DIR_OFFS], xmm0

    xor al, al
    inc al

	pop rbp
	ret

camera_init:
	push rbp
	mov rbp, rsp
	sub rsp, 0x60

	pslldq xmm1, 8
	orps xmm0, xmm1 ; from
	movups [rdi+CAMERA_ORIGIN_OFFS], xmm0 ; c->origin = from

	pslldq xmm3, 8
	vorps xmm1, xmm2, xmm3 ; to
	vsubps xmm1, xmm0, xmm1 ; from-to

	pslldq xmm5, 8
	vorps xmm2, xmm4, xmm5 ; vup

	movss xmm3, [rbp+0x10] ; aperture
	mulss xmm3, [half]
	movss [rdi+CAMERA_LR_OFFS], xmm3 ; c->lens_radius = aperture/2.0

	movss xmm3, [rbp+0x18] ; focus_dist

	movaps [rsp], xmm0      ; from
	movaps [rsp+0x10], xmm1 ; from-to
	movaps [rsp+0x20], xmm2 ; vup
	movaps [rsp+0x30], xmm6 ; vfov
	movaps [rsp+0x40], xmm7 ; aspect_ratio
	movaps [rsp+0x50], xmm3 ; focus_dist
	push rdi
	sub rsp, 8

	vmulss xmm0, xmm6, [halfdegtorad] ; theta
	call tanf

	add rsp, 8
	pop rdi
	movaps xmm1, [rsp+0x10] ; from-to
	movaps xmm2, [rsp+0x20] ; vup
	movaps xmm6, [rsp+0x30] ; vfov
	movaps xmm7, [rsp+0x40] ; aspect_ratio
	movaps xmm3, [rsp+0x50] ; focus_dist

	vaddss xmm4, xmm0, xmm0 ; vp_height
	vmulss xmm5, xmm7, xmm4 ; vp_width

	v3p_normalized xmm1, xmm8
	movups [rdi+CAMERA_W_OFFS], xmm1

	v3p_cross xmm8, xmm2, xmm1
	v3p_normalized xmm8, xmm2
	movups [rdi+CAMERA_U_OFFS], xmm8

	v3p_cross xmm9, xmm1, xmm8
	movups [rdi+CAMERA_V_OFFS], xmm9

	mulss xmm5, xmm3
	shufps xmm5, xmm5, 0
	mulps xmm5, xmm8
	movups [rdi+CAMERA_HORIZ_OFFS], xmm5

	mulss xmm4, xmm3
	shufps xmm4, xmm4, 0
	mulps xmm4, xmm9
	movups [rdi+CAMERA_VERTI_OFFS], xmm4

	movaps xmm0, [rsp]
	addps xmm4, xmm5
	movss xmm5, [half]
	shufps xmm5, xmm5, 0
	mulps xmm4, xmm5
	shufps xmm3, xmm3, 0
	mulps xmm1, xmm3
	addps xmm1, xmm4
	subps xmm0, xmm1
	movups [rdi+CAMERA_BLCORN_OFFS], xmm0

	add rsp, 0x60
	pop rbp
	ret

camera_get_ray:
	push rbp
	mov rbp, rsp
    sub rsp, 0x20

	movss xmm2, xmm0

    mov [rsp], rdi
    mov [rsp+0x08], rsi
    movss [rsp+0x10], xmm2
    movss [rsp+0x14], xmm1

	call vec3_rnd_unit_sphere ; xmm0=rand_unit

    mov rdi, [rsp]
    mov rsi, [rsp+0x08]
    movss xmm2, [rsp+0x10]
    movss xmm1, [rsp+0x14]

	movss xmm3, [rsi+CAMERA_LR_OFFS]
	shufps xmm3, xmm3, 0
	mulps xmm0, xmm3 ; rd

	vshufps xmm3, xmm0, xmm0, 0
	mulps xmm3, [rsi+CAMERA_U_OFFS]

	vshufps xmm4, xmm0, xmm0, 0b01010101
	mulps xmm4, [rsi+CAMERA_V_OFFS]
	addps xmm3, xmm4 ; offset

	addps xmm3, [rsi+CAMERA_ORIGIN_OFFS]
	movups [rdi+RAY_ORIG_OFFS], xmm3

	shufps xmm2, xmm2, 0
	mulps xmm2, [rsi+CAMERA_HORIZ_OFFS]
	addps xmm2, [rsi+CAMERA_BLCORN_OFFS]
	shufps xmm1, xmm1, 0
	mulps xmm1, [rsi+CAMERA_VERTI_OFFS]
	subps xmm1, xmm3
	addps xmm1, xmm2
	movups [rdi+RAY_DIR_OFFS], xmm1

    add rsp, 0x20
	pop rbp
	ret

%define RENDER_WIDTH_OFFS    0
%define RENDER_HEIGHT_OFFS   (RENDER_WIDTH_OFFS+4)
%define RENDER_RAND1_OFFS    (RENDER_HEIGHT_OFFS+4)
%define RENDER_RAND2_OFFS    (RENDER_RAND1_OFFS+REAL_SIZE)
%define RENDER_RAY_OFFS      (RENDER_RAND2_OFFS+REAL_SIZE)
%define RENDER_PIXEL_OFFS    (RENDER_RAY_OFFS+RAY_SIZE)
%define RENDER_BG_OFFS       (RENDER_PIXEL_OFFS+VEC3_SIZE)
%define RENDER_ARGS_SIZE     (RENDER_BG_OFFS+VEC3_SIZE)

scene_render:
    push rbp
    mov rbp, rsp
    push rbx
    push r12
    push r13
    push r14
    push r15
    sub rsp, RENDER_ARGS_SIZE
    sub rsp, 8 ; align 16 bytes

    mov r15, rdi ; backup scene
    mov rbx, rsi ; backup image
    mov edx, [rdi+SCENE_OUTPUT_OFFS+OUTPUT_HEIGHT_OFFS]
    mov [rsp+RENDER_HEIGHT_OFFS], edx ; height
    mov ecx, [rdi+SCENE_OUTPUT_OFFS+OUTPUT_WIDTH_OFFS]
    mov [rsp+RENDER_WIDTH_OFFS], ecx ; width
    xor r12d, r12d ; initial j value
    pxor xmm0, xmm0
    movaps [rsp+RENDER_BG_OFFS], xmm0

    .height_loop:
        xor r13d, r13d ; initial i value
        .width_loop: ; 
            xor r14d, r14d
            pxor xmm0, xmm0
            movaps [rsp+RENDER_PIXEL_OFFS], xmm0
            .spp_loop: ; 
                rnd xmm0
                movss [rsp+RENDER_RAND1_OFFS], xmm0
                rnd xmm0

                ; calculate u vec
                cvtsi2ss xmm2, r13d ; (float) i
                mov edi, [rsp+RENDER_WIDTH_OFFS]
                dec edi ; width - 1
                cvtsi2ss xmm3, edi ; (float) (width - 1)
                addss xmm0, xmm2
                divss xmm0, xmm3 ; u = (i + rnd()) / (width - 1)

                ; calculate v vec
                movss xmm1, [rsp+RENDER_RAND1_OFFS]
                cvtsi2ss xmm2, r12d ; (float) j
                mov edi, [rsp+RENDER_HEIGHT_OFFS]
                dec edi ; height - 1
                cvtsi2ss xmm3, edi ; (float) (height - 1)
                addss xmm1, xmm2
                divss xmm1, xmm3 ; v = (j + rnd()) / (height - 1)

                lea rdi, [rsp+RENDER_RAY_OFFS] ; prepare space for return Ray
                lea rsi, [r15+SCENE_CAMERA_OFFS] ; s->camera
                call camera_get_ray

                mov rsi, rdi ; ray address
                mov rdi, [r15+SCENE_WORLD_OFFS] ; s->world
                vmovaps xmm0, [rsp+RENDER_BG_OFFS]
                mov edx, [r15+SCENE_OUTPUT_OFFS+OUTPUT_DEPTH_OFFS]
                call ray_color

                addps xmm0, [rsp+RENDER_PIXEL_OFFS]
                movaps [rsp+RENDER_PIXEL_OFFS], xmm0

                inc r14d
                cmp r14d, [r15+SCENE_OUTPUT_OFFS+OUTPUT_SPP_OFFS]
                jl .spp_loop 

            mov eax, [rsp+RENDER_WIDTH_OFFS]
            mul r12d
            add eax, r13d ; idx = j * w + i
            sal eax, 4    ; * 16 (Vec3)
            movups [rbx+rax], xmm0 ; image[idx] = pixel

            inc r13d
            cmp r13d, [rsp+RENDER_WIDTH_OFFS]
            jl .width_loop 
        inc r12d
        cmp r12d, [rsp+RENDER_HEIGHT_OFFS]
        jl .height_loop

    add rsp, 8
    add rsp, RENDER_ARGS_SIZE
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    pop rbp
    ret

ray_color:
    push rbp
    mov rbp, rsp
    push r12
    push r13
    push r14
    push r15
    sub rsp, RECORD_SIZE+RAY_SIZE+3*VEC3_SIZE

    movaps xmm1, xmm0 ; save bg color
    pxor xmm0, xmm0
    test edx, edx ; check depth <= 0
    jz .rc_return

    ; save params
    movaps [rsp], xmm1
    mov r12, rdi
    mov r13, rsi
    mov r14, rdx

    mov rax, [rdi+HITTABLE_HIT_OFFS]
    movss xmm0, [eps]
    movss xmm1, [inf]
    lea rdx, [rsp+3*VEC3_SIZE]
    call rax ; world->hit(...)

    ; restore params
    movaps xmm1, [rsp]
    mov rdi, r12
    mov rsi, r13
    mov rdx, r14

    test al, al
    jnz .rc_world_hit

    ; no hit, return bg
    movaps xmm0, xmm1
    jmp .rc_return

    .rc_world_hit:
    ; params already saved
    lea rsi, [rsp+3*VEC3_SIZE]
    mov rdi, [rsi+RECORD_SM_OFFS] ; get material
    mov rdi, [rdi]
    mov r15, rdi ; save material
    mov rax, [rdi+MATERIAL_EMITTED_OFFS]

    call rax ; m->emitted(m)
    pslldq xmm1, 8
    orps xmm0, xmm1 ; emit functions implemented in C
    movaps [rsp+VEC3_SIZE], xmm0 ; save emitted

    mov rdi, r15
    mov rsi, r13
    lea rdx, [rsp+3*VEC3_SIZE] ; record
    lea rcx, [rsp+2*VEC3_SIZE] ; attenuation
    lea r8, [rsp+3*VEC3_SIZE+RECORD_SIZE] ; scattered
    mov rax, [rdi+MATERIAL_SCATTER_OFFS]

    call rax

    test al, al
    jnz .rc_mat_scatter

    ; no scatter, return emitted
    movaps xmm0, [rsp+VEC3_SIZE]
    jmp .rc_return

    .rc_mat_scatter:
    mov rdi, r12
    lea rsi, [rsp+3*VEC3_SIZE+RECORD_SIZE] ; scattered
    mov rdx, r14
    dec edx ; depth - 1
    movaps xmm0, [rsp] ; restore bg
    call ray_color

    mulps xmm0, [rsp+2*VEC3_SIZE] ; attenuation
    addps xmm0, [rsp+VEC3_SIZE]   ; emitted

    .rc_return:
    add rsp, RECORD_SIZE+RAY_SIZE+3*VEC3_SIZE
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbp
    ret
; vi: ft=x86asm ts=4 sw=4 et
