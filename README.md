# Bomberman | Sistemas Digitais (TP01) 

<p align="center">
  <img src="imagens/top45_01.jpg" width = "600" />
</p>
<p align="center"><strong>Kit de desenvolvimento DE1-SoC</strong></p>


<h2>  Componentes da Equipe: <br></h2>
<uL> 
  <li><a href="https://github.com/Silva-Alisson">Alisson Silva</a></li>
  <li><a href="https://github.com/DaviOSC">Davi Oliveira</a></li>
  <li><a href="https://github.com/kauaQuintella">Kauã Quintella</a></li>
  <li><a href="https://github.com/Viktor-401">Sinval Victor</a></li>
</ul>

<div align="justify"> 

## Introdução

Este relatório técnico apresenta o desenvolvimento de um jogo usando biblioteca em Assembly para uma GPU do VGA e a biblioteca do acelerômetro ADXL345 na plataforma de desenvolvimento DE1-SoC com arquitetura ARMv7A. Estas bibliotecas foram ultilizada para rodar o jogo em estilo Bomberman. E para isso, foi necessário a aplicação prática dos conceitos de threads, programação em C e Assembly. Ao decorrer da leitura, é descrito o código do jogo, incluindo a explicação dos métodos.

## Fundamentação Teórica

### Kit de desenvolvimento DE1-SoC

A placa que foi usada para executar o jogo possue uma arquitetura baseada na *Altera System-on-Chip* (SoC) FPGA, que combina um Cortex-A9 dual core com cores embarcados com lógica programável. Nela vem integrado o *HPS* (*Hard Processor System*) baseado em ARM, consistindo no processador, periféricos como o acelerômetro (ADLX456) ultilizado e a interface de memória. O sistema do Hard Processor vem ligado perfeitamente à estrutura da FPGA usando um backbone interconectado de alta-bandalarga. (DE1-SoC Manual, 2019)

Ademais, o DE1-SoC possui as seguintes especificações gerais, de acordo com o FPGA Academy:

- Main Features:
    - Intel® Cyclone V SoC FPGA
        - 85k logic-element FPGA
        - ARM Cortex A9 MPCORE
    - 1 GB DDR, 64 MB SDRAM
    - Micro SD Card


- Basic IO:
    - 10 slide switches, 4 pushbuttons
    - 10 LEDs, six 7-segment displays

- IO Devices:
    - Audio in/out
    - VGA Video out, S-Video in
    - Ethernet, USB, Accelerometer
    - A/D converter, PS/2

<p align="center">
  <img src="imagens/kitDesenvolvimentoTopView.png" width = "800" />
</p>
<p align="center"><strong>Layout e componentes do DE1-SoC</strong></p>


### G-Sensor ADXL345

Esse sensor é um acelerômetro de 3 eixos, que realiza medições de alta resolução. A saída digitalizada é formatada em 16 bits com complemento de dois e pode ser acessada via interface I2C e seu endereço é 0x53.

Para a comunicação com o acelerômetro, informações obtidas no datasheet do ADXL345 e nas aulas de Arquitetura de Computadores foram de extrema importância. 

### Protocolo I2C

O I2C (*Inter-Integrated Circuit*), é um protocolo de comunicação serial síncrono, bastante utilizado na interação entre dispositivos periféricos e processadores ou microcontroladores. A comunicação ocorre utilizando dois fios: o SDA, que transporta os dados, e o SCL, responsável pelo sinal de clock que sincroniza a troca de informações. Quando aplicamos isso no G-Sensor, ele opera como um dispositivo *slave* dentro do barramento, enquanto o processador atua como *master*, controlando toda a comunicação e o envio de comandos.

Cada dispositivo conectado ao barramento I2C possui um endereço de 7 bits, o que facilita a identificação. A interação acontece quando o processador, na função de master, envia o endereço do acelerômetro e, a partir daí, realiza operações de leitura ou escrita nos registradores internos, permitindo, por exemplo, a configuração de parâmetros ou a coleta de dados do sensor.

### Linguagem C para o jogo e biblioteca do acelerômetro

O jogo foi elaborado em lingagem C por ser um requisito do problema, sendo usado o GCC para a compilação. A IDE ultilizada foi o Visual Studio Code.

### Linguagem Assembly para arquitetura ARMv7A

Para acessar a placa gráfica e usar as suas funcionalidades, foi usada a linguagem Assembly para a arquitetura da placa.

##### De acordo com o _ARM Architecture Reference Manual_:

