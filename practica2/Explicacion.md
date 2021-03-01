#### Alejandro Rubio Martínez

##### 5.1	Sumar N enteros sin signo de 32 bits sobre dos registros de 32 bits usando uno de ellos como acumulador de 		 acarreos (N≈16) 

En primer lugar vamos a cambiar el tamaño de la lista por una de 16 elementos, y que repita 16 veces el número 1. Para ello nos vamos a suma.s y editamos lo siguiente para el archivo media.s:

``````asm
.section .data
lista:		.int 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
longlista:	.int   (.-lista)/4
``````

Ahora tenemos una lista de 16 veces el entero 1. Veamos que obtenemos al ejecutarlo. Para ello vamos a crear el binario ejecutable con gcc usándola orden:

```bash
gcc media.s -o media -no-pie -nostartfiles
```

Al realizar esto y ejecutar obtenemos el siguiente resultado:

![](/home/arubiom/Desktop/git/EC/capturas/1-5.1.png)

Donde podemos observar como obtenemos el resultado esperado.

Tenemos que el valor mínimo para que no se produzco acarreo al sumar 16 veces es el 0x1000 0000. Esto es fácil de ver sumando mentalmente pues en las primeras 15 sumas el resultado que tendremos será 0xf000 0000, el cual todavía está permitido, pero si volvemos a suma 0x1000 0000 el resultado que tenemos sería 0x0001 0000 0000, que ya son más de 32 bits, y entonces truncaría en los 32 primeros bits, quedando de resultado el 0x0000 0000. Podemos ver como esto es cierto experimentalmente haciendo lo siguiente:

Primero cambiamos los valores que queremos sumar en media.s:

```assembly
.section .data
lista:		.int 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000
longlista:	.int   (.-lista)/4
```

Ejecutamos como antes:

![](/home/arubiom/Desktop/git/EC/capturas/2-5.1.png)

Donde vemos que efectivamente el resultado es el esperado.

Sin embargo si el valor que usamos fuera 0x0fff ffff no se produciría acarreo, y esto también es fácil de ver mentalmente pues si sumamos las primeras 15 tenemos el valor 0xefff fff1, que si le sumamos el dato una vez más obtenemos 0xffff fff0, el cual es un valor que no produce acarreo. Veámoslo experimentalmente:

```assembly
.section .data
lista:		.int 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff
longlista:	.int   (.-lista)/4
```

![](/home/arubiom/Desktop/git/EC/capturas/3-5.1.png)

Sin embargo, si pasamos 0xffff fff0 a binario sería 0b1111 1111 1111 1111 1111 1111 1111 0000, que si ya finalmente lo pasamos a decimal sería 4294967280, y sin embargo vemos como se devuelve el valor 240. Vamos a depurarlo paso a paso para ver que está ocurriendo. Para ello primero compilamos indicando al compilador que no optimice el binario para que este y nuestro código sean iguales. Hacemos:

```bash
gcc -g media.s -o media -no-pie -nostartfiles
```

Ahora lanzamos con gdb:

```bash
gdb media
```

Y ya estamos en la interfaz de gdb. Primero colocamos un breakpoint donde nos interese, en mi caso en la línea 28, que es la que coincide con la subrutina bucle. Para ello escribimos break 28 y seguimos ejecutando línea por línea. Al final vemos que todo va como queremos hasta llamar a la función exit. Para evitar este fallo vamos a utilizar printf() de libC y para ello realizamos los siguientes cambios en media.s:

```assembly
.section .text
main: .global main

	call trabajar	# subrutina de usuario
	call imprim_C	# printf()  de libC
	call acabar_C	# exit()    de libC
    ret
```



Y además implementamos las dos funciones imprim_C y acabar_C:

```assembly
imprim_C:			# requiere libC
	mov   $formato, %rdi
	mov   resultado,%esi
	mov   resultado,%edx
	mov          $0,%eax	# varargin sin xmm
	call  printf		# == printf(formato, res, res);
	

acabar_C:			# requiere libC
	mov  resultado, %edi
	call exit		# ==  exit(resultado)
```

Con esto al ejecutar obtenemos:

![](/home/arubiom/Desktop/git/EC/capturas/4-5.1.png)

Que es el resultado que esperábamos obtener. Ya podemos comprobar que nuestro programa es correcto sin necesidad de usar gdb. Veamos ahora que pasa cuando sumamos 16 veces el dato 0x1000 0000:

![](/home/arubiom/Desktop/git/EC/capturas/5-5.1.png)

