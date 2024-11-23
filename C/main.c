#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "config.c"
#include "gpu_lib.h"
#include "accel_lib.h"
#include <fcntl.h>
#include <linux/input.h>
// #include <X11/Xlib.h>

bool sair = false;
bool gameOver = false;
void encerrarJogo()
{
    sair = true;
    raise(SIGTERM);
}

int main()
{
    // Setup
    struct input_event ev;
    create_mapping_memory();
    // Configurar signal para encerrar jogo ao usuario usar Ctrl + C
    signal(SIGINT, encerrarJogo);
    srand(time(NULL)); // seed de aleatoriedade

    // Mapeamento e acesso do /dev/mem para acessar o acelerometro via I2C
    int fd = open_and_map();
    // Verificação dos periféricos da DE1-SoC
    if (fd == -1)
    {
        printf("Erro na inicialização de periféricos.\n");
        return -1;
    }

    // Abrindo o dispositivo de entrada do mouse
    int fdMouse = open("/dev/input/event0", O_RDONLY);
    if (fdMouse == -1)
    {
        perror("Erro ao abrir o dispositivo de entrada");
        return -1;
    }

    set_background_color(0, 0, 0);
    I2C0_init();
    accel_init();
    clear_background();

    int accel_x, accel_y, mouse_x, mouse_y, inputKEY, leftButtonPressed, gamestate, cooldownMovimento = 0;

    inputKEY = read_keys();
    int indexCor = 1;

    Map *map = generateMap(mapLayout);

    Player player1 =
        {
            .HP = DEFAULT_HP,
            .bombs = DEFAULT_BOMBS,
            .power = DEFAULT_POWER,
            .posX = STARTING_P1_X,
            .posY = STARTING_P1_Y,
            .facingDirection = DOWN};

    Player player2 =
        {
            .HP = DEFAULT_HP,
            .bombs = DEFAULT_BOMBS,
            .power = DEFAULT_POWER,
            .posX = STARTING_P2_X,
            .posY = STARTING_P2_Y,
            .facingDirection = UP};
    set_sprite(1, 0, 1, player1.posX * 24, player1.posY * 24);
    set_sprite(2, 1, 1, player2.posX * 24, player2.posY * 24);

    // int countPassWhile = 0;
    while (!sair)
    {

        while (inputKEY == 0)
        {
            inputKEY = read_keys();

            // TODO Game Menu

            if (indexCor < 9)
            {
                indexCor++;
            }
            else
            {
                indexCor = 1;
            }
        }

        Map *map = generateMap(mapLayout);

        Player player1 =
            {
                .HP = DEFAULT_HP,
                .bombs = DEFAULT_BOMBS,
                .power = DEFAULT_POWER,
                .posX = STARTING_P1_X,
                .posY = STARTING_P1_Y,
                .facingDirection = DOWN};

        Player player2 =
            {
                .HP = DEFAULT_HP,
                .bombs = DEFAULT_BOMBS,
                .power = DEFAULT_POWER,
                .posX = STARTING_P2_X,
                .posY = STARTING_P2_Y,
                .facingDirection = UP};

        gamestate = updateGame(map, &player1, &player2);
        while ((gamestate == ONGOING) && !sair)
        {
            // countPassWhile += 1;
            // printf("QNTwhile: %d\n", countPassWhile);
            ImprimirTabuleiro(map);

            // Movimento Player1
            if (cooldownMovimento == COOLDOWN_INPUT)
            {
                // Movimento Player 1
                accel_x = get_calibrated_accel_x(); // Receber inclinação da placa
                accel_y = get_calibrated_accel_y();

                int direction = readInputsP1(accel_x, accel_y);

                // printf("pos %d\n",player1.posX);
                if (movePlayer(*map, &player1, direction))
                {
                    set_sprite(1, 0, 1, player1.posX * 24, player1.posY * 24);
                    cooldownMovimento = 0;
                }

                // Movimento Player 2. Lendo o evento do dispositivo
                if (read(fdMouse, &ev, sizeof(struct input_event)) < sizeof(struct input_event))
                {
                    perror("Erro ao ler o evento");
                    close(fdMouse);
                    return -1;
                }

                // Verificando se o evento é de movimento do mouse (REL_X ou REL_Y)
                if (ev.type == EV_REL)
                {
                    if (ev.code == REL_X)
                    {
                        direction = readInputsP2(0, ev.value);
                    }
                    else if (ev.code == REL_Y)
                    {
                        direction = readInputsP2(1, ev.value);
                    }
                }
                else if (ev.type == EV_KEY)
                {
                    placeBomb(&player2, map);
                    direction = -1;
                }
                else
                {
                    direction = -1;
                }

                if (movePlayer(*map, &player2, direction))
                {
                    set_sprite(2, 1, 1, player2.posX * 24, player2.posY * 24);
                    cooldownMovimento = 0;
                }
            }
            else
            {
                cooldownMovimento++;
            }

            inputKEY = read_keys();
            if (inputKEY == 8)
            {
                Pause();
            }
            else if (inputKEY == 1)
            {
                placeBomb(&player1, map);
            }

            gamestate = updateGame(map, &player1, &player2);
        }
    }
}

