EXECS= k_means
CC=gcc
MPICC=mpic++

FLAGS=-O3 -Wall -g -lm -fopenmp

all: ${EXECS}

k_means: main.cpp
	${MPICC} -o main $^ ${FLAGS} `pkg-config --cflags --libs opencv`

clean:
	rm -r main main.dSYM
