mouse:
	gcc -o mouse -pthread mouse2.c
	sudo ./mouse

main:
	gcc -o main -pthread gpu_lib.s accel_lib.c main.c 
	./main

test: 
	gcc -o test testNewSprites.c gpu_lib.s
	./test

thr: 
	gcc -o thr -pthread threads.c 
	./thr