from colorama import Back
from sprites.mapa import layout
import os
import copy as c

# PLAYERS FLUTUANTES!!

# config

MAP_SIZE = 21
BOMB_TIME = 3 # ticks
MOVE_COOLDOWN = 5

# Default values
DEFAULT_LIVES = 1   
DEFAULT_BOMBS = 1
DEFAULT_POWER = 4
DEFAULT_VELOCITY = 1
P1_X = 1
P1_Y = 1
P2_X = 19
P2_Y = 19

# Tipos para serem usados na estrutura Celula

NADA = 0
BLOCO = 1
BARREIRA = 2 # Bloco indestrutivel
PLAYER1 = 3
BOMBA = 4
ITEM = 5
PLAYER2 = 6

# Cores
COR_NADA = Back.WHITE + "  "
COR_BLOCO = Back.LIGHTCYAN_EX + "  "
COR_BARREIRA = Back.BLACK + "  "
COR_PLAYER1 = Back.BLUE + "P1"
COR_PLAYER2 = Back.RED + "P2"
COR_BOMBA = Back.GREEN+ "@*"
COR_ITEM = Back.YELLOW + "  "


# Estados do jogo
EM_PROGRESSO = 0
JOGADOR_1_WIN = 1
JOGADOR_2_WIN = 2
EMPATE = 3

# direções
UP = 0
DOWN = 1
LEFT = 2
RIGHT = 3

class Celula:
    tipo = 0
    objeto = None

    def __init__(self, tipo, objeto) -> None:
        self.tipo = tipo
        self.objeto = objeto


class Player:
    vida = DEFAULT_LIVES
    bombas = DEFAULT_BOMBS
    poder = DEFAULT_POWER
    velocidade = DEFAULT_VELOCITY
    x = P1_X
    y = P1_Y
    direcao = DOWN

    def __init__(self, vida, bombas, poder, velocidade, x, y, direcao) -> None:
        self.vida = vida
        self.bombas = bombas
        self.poder = poder
        self.velocidade = velocidade
        self.x = x
        self.y = y
        self.direcao = direcao

# -1 : bloco indestrutivel
# 0 : nada
# 1 : bloco destrutivel / vida do bloco
class Bloco:
    vida = 1 # positivo : destrutivel, negativo : indestrutivel
    item = ""

    def __init__(self, vida, item) -> None:
        self.vida = vida
        self.item = item
    
class Bomba:
    dono = None
    tempo = BOMB_TIME
    poder = DEFAULT_POWER

    def __init__(self, dono=Player, tempo=int, poder=int) -> None:
        self.dono = dono
        self.tempo = tempo
        self.poder = poder

class Item:
    sprite = "#"
    vida = 1 # se o item for explodido ele some
    maisPoder = 0 # Quantidade de poder que adiciona a bomba
    maisBomba = 1 # Quantidade de bombas que adiciona ao jogador
    maisVelocidade = 0 # Quantidade de velocidade que adiciona ao jogador
    maisVida = 0 # Quantidade de vida que adiciona ao jogador

    def __init__(self, sprite, vida, maisPoder, maisBomba, maisVelocidade, maisVida) -> None:
        self.sprite = sprite
        self.vida = vida
        self.maisPoder = maisPoder
        self.maisBomba = maisBomba
        self.maisVelocidade = maisVelocidade
        self.maisVida = maisVida

class Mapa:
    matriz = [[Celula(NADA, None) for i in range(20)] for i in range(20)]
    x = 0
    y = 0

    def __init__(self, matriz,x, y) -> None:
        self.matriz = matriz
        self.x = x
        self.y = y

def dano(celula=Celula):
    match celula.tipo:
        case 0: # NADA
            return False
        case 1: # Bloco
            celula.objeto.vida -= 1
            if celula.objeto.vida == 0: 
                # Mudar se quiser adicionar Item
                celula.tipo = NADA
                celula.objeto = None 
            return True
        case 2: # Barreira
            return True
        case 3: # Player
            celula.objeto.vida -= 1
            if celula.objeto.vida == 0: 
                celula.tipo = NADA
                celula.objeto = None 
                print("Jogador morto")
            return True
        case 4: # Bomba
            celula.objeto.tempo = 0
            explodir(celula.objeto)

def dano_player(x=int, y=int):
    if x == p1.x and y == p1.y: 
        p1.vida -= 1
    if x == p2.x and y == p2.y:
        p2.vida -= 1

def explodir(bomba=Bomba):
    if bomba.tempo > 0:
        bomba.tempo -= 1
        print(bomba.tempo)
        return False
    i = 1
    while not dano(mapa.matriz[mapa.x-i][mapa.y]) and i <= bomba.poder: # Dar dano até o fim do raio da bomba ou acertar algo
        dano_player(mapa.x-i, mapa.y)
        i +=1
    i = 1
    while not dano(mapa.matriz[mapa.x+i][mapa.y]) and i <= bomba.poder: # 4 direções
        dano_player(mapa.x+i, mapa.y)
        i += 1
    i = 1
    while not dano(mapa.matriz[mapa.x][mapa.y-i]) and i <= bomba.poder:
        dano_player(mapa.x, mapa.y-i)
        i += 1
    i = 1
    while not dano(mapa.matriz[mapa.x][mapa.y+i]) and i <= bomba.poder:
        dano_player(mapa.x, mapa.y+i)
        i += 1

    mapa.matriz[mapa.x][mapa.y] = Celula(NADA, None)
    bomba.dono.bombas += 1

    return True

