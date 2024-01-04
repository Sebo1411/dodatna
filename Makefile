# -*- Makefile -*-

CC=gcc
FLAGS=-lws2_32 -std=c17 --static -Ofast -Wall -Wextra
dFLAGS=-lws2_32 -std=c17 --static -g -Wall -Wextra

all: bin/client.exe bin/server.exe bin/dclient.exe bin/dserver.exe

release: bin/client.exe bin/server.exe

debug: bin/dclient.exe bin/dserver.exe

bin/client.exe: intermediates/client.o intermediates/handleErr.o
	$(CC) intermediates/client.o intermediates/handleErr.o $(FLAGS) -o bin/client.exe

bin/server.exe: intermediates/server.o intermediates/handleErr.o
	$(CC) intermediates/server.o intermediates/handleErr.o $(FLAGS) -o bin/server.exe

intermediates/client.o: src/client.c src/defaults.h
	$(CC) -c src/client.c $(FLAGS) -o intermediates/client.o

intermediates/server.o: src/server.c src/defaults.h
	$(CC) -c src/server.c $(FLAGS) -o intermediates/server.o

intermediates/handleErr.o: src/handleErr.c
	$(CC) -c src/handleErr.c $(FLAGS) -o intermediates/handleErr.o



bin/dclient.exe: intermediates/dclient.o intermediates/dhandleErr.o
	$(CC) intermediates/dclient.o intermediates/dhandleErr.o $(dFLAGS) -o bin/dclient.exe

bin/dserver.exe: intermediates/dserver.o intermediates/dhandleErr.o
	$(CC) intermediates/dserver.o intermediates/dhandleErr.o $(dFLAGS) -o bin/dserver.exe

intermediates/dclient.o: src/client.c src/defaults.h
	$(CC) -c src/client.c $(dFLAGS) -o intermediates/dclient.o

intermediates/dserver.o: src/server.c src/defaults.h
	$(CC) -c src/server.c $(dFLAGS) -o intermediates/dserver.o

intermediates/dhandleErr.o: src/handleErr.c
	$(CC) -c src/handleErr.c $(dFLAGS) -o intermediates/dhandleErr.o

clean:
	del /f "intermediates\handleErr.o" "intermediates\server.o" "intermediates\client.o"

dclean:
	del /f "intermediates\dhandleErr.o" "intermediates\dserver.o" "intermediates\dclient.o"