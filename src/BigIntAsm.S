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
  	jna .L3
  	mov rcx, QWORD PTR [rdi+24] //move pointer of bigIntArray to rcx
  	jmp .L2
.L1:
  	sub rax, 1 	//num--
  	cmp rax, rdx //num > x->start + 1;
  	jna .L3
.L2:
  	cmp QWORD PTR [rcx-8+rax*8], 0 //x->bigIntArray[num - 1] == 0
  	je .L1
.L3:
  	sub rax, rsi
  	ret

//bigInt *add_Asm(bigInt *x, bigInt *y, bool negative);
add_Asm:
    //rdi = x, rsi = y, rdx = negative
    push r14
    push r15
    push rbx
    push rdx
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
    push r14
    push r15
    push rbx
    mov r14, [rdi + 24] //r14 = bigger->bigIntArray
    mov r15, [rsi + 24] //r15 = smaller->bigIntArray
    mov r11, [rdx + 24] //r11 = res->BigIntArray
    xor rbx, rbx
    jmp .LShiftAddHelperEntry

//void addHelper(bigInt *bigger, bigInt *smaller, bigInt *res);
addHelper_Asm:
    //rdi = bigger, rsi = smaller, rdx = res
    //rcx = biggerCount, r8 = smallerCount, r9 = i
    push r14
    push r15
    push rbx
    mov rcx, [rdi]
    mov r8, [rsi]
    xor r9, r9
    mov r14, [rdi + 24] //r14 = bigger->bigIntArray
    mov r15, [rsi + 24] //r15 = smaller->bigIntArray
    mov r11, [rdx + 24] //r11 = res->BigIntArray
    xor rbx, rbx
.LShiftAddHelperEntry:
    cmp r8, [rsi + 8]
    jz .LAddLoopAddHelperEnd
.LAddLoopAddHelper:
    add bl, -1
    mov r10, [r14 + rcx*8]
    adc r10, [r15 + r8*8]
    mov [r11 + r9*8], r10
    setc bl
    add rcx, 1
    add r8, 1
    add r9, 1
    cmp r8, [rsi + 8]
    jnz .LAddLoopAddHelper
.LAddLoopAddHelperEnd:
    //Add loop ende
    test bl, bl
    jz .LCopyAddHelper
    cmp rcx, [rdi + 8]
    jz .LLastCarryAddHelper
.LCopyAddLoop:
    mov r10, [r14 + rcx*8]
    add r10, 1
    mov [r11 + r9*8], r10
    setc bl
    add rcx, 1
    add r9, 1
    test bl, bl
    jz .LCopyAddHelper
    cmp rcx, [rdi + 8]
    jz .LLastCarryAddHelper
    jmp .LCopyAddLoop
.LCopyAddHelper:
    cmp rcx, [rdi + 8]
    jz .LEndeAddHelper
.LCopyLoopAddHelper:
    mov r10, [r14 + rcx*8]
    mov [r11 + r9*8], r10
    add rcx, 1
    add r9, 1
    cmp rcx, [rdi + 8]
    jnz .LCopyLoopAddHelper
.LEndeAddHelper:
    pop rbx
    pop r15
    pop r14
    ret
.LLastCarryAddHelper:
    add [r11 + r9*8], bl
    pop rbx
    pop r15
    pop r14
    ret

//*bigInt *sub(bigInt *x, bigInt *y, bool negative);
sub_Asm:
    //rdi = x, rsi = y, rdx = neagtive
    push rbx
    push r15
    push rdx
    mov r8, [rdi + 8]
    sub r8, [rdi] //r8 = xLen
    mov r9, [rsi + 8]
    sub r9, [rsi] //r9 = yLen
    push rdi
    push rsi
    push r8
    push r9
    mov rdi, r8
    call newBigInt //rax = res
    pop r9
    pop r8
    pop rsi
    pop rdi
    mov r15, [rax + 24] //r15 = res->BigIntArray
    mov r10, [rdi + 24] //r10 = x->bigIntArray
    mov r11, [rsi + 24] //r11 = y->bigIntArray
    mov rdi, [rdi] //rdi = xCount
    mov rsi, [rsi] //rsi = yCount
    xor rcx, rcx
    xor rbx, rbx
.LXBiggerLoop:
    add bl, -1
    mov rdx, [r10 + rdi*8]
    sbb rdx, [r11 + rsi*8]
    mov [r15 + rcx*8], rdx
    setc bl
    add rcx, 1
    add rsi, 1
    add rdi, 1
    cmp rcx, r9
    jnz .LXBiggerLoop
    //loopEnd
    test bl, bl
    jz .LCopySubHelper2
    cmp rcx, r8
    jz .LResizeSub
.LCopyLoopSubCarry2:
    mov rdx, [r10 + rdi*8]
    sub rdx, 1
    mov [r15 + rcx*8], rdx
    setc bl
    add rcx, 1
    add rdi, 1
    cmp rcx, r8
    jz .LResizeSub
    test bl, bl
    jnz .LCopyLoopSubCarry2
.LCopySubHelper2:
    cmp rcx, r8
    jz .LResizeSub
.LCopyLoopSub2:
    mov rdx, [r10 + rdi*8]
    mov [r15 + rcx*8], rdx
    add rcx, 1
    add rdi, 1
    cmp rcx, r8
    jnz .LCopyLoopSub2
.LResizeSub:
    //resize - rax = res
    mov rcx, [rax + 24] //res->bigIntArray
    mov rdx, [rax + 8] //res->end
    cmp QWORD PTR [rcx + rdx*8 - 8], 0
    jnz .LSubEnde
    sub rdx, [rax]
    cmp rdx, 1
    jbe .LSubEnde
    sub QWORD PTR [rax + 8], 1
.LSubEnde:
    pop rdx
    mov [rax + 32], rdx
    pop r15
    pop rbx
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