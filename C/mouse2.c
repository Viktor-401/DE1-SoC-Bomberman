#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/input.h>
#include <stdio.h>
#include <pthread.h>

const INPUT_INCLINACAO = 65;
const INPUT_INCLINACAO_MOUSE = 20;

// Directions
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

int x,y;

void *thread_function(void *arg);

int main()
{
    pthread_t thread1;
    pthread_create(&thread1, NULL, thread_function, NULL);

    while (1)
    {
        printf("main\n");
    }
    
   
}

int readInputsP2(int player_x, int player_y, int eixo, int valor)
{
    if (eixo == 1) // Movimento no eixo Y
    {
        //printf("Dif Y: %d\n", valor-player_y);
        if (valor-player_y > INPUT_INCLINACAO_MOUSE)
        {
            printf("Down\n");
            return DOWN;
        }
        else if (valor-player_y < INPUT_INCLINACAO_MOUSE)
        {
            printf("UP\n");
            return UP;
        }
    }
    else // Movimento no eixo X
    {
       // printf("Dif X: %d\n", valor-player_x);
        if (valor-player_x > INPUT_INCLINACAO_MOUSE)
        {
            printf("Right\n");
            return RIGHT;
        }
        else if (valor-player_x < INPUT_INCLINACAO_MOUSE)
        {
            printf("Left\n");
            return LEFT;
        }
    }
    return -1;
}

void *thread_function(void *arg) {
    int fd;
    struct input_event ev;
    // struct input_event *pMouse;

    int mouse_x, mouse_y;

    // Abrindo o dispositivo de entrada do mouse
    fd = open("/dev/input/event0", O_RDONLY); // Substitua X pelo número correto do seu dispositivo
    if (fd == -1)
    {
        perror("Erro ao abrir o dispositivo de entrada");
        return -1;
    }

    // pMouse = mmap(NULL, 4098, O_RDWR | O_SYNC, MAP_SHARED,fd,0);
    int SENS = 5;

    while (1)
    {
        // Lendo o evento do dispositivo
        if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event))
        {
            perror("Erro ao ler o evento");
            close(fd);
            return -1;
        }

        // ev = *pMouse;

        // Verificando se o evento é de movimento do mouse (REL_X ou REL_Y)
        if (ev.type == EV_REL)
        {
            // if (event.type == MotionNotify)
            // // Pega a posição do mouse
            // int mouseX = event.xmotion.x;
            // int mouseY = event.xmotion.y;
            
            if (ev.code == REL_X)
            {
                x += ev.value/SENS;
                printf("X: %d Y: %d\n", x, y);
                readInputsP2(100,100,0, x);

            }
            else if (ev.code == REL_Y)
            {
                y += ev.value/SENS;
                printf("X: %d Y: %d\n", x, y);
                readInputsP2(100, 100, 1, y);
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