Seguimos viendo como se pierde el bit de acarreo en este caso. Para solucionarlo lo que vamos a hacer es guardar el acarreo cada vez que ocurra en otro registro y luego los concatenamos y almacenamos en un registro de 64 bits. Para ello necesitamos usar la orden JNC para saber cuando hay que incrementar el acarreo, y para ello vamos a necesitar una nueva etiqueta:

```assembly
suma:
	mov  $0, %eax
	mov  $0, %edx
	mov  $0, %rsi
bucle:
	add  (%rbx,%rsi,4), %eax
	jnc	  incrementos
	inc   %edx
incrementos:
	inc   %rsi
	cmp   %rsi,%rcx
	jne    bucle

	ret
```



También vamos a tener que cambiar el tipo de resultado a .quad para que ocupe 8 bytes y decirle al formato que va a tener ese tamaño. Para ello modificamos los datos:

```assembly
.section .data
lista:		.int 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000
longlista:	.int (.-lista)/4
resultado:	.quad   0
formato: 	.asciz	"suma = %lu = 0x%lx hex\n"
```

Ahora para concatenar los registros EDX:EAX tan solo hacemos movs pero moviendo el offset:

```assembly
mov  %eax, resultado
mov  %edx, resultado+4
```



Finalmente ya para que nos muestre por pantalla el resultado en 64 bits cambiamos los registros de imprim_C:

```assembly
imprim_C:			# requiere libC
	mov   $formato, %rdi
	mov   resultado,%rsi
	mov   resultado,%rdx
	mov          $0,%eax	# varargin sin xmm
	call  printf		# == printf(formato, res, res);
	ret
```

Ya por último se ha podido observar como el compilador muestra un warning "cannot find entry symbol _start" y esto se debe a que -nostartfiles no es necesario ponerlo cuando usamos gcc y definimos el main. Al final probamos el programa:

![](/home/arubiom/Desktop/git/EC/capturas/6-5.1.png)

Efectivamente obtenemos el resultado que queríamos por lo cual ya sabemos que nuestro programa suma con acarreo y sin signo. Ya solo nos falta ver que pasaría para sumar 16 veces el número más grande posible, e 0xffff ffff. Para ello cambiemos los datos:

```assembly
.section .data
lista:		.int 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
longlista:	.int   (.-lista)/4
```

Ahora tan solo probamos el programa:

![](/home/arubiom/Desktop/git/EC/capturas/7-5.1.png)

Efectivamente nuestro programa hace lo esperado.

##### 5.2. Sumar N enteros sin signo de 32 bits sobre dos registros de 32 bits mediante extensión con ceros (N≈16)

Ahora tenemos una solución al problema de la suma con acarreo, pero esto no significa que sea la mejor forma posible. Vamos a hacer ahora uso de la orden ADC para sumar por separado las partes más y menos significativas de los números. Para ello podemos imaginar un número sin signo de 32 bits como uno de 64 bits en el que los 32 de la izquierda son ceros, que será la parte a la que se le sume el acarreo si lo hubiera. Para esto hacemos los siguientes cambios:

```assembly
suma:
	mov  $0, %eax
	mov  $0, %edx
	mov  $0, %rsi
bucle:
	add  (%rbx,%rsi,4), %eax
	adc   $0, %edx
	inc   %rsi
	cmp   %rsi,%rcx
	jne    bucle

	ret
```

Comprobamos ahora si funciona para todos los ejemplos realizados anteriormente:

```assembly
.section .data
lista:		.int 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1
longlista:	.int   (.-lista)/4
```

![](/home/arubiom/Desktop/git/EC/capturas/1-5.2.png)

Para el primer ejemplo (no tiene acarreo) funciona. Veamos el siguiente:

```assembly
.section .data
lista:		.int 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff
longlista:	.int   (.-lista)/4
```

![](/home/arubiom/Desktop/git/EC/capturas/2-5.2.png)

También es correcto. Vamos con el siguiente ejemplo:

```assembly
.section .data
lista:		.int 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000, 0x10000000
longlista:	.int   (.-lista)/4
```

![](/home/arubiom/Desktop/git/EC/capturas/3-5.2.png)

Podemos ver como este método funciona también para sumas que tengan acarreo. Veamos ya por último el ejemplo del máximo número que podemos representar con 32 bits. Este es:

```assembly
.section .data
lista:		.int 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
longlista:	.int   (.-lista)/4
```

![](/home/arubiom/Desktop/git/EC/capturas/4-5.2.png)

Efectivamente esto es correcto para todos los ejemplos.

