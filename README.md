# Bomberman 

# Diferenças do jogo original

- A bomba é posicionada na frente do player
- O movimento não é continuo, 

# Bibliotecas

`#inclued <X11/Xlib.h>` : Biblioteca para ler eventos de mouse 

# Defines
Tipos para serem usados na estrutura Celula

- NADA : 0
- BLOCO : 1
- PLAYER : 2
- BOMBA : 3
- ITEM : 4

Estados do jogo:
- EM_PROGRESSO : 0
- JOGADOR_1_WIN : 1
- JOGADOR_2_WIN : 2

# Estruturas

## Bloco
- int vida : Vida até ser explodido, vida negativa significa bloco indestrutivel
- Item * item : Ponteiro para o item que esse bloco carrega (extra)
- Cor/Sprite : Aparência do bloco (código da cor/id do sprite)

## Player

- int vida : vida do jogador
- int bombas : quantidade de bombas que o jogador pode colocar
- int raio : raio de explosão da bomba do jogador
- int velocidade : velocidade do jogador
- int sprite : aparência do jogador (id do sprite)

Extra:
- int x : posição x (posição na tela, não em relação a matriz)
- int y : posição y

Motivo para posição em relação a tela: O jogador terá movimento continuo, e não matricial como ocorria com os tetrominos

## Bomba

- int tempo : tempo para a explosão
- int raio : raio de explosão da bomba 

## Item (extra)

- int sprite : id do sprite
- int vida : itens podem ser destruidos
- int maisRaio : Quantidade de raio de explosão que adiciona a bomba
- int maisBomba : Quantidade de bombas que adiciona ao jogador
- int maisVelocidade : Quantidade de velocidade que adiciona ao jogador
- int maisVida : Quantidade de vida que adiciona ao jogador

## Celula

- int tipo : tipo da estrutura apontado pelo ponteiro
- void * ponteiroEstrutura : ponteiro para alguma das estruturas anteriores

## Mapa
Global

- Celula * matriz[20][20] : mapa do jogo, uma matriz de ponteiros para as células
- int x : posição da varredura atual da matriz
- int y : posição da varredura atual da matriz

x e y serão explicados na função de varredura(atualização de estado do jogo) da matriz

# Funções

## gerar_mapa

Gera o mapa, definindo a posição dos blocos destrutiveis e itens contidos em cada bloco

Caso itens não sejam feitos, é desnecessário, uma matriz pré carregada já é suficiente

## mover_jogador
Move o jogador na direção indicada

Parâmetros: 
- Player * jogador
- int direcao

Retorno:
- bool : true = movimento realizado

## checar_colisao
Checa se a colisão ao mover o jogador na posição indicada

Extra: Colisões com items são tratadas de maneira diferente, o jogador absorve o item

Parâmetros:
- Player * jogador
- int direcao

Retorno: 
- bool : true = há colisão (false para colisões com itens)

## explodir
Verifica se a bomba deve explodir, se não retorna false, se sim
causa dano nos objetos dentro do raio de explosão e retorna true

Parâmetros:
- Bomba * bomba

Retorno: 
- bool : true = bomba explodiu

## dano
Indentifica o objeto a ser causado dano, e subtrai a vida com o valor intensidade

Parâmetros:
- Celula * celula
- int intensidade : quanto de vida é retirada

Retorno:
- bool : true = dano causado(falso para objetos indestrutiveis (vida negativa) )

## morto
Verifica se o objeto está morto, se sim, muda o tipo da celula para NADA

Parâmetros:
- Celula * celula

Retorno: 
- bool : true = objeto morto

## atualizar_celula
Checa o tipo e realiza as ações necessárias para cada tipo de célula

Parâmetros: 
- Celula * celula

Lógica: 
tipo = Celula->tipo

if tipo == NADA or BLOCO or ITEM:
    
    // fazer nada
    return true
else if tipo == BOMBA:
    
    explodir((Bomba*)Celula->ponteiroEstrutura)
    return true
else if tipo == PLAYER:
    
    mover((Player*)Celula->ponteiroEstrutura)
    return true

// erro

return false

## varrer_matriz
Varre a matriz atualizando cada célula

Não recebe parâmetros

Retorno: 
- int estado do jogo : EM_PROGRESSO, JOGADOR_1_WIN OU JOGADOR_2_WIN


# Jogo

- Menu
    - Start
        - varrer_matriz
        - mostrar_tela
        - delay
        - Verificar estado do jogo
            - Voltar ao inicio do loop
            - ou Encerrar jogo e mostrar vencedor
        