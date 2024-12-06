#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <sys/mman.h>
#include <pthread.h>
#include "config.c"
#include "gpu_lib.h"
#include "accel_lib.h"
#include <fcntl.h>
#include <linux/input.h>
#include <math.h>

bool exitGame = false;
void encerrarJogo()
{
    exitGame = true;
    raise(SIGTERM);
}

volatile int directionP1;
volatile int directionP2;
int pause;
Mouse mouse;
volatile struct input_event ev;
pthread_t threadMouse, threadJogo, threadAccell, threadKeys;
Map *map;
Player player1, player2;

void *readMouse();
void *readAccell();
void *readKeys();
void *thread_jogo();
Map *generateMap(int layout[MAP_SIZE][MAP_SIZE]);
void verifyDamagePlayer(Player *player, int x, int y);
bool explode(Player *player1, Player *player2, Bomb *bomb, Map *map);
bool takeDamage(Cell *cell, Player *player1, Player *player2, Map *map);
bool movePlayer(Map map, Player *player, int direction);
bool placeBomb(Player *player, Map *map);
int updateGame(Map *map, Player *player1, Player *player2);
int readInputsP1(int accel_x, int accel_y);
int readInputsP2(int eixo, int valor);
void ImprimirTextMatrix(int matriz[SCREEN_HEIGHT][SCREEN_WIDTH]);
void Pause();
void ImprimirTabuleiro(Map *map);
void LoadSprite(int spriteSlot, int matriz[20][20]);
void Delay(float segundos);
Map *GameSetup(Map *map, Player *player1, Player *player2);
void GameMenu();
int InitDevices(Mouse *mouse);
int GetDirections(Mouse *mouse, Player *player1, Player *player2, int *directionP1, int *directionP2);
void Movement(int *cooldownMovement, Player *player1, Player *player2, Map map);
void InGameActions(Mouse *mouse, Player *player1, Player *player2, Map *map);
void ProgramActions(bool *exitGame);
void Imprimircoisas2x(int x, int y, int matriz[20][20]);


int main()
{
    // Setup
    signal(SIGINT, encerrarJogo);
    srand(time(NULL)); // seed de aleatoriedade

    if (InitDevices(&mouse) == -1)
    {
        return -1;
    }
    pthread_create(&threadJogo, NULL, thread_jogo, NULL);
    pthread_create(&threadMouse, NULL, readMouse, NULL);
    pthread_create(&threadAccell, NULL, readAccell, NULL);
    pthread_create(&threadKeys, NULL, readKeys, NULL);

    pthread_join(threadJogo, NULL);
}

void * thread_jogo()
{
    LoadSprite(0, player1Sprite);
    LoadSprite(1, player2Sprite);
    LoadSprite(2, bombSprites[0]);
    LoadSprite(3, bombSprites[1]);
    LoadSprite(4, bombSprites[2]);
    LoadSprite(5, bombSprites[3]);

    int gamestate, cooldownMovimento = 0;

    // GameSetup(&map, &player1, &player2);

    clear_all();
    // int countPassWhile = 0;

    // pthread_create(&threadMouse, NULL, readMouse, NULL);
    set_background_color(7, 7, 7);
    int index = 0;
    while (!exitGame)
    {
        GameMenu();
        clear_all();
        map = GameSetup(map, &player1, &player2);
        gamestate = updateGame(map, &player1, &player2);
        while ((gamestate == ONGOING) && !exitGame)
        {
            while(pause)
            {
                clear_sprite(-1);
                ImprimirTextMatrix(PauseScreen); 
            }
            
            ImprimirTabuleiro(map); 

            ImprimirDesenhosBomberman(60,15,miniBombermanDrawn[index/4]);
            if(index == 27)
            {
                index = 0;

            }
            index++;    
            set_polygon(0, 0007, 1, 1, mouse.y+5, mouse.x+5);
            Movement(&cooldownMovimento, &player1, &player2, *map);
            // ProgramActions(&exitGame);
            gamestate = updateGame(map, &player1, &player2);
            switch (gamestate)
            {
            case P1WINS:
                clear_all();
                printf("P1 Wins\n");
                ImprimirTextMatrix(P1WScreen);
                break;
                break;
            case P2WINS:
                clear_all();
                printf("P2 Wins\n");
                ImprimirTextMatrix(P2WScreen);
                break;
                break;
            case DRAW:
                clear_all();
                printf("Draw\n");
                ImprimirTextMatrix(drawScreen);
                break;
                break;
            }
            
        }

        while (!read_keys())
        {
            Delay(0.3);
        }
        Delay(0.3); 
        
    }
    pthread_exit(NULL);
}