Como se puede observar, es bastante engorroso pasarle los tests al programa uno a uno. Para ello buscamos una solución, como por ejemplo, dejar comentado los tests y solo descomentar el que queramos pasar. Algo así, haciendo uso de las órdenes .macro y .irpc:

```assembly
.section .data
	.macro linea  
	 	.int 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1  
		#  .int 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff 
		# .int 5000000000, 5000000000, 5000000000, 5000000000
	.endm
lista: .irpc i,1234 
		linea 
		.endr
```

En este caso estamos usando los datos .int 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1 y el resultado no cambiará al de antes:

![](/home/arubiom/Desktop/git/EC/capturas/5-5.2.png)

Pero aún así podríamos mejorar la comodidad a la hora de pasar los tests aún más usando el compilación condicional con cpp. Para esto vamos a diseñar de nuevo los datos, concretamente los valores de la lista:

```assembly
.section .data
#ifndef TEST 
#define TEST 9 
#endif 
	.macro linea  
#if TEST==1 
	.int 1,1,1,1 
#elif TEST==2
	.int 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff 
...
#elif TEST==8 
	.int 5000000000,5000000000,5000000000,5000000000 
#else 
	.error "Definir TEST entre 1..8" 
#endif 
	.endm 
```

Donde se entiende que "..." son otros tests distintos. También aprovechamos para cambiar el formato del printf:

```assembly
formato: 
	.ascii "resultado \t =   %18lu (uns)\n" 
	.ascii   "\t\t = 0x%18lx (hex)\n" 
	.asciz"\t\t = 0x %08x %08x\n"
```

Así obtendremos más información de cada tests. En concreto los tests que vamos a pasar son los siguientes:

```assembly
#if TEST==1 
	.int 1,1,1,1
#elif TEST==2
	.int 0x0fffffff, 0x0fffffff, 0x0fffffff, 0x0fffffff
#elif TEST==3
	.int 0x10000000, 0x10000000, 0x10000000, 0x10000000
#elif TEST==4
	.int 0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff
#elif TEST==5
	.int -1, -1, -1, -1
#elif TEST==6
	.int 200000000, 200000000, 200000000, 200000000
#elif TEST==7
	.int 300000000, 300000000, 300000000, 300000000
#elif TEST==8
	.int 5000000000, 5000000000, 5000000000, 5000000000
```

Pero para ejecutarlos paso a paso necesitamos enviarlos uno a uno al compilador. Para ello utilizamos el siguiente script de bash:

```bash
for i in $(seq 1 9); do
	rm unsigned
	gcc -x assembler-with-cpp -D TEST=$i –no-pie unsignedsum2.s -o unsigned
	printf "__TEST%02d__%35s\n" $i "" | tr " " "-" ; ./unsigned 
done
```

Ahora solo lo ejecutamos y vemos los resultados:

![](/home/arubiom/Desktop/git/EC/capturas/6-5.2.png)

Vemos que los tests del 1 al 6 no hay problema, a excepción del 5, que como era esperado no es el resultado correcto, pues nuestro programa no trabaja con signos.

![](/home/arubiom/Desktop/git/EC/capturas/7-5.2.png)

El test 7 también pasa correctamente. Veamos sin embargo que pasa ahora con el 8:

![](/home/arubiom/Desktop/git/EC/capturas/8-5.2.png)

Para empezar el test 8 nos dice que está truncando los números, pues habíamos puesto un número que no cabía en 32 bits. Luego el resultado está calculado para este resultado.

![](/home/arubiom/Desktop/git/EC/capturas/9-5.2.png)

Vemos como el test 9 no existe y así se indica. Con esto nos quedaría ya listo el programa para sumar dos enteros de 32 bits sin signo. Ahora veamos lo que tenemos que hacer para sumar dos enteros también de 32 bits pero con signo.

##### 5.3. Sumar N enteros con signo de 32 bits sobre dos registros de 32 bits (mediante extensión de signo, naturalmente) (N≈16)

Ahora nuestro problema radica en que nuestro programa no tiene en cuenta el signo del elemento de la lista. Para evitarlo lo que buscamos hacer es primero extender el signo del número en cuestión y luego sumarlo. Para ello antes de sumarlo usamos la orden cltd en nuestro caso que es la que nos sirve para los registros EDX:EAX. También vamos a necesitar nuevos registros para acumular, y para ello usamos EDI y EBP, aunque luego el resultado tiene que seguir quedando en EDX:EAX:

```assembly
suma:
	mov  $0, %edi
	mov  $0, %ebp
	mov  $0, %rsi
bucle:
    mov (%rbx,%rsi,4), %eax
    cltd
	add   %eax, %edi
	adc   %edx, %ebp
	inc   %rsi
	cmp   %rsi,%rcx
	jne    bucle

    mov %edi, %eax
    mov %ebp, %edx

	ret
```



