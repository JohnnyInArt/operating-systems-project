CC = gcc

all: TCPServer TCPClient

TCPServer: TCPServer.o
	$(CC) -o TCPServer TCPServer.o

TCPClient: TCPClient.o
	$(CC) -o TCPClient TCPClient.o

TCPServer.o: TCPServer.c TCPServer.h
	$(CC)   -c TCPServer.c -o TCPServer.o

TCPClient.o: TCPClient.c TCPClient.h
	$(CC)  -c TCPClient.c -o TCPClient.o

clean:
	rm -f TCPServer TCPClient TCPServer.o TCPClient.o