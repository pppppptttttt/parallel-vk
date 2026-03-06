# Домашнее задание 1

## Задание

Написать на C++ функцию сложения двух векторов float* размером 1M элементов.

Реализовать три версии:
- наивная (цикл)
- с интринсиками SSE
- с интринсиками AVX

Отчет: Замерить время (с помощью rdtsc или std::chrono) и объяснить, почему прирост не в 4/8 раз, а меньше.

## Отчет

### Замеры

| Версия | Время     |
|--------|-----------|
| Naive: | 2276876ns |
| SSE:   | 2198887ns |
| AVX:   | 1763069ns |

Прироста между наивным циклом и SSE почти нет, у AVX он незначительный.
Сравним ассемблер и поймем почему.

### Ассемблер `NaiveSum`

```asm
NaiveSum:
.LFB11777:
	.loc 6 49 28
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
.LBB24:
	.loc 6 50 22
	movq	$0, -8(%rbp)
	.loc 6 50 5
	jmp	.L22
.L23:
	.loc 6 51 28
	movq	-8(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v1E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm1
	.loc 6 51 36
	movq	-8(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v2E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm0
	.loc 6 51 30
	vaddss	%xmm0, %xmm1, %xmm0
	.loc 6 51 22
	movq	-8(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_19out_naiveE(%rip), %rax
	vmovss	%xmm0, (%rdx,%rax)
	.loc 6 50 5 discriminator 3
	addq	$1, -8(%rbp)
.L22:
	.loc 6 50 31 discriminator 1
	cmpq	$999999, -8(%rbp)
	jbe	.L23
.LBE24:
	.loc 6 53 1
	nop
	nop
	popq	%rbp
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```

В теле цикла считается адрес, два скалярных  load (`vmovss`),
add (`vaddss`) и скалярный store в `out_naive` (`vmovss`). Прибавляет по одному
`float` за раз как и ожидалось

### Ассемблер `SseSum`

```asm

SseSum:
.LFB11778:
	.loc 6 55 26
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	subq	$8, %rsp
	.loc 6 56 17
	movq	$0, -128(%rbp)
.LBB25:
	.loc 6 57 5
	jmp	.L25
.L29:
.LBB26:
	.loc 6 58 37
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 58 33
	leaq	_ZN12_GLOBAL__N_12v1E(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -104(%rbp)
.LBB27:
.LBB28:
	.file 7 "/usr/lib/gcc/x86_64-linux-gnu/13/include/xmmintrin.h"
	.loc 7 940 23
	movq	-104(%rbp), %rax
	vmovups	(%rax), %xmm0
.LBE28:
.LBE27:
	.loc 6 58 33
	vmovaps	%xmm0, -96(%rbp)
	.loc 6 59 37
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 59 33
	leaq	_ZN12_GLOBAL__N_12v2E(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -112(%rbp)
.LBB29:
.LBB30:
	.loc 7 940 23
	movq	-112(%rbp), %rax
	vmovups	(%rax), %xmm0
.LBE30:
.LBE29:
	.loc 6 59 33
	vmovaps	%xmm0, -80(%rbp)
	vmovaps	-96(%rbp), %xmm0
	vmovaps	%xmm0, -32(%rbp)
	vmovaps	-80(%rbp), %xmm0
	vmovaps	%xmm0, -16(%rbp)
.LBB31:
.LBB32:
	.loc 7 192 45
	vmovaps	-32(%rbp), %xmm0
	vaddps	-16(%rbp), %xmm0, %xmm0
.LBE32:
.LBE31:
	.loc 6 60 31
	vmovaps	%xmm0, -64(%rbp)
	.loc 6 61 31
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 61 22
	leaq	_ZN12_GLOBAL__N_17out_sseE(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -120(%rbp)
	vmovaps	-64(%rbp), %xmm0
	vmovaps	%xmm0, -48(%rbp)
.LBB33:
.LBB34:
	.loc 7 989 20
	vmovaps	-48(%rbp), %xmm0
	movq	-120(%rbp), %rax
	vmovups	%xmm0, (%rax)
	.loc 7 990 1
	nop
.LBE34:
.LBE33:
.LBE26:
	.loc 6 57 30 discriminator 2
	addq	$4, -128(%rbp)
.L25:
	.loc 6 57 14 discriminator 1
	movq	-128(%rbp), %rax
	addq	$4, %rax
	.loc 6 57 18 discriminator 1
	cmpq	$1000000, %rax
	jbe	.L29
.LBE25:
	.loc 6 64 5
	jmp	.L30
.L31:
	.loc 6 65 26
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v1E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm1
	.loc 6 65 34
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v2E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm0
	.loc 6 65 28
	vaddss	%xmm0, %xmm1, %xmm0
	.loc 6 65 20
	movq	-128(%rbp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_17out_sseE(%rip), %rax
	vmovss	%xmm0, (%rdx,%rax)
	.loc 6 64 5 discriminator 2
	addq	$1, -128(%rbp)
.L30:
	.loc 6 64 14 discriminator 1
	cmpq	$999999, -128(%rbp)
	jbe	.L31
	.loc 6 67 1
	nop
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```

