#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "config.c"
#include "gpu_lib.h"
#include "accel_lib.h"
#include <fcntl.h>
#include <linux/input.h>

bool exitGame = false;
void encerrarJogo()
{
    exitGame = true;
    raise(SIGTERM);
}

Map *generateMap(int layout[MAP_SIZE][MAP_SIZE]);
void verifyDamagePlayer(Player *player, int x, int y);
bool explode(Player *player1, Player *player2, Bomb *bomb, Map *map);
bool takeDamage(Cell *cell, Player *player1, Player *player2, Map *map);
bool movePlayer(Map map, Player *player, int direction);
bool placeBomb(Player *player, Map *map);
int updateGame(Map *map, Player *player1, Player *player2);
int readInputsP1(int accel_x, int accel_y);
int readInputsP2(int player_x, int player_y, int eixo, int valor);
void ImprimirTextMatrix(int matriz[SCREEN_HEIGHT][SCREEN_WIDTH], int indexCor);
void Pause();
void ImprimirTabuleiro(Map *map);
void LoadSprite(int spriteSlot, int matriz[20][20]);
void Delay(float segundos);
Map* GameSetup(Map *map, Player *player1, Player *player2);
void GameMenu();
int InitDevices(int *fdMouse);
int GetDirections(Mouse *mouse, Player *player1, Player *player2,int *directionP1, int *directionP2);
int Movement(int *cooldownMovement, Mouse *mouse, Player *player1, Player *player2, Map map);
void InGameActions(int fdMouse, Player *player1, Player *player2, Map *map);
void ProgramActions(bool *exitGame);

int main()
{
    // Setup
    signal(SIGINT, encerrarJogo);
    srand(time(NULL)); // seed de aleatoriedade

    struct input_event ev;
    int fdMouse;
    if (InitDevices(&fdMouse) == -1)
    {
        return -1;
    }

    int gamestate, cooldownMovimento = 0;

    Map *map;
    Player player1, player2;
    Mouse mouse = {500,400,fdMouse};
    //GameSetup(&map, &player1, &player2);

    LoadSprite(1, player);
    LoadSprite(2, player);

    // int countPassWhile = 0;
    while (!exitGame)
    {   
        Imprimircoisas2x(10, 7, bonecoBlock); // Conferir o local dessa tela, pode ser antes.
        GameMenu();
        map = GameSetup(map, &player1, &player2);
        mouse.x = 456;
        mouse.y = 456;
        gamestate = updateGame(map, &player1, &player2);
        while ((gamestate == ONGOING) && !exitGame)
        {
            // printf("P1: HP=%d Bombs=%d X=%d Y=%d\n", player1.HP, player1.bombs, player1.posX, player1.posY);
            // printf("P2: HP=%d Bombs=%d X=%d Y=%d\n", player2.HP, player2.bombs, player2.posX, player2.posY);
            // countPassWhile += 1;
            // printf("QNTwhile: %d\n", countPassWhile);
            // printf("ONGOING\n");     
            ImprimirTabuleiro(map);
            printf("Mouse X: %d Y: %d\n", mouse.x, mouse.y);
            set_polygon(0,0007, 1, 1, mouse.y, mouse.x);
            // printf("CD: %d\n", cooldownMovimento);
            if (Movement(&cooldownMovimento, &mouse, &player1, &player2, *map) == -1)
            {
                return -1;
            }
            
            InGameActions(mouse.fdMouse, &player1, &player2, map);
            ProgramActions(&exitGame);

            gamestate = updateGame(map, &player1, &player2);
            switch (gamestate)
            {
            case P1WINS:
                printf("P1 Wins\n");
                break;
            case P2WINS:
                printf("P2 Wins\n");
                break;
            case DRAW:
                printf("Draw\n");
                break;
            }
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

void verifyDamagePlayer(Player *player, int x, int y)
{
    if ((x == player->posX) && (y == player->posY))
    {
        // printf("\n"); //NÃO RETIRE ESSE PRINT
        player->HP -= 1;
    }
}

bool explode(Player *player1, Player *player2, Bomb *bomb, Map *map)
{
    if (bomb->timer > 0)
    {
        // printf("TESTE\n");
        // printf("%d\n", bomb->timer);
        bomb->timer -= 1;
        return false;
    }
    int i = 1;

    verifyDamagePlayer(player1, map->x, map->y);
    verifyDamagePlayer(player2, map->x, map->y);


    while (!takeDamage(&map->matriz[map->x - i][map->y], player1, player2, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player1, map->x - i, map->y);
        verifyDamagePlayer(player2, map->x - i, map->y);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x + i][map->y], player1, player2, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player1, map->x + i, map->y);
        verifyDamagePlayer(player2, map->x + i, map->y);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y - i], player1, player2, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player1, map->x, map->y - i);
        verifyDamagePlayer(player2, map->x, map->y - i);
        i++;
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y + i], player1, player2, map) && i <= bomb->power)
    {
        verifyDamagePlayer(player1, map->x, map->y + i);
        verifyDamagePlayer(player2, map->x, map->y + i);
        i++;
    }
    // printf("mapx = %d\n", map->x);
    // printf("mapy = %d\n", map->y);
    // Cell cell = map->matriz[map->x][map->y];
    // cell.type = EMPTY_CELL;
    map->matriz[map->x][map->y].type = EMPTY_CELL;
    bomb->owner->bombs += 1;

    clear_background();
    printf("P1: %d\n", player1->HP);
    printf("P2: %d\n", player2->HP);
    return true;
}

