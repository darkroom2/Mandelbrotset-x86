section .data
ER2: dq 4.0	; 64bit 4.0 escape radius squared

section .text ; makes this executable
bits 64 ; allow 64-bit register names
default rel
global mandelbrotASM ; makes this visible to linker
; RCX pixelArray	( rdi )
; RDX width iXmax	( rsi )
; R8 height iYmax	( rdx )
; R9
; XMM0 CxMin
; XMM1 CyMin
; XMM2 pW
; XMM3 pH
mandelbrotASM:
    push rbp
    mov rbp, rsp
	mov rdx, rax
	mov r8, rax

	movsd xmm10, [ER2]	; XMM10 = 4.0
	movsd xmm9, xmm0	; XMM9 = CxMin
start:
	mov r9, 0x000000	; black color
	movsd xmm4, xmm0	; XMM4 = Cx0
	movsd xmm5, xmm1	; XMM5 = Cy0
converge:
	movsd xmm6, xmm4	; XMM6 = Cx0
	mulsd xmm6, xmm6	; XMM6 = Cx0^2

	movsd xmm7, xmm5	; XMM7 = Cy0
	mulsd xmm7, xmm7	; XMM7 = Cy0^2

	movsd XMM8, XMM6	
	addsd XMM8, xmm7	; XMM8 = Cy0^2 + Cx0^2

	ucomisd xmm8, xmm10	; If ( XMM8 > 4.0)
	jnb colour
calccoords:
	movsd XMM8, XMM6
	subsd XMM8, xmm7
	addsd xmm8, xmm0

	movsd xmm6, xmm4
	mulsd xmm6, xmm5
	addsd xmm6, xmm6
	addsd xmm6, xmm1
	movsd xmm5, xmm6

	movsd xmm4, xmm8
cond:
	add r9, 0x010101	; depending on iterations, we make the grayscale brighter
	cmp r9, 0xffffff
	jne converge
colour:
	mov [rcx], r9
	add rcx, 3
next:
	addsd XMM0, xmm2	; XMM0 = XMM0 + pW
	dec rax
	jnz start
nextline:
	movsd XMM0, xmm9	; XMM0 = CxMin
	addsd xmm1, xmm2	; XMM1 = XMM1 + pH
	mov	 rax, rdx
	dec r8
;	cmp r8, 0x260
	jnz start
; RCX pixelArray	( rdi )
; RDX width iXmax	( rsi )
; R8 height iYmax	( rdx )

end:
    mov rsp, rbp
    pop rbp
    ret