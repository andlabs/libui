# 22 october 2017
# clang -o writewidths writewidths.c writewidths.s -g -Wall -Wextra -pedantic -g
# thanks to:
# - http://www.idryman.org/blog/2014/12/02/writing-64-bit-assembly-on-mac-os-x/
# - https://developer.apple.com/library/content/documentation/DeveloperTools/Reference/Assembler/060-i386_Addressing_Modes_and_Assembler_Instructions/i386_intructions.html#//apple_ref/doc/uid/TP30000825-TPXREF101
# - https://stackoverflow.com/questions/46309041/trivial-macos-assembly-64-bit-program-has-incorrect-stack-alignment
# - https://www.google.com/search?q=macos+implement+main+in+assembly+-nasm&oq=macos+implement+main+in+assembly+-nasm&gs_l=psy-ab.3...12877.13839.0.13988.6.6.0.0.0.0.117.407.4j1.5.0....0...1.1.64.psy-ab..1.0.0....0.et6MkokjvwA
# - https://stackoverflow.com/questions/2529185/what-are-cfi-directives-in-gnu-assembler-gas-used-for

.section __DATA,__data

double10:
	.quad	0x4024000000000000
doubleNeg05:
	.quad	0xbfe0000000000000

fmt:
	.asciz	"unregistered OS2 %ld:\tfloat64as(%g, 0x%016lx)\n"

.section __TEXT,__text
.globl _realMain
_realMain:
	pushq		%rbp
	movq		%rsp, %rbp
	addq			$8, %rsp

	xorq			%rcx, %rcx
loop:
	pushq		%rcx
	# the code from core text
	movzwl		%cx, %ecx
	xorps		%xmm0, %xmm0
	cvtsi2sdl		%ecx, %xmm0
	divsd		double10(%rip), %xmm0
	addsd		doubleNeg05(%rip), %xmm0
	# end core text code
	popq		%rcx
	pushq		%rcx
	movd		%xmm0, %rdx
	movzwq		%cx, %rsi
	leaq			fmt(%rip), %rdi
	callq			_printf
	popq		%rcx
	incw			%cx
	cmpw		$10, %cx
	jbe			loop

	xorq			%rax, %rax
	subq			$8, %rsp
	popq		%rbp
	ret
