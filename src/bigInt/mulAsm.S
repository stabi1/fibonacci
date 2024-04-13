.intel_syntax noprefix
.global naiveMul_Asm
.global getOccupiedFields_Asm

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
    mov rdx, [r10 + rcx*8]
    add rsi, 12
    jge .LShortLoopStart
.LLoop2:
    sub rsi, 12

    lea r9, [rcx + rsi] // r9 = n = i + j
    xor rbp, rbp

    //Mul
    //1
    mulx rdi, rax, [r11 + rsi*8]

    adcx rax, [rbx + r9*8]
    mov [rbx + r9*8], rax
    adcx rdi, [rbx + r9*8 + 8]

    //2
    mulx r12, r13, [r11 + rsi*8 + 8]

    adox r13, rdi
    mov [rbx + r9*8 + 8], r13
    adox r12, [rbx + r9*8 + 16]

    //3
    mulx rdi, rax, [r11 + rsi*8 + 16]

    adcx rax, r12
    mov [rbx + r9*8 + 16], rax
    adcx rdi, [rbx + r9*8 + 24]

    //4
    mulx r12, r13, [r11 + rsi*8 + 24]

    adox r13, rdi
    mov [rbx + r9*8 + 24], r13
    adox r12, [rbx + r9*8 + 32]

    //5
    mulx rdi, rax, [r11 + rsi*8 + 32]

    adcx rax, r12
    mov [rbx + r9*8 + 32], rax
    adcx rdi, [rbx + r9*8 + 40]

    //6
    mulx r12, r13, [r11 + rsi*8 + 40]

    adox r13, rdi
    mov [rbx + r9*8 + 40], r13
    adox r12, [rbx + r9*8 + 48]

    //7
    mulx rdi, rax, [r11 + rsi*8 + 48]

    adcx rax, r12
    mov [rbx + r9*8 + 48], rax
    adcx rdi, [rbx + r9*8 + 56]

    //8
    mulx r12, r13, [r11 + rsi*8 + 56]

    adox r13, rdi
    mov [rbx + r9*8 + 56], r13
    adox r12, [rbx + r9*8 + 64]

    //9
    mulx rdi, rax, [r11 + rsi*8 + 64]

    adcx rax, r12
    mov [rbx + r9*8 + 64], rax
    adcx rdi, [rbx + r9*8 + 72]

    //10
    mulx r12, r13, [r11 + rsi*8 + 72]

    adox r13, rdi
    mov [rbx + r9*8 + 72], r13
    adox r12, [rbx + r9*8 + 80]

    //11
    mulx rdi, rax, [r11 + rsi*8 + 80]

    adcx rax, r12
    mov [rbx + r9*8 + 80], rax
    adcx rdi, [rbx + r9*8 + 88]

    //12
    mulx r12, r13, [r11 + rsi*8 + 88]

    adox r13, rdi
    mov [rbx + r9*8 + 88], r13
    adox r12, [rbx + r9*8 + 96]
    adcx r12, rbp
    mov [rbx + r9*8 + 96], r12

    mov rax, 0
    adox rax, [rbx + r9*8 + 104]
    adcx rax, rbp
    mov [rbx + r9*8 + 104], rax

    add rsi, 24
    jl .LLoop2 //leave loop2

.LShortLoopStart:
    sub rsi, 12
    jz .LShortFinish
.LShortLoop:
    lea r9, [rcx + rsi] // r9 = n = i + j

    mulx r12, rax, [r11 + rsi*8]

    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], r12
    adc qword ptr [rbx + r9*8 + 16], 0

    add rsi, 1
    jl .LShortLoop

.LShortFinish:
    add rcx, 1
    jl .LLoop1 //leave loop1

.LResize:
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



//size_t getOccupiedFields_Asm(bigInt *x);
getOccupiedFields_Asm:
    mov rax, QWORD PTR [rdi+8] //size_t num = x->end;
    mov rsi, QWORD PTR [rdi] //x->start
    mov rdx, rsi
    add rdx, 1 //x->start + 1
    cmp rax, rsi //num > x->start + 1
    jna .LGetOccFieldsEnd
    mov rcx, QWORD PTR [rdi+24] //move pointer of bigIntArray to rcx
    jmp .LIsZero
.LStartLoopGetOccFields:
    sub rax, 1 	//num--
    cmp rax, rdx //num > x->start + 1;
    jna .LGetOccFieldsEnd
.LIsZero:
    cmp QWORD PTR [rcx-8+rax*8], 0 //x->bigIntArray[num - 1] == 0
    je .LStartLoopGetOccFields
.LGetOccFieldsEnd:
    sub rax, rsi
    ret