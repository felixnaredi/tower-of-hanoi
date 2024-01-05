CFLAGS=-O3
CC=clang

main: main.c hanoi puzzle_record
	${CC} ${CFLAGS} -lncurses hanoi.o puzzle_record.o main.c -o hanoi

hanoi: hanoi.c
	${CC} ${CFLAGS} -c hanoi.c

puzzle_record: puzzle_record.c
	${CC} ${CFLAGS} -c puzzle_record.c

clean:
	rm -rf hanoi.o puzzle_record.o hanoi
