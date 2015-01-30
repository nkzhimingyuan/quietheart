#include<stdio.h>
#include<stdlib.h>
#include<sys/mman.h>

const int alloc_size = 32 * 1024 * 1024;//32M
int main()
{
        char *memory = malloc(alloc_size);
        if(mlock(memory,alloc_size) == -1)
		{//mlock please refer to the man page. it locks the memory(page unit) for not to swap. needs root authority
                perror("mlock");
                return (-1);
        }
        size_t i;
        size_t page_size = getpagesize();
        for(i=0;i<alloc_size;i+=page_size) 
		{
                printf("i=%zd\n",i);
                memory[i] = 0;
        }

        if(munlock(memory,alloc_size) == -1) 
		{
                perror("munlock");
                return (-1);
        }

        return 0;
}