Map *generateMap(int layout[MAP_SIZE][MAP_SIZE])
{

    Map *map = (Map *)malloc(sizeof(Map));
    int i, j;
    for (i = 0; i < MAP_SIZE; i++)
    {
        for (j = 0; j < MAP_SIZE; j++)
        {
            switch (layout[i][j])
            {
            case EMPTY_CELL:
                map->matriz[i][j].type = EMPTY_CELL;
                break;

            case BLOCK:
                map->matriz[i][j].type = BLOCK;
                Block *block1 = &map->matriz[i][j].object.block;
                block1->HP = 1;
                break;

            case BARRIER:
                map->matriz[i][j].type = BARRIER;
                Block *block2 = &map->matriz[i][j].object.block;
                block2->HP = 1;
                break;
            }
        }
    }
    return map;
}
bool takeDamage(Cell *cell, Player player, Map *map);

void verifyDamagePlayer(Player player, int x, int y)
{
    if ((x == player.posX) && (y == player.posY))
    {
        printf("MORREU\n");
        player.HP -= 1;
    }
}

bool explode(Player player, Bomb *bomb, Map *map)
{
    if (bomb->timer > 0)
    {
        printf("%d\n", bomb->timer);
        bomb->timer -= 1;
        return false;
    }
    int i = 1;

    while (!takeDamage(&map->matriz[map->x - i][map->y], player, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player, map->x - i, map->y);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x + i][map->y], player, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player, map->x + i, map->y);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y - i], player, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player, map->x, map->y - i);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y + i], player, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player, map->x, map->y + i);
        i++;
    }
    printf("mapx = %d\n", map->x);
    printf("mapy = %d\n", map->y);
    Cell cell = map->matriz[map->x][map->y];
    cell.type = EMPTY_CELL;
    bomb->owner.bombs += 1;

    clear_background();
    return true;
}

bool takeDamage(Cell *cell, Player player, Map *map)
{
    switch (cell->type)
    {
    case BLOCK:
        printf("Error1\n");
        cell->object.block.HP -= 1;
        if (cell->object.block.HP == 0)
        {
            cell->type = EMPTY_CELL;
        }
        return true;
    case BARRIER:
        return true;
    case BOMB:
        printf("Error2\n");
        cell->object.bomb.timer = 0;
        explode(player, &cell->object.bomb, map);
        return true;
    }
    return false;
}

bool movePlayer(Map map, Player *player, int direction)
{
    switch (direction)
    {
    case LEFT:
        if (map.matriz[player->posX - 1][player->posY].type == EMPTY_CELL)
        {
            player->facingDirection = UP;
            player->posX -= 1;
            return true;
        }
    case RIGHT:
        if (map.matriz[player->posX + 1][player->posY].type == EMPTY_CELL)
        {
            player->facingDirection = DOWN;
            player->posX += 1;
            return true;
        }
    case UP:
        if (map.matriz[player->posX][player->posY - 1].type == EMPTY_CELL)
        {
            player->facingDirection = LEFT;
            player->posY -= 1;
            return true;
        }
    case DOWN:
        if (map.matriz[player->posX][player->posY + 1].type == EMPTY_CELL)
        {
            player->facingDirection = RIGHT;
            player->posY += 1;
            return true;
        }
    default:
        break;
    }
    return false;
}

