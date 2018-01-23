all: clean client server
clean:
	mkdir build/server -p
	-rm -rf build/client.out
	-rm -rf build/server/server.out

client: 
	clang client.c -Wall -o build/client.out -fcolor-diagnostics
server:
	clang server.c -Wall -o build/server/server.out -fcolor-diagnostics
