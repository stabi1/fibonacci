.intel_syntax noprefix
.global shiftAddSameNumber_Asm
.global do_add_asm
.global do_sub_asm
.global shiftAdd_Asm
.global shiftLeft_Asm
.global shiftRight_Asm

// void shiftAddSameNumber_Asm(const bigInt *x, const bigInt *toShift, size_t n)
shiftAddSameNumber_Asm:
    //rdi = x, rsi = toShift, rdx = n
    push r14
    push r15
    push rbx

    mov r9, [rdi + 8] //r9 := x->end
    mov r10, [rsi + 8] //r10 := toShift->end
    mov r11, [rdi + 24] //r11 = x->bigIntArray
    mov r15, [rsi + 24] //r15 = toShift->bigIntArray
    mov rdi, [rdi] //rdi := x->start
    mov rsi, [rsi] //rsi := toShift->start
    lea r11, [r11 + r9*8]
    lea r15, [r15 + r10*8]

    // xCount = rcx, toShiftCount = r8
    mov rcx, r9
    sub rcx, rdi
    neg rcx

    mov r8, r10
    sub r8, rsi
    neg r8

    lea rcx, [rcx + rdx] // add n to xCount

    xor rbx, rbx

    add r8, 12
    jge .LSASMLoopMultipleEnd
.LSASMLoopMultiple:
    sub r8, 12
    add bl, -1

    //1
    mov r10, [r15 + r8*8]
    adc [r11 + rcx*8], r10

    //2
    mov r10, [r15 + r8*8 + 8]
    adc [r11 + rcx*8 + 8], r10

    //3
    mov r10, [r15 + r8*8 + 16]
    adc [r11 + rcx*8 + 16], r10

    //4
    mov r10, [r15 + r8*8 + 24]
    adc [r11 + rcx*8 + 24], r10

    //5
    mov r10, [r15 + r8*8 + 32]
    adc [r11 + rcx*8 + 32], r10

    //6
    mov r10, [r15 + r8*8 + 40]
    adc [r11 + rcx*8 + 40], r10

    //7
    mov r10, [r15 + r8*8 + 48]
    adc [r11 + rcx*8 + 48], r10

    //8
    mov r10, [r15 + r8*8 + 56]
    adc [r11 + rcx*8 + 56], r10

    //9
    mov r10, [r15 + r8*8 + 64]
    adc [r11 + rcx*8 + 64], r10

    //10
    mov r10, [r15 + r8*8 + 72]
    adc [r11 + rcx*8 + 72], r10

    //11
    mov r10, [r15 + r8*8 + 80]
    adc [r11 + rcx*8 + 80], r10

    //12
    mov r10, [r15 + r8*8 + 88]
    adc [r11 + rcx*8 + 88], r10

    setc bl
    add rcx, 12
    add r8, 24
    jl .LSASMLoopMultiple

    //Add multiple loop ende
.LSASMLoopMultipleEnd:
    sub r8, 12
    jz .LSASMLoopSingeEnd
.LSASMLoopSingle:
    add bl, -1

    mov r10, [r15 + r8*8]
    adc [r11 + rcx*8], r10

    setc bl
    add rcx, 1
    add r8, 1
    jnz .LSASMLoopSingle

.LSASMLoopSingeEnd:
    //Add single loop ende
    test bl, bl
    jz .LSASMEnd
.LSASMCopyCarryLoop:
    adc qword ptr [r11 + rcx*8], 1
    setc bl
    add rcx, 1
    test bl, bl
    jnz .LSASMCopyCarryLoop

.LSASMEnd:
    pop rbx
    pop r15
    pop r14
    ret






//bigInt *shiftAdd(const bigInt *x, const bigInt *toShift, size_t n)
shiftAdd_Asm:
    //rdi = x, rsi = toShift, rdx = n
    push r12 //free
    push r13
    push r14
    push r15
    push rbx
    mov r14, rdi //r14 = x
    mov r15, rsi //r15 = toShift
    mov rbx, rdx //rbx = n
    mov r8, [rdi + 8] // x->end
    sub r8, [rdi] // r8 = xLen
    mov r9, [rsi + 8]
    sub r9, [rsi]
    add r9, rdx // r9 = toShiftLen
    mov r13, [rdi] //x->start

    //createRes
    push r8
    push r9
    cmp r8, r9
    jae .LxSizeBiggerShiftAdd
    mov rdi, r9
    add rdi, 1
    call newBigInt //rax = res
    jmp .LCopyXShiftAdd
.LxSizeBiggerShiftAdd:
    mov rdi, r8
    add rdi, 1
    call newBigInt //rax = res
.LCopyXShiftAdd: //copy x
    pop r9
    pop r8
    mov r10, [rax + 24] //r10 = res->bigIntArray
    mov r11, [r14 + 24] //r11 = x->bigIntArray
    xor rcx, rcx // rcx = i
    xor rdi, rdi // rdi = nCounter
    mov rsi, [r14] // rsi = xCount
    //LoopStart

    cmp rdi, rbx
    jz .LStartAddShiftAdd
    add rsi, 1
    cmp rsi, [r14 + 8]
    jae .LNoSSE
    sub rsi, 1
.LSSESchleife:
    lddqu xmm0, [r11 + rsi*8]
    movdqu [r10 + rcx*8], xmm0
    add rcx, 2 //inc counters
    add rdi, 2
    add rsi, 2
    cmp rdi, rbx
    jae .LSSEEnde
    cmp rsi, [r14 + 8]
    jb .LSSESchleife
.LSSEEnde:
    sub rcx, 2
    sub rsi, 1
    sub rdi, 2
.LNoSSE:
    sub rsi, 1
    cmp rdi, rbx
    jz .LStartAddShiftAdd
    cmp rsi, [r14 + 8]
    jnz .LCopyXShiftAddLoop
.LCopyXShiftAddLoop:
    mov rdx, [r11 + rsi*8] //copy
    mov [r10 + rcx*8], rdx
    add rcx, 1 //inc counters
    add rdi, 1
    add rsi, 1
    cmp rdi, rbx
    jz .LStartAddShiftAdd
    cmp rsi, [r14 + 8]
    jnz .LCopyXShiftAddLoop
    //LoopEnde
.LShiftAddZeroCopyLoop:
    mov QWORD PTR [r10 + rcx*8], 0
    add rcx, 1
    add rdi, 1
    cmp rdi, rbx
    jnz .LShiftAddZeroCopyLoop
    //LoopEnde
.LStartAddShiftAdd:
    mov rbx, rax //res sichern
    sub rsi, r13 // xCount - x->start
    cmp r8, r9
    jae .LxSizeBiggerShiftAdd2
    mov r8, rsi
    mov r9, rcx
    xor rcx, rcx
    mov rdx, rax
    mov rsi, r14
    mov rdi, r15
    call shiftAddHelper_Asm
    jmp .LResizeShiftAdd
.LxSizeBiggerShiftAdd2:
    mov r9, rcx //parameter verschieben
    xor r8, r8
    mov rcx, rsi
    mov rdx, rax
    mov rsi, r15
    mov rdi, r14
    call shiftAddHelper_Asm
.LResizeShiftAdd:
    //resize - rbx = res
    mov rcx, [rbx + 24] //res->bigIntArray
    mov rdx, [rbx + 8] //res->end
    cmp QWORD PTR [rcx + rdx*8 - 8], 0
    jnz .LShiftAddEnde
    sub rdx, [rbx]
    cmp rdx, 1
    jbe .LShiftAddEnde
    sub QWORD PTR [rbx + 8], 1
.LShiftAddEnde:
    mov rax, rbx
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret


//void shiftAddHelper(const bigInt *bigger, const bigInt *smaller, bigInt *res, size_t biggerCount, size_t smallerCount, size_t i);
shiftAddHelper_Asm:
    //rdi = bigger, rsi = smaller, rdx = res, rcx = biggerCount, r8 = samllerCount, r9 = i
    push r12
    push r13
    push r14
    push r15
    push rbx
    mov r12, [rdi + 8] //r12 := bigger->end
    mov r13, [rsi + 8] //r13 := smaller->end
    mov r14, [rdi + 24] //r14 = bigger->bigIntArray
    mov r15, [rsi + 24] //r15 = smaller->bigIntArray
    mov r11, [rdx + 24] //r11 = res->BigIntArray
    mov rdi, [rdi] //rdi := bigger->start
    mov rsi, [rsi] //rsi := smaller->start
    lea r14, [r14 + r12*8]
    lea r15, [r15 + r13*8]
    mov rbx, [rdx + 8]
    lea r11, [r11 + rbx*8]

    mov rbx, rcx
    mov rcx, r12
    sub rcx, rdi
    neg rcx
    add rcx, rbx

    mov rbx, r8
    mov r8, r13
    sub r8, rsi
    neg r8
    add r8, rbx

    mov rbx, r9
    mov r9, [rdx + 8]
    sub r9, [rdx]
    neg r9
    add r9, rbx

    jmp .LShiftAddHelperEntry


//void do_add_asm(const bigInt *bigger, const bigInt *smaller, bigInt *res);
do_add_asm:
    //rdi = bigger, rsi = smaller, rdx = res
    //rcx = biggerCount, r8 = smallerCount, r9 = i
    push r12
    push r13
    push r14
    push r15
    push rbx
    mov r12, [rdi + 8] //r12 := bigger->end
    mov r13, [rsi + 8] //r13 := smaller->end
    mov r14, [rdi + 24] //r14 = bigger->bigIntArray
    mov r15, [rsi + 24] //r15 = smaller->bigIntArray
    mov r11, [rdx + 24] //r11 = res->BigIntArray
    mov rdi, [rdi] //rdi := bigger->start
    mov rsi, [rsi] //rsi := smaller->start
    lea r14, [r14 + r12*8]
    lea r15, [r15 + r13*8]
    mov rbx, [rdx + 8]
    lea r11, [r11 + rbx*8]

    // biggerCount = rcx, samllerCount = r8, i = r9
    mov rcx, r12
    sub rcx, rdi
    neg rcx

    mov r8, r13
    sub r8, rsi
    neg r8

    mov r9, rbx
    sub r9, [rdx]
    neg r9

.LShiftAddHelperEntry:
    xor rbx, rbx

    add r8, 12
    jge .LAddLoopMultipleEnd
.LAddLoopMultiple:
    sub r8, 12
    add bl, -1

    //1
    mov r10, [r14 + rcx*8]
    adc r10, [r15 + r8*8]
    mov [r11 + r9*8], r10

    //2
    mov r10, [r14 + rcx*8 + 8]
    adc r10, [r15 + r8*8 + 8]
    mov [r11 + r9*8 + 8], r10

    //3
    mov r10, [r14 + rcx*8 + 16]
    adc r10, [r15 + r8*8 + 16]
    mov [r11 + r9*8 + 16], r10

    //4
    mov r10, [r14 + rcx*8 + 24]
    adc r10, [r15 + r8*8 + 24]
    mov [r11 + r9*8 + 24], r10

    //5
    mov r10, [r14 + rcx*8 + 32]
    adc r10, [r15 + r8*8 + 32]
    mov [r11 + r9*8 + 32], r10

    //6
    mov r10, [r14 + rcx*8 + 40]
    adc r10, [r15 + r8*8 + 40]
    mov [r11 + r9*8 + 40], r10

    //7
    mov r10, [r14 + rcx*8 + 48]
    adc r10, [r15 + r8*8 + 48]
    mov [r11 + r9*8 + 48], r10

    //8
    mov r10, [r14 + rcx*8 + 56]
    adc r10, [r15 + r8*8 + 56]
    mov [r11 + r9*8 + 56], r10

    //9
    mov r10, [r14 + rcx*8 + 64]
    adc r10, [r15 + r8*8 + 64]
    mov [r11 + r9*8 + 64], r10

    //10
    mov r10, [r14 + rcx*8 + 72]
    adc r10, [r15 + r8*8 + 72]
    mov [r11 + r9*8 + 72], r10

    //11
    mov r10, [r14 + rcx*8 + 80]
    adc r10, [r15 + r8*8 + 80]
    mov [r11 + r9*8 + 80], r10

    //12
    mov r10, [r14 + rcx*8 + 88]
    adc r10, [r15 + r8*8 + 88]
    mov [r11 + r9*8 + 88], r10

    setc bl
    add rcx, 12
    add r9, 12
    add r8, 24
    jl .LAddLoopMultiple

    //Add multiple loop ende
.LAddLoopMultipleEnd:
    sub r8, 12
    jz .LAddLoopSingeEnd
.LAddLoopSingle:
    add bl, -1

    mov r10, [r14 + rcx*8]
    adc r10, [r15 + r8*8]
    mov [r11 + r9*8], r10

    setc bl
    add rcx, 1
    add r9, 1
    add r8, 1
    jnz .LAddLoopSingle

.LAddLoopSingeEnd:
    //Add single loop ende
    test bl, bl
    jz .LAddCopy
    cmp rcx, 0
    jz .LAddLastCarry
