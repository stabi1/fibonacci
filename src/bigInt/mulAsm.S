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
    push rbp

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
    add rdi, 1
    push r10
    push r11
    call newBigInt
    mov r8, rax // r8 = result
    pop r11
    pop r10

    //Loops
    mov rbx, [r8 + 24] // rbx := result->BigIntArray
    sub rbx, 8 //because of the len res = lenx + leny + 1
    mov rax, [r8 + 8]
    lea rbx, [rbx + rax*8]
    // i = rcx, j = rsi
    mov rcx, r13 // i=x->end - x->start
    sub rcx, r12
    neg rcx // r12, r13 new purpose
.LLoop1:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    add rsi, 12
    jge .LShortLoopStart
.LLoop2:
    sub rsi, 12

    mov r9, rcx // r9 = n i + j
    add r9, rsi
    xor rbp, rbp

    //Mul
    //1
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8]

    adcx rax, [rbx + r9*8]
    mov [rbx + r9*8], rax
    adcx rdi, [rbx + r9*8 + 8]
    mov [rbx + r9*8 + 8], rdi

    //2
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 8]

    adox r13, [rbx + r9*8 + 8]
    mov [rbx + r9*8 + 8], r13
    adox r12, [rbx + r9*8 + 16]
    mov [rbx + r9*8 + 16], r12

    //3
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8 + 16]

    adcx rax, [rbx + r9*8 + 16]
    mov [rbx + r9*8 + 16], rax
    adcx rdi, [rbx + r9*8 + 24]
    mov [rbx + r9*8 + 24], rdi

    //4
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 24]

    adox r13, [rbx + r9*8 + 24]
    mov [rbx + r9*8 + 24], r13
    adox r12, [rbx + r9*8 + 32]
    mov [rbx + r9*8 + 32], r12

    //5
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8 + 32]

    adcx rax, [rbx + r9*8 + 32]
    mov [rbx + r9*8 + 32], rax
    adcx rdi, [rbx + r9*8 + 40]
    mov [rbx + r9*8 + 40], rdi

    //6
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 40]

    adox r13, [rbx + r9*8 + 40]
    mov [rbx + r9*8 + 40], r13
    adox r12, [rbx + r9*8 + 48]
    mov [rbx + r9*8 + 48], r12

    //7
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8 + 48]

    adcx rax, [rbx + r9*8 + 48]
    mov [rbx + r9*8 + 48], rax
    adcx rdi, [rbx + r9*8 + 56]
    mov [rbx + r9*8 + 56], rdi

    //8
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 56]

    adox r13, [rbx + r9*8 + 56]
    mov [rbx + r9*8 + 56], r13
    adox r12, [rbx + r9*8 + 64]
    mov [rbx + r9*8 + 64], r12

    //9
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8 + 64]

    adcx rax, [rbx + r9*8 + 64]
    mov [rbx + r9*8 + 64], rax
    adcx rdi, [rbx + r9*8 + 72]
    mov [rbx + r9*8 + 72], rdi

    //10
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 72]

    adox r13, [rbx + r9*8 + 72]
    mov [rbx + r9*8 + 72], r13
    adox r12, [rbx + r9*8 + 80]
    mov [rbx + r9*8 + 80], r12

    //11
    mov rdx, [r10 + rcx*8]
    mulx rdi, rax, [r11 + rsi*8 + 80]

    adcx rax, [rbx + r9*8 + 80]
    mov [rbx + r9*8 + 80], rax
    adcx rdi, [rbx + r9*8 + 88]
    mov [rbx + r9*8 + 88], rdi

    //12
    mov rdx, [r10 + rcx*8]
    mulx r12, r13, [r11 + rsi*8 + 88]

    adox r13, [rbx + r9*8 + 88]
    mov [rbx + r9*8 + 88], r13
    adox r12, [rbx + r9*8 + 96]
    adcx r12, rbp
    mov [rbx + r9*8 + 96], r12

    mov rax, 0
    adox rax, [rbx + r9*8 + 104]
    adcx rax, rbp
    mov [rbx + r9*8 + 104], rax

    jc .LCarryLoop1
    jno .LCarryLoop1End
.LCarryLoop1:
    mov rax, 0
    adox rax, [rbx + r9*8 + 112]
    adcx rax, rbp
    mov [rbx + r9*8 + 112], rax
    jc .LCarryLoop1Inc
    jno .LCarryLoop1End
.LCarryLoop1Inc:
    add r9, 1
    jmp .LCarryLoop1

.LCarryLoop1End:
    add rsi, 24
    jl .LLoop2 //leave loop2

.LShortLoopStart:
    sub rsi, 12
    jz .LShortFinish
.LShortLoop:
    mov r9, rcx // r9 = n i + j
    add r9, rsi

    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx
    adc qword ptr [rbx + r9*8 + 16], 0

    jnc .LShortLoopFinish
.LCarryLoop2:
    adc qword ptr [rbx + r9*8 + 24], 0
    jnc .LShortLoopFinish
    add r9, 1
    jmp .LCarryLoop2

.LShortLoopFinish:
    add rsi, 1
    jl .LShortLoop

.LShortFinish:
    add rcx, 1
    jl .LLoop1 //leave loop1

    //resize result
    mov r15, r8 //result = r15
    mov rdi, r8
    call getOccupiedFields_Asm
    mov rcx, [r15]
    add rcx, rax
    mov [r15+8], rcx

    //leave the function
    mov rax, r15
    pop rbp
    pop r15
    pop r14
    pop r13
    pop r12
    pop rbx
    ret