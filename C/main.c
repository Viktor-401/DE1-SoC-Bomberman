#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#include "config.c"
#include "gpu_lib.h"
#include "accel_lib.h"
#include <X11/Xlib.h>

bool sair = false;
bool gameOver = false;
void encerrarJogo()
{
	sair = true;
	raise(SIGTERM);
}

typedef union  Object
{
    Block block;
    Item item;
    Player player;
    Bomb bomb;
    int empty;
}Object;
typedef struct Cell
{
    int type;
    Object object;
}Cell;

typedef struct Item
{
    int HP;
    int powerBoost;
    int increaseBombs;
    int speedBoost;
    int HPBoost;
}Item;
typedef struct Block{
    int HP;
    Item item;
}Block;

typedef struct Bomb
{
    int timer;
    int power;
    Player owner;
}Bomb;

typedef struct Player
{
    //TODO Sprites
    int HP;
    int bombs;
    int power;
    int posX;
    int posY;
    int facingDirection;
}Player;

typedef struct Map
{
    Cell matriz[MAP_SIZE][MAP_SIZE];
    int x;
    int y;
    
}Map;

Map* generateMap(int layout[MAP_SIZE][MAP_SIZE]) {

    Map* map = (Map*) malloc(sizeof(Map));
    for (int i = 0; i < MAP_SIZE; i++) {
        for (int j = 0; j < MAP_SIZE; j++) {
            switch (layout[i][j]) {
                case EMPTY_CELL:
                    map->matriz[i][j].type = EMPTY_CELL;
                    break;

                case BLOCK:
                    map->matriz[i][j].type = BLOCK;
                    Block* block = &map->matriz[i][j].object.block;
                    block->HP = 1;
                    break;

                case BARRIER:
                    map->matriz[i][j].type = BARRIER;
                    Block* block = &map->matriz[i][j].object.block;
                    block->HP = 1;
                    break;
            }
        }
    }
    return map;
}

bool explode(Player player, Bomb bomb, Map * map)
{
    if (bomb.timer>0)
    {
        bomb.timer-=1;
        return false;
    }
    int i = 0;
    while(!takeDamage(&map->matriz[map->x-1][map->y], player, map) && i<= bomb.power)
    {
        verifyDamagePlayer(player, map->x-1, map->y);
        i++;
    }
    int i = 0;
    while(!takeDamage(&map->matriz[map->x+1][map->y], player, map) && i<= bomb.power)
    {
        verifyDamagePlayer(player, map->x+1, map->y);
        i++;
    }
    int i = 0;
    while(!takeDamage(&map->matriz[map->x][map->y-1], player, map) && i<= bomb.power)
    {
        verifyDamagePlayer(player, map->x, map->y-1);
        i++;
    }
    int i = 0;
    while(!takeDamage(&map->matriz[map->x][map->y+1], player, map) && i<= bomb.power)
    {
        verifyDamagePlayer(player, map->x, map->y+1);
        i++;
    }

    Cell cell = map->matriz[map->x][map->y];
    cell.type = EMPTY_CELL;
    cell.object.empty = EMPTY_CELL;
    bomb.owner.bombs += 1;
    return true;
}

bool takeDamage(Cell* cell, Player player, Map *map)
{
    switch (cell->type)
    {
        case EMPTY_CELL:
            return false;
        case BLOCK:
            cell->object.block.HP -=1;
            if(cell->object.block.HP == 0)
            {
                cell->type = EMPTY_CELL;
                cell->object.empty = EMPTY_CELL;
            }
            return true;
        case BARRIER:
            return true;
        case BOMB:
            cell->object.bomb.timer = 0;
            explode(player,cell->object.bomb, map);
            break;
        default:
            cell->object.player.HP -=1;
            if(cell->object.player.HP == 0)
            {
                cell->type = EMPTY_CELL;
                cell->object.empty = EMPTY_CELL;
            }
            return true;
    }
    return false;
}

void verifyDamagePlayer(Player player, int x,int y)
{
    if((x==player.posX) && (y == player.posY))
    {
        player.HP -=1;
    }
}

