#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "config.c"
#include "gpu_lib.h"
#include "accel_lib.h"


void desenhar_quadrado_na_gpu(unsigned long sprite_slot) {
    // Matriz 20x20 representando o sprite (valores de 0 a 5)
    unsigned long matriz_sprite[20][20] = {
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1},
        {1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    };

    // Tabela de cores para os números da matriz
    int tabela_cores[6] = {
    0b000000000,   // 0: Preto ou transparente
    0b111000000, // 2: Vermelho
    0b000000111, // 1: Azul
    0b110011001, // 3: Verde
    0b010101101, // 4: Amarelo
    0b001010110  // 5: Roxo
    };


    // Enviar os pixels para a GPU
    int y =0;
    int x = 0;
    for (y = 0; y < 20; y++) {
        for (x = 0; x < 20; x++) {
            set_sprite_memory(1, 0b110000001, x, y);
        }
    }
}

void LoadSprite(int spriteSlot, int matriz[20][20])
{
    int i, j;
    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 20; j++)
        {
            set_sprite_memory(spriteSlot, matriz[j][i], i, j);
        }
    }
}

int main() {
    unsigned long sprite_slot = 2;  // Usaremos o slot 0 para o sprite
    create_mapping_memory();
    clear_all();
    desenhar_quadrado_na_gpu(sprite_slot);
    LoadSprite(2, player2Sprite);
    set_sprite(2,2,1,40,40);
    
    return 0;
}
