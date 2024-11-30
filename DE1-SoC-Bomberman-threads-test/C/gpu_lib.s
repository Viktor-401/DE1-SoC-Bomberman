	.section .rodata
	.align 2
pathDevMem:
	.ascii	"/dev/mem\000"
	.align 2
lw_alt:
	.word 0xff200				@ ALT_LWFPGASLVS_OFST / 4

	.data
	.align 2
pDevMem:						@ ponteiro para saida do mapeamento
	.zero 4						
	.align 2
fd:								@ file descriptor
	.zero 4

	.text
	.align 2
	.global	create_mapping_memory
	.type	create_mapping_memory, %function
@ int create_mapping_memory()
create_mapping_memory:
	push	{r4-r7, lr}
	ldr r0, =pathDevMem						@ open("/dev/mem" ...
	movw r1, #4098
	movt r1, #16							@ ...(O_RDWR | O_SYNC))
	mov r7, #5								@ syscall: open
	svc #0

	ldr r3, =fd
	str	r0, [r3]							@ guardar fd

	mov r4, r0								@ r4 = fd
	mov r0, #0								@ mmap2(0,
	mov r1, #4096							@ 4096,
	mov r2, #3								@ (PROT_READ | PROT_WRITE),
	mov r3, #1								@ MAP_SHARED, r4 = fd,
	ldr r5, =lw_alt							@ 0xff200)
	ldr r5, [r5]							
	mov r7, #192							@ syscall: mmap2
	svc #0

	ldr r1, =pDevMem
	str r0, [r1]							@ guardar ponteiro

	pop {r4-r7, lr}
	bx lr

	.align 2
	.global close_mapping_memory  
	.type	close_mapping_memory, %function
close_mapping_memory:						
    pop {lr}

    ldr r0, =pDevMem						@ munmap(pDevMem, 

    mov r1, #4096    						@ 4096)
    mov r7, #91                				@ syscall: munmap
    swi #0                      

	cmp r0, #0								@ if(munmap == 0)
	beq munmap_sucesso

	ldr r0, =fd								@ else ...

    mov r7, #6 								@ close(fd)
    swi #0  

munmap_sucesso:
    pop {lr}
    bx lr                 

	.align 2
	.global	send_instruction
	.type	send_instruction, %function
@ void send_instruction(unsigned int dataA, unsigned int dataB)
send_instruction: 
    @ r0 = dataA
    @ r1 = dataB
	push	{lr}
	ldr r3, =pDevMem				@ r3 = endereço do label
	ldr	r3, [r3]                    @ r3 = ponteiro do mapeamento
while_send_instruction:
	ldr	r2, [r3, #0xb0]             @ r2 = valor em WrFull
	cmp	r2, #0                     	@ if (*pWrFull == 0)
	bne	while_send_instruction      @ while (1)
	
	mov	r2, #0
	str	r2, [r3, #0xc0]             @ *pWrReg = 0

	str	r0, [r3, #0x80]             @ *pDataA = dataA

	str	r1, [r3, #0x70]             @ *pDATAB = dataB

	mov	r2, #1
	str	r2, [r3, #0xc0]             @ *pWrReg = 1

	mov	r2, #0
	str	r2, [r3, #0xc0]             @ *pWrReg = 0

	pop	{lr}
	bx	lr
	
	.align 2
    .global	set_background_color
    .type	set_background_color, %function
@ void set_background_color(unsigned long vermelho, 
@ unsigned long verde, unsigned long azul)
set_background_color:
    @ dataA = reg [8:4], opcode[3:0]
	@ dataA = 0 (reg = 0 e opcode = 0)
    @ dataB = azul[8:6], verde[5:3], vermelho[2:0]
    push    {lr}    

    lsl r2, r2, #6          @ r2 = azul
    lsl r1, r1, #3          @ r1 = verde

    orr r1, r1, r2          @ r1 = [azul, verde]
    orr r1, r1, r0          @ r1 = [azul, verde, vermelho]
                            @ r1 = dataB
    mov	r0, #0              @ r0 = dataA

    bl send_instruction      @ send_instruction(dataA, dataB)
    pop     {lr}
	bx lr

	.align 2
    .global	set_background_block
    .type	set_background_block, %function
@ void set_background_block(unsigned long linha, unsigned long coluna, 
@ unsigned long vermelho, unsigned long verde, unsigned long azul)
set_background_block:
	push	{lr}				
	@ dataA = endereco[17:4], opcode[3:0]
	@ endereco = (linha * 80) + coluna
	@ opcode = 2
	@ dataB = azul[8:6], verde[5:3], vermelho[2:0]
	lsl r3, r3, #3		@ r3 = verde	
	orr r3, r3, r2		@ r3 = [verde, vermelho]

	ldr	r2, [sp, #4]	@ r2 = azul
	lsl r2, r2, #6			
	orr r3, r3, r2		@ r3 = [azul, verde, vermelho]	

	mov r2, #80			
	mul r0, r2, r0		@ r0 = linha * 80		
	add r0, r0, r1 		@ r0 = endereco

	lsl r0, #4			
	add r0, r0, #2		@ r0 = dataA

	mov r1, r3			@ r1 = dataB

	bl	send_instruction 		

	pop 	{lr}
    bx lr

	.align 2
    .global	background_box
    .type	background_box, %function
/* 
Desenha um retângulo usando os background blocks.
A coordenada do retângulo é referente ao pixel do canto superior esquerdo.
Cor deve ser dado por um octal de 3 digitos. Ex: O401. R=4, G=0, B=1.
void background_box(unsigned long x, unsigned long y, 
unsigned long largura, unsigned long altura, unsigned long cor)
*/
background_box:
	push {r4-r7,lr}				

	add r6, r0, r2					@ x + largura = x final
	add r7, r1, r3					@ y + altura = y final

	ldr r5, [sp, #20]				@ r5 = cor

	mov r4, r0 						@ r4 = x
	mov r0, r1						@ r0 = y
	mov r1, r4 						@ r1 = x
	@ máscaras para extrair cada componente da cor		
	lsr r2, r5, #6					@ r2 = vermelho
	and r4, r5, #0b000111000 			
	lsr r3, r4, #3  				@ r3 = verde
	and r4, r5, #0b000000111 		    	@ r4 = azul
	push {r1}					@ salva argumentos na stack
	push {r2}
	push {r3}
	push {r4}

	mov r4, r0						@ contadores: i, j
	mov r5, r1						@ i = x, j = y

loop_background_box:
									@ set_background_block(i,j,vermelho, verde, azul);
	ldr r2, [sp, #8]				
	ldr r3, [sp, #4]
	mov r0, r4
	mov r1, r5
	bl set_background_block	
									@ for (j = x; j < x+largura; j++)
	add r5, r5, #1					@ j ++
	cmp r5, r6						@ j < x + largura
	bne loop_background_box
									@ for (i = y; i < y+altura; i++)
	add r4, r4, #1					@ i ++
	ldr r5, [sp, #12]				@ j = x
	cmp r4, r7						@ i < y + altura
	bne loop_background_box

	pop {r4}
	pop {r3}
	pop {r2}
	pop {r1}
	pop {r4-r7, lr}
    bx lr

/* Atualiza a cor de um pixel do sprite na memória da gpu.
Sprite slot: Inteiro de 0 até 31
Cor: Número octal de 3 dígitos(em decimal, de 0 até 511)
X e Y: Inteiro de 0 a 19
void set_sprite_memory(unsigned long sprite_slot, unsigned long cor,
	 unsigned long x, unsigned long y); */
	.align 2
    .global	set_sprite_memory
    .type	set_sprite_memory, %function
set_sprite_memory:
	push {r4, lr}				

	ldr r4, =400					@ tamanho do sprite = 400 palavras
	mul r0, r0, r4					@ r0 = deslocamento do sprite na memória

	mov r4, #20						@ largura do sprite = 20 palavras
	mul r3, r3, r4			
	add r3, r2, r3					@ r3 = deslocamento do pixel em relação ao sprite

	add r0, r0, r3					@ r0 = deslocamento total
	lsl r0, r0, #4					@ shift para adicionar opcode e enviar instrução pelo dataA
	add r0, r0, #1 					@ opcode = 1 
									@ r0 = dataA

	@ máscaras para extrair cada componente da cor 000 000 111		
	and r3, r1, #0b000111000 			@ r3 = [verde,000] = 000 000 000 
	and r4, r1, #0b000000111 		 	@ r4 = [000,000,azul] = 000 000 111
	lsr r1, r1, #6					@ r1 = vermelho = 000 000 000 
	lsl r4, r4, #6					@ r4 = [azul,000,000] = 111 000 000 	

	orr r1, r1, r3					@ r1 = [azul, verde, vermelho] 
	orr r1, r1, r4					@ r1 = dataB
										
	bl send_instruction

	pop {r4, lr}
	bx lr

	.align 2
    .global	set_sprite
    .type	set_sprite, %function
@void set_sprite(unsigned long id, unsigned long sprite_image,
@   unsigned long ativado, unsigned long x, unsigned long y)
set_sprite:
	push	{lr}				

    @ dataA = id[8:4], opcode[3:0]
    @ dataB = ativado[29], x[28:19], y[18:9], sprite_image[8:0]
    @ opcode = 0

    lsl r0, r0, #4      @ r0 = dataA  

    lsl r2, r2, #29     @ r2 = ativado
    lsl r3, r3, #19     @ r3 = x

    orr r1, r1, r2      @ r1 = sprite_image
    orr r1, r1, r3      @ dataB falta y

    ldr	r2, [sp, #4]	@ r2 = y
    lsl r2,r2, #9  
    orr r1, r1, r2      @ r1 = dataB
          
    bl send_instruction      

    pop     {lr}
    bx lr

	.align 2
    .global	clear_sprite
    .type	clear_sprite, %function
/* Id : id do poligono a ser desativado. -1 desativa todos os poligonos
void clear_sprite(unsigned long id);*/
clear_sprite:
	push	{r4, lr}		
	@ r0 = id
	@ r4 = contador

	mov r4, #1					@ i = 1

	cmp r0, #-1					@ if(id == -1)
	beq clear_all_sprite		
								@ else
	mov r1, #0					@ set_sprite(id = r0, sprite_image = 0,
	mov r2, #0					@ ativado = 0,
	mov r3, #0					@ x = 0,
	push	{r2}				@ y =0)
	bl set_sprite
	pop 	{r2}

	bl exit_sprite

clear_all_sprite:				@ then
	mov r0, r4

	mov r1, #0					@ set_sprite(id = r0, sprite_image = 0,
	mov r2, #0					@ ativado = 0,
	mov r3, #0					@ x = 0,
	push	{r2}				@ x = 0, y =0)
	bl set_sprite
	pop 	{r2}

	add r4, #1

	cmp r4, #32					@ if(id == 15)
	bne clear_all_sprite		@ break;

exit_sprite:
    pop     {r4, lr}
    bx lr


	.align 2
    .global	set_polygon
    .type	set_polygon, %function
@ void set_polygon(unsigned long id, unsigned long cor, unsigned long forma,
@   unsigned long tamanho, unsigned long x, unsigned long y)
set_polygon:
	push	{lr}		
    @ dataA = id[7:4], opcode[3:0]
    @ dataB = Forma[31], BGR[30:22], Tamanho[21:18], y[17:9], x[8:0]

    lsl r0, r0, #4      @ r0 = dataA
    orr r0, r0, #3      @ opcode = 3

    lsl r1, r1, #22     @ r1 = cor(BGR)
    lsl r2, r2, #31     @ r2 = forma
    lsl r3, r3, #18     @ r3 = tamanho
      
    orr r1, r1, r2      @ r1 = dataB
    orr r1, r1, r3      @ dataB faltando x e y  
    
    ldr	r2, [sp, #8]    @ r2 = x        
    ldr	r3, [sp,#4]	    @ r3 = y

    lsl r3, r3, #9      

    orr r1, r1, r2          
    orr r1, r1, r3      @ dataB completo

    bl send_instruction      @ sendInstruction(dataA, dataB)

    pop     {lr}
    bx lr

	.align 2
    .global	clear_polygon
    .type	clear_polygon, %function
/* Id : id do poligono a ser desativado. -1 desativa todos os poligonos
void clear_polygon(unsigned long id);*/
clear_polygon:
	push	{r4, lr}		
	@ r0 = id
	@ r4 = contador

	mov r4, #0

	cmp r0, #-1					@ if(id == -1)
	beq clear_all_polygon		
								@ else
	mov r1, #0					@ set_polygon(id = r0, cor = 0,
	mov r2, #0					@ forma = 0,
	mov r3, #0					@ tamanho = 0,
	push	{r2, r3}			@ x = 0, y =0)
	bl set_polygon
	pop 	{r2, r3}

	bl exit_polygon

clear_all_polygon:				@ then
	mov r0, r4

	mov r1, #0					@ set_polygon(id = r0, cor = 0,
	mov r2, #0					@ forma = 0,
	mov r3, #0					@ tamanho = 0,
	push	{r1, r2, r3}			@ x = 0, y =0)
	bl set_polygon
	pop 	{r1, r2, r3}

	add r4, #1

	cmp r4, #15					@ if(id == 15)
	bne clear_all_polygon		@ break;

exit_polygon:
    pop     {r4, lr}
    bx lr

	.align 2
    .global	clear_background
    .type	clear_background, %function
clear_background:
	push {lr}

	mov r0 , #0
	mov r1 , #0
	mov r2 , #100
	ldr r3 , =0b110111111				@ 0677 = cor transparente
	push {r3}
	mov r3, #60
	bl background_box

	pop {r3,lr}
	bx lr

	.align 2
    .global	clear_all
    .type	clear_all, %function
clear_all:
	push {lr}

	bl clear_background

	mov r0 , #-1
	bl clear_sprite

	mov r0 , #-1
	bl clear_polygon

	pop {lr}
	bx lr

	.align 2
    .global	read_keys
    .type	read_keys, %function
read_keys:
	ldr r1, =pDevMem
	ldr r1, [r1]
	ldr r0, [r1, #0x0]
	mvn r0, r0;
	add r0, r0, #16
	bx lr
