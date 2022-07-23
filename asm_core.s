; GLOBALS & EXTERNS {{{
global vec3_add
global vec3_prod
global vec3_unscale
global vec3_sub
global vec3_norm2

global box_hit
global plane_hit

extern list_get
; }}}

; OFFSETS {{{
%define REAL_SIZE 4 ; float
%define VEC3_SIZE (REAL_SIZE * 4)

%define RAY_DIR_OFFS VEC3_SIZE

; Hittable {{{
%define HITTABLE_DESTROY_OFFS    8
%define HITTABLE_BBOX_OFFS      16
%define HITTABLE_REFP_OFFS      24
%define HITTABLE_SIZE           (HITTABLE_REFP_OFFS+VEC3_SIZE)
; }}}

; Record {{{
%define RECORD_P_OFFS       0
%define RECORD_NORMAL_OFFS  VEC3_SIZE
%define RECORD_SM_OFFS      (RECORD_NORMAL_OFFS+VEC3_SIZE)
%define RECORD_T_OFFS       (RECORD_SM_OFFS+8)
%define RECORD_FFACE_OFFS   (RECORD_T_OFFS+REAL_SIZE)
%define RECORD_SIZE         48 ; size is aligned to largest property
; }}}

; Box {{{
%define BOX_HITTABLE_OFFS   0
%define BOX_CBACK_OFFS      HITTABLE_SIZE
%define BOX_CFRONT_OFFS     (BOX_CBACK_OFFS+VEC3_SIZE)
%define BOX_FACES_OFFS      (BOX_CFRONT_OFFS+VEC3_SIZE)
%define BOX_SM_OFFS         (BOX_FACES_OFFS+8)
; }}}

; List {{{
%define LIST_HITTABLE_OFFS   0
%define LIST_LIST_OFFS       HITTABLE_SIZE
%define LIST_SIZE_OFFS       (LIST_LIST_OFFS+8)
%define LIST_REFPSUM_OFFS    (LIST_SIZE_OFFS+8)
%define LIST_SM_OFFS         (LIST_REFPSUM_OFFS+VEC3_SIZE)
%define LIST_BBOX_OFFS       (LIST_SM_OFFS+8)
%define LIST_SIZE            (LIST_BBOX_OFFS+8)
; }}}

; Plane {{{
%define PLANE_HITTABLE_OFFS 0
%define PLANE_ORIGIN_OFFS HITTABLE_SIZE
%define PLANE_NORMAL_OFFS (PLANE_ORIGIN_OFFS+VEC3_SIZE)
%define PLANE_SM_OFFS (PLANE_NORMAL_OFFS+VEC3_SIZE)
%define PLANE_SIZE (PLANE_SM_OFFS+8)
;}}}
;}}}

; MACROS {{{
%macro v3p_dot 2 ; v1, v2 -> v1 . v2
	vdpps %1, %2, 0xF1
%endmacro

%macro v3p_scale 3 ; v1, t, v2 -> v1=t*v2
	vshufps %2, %2, 0b00000000
	vmulps %1, %2, %3
%endmacro
;}}}

section .data

;DATA {{{
; 32-bit absolute value mask 
fabs_mask: dd 0x7FFFFFFF
ones_mask: dd 0xFFFFFFFF
eps:  dd 1.0e-4
zero: dd 0.0e0
;}}}

section .text

