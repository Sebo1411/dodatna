# -*- Makefile -*-

all: bin/client.exe bin/server.exe

bin/client.exe: intermediates/client.o intermediates/handleErr.o
	gcc intermediates/client.o intermediates/handleErr.o -lws2_32 -std=c17 --static -o bin/client.exe

bin/server.exe: intermediates/server.o intermediates/handleErr.o
	gcc intermediates/server.o intermediates/handleErr.o -lws2_32 -std=c17 --static -o bin/server.exe

intermediates/client.o: src/client.c
	gcc -c src/client.c -lws2_32 -std=c17 --static -o intermediates/client.o

intermediates/server.o: src/server.c
	gcc -c src/server.c -lws2_32 -std=c17 --static -o intermediates/server.o

intermediates/handleErr.o: src/handleErr.c
	gcc -c src/handleErr.c -lws2_32 -std=c17 --static -o intermediates/handleErr.o

clean:
	del /f "intermediates\handleErr.o" "intermediates\server.o" "intermediates\client.o"