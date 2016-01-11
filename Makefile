DIR_INC = ./include
DIR_SRC = ./src
DIR_OBJ = ./obj
DIR_BIN = ./bin

SRC = $(wildcard ${DIR_SRC}/*.c)  
OBJ = $(patsubst %.c,${DIR_OBJ}/%.o,$(notdir ${SRC})) 

TARGET = main

BIN_TARGET = ${DIR_BIN}/${TARGET}

CC = gcc
CFLAGS = -g -Wall -I${DIR_INC}

${BIN_TARGET}:${OBJ}
	$(CC) $(OBJ) -I/usr/include/mysql  -L/usr/lib/mysql -lmysqlclient -o $@ -pthread
    
${DIR_OBJ}/%.o:${DIR_SRC}/%.c
	$(CC) $(CFLAGS)  -I/usr/include/mysql  -L/usr/lib/mysql -lmysqlclient -c $< -o $@  -pthread
.PHONY:clean
clean:
	rm -f ./obj/*
	rm -f ./bin/*