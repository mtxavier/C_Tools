	.file	"RawSwitch.Modele.c"
	.section	.text.unlikely,"ax",@progbits
.LCOLDB0:
	.text
.LHOTB0:
	.p2align 4,,15
	.globl	ThdFiberRawLaunch
	.type	ThdFiberRawLaunch, @function
ThdFiberRawLaunch:
.LFB0:
	.cfi_startproc
	pushq	%rbx
	.cfi_def_cfa_offset 16
	.cfi_offset 3, -16
	movq	%rdi, %rbx
#APP
# 25 "RawSwitch.Modele.c" 1
	pushq %rsi
	pushq %rdi
	pushq %rbp
	
# 0 "" 2
#NO_APP
	movq	24(%rdi), %rax
	movq	(%rdi), %rdx
#APP
# 31 "RawSwitch.Modele.c" 1
	movq  %rsp,%rcx
	movq  %rdx,%rsp
	
# 0 "" 2
#NO_APP
	movq	%rcx, (%rdi)
	call	*(%rax)
#APP
# 39 "RawSwitch.Modele.c" 1
	popq  %rbp
	popq  %rdi
	popq  %rsi
	
# 0 "" 2
#NO_APP
	movq	%rbx, %rax
	popq	%rbx
	.cfi_def_cfa_offset 8
	ret
	.cfi_endproc
.LFE0:
	.size	ThdFiberRawLaunch, .-ThdFiberRawLaunch
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB1:
	.text
.LHOTB1:
	.p2align 4,,15
	.globl	ThdFiberRawSwitch
	.type	ThdFiberRawSwitch, @function
ThdFiberRawSwitch:
.LFB1:
	.cfi_startproc
	movq	%rdi, %rax
#APP
# 49 "RawSwitch.Modele.c" 1
	pushq %rbx
	pushq %rsi
	pushq %rdi
	pushq %rbp
	
# 0 "" 2
#NO_APP
	movq	(%rdi), %rdx
#APP
# 56 "RawSwitch.Modele.c" 1
	movq  %rsp,%rcx
	movq  %rdx,%rsp
	
# 0 "" 2
#NO_APP
	movq	%rcx, (%rdi)
#APP
# 63 "RawSwitch.Modele.c" 1
	popq  %rbp
	popq  %rdi
	popq  %rsi
	popq  %rbx
	
# 0 "" 2
#NO_APP
	ret
	.cfi_endproc
.LFE1:
	.size	ThdFiberRawSwitch, .-ThdFiberRawSwitch
	.section	.text.unlikely
.LCOLDE1:
	.text
.LHOTE1:
	.ident	"GCC: (Debian 4.9.2-10) 4.9.2"
	.section	.note.GNU-stack,"",@progbits
