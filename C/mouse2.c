#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <linux/input.h>

const INPUT_INCLINACAO = 65;
const INPUT_INCLINACAO_MOUSE = 5;

// Directions
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int main()
{
    int fd;
    struct input_event ev;

    int mouse_x, mouse_y;

    // Abrindo o dispositivo de entrada do mouse
    fd = open("/dev/input/event6", O_RDONLY); // Substitua X pelo número correto do seu dispositivo
    if (fd == -1)
    {
        perror("Erro ao abrir o dispositivo de entrada");
        return -1;
    }

    while (1)
    {
        // Lendo o evento do dispositivo
        if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event))
        {
            perror("Erro ao ler o evento");
            close(fd);
            return -1;
        }

        // Verificando se o evento é de movimento do mouse (REL_X ou REL_Y)
        if (ev.type == EV_REL)
        {
            if (ev.code == REL_X)
            {
                readInputsP2(0, ev.value);
            }
            else if (ev.code == REL_Y)
            {
                readInputsP2(1, ev.value);
            }
        }
        else if (ev.type == EV_KEY)
        {
            printf("Olá\n");
        }
    }
    close(fd);
    return 0;
}

int readInputsP2(int eixo, int valor)
{
    if (eixo == 1) // Movimento no eixo Y
    {
        if (valor > INPUT_INCLINACAO_MOUSE)
        {
            printf("Down\n");
            return DOWN;
        }
        else if (valor < -INPUT_INCLINACAO_MOUSE)
        {
            printf("UP\n");
            return UP;
        }
    }
    else // Movimento no eixo X
    {
        if (valor > INPUT_INCLINACAO_MOUSE)
        {
            printf("Right\n");
            return RIGHT;
        }
        else if (valor < -INPUT_INCLINACAO_MOUSE)
        {
            printf("Left\n");
            return LEFT;
        }
    }
    return -1;
}