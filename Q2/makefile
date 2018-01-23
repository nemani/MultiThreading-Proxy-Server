.PHONY: time clean client server
all: clean client server
clean:
	mkdir build/server -p
	-rm -rf build/client.out
	-rm -rf build/server/server.out

client: 
	clang Client/client.c -Wall -o build/client.out -fcolor-diagnostics
server:
	clang Server/server.c -Wall -o build/server/server.out -fcolor-diagnostics

time: clean client server
	cd build/server;  ./server.out& 
	cd build/; time ./client.out

