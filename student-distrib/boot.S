# boot.S - start point for the kernel after GRUB gives us control
# vim:ts=4 noexpandtab

#define ASM     1

#include "multiboot.h"
#include "x86_desc.h"

.text

    # Multiboot header (required for GRUB to boot us)
    .long MULTIBOOT_HEADER_MAGIC
    .long MULTIBOOT_HEADER_FLAGS
    .long -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

# Entrypoint to the kernel
.globl start, _start

.align 4
start:
_start:
    # Make sure interrupts are off
    cli
    jmp     continue

continue:
    lgdt gdt_desc_ptr   # Load the GDT, as defined in x86_desc.S

    # Initialize & load IDT
    pushl %eax  # Preserve EAX, ECX, EDX, as by C calling convention
    pushl %ecx
    pushl %edx
    call idt_init   # in idt.c, handles initialization of IDT
                    # Without this you will be thrown right back to GRUB screen
    popl %edx   # Restore EAX, ECX, EDX
    popl %ecx
    popl %eax

    lidt idt_desc_ptr   # Load the IDT

    # Load CS with the new descriptor value
    ljmp    $KERNEL_CS, $keep_going

keep_going:
    # Set up ESP so we can have an initial stack
    movl    $0x800000, %esp

    # Set up the rest of the segment selector registers
    movw    $KERNEL_DS, %cx
    movw    %cx, %ss
    movw    %cx, %ds
    movw    %cx, %es
    movw    %cx, %fs
    movw    %cx, %gs

    # Push the parameters that entry() expects (see kernel.c):
    # eax = multiboot magic
    # ebx = address of multiboot info struct
    pushl   %ebx
    pushl   %eax

    # Jump to the C entrypoint to the kernel.
    call    entry

    # We'll never get back here, but we put in a hlt anyway.
halt:
    hlt
    jmp     halt
