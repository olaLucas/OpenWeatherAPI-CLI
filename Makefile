SOURCES = main.c $(LIB_OUTPUT)
TARGET = main
CC = gcc 

LIBS_CURL != curl-config --libs
LIBS_CJSON != pkg-config libcjson --libs --cflags

main : request.o main.o
	$(CC) bin/main.o bin/request.o -g -Wall $(LIBS_CJSON) $(LIBS_CURL) -o $(TARGET) 

main.o : src/main.c
	$(CC) src/main.c -c -g -Wall -o bin/main.o

request.o :  libs/request.c libs/request.h
	$(CC) libs/request.c -c -g -Wall -o bin/request.o

clean :
	rm $(TARGET)
	rm main.o
	rm bin/request.o
