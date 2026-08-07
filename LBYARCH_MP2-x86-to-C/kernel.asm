; kernel.asm
; x86-64 version of the dot product kernel
; sdot = A[0]*B[0] + A[1]*B[1] + ... + A[n-1]*B[n-1]
;
; rcx = pointer to A
; rdx = pointer to B
; r8  = n
; return value (sdot) goes back in xmm0
;
; using movsd/mulsd/addsd here - these are scalar SSE2 double precision
.code

dot_product_asm PROC
    xorpd xmm0, xmm0 ; sdot = 0
    xor rax, rax ; i = 0

    test r8, r8 ; if n <= 0 skip loop
    jle done

loop_start:
    movsd xmm1, qword ptr [rcx + rax*8]   ; xmm1 = A[i]
    movsd xmm2, qword ptr [rdx + rax*8]   ; xmm2 = B[i]

    mulsd xmm1, xmm2 ; xmm1 = A[i] * B[i]
    addsd xmm0, xmm1 ; sdot = sdot + xmm1

    inc rax
    cmp rax, r8
    jl loop_start

done:
    ret ; sdot is sitting in xmm0, that's the return value

dot_product_asm ENDP

END