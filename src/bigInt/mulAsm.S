.intel_syntax noprefix
.global naiveMul_Asm

.align 16
naiveMul_Asm: //mul using naive multiplication
    //*x in rdi, *y in rsi
    push rbx
    push r12
    push r13
    push r14
    push r15

    mov r12, [rdi] //r12 := x->start
    mov r13, [rdi + 8] //r13 := x->end
    mov r14, [rsi] //r14 := y->start
    mov r15, [rsi + 8] //r15 := y->end
    mov r10, [rdi + 24] // r10 := x->bigIntArray
    mov r11, [rsi + 24] // r11 := y->bigIntArray

    lea r10, [r10 + r13*8] //pointer to array ends
    lea r11, [r11 + r15*8]

    mov rdi, r13 //calculate res len
    sub rdi, r12
    add rdi, r15
    sub rdi, r14
    push r10
    push r11
    call newBigInt
    mov r8, rax // r8 = result
    pop r11
    pop r10

    //first loop
    mov rbx, [r8 + 24] // rbx := result->BigIntArray
    mov rdi, [r8 + 8] //pointer to array end
    lea rbx, [rbx + rdi*8]
    // i = rcx, j = rsi
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi

    mov rcx, r13 // i=x->end - x->start
    sub rcx, r12
    neg rcx // r12, r13 new purpose

    //n = r13
    //carry = r12
    xor r12, r12
    mov rdx, [r10 + rcx*8]
.LLoop1Start:
    mulx r9, rax, [r11 + rsi*8]
    lea r13, [rsi + rcx]
    add rax, r12
    adc r9, 0
    mov [rbx + r13*8], rax
    mov r12, r9

    add rsi, 1
    jnz .LLoop1Start
    //Loop1 end
    add [rbx + r13*8+8], r12

    //Double loop start
    add rcx, 1
    jz .LResize

.LOuterLoopStart:
    xor r12, r12
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    mov rdx, [r10 + rcx*8]
    add rsi, 4
    jge .LShortInnerLoop

.LInnerLoopStart:
    sub rsi, 4
    lea r13, [rsi+ rcx]

    mulx r9, rax, [r11 + rsi*8]
    add rax, r12
    adc r9, 0
    add rax, [rbx + r13*8]
    mov [rbx + r13*8], rax
    mov r12, r9

    mulx r9, rax, [r11 + rsi*8 + 8]
    adc rax, r12
    adc r9, 0
    add rax, [rbx + r13*8+8]
    mov [rbx + r13*8+8], rax
    mov r12, r9

    mulx r9, rax, [r11 + rsi*8+16]
    adc rax, r12
    adc r9, 0
    add rax, [rbx + r13*8+16]
    mov [rbx + r13*8+16], rax
    mov r12, r9

    mulx r9, rax, [r11 + rsi*8+24]
    adc rax, r12
    adc r9, 0
    add rax, [rbx + r13*8+24]
    adc r9, 0
    mov [rbx + r13*8+24], rax
    mov r12, r9

    add rsi, 8
    jl .LInnerLoopStart

.LShortInnerLoop:
    sub rsi, 4
.LInnerLoopShortStart:
    mulx r9, rax, [r11 + rsi*8]
    lea r13, [rsi+ rcx]
    add rax, r12
    adc r9, 0
    add rax, [rbx + r13*8]
    adc r9, 0
    mov [rbx + r13*8], rax
    mov r12, r9

    add rsi, 1
    jnz .LInnerLoopShortStart
    //InnerLoop end
    add [rbx + r13*8+8], r12

    add rcx, 1
    jnz .LOuterLoopStart

.LResize:
    //resize - res r8
    mov rcx, [r8 + 24] //res->bigIntArray
    mov rdx, [r8 + 8] //res->end
    cmp QWORD PTR [rcx + rdx*8 - 8], 0
    jnz .LMulEnd
    sub rdx, [r8]
    cmp rdx, 1
    jbe .LMulEnd
    sub QWORD PTR [r8 + 8], 1

.LMulEnd:
    mov rax, r8
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret
