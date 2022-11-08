.intel_syntax noprefix
.global getOccupiedFields_Asm
.global getLowerFrom_Asm
.global getUpperFrom_Asm
.global add_Asm
.global shiftAdd_Asm
.global sub_Asm
.global shiftLeft_Asm
.global shiftRight_Asm

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

//bigInt *add_Asm(bigInt *x, bigInt *y, bool negative);
add_Asm:
    //rdi = x, rsi = y, rdx = negative
    push r14
    push r15
    push rbx
    push rdx
    push rax //align stack pointer
    mov r14, rdi //r14 = x
    mov r15, rsi //r15 = y
    mov r8, [rdi + 8]
    sub r8, [rdi] // r8 = xLen
    mov r9, [rsi + 8]
    sub r9, [rsi] // r9 = yLen
    cmp r8, r9
    jae .LxSizeBigger
    mov rdi, r9
    add rdi, 1
    call newBigInt
    mov rdi, r15
    mov rsi, r14
    mov rdx, rax
    mov rbx, rax // rbx = res
    call addHelper_Asm
    jmp .LResize
.LxSizeBigger:
    mov rdi, r8
    add rdi, 1
    call newBigInt
    mov rdi, r14
    mov rsi, r15
    mov rdx, rax
    mov rbx, rax // rbx = res
    call addHelper_Asm
.LResize:
    //resize - rbx = res
    mov rcx, [rbx + 24] //res->bigIntArray
    mov rdx, [rbx + 8] //res->end
    cmp QWORD PTR [rcx + rdx*8 - 8], 0
    jnz .LAddEnde
    sub rdx, [rbx]
    cmp rdx, 1
    jbe .LAddEnde
    sub QWORD PTR [rbx + 8], 1
.LAddEnde:
    mov rax, rbx
    pop r8
    pop rdx
    mov [rax + 32], rdx
    pop rbx
    pop r15
    pop r14
    ret

//bigInt *shiftAdd(bigInt *x, bigInt *toShift, size_t n); uses SSE
shiftAdd_Asm:
    //rdi = x, rsi = toShift, rdx = n
    push r14
    push r15
    push rbx
    mov r14, rdi //r14 = x
    mov r15, rsi //r15 = toShift
    mov rbx, rdx //rbx = n
    mov r8, [rdi + 8]
    sub r8, [rdi] // r8 = xLen
    mov r9, [rsi + 8]
    sub r9, [rsi]
    add r9, rdx // r9 = toShiftLen

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
    lddqu xmm0, [r11 + rcx*8]
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
    cmp r8, r9
    jae .LxSizeBiggerShiftAdd2
    mov r8, rsi
    mov r9, rcx
    mov rcx, [r15]
    mov rdx, rax
    mov rsi, r14
    mov rdi, r15
    call shiftAddHelper_Asm
    jmp .LResizeShiftAdd
.LxSizeBiggerShiftAdd2:
    mov r9, rcx //parameter verschieben
    mov r8, [r15]
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
    ret

//void shiftAddHelper(bigInt *bigger, bigInt *smaller, bigInt *res, size_t biggerCount, size_t smallerCount, size_t i);
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

//void addHelper(bigInt *bigger, bigInt *smaller, bigInt *res);
addHelper_Asm:
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
    add rcx, 12
    jge .LAddCopyLoopSingleStart
.LAddCopyLoopSSE:
    sub rcx, 12

    //1
    lddqu xmm0, [r14 + rcx*8]
    movdqu [r11 + r9*8], xmm0
    //2
    lddqu xmm1, [r14 + rcx*8 + 16]
    movdqu [r11 + r9*8+ 16], xmm1
    //3
    lddqu xmm2, [r14 + rcx*8 + 32]
    movdqu [r11 + r9*8 + 32], xmm2
    //4
    lddqu xmm3, [r14 + rcx*8 + 48]
    movdqu [r11 + r9*8 + 48], xmm3
    //5
    lddqu xmm3, [r14 + rcx*8 + 64]
    movdqu [r11 + r9*8 + 64], xmm3
    //6
    lddqu xmm3, [r14 + rcx*8 + 80]
    movdqu [r11 + r9*8 + 80], xmm3

    add r9, 12
    add rcx, 24
    jl .LAddCopyLoopSSE

.LAddCopyLoopSingleStart:
    sub rcx, 12
    jz .LAddHelperEnde
.LAddCopyLoopSingle:
    mov r10, [r14 + rcx*8]
    mov [r11 + r9*8], r10
    add r9, 1
    add rcx, 1
    jnz .LAddCopyLoopSingle
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

//bigInt *sub(bigInt *x, bigInt *y, bool negative)
sub_Asm:
    //rdi = x, rsi = y, rdx = negative
    push r14
    push r15
    push rbx
    push rdx
    push rax //align stack pointer
    mov r14, rdi //r14 = x
    mov r15, rsi //r15 = y
    mov r8, [rdi + 8]
    sub r8, [rdi] // r8 = xLen
    mov r9, [rsi + 8]
    sub r9, [rsi] // r9 = yLen
    cmp r8, r9
    jae .LxSizeBiggerSub
    mov rdi, r9
    call newBigInt
    mov rdi, r15
    mov rsi, r14
    mov rdx, rax
    mov rbx, rax // rbx = res
    call subHelper_Asm
    jmp .LResizeSub
.LxSizeBiggerSub:
    mov rdi, r8
    call newBigInt
    mov rdi, r14
    mov rsi, r15
    mov rdx, rax
    mov rbx, rax // rbx = res
    call subHelper_Asm