bool movePlayer(Map map, Player player, int direction)
{
    switch (direction)
    {
        case UP:
            if(map.matriz[player.posX-1][player.posY].type == EMPTY_CELL)
            {
                player.facingDirection = UP;
                player.posX -= 1;
                return true;
            }
        case DOWN:
            if (map.matriz[player.posX+1][player.posY].type == EMPTY_CELL)
            {
                player.facingDirection = DOWN;
                player.posX += 1;
                return true;
            }
        case LEFT:
            if (map.matriz[player.posX][player.posY-1].type == EMPTY_CELL)
            {
                player.facingDirection = LEFT;
                player.posY -= 1;
                return true;
            }
        case RIGHT:
            if (map.matriz[player.posX][player.posY+1].type == EMPTY_CELL)
            {
                player.facingDirection = RIGHT;
                player.posY += 1;
                return true;
            }
        default:
            break;
    }
    return false;
}

bool placeBomb(Player* player, Map* map)
{
    bool bombPlaced = false;
    if(player->bombs>0)
    {
        Bomb* bomb = &map->matriz[player->posX][player->posY].object.bomb;
        bomb->timer = BOMB_TIMER;
        bomb->power = player->power;
        bomb->owner = *player;

        map->matriz[player->posX][player->posY].type = BOMB;

        bombPlaced = true;
    }
    if(bombPlaced)
    {
        player->bombs -=1;
    }
    return bombPlaced;
}

int updateGame(Map* map, Player* player1, Player* player2)
{
    int i, j;
    for(i=0; i<MAP_SIZE; i++)
    {
        for(j=0; j<MAP_SIZE; j++)
        {
            map->x = i;
            map->y = j;
            if(map->matriz[i][j].type == BOMB)
            {
                explode(*player1, map->matriz[i][j].object.bomb, map);
            }
        }
    }

    if(player1->HP<1 && player2->HP<1)
        return DRAW;
    else if(player2->HP<1)
        return P1WINS;
    else if(player1->HP<1)
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

    if (accel_y > INPUT_INCLINACAO) 
        return UP;
    else if (accel_y < -INPUT_INCLINACAO)
        return DOWN;

    return -1; 
}

int readInputsP2(int mouse_x, int mouse_y)
{
    if (abs(mouse_x) > abs(mouse_y))
    {
        if (mouse_x > INPUT_INCLINACAO) 
            return RIGHT;
        else if (mouse_x < -INPUT_INCLINACAO)
            return LEFT;
    }

    if (mouse_y > INPUT_INCLINACAO) 
        return UP;
    else if (mouse_y < -INPUT_INCLINACAO)
        return DOWN;

    return -1; 
}

