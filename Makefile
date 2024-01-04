CFLAGS=-O3
CC=clang

main: main.c hanoi
	${CC} ${CFLAGS} -lncurses hanoi.o main.c -o hanoi

hanoi: hanoi.c
	${CC} ${CFLAGS} -c hanoi.c

clean:
	rm -rf hanoi.o hanoi