.LAddCopyCarryLoop:
    mov r10, [r14 + rcx*8]
    add r10, 1
    mov [r11 + r9*8], r10
    setc bl
    add rcx, 1
    add r9, 1
    test bl, bl
    jz .LAddCopy
    cmp rcx, 0
    jz .LAddLastCarry
    jmp .LAddCopyCarryLoop
.LAddCopy:
    //ende copy add loop
    // r14 = bigger->bigIntArray
    // r11 = res->BigIntArray
    // biggerCount = rcx, i = r9
    lea rdi, [r11 + r9*8] // *dest
    lea rsi, [r14 + rcx*8] // *src
    neg rcx // blocks to copy
    lea rdx, [rcx*8] // num bytes to copy
    call memcpy
.LAddHelperEnde:
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret
.LAddLastCarry:
    add [r11 + r9*8], bl
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret


//*bigInt *do_sub_asm(const bigInt *bigger, const bigInt *smaller, bigInt *res);
// len(bigger) == len(res)
do_sub_asm:
    //rdi = bigger, rsi = smaller, rdx = res
    push r12
    push r13
    push r14
    push r15
    push rbx
    mov r12, [rdi + 8] //r12 := bigger->end
    mov r13, [rsi + 8] //r13 := smaller->end
    mov r14, [rdi + 24] //r14 = bigger->bigIntArray
    mov r15, [rsi + 24] //r15 = smaller->bigIntArray
    mov r11, [rdx + 24] //r11 = res->BigIntArray
    mov rdi, [rdi] //rdi := bigger->start
    mov rsi, [rsi] //rsi := smaller->start
    lea r14, [r14 + r12*8]
    lea r15, [r15 + r13*8]
    mov rbx, [rdx + 8]
    lea r11, [r11 + rbx*8]

    // biggerCount = rcx, samllerCount = r8
    mov rcx, r12
    sub rcx, rdi
    neg rcx

    mov r8, r13
    sub r8, rsi
    neg r8

    xor rbx, rbx

    add r8, 12
    jge .LSubMultipleLoopEnd
.LSubMultipleLoop:
    sub r8, 12
    add bl, -1

    //1
    mov r10, [r14 + rcx*8]
    sbb r10, [r15 + r8*8]
    mov [r11 + rcx*8], r10

    //2
    mov r10, [r14 + rcx*8 + 8]
    sbb r10, [r15 + r8*8 + 8]
    mov [r11 + rcx*8 + 8], r10

    //3
    mov r10, [r14 + rcx*8 + 16]
    sbb r10, [r15 + r8*8 + 16]
    mov [r11 + rcx*8 + 16], r10

    //4
    mov r10, [r14 + rcx*8 + 24]
    sbb r10, [r15 + r8*8 + 24]
    mov [r11 + rcx*8 + 24], r10

    //5
    mov r10, [r14 + rcx*8 + 32]
    sbb r10, [r15 + r8*8 + 32]
    mov [r11 + rcx*8 + 32], r10

    //6
    mov r10, [r14 + rcx*8 + 40]
    sbb r10, [r15 + r8*8 + 40]
    mov [r11 + rcx*8 + 40], r10

    //7
    mov r10, [r14 + rcx*8 + 48]
    sbb r10, [r15 + r8*8 + 48]
    mov [r11 + rcx*8 + 48], r10

    //8
    mov r10, [r14 + rcx*8 + 56]
    sbb r10, [r15 + r8*8 + 56]
    mov [r11 + rcx*8 + 56], r10

    //9
    mov r10, [r14 + rcx*8 + 64]
    sbb r10, [r15 + r8*8 + 64]
    mov [r11 + rcx*8 + 64], r10

    //10
    mov r10, [r14 + rcx*8 + 72]
    sbb r10, [r15 + r8*8 + 72]
    mov [r11 + rcx*8 + 72], r10

    //11
    mov r10, [r14 + rcx*8 + 80]
    sbb r10, [r15 + r8*8 + 80]
    mov [r11 + rcx*8 + 80], r10

    //12
    mov r10, [r14 + rcx*8 + 88]
    sbb r10, [r15 + r8*8 + 88]
    mov [r11 + rcx*8 + 88], r10

    setc bl
    add rcx, 12
    add r8, 24
    jl .LSubMultipleLoop

    //sub multiple loop end
.LSubMultipleLoopEnd:
    sub r8, 12
    jz .LSubSingleLoopEnd