O conjunto de instruções ARM é um conjunto de instruções de 32 bits que fornece funções abrangentes de processamento de dados e controle. 

O conjunto de instruções Thumb foi desenvolvido como um conjunto de instruções de 16 bits, com um subconjunto das funcionalidades do conjunto de instruções ARM. Ele oferece uma densidade de código significativamente melhorada, ao custo de uma pequena redução no desempenho. Um processador que executa instruções Thumb pode alternar para a execução de instruções ARM em segmentos críticos para o desempenho, especialmente para lidar com interrupções.

Perfil de Aplicação ARMv7-A:

- Implementa uma arquitetura ARM tradicional com múltiplos modos.
- Suporta uma Arquitetura de Sistema de Memória Virtual (VMSA) baseada em uma Unidade de Gerenciamento de Memória (MMU). Uma implementação ARMv7-A pode ser chamada de implementação VMSAv7.
- Suporta os conjuntos de instruções ARM e Thumb.

##### Instruções usadas para a biblioteca:

1. **`.section`, `.align`, `.ascii`, `.word`, `.zero`**: São diretivas de montagem, não instruções de máquina. Elas especificam detalhes sobre como organizar os dados e onde colocá-los na memória.

2. **`push` e `pop`**: Gerenciam a pilha. 

3. **`ldr`**: Carrega um valor na memória para um registrador.

4. **`str`**: Armazena o valor de um registrador em uma posição da memória.

5. **`mov`, `movw`, `movt`**: Movem valores para registradores.

6. **`svc` e `swi`**: Executam chamadas ao sistema (syscalls).

7. **`cmp` e `bne`/`beq`**: Comparam registradores e desviam condicionalmente.

8. **`add`, `sub`**: Realizam operações de adição e subtração entre registradores.

9. **`mul`**: Multiplicação.

10. **`lsl` e `lsr`**: Realizam deslocamentos lógicos (bit shifts).

11. **`orr` e `and`**: Realizam operações de OR e AND bit-a-bit.

12. **`bx lr`**: Retorna de uma função, usando o registrador de link (`lr`). 

Cada instrução desempenha um papel importante, desde configurar chamadas ao sistema para manipulação de memória até operações específicas de bits, necessários para o controle de hardware ou configuração de cores no sistema.

### Arquitetura Baseada em Sprites para criação de Jogos 2D

<p align="center">
  <img src="imagens/arquiteturaVGA.png" width = "800" />
</p>
<p align="center"><strong>Representação da Arquitetura.</strong></p>

A arquitetura desenvolvida para o projeto é baseada em sprites e voltada para criação de jogos 2D em FPGAs, aproveitando o padrão VGA para exibir gráficos. A estrutura é composta por um processador principal (Nios II) para executar a lógica do jogo em C, e um processador gráfico responsável pela renderização dos sprites e elementos visuais. O sistema inclui memórias dedicadas para armazenamento dos sprites e do background, permitindo atualização rápida e eficiente dos gráficos. Um co-processador em estrutura pipeline auxilia na criação de polígonos (como quadrados e triângulos) e na análise de colisão entre elementos da tela, operando de forma paralela e em tempo real. Essa arquitetura modular permite que jogos sejam desenvolvidos e controlados por meio de uma API, abstraindo detalhes de baixo nível e facilitando a interação com os elementos gráficos.

### Monitor VGA

O monitor empregado no projeto foi o DELL M782p, um modelo CRT que utiliza um tubo de raios catódicos para gerar imagens. Com uma tela de 17 polegadas e resolução máxima de 1280x1024 pixels, ele possui uma interface VGA para conexão com o computador ou uma placa de desenvolvimento. Monitores CRT são conhecidos por sua reprodução de cores intensas e rápida resposta, sendo uma escolha apropriada para projetos que necessitam de interação em tempo real, como jogos e simulações.

### "Bomberman"

<p align="center">
  <img src="imagens/bombermanLogo.png" width = "800" />
</p>
<p align="center"><strong>Logo do jogo original.</strong></p>

Segundo a Wikipédia, Bomberman é uma série de jogos de estratégia criada pela Hudson Soft. O objetivo é completar as fases usando bombas de forma estratégica para destruir obstáculos e inimigos. No projeto, o jogo inspirado foi feito obedecendo estas funcionalidades:

1. O jogador pode por uma bomba para eliminar seu inimigo ou para quebrar paredes;
2. A bomba tem um tempo para explodir, quando finalizado, ela pode detonar outras bombas, destruir paredes quebráveis e causar danos aos jogadores.
3. A bomba é posicionada na frente do player.
4. O movimento do player não é continuo.




