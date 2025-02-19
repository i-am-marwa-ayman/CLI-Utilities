#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>

struct chunk {
    int size;
    char *raw_data;
    char *coded_data;
};

int get_file_size(FILE *fp){
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    return res;
}

int main(int argc,char * argv[]){
    if(argc == 1){
        printf("Usage:pzip <file1> <file2> ...]\n");
        return 1;
    }
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL){
            printf("pzip: somthing went wrong can't open %s file or file don't exist!\n",argv[i]);
            return -1;
        }
        int file_size = get_file_size(fp);
        int chunk_num = get_nprocs();

        int chunk_size = file_size / chunk_num;
        struct chunk chunks[chunk_num];

        char *start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
        for(int j = 0;j < chunk_num;j++){
            chunks[j].raw_data = start + (j * chunk_size);
            chunks[j].coded_data = NULL;
            if(j == chunk_num - 1){
                chunks[j].size = file_size - chunk_size * j;
            } else {
                chunks[j].size = chunk_size;
            }
        }
        for (int j = 0; j < chunk_num; j++) {
            for (int c = 0; c < chunks[j].size; c++) {
                putchar(chunks[j].raw_data[c]);
            }
        }
        
        munmap(start, file_size);
        fclose(fp);
    }
    return 0;
}