int readMouseState(Display *display, Window window, int *mouseX, int *mouseY, int *leftButtonPressed) {
    XEvent ev;
    XNextEvent(display, &ev);

    // Inicializa os valores
    *leftButtonPressed = 0;

    if (ev.type == MotionNotify) {
        // Se for um evento de movimento, atualiza as coordenadas do mouse
        *mouseX = ev.xmotion.x;
        *mouseY = ev.xmotion.y;
    }

    if (ev.type == ButtonPress || ev.type == ButtonRelease) {
        // Se for um evento de pressionamento ou liberação de botão, verifica o botão
        if (ev.xbutton.button == LEFT_BUTTON) {
            *mouseX = ev.xbutton.x;
            *mouseY = ev.xbutton.y;

            if (ev.type == ButtonPress) {
                *leftButtonPressed = 1; // Botão esquerdo pressionado
            }
            else if (ev.type == ButtonRelease) {
                *leftButtonPressed = 0; // Botão esquerdo liberado
            }
        }
    }
}
int *full;
int main()
{
    // Setup
	full = create_mapping_memory();
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
	set_background_color(0, 0, 0);
	I2C0_init();
	accel_init();
	limpaTela(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    
    Display *display = XOpenDisplay(NULL);
    Window window = XCreateSimpleWindow(display, DefaultRootWindow(display), 10, 10, 640, 480, 1, BlackPixel(display, 0), WhitePixel(display, 0));
    
    XMapWindow(display, window);
    XSelectInput(display, window, PointerMotionMask | ButtonPressMask | ButtonReleaseMask);

    int accel_x, accel_y, mouse_x, mouse_y, inputKEY, leftButtonPressed, gamestate;
    
    inputKEY = read_keys();
    int indexCor = 1;


	while (inputKEY == 0)
	{
		inputKEY = read_keys();
		Delay(0.3);
		//ImprimirtTextMatrix(titleMatriz, indexCor);
		if (indexCor < 9)
		{
			indexCor++;
		}
		else
		{
			indexCor = 1;
		}
	}

    Map* map = generateMap(mapLayout);

    Player player1 =
    {
    .HP = DEFAULT_HP,
    .bombs = DEFAULT_BOMBS,
    .power = DEFAULT_POWER,
    .posX = STARTING_P1_X,
    .posY = STARTING_P1_Y,
    .facingDirection = DOWN
    };

    Player player2 =
    {
        .HP = DEFAULT_HP,
        .bombs = DEFAULT_BOMBS,
        .power = DEFAULT_POWER,
        .posX = STARTING_P1_X,
        .posY = STARTING_P2_Y,
        .facingDirection = UP
    };
    while(!sair)
    {
        while ((gamestate == 0) && !sair)
		{
            
            gamestate = updateGame(map, &player1, &player2);
            movePlayer(*map, player1, readInputsP1(accel_x, accel_y));
            movePlayer(*map, player2, readInputsP2(mouse_x, mouse_y));
            readMouseState(display, window, &mouseX, &mouseY, &leftButtonPressed); 
            inputKEY = read_keys();
            if(inputKEY == 8)
            {
                Pause();
            }
            else if (inputKEY == 1)
            {
                placeBomb(&player1, &map);
            }
            else if(leftButtonPressed)
            {
                placeBomb(&player2, &map);
            }

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
	while (clock() < start_time + microSegundos);
}

void limpaTela(int x, int y, int limite_x, int limite_y)
{
	int i, j;
	for (i = x; i < limite_x; i++)
	{
		for (j = y; j < limite_y; j++)
		{
			paintBackgroundBlock(i, j, 6, 7, 7);
		}
	}
}

void paintBackgroundBlock(int i, int j, int R, int G, int B)
{
    while (1)
    {
        if (*((full + 0xb0 / sizeof(int))) == 0)
        {
            set_background_block(i, j, R, G, B);
            break;
        }
    }
}

void paintLargeBackgroundBlock(int column, int line, int R, int G, int B, int size)
{
	int i, j = 0;
	for (i = 0; i < size; i++)
	{
		for (j = 0; j < size; j++)
		{
			set_background_block((column * size) + j, (line * size) + i, R, G, B);
	    }
    }
}

void ImprimirtTextMatrix(int matriz[SCREEN_HEIGHT][SCREEN_WIDTH], int indexCor)
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
	limpaTela(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
	while (1)
	{
		//ImprimirtTextMatrix(PauseMatriz, indexCor);
		Delay(0.3);
		if (indexCor < 9)
		{
			indexCor++;
		}
		else
		{
			indexCor = 1;
		}
		
		if(read_keys() == 8)
		{
			break;
		}

	}
	Delay(0.3);
	limpaTela(0,0,SCREEN_WIDTH,SCREEN_HEIGHT);
}

void ImprimirTabuleiro(Map *map)
{
	int i;
	int j;

	for (i = 0; i < SCREEN_HEIGHT; i++)
	{
		for (j = 0; j < SCREEN_WIDTH; j++)
		{
			if (map->matriz[i][j] > 0)
			{
				paintBackgroundBlock((i), (j + 5), LISTA_CORES[map->matriz[i][j]]->R, LISTA_CORES[map->matriz[i][j]]->G, LISTA_CORES[map->matriz[i][j]]->B);
			}
		}
	}
}