//Screen setting
const SCREEN_HEIGHT = 60 ;
const SCREEN_WIDTH = 80;

//game settings
const MAP_SIZE = 21;
const BOMB_TIMER = 3;
const DEFAULT_HP = 1; 
const DEFAULT_BOMBS = 1;
const DEFAULT_POWER = 4;
const DEFAULT_VELOCITY = 1;
const STARTING_P1_X = 1;
const STARTING_P1_Y = 1;
const STARTING_P2_X = 19;
const STARTING_P2_Y = 19;

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

//Colors
typedef struct Cor
{
	int R;
	int G;
	int B;
} Cor;

const Cor BLACK = {0, 0, 0};
const Cor GRAY = {7, 7, 7};
const Cor YELLOW = {7, 7, 0};
const Cor ORANGE = {7, 4, 1};
const Cor BLUE = {0, 0, 7};
const Cor CYAN = {0, 7, 7};
const Cor MAGENTA = {7, 0, 7};
const Cor GREEN = {0, 7, 0};
const Cor RED = {7, 0, 0};

const Cor *LISTA_CORES[] = {&BLACK, &GRAY, &YELLOW, &ORANGE, &BLUE, &CYAN, &MAGENTA, &GREEN, &RED};

int mapLayout[MAP_SIZE][MAP_SIZE] = {
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2}, 
    {2, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 2}, 
    {2, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0, 2}, 
    {2, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 2}, 
    {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, 
    {2, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 2}, 
    {2, 1, 2, 0, 2, 1, 2, 1, 2, 0, 2, 0, 2, 1, 2, 1, 2, 0, 2, 1, 2}, 
    {2, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 2}, 
    {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, 
    {2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2}, 
    {2, 0, 2, 1, 2, 0, 2, 1, 2, 0, 2, 0, 2, 1, 2, 0, 2, 1, 2, 0, 2}, 
    {2, 0, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 2}, 
    {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, 
    {2, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 2}, 
    {2, 1, 2, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0, 2, 1, 2}, 
    {2, 1, 1, 1, 1, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 2}, 
    {2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2}, 
    {2, 0, 1, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 0, 2}, 
    {2, 0, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 1, 2, 0, 2}, 
    {2, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 2},
    {2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2},
};

//int titleMatriz[SCREEN_HEIGHT][SCREEN_WIDTH]=0