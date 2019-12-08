CC 			= gcc
CFLAGS 		= -Wall -std=c99
LIBS 		= -lpthread
HEADERS		= $(wildcard *.h)
SOURCES 	= $(wildcard *.c)
ESOURCES 	= main.c 
OSOURCES	= $(filter-out ${ESOURCES}, ${SOURCES})
OBJS	 	= $(patsubst %.c, %.o, ${OSOURCES})
EXECS	 	= $(patsubst %.c, %.out, ${ESOURCES})

.PHONY: all clean

all: ${EXECS} ${OBJS}

%.out: ${OBJS} %.c
	${CC} ${CFLAGS} $^ ${LIBS} -o $@

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $<

clean:
	-@rm -rf *.o *.out

