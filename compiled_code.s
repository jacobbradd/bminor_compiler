    .data
    .globl a
a:
    .quad 2
    .globl zzz
zzz:
    .quad 3
    .globl my_arr
my_arr:
    .quad 1
    .quad 2
    .quad 3
    .quad 4
    .quad 5

    .section    .rodata
.L1:
    .string "%s\n"
.L2:
    .string "HELLO WORLD!"
.L3:
    .string "Here's j:"
.L4:
    .string "%d\n"
.L7:
    .string "abc123"
.L28:
    .string "Here's s:"
.L29:
    .string "Outputs:"

    .text
    .globl b
b:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %rbx
    subq $32, %rsp
    leaq .L2(%rip), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq $0, %rbx
    movq %rbx, -56(%rbp)
    movq $3, %rbx
    movq %rbx, %r15
    movq a(%rip), %rbx
    imulq %rbx, %r15
    movq -56(%rbp), %rbx
    addq $1, -56(%rbp)
    addq %r15, %rbx
    movq %rbx, %r15
    movq %r15, -64(%rbp)
    movq $42, %rbx
    movq %rbx, %r15
    movq zzz(%rip), %rbx
    addq %rbx, %r15
    movq %r15, -72(%rbp)
    leaq .L3(%rip), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq -72(%rbp), %rbx
    leaq .L4(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq a(%rip), %rbx
    movq %rbx, %r15
    movq a(%rip), %rbx
    imulq %rbx, %r15
    movq a(%rip), %rbx
    imulq %rbx, %r15
    movq a(%rip), %rbx
    imulq %rbx, %r15
    movq %r15, %rax
    jmp b_EP
b_EP:
    addq $32, %rsp
    popq %rbx
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbp
    retq
    .globl main
main:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %rbx
    subq $32, %rsp
    movq $16, %rbx
    movq %rbx, -56(%rbp)
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $3, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setg %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L5
    movq -56(%rbp), %rbx
    addq $1, -56(%rbp)
    jmp .L6
.L5:
.L6:
    movq $0, %rbx
    movq %rbx, -56(%rbp)
    movq $3, %rbx
    movq %rbx, -64(%rbp)
    leaq .L7(%rip), %rbx
    movq %rbx, -72(%rbp)
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $0, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L8
.L10:
    movq -56(%rbp), %rbx
    movq %rbx, %rax
    movq $2, %rbx
    cqto
    idivq %rbx
    movq %rax, %r15
    movq $50, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L11
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $25, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L12
    movq -64(%rbp), %rbx
    movq %rbx, %r15
    movq $2, %rbx
    addq %rbx, %r15
    movq %r15, -64(%rbp)
    jmp .L13
.L12:
    movq -72(%rbp), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq -56(%rbp), %rbx
    leaq .L4(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
.L13:
    movq -56(%rbp), %rbx
    addq $1, -56(%rbp)
    jmp .L10
.L11:
    jmp .L9
.L8:
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $0, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setg %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L14
.L16:
    movq -56(%rbp), %rbx
    movq %rbx, %rax
    movq $2, %rbx
    cqto
    idivq %rbx
    movq %rax, %r15
    movq $25, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L17
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $25, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L18
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $5, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setg %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L20
    movq -64(%rbp), %rbx
    addq $1, -64(%rbp)
    jmp .L21
.L20:
    movq -64(%rbp), %rbx
    movq %rbx, %r15
    movq $2, %rbx
    addq %rbx, %r15
    movq %r15, -64(%rbp)
.L21:
    jmp .L19
.L18:
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $5, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setg %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L22
    movq -72(%rbp), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    jmp .L23
.L22:
    movq -56(%rbp), %rbx
    leaq .L4(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
.L23:
.L19:
    movq -56(%rbp), %rbx
    addq $1, -56(%rbp)
    jmp .L16
.L17:
    jmp .L15
.L14:
.L15:
.L9:
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $0, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L24
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $1, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L26
    movq -64(%rbp), %rbx
    movq %rbx, %r15
    movq $10, %rbx
    subq %rbx, %r15
    movq %r15, -64(%rbp)
    jmp .L27
.L26:
    movq -64(%rbp), %rbx
    movq %rbx, %r15
    movq $10, %rbx
    addq %rbx, %r15
    movq %r15, -64(%rbp)
.L27:
    jmp .L25
.L24:
    movq -64(%rbp), %rbx
    subq $1, -64(%rbp)
.L25:
    leaq .L28(%rip), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq -72(%rbp), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq -64(%rbp), %rbx
    movq %rbx, -64(%rbp)
    leaq .L29(%rip), %rbx
    leaq .L1(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq -64(%rbp), %rbx
    leaq .L4(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    pushq %r10
    pushq %r11
    call c
    popq %r11
    popq %r10
    movq %rax, %rbx
    leaq .L4(%rip), %rdi
    movq %rbx, %rsi
    movq $0, %rax
    call printf@plt
    movq $0, %rbx
    movq %rbx, %rax
    jmp main_EP
main_EP:
    addq $32, %rsp
    popq %rbx
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbp
    retq
    .globl c
c:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %rbx
    subq $32, %rsp
    movq $2, %rbx
    movq %rbx, -56(%rbp)
    movq -56(%rbp), %rbx
    movq %rbx, -72(%rbp)
    movq -72(%rbp), %rdi
    pushq %r10
    pushq %r11
    call compute
    popq %r11
    popq %r10
    movq %rax, %rbx
    movq %rbx, %rax
    jmp c_EP
c_EP:
    addq $32, %rsp
    popq %rbx
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbp
    retq
    .globl compute
compute:
    pushq %rbp
    movq %rsp, %rbp
    pushq %r15
    pushq %r14
    pushq %r13
    pushq %r12
    pushq %rbx
    subq $32, %rsp
    movq $1, %rbx
    movq %rbx, -56(%rbp)
    movq $0, %rbx
    movq %rbx, -64(%rbp)
    movq $0, %rbx
    movq %rbx, -56(%rbp)
.L30:
    movq -56(%rbp), %rbx
    movq %rbx, %r15
    movq $10, %rbx
    xorq %rax, %rax
    cmpq %rbx, %r15
    setl %al
    movzbq %al, %rax
    movq %rax, %r15
    movq %r15, %rax
    testq %rax, %rax
    jz .L31
    movq -64(%rbp), %rbx
    movq %rbx, %r15
    movq -56(%rbp), %rbx
    addq %rbx, %r15
    movq %r15, -64(%rbp)
    movq -56(%rbp), %rbx
    addq $1, -56(%rbp)
    jmp .L30
.L31:
    movq $2, %rbx
    movq %rbx, -72(%rbp)
    movq -72(%rbp), %rbx
    movq %rbx, %rax
    movq -72(%rbp), %rbx
    movq %rbx, %r15
    movq -72(%rbp), %rbx
    subq %rbx, %r15
    movq %r15, %r14
    movq -72(%rbp), %rbx
    movq %rbx, %r15
    movq -72(%rbp), %rbx
    addq %rbx, %r15
    subq %r15, %r14
    movq -72(%rbp), %rbx
    xorq %r14, %rbx
    movq %rbx, %r14
    cqto
    idivq %r14
    movq %rdx, %r14
    movq %r14, %rax
    movq -72(%rbp), %rbx
    cqto
    idivq %rbx
    movq %rax, %r14
    movq -72(%rbp), %rbx
    movq %rbx, %r15
    movq -72(%rbp), %rbx
    imulq %rbx, %r15
    addq %r14, %r15
    movq %r15, %r14
    movq %rsi, %rbx
    movq %rbx, %r15
    movq -72(%rbp), %rbx
    orq %rbx, %r15
    movq %rsi, %rbx
    orq %rbx, %r15
    movq -72(%rbp), %rbx
    andq %r15, %rbx
    movq %rbx, %r15
    orq %r15, %r14
    movq %r14, -72(%rbp)
    movq -64(%rbp), %rbx
    movq %rbx, %rax
    jmp compute_EP
compute_EP:
    addq $32, %rsp
    popq %rbx
    popq %r12
    popq %r13
    popq %r14
    popq %r15
    popq %rbp
    retq
