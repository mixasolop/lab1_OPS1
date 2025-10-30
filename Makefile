CC=gcc
C_FLAGS=-Wall -g -Wno-unused
L_FLAGS=-fsanitize=address,undefined,leak

TARGET=sop-bib
FILES=${TARGET}.o

.PHONY: clean all

${TARGET} : ${FILES}
	${CC} ${L_FLAGS} -o ${TARGET} ${FILES}

${TARGET}.o: ${TARGET}.c
	${CC} ${C_FLAGS} -o ${TARGET}.o -c ${TARGET}.c

all: ${TARGET}

clean:
	rm -f ${FILES} ${TARGET}