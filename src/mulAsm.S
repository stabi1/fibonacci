.intel_syntax noprefix
.global naivMul_AsmSicher
.global naivMul_AsmArbeit
.global naivMul_AsmVergleich

.align 16
naivMul_AsmArbeit: //(mit pointer ans ende, kein cmp + sehr entrollt + keinCarryLoop -> sehr sehr sehr gut)
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
.LLoop1:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    add rsi, 8
    jge .LKurzLoopAnfang
.LLoop2:
    sub rsi, 8

    mov r9, rcx // r9 = n i + j
    add r9, rsi

    //Mul
    //1
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8]

    add rax, [rbx + r9*8]
    mov [rbx + r9*8], rax
    adcx rdx, [rbx + r9*8 + 8]
    mov [rbx + r9*8 + 8], rdx

    //2
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 8]

    xor rbp, rbp
    adox rax, [rbx + r9*8 + 8]
    mov [rbx + r9*8 + 8], rax
    adox rdx, [rbx + r9*8 + 16]
    mov [rbx + r9*8 + 16], rdx

    //3
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 16]

    adcx rax, [rbx + r9*8 + 16]
    mov [rbx + r9*8 + 16], rax
    adcx rdx, [rbx + r9*8 + 24]
    mov [rbx + r9*8 + 24], rdx

    //4
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 24]

    adox rax, [rbx + r9*8 + 24]
    mov [rbx + r9*8 + 24], rax
    adox rdx, [rbx + r9*8 + 32]
    mov [rbx + r9*8 + 32], rdx

    //5
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 32]

    adcx rax, [rbx + r9*8 + 32]
    mov [rbx + r9*8 + 32], rax
    adcx rdx, [rbx + r9*8 + 40]
    mov [rbx + r9*8 + 40], rdx

    //6
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 40]

    adox rax, [rbx + r9*8 + 40]
    mov [rbx + r9*8 + 40], rax
    adox rdx, [rbx + r9*8 + 48]
    mov [rbx + r9*8 + 48], rdx

    //7
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 48]

    adcx rax, [rbx + r9*8 + 48]
    mov [rbx + r9*8 + 48], rax
    adcx rdx, [rbx + r9*8 + 56]
    mov [rbx + r9*8 + 56], rdx

    //8
    mov rdx, [r10 + rcx*8]
    mulx rdx, rax, [r11 + rsi*8 + 56]

    adox rax, [rbx + r9*8 + 56]
    mov [rbx + r9*8 + 56], rax
    adox rdx, [rbx + r9*8 + 64]
    adcx rdx, rbp
    mov [rbx + r9*8 + 64], rdx

    xor rax, rax
    adox rax, [rbx + r9*8 + 72]
    adcx rax, rbp
    mov [rbx + r9*8 + 72], rax

    add rsi, 16
    jl .LLoop2 //leave loop2
.LKurzLoopAnfang:
    sub rsi, 8
    jz .LKurzFertig
.LKurzLoop:
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx

    jnc .LKurzLoopEnd
.LCarryLoop2:
    adc qword ptr [rbx + r9*8 + 16], 0
    jnc .LKurzLoopEnd
    add r9, 1
    jmp .LCarryLoop2

.LKurzLoopEnd:
    add rsi, 1
    jl .LKurzLoop

.LKurzFertig:
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
.JmpKurzLoop:
    sub rsi, 8
    jmp .LKurzLoop




.align 16
//void naivMul_Asm(bigInt *x, bigInt *y);
naivMul_AsmSicher:
 //(mit pointer ans ende, kein cmp + sehr entrollt + keinCarryLoop -> sehr sehr sehr gut)
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
.LLoop1S:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    add rsi, 8
    jge .LKurzLoopAnfangS
.LLoop2S:
    sub rsi, 8
    //Mul
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi

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

    //5
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 32]

    add [rbx + r9*8 + 32], rax
    adc [rbx + r9*8 + 40], rdx
    adc qword ptr [rbx + r9*8 + 48], 0

    //6
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 40]

    add [rbx + r9*8 + 40], rax
    adc [rbx + r9*8 + 48], rdx
    adc qword ptr [rbx + r9*8 + 56], 0

    //7
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 48]

    add [rbx + r9*8 + 48], rax
    adc [rbx + r9*8 + 56], rdx
    adc qword ptr [rbx + r9*8 + 64], 0

    //8
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8 + 56]

    add [rbx + r9*8 + 56], rax
    adc [rbx + r9*8 + 64], rdx
    adc qword ptr [rbx + r9*8 + 72], 0

    add rsi, 16
    jl .LLoop2S //leave loop2
.LKurzLoopAnfangS:
    sub rsi, 8
    jz .LKurzFertigS
.LKurzLoopS:
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx

    jnc .LKurzLoopEndS
.LCarryLoop2S:
    adc qword ptr [rbx + r9*8 + 16], 0
    jnc .LKurzLoopEndS
    add r9, 1
    jmp .LCarryLoop2S

.LKurzLoopEndS:
    add rsi, 1
    jl .LKurzLoopS

.LKurzFertigS:
    add rcx, 1
    jl .LLoop1S //leave loop1

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

.align 16
naivMul_AsmVergleich: //(mit pointer ans ende, kein cmp + sehr entrollt + keinCarryLoop -> sehr sehr sehr gut)
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
    jge .LSkipMultipleLoopV
    sub r10, 16
.LLoop1V:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
    add rsi, 4
    jge .JmpKurzLoopV
.LLoop2V:
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
    jl .LLoop2V //leave loop2
    sub rsi, 4
    jz .LKurzFertigV
.LKurzLoopV:
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
    jl .LKurzLoopV

.LKurzFertigV:
    add rcx, 1
    jl .LLoop1V //leave loop1

    add r10, 16
.LSkipMultipleLoopV:
    sub rcx, 2
.LLoop1EinzelV:
    mov rsi, r15 // j=y->end - y->start
    sub rsi ,r14
    neg rsi
.LLoop2EinzelV:

    //Mul
    mov rax, [r10 + rcx*8]
    mul qword ptr [r11 + rsi*8]

    mov r9, rcx // r9 = n i + j
    add r9, rsi

    //add
    add [rbx + r9*8], rax
    adc [rbx + r9*8 + 8], rdx

    //carry loop
    jnc .LLoopEndEinzelV
    add r9, 2 // i := r9
    jz .LLoopEndEinzelV
.LLoopEinzelV:
    add qword ptr [rbx + r9*8], 1
    jnc .LLoopEndEinzelV
    setc al
    add r9, 1
    jz .LLoopEinzelV

.LLoopEndEinzelV:
    add rsi, 1
    jnz .LLoop2EinzelV //leave loop2
    add rcx, 1
    jnz .LLoop1EinzelV //leave loop1

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
.JmpKurzLoopV:
    sub rsi, 4
    jmp .LKurzLoopV