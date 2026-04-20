.intel_syntax noprefix
.global mod_prime

// barret reduction
// uint64_t mod_prime(unsigned __int128 x);
mod_prime:
    movabs  r8, 0x39975a3fe9867f21
    mov     rax, rdi
    mul     r8
    mov     rcx, rdx
    mov     rax, rsi
    mul     r8
    mov     r8, rdx
    mov     r9, rax
    add     r9, rcx
    adc     r8, 0
    movabs  r10, 0x2f684bda12f684bc
    mov     rax, rdi
    mul     r10
    mov     rcx, rdx
    add     rax, r9
    adc     rcx, r8
    mov     rax, rsi
    mul     r10
    add     rax, rcx
    adc     rdx, 0
    add     rax, rdi
    adc     rdx, rsi
    movabs  rax, 0x27FFFFFFFFFFFFFF
    imul    rax, rdx
    add     rax, rdi
    ret