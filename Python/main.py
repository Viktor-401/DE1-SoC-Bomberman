from colorama import Back
from sprites.mapa import layout
import os
import copy as c

# PLAYERS FLUTUANTES!!

# config

MAP_SIZE = 21
BOMB_TIME = 3 # ticks
BOMB_POWER = 4 # 4 quadrados de raio
PLAYER_VELOCITY = 1
MOVE_COOLDOWN = 5

# Tipos para serem usados na estrutura Celula

NADA = 0
BLOCO = 1
BARREIRA = 2 # Bloco indestrutivel
PLAYER = 3
BOMBA = 4
ITEM = 5

# Cores
COR_NADA = Back.WHITE + "  "
COR_BLOCO = Back.RED + "  "
COR_BARREIRA = Back.BLACK + "  "
COR_PLAYER = Back.BLUE + "P1"
COR_BOMBA = Back.GREEN+ "@*"
COR_ITEM = Back.YELLOW + "  "

# Estados do jogo
EM_PROGRESSO = 0
JOGADOR_1_WIN = 1
JOGADOR_2_WIN = 2

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
    vida = 1
    bombas = 1
    velocidade = 1
    x = 0
    y = 0
    direcao = DOWN

    def __init__(self, vida, bombas, velocidade, x, y, direcao) -> None:
        self.vida = vida
        self.bombas = bombas
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
    poder = BOMB_POWER

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
    matrizAtual = [[Celula(NADA, None) for i in range(20)] for i in range(20)]
    matrizNova = [[Celula(NADA, None) for i in range(20)] for i in range(20)]
    x = 0
    y = 0

    def __init__(self, matrizAtual, matrizNova,x, y) -> None:
        self.matrizAtual = matrizAtual
        self.matrizNova = matrizNova
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

def explodir(bomba=Bomba):
    if bomba.tempo > 0:
        bomba.tempo -= 1
        print(bomba.tempo)
        return False
    i = 1
    while not dano(mapa.matrizNova[mapa.x-i][mapa.y]) and i <= bomba.poder: # Dar dano até o fim do raio da bomba ou acertar algo
        i +=1
    i = 1
    while not dano(mapa.matrizNova[mapa.x+i][mapa.y]) and i <= bomba.poder: # 4 direções
        i += 1
    i = 1
    while not dano(mapa.matrizNova[mapa.x][mapa.y-i]) and i <= bomba.poder:
        i += 1
    i = 1
    while not dano(mapa.matrizNova[mapa.x][mapa.y+i]) and i <= bomba.poder:
        i += 1

    mapa.matrizNova[mapa.x][mapa.y] = Celula(NADA, None)
    bomba.dono.bombas += 1

    return True

def mover_jogador(direcao):
    match direcao:
        case 0: # up
            if mapa.matrizAtual[mapa.x-1][mapa.y].tipo == NADA: # se quadrado vazio
                mapa.matrizNova[mapa.x][mapa.y].objeto.direcao = UP
                mapa.matrizNova[mapa.x-1][mapa.y] = mapa.matrizAtual[mapa.x][mapa.y] # mover Player para o quadrado
                mapa.matrizNova[mapa.x][mapa.y] = Celula(NADA, None) # posição anterior do Player é vazia
                return True
        case 1: # down
            if mapa.matrizAtual[mapa.x+1][mapa.y].tipo == NADA: 
                mapa.matrizNova[mapa.x][mapa.y].objeto.direcao = DOWN
                mapa.matrizNova[mapa.x+1][mapa.y] = mapa.matrizAtual[mapa.x][mapa.y]
                mapa.matrizNova[mapa.x][mapa.y] = Celula(NADA, None)
                return True
        case 2: # left
            if mapa.matrizAtual[mapa.x][mapa.y-1].tipo == NADA:
                mapa.matrizNova[mapa.x][mapa.y].objeto.direcao = LEFT
                mapa.matrizNova[mapa.x][mapa.y-1] = mapa.matrizAtual[mapa.x][mapa.y]
                mapa.matrizNova[mapa.x][mapa.y] = Celula(NADA, None)
                return True
        case 3: # right
            if mapa.matrizAtual[mapa.x][mapa.y+1].tipo == NADA:
                mapa.matrizNova[mapa.x][mapa.y].objeto.direcao = RIGHT
                mapa.matrizNova[mapa.x][mapa.y+1] = mapa.matrizAtual[mapa.x][mapa.y]
                mapa.matrizNova[mapa.x][mapa.y] = Celula(NADA, None)
                return True
    return False

def colocar_bomba(player=Player):
    direcao = player.direcao
    bombaColocada = False
    if player.bombas > 0:
        match direcao:
            case 0: # up
                if mapa.matrizAtual[mapa.x-1][mapa.y].tipo == NADA: # se quadrado vazio
                    mapa.matrizNova[mapa.x-1][mapa.y] = Celula(BOMBA, Bomba(player, BOMB_TIME, BOMB_POWER)) # Colocar bomba
                    bombaColocada = True
            case 1: # down
                if mapa.matrizAtual[mapa.x+1][mapa.y].tipo == NADA: 
                    mapa.matrizNova[mapa.x+1][mapa.y] = Celula(BOMBA, Bomba(player, BOMB_TIME, BOMB_POWER))
                    bombaColocada = True
            case 2: # left
                if mapa.matrizAtual[mapa.x][mapa.y-1].tipo == NADA:
                    mapa.matrizNova[mapa.x][mapa.y-1] = Celula(BOMBA, Bomba(player, BOMB_TIME, BOMB_POWER))
                    bombaColocada = True
            case 3: # right
                if mapa.matrizAtual[mapa.x][mapa.y+1].tipo == NADA:
                    mapa.matrizNova[mapa.x][mapa.y+1] = Celula(BOMBA, Bomba(player, BOMB_TIME, BOMB_POWER))
                    bombaColocada = True
    if bombaColocada: player.bombas -= 1
    return bombaColocada

def atualizar_celula(celula=Celula):
    match celula.tipo:
        case 3: # Player
            direcao = input("Movimento(u,d,l,r):")
            match direcao:
                case "u": direcao = UP
                case "d": direcao = DOWN
                case "l": direcao = LEFT
                case "r": direcao = RIGHT
                case _: direcao = 4

            mover_jogador(direcao)
            if input("Colocar Bomba:") == "s":
                colocar_bomba(celula.objeto)

            print("bombas:" + str(celula.objeto.bombas))
        case 4: # Bomba
            explodir(celula.objeto)

def atualizar_jogo(mapa=Mapa):
    for y in range(MAP_SIZE):
        for x in range(MAP_SIZE):
            mapa.x = x
            mapa.y = y
            atualizar_celula(mapa.matrizAtual[x][y])

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
                case 3:
                    # Player
                    matriz[i][j] = Celula(PLAYER, Player(1,1,1,0,0,DOWN))
    mapa = Mapa(matriz, c.deepcopy(matriz), 0, 0)

    return mapa

def print_map(mapa=Mapa):
    for listaCelulas in mapa.matrizNova:
        for celula in listaCelulas:
            match celula.tipo:
                case 0: # Nada
                    print(COR_NADA, end="")
                case 1: # Bloco
                    print(COR_BLOCO, end="")
                case 2: # Barreira
                    print(COR_BARREIRA, end="")
                case 3: # Player
                    print(COR_PLAYER, end="")
                case 4: # Bomba
                    print(COR_BOMBA, end="")
        print(Back.RESET +"")

mapa = gerar_mapa(layout)

while True:
    print_map(mapa)
    mapa.matrizAtual = c.copy(mapa.matrizNova)
    atualizar_jogo(mapa)