bool takeDamage(Cell *cell, Player *player1, Player *player2, Map *map)
{
    switch (cell->type)
    {
    case BLOCK:
        // printf("Error1\n");
        cell->object.block.HP -= 1;
        if (cell->object.block.HP == 0)
        {
            cell->type = EMPTY_CELL;
        }
        return true;
    case BARRIER:
        return true;
    case BOMB:
        // printf("Error2\n");
        cell->object.bomb.timer = 0;
        explode(player1, player2, &cell->object.bomb, map);
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
        bomb->owner = player;

        map->matriz[player->posX][player->posY].type = BOMB;
        // printf("mappx = %d\n", map->x);
        // printf("mappy = %d\n", map->y);

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
                explode(player1, player2, &map->matriz[i][j].object.bomb, map);
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

int readInputsP2(int player_x, int player_y, int eixo, int valor)
{
    if (eixo == 1) // Movimento no eixo Y
    {
        //printf("Dif Y: %d\n", valor-player_y);
        if (valor-(player_y*24) > INPUT_INCLINACAO_MOUSE)
        {
            printf("Down\n");
            return DOWN;
        }
        else if (valor-(player_y*24) < INPUT_INCLINACAO_MOUSE)
        {
            printf("UP\n");
            return UP;
        }
    }
    else // Movimento no eixo X
    {
    //    printf("Dif X: %d\n", valor-player_x);
        if (valor-(player_x*24) > INPUT_INCLINACAO_MOUSE)
        {
            printf("Right\n");
            return RIGHT;
        }
        else if (valor-(player_x*24) < INPUT_INCLINACAO_MOUSE)
        {
            printf("Left\n");
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
                //paintBackgroundBlock((i), (j), LISTA_CORES[indexCor]->R, LISTA_CORES[indexCor]->G, LISTA_CORES[indexCor]->B);
            }
        }
    }
}

void Pause()
{
    clear_all();
    while (1)
    {
        // TODO Tela de pause
        Delay(0.3);
        if (read_keys() == 8)
        {
            break;
        }
    }
    Delay(0.3);
    clear_all();
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
void Imprimircoisas2x(int x, int y, int matriz[20][20])
{
    int i;
    int j;
    int color;

    for (i = x; i < 20; i++)
    {
        for (j = y; j < 20; j++)
        {

            const Cor *cor = LISTA_CORES[matriz[i][j]];

            color = (cor->R << 6) | (cor->G << 3) | cor->B;

            background_box(i * 2, j * 2, 2, 2, color);
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
            set_sprite_memory(spriteSlot, matriz[i][j], i, j);
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

int InitDevices(int *fdMouse)
{
    // Setup
    create_mapping_memory();

    // Mapeamento e acesso do /dev/mem para acessar o acelerometro via I2C
    int fd = open_and_map();
    // Verificação dos periféricos da DE1-SoC
    if (fd == -1)
    {
        printf("Erro na inicialização de periféricos.\n");
        return -1;
    }

    // Abrindo o dispositivo de entrada do mouse
    *fdMouse = open("/dev/input/event0", O_RDONLY);
    if (*fdMouse == -1)
    {
        printf("Erro ao abrir o dispositivo de entrada\n");
        return -1;
    }

    I2C0_init();
    accel_init();

    set_background_color(0, 0, 0);
    clear_background();
}

Map* GameSetup(Map *map, Player *player1, Player *player2)
{
    map = generateMap(mapLayout);
    Player player_1 =
        {
            .HP = DEFAULT_HP,
            .bombs = DEFAULT_BOMBS,
            .power = DEFAULT_POWER,
            .posX = STARTING_P1_X,
            .posY = STARTING_P1_Y,
            .facingDirection = DOWN};
    *player1 = player_1;

    Player player_2 =
    {
        .HP = DEFAULT_HP,
        .bombs = DEFAULT_BOMBS,
        .power = DEFAULT_POWER,
        .posX = STARTING_P2_X,
        .posY = STARTING_P2_Y,
        .facingDirection = UP};

    *player2 = player_2;

    set_sprite(1, 0, 1, player1->posX * 24, player1->posY * 24);
    set_sprite(2, 1, 1, player2->posX * 24, player2->posY * 24);
    
    return map;
}

void GameMenu() // Game Menu
{
    int indexColor = 0;
    Cor colors[3] = {YELLOW, ORANGE, RED};
    int inputKEY = read_keys();
    while (inputKEY == 0)
    {
        inputKEY = read_keys();

        // TODO Game Menu
        //printf("Bomberman!");
    }
}

int GetMouseEvent(int fdMouse, struct input_event *ev)
{
    read(fdMouse, ev, sizeof(struct input_event)) < sizeof(struct input_event);
    return 0;
}

int GetDirections(Mouse *mouse, Player *player1, Player *player2,int *directionP1, int *directionP2)
{
    // printf("PASSOU\n");
    int accel_x = get_calibrated_accel_x(); // Receber inclinação da placa
    int accel_y = get_calibrated_accel_y();

    *directionP1 = readInputsP1(accel_x, accel_y);

    struct input_event ev;
    if (GetMouseEvent(mouse->fdMouse, &ev) != -1)
    {
        if (ev.type == EV_REL)
        {
            if (ev.code == REL_X)
            {   
                int newX = mouse->x + ev.value;  
                if (newX > 480)
                {
                    mouse->x = 480;
                }
                else if (newX < 20)
                {
                    mouse->x = 20;
                }
                else 
                {
                    mouse->x = newX;
                }
                *directionP2 = readInputsP2(player2->posX, player2->posY, 0, mouse->x);
            }
            else if (ev.code == REL_Y)
            {
                int newY = mouse->y + ev.value;  
                if (newY > 480)
                {
                    mouse->y = 480;
                }
                else if (newY < 20)
                {
                    mouse->y = 20;
                }
                else 
                {
                    mouse->y = newY;
                }

                *directionP2 = readInputsP2(player2->posX, player2->posY, 1, mouse->y);
            }
        }
        else
        {
            *directionP2 = -1;
        }
        return 0;
    }
    return -1;
}

int Movement(int *cooldownMovement, Mouse *mouse, Player *player1, Player *player2, Map map)
{
    int directionP1, directionP2;
    GetDirections(mouse, player1, player2, &directionP1, &directionP2);
    if (*cooldownMovement == COOLDOWN_INPUT)
    {
        if (GetDirections(mouse, player1, player2, &directionP1, &directionP2) == 0)
        {
            printf("DIRECTION P1: %d\n",directionP1);
            printf("DIRECTION P2: %d\n",directionP2);
            if (movePlayer(map, player2, directionP2))
            {
                set_sprite(2, 1, 1, player2->posX * 24, player2->posY * 24);
                *cooldownMovement = 0;
            }
            if (movePlayer(map, player1, directionP1))
            {
                set_sprite(1, 0, 1, player1->posX * 24, player1->posY * 24);
                *cooldownMovement = 0;
            }
        }
        else
        {
            return -1;
        }
    }
    else
    {
        // printf("PASSOU3\n");
        (*cooldownMovement)++;
    }
}

void InGameActions(int fdMouse, Player *player1, Player *player2, Map *map)
{
    struct input_event ev;
    if (GetMouseEvent(fdMouse, &ev) != -1 && ev.type == EV_KEY)
    {
        placeBomb(player2, map);
    }
    if (read_keys() == 1)
    {
        placeBomb(player1, map);
    }        
}

void ProgramActions(bool *exitGame)
{
    int key = read_keys();
    switch (key)
    {
    case 8:
        Pause();
        break;

    case 4:
        *exitGame = true;
        break;
    }
}