Map *generateMap(int layout[MAP_SIZE][MAP_SIZE])
{
    free(map);
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
                // map->matriz[i][j].type = EMPTY_CELL;
                // break;

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
        if(bomb->timer == (BOMB_TIMER/4)*3)
        {
            set_sprite(bomb->owner->bombID, 3,1,map->x*(BLOCK_SIZE * 8) + 5, map->y*(BLOCK_SIZE * 8) + 6);
        }
        else if(bomb->timer == (BOMB_TIMER/4)*2)
        {
            set_sprite(bomb->owner->bombID, 4,1,map->x*(BLOCK_SIZE * 8) + 5, map->y*(BLOCK_SIZE * 8) + 6);
        }
        else if(bomb->timer == BOMB_TIMER/4)
        {
            set_sprite(bomb->owner->bombID, 5,1,map->x*(BLOCK_SIZE * 8) + 5, map->y*(BLOCK_SIZE * 8) + 6);
        }
        bomb->timer -= 1;
        return false;
    }
    int i = 1;

    verifyDamagePlayer(player1, map->x, map->y);
    verifyDamagePlayer(player2, map->x, map->y);

    while (!takeDamage(&map->matriz[map->x - i][map->y], player1, player2, map) && i < bomb->power)
    {
        set_sprite(bomb->owner->bombID+i, 5,1,(map->x-i)*(BLOCK_SIZE * 8) + 5, map->y*(BLOCK_SIZE * 8) + 6);
        verifyDamagePlayer(player1, map->x - i, map->y);
        verifyDamagePlayer(player2, map->x - i, map->y);
        i++;
        Delay(0.08);
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x + i][map->y], player1, player2, map) && i < bomb->power)
    {
        set_sprite(bomb->owner->bombID+i, 5,1,(map->x+i)*(BLOCK_SIZE * 8) + 5, map->y*(BLOCK_SIZE * 8) + 6);
        verifyDamagePlayer(player1, map->x + i, map->y);
        verifyDamagePlayer(player2, map->x + i, map->y);
        i++;
        Delay(0.08);
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y - i], player1, player2, map) && i < bomb->power)
    {
        set_sprite(bomb->owner->bombID+i, 5,1,(map->x)*(BLOCK_SIZE * 8) + 5, (map->y-i)*(BLOCK_SIZE * 8) + 6);
        verifyDamagePlayer(player1, map->x, map->y - i);
        verifyDamagePlayer(player2, map->x, map->y - i);
        i++;
        Delay(0.08);
    }
    i = 1;
    while (!takeDamage(&map->matriz[map->x][map->y + i], player1, player2, map) && i < bomb->power)
    {
        set_sprite(bomb->owner->bombID+i, 5,1,(map->x)*(BLOCK_SIZE * 8) + 5, (map->y+i)*(BLOCK_SIZE * 8) + 6);
        verifyDamagePlayer(player1, map->x, map->y + i);
        verifyDamagePlayer(player2, map->x, map->y + i);
        i++;
        Delay(0.08);
    }
    // printf("mapx = %d\n", map->x);
    // printf("mapy = %d\n", map->y);
    // Cell cell = map->matriz[map->x][map->y];
    // cell.type = EMPTY_CELL;
    map->matriz[map->x][map->y].type = EMPTY_CELL;
    bomb->owner->bombs += 1;
    // set_sprite(bomb->owner->bombID,0,0,0,0);
    for(i=0;i<bomb->power;i++)
    {
        set_sprite(bomb->owner->bombID+i,0,0,0,0);
    }
    //clear_background();
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
        cell->object.bomb.timer = 1;
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
            player->posX -= 1;
            return true;
        }
        break;
    case RIGHT:
        if (map.matriz[player->posX + 1][player->posY].type == EMPTY_CELL)
        {
            player->posX += 1;
            return true;
        }
        break;
    case UP:
        if (map.matriz[player->posX][player->posY - 1].type == EMPTY_CELL)
        {
            player->posY -= 1;
            return true;
        }
        break;
    case DOWN:
        if (map.matriz[player->posX][player->posY + 1].type == EMPTY_CELL)
        {
            player->posY += 1;
            return true;
        }
        break;
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
        set_sprite(player->bombID,2,1,(player->posX*(BLOCK_SIZE * 8) + 5), (player->posY*(BLOCK_SIZE * 8) + 6));
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
    if (abs(accel_x) > abs(accel_y))
    {
        if (accel_x > INPUT_INCLINACAO)
            return RIGHT;
        else if (accel_x < -INPUT_INCLINACAO)
            return LEFT;
    }
    else{
    if (accel_y > INPUT_INCLINACAO)
        return UP;
    else if (accel_y < -INPUT_INCLINACAO)
        return DOWN;
    }

    return -1;

}