def mover_jogador(player=Player, direcao=int):
    match direcao:
        case 0: # up
            if mapa.matriz[player.x-1][player.y].tipo == NADA: # se quadrado vazio
                player.direcao = UP
                player.x -= 1
                return True
        case 1: # down
            if mapa.matriz[player.x+1][player.y].tipo == NADA: 
                player.direcao = DOWN
                player.x += 1
                return True
        case 2: # left
            if mapa.matriz[player.x][player.y-1].tipo == NADA:
                player.direcao = LEFT
                player.y -= 1
                return True
        case 3: # right
            if mapa.matriz[player.x][player.y+1].tipo == NADA:
                player.direcao = RIGHT
                player.y += 1
                return True
    return False

def colocar_bomba(player=Player):
    direcao = player.direcao
    bombaColocada = False
    if player.bombas > 0:
        match direcao:
            case 0: # up
                if mapa.matriz[player.x-1][player.y].tipo == NADA: # se quadrado vazio
                    mapa.matriz[player.x-1][player.y] = Celula(BOMBA, Bomba(player, BOMB_TIME, player.poder)) # Colocar bomba
                    bombaColocada = True
            case 1: # down
                if mapa.matriz[player.x+1][player.y].tipo == NADA: 
                    mapa.matriz[player.x+1][player.y] = Celula(BOMBA, Bomba(player, BOMB_TIME, player.poder))
                    bombaColocada = True
            case 2: # left
                if mapa.matriz[player.x][player.y-1].tipo == NADA:
                    mapa.matriz[player.x][player.y-1] = Celula(BOMBA, Bomba(player, BOMB_TIME, player.poder))
                    bombaColocada = True
            case 3: # right
                if mapa.matriz[player.x][player.y+1].tipo == NADA:
                    mapa.matriz[player.x][player.y+1] = Celula(BOMBA, Bomba(player, BOMB_TIME, player.poder))
                    bombaColocada = True
    if bombaColocada: player.bombas -= 1
    return bombaColocada

def atualizar_jogo(mapa=Mapa):
    for y in range(MAP_SIZE):
        for x in range(MAP_SIZE):
            mapa.x = x
            mapa.y = y
            if mapa.matriz[x][y].tipo == BOMBA:
                explodir(mapa.matriz[x][y].objeto) 
    
    if p1.vida < 1 and p2.vida < 1:
        return EMPATE
    elif p1.vida < 1:
        return JOGADOR_2_WIN
    elif p2.vida < 1:
        return JOGADOR_1_WIN
    else:
        return EM_PROGRESSO

def gerar_mapa(layout=list):
    matriz = [[0 for i in range(21)] for i in range(21)]
    for i in range(21):
        for j in range(21):
            match layout[i][j]:
                case 0:
                    # Nada
                    matriz[i][j] = Celula(NADA, None)
                case 1:
                    # Bloco
                    matriz[i][j] = Celula(BLOCO, Bloco(1, None))
                case 2:
                    # Barreira
                    matriz[i][j] = Celula(BARREIRA, Bloco(1, None))
    mapa = Mapa(matriz, 0, 0)

    return mapa

def player_stats(player=Player):
    print("Vidas:" + str(player.vida))
    print("Bombas:" + str(player.bombas))
    print("Poder:" + str(player.poder))
    print("Velocidade:" + str(player.velocidade))
    print("X:" + str(player.x))
    print("Y:" + str(player.y))
    print("Direção:" + str(player.direcao))

def inputs(player=Player):
    direcao = input("Movimento(u,d,l,r):")
    match direcao:
        case "u": direcao = UP
        case "d": direcao = DOWN
        case "l": direcao = LEFT
        case "r": direcao = RIGHT
        case _: direcao = 4

    mover_jogador(player, direcao)
    if input("Colocar Bomba:") == "s":
        colocar_bomba(player)

def print_map(mapa=Mapa):
    for listaCelulas in mapa.matriz:
        for celula in listaCelulas:
            match celula.tipo:
                case 0: # Nada
                    print(COR_NADA, end="")
                case 1: # Bloco
                    print(COR_BLOCO, end="")
                case 2: # Barreira
                    print(COR_BARREIRA, end="")
                case 3: # Player 1
                    print(COR_PLAYER1, end="")
                case 4: # Bomba
                    print(COR_BOMBA, end="")
                case 6: # Player 2
                    print(COR_PLAYER2, end="")
        print(Back.RESET +"")

mapa = gerar_mapa(layout)
screen = c.deepcopy(mapa)

p1 = Player(DEFAULT_LIVES, DEFAULT_BOMBS, DEFAULT_POWER, DEFAULT_VELOCITY, P1_X, P1_Y, DOWN)
p2 = Player(DEFAULT_LIVES, DEFAULT_BOMBS, DEFAULT_POWER, DEFAULT_VELOCITY, P2_X, P2_Y, UP)
screen.matriz[p1.x][p1.y] = Celula(PLAYER1, p1)
screen.matriz[p2.x][p2.y] = Celula(PLAYER2, p2)

while True:
    if input("(s)tart | (e)xit: ") == "e":
        break

    while True:
        player_stats(p1)
        player_stats(p2)

        print_map(screen)
        estadoPartida = atualizar_jogo(mapa)

        if estadoPartida == EMPATE:
            print("Ambos jogadores morreram. Empate.")
            break
        elif estadoPartida == JOGADOR_1_WIN:
            print("Vitória do Jogador 1!!")
            break
        elif estadoPartida == JOGADOR_2_WIN:
            print("Vitória do Jogador 2!!")
            break

        inputs(p1)
        inputs(p2)

        screen = c.deepcopy(mapa)
        screen.matriz[p1.x][p1.y] = Celula(PLAYER1, p1)
        screen.matriz[p2.x][p2.y] = Celula(PLAYER2, p2)