Ahora solo tenemos que especificar en el formato que vamos a trabajar con signos, esto es:

```assembly
.ascii   "resultado \t =   %18ld (sgn)\n"
```

Y ya tan solo le pasamos los tests para ver si funciona correctamente. Los tests que usamos esta vez son:

```assembly
#if TEST==1 
	.int -1 ,-1 ,-1, -1
#elif TEST==2
	.int 0x04000000, 0x04000000, 0x04000000, 0x04000000
#elif TEST==3
	.int 0x08000000, 0x08000000, 0x08000000, 0x08000000
#elif TEST==4
	.int 0x10000000, 0x10000000, 0x10000000, 0x10000000
#elif TEST==5
	.int 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff
#elif TEST==6
	.int 0x80000000, 0x80000000, 0x80000000, 0x80000000
#elif TEST==7
	.int 0xF0000000, 0xF0000000, 0xF0000000, 0xF0000000
#elif TEST==8
	.int 0xF8000000, 0xF8000000, 0xF8000000, 0xF8000000
#elif TEST==9
    .int 0xF7FFFFFF. 0xF7FFFFFF, 0xF7FFFFFF, 0xF7FFFFFF
#elif TEST==10
    .int 100000000, 100000000, 100000000, 100000000
#elif TEST==11
    .int 200000000, 200000000, 200000000, 200000000
#elif TEST==12
    .int 300000000, 300000000, 300000000, 300000000
#elif TEST==13
    .int 2000000000, 2000000000, 2000000000, 2000000000
#elif TEST==14
    .int 3000000000, 3000000000, 3000000000, 3000000000
#elif TEST==15
    .int -100000000, -100000000, -100000000, -100000000
#elif TEST==16
    .int -200000000, -200000000, -200000000, -200000000
#elif TEST==17
    .int -300000000, -200000000, -200000000, -200000000
#elif TEST==18
    .int -2000000000, -2000000000, -2000000000, -2000000000
#elif TEST==19
    .int -3000000000, -3000000000, -3000000000, -3000000000
```

Realizando pequeñas modificaciones en el script de ejecutar los tests:

```bash
for i in $(seq 1 19); do
	rm signed;
	gcc -x assembler-with-cpp -D TEST=$i signedsum.s -no-pie -o signed;
	printf "__TEST%02d__%35s\n" $i "" | tr " " "-" ; ./signed;
done
```

Ejecutamos e interpretemos los resultados:

![](/home/arubiom/Desktop/git/EC/capturas/1-5.3.png)

Vemos como los tests del 1 al 6 el resultado es correcto.

![](/home/arubiom/Desktop/git/EC/capturas/2-5.3.png)

De igual manera los tests 7 a 12 están bien.

![](/home/arubiom/Desktop/git/EC/capturas/3-5.3.png)

Ya aquí podemos observar que el TEST14 su resultado es erróneo, puesto que estábamos sumando positivos y obtenemos un negativo. Esto se debe a que nos hemos desbordado y en la suma hemos pasado el número más grande que podíamos representar.

![](/home/arubiom/Desktop/git/EC/capturas/4-5.3.png)

El TEST19 como vemos no hace lo que esperábamos puesto que para empezar los números introducidos no caben en un registro de 32 bits, por lo que el compilador los trunca para trabajar con ellos.

##### 5.4. Media y resto de N enteros con signo de 32 bits calculada usando registros de 32 bits (N≈16)

Ahora lo que buscamos no es obtener el resultado de la suma de la lista, sino su media y su resto al dividir por la longitud. Para ello usamos la orden IDIV, que divide el contenido de los registros EDX:EAX entre el número que elijamos, y almacena el cociente en EAX y el resto en EDX. Así tenemos un dividendo de 64 bits y un resto y cociente de 32 bits.

Dejamos de la siguiente forma la subrutina suma:

```assembly
suma:
	mov  $0, %edi
	mov  $0, %ebp
	mov  $0, %rsi
bucle:
    mov (%rbx,%rsi,4), %eax
    cltd
	add   %eax, %edi
	adc   %edx, %ebp
	inc   %rsi
	cmp   %rsi,%rcx
	jne    bucle

    mov %edi, %eax
    mov %ebp, %edx

    idiv %ecx

	ret
```

Y claro está ahora hay que tener cuidado en donde guardamos los resultados entonces declaramos dos variables nuevas resto y media:

```assembly
media:	.int   0
resto:  .int   0
```

