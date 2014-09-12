default:all

all:mmap1 mmap2 mmap3_serv mmap3_client

mmap1: mmap1.c shmapi.c shmapi.h
	gcc -o $@ $^

mmap2: mmap2.c shmapi.c shmapi.h
	gcc -o $@ $^

mmap3_serv: mmap3_serv.c shmapi.c shmapi.h
	gcc -o $@ $^

mmap3_client: mmap3_client.c shmapi.c shmapi.h
	gcc -o $@ $^

.PHONY:clean
clean:
	@touch mmap1 mmap2 mmap3_serv mmap3_client mmap_shared
	@rm mmap1 mmap2 mmap3_serv mmap3_client mmap_shared