.LSubSingleLoop:
    add bl, -1

    mov r10, [r14 + rcx*8]
    sbb r10, [r15 + r8*8]
    mov [r11 + rcx*8], r10

    setc bl
    add rcx, 1
    add r8, 1
    jnz .LSubSingleLoop

.LSubSingleLoopEnd:
    //single loop end
    test bl, bl
    jz .LCopySub
    cmp rcx, 0
    jz .LSubHelperEnde
.LSubCopyCarryLoop:
    mov r10, [r14 + rcx*8]
    sub r10, 1
    mov [r11 + rcx*8], r10
    setc bl
    add rcx, 1
    jz .LSubHelperEnde
    test bl, bl
    jnz .LSubCopyCarryLoop
.LCopySub:
    // r14 = bigger->bigIntArray
    // r11 = res->BigIntArray
    // biggerCount = rcx
    lea rdi, [r11 + rcx*8] // *dest
    lea rsi, [r14 + rcx*8] // *src
    neg rcx // blocks to copy
    lea rdx, [rcx*8] // num bytes to copy
    call memcpy
.LSubHelperEnde:
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret


//bigInt *shiftLeft(const bigInt *x, size_t n);
// n < 64
shiftLeft_Asm:
    //rdi = x, rsi = n
    push r15
    mov r8, [rdi + 8]
    sub r8, [rdi] //r8 = xLen
    add r8, 1
    push rdi
    push rsi
    mov rdi, r8
    call newBigInt // rax = res
    pop rsi
    pop rdi
    mov r15, [rdi] //r15 = i
    xor rdx, rdx //rdx = j
    mov r8, [rdi + 24] //r8 = x->bigIntArray
    mov r9, [rax + 24] //r9 = res->bigIntArry
.LShiftLeftLoop:
    mov r10, [r8 + r15*8]
    mov r11, r10
    mov rcx, rsi
    shl r10, cl
    or r10, [r9 + rdx*8]
    mov [r9 + rdx*8], r10
    mov rcx, 64
    sub rcx, rsi
    shr r11, cl
    or r11, [r9 + rdx*8 + 8]
    mov [r9 + rdx*8 + 8], r11
    add rdx, 1
    add r15, 1
    cmp r15, [rdi + 8]
    jnz .LShiftLeftLoop

    //resize - rax = res
    mov rdx, [rax + 8] //res->end
    cmp QWORD PTR [r9 + rdx*8 - 8], 0
    jnz .LShiftLeftEnd
    sub rdx, [rax]
    cmp rdx, 1
    jbe .LShiftLeftEnd
    sub QWORD PTR [rax + 8], 1
.LShiftLeftEnd:
    pop r15
    ret


//bigInt *shiftRight(const bigInt *x, size_t n);
// n < 64
shiftRight_Asm:
    //rdi = x, rsi = n
    push r15
    mov r8, [rdi + 8]
    sub r8, [rdi] //r8 = xLen
    push rdi
    push rsi
    mov rdi, r8
    call newBigInt // rax = res
    pop rsi
    pop rdi
    mov r15, [rdi] //r15 = i
    xor rdx, rdx //rdx = j
    mov r8, [rdi + 24] //r8 = x->bigIntArray
    mov r9, [rax + 24] //r9 = res->bigIntArry

    mov r10, [r8 + r15*8]
    mov rcx, rsi
    shr r10, cl
    or r10, [r9 + rdx*8]
    mov [r9 + rdx*8], r10
    add rdx, 1
    add r15, 1
    cmp r15, [rdi + 8]
    jz .LResizeShiftRight
.LShiftRightLoop:
    mov r10, [r8 + r15*8]
    mov r11, r10
    mov rcx, rsi
    shr r10, cl
    or r10, [r9 + rdx*8]
    mov [r9 + rdx*8], r10
    mov rcx, 64
    sub rcx, rsi
    shl r11, cl
    or r11, [r9 + rdx*8 - 8]
    mov [r9 + rdx*8 - 8], r11
    add rdx, 1
    add r15, 1
    cmp r15, [rdi + 8]
    jnz .LShiftRightLoop

.LResizeShiftRight:
    //resize - rax = res
    mov rdx, [rax + 8] //res->end
    cmp QWORD PTR [r9 + rdx*8 - 8], 0
    jnz .LShiftRightEnd
    sub rdx, [rax]
    cmp rdx, 1
    jbe .LShiftRightEnd
    sub QWORD PTR [rax + 8], 1
.LShiftRightEnd:
    pop r15
    ret