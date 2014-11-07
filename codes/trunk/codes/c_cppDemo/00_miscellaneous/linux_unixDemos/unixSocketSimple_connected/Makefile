default:server client

server:myunixsocket.h myunixsocket.c server.c
	gcc -o $@ myunixsocket.c server.c

client:myunixsocket.h myunixsocket.c client.c
	gcc -o $@ myunixsocket.c client.c

clean:server client
	rm -rf $^