Y cambiamos el formato para que salga por pantalla como queremos:

```assembly
formato:    .asciz "Media = %d = 0x%x hex\n Resto = %d = 0x%x hex\n"
```

Además tendremos que cambiar el imprimir para que los registros que se muestren por pantalla sean los adecuados:

```assembly
imprim_C:			# requiere libC
	mov $formato, %rdi  # argumentos
    mov media, %esi # argumentos
    mov media, %edx # argumentos
    mov resto, %ecx
    mov resto, %r8
    mov $0, %eax
    call printf
    ret

acabar_C:			# requiere libC
	mov  media, %edi
	call exit		# ==  exit(resultado)
	ret

```

Así que así ya tenemos el programa funcional que calcula la media y el resto. Ahora comprobemos que funciona pasándole los siguientes tests:

```assembly
#if TEST==1
    .int 1,2,1,2
#elif TEST==2
    .int -1,-2,-1,-2
#elif TEST==3
    .int 0x7fffffff,0x7fffffff,0x7fffffff,0x7fffffff
#elif TEST==4
    .int 0x80000000,0x80000000,0x80000000,0x80000000
#elif TEST==5
    .int 0xffffffff,0xffffffff,0xffffffff,0xffffffff
#elif TEST==6
    .int 2000000000,2000000000,2000000000,2000000000
#elif TEST==7
    .int 3000000000,3000000000,3000000000,3000000000
#elif TEST==8
    .int -2000000000,-2000000000,-2000000000,-2000000000
#elif TEST==9
    .int -3000000000,-3000000000,-3000000000,-3000000000
#elif TEST>=10 && TEST <=14
    .int 1,1,1,1
#elif TEST >= 15 && TEST<=19
    .int -1,-1,-1,-1
#else
    .error "Definir test"
#endif
    .endm

    .macro linea0
#if TEST>=1 && TEST<=9
    linea
#elif TEST==10
    .int 0,2,1,1
#elif TEST==11
    .int 1,2,1,1
#elif TEST==12
    .int 8,2,1,1
#elif TEST==13
    .int 15,2,1,1
#elif TEST==14
    .int 16,2,1,1
#elif TEST==15
    .int 0,-2,-1,-1
#elif TEST==16
    .int -1,-2,-1,-1
#elif TEST==17
    .int -8,-2,-1,-1
#elif TEST==18
    .int -15,-2,-1,-1
#elif TEST==19
    .int -16,-2,-1,-1
#else
    .error "Definir test"
#endif
    .endm



lista:	    linea0 
        .irpc i,123
            linea
        .endr
```

Utilizando dos macros en este caso para definir los tests de la forma más cómoda posible. Vamos a ejecutar el programa con la script modificada siguiente:

```bash
for i in $(seq 1 19); do
	rm media;
	gcc -x assembler-with-cpp -D TEST=$i media.s -no-pie -o media;
	printf "__TEST%02d__%35s\n" $i "" | tr " " "-" ; ./media;
done

```



Veamos los resultados ahora de los tests:

![](/home/arubiom/Desktop/git/EC/capturas/1-5.4.png)

Aquí vemos como todos los resultados de los tests son lo que esperábamos menos el test 7, esto debido a que ha habido desbordamiento al sumar, puesto que los valores introducidos son muy grandes. A ver ahora si los siguientes tests también son correctos:

![](/home/arubiom/Desktop/git/EC/capturas/2-5.4.png)

Ahora vemos para empezar que los valores introducidos en el test 9 son demasiado grandes para guardarlos en 32 bits así que el programa los trunca a enteros positivos a pesar de ser negativos, lo cual es el motivo por el que obtenemos esa media. Veamos lo siguiente tests que en principio no debería haber ningún problema:

![](/home/arubiom/Desktop/git/EC/capturas/3-5.4.png)

![](/home/arubiom/Desktop/git/EC/capturas/4-5.4.png)

Vemos como el resto de tests son correctos por lo que podemos asumir que nuestro programa es correcto.

Ahora vamos a discutir el signo del módulo (resto). Si buscamos en wikipedia la definición de división truncada encontramos lo siguiente:

"Many implementations use *truncated division*, where the quotient is defined by  truncation  $q = trunc(a/n)$ and thus according to equation the remainder would have *same sign as the dividend*.  The quotient is rounded towards zero: equal to the first integer in the direction of zero from the exact rational quotient.

$r=a-n\cdot trunc(a/n)$"

De donde podemos extraer básicamente que el signo del resto coincidirá con el del dividendo. Esto es, EDX conservará los bits más significativos si son 0 ó f.