	.section	__TEXT,__text,regular,pure_instructions
	.build_version macos, 10, 15	sdk_version 10, 15
	.globl	_main                   ## -- Begin function main
	.p2align	4, 0x90
_main:                                  ## @main
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	subq	$96, %rsp
	movq	___stack_chk_guard@GOTPCREL(%rip), %rax
	movq	(%rax), %rax
	movq	%rax, -8(%rbp)
	movl	$0, -52(%rbp)
	movl	$0, -56(%rbp)
	movq	$0, -64(%rbp)
LBB0_1:                                 ## =>This Inner Loop Header: Depth=1
	cmpq	$4, -64(%rbp)
	jae	LBB0_4
## %bb.2:                               ##   in Loop: Header=BB0_1 Depth=1
	xorl	%eax, %eax
	movl	%eax, %esi
	movq	-64(%rbp), %rcx
	shlq	$3, %rcx
	leaq	-48(%rbp), %rdx
	addq	%rcx, %rdx
	leaq	-56(%rbp), %rcx
	movq	%rdx, %rdi
	leaq	_increment_routine(%rip), %rdx
	callq	_pthread_create
	movl	%eax, -76(%rbp)         ## 4-byte Spill
## %bb.3:                               ##   in Loop: Header=BB0_1 Depth=1
	movq	-64(%rbp), %rax
	addq	$1, %rax
	movq	%rax, -64(%rbp)
	jmp	LBB0_1
LBB0_4:
	movq	$0, -72(%rbp)
LBB0_5:                                 ## =>This Inner Loop Header: Depth=1
	cmpq	$4, -72(%rbp)
	jae	LBB0_8
## %bb.6:                               ##   in Loop: Header=BB0_5 Depth=1
	xorl	%eax, %eax
	movl	%eax, %esi
	movq	-72(%rbp), %rcx
	movq	-48(%rbp,%rcx,8), %rdi
	callq	_pthread_join
	movl	%eax, -80(%rbp)         ## 4-byte Spill
## %bb.7:                               ##   in Loop: Header=BB0_5 Depth=1
	movq	-72(%rbp), %rax
	addq	$1, %rax
	movq	%rax, -72(%rbp)
	jmp	LBB0_5
LBB0_8:
	movl	-56(%rbp), %esi
	leaq	L_.str(%rip), %rdi
	movb	$0, %al
	callq	_printf
	movq	___stack_chk_guard@GOTPCREL(%rip), %rdi
	movq	(%rdi), %rdi
	movq	-8(%rbp), %rcx
	cmpq	%rcx, %rdi
	movl	%eax, -84(%rbp)         ## 4-byte Spill
	jne	LBB0_10
## %bb.9:
	xorl	%eax, %eax
	addq	$96, %rsp
	popq	%rbp
	retq
LBB0_10:
	callq	___stack_chk_fail
	ud2
	.cfi_endproc
                                        ## -- End function
	.p2align	4, 0x90         ## -- Begin function increment_routine
_increment_routine:                     ## @increment_routine
	.cfi_startproc
## %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movq	%rdi, -8(%rbp)
	movq	-8(%rbp), %rdi
	movq	%rdi, -16(%rbp)
	movl	$0, -20(%rbp)
LBB1_1:                                 ## =>This Inner Loop Header: Depth=1
	cmpl	$1000000, -20(%rbp)     ## imm = 0xF4240
	jge	LBB1_4
## %bb.2:                               ##   in Loop: Header=BB1_1 Depth=1
	movq	-16(%rbp), %rax
	movl	(%rax), %ecx
	addl	$1, %ecx
	movl	%ecx, (%rax)
## %bb.3:                               ##   in Loop: Header=BB1_1 Depth=1
	movl	-20(%rbp), %eax
	addl	$1, %eax
	movl	%eax, -20(%rbp)
	jmp	LBB1_1
LBB1_4:
	xorl	%eax, %eax
                                        ## kill: def $rax killed $eax
	popq	%rbp
	retq
	.cfi_endproc
                                        ## -- End function
	.section	__TEXT,__cstring,cstring_literals
L_.str:                                 ## @.str
	.asciz	"%d\n"


.subsections_via_symbols
