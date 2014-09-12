default:all

all:serv client

serv: serv.c shmapi.c shmapi.h
	gcc -o $@ $^
client: client.c shmapi.c shmapi.h
	gcc -o $@ $^

.PHONY:clean
clean:
	@touch serv client
	@rm serv client
