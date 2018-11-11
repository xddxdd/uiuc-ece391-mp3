#define ASM 1

.globl syscall_wrap

syscall_wrap:
    pushl %ecx
    pushl %edx
    pushl %ebx
    pushl %ebp
    pushl %esi
    pushl %edi

    pushl %edx     # arg 3
    pushl %ecx     # arg 2
    pushl %ebx     # arg 1

    cmp $1, %eax
    jl invalid_syscall
    cmp $10, %eax
    jg invalid_syscall

    movl syscall_jumptable(, %eax, 4), %eax
    call *%eax
    jmp end_syscall

invalid_syscall:
    movl $-1, %eax

end_syscall:
    popl %ebx
    popl %ecx
    popl %edx

    popl %edi
    popl %esi
    popl %ebp
    popl %ebx
    popl %edx
    popl %ecx

    iret

syscall_jumptable:
    .long 0x0
    .long halt
    .long execute
    .long read
    .long write
    .long open
    .long close
    .long getargs
    .long vidmap
    .long set_handler
    .long sigreturn