; GLOBAL Functions {{{
; Vec3 Functions {{{
vec3_add: ; vec3 v1, vec3 v2 {{{
	push rbp
	mov rbp, rsp

	addps xmm0, xmm2
	addps xmm1, xmm3

	pop rbp
	ret
;}}}

vec3_prod: ; vec3 v1, vec3 v2 {{{
	push rbp
	mov rbp, rsp

	mulps xmm0, xmm2
	mulps xmm1, xmm3

	pop rbp
	ret
;}}}

vec3_unscale: ; vec3 v1, real s {{{
	push rbp
	mov rbp, rsp

	; v1 = x y 0 0 
	; v2 = z _ 0 0 
	vshufps xmm2, xmm2, 0b00000000
	divps xmm0, xmm2 ; v1 = x/s y/s 0 0
	divps xmm1, xmm2 ; v2 = z/s  0  0 0

	pop rbp
	ret
;}}}

vec3_sub: ; vec3 v1, vec3 v2 {{{
	push rbp
	mov rbp, rsp

	subps xmm0, xmm2
	subps xmm1, xmm3

	pop rbp
	ret
;}}}

vec3_norm2: ; vec3 v = xmm0 | xmm1  {{{
	push rbp
	mov rbp, rsp

	vpslldq xmm1, 0x8 ;xmm1 = 0 0 z 0
	vorps xmm0, xmm1
	v3p_dot xmm0, xmm0 ; F -> use all packed floats, 1 -> store result in least significant dword

	pop rbp
	ret
;}}}
;}}}

; Hittable Functions {{{
box_hit: ; Hittable *_self, Ray *ray, real t_min, real t_max, Record *rec {{{
	push rbp
	mov rbp, rsp
	push rbx
	push r12
	push r13
	push r14
	sub rsp, RECORD_SIZE+0x40

	xor rax, rax
	; first RECORD_SIZE bytes are for a tmp Record struct
	; we can vmovaps since RECORD_SIZE is aligned to 16 bytes
	vmovaps [rbp-(RECORD_SIZE+0x10)], xmm0 ; t_min
	vmovaps [rbp-(RECORD_SIZE+0x20)], xmm1 ; t_max
	mov [rbp-(RECORD_SIZE+0x28)], rdx ; rec
	mov [rbp-(RECORD_SIZE+0x30)], rdi ; self
	mov [rbp-(RECORD_SIZE+0x31)], al ; hit_anything = false

	xor ebx, ebx ; i = 0
	mov r12, [rdi+BOX_FACES_OFFS] ; self->faces
	mov r13d, [r12+LIST_SIZE_OFFS] ; self->faces->size
	mov r14, rsi ; ray

	.box_face_loop:
		; Hittable *h = list_get(self->faces, i)
		mov rdi, r12
		mov esi, ebx
		call list_get

		; h->hit(h, ray, t_min, t_max, tmp)
		mov rdi, rax
		mov rsi, r14
		vmovaps xmm0, [rbp-(RECORD_SIZE+0x10)]
		vmovaps xmm1, [rbp-(RECORD_SIZE+0x20)]
		lea rdx, [rbp-RECORD_SIZE]
		call [rax]
		test al, al
		jz .box_face_loop_continue ; no hit

		; box_is_inside(self, tmp.p)
		mov rdi, [rbp-(RECORD_SIZE+0x30)]
		vmovups xmm0, [rbp-(RECORD_SIZE-RECORD_P_OFFS)]
		call box_is_inside
		test al, al
		jz .box_face_loop_continue ; not inside

		mov byte [rbp-(RECORD_SIZE+0x31)], 1 ; hit_anything = true
		vmovss xmm0, [rbp-(RECORD_SIZE-RECORD_T_OFFS)] ; xmm0 = tmp->t
		vmovss [rbp-(RECORD_SIZE+0x20)], xmm0 ; t_max = xmm0

		; *rec = tmp
		vmovups xmm0, [rbp-(RECORD_SIZE-RECORD_P_OFFS)]
		vmovups xmm1, [rbp-(RECORD_SIZE-RECORD_NORMAL_OFFS)]
		mov r8, [rbp-(RECORD_SIZE-RECORD_SM_OFFS)]
		mov r9, [rbp-(RECORD_SIZE-RECORD_T_OFFS)]

		mov rdx, [rbp-(RECORD_SIZE+0x28)] ; rec
		vmovups [rdx+RECORD_P_OFFS], xmm0
		vmovups [rdx+RECORD_NORMAL_OFFS], xmm1
		mov [rdx+RECORD_SM_OFFS], r8
		mov [rdx+RECORD_T_OFFS], r9

	.box_face_loop_continue:
	inc ebx ; i++
	cmp ebx, r13d ; i < self->size
	jl .box_face_loop

	mov al, [rbp-(RECORD_SIZE+0x31)]

	add rsp, RECORD_SIZE+0x40
	pop r14
	pop r13
	pop r12
	pop rbx
	pop rbp
	ret
;}}}

plane_hit: ; Hittable *_self, Ray *ray, real t_min, real t_max, Record *hr {{{
	push rbp
	mov rbp, rsp

	xor rax, rax ; false

	; check if ray direction is perpendicular to plane normal
	vmovups xmm2, [rsi+RAY_DIR_OFFS] ; ray->direction
	vmovups xmm3, [rdi+PLANE_NORMAL_OFFS] ; self->normal
	vdpps xmm4, xmm2, xmm3, 0xF1
	vandps xmm4, [fabs_mask]
	vcomiss xmm4, [eps] ; xmm2 < EPS
	jbe .plane_hit_return ; if it is, return false (no plane hit)

	vmovups xmm4, [rdi+PLANE_ORIGIN_OFFS] ; self->origin
	vmovups xmm5, [rsi] ; r->origin
	vsubps xmm4, xmm5
	v3p_dot xmm4, xmm3
	v3p_dot xmm2, xmm3
	vdivss xmm4, xmm2 ; xmm4 = t
	vcomiss xmm4, xmm0
	jb .plane_hit_return  ; t < t_min
	vcomiss xmm1, xmm4
	jb .plane_hit_return  ; t_max < t

	vmovss [rdx+RECORD_T_OFFS], xmm4 ; record->t = t
	mov r8, [rdi+BOX_SM_OFFS]
	mov [rdx+RECORD_SM_OFFS], r8 ; record->sm = box->xm

	mov rdi, rsi
	vmovaps xmm0, xmm4
	call ray_at
	vmovups [rdx+RECORD_P_OFFS], xmm0 ; record->p = ray_at(ray, t)

	mov rdi, rdx
	vmovaps xmm0, xmm3
	call record_set_face_normal ; rec, normal

	inc rax ; return true

	.plane_hit_return:
	pop rbp
	ret
;}}}
;}}}
;}}}

