#define ASM 1

.globl syscall_wrap

syscall_wrap:
    pushl %ebp
    pushl %esi
    pushl %edi

    pushl %edx     # arg 3
    pushl %ecx     # arg 2
    pushl %ebx     # arg 1

    cmp $1, %eax
    jl invalid_syscall
    cmp $16, %eax
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

    iret

syscall_jumptable:
    .long 0x0
    .long syscall_halt
    .long syscall_execute
    .long syscall_read
    .long syscall_write
    .long syscall_open
    .long syscall_close
    .long syscall_getargs
    .long syscall_vidmap
    .long syscall_set_handler
    .long syscall_sigreturn
    .long syscall_ioctl
    .long syscall_shutdown
    .long syscall_reboot
    .long syscall_ps
    .long syscall_poke
    .long syscall_status_msg
