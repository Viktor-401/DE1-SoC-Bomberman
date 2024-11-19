#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


const MAP_SIZE = 21;
const BOMB_TIMER = 3;

//Directions
const UP = 0;
const DOWN = 1;
const LEFT = 2;
const RIGHT = 3;

// Cell Types
const EMPTY_CELL = 0;
const BLOCK = 1;
const BARRIER = 2;
const BOMB = 3;
const ITEM = 4;
const PLAYER1 = 5;
const PLAYER2 = 6;

//Game Stats
const ONGOING = 0;
const DRAW = 1;
const P1WINS = 2;
const P2WINS = 3;

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

void explode(Bomb bomb)
{
    //TODO
}

bool takeDamage(Cell* cell)
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
            explode(cell->object.bomb);
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

void damagePlayer(Player player, int x,int y)
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
                explode(map->matriz[i][j].object.bomb);
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
int main()
{

}