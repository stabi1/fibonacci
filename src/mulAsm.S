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

    //store pointer
    mov r12, [rdi] //r12 := x->start
    mov r13, [rdi + 8] //r13 := x->end
    mov r14, [rsi] //r14 := y->start
    mov r15, [rsi + 8] //r15 := y->end
    mov r10, [rdi + 24] // r10 := x->bigIntArray
    mov r11, [rsi + 24] // r11 := y->bigIntArray
    mov rcx, r13
    mov rsi, r15
    lea r10, [r10 + rcx*8]
    lea r11, [r11 + rsi*8]

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

    //Loops
    mov rbx, [r8 + 24] // rbx := result->BigIntArray
    mov rax, [r8 + 8]
    lea rbx, [rbx + rax*8]
    // i = rcx, j = rsi
    mov rcx, r13 // i=x->end - x->start
    sub rcx, r12
    neg rcx
    add rcx, 2
    cmp rcx, 0
    jge .LSkipMultipleLoop
    sub r10, 16
.LLoop1:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    add rsi, 4
    jge .JmpKurzLoop
.LLoop2:
    sub rsi, 4
    //Mul
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi
    sub r9, 2

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx
    adc qword ptr [rbx + r9*8 + 16], 0

    //second
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 8]

    add [rbx + r9*8 + 8], rax
    adc [rbx + r9*8 + 16], rdx
    adc qword ptr [rbx + r9*8 + 24], 0

    //third
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 16]

    add [rbx + r9*8 + 16], rax
    adc [rbx + r9*8 + 24], rdx
    adc qword ptr [rbx + r9*8 + 32], 0

    //fourth
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 24]

    add [rbx + r9*8 + 24], rax
    adc [rbx + r9*8 + 32], rdx
    adc qword ptr [rbx + r9*8 + 40], 0

    add rsi, 8
    jl .LLoop2 //leave loop2
    sub rsi, 4
    jz .LKurzFertig
.LKurzLoop:
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi
    sub r9, 2

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx
    adc qword ptr [rbx + r9*8 + 16], 0

    add rsi, 1
    jl .LKurzLoop

.LKurzFertig:
    add rcx, 1
    jl .LLoop1 //leave loop1

    add r10, 16
.LSkipMultipleLoop:
    sub rcx, 2
.LLoop1Einzel:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
.LLoop2Einzel:

    //Mul
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx

    //carry loop
    jnc .LLoopEndEinzel
    add r9, 2 // i := r9
    jz .LLoopEndEinzel
.LLoopEinzel:
    add qword ptr [rbx + r9*8], 1
    jnc .LLoopEndEinzel
    setc al
    add r9, 1
    jz .LLoopEinzel

.LLoopEndEinzel:
    add rsi, 1
    jnz .LLoop2Einzel //leave loop2
    add rcx, 1
    jnz .LLoop1Einzel //leave loop1

    //resize result
    mov r15, r8 //result = r15
    mov rdi, r8
    call getOccupiedFields_Asm
    mov rcx, [r15]
    add rcx, rax
    mov [r15+8], rcx

    //leave the function
    mov rax, r15
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret
.JmpKurzLoop:
    sub rsi, 4
    jmp .LKurzLoop


