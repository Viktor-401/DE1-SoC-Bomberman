# converte um svg em uma matriz 20x20 pra código C
# tava sem internet, não tinha nada pra fazer
LARGURA = 20
ALTURA = 27

lista = []
with open("DE1-SoC-Bomberman-master\Python\sprite7.svg", "r") as f:
    lista = f.readlines()
    print("int matriz[ALTURA][LARGURA] = \n{")
    for i in range(ALTURA):
        print("{",end="")
        for j in range(LARGURA):
            linha = lista[(j+(i*LARGURA))+2]
            index = linha.index("#")
            r = int(linha[index+1:index+3], base= 16)//32
            g = int(linha[index+3:index+5], base= 16)//32
            b = int(linha[index+5:index+7], base= 16)//32
            print(f"0{r}{g}{b}",end=", ")
        print("},")
    print("}")


