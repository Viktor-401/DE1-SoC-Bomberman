import copy

class a:
    b = 1
    c = 2

    def __init__(self, b, c) -> None:
        self.b = b 
        self.c = c

class b:
    e = 0

    def __init__(self) -> None:
        self.e = 12

def f(a):
    a.b = 5

lista = [1,2,3]
b1 = b()
a1 = a(b1,2)
b1.e = 5

print(a1.b.e)