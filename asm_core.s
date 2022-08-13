; GLOBALS & EXTERNS {{{
global vec3_add
global vec3_prod
global vec3_unscale
global vec3_sub
global vec3_norm2

global box_hit
global plane_hit
global sphere_hit
global triangle_hit

global lambertian_scatter
global metal_scatter
global dielectric_scatter

global camera_init
global camera_get_ray

extern list_get
extern tanf
; }}}

; OFFSETS {{{
%define REAL_SIZE 4 ; float
%define VEC3_SIZE (REAL_SIZE * 4)

%define RAY_ORIG_OFFS   0
%define RAY_DIR_OFFS    VEC3_SIZE

; Hittable {{{
%define HITTABLE_HIT_OFFS        0
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

; List {{{
%define LIST_HITTABLE_OFFS   0
%define LIST_BBOX_OFFS       HITTABLE_SIZE
%define LIST_SM_OFFS         (LIST_BBOX_OFFS+8)
%define LIST_REFPSUM_OFFS    (LIST_SM_OFFS+8)
%define LIST_LIST_OFFS       (LIST_REFPSUM_OFFS+VEC3_SIZE)
%define LIST_SIZE_OFFS       (LIST_LIST_OFFS+8)
%define LIST_CAP_OFFS        (LIST_SIZE_OFFS+4)
%define LIST_SIZE            (LIST_CAP_OFFS+4)
; }}}

; Box {{{
%define BOX_HITTABLE_OFFS   0
%define BOX_SM_OFFS         HITTABLE_SIZE
%define BOX_CBACK_OFFS      BOX_SM_OFFS+8
%define BOX_CFRONT_OFFS     (BOX_CBACK_OFFS+VEC3_SIZE)
%define BOX_FACES_OFFS      (BOX_CFRONT_OFFS+VEC3_SIZE)
; }}}

; Plane {{{
%define PLANE_HITTABLE_OFFS 0
%define PLANE_SM_OFFS       HITTABLE_SIZE
%define PLANE_ORIGIN_OFFS   (PLANE_SM_OFFS+8)
%define PLANE_NORMAL_OFFS   (PLANE_ORIGIN_OFFS+VEC3_SIZE)
%define PLANE_SIZE          (PLANE_NORMAL_OFFS+VEC3_SIZE)
;}}}

; Sphere {{{
%define SPHERE_HITTABLE_OFFS   0
%define SPHERE_BBOX_OFFS       HITTABLE_SIZE
%define SPHERE_SM_OFFS         SPHERE_BBOX_OFFS+8
%define SPHERE_CENTER_OFFS     (SPHERE_SM_OFFS+8)
%define SPHERE_RADIUS_OFFS     (SPHERE_CENTER_OFFS+VEC3_SIZE)
%define SPHERE_SIZE            (SPHERE_RADIUS_OFFS+REAL_SIZE)
;}}}

; Triangle {{{
%define TRIANGLE_HITTABLE_OFFS   0
%define TRIANGLE_BBOX_OFFS       HITTABLE_SIZE
%define TRIANGLE_SM_OFFS         TRIANGLE_BBOX_OFFS+8
%define TRIANGLE_P1_OFFS         (TRIANGLE_SM_OFFS+8)
%define TRIANGLE_P2_OFFS         (TRIANGLE_P1_OFFS+VEC3_SIZE)
%define TRIANGLE_P3_OFFS         (TRIANGLE_P2_OFFS+VEC3_SIZE)
%define TRIANGLE_SIZE            (TRIANGLE_P3_OFFS+VEC3_SIZE)
;}}}

; Materials {{{
%define MATERIAL_SCATTER_OFFS 0
%define MATERIAL_EMITTED_OFFS 8
%define MATERIAL_SIZE         (2*8)

; All materials share the same structure
%define MATALL_MATERIAL_OFFS 0
%define MATALL_ALBEDO_OFFS   MATERIAL_SIZE
%define MATALL_ALPHA_OFFS    (MATALL_ALBEDO_OFFS+VEC3_SIZE)
;}}}

; Camera {{{
%define CAMERA_ORIGIN_OFFS 0
%define CAMERA_BLCORN_OFFS (CAMERA_ORIGIN_OFFS+VEC3_SIZE)
%define CAMERA_HORIZ_OFFS  (CAMERA_BLCORN_OFFS+VEC3_SIZE)
%define CAMERA_VERTI_OFFS  (CAMERA_HORIZ_OFFS+VEC3_SIZE)
%define CAMERA_U_OFFS      (CAMERA_VERTI_OFFS+VEC3_SIZE)
%define CAMERA_V_OFFS      (CAMERA_U_OFFS+VEC3_SIZE)
%define CAMERA_W_OFFS      (CAMERA_V_OFFS+VEC3_SIZE)
%define CAMERA_LR_OFFS     (CAMERA_W_OFFS+VEC3_SIZE)
%define CAMERA_SIZE        (CAMERA_LR_OFFS+REAL_SIZE)
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

%macro v3p_cross 3 ; v1 = v2 x v3 ; v2 = (x2,y2,z2,_), v3 = (x3,y3,z3,_)
	vshufps xmm6, %2, %2, 0b11001001                        ; (y2,z2,x2)
	vshufps xmm7, %3, %3, 0b11010010                        ; (z3,x3,y3)
	vmulps %1, xmm6, xmm7 ; v1 = (y2*z3, z2*x3, x2*y3)
	vshufps xmm6, %2, %2, 0b11010010                        ; (z2,x2,y2)
	vshufps xmm7, %3, %3, 0b11001001                        ; (y3,z3,x3)
	vmulps xmm6, xmm7    ; (z2*y3, x2*z3, y2*x3)
	vsubps %1, xmm6   ; v1 = (y2*z3-z2*y3, z2*x3-x2*z3, (x2*y3-y2*x3))
%endmacro

%macro v3p_reflect 4
	vdpps %1, %2, %3, 0xF1
	vmovss %4, [two]
	vmulss %1, %4
	vshufps %1, %1, 0b00000000
	vmulps %1, %3
	vsubps %1, %2, %1
%endmacro

%macro v3p_normalized  2
	vdpps %2, %1, %1, 0xF1 ; norm2
	vrsqrtss %2, %2        ; 1/sqrt(norm2)
	vshufps %2, %2, 0b00000000 ;normalize
	vmulps %1, %2
%endmacro
;}}}

section .data

;DATA {{{
; 32-bit absolute value mask 
fabs_mask: dd 0x7FFFFFFF
rand_mask:
fnabs_mask: dd 0x80000000
ones_mask:  dd 0xFFFFFFFF
eps:  dd 1.0e-4
zero: dd 0.0e0
half: dd 0.5e0
one:  dd 1.0e0
two:  dd 2.0e0
degtorad: dd 0.017453292519943295
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

; Random vec3 in unit sphere
vec3_rnd_unit: ;{{{
	sub rsp, 0x18
	vmovaps [rsp], xmm1

	.vec3_rnd_loop:
		call randvec
		vdpps xmm1, xmm0, xmm0, 0xF1
		vcomiss xmm1, [one]
		jae .vec3_rnd_loop

	vmovaps xmm1, [rsp]
	add rsp, 0x18
	ret
;}}}
;}}}

; Random {{{
; inspired by: https://xoofx.com/blog/2009/10/25/random-float-number-using-x86-asm-code/#v3
randf: ; store a random float in (-1;1] range in xmm0 {{{
	push rax
	push rcx
	sub rsp, 0x18
	vmovaps [rsp], xmm1

	mov ecx, 10 ; retries
	.randf_retry:
		rdseed eax
		jc .randf_done
		loop .randf_retry

	.randf_done:
	imul eax, 16007
	vxorps xmm0, xmm0
	vcvtsi2ss xmm0, eax
	vcvtsi2ss xmm1, dword [rand_mask]
	vdivss xmm0, xmm1 ; scale float down to (-1;1]

	vmovaps xmm1, [rsp]
	add rsp, 0x18
	pop rcx
	pop rax
	ret
;}}}

randvec: ; store 3 random floats in range (-1;1] in xmm0 {{{
	sub rsp, 0x18
	vmovaps [rsp], xmm1

	call randf ; random x
	vmovaps xmm1, xmm0

	call randf ; random y
	vpslldq xmm0, 4
	vorps xmm1, xmm0

	call randf ; random z
	vpslldq xmm0, 8
	vorps xmm0, xmm1

	vmovaps xmm1, [rsp]
	add rsp, 0x18
	ret
;}}}

randf01: ;{{{ store a random float in [0;1) range in xmm0
	sub rsp, 0x18
	vmovups [rsp], xmm1

	call randf
	vxorps xmm1, xmm1
	vsubss xmm1, [one] ; -1
	vaddss xmm0, xmm1 ; move range to (-2;0]
	vaddss xmm1, xmm2 ; -2
	vdivss xmm0, xmm1 ; move range to [0;1)

	vmovups xmm1, [rsp]
	add rsp, 0x18
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
		call [rdi+BOX_HITTABLE_OFFS+HITTABLE_HIT_OFFS]
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
	vmovups xmm5, [rsi+RAY_ORIG_OFFS] ; r->origin
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

sphere_hit: ; Hittable *_self, Ray *ray, real t_min, real t_max, Record *hr {{{
	push rbp
	mov rbp, rsp

	xor rax, rax

	vmovups xmm2, [rsi+RAY_ORIG_OFFS]      ; r->origin
	vmovups xmm3, [rdi+SPHERE_CENTER_OFFS] ; center
	vsubps  xmm2, xmm3               ; oc = center to ray origin
	vmovups xmm4, [rsi+RAY_DIR_OFFS] ; r->direction
	vdpps xmm5, xmm4, xmm4, 0xF1     ; a = norm2(r->direction)
	vdpps xmm6, xmm2, xmm4, 0xF1     ; hb = half b
	vxorps xmm9, xmm9
	vsubss xmm6, xmm9, xmm6          ; -hb
	vdpps xmm2, xmm2, 0xF1           ; norm2(oc)
	vmovss xmm8, [rdi+SPHERE_RADIUS_OFFS] ; radius
	vmulss xmm7, xmm8, xmm8          ; radius^2
	vsubss xmm2, xmm7                ; c = norm2(oc) - radius^2
	vmulss xmm2, xmm5                ; c * a
	vmulss xmm7, xmm6, xmm6          ; hb^2 = (-hb) * (-hb)
	vsubss xmm2, xmm7, xmm2 ; discriminant

	vcomiss xmm2, [zero]
	jb .sphere_hit_return

	vsqrtss xmm2, xmm2 ; sqrt(discriminant)
	vsubss xmm7, xmm6, xmm2
	vdivss xmm7, xmm5  ; root = (-hb - sqrtd) / a
	vcomiss xmm7, xmm0
	jb .sphere_hit_check_other_root ; root < t_min
	vcomiss xmm1, xmm7
	jae .sphere_hit_did_hit ; t_max >= root

	.sphere_hit_check_other_root:
	vaddss xmm7, xmm6, xmm2
	vdivss xmm7, xmm5  ; root = (-hb + sqrtd) / a
	vcomiss xmm7, xmm0 ; root < t_min
	jb .sphere_hit_return
	vcomiss xmm1, xmm7 ; t_max < root
	jb .sphere_hit_return

	.sphere_hit_did_hit:
	inc rax
	vmovss [rdx+RECORD_T_OFFS], xmm7
	mov r8, [rdi+SPHERE_SM_OFFS]
	mov [rdx+RECORD_SM_OFFS], r8

	; ray_at(r, rec->t)
	vmovaps xmm0, xmm7
	mov rdi, rsi
	call ray_at
	vmovups [rdx+RECORD_P_OFFS], xmm0

	vshufps xmm8, xmm8, 0b00000000
	vsubps xmm0, xmm3
	vdivps xmm0, xmm8 ; outward_normal
	mov rdi, rdx
	call record_set_face_normal

	.sphere_hit_return:
	pop rbp
	ret

;}}}

triangle_hit: ; {{{
	push rbp
	mov rbp, rsp

	call vec3_rnd_unit

	xor rax, rax

	vmovups xmm2, [rdi+TRIANGLE_P1_OFFS]
	vmovups xmm3, [rdi+TRIANGLE_P2_OFFS]
	vmovups xmm4, [rdi+TRIANGLE_P3_OFFS]
	vsubps xmm8, xmm3, xmm2
	vsubps xmm9, xmm4, xmm2
	v3p_cross xmm5, xmm8, xmm9 ; xmm5 = normal

	; perpendicular(normal, r->direction)
	vmovups xmm6, [rsi+RAY_DIR_OFFS] ; r->direction
	vdpps xmm7, xmm5, xmm6, 0xF1
	vandps xmm7, [fabs_mask]
	vcomiss xmm7, [eps] ; xmm7 < EPS
	jb .triangle_hit_return

	; check if ray intersection is outside of t_range
	vsubps xmm8, xmm2, [rsi+RAY_ORIG_OFFS]
	vdpps xmm8, xmm5, 0xF1 ; t_nom
	vdpps xmm6, xmm5, 0xF1 ; t_denom
	vdivss xmm8, xmm6 ; t
	vcomiss xmm8, xmm0
	jb .triangle_hit_return
	vcomiss xmm1, xmm8
	jb .triangle_hit_return

	mov rcx, [rdi+TRIANGLE_SM_OFFS] ; save sm, since we'll overwrite rdi

	mov rdi, rsi
	vmovaps xmm0, xmm8
	call ray_at

	; first boundary check
	vsubps xmm9, xmm3, xmm2 ; (p2 - p1)
	vsubps xmm10, xmm0, xmm2 ; (p - p1)
	v3p_cross xmm1, xmm9, xmm10
	vdpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
	vcomiss xmm1, [zero]
	jbe .triangle_hit_return ; dot(...) <= 0

	;second boundary check
	vsubps xmm9, xmm4, xmm3 ; (p3 - p2)
	vsubps xmm10, xmm0, xmm3 ; (p - p2)
	v3p_cross xmm1, xmm9, xmm10
	vdpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
	vcomiss xmm1, [zero]
	jbe .triangle_hit_return ; dot(...) <= 0

	;third boundary check
	vsubps xmm9, xmm2, xmm4 ; (p1 - p3)
	vsubps xmm10, xmm0, xmm4 ; (p - p3)
	v3p_cross xmm1, xmm9, xmm10
	vdpps xmm1, xmm5, 0xF1 ; dot(normal, cross(...))
	vcomiss xmm1, [zero]
	jbe .triangle_hit_return ; dot(...) <= 0

	; triangle hit
	inc rax
	vmovss [rdx+RECORD_T_OFFS], xmm8
	vmovups [rdx+RECORD_P_OFFS], xmm0
	mov [rdx+RECORD_SM_OFFS], rcx
	mov rdi, rdx
	vmovaps xmm0, xmm5
	call record_set_face_normal

	.triangle_hit_return:
	pop rbp
	ret
;}}}
;}}}

; Scatter Functions {{{
lambertian_scatter: ; {{{
	push rbp
	mov rbp, rsp

	mov rax, 1 ; return value

	call vec3_rnd_unit
	vmovups xmm1, [rdx+RECORD_NORMAL_OFFS] ; rec->normal
	vaddps xmm3, xmm0, xmm1 ; scatter_dir
	vmovaps xmm4, xmm3      ; save scatter_dir

	; check if scatter_dir is near zero
	vmovss xmm2, [fabs_mask]
	vshufps xmm2, xmm2, 0b00000000
	vorps xmm4, xmm2 ; fabs(scatter_dir)

	vmovss xmm2, [eps]
	vshufps xmm2, xmm2, 0b00000000
	cmpltps xmm4, xmm2 ; xmm4 < (eps, eps, eps, eps)?

	vmovss xmm2, [ones_mask]
	vshufps xmm2, xmm2, 0b00000000
	vptest xmm4, xmm2 ; if xmm4 ANDN xmm2 === 0...0, then xmm2 is all ones and CF=1
	jnc .lambertian_scatter_not_near_zero ; if CF=0, scatter is not near zero

	vmovaps xmm3, xmm1 ; scatter_dir = rec->normal

	.lambertian_scatter_not_near_zero:
	; *scattered = Ray(p, scatter_dir)
	vmovups xmm5, [rdx+RECORD_P_OFFS]
	vmovups [r8+RAY_ORIG_OFFS], xmm5
	vmovups [r8+RAY_DIR_OFFS], xmm3

	; *attenuation = self->albedo
	vmovups xmm5, [rdi+MATALL_ALBEDO_OFFS]
	vmovups [rcx], xmm5

	pop rbp
	ret
;}}}

metal_scatter: ; {{{
	push rbp
	mov rbp, rsp

	xor rax, rax

	vmovups xmm0, [rsi+RAY_DIR_OFFS]
	v3p_normalized xmm0, xmm2
	vmovups xmm1, [rdx+RECORD_NORMAL_OFFS] ; rec->normal
	v3p_reflect xmm4, xmm1, xmm2, xmm3     ; reflected

	; scattered->orig = rec->p
	vmovups xmm2, [rdx+RECORD_P_OFFS]
	vmovups [r8+RAY_ORIG_OFFS], xmm2

	; scattered->dir = reflected + self->fuzz * vec3_rnd_unit()
	call vec3_rnd_unit
	vmovss xmm3, [rdi+MATALL_ALPHA_OFFS]
	vshufps xmm3, xmm3, 0b00000000
	vmulps xmm0, xmm3
	vaddps xmm0, xmm4
	vmovups [r8+RAY_DIR_OFFS], xmm0

	; *attenuation = self->albedo
	vmovups xmm2, [rdi+MATALL_ALBEDO_OFFS]
	vmovups [rcx], xmm2

	vdpps xmm1, xmm4, 0xF1
	vcomiss xmm1, [zero]

	jbe .metal_scatter_return ; dot(...) <= 0
	inc rax
	
	.metal_scatter_return:
	pop rbp
	ret
;}}}

dielectric_scatter: ; {{{
	push rbp
	mov rbp, rsp

	vmovups xmm1, [rdi+MATALL_ALPHA_OFFS] ; ref_ratio = self->ir
	mov ax, [rdx+RECORD_FFACE_OFFS]
	test ax, ax
	jz .dielectric_scatter_not_fface

	vrcpss xmm1, xmm1

	.dielectric_scatter_not_fface:
	vmovups xmm2, [rsi+RAY_DIR_OFFS]
	v3p_normalized xmm2, xmm3 ; unit_dir

	vxorps xmm3, xmm3
	vsubps xmm5, xmm3, xmm2 ; -unit_dir
	vmovups xmm9, [rdx+RECORD_NORMAL_OFFS] ; rec->normal
	vdpps xmm5, xmm9, 0xF1
	vmovss xmm3, [one]
	vminss xmm5, xmm3 ; cos_theta = min(dot(-unit_dir, rec->normal), 1)
	vmulss xmm6, xmm5, xmm5
	vsubss xmm6, xmm3, xmm6
	vsqrtss xmm6, xmm6 ; sin_theta
	vmulss xmm6, xmm1

	vcomiss xmm6, xmm3 ; ref_ratio * sin_theta > 1
	ja .dielectric_scatter_do_reflect ; cannot refract

	; Use Schlick's approximation for reflectance.
	vsubss xmm6, xmm3, xmm1
	vaddss xmm7, xmm3, xmm1
	vdivss xmm6, xmm7 ; r0
	vmulss xmm6, xmm6 ; r0 *= r0

	vsubss xmm5, xmm3, xmm5 ; (1-cos_theta)
	vmovss xmm6, xmm5
	vmulss xmm5, xmm5 ; (1-cos_theta)^2
	vmulss xmm5, xmm5 ; (1-cos_theta)^4
	vmulss xmm5, xmm6 ; (1-cos_theta)^5
	vsubss xmm7, xmm3, xmm6 ; (1-r0)
	vmulss xmm7, xmm5
	vaddss xmm6, xmm7

	call randf01
	vcomiss xmm6, xmm0
	jbe .dielectric_scatter_do_refract  ; reflectance <= rnd

	.dielectric_scatter_do_reflect:
	v3p_reflect xmm0, xmm2, xmm9, xmm5
	jmp .dielectric_scatter_return

	.dielectric_scatter_do_refract:
	vxorps xmm4, xmm4
	vsubps xmm4, xmm2
	vdpps xmm4, xmm9, 0xF1
	vminss xmm4, xmm3 ; cos_theta
	vshufps xmm4, xmm4, 0b00000000
	vmulps xmm5, xmm4, xmm9
	vaddps xmm5, xmm2
	vshufps xmm1, xmm1, 0b00000000
	vmulps xmm5, xmm1 ; r_out_perp

	vdpps xmm6, xmm5, xmm5, 0xF1
	vsubss xmm6, xmm3, xmm6
	vandps xmm6, [fabs_mask]
	vsqrtss xmm6, xmm6
	vshufps xmm6, xmm6, 0b00000000
	vmulps xmm6, xmm9
	vsubps xmm0, xmm5, xmm6 ; sub instead of add, because we use sqrt instead of -sqrt

	.dielectric_scatter_return:
	; *attenuation = self->albedo
	vmovups xmm1, [rdi+MATALL_ALBEDO_OFFS]
	vmovups [rcx], xmm1

	vmovups xmm1, [rdx+RECORD_P_OFFS]
	vmovups [rsi+RAY_ORIG_OFFS], xmm1
	vmovups [rsi+RAY_DIR_OFFS], xmm0

	xor rax, rax
	inc rax

	pop rbp
	ret
;}}}
;}}}

; Camera Functions {{{
camera_init: ;{{{
	push rbp
	mov rbp, rsp
	sub rsp, 0x60

	vpslldq xmm1, 0x8
	vorps xmm0, xmm1 ; from
	vmovups [rdi+CAMERA_ORIGIN_OFFS], xmm0 ; c->origin = from

	vpslldq xmm3, 0x8
	vorps xmm1, xmm2, xmm3 ; to
	vsubps xmm1, xmm0, xmm1 ; from-to

	vpslldq xmm5, 0x8
	vorps xmm2, xmm4, xmm5 ; vup

	vmovss xmm3, [rbp+0x10] ; aperture
	vmulss xmm3, [half]
	vmovss [rdi+CAMERA_LR_OFFS], xmm3 ; c->lens_radius = aperture/2.0

	vmovss xmm3, [rbp+0x18] ; focus_dist

	vmovaps [rsp], xmm0      ; from
	vmovaps [rsp+0x10], xmm1 ; from-to
	vmovaps [rsp+0x20], xmm2 ; vup
	vmovaps [rsp+0x30], xmm6 ; vfov
	vmovaps [rsp+0x40], xmm7 ; aspect_ratio
	vmovaps [rsp+0x50], xmm3 ; focus_dist
	push rdi
	sub rsp, 8

	vmulss xmm6, [degtorad] ; theta
	vmulss xmm0, xmm6, [half]
	call tanf

	add rsp, 8
	pop rdi
	vmovaps xmm1, [rsp+0x10] ; from-to
	vmovaps xmm2, [rsp+0x20] ; vup
	vmovaps xmm6, [rsp+0x30] ; vfov
	vmovaps xmm7, [rsp+0x40] ; aspect_ratio
	vmovaps xmm3, [rsp+0x50] ; focus_dist

	vmulss xmm4, xmm0, [two] ; vp_height
	vmulss xmm5, xmm7, xmm4 ; vp_width

	v3p_normalized xmm1, xmm8
	vmovups [rdi+CAMERA_W_OFFS], xmm1

	v3p_cross xmm8, xmm2, xmm1
	v3p_normalized xmm8, xmm2
	vmovups [rdi+CAMERA_U_OFFS], xmm8

	v3p_cross xmm9, xmm1, xmm8
	vmovups [rdi+CAMERA_V_OFFS], xmm9

	vmulss xmm5, xmm3
	vshufps xmm5, xmm5, 0b00000000
	vmulps xmm5, xmm8
	vmovups [rdi+CAMERA_HORIZ_OFFS], xmm5

	vmulss xmm4, xmm3
	vshufps xmm4, xmm4, 0b00000000
	vmulps xmm4, xmm9
	vmovups [rdi+CAMERA_VERTI_OFFS], xmm4

	vmovaps xmm0, [rsp]
	vaddps xmm4, xmm5
	vmovss xmm5, [half]
	vshufps xmm5, xmm5, 0b00000000
	vmulps xmm4, xmm5
	vshufps xmm3, xmm3, 0b00000000
	vmulps xmm1, xmm3
	vaddps xmm1, xmm4
	vsubps xmm0, xmm1
	vmovups [rdi+CAMERA_BLCORN_OFFS], xmm0

	add rsp, 0x60
	pop rbp
	ret
;}}}

camera_get_ray: ;{{{
	push rbp
	mov rbp, rsp

	vmovss xmm2, xmm0
	call vec3_rnd_unit ; xmm0=rand_unit
	vmovss xmm3, [rsi+CAMERA_LR_OFFS]
	vshufps xmm3, xmm3, 0b00000000
	vmulps xmm0, xmm3 ; rd

	vshufps xmm3, xmm0, xmm0, 0b00000000
	vmulps xmm3, [rsi+CAMERA_U_OFFS]

	vshufps xmm4, xmm0, xmm0, 0b01010101
	vmulps xmm4, [rsi+CAMERA_V_OFFS]
	vaddps xmm3, xmm4 ; offset

	vaddps xmm3, [rsi+CAMERA_ORIGIN_OFFS]
	vmovups [rdi+RAY_ORIG_OFFS], xmm3

	vshufps xmm2, xmm2, 0b00000000
	vmulps xmm2, [rsi+CAMERA_HORIZ_OFFS]
	vaddps xmm2, [rsi+CAMERA_BLCORN_OFFS]
	vshufps xmm1, xmm1, 0b00000000
	vmulps xmm1, [rsi+CAMERA_VERTI_OFFS]
	vsubps xmm1, xmm3
	vaddps xmm1, xmm2
	vmovups [rdi+RAY_DIR_OFFS], xmm1

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
	vaddps xmm0, [rdi+RAY_ORIG_OFFS]

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