## Descrição de alto nível

### Explicação da <a href="https://github.com/Viktor-401/DE1-SoC-Bomberman/blob/master/DE1-SoC-Bomberman-master/C/main.c">main.c</a>

O projeto **Bomberman** é uma adaptação simplificada e personalizada do clássico jogo, com diferenças notáveis em sua mecânica e estrutura de código. Aqui está uma visão geral de alto nível de como o jogo foi implementado com base nas informações fornecidas:



#### Estrutura Geral

A base do jogo é composta por um mapa dividido em células, representado por uma matriz 20x20. Cada célula pode conter diferentes tipos de objetos como blocos, jogadores, bombas e itens. O jogo é processado em ciclos contínuos, onde cada célula do mapa é atualizada, permitindo interações dinâmicas e em tempo real.

#### Componentes Principais

##### **Mapa**
O mapa é representado por uma matriz de células (`Celula`), onde cada célula possui um tipo e um ponteiro para uma estrutura específica:
- **NADA**: Células vazias.
- **BLOCO**: Obstáculos com vida que podem ser destruídos (ou não, no caso de blocos indestrutíveis).
- **PLAYER**: Representam os jogadores no jogo, com atributos como vida, bombas disponíveis, velocidade, entre outros.
- **BOMBA**: Objetos colocados pelos jogadores que explodem após um determinado tempo.
- **ITEM**: Melhorias coletáveis que aumentam atributos como raio de explosão, número de bombas, vida, ou velocidade.

##### **Estruturas**
Cada elemento do jogo é representado por uma estrutura específica:
- **Bloco**: Elementos destrutíveis ou indestrutíveis do cenário.
- **Player**: Contém atributos para controlar a jogabilidade, como vida, bombas, raio de explosão, velocidade e aparência.
- **Bomba**: Define o comportamento das explosões, com tempo e raio de efeito.
- **Item**: Itens que concedem bônus ao jogador, como mais vida, velocidade ou bombas.

#### Principais Funções

##### **Interações Fundamentais**
- **mover_jogador()**: Controla o movimento dos jogadores, verificando colisões com outros objetos.
- **checar_colisao()**: Verifica se há obstruções ou itens no caminho do jogador.
- **explodir()**: Garante que as bombas causem dano aos objetos dentro do raio de explosão quando o tempo se esgota.
- **dano()**: Aplica dano a objetos, ajustando suas vidas ou marcando-os como destruídos.
- **morto()**: Remove objetos do jogo quando sua vida atinge zero.

##### **Atualização do Jogo**
- **atualizar_celula()**: Determina as ações necessárias com base no tipo de objeto presente em uma célula (mover jogadores, explodir bombas, etc.).
- **varrer_matriz()**: Atualiza todas as células do mapa e verifica o estado do jogo, retornando se ele continua ou se algum jogador venceu.

##### **Gerenciamento do Mapa**
- **gerar_mapa()**: Inicializa o cenário do jogo, posicionando blocos destrutíveis e, opcionalmente, itens.



#### **Fluxo do Jogo**
1. **Menu Inicial**: O jogador inicia o jogo a partir de uma tela de menu.

<p align="center">
  <img src="imagens/mainScreen.gif" width = "800" />
</p>
<p align="center"><strong>Menu inicial com a logo do jogo.</strong></p>


2. **Ciclo de Jogo**:
   - A matriz do mapa é varrida e atualizada.
   - O estado do jogo é verificado.
   - A tela é atualizada para refletir as mudanças.
   - Um pequeno atraso é aplicado antes de repetir o ciclo.

<p align="center">
  <img src="imagens/inGame.gif" width = "800" />
</p>
<p align="center"><strong>Jogo funcionando.</strong></p>

3. **Condição de Vitória**:
   - O jogo termina quando apenas um jogador permanece vivo, determinando o vencedor.

<p align="center">
  <img src="imagens/p1Win.gif" width = "800" />
</p>
<p align="center"><strong>Jogador 1 ganhando o jogo.</strong></p>

### Como as Threads Funcionariam no Fluxo do Jogo
#### Inicialização

No projeto, a estrutura inicial do programa foi projetada para distribuir as tarefas principais entre diferentes threads, otimizando a organização e a execução simultânea das funcionalidades. Foram criadas quatro threads principais, com responsabilidades bem definidas:

1. **Menu Inicial**Thread Principal (Main): Responsável pelo controle geral do jogo, incluindo a inicialização dos componentes, a coordenação entre as demais threads e a gestão do loop principal de execução.

