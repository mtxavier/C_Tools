	.file	"RawSwitch.Modele.c"
	.text
	.p2align 4,,15
.globl ThdFiberRawLaunch
	.type	ThdFiberRawLaunch, @function
ThdFiberRawLaunch:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	%ebx
	movl	8(%ebp), %ebx
	pushl %esi
	pushl %edi
	movl	12(%ebx), %eax
	movl	(%ebx), %edx
	movl  %esp,%ecx
	movl  %edx,%esp
	movl	%ecx, (%ebx)
	subl	$20, %esp
	movl	%ebx, (%esp)
	call	*(%eax)
	addl	$20, %esp
	movl	%ebx, %eax
	popl  %edi
	popl  %esi
	popl	%ebx
	popl	%ebp
	ret
	.size	ThdFiberRawLaunch, .-ThdFiberRawLaunch
	.p2align 4,,15
.globl ThdFiberRawSwitch
	.type	ThdFiberRawSwitch, @function
ThdFiberRawSwitch:
	pushl	%ebp
	movl	%esp, %ebp
	movl	8(%ebp), %eax
	movl	(%eax), %edx
	pushl %ebx
	pushl %esi
	pushl %edi
	movl  %esp,%ecx
	movl  %edx,%esp
	movl	%ecx, (%eax)
	popl  %edi
	popl  %esi
	popl  %ebx
	popl	%ebp
	ret
	.size	ThdFiberRawSwitch, .-ThdFiberRawSwitch
	.ident	"GCC: (Debian 4.4.5-8) 4.4.5"
	.section	.note.GNU-stack,"",@progbits