bool placeBomb(Player *player, Map *map)
{
    bool bombPlaced = false;
    if (player->bombs > 0)
    {
        Bomb *bomb = &map->matriz[player->posX][player->posY].object.bomb;
        bomb->timer = BOMB_TIMER;
        bomb->power = player->power;
        bomb->owner = *player;

        map->matriz[player->posX][player->posY].type = BOMB;
        printf("mappx = %d\n", map->x);
        printf("mappy = %d\n", map->y);

        bombPlaced = true;
    }
    if (bombPlaced)
    {
        player->bombs -= 1;
    }
    return bombPlaced;
}

int updateGame(Map *map, Player *player1, Player *player2)
{
    int i, j;
    for (i = 0; i < MAP_SIZE; i++)
    {
        for (j = 0; j < MAP_SIZE; j++)
        {
            map->x = i;
            map->y = j;
            if (map->matriz[i][j].type == BOMB)
            {
                explode(*player1, &map->matriz[i][j].object.bomb, map);
            }
        }
    }

    if (player1->HP < 1 && player2->HP < 1)
        return DRAW;
    else if (player2->HP < 1)
        return P1WINS;
    else if (player1->HP < 1)
        return P2WINS;
    else
        return ONGOING;
}

int readInputsP1(int accel_x, int accel_y)
{
    if (accel_x > INPUT_INCLINACAO)
        return RIGHT;
    else if (accel_x < -INPUT_INCLINACAO)
        return LEFT;

    if (accel_y > INPUT_INCLINACAO)
        return UP;
    else if (accel_y < -INPUT_INCLINACAO)
        return DOWN;

    return -1;
}

int readInputsP2(int eixo, int valor)
{
    if (eixo == 0) // Movimento no eixo Y
    {
        if (valor > INPUT_INCLINACAO_MOUSE)
        {
            return DOWN;
        }
        else if (valor < -INPUT_INCLINACAO_MOUSE)
        {
            return UP;
        }
    }
    else // Movimento no eixo X
    {
        if (valor > INPUT_INCLINACAO_MOUSE)
        {
            return RIGHT;
        }
        else if (valor < -INPUT_INCLINACAO_MOUSE)
        {
            return LEFT;
        }
    }
    return -1;
}

void ImprimirTextMatrix(int matriz[SCREEN_HEIGHT][SCREEN_WIDTH], int indexCor)
{
    int i;
    int j;
    for (i = 0; i < SCREEN_HEIGHT; i++)
    {
        for (j = 0; j < SCREEN_WIDTH; j++)
        {
            if (matriz[i][j] != 0)
            {
                paintBackgroundBlock((i), (j), LISTA_CORES[indexCor]->R, LISTA_CORES[indexCor]->G, LISTA_CORES[indexCor]->B);
            }
        }
    }
}

void Pause()
{
    int indexCor = 1;
    limpaTela(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
    while (1)
    {
        // ImprimirTextMatrix(PauseMatriz, indexCor);
        Delay(0.3);
        if (indexCor < 9)
        {
            indexCor++;
        }
        else
        {
            indexCor = 1;
        }

        if (read_keys() == 8)
        {
            break;
        }
    }
    Delay(0.3);
    limpaTela(0, 0, SCREEN_HEIGHT, SCREEN_WIDTH);
}

void ImprimirTabuleiro(Map *map)
{
    int i;
    int j;
    int color;
    for (i = 0; i < 21; i++)
    {
        for (j = 0; j < 21; j++)
        {
            color = LISTA_CORES[map->matriz[i][j].type]->R << 6 | LISTA_CORES[map->matriz[i][j].type]->G << 3 | LISTA_CORES[map->matriz[i][j].type]->B;
            background_box(i * 3, j * 3, 3, 3, color);
            // paintBackgroundBlock((i), (j), LISTA_CORES[map->matriz[i][j].type]->R, LISTA_CORES[map->matriz[i][j].type]->G, LISTA_CORES[map->matriz[i][j].type]->B);
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
            set_sprite_memory(1, matriz[i][j], j, i);
        }
    }
}

void Delay(float segundos)
{
    // converter segundos para microsegundos
    int microSegundos = 1000000 * segundos;

    // tempo inicial
    clock_t start_time = clock();

    // loop até o Delay necessário
    while (clock() < start_time + microSegundos)
        ;
}