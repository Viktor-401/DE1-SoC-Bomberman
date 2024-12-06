# converte um svg em uma matriz 20x20 pra código C
# tava sem internet, não tinha nada pra fazer

lista = []
with open("Python/Sprite-0001.svg", "r") as f:
    lista = f.readlines()
    print("int matriz[20][20] = \n{")
    for i in range(20):
        print("{",end="")
        for j in range(20):
            linha = lista[(j+(i*20))+2]
            index = linha.index("#")
            r = int(linha[index+1:index+3], base= 16)//32
            g = int(linha[index+3:index+5], base= 16)//32
            b = int(linha[index+5:index+7], base= 16)//32
            print(f"0{r}{g}{b}",end=", ")
        print("}")
    print("}")