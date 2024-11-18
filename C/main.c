#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>


const MAP_SIZE = 21;


// Cell Types
const EMPTY_CELL = 0;
const BLOCK = 1;
const BARRIER = 2;
const BOMB = 3;
const ITEM = 4;
const PLAYER1 = 5;
const PLAYER2 = 6;
typedef union  Object
{
    Block* block;
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
                    map->matriz[i][j].object.block = (Block*) malloc(sizeof(Block));

                    map->matriz[i][j].object.block->HP = 1;
                    break;

                case BARRIER:
                    map->matriz[i][j].type = BARRIER;
                    map->matriz[i][j].object.block = (Block*) malloc(sizeof(Block));

                    map->matriz[i][j].object.block->HP = 1;
                    break;
            }
        }
    }

    return map;
}

bool takeDamage(Cell cell)
{
    switch (cell.type)
    {
        case EMPTY_CELL:
            return false;
        case BLOCK:
            cell.object.block->HP -=1;
            if(cell.object.block->HP == 0)
            {
                cell.type = EMPTY_CELL;
                cell.object.empty = EMPTY_CELL;
            }
            return true;
        case BARRIER:
            return true;
        case BOMB:
            cell.object.bomb.timer = 0;
            explode(cell.object.bomb);
            break;
        default:
            cell.object.player.HP -=1;
            if(cell.object.player.HP == 0)
            {
                cell.type = EMPTY_CELL;
                cell.object.empty = EMPTY_CELL;
            }
            return true;
    }

    return false;
}
int main()
{

}