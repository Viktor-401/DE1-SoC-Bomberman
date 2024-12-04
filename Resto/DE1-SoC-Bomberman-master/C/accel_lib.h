#ifndef accel_lib_h
#define accel_lib_h
#include <stdint.h>
#define DEVID 0x00

#define THRESH_TAP 0x1D
#define OFSX 0x1E
#define OFSY 0x1F
#define OFSZ 0x20
#define DUR 0x21
#define LATENT 0x22
#define WINDOW 0x23
#define THRESH_ACT 0x24
#define THRESH_INACT 0x25
#define TIME_INACT 0x26
#define ACT_INACT_CTL 0x27
#define THRESH_FF 0x28
#define TIME_FF 0x29
#define TAP_AXES 0x2A
#define ACT_TAP_STATUS 0x2B
#define BW_RATE 0x2C
#define POWER_CTL 0x2D
#define INT_ENABLE 0x2E
#define INT_MAP 0x2F
#define INT_SOURCE 0x30
#define DATA_FORMAT 0x31
#define DATA_X0 0x32
#define DATA_X1 0x33
#define DATA_Y0 0x34
#define DATA_Y1 0x35
#define DATA_Z0 0x36
#define DATA_Z1 0x37
#define FIFO_CTL 0x38
#define FIFO_STATUS 0x39

#define I2C0_BASE_ADDR 0xFFC04000 // Endereço base do controlador I2C0
#define I2C0_REG_SIZE 0x1000

/* Definições dos offsets dos registradores do controlador I2C. Esses valores correspondem a endereços
de registradores dentro do bloco I2C que controlam a comunicação entre o FPGA e o acelerômetro.*/

#define I2C0_CON 0x00
#define I2C0_TAR 0x04
#define I2C0_DATA_CMD 0x10
#define I2C0_FS_SCL_HCNT 0x1C
#define I2C0_FS_SCL_LCNT 0x20
#define I2C0_CLR_INTR 0x40
#define I2C0_ENABLE 0x6C
#define I2C0_RXFLR 0x78
#define I2C0_ENABLE_STATUS 0x9C

//int16_t accel_x_offset = 0;

/*Um ponteiro global para os registradores mapeados da interface I2C.
 Ele será usado para acessar o hardware diretamente via mmap.*/
volatile uint32_t *i2c0_regs;

int open_and_map();

// Desfaz o mapeamento de memória e fecha o descritor do arquivo.
void close_and_unmap(int fd);

/*Essas funções escrevem e leem valores dos registradores do I2C. A conversão offset / 4
é necessária porque o volatile uint32_t *base aponta para uma memória de 32 bits,
enquanto os registradores têm offsets em bytes. */
void write_register(volatile uint32_t *base, uint32_t offset, int32_t value);

int32_t read_register(volatile uint32_t *base, uint32_t offset);

// Esta função configura o I2C. Ela habilita o controlador, define a taxa de clock, e define o endereço de destino do acelerômetro (0x53).
void I2C0_init();

// Envia comandos ao acelerômetro escrevendo valores nos registradores via I2C.
void accel_reg_write(uint8_t addressA, int8_t value);

// Lê dados do acelerômetro.
void accel_reg_read(uint8_t addressA, int8_t *value);

// Função de calibração para capturar o valor do eixo X em 180 graus

/*Para calibrar o acelerômetro e definir que a posição em 180 graus será o ponto de referência (0),
 você pode implementar uma função de calibração que armazena o valor lido do eixo X quando a placa
 está na posição de 180 graus e, em seguida, usa esse valor como um offset para ajustar as leituras
 futuras. Aqui está uma abordagem para implementar essa calibração:

    1.Leitura do valor de referência (offset): Quando você calibra o acelerômetro, deve ler o valor
      do eixo X na posição de 180 graus. Esse valor será armazenado como um "offset".

    2.Ajuste das leituras futuras: Para todas as leituras subsequentes do acelerômetro, você subtrai
     o valor do "offset" para que 180 graus seja considerado como 0.*/
void calibrate_accel_x();

// Configura o acelerômetro para o modo de medição com sensibilidade de ±16g e frequência de 200 Hz.
void accel_init();

// Função para obter o valor ajustado do eixo X
int get_calibrated_accel_x();

#endif