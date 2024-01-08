CFLAGS=-O3 -Wall
CC=clang

main: main.c hanoi record
	${CC} ${CFLAGS} -lncurses hanoi.o record.o main.c -o hanoi

hanoi: hanoi.c
	${CC} ${CFLAGS} -c hanoi.c

record: record.c
	${CC} ${CFLAGS} -c record.c

clean:
	rm -rf hanoi.o record.o hanoi *.dSYM
