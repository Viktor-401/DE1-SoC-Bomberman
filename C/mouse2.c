#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>

int main() {
    int fd;
    struct input_event ev;

    // Abrindo o dispositivo de entrada do mouse
    fd = open("/dev/input/eventX", O_RDONLY);  // Substitua X pelo número correto do seu dispositivo
    if (fd == -1) {
        perror("Erro ao abrir o dispositivo de entrada");
        return -1;
    }

    while (1) {
        // Lendo o evento do dispositivo
        if (read(fd, &ev, sizeof(struct input_event)) < sizeof(struct input_event)) {
            perror("Erro ao ler o evento");
            close(fd);
            return -1;
        }

        // Verificando se o evento é de movimento do mouse (REL_X ou REL_Y)
        if (ev.type == EV_REL) {
            if (ev.code == REL_X) {
                printf("Movimento no eixo X: %d\n", ev.value);
            } else if (ev.code == REL_Y) {
                printf("Movimento no eixo Y: %d\n", ev.value);
            }
        }
    }

    close(fd);
    return 0;
}