int readInputsP2(int eixo, int valor)
{
    if (eixo == 1) // Movimento no eixo Y
    {
        // printf("Dif Y: %d\n", valor-player_y);
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
        //    printf("Dif X: %d\n", valor-player_x);
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

void ImprimirTextMatrix(int matriz[SCREEN_HEIGHT][SCREEN_WIDTH])
{
    int i;
    int j;
    for (i = 0; i < SCREEN_WIDTH; i++)
    {
        for (j = 0; j < SCREEN_HEIGHT; j++)
        {
            background_box(i,j,1,1, matriz[j][i]);

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
    int block[4][4] = {
        {0,0,0,1},
        {1,1,1,1},
        {0,1,0,0},
        {1,1,1,1},
    };

    int i, i2;
    int j, j2;
    int color;
    for (i = 0; i < MAP_SIZE; i++)
    {
        for (j = 0; j < MAP_SIZE; j++)
        {
                switch (map->matriz[i][j].type)
                {
                case EMPTY_CELL:
                    background_box(i * BLOCK_SIZE, j * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0333);
                    break;
                case BARRIER:
                    for (i2 = 0; i2 < BLOCK_SIZE; i2++)
                    {
                        for (j2 = 0; j2 < BLOCK_SIZE; j2++)
                        {
                            if (block[i2][j2] == 1)
                            {
                                set_background_block((i*BLOCK_SIZE)+i2,(j*BLOCK_SIZE)+j2, 2,2,2);
                            }
                            else
                            {
                                set_background_block((i*BLOCK_SIZE)+i2,(j*BLOCK_SIZE)+j2, 1,1,1);
                            }
                        }
                        
                    }
                    break;
                case BOMB:
                    background_box(i * BLOCK_SIZE, j * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0222);
                    break;
                case BLOCK:
                        for (i2 = 0; i2 < BLOCK_SIZE; i2++)
                        {
                            for (j2 = 0; j2 < BLOCK_SIZE; j2++)
                            {
                                if (block[j2][i2] == 1)
                                {
                                    background_box((i*BLOCK_SIZE)+i2,(j*BLOCK_SIZE)+j2,1,1, 0642);
                                }
                                else
                                {
                                    background_box((i*BLOCK_SIZE)+i2,(j*BLOCK_SIZE)+j2,1,1, 0621);
                                }
                            }
                            
                        }
                    break;
                    
                    // background_box(i * BLOCK_SIZE, j * BLOCK_SIZE, BLOCK_SIZE, BLOCK_SIZE, 0722);
                default:
                    set_polygon(1,111111111,1,1,40,40);
                    break;
                }
                // color = LISTA_CORES[map->matriz[i][j].type]->R << 6 | LISTA_CORES[map->matriz[i][j].type]->G << 3 | LISTA_CORES[map->matriz[i][j].type]->B;
                // background_box(i * 3, j * 3, 3, 3, color);
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

int InitDevices(Mouse *mouse)
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
    mouse->fdMouse = open("/dev/input/event0", O_RDONLY);
    if (mouse->fdMouse == -1)
    {
        printf("Erro ao abrir o dispositivo de entrada\n");
        return -1;
    }

    I2C0_init();
    accel_init();

    set_background_color(0, 0, 0);
    clear_background();
}

Map *GameSetup(Map *map, Player *player1, Player *player2)
{
    map = generateMap(mapLayout);
    Player player_1 =
        {
            .HP = DEFAULT_HP,
            .bombs = DEFAULT_BOMBS,
            .power = DEFAULT_POWER,
            .posX = STARTING_P1_X,
            .posY = STARTING_P1_Y,
            .bombID = 10};
    *player1 = player_1;

    Player player_2 =
        {
            .HP = DEFAULT_HP,
            .bombs = DEFAULT_BOMBS,
            .power = DEFAULT_POWER,
            .posX = STARTING_P2_X,
            .posY = STARTING_P2_Y,
            .bombID = 15};

    *player2 = player_2;

    set_sprite(1, 0, 1, player1->posX * (BLOCK_SIZE * 8) + 5, player1->posY * (BLOCK_SIZE * 8) + 6);
    set_sprite(2, 1, 1, player2->posX * (BLOCK_SIZE * 8) + 5, player2->posY * (BLOCK_SIZE * 8) + 6);

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
        //Imprimircoisas2x(10, 7, bonecoBlock); // Conferir o local dessa tela, pode ser antes.
        ImprimirTextMatrix(gameTitle);
        // printf("Bomberman!");
    }
}

void Imprimircoisas2x(int x, int y, int matriz[20][20])
{
    int i;
    int j;
    int color;

    for (i = 0; i < 20; i++)
    {
        for (j = 0; j < 20; j++)
        {
            Cor *cor = LISTA_CORES[matriz[i][j]];

            color = (cor->R << 6) | (cor->G << 3) | cor->B;

            background_box((j+x), (i+y), 1, 1, color);
        }
    }
}

void ImprimirDesenhosBomberman(int x, int y, int matriz[27][20])
{
    int i;
    int j;
    int color;

    for (i = 0; i < 27; i++)
    {
        for (j = 0; j < 20; j++)
        {

            background_box((j+x), (i+y), 1, 1, matriz[i][j]);
        }
    }
}

int GetMouseEvent(int mouse, struct input_event *ev)
{
    read(mouse, ev, sizeof(struct input_event)) < sizeof(struct input_event);
    return 0;
}

// int GetDirections(Mouse *mouse, Player *player1, Player *player2, int *directionP1, int *directionP2)
// {
//     // printf("PASSOU\n");
//     int accel_x = get_calibrated_accel_x(); // Receber inclinação da placa
//     int accel_y = get_calibrated_accel_y();

//     *directionP1 = readInputsP1(accel_x, accel_y);

//     struct input_event ev;
//     if (GetMouseEvent(mouse->mouse, &ev) != -1)
//     {
//         if (ev.type == EV_REL)
//         {
//             if (ev.code == REL_X)
//             {
//                 int newX = ev.value;

//                 *directionP2 = readInputsP2(0, newX);
//             }
//             else if (ev.code == REL_Y)
//             {
//                 int newY = ev.value;
//                 *directionP2 = readInputsP2(1, newY);
//             }
//         }
//         else
//         {
//             *directionP2 = -1;
//         }
//         return 0;
//     }
//     return -1;
// }

void Movement(int *cooldownMovement, Player *player1, Player *player2, Map map)
{
    // GetDirections(mouse, player1, player2, &directionP1, &directionP2);
    if (*cooldownMovement == COOLDOWN_INPUT)
    {

    // //printf("DIRECTION P2: %d\n",*directionP2);
        if (movePlayer(map, player1, directionP1))
        {
            // printf("move\n");
            // printf("direction P1 = %d\n", directionP1);

            set_sprite(1, 0, 1, player1->posX * (BLOCK_SIZE * 8) + 5, player1->posY * (BLOCK_SIZE * 8) + 6);
            *cooldownMovement = 0;
        }
        if (movePlayer(map, player2, directionP2))
        {
            set_sprite(2, 1, 1, player2->posX * (BLOCK_SIZE * 8) + 5, player2->posY * (BLOCK_SIZE * 8) + 6);
            *cooldownMovement = 0;
        }
    }
    else
    {
    // printf("PASSOU3\n");
    //  printf("TESTE2\n");
        (*cooldownMovement)++;
    }
}

void InGameActions(Mouse *mouse, Player *player1, Player *player2, Map *map)
{
    struct input_event ev;
    if (GetMouseEvent(mouse->fdMouse, &ev) != -1 && ev.type == EV_KEY)
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

void *readMouse()
{
    while (1)
    {
        read(mouse.fdMouse, &ev, sizeof(struct input_event));
        if (ev.type == EV_REL)
        {
            if (ev.code == REL_X)
            {
                int newX = mouse.x + ev.value;  
                if (newX > 480)
                {
                    mouse.x = 480;
                }
                else if (newX < 20)
                {
                    mouse.x = 20;
                }
                else 
                {
                    mouse.x = newX;
                }


            }
            else if (ev.code == REL_Y)
            {
                int newY = mouse.y + ev.value;  
                if (newY > 480)
                {
                    mouse.y = 480;
                }
                else if (newY < 20)
                {
                    mouse.y = 20;
                }
                else 
                {
                    mouse.y = newY;
                }
            }
        }
        else if (ev.type == EV_KEY)
        {
            placeBomb(&player2, map);
        }
        else
        {
            directionP2 = -1;
        }

        int deltaX = player2.posX*((BLOCK_SIZE * 8))+10 - mouse.x;
        int deltaY = player2.posY*((BLOCK_SIZE * 8))+9 - mouse.y;
        if (abs(deltaX) > abs(deltaY))
        {
            if (deltaX > INPUT_INCLINACAO_MOUSE)
            {
                directionP2 = LEFT;
            }
            else if (deltaX < -INPUT_INCLINACAO_MOUSE)
            {
                directionP2 = RIGHT;
            }
        }
        else
        {
            if (deltaY > INPUT_INCLINACAO_MOUSE)
            {
                directionP2 = UP;
            }
            else if (deltaY< -INPUT_INCLINACAO_MOUSE)
            {
                directionP2 = DOWN;
            }
        }
    }
}

void *readAccell()
{
    while (1)
    {
        Delay(0.4);
        int x = get_calibrated_accel_x();
        int y = get_calibrated_accel_y();
        directionP1 = readInputsP1(x, y);
    }
}

void *readKeys()
{
    while (1)
    {
        Delay(0.4);
        int key = read_keys();
        switch (key)
        {
            case 8:
                if(pause)
                {
                    pause = false;
                }
                else
                {
                    pause = true;
                }
                break;
            case 4:
                exitGame = true;
                break;
            case 1:
                placeBomb(&player1, map);
                break;
        }
    }
    
}