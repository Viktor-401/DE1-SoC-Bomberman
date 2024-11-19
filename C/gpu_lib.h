#ifndef gpu_lib_H
#define gpu_lib_H

// Barramentos da GPU
#define DATA_A 0X80
#define DATA_B 0X70
#define RESET_PULSECOUNTER 0X90
#define SCREEN 0XA0
#define WRFULL 0XB0
#define WRREG 0XC0

// Mapeamento de memória

#define ALT_LWFPGASLVS_OFST 0xff200000
#define HW_REGS_BASE 0xfc000000
#define HW_REGS_SPAN 0x04000000
#define HW_REGS_MASK (HW_REGS_SPAN - 1) // 0x3FFFFFF

/* Atualiza a cor de um pixel do sprite na memória da gpu.
Sprite slot: Inteiro de 0 até 31
Cor: Número octal de 3 dígitos(em decimal, de 0 até 511)
X e Y: Inteiro de 0 a 19
void set_sprite_memory(unsigned long sprite_slot, unsigned long cor,
     unsigned long x, unsigned long y); */
void set_sprite_memory(unsigned long sprite_slot, unsigned long cor, unsigned long x, unsigned long y);

/*Id : id do sprite a ser desativado. -1 desativa todos os sprites*/
void clear_sprite(unsigned long id);

/*Id : id do poligono a ser desativado. -1 desativa todos os poligonos*/
void clear_polygon(unsigned long id);

/*Limpa qualquer elemento na tela e reseta todos os blocos do background
para a cor padrão do background*/
void clear_all();

/*Mostra um sprite na tela:
id: id do sprite para ser referenciado em outras funções. 1 <= id <=31
sprite_image: imagem associada a esse id. 0 <= sprite_image <=31
ativado: 0 = desativado, 1 = ativado
x,y: coordenada x,y do sprite na tela*/
void set_sprite(unsigned long id, unsigned long sprite_image, unsigned long ativado,
                unsigned long x, unsigned long y);

/*Define uma cor padrão para todos os blocos do background
Cada elemento de cor deve ser um inteiro >= 0 e <= 7*/
void set_background_color(unsigned long vermelho, unsigned long verde,
                          unsigned long azul);

/*Define a cor de um bloco de background especifico
linha, coluna: posição do bloco na tela
RGB: Cada elemento de cor deve ser um inteiro >= 0 e <= 7*/
void set_background_block(unsigned long linha, unsigned long coluna, unsigned long vermelho,
                          unsigned long verde, unsigned long azul);

/*
Desenha um retângulo usando os background blocks.
A coordenada do retângulo é referente ao pixel do canto superior esquerdo.
Cor deve ser dado por um octal de 3 digitos. Ex: O401. R=4, G=0, B=1.
void background_box(unsigned long x, unsigned long y,
unsigned long largura, unsigned long altura, unsigned long cor)
*/
void background_box(unsigned long x, unsigned long y,
                    unsigned long largura, unsigned long altura, unsigned long cor);

/*Reseta todos os blocos do background
para a cor padrão do background*/
void clear_background();

/*Mostra um poligono na tela:
id: id do poligono para ser referenciado em outras funções. 0 <= id <=14
cor: deve ser dado por um octal de 3 digitos. Ex: O401. R=4, G=0, B=1.
forma: 0 = quadrado, 1 = triângulo
tamanho: um inteiro >= 0 e <= 15. tamanho = 0 significa poligono desabilitado
x,y: coordenada x,y do poligono na tela*/
void set_polygon(unsigned long id, unsigned long cor, unsigned long forma,
                 unsigned long tamanho, unsigned long x, unsigned long y);

/*Cria o mapeamento para acessar o processador gráfico na fpga*/
int create_mapping_memory();

/*Fecha o mapeamento para acessar o processador gráfico na fpga*/
void close_mapping_memory();

/*Lê o valor dos push-buttons da fpga*/
int read_keys();
#endif