2. **Menu Inicial**Thread para o Controle do Mouse: Dedicada à captura e processamento dos movimentos do mouse, permitindo sua integração como uma entrada do jogador no sistema. Essa thread monitora e interpreta os eventos do dispositivo para atualizar o estado do jogo em tempo real.

3. **Menu Inicial**Thread para o Acelerômetro: Focada no gerenciamento dos dados do acelerômetro da placa DE1-SoC, realizando a leitura, o processamento e a calibração dos valores capturados. Essa funcionalidade foi crucial para mapear o movimento da placa às ações dentro do jogo.

4. **Menu Inicial**Thread para os Botões da Placa: Encarga de monitorar os estados dos botões disponíveis na DE1-SoC, reconhecendo pressões e liberações em tempo real. Essa thread gerencia as entradas físicas adicionais, sincronizando as ações dos jogadores com as mecânicas do jogo.

Essa arquitetura permitiu a execução eficiente das tarefas, com cada thread operando de forma independente, mas sincronizada, garantindo a interação fluida entre o jogador e o sistema.
#### Interação Contínua

O jogador insere comandos (através de eventos de input na thread de entrada).
A thread de atualização processa os comandos e atualiza as células da matriz.
Bombas que são ativadas têm sua thread própria, que monitora o temporizador e aciona a explosão na hora certa.
#### Renderização em Paralelo

Enquanto as lógicas do jogo estão sendo processadas, a thread de renderização continua desenhando o estado atualizado na tela.

### Explicação detalhada das funções da <a href="https://github.com/Viktor-401/DE1-SoC-Bomberman/blob/master/DE1-SoC-Bomberman-master/C/main.c">main.c</a>

#### `gerar_mapa() : void`
- **Propósito**: Inicializa o mapa do jogo, posicionando blocos destrutíveis e itens em locais predeterminados ou aleatórios.
- **Implementação**:
  - A função configura as células da matriz global `matriz[20][20]`.
  - Cada célula recebe um tipo e, se necessário, é associada a uma estrutura correspondente (como `Bloco` ou `Item`).
  - Se itens não forem implementados, uma matriz pré-configurada já é suficiente.

---

#### `mover_jogador(Player* jogador, int direcao) : bool`
- **Propósito**: Move o jogador na direção especificada.
- **Implementação**:
  - Verifica se o movimento é permitido usando a função `checar_colisao`.
  - Atualiza as coordenadas `x` e `y` do jogador com base na direção.
  - Retorna `true` se o movimento for realizado com sucesso, ou `false` caso contrário.

---

#### `checar_colisao(Player* jogador, int direcao) : bool`
- **Propósito**: Verifica se há algum obstáculo no caminho do jogador.
- **Implementação**:
  - Calcula a nova posição do jogador com base na direção.
  - Examina o conteúdo da célula na posição alvo.
  - Retorna:
    - `true` se houver colisão com objetos sólidos (como blocos).
    - `false` para colisões com itens (o jogador os coleta).

---

#### `explodir(Bomba* bomba) : bool`
- **Propósito**: Lida com a explosão de uma bomba e seus efeitos no ambiente.
- **Implementação**:
  - Verifica se o tempo da bomba chegou a zero.
  - Se sim, aplica danos em objetos dentro do raio de explosão, com base na matriz global.
  - Retorna:
    - `true` se a bomba explodiu.
    - `false` caso contrário.

---

#### `dano(Celula* celula, int intensidade) : bool`
- **Propósito**: Aplica dano ao objeto dentro de uma célula específica.
- **Implementação**:
  - Identifica o tipo do objeto na célula.
  - Reduz a vida do objeto pelo valor de `intensidade`.
  - Retorna:
    - `true` se o dano foi causado com sucesso.
    - `false` se o objeto for indestrutível (vida negativa).

---

#### `morto(Celula* celula) : bool`
- **Propósito**: Verifica se um objeto em uma célula foi destruído.
- **Implementação**:
  - Examina a vida do objeto na célula.
  - Se a vida for menor ou igual a zero, o tipo da célula é alterado para `NADA`.
  - Retorna:
    - `true` se o objeto foi destruído.
    - `false` caso contrário.

---