Видно, что в цикле оооооочень часто загруженные данные выгружаются в стек и загружаюстя
из него. Сложение, например, выглядит как `vaddps	-16(%rbp), %xmm0, %xmm0`
Из-за этого скорость близка к наивной.

### Ассемблер `AvxSum`

```asm
AvxSum:
.LFB11779:
	.loc 6 69 26
	.cfi_startproc
	endbr64
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset 6, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register 6
	andq	$-32, %rsp
	subq	$104, %rsp
	.loc 6 70 17
	movq	$0, -120(%rsp)
.LBB35:
	.loc 6 71 5
	jmp	.L33
.L37:
.LBB36:
	.loc 6 72 40
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 72 36
	leaq	_ZN12_GLOBAL__N_12v1E(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -96(%rsp)
.LBB37:
.LBB38:
	.file 8 "/usr/lib/gcc/x86_64-linux-gnu/13/include/avxintrin.h"
	.loc 8 905 23
	movq	-96(%rsp), %rax
	vmovups	(%rax), %ymm0
.LBE38:
.LBE37:
	.loc 6 72 36
	vmovaps	%ymm0, -88(%rsp)
	.loc 6 73 40
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 73 36
	leaq	_ZN12_GLOBAL__N_12v2E(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -104(%rsp)
.LBB39:
.LBB40:
	.loc 8 905 23
	movq	-104(%rsp), %rax
	vmovups	(%rax), %ymm0
.LBE40:
.LBE39:
	.loc 6 73 36
	vmovaps	%ymm0, -56(%rsp)
	vmovaps	-88(%rsp), %ymm0
	vmovaps	%ymm0, 40(%rsp)
	vmovaps	-56(%rsp), %ymm0
	vmovaps	%ymm0, 72(%rsp)
.LBB41:
.LBB42:
	.loc 8 149 45
	vmovaps	40(%rsp), %ymm0
	vaddps	72(%rsp), %ymm0, %ymm0
.LBE42:
.LBE41:
	.loc 6 74 34
	vmovaps	%ymm0, -24(%rsp)
	.loc 6 75 34
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	.loc 6 75 25
	leaq	_ZN12_GLOBAL__N_17out_avxE(%rip), %rax
	addq	%rdx, %rax
	movq	%rax, -112(%rsp)
	vmovaps	-24(%rsp), %ymm0
	vmovaps	%ymm0, 8(%rsp)
.LBB43:
.LBB44:
	.loc 8 911 20
	vmovaps	8(%rsp), %ymm0
	movq	-112(%rsp), %rax
	vmovups	%ymm0, (%rax)
	.loc 8 912 1
	nop
.LBE44:
.LBE43:
.LBE36:
	.loc 6 71 30 discriminator 2
	addq	$8, -120(%rsp)
.L33:
	.loc 6 71 14 discriminator 1
	movq	-120(%rsp), %rax
	addq	$8, %rax
	.loc 6 71 18 discriminator 1
	cmpq	$1000000, %rax
	jbe	.L37
.LBE35:
	.loc 6 78 5
	jmp	.L38
.L39:
	.loc 6 79 26
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v1E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm1
	.loc 6 79 34
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_12v2E(%rip), %rax
	vmovss	(%rdx,%rax), %xmm0
	.loc 6 79 28
	vaddss	%xmm0, %xmm1, %xmm0
	.loc 6 79 20
	movq	-120(%rsp), %rax
	leaq	0(,%rax,4), %rdx
	leaq	_ZN12_GLOBAL__N_17out_avxE(%rip), %rax
	vmovss	%xmm0, (%rdx,%rax)
	.loc 6 78 5 discriminator 2
	addq	$1, -120(%rsp)
.L38:
	.loc 6 78 14 discriminator 1
	cmpq	$999999, -120(%rsp)
	jbe	.L39
	.loc 6 81 1
	nop
	nop
	leave
	.cfi_def_cfa 7, 8
	ret
	.cfi_endproc
```

Опять много обращений к стеку, но итераций в два раза меньше. И прирост по сравнению
с SSE не в два раза потому, что у AVX эти обращения дороже.