; LOCAL Functions {{{
ray_at: ; Ray *r, real t {{{
	sub rsp, 0x18
	vmovaps [rsp], xmm1 ; save xmm1

	vmovups xmm1, [rdi+RAY_DIR_OFFS] ;xmm1 = r->direction
	v3p_scale xmm0, xmm0, xmm1 ; xmm0 = t * r->direction
	vaddps xmm0, [rdi]

	vmovaps xmm1, [rsp] ; restore xmm1
	add rsp, 0x18
	ret
;}}}

record_set_face_normal: ; Record *rec, Ray *r, Vec3 normal {{{
	push rax
	sub rsp, 0x10
	vmovaps [rsp], xmm1

	mov al, 1 ; front_face = true
	vmovups xmm1, [rsi+RAY_DIR_OFFS]
	v3p_dot xmm1, xmm0
	vcomiss xmm1, [zero]
	jb .record_front_face
	xor al, al ; front_face = false
	vxorps xmm1, xmm1
	vsubps xmm0, xmm1, xmm0 ; xmm0 = -xmm0

	.record_front_face:
	mov [rdi+RECORD_FFACE_OFFS], al
	vmovups [rdi+RECORD_NORMAL_OFFS], xmm0

	vmovaps xmm1, [rsp]
	add rsp, 0x10
	pop rax
	ret
;}}}

box_is_inside: ; Box *self, Vec3 p {{{
	sub rsp, 0x38
	vmovaps [rsp], xmm1
	vmovaps [rsp+0x10], xmm2
	vmovaps [rsp+0x20], xmm3
	mov eax, 1 ; true

	vmovups xmm1, [rdi+BOX_CBACK_OFFS] ; xmm0 = cb.x cb.y cb.z 0
	vmovss xmm2, [eps]
	vshufps xmm2, xmm2, 0b01000000     ; xmm1 = eps eps eps 0
	vsubps xmm1, xmm2                  ; xmm0 = (cb.x - eps) (cb.y - eps) (cb.y - eps) 0
	cmpleps xmm1, xmm0 ; self->cback <= p (coord by coord)

	vmovups xmm3, [rdi+BOX_CFRONT_OFFS] ; xmm3 = cf.x cf.y cf.z 0
	vaddps xmm3, xmm2                   ; xmm3 = (cf.x + eps) (cf.y + eps) (cf.z + eps) 0
	cmpnltps xmm3, xmm0  ; self->cfront >= p (coord by coord)

	vandps xmm1, xmm3
	vmovss xmm2, [ones_mask]
	vshufps xmm2, xmm2, 0b00000000
	vptest xmm1, xmm2 ; if xmm1 ANDN xmm2 === 0...0, then xmm2 is all ones and CF=1
	jc .box_is_inside_return ; if CF=1, function returns true

	xor eax, eax ; false

	.box_is_inside_return:
	vmovaps xmm3, [rsp+0x20]
	vmovaps xmm2, [rsp+0x10]
	vmovaps xmm1, [rsp]
	add rsp, 0x38
	ret
;}}}
;}}}

; vi: fdm=marker