#### `atualizar_celula(Celula* celula) : bool`
- **Propósito**: Atualiza o estado de uma célula com base no tipo de objeto que ela contém.
- **Implementação**:
  - Identifica o tipo da célula.
  - Realiza a ação correspondente:
    - `NADA` ou `BLOCO`: nenhuma ação.
    - `BOMBA`: chama a função `explodir`.
    - `PLAYER`: chama a função `mover`.
  - Retorna:
    - `true` se a célula foi atualizada com sucesso.
    - `false` em caso de erro.

---

#### `varrer_matriz() : int`
- **Propósito**: Percorre a matriz do jogo, atualizando o estado de cada célula.
- **Implementação**:
  - Itera por todas as células da matriz `matriz[20][20]`.
  - Chama a função `atualizar_celula` para cada célula.
  - Avalia o estado do jogo:
    - Retorna `EM_PROGRESSO` se o jogo continuar.
    - Retorna `JOGADOR_1_WIN` ou `JOGADOR_2_WIN` quando um vencedor for determinado.

---

### Funções auxiliares e lógica de estado

#### Fluxo do jogo no menu
- **Menu principal**:
  - O jogador pode iniciar o jogo.
  - Durante o jogo:
    - **Atualizações**: Chamadas constantes de `varrer_matriz` e `mostrar_tela`.
    - **Delay**: Insere pausas entre ciclos de atualização para sincronizar as ações.
    - **Verificação de estado**: Determina se o jogo continua ou termina.

### Como executar o projeto?

Após baixar os arquivos da pasta `source`, basta executar o arquivo makefile usando `make main` no terminal.

      #make main
      main:
        gcc -o main -pthread gpu_lib.s accel_lib.c main.c 
        ./main


<div align="justify">

### Telas

<p align="center">
  <img src="imagens/p1Win.jpeg" width = "800" />
</p>
<p align="center"><strong>Vitória do Player 1.</strong></p>
<p align="center">
  <img src="imagens/p2Win.jpeg" width = "800" />
</p>
<p align="center"><strong>Vítoria do Player 2.</strong></p>
<p align="center">
  <img src="imagens/drawn.jpeg" width = "800" />
</p>
<p align="center"><strong>Empate entre os jogadores.</strong></p>
<p align="center">
  <img src="imagens/mainScreen.jpeg" width = "800" />
</p>
<p align="center"><strong>Main screen do jogo.</strong></p>


## Conclusão 

O projeto Bomberman na DE1-SoC utilizou uma arquitetura baseada em threads, onde a thread principal (main) gerenciava o loop do jogo e coordenava outras threads dedicadas, como a do mouse, que captava movimentos do jogador; a do acelerômetro, que processava dados do sensor ADXL345 para controle adicional; e a dos botões, que monitorava ações físicas como movimentação e colocação de bombas.

A lógica do jogo foi implementada em C, enquanto o controle do hardware gráfico e periféricos foi feito em Assembly, utilizando instruções específicas da arquitetura ARMv7A para otimizar a movimentação de sprites e a detecção de colisões. O acelerômetro ADXL345 foi integrado via protocolo I2C, permitindo uma interação inovadora com dados de movimento em três eixos.

A renderização gráfica foi realizada por sprites e background block, o que facilitou a criação de elementos visuais, como jogadores, blocos, bombas e explosões. A matriz 20x20 que representava o mapa gerenciava o estado de cada célula, garantindo interações consistentes dentro da mecânica tradicional do Bomberman, como a colocação de bombas, destruição de blocos e confronto entre jogadores.

## Bibliografia

Using the Accelerometer on DE-Series Boards. Disponível em: https://github.com/fpgacademy/Tutorials/releases/download/v21.1/Accelerometer.pdf. Acessado em: 23 de setembro de 2024.

TERASIC. DE1-SoC User-Manual. Disponível em: https://drive.google.com/file/d/1HzYtd1LwzVC8eDobg0ZXE0eLhyUrIYYE/view. Acessado em: 26 de setembro de 2024.

FPGA Academy. Disponível em: https://fpgacademy.org/. Acessado em: 26 de setembro de 2024.

SOUZA, Fábio. Comunicação I2C. Postado em: 03 de janeiro de 2023. Disponível em: https://embarcados.com.br/comunicacao-i2c/. Acessado em: 26 de setembro de 2024.

PATTERSON, David A.; HENNESSY, John L. Computer Organization and Design: The Hardware Software Interface, ARM Edition. 2016. Morgan Kaufmann. ISBN: 978-0-12-801733-3.

ARM. ARM Architecture Reference Manual ARMv7-A and ARMv7-R edition. Disponível em: https://developer.arm.com/documentation/ddi0406/latest. 

https://pt.wikipedia.org/wiki/Bomberman
