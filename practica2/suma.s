.section .data
lista:		.int 1,2,10, 1,2,0b10, 1,2,0x10
longlista:	.int   (.-lista)/4
resultado:	.int   0
  formato: 	.asciz	"suma = %u = 0x%x hex\n"


.section .text
 main: .global  main

	call trabajar	# subrutina de usuario
	call imprim_C	# printf()  de libC	
	call acabar_C	# exit()    de libC
	

trabajar:
	mov     $lista, %rbx
	mov  longlista, %ecx
	call suma		# == suma(&lista, longlista);
	mov  %eax, resultado
	
suma:
	push     %rdx
	mov  $0, %eax
	mov  $0, %rdx
bucle:
	add  (%rbx,%rdx,4), %eax
	inc   %rdx
	cmp   %rdx,%rcx
	jne    bucle

	pop   %rdx
	ret

imprim_C:			# requiere libC
	mov   $formato, %rdi
	mov   resultado,%esi
	mov   resultado,%edx
	mov          $0,%eax	# varargin sin xmm
	call  printf		# == printf(formato, res, res);
	

acabar_C:			# requiere libC
	mov  resultado, %edi
	call _exit		# ==  exit(resultado)
	