.LResizeSub:
    //resize - rbx = res
    mov rcx, [rbx + 24] //res->bigIntArray
    mov rdx, [rbx + 8] //res->end
    cmp QWORD PTR [rcx + rdx*8 - 8], 0
    jnz .LSubEnde
    sub rdx, [rbx]
    cmp rdx, 1
    jbe .LSubEnde
    sub QWORD PTR [rbx + 8], 1
.LSubEnde:
    mov rax, rbx
    pop r8
    pop rdx
    mov [rax + 32], rdx
    pop rbx
    pop r15
    pop r14
    ret




//*bigInt *sub(bigInt *bigger, bigInt *smaller, bigInt *res);
subHelper_Asm:
    //rdi = bigger, rsi = smaller, rdx = res
    push r12
    push r13
    push r14
    push r15
    push rbx
    mov r12, [rdi + 8] //r8 := bigger->end
    mov r13, [rsi + 8] //r9 := smaller->end
    mov r14, [rdi + 24] //r10 = bigger->bigIntArray
    mov r15, [rsi + 24] //r11 = smaller->bigIntArray
    mov r11, [rdx + 24] //r15 = res->BigIntArray
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

    xor rbx, rbx

    add r8, 12
    jge .LSubMultipleLoopEnd
.LSubMultipleLoop:
    sub r8, 12
    add bl, -1

    //1
    mov r10, [r14 + rcx*8]
    sbb r10, [r15 + r8*8]
    mov [r11 + r9*8], r10

    //2
    mov r10, [r14 + rcx*8 + 8]
    sbb r10, [r15 + r8*8 + 8]
    mov [r11 + r9*8 + 8], r10

    //3
    mov r10, [r14 + rcx*8 + 16]
    sbb r10, [r15 + r8*8 + 16]
    mov [r11 + r9*8 + 16], r10

    //4
    mov r10, [r14 + rcx*8 + 24]
    sbb r10, [r15 + r8*8 + 24]
    mov [r11 + r9*8 + 24], r10

    //5
    mov r10, [r14 + rcx*8 + 32]
    sbb r10, [r15 + r8*8 + 32]
    mov [r11 + r9*8 + 32], r10

    //6
    mov r10, [r14 + rcx*8 + 40]
    sbb r10, [r15 + r8*8 + 40]
    mov [r11 + r9*8 + 40], r10

    //7
    mov r10, [r14 + rcx*8 + 48]
    sbb r10, [r15 + r8*8 + 48]
    mov [r11 + r9*8 + 48], r10

    //8
    mov r10, [r14 + rcx*8 + 56]
    sbb r10, [r15 + r8*8 + 56]
    mov [r11 + r9*8 + 56], r10

    //9
    mov r10, [r14 + rcx*8 + 64]
    sbb r10, [r15 + r8*8 + 64]
    mov [r11 + r9*8 + 64], r10

    //10
    mov r10, [r14 + rcx*8 + 72]
    sbb r10, [r15 + r8*8 + 72]
    mov [r11 + r9*8 + 72], r10

    //11
    mov r10, [r14 + rcx*8 + 80]
    sbb r10, [r15 + r8*8 + 80]
    mov [r11 + r9*8 + 80], r10

    //12
    mov r10, [r14 + rcx*8 + 88]
    sbb r10, [r15 + r8*8 + 88]
    mov [r11 + r9*8 + 88], r10

    setc bl
    add rcx, 12
    add r9, 12
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
    mov [r11 + r9*8], r10

    setc bl
    add rcx, 1
    add r9, 1
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
    mov [r11 + r9*8], r10
    setc bl
    add r9, 1
    add rcx, 1
    jz .LSubHelperEnde
    test bl, bl
    jnz .LSubCopyCarryLoop
.LCopySub:
    cmp rcx, 0
    jz .LSubHelperEnde
    //ende copy sub loop
    add rcx, 12
    jge .LSubCopyLoopSingleStart
.LSubCopyLoopSSE:
    sub rcx, 12

    //1
    lddqu xmm0, [r14 + rcx*8]
    movdqu [r11 + r9*8], xmm0
    //2
    lddqu xmm1, [r14 + rcx*8 + 16]
    movdqu [r11 + r9*8+ 16], xmm1
    //3
    lddqu xmm2, [r14 + rcx*8 + 32]
    movdqu [r11 + r9*8 + 32], xmm2
    //4
    lddqu xmm3, [r14 + rcx*8 + 48]
    movdqu [r11 + r9*8 + 48], xmm3
    //5
    lddqu xmm3, [r14 + rcx*8 + 64]
    movdqu [r11 + r9*8 + 64], xmm3
    //6
    lddqu xmm3, [r14 + rcx*8 + 80]
    movdqu [r11 + r9*8 + 80], xmm3

    add r9, 12
    add rcx, 24
    jl .LSubCopyLoopSSE

.LSubCopyLoopSingleStart:
    sub rcx, 12
    jz .LSubHelperEnde
.LSubCopyLoopSingle:
    mov r10, [r14 + rcx*8]
    mov [r11 + r9*8], r10
    add r9, 1
    add rcx, 1
    jnz .LSubCopyLoopSingle
.LSubHelperEnde:
    pop rbx
    pop r15
    pop r14
    pop r13
    pop r12
    ret

//bigInt *shiftLeft(bigInt *x, size_t n);
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

//bigInt *shiftRight(bigInt *x, size_t n);
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