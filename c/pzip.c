#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/sysinfo.h>
#include <sys/mman.h>



typedef struct {
    int count;
    char letter;
} coded_data;

coded_data **res;
typedef struct {
    int ind;
    int size;
    char *raw_data;
} chunk;

int get_file_size(FILE *fp){
    fseek(fp, 0L, SEEK_END);
    int res = ftell(fp);
    rewind(fp);
    return res;
}

void *zip_chunk(void *args){
    chunk cur_chunk = *(chunk*)args;
    res[cur_chunk.ind] = malloc(sizeof(coded_data) * cur_chunk.size);
    int count = 0;
    char letter = '\0';
    int ptr = 0;
    for (int c = 0; c < cur_chunk.size; c++){
        char cur_letter = cur_chunk.raw_data[c];
        if (letter != cur_letter){
            if (count != 0){
                coded_data new_letter;
                new_letter.count = count;
                new_letter.letter = letter;
                res[cur_chunk.ind][ptr++] = new_letter;
            }
            letter = cur_letter;
            count = 1;
        } else {
            count++;
        }
    }
    if (count != 0){
        coded_data new_letter;
        new_letter.count = count;
        new_letter.letter = letter;
        res[cur_chunk.ind][ptr++] = new_letter;
    }
    return NULL;
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
        chunk chunks[chunk_num];

        char *start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
        for(int j = 0;j < chunk_num;j++){
            chunks[j].ind = j;
            chunks[j].raw_data = start + (j * chunk_size);
            if(j == chunk_num - 1){
                chunks[j].size = file_size - chunk_size * j;
            } else {
                chunks[j].size = chunk_size;
            }
        }

        res = malloc(sizeof(coded_data*) * chunk_num);
        pthread_t zippers[chunk_num];
        for (int j = 0; j < chunk_num; j++) {
            pthread_create(&zippers[j], NULL, zip_chunk, (void *)&chunks[j]);
        }
        for (int j = 0; j < chunk_num; j++) {
            pthread_join(zippers[j], NULL);
        }
        for(int j = 0;j < chunk_num;j++){
            for(int k = 0;k < chunks[j].size;k++){
                if(res[j][k].count > 0){
                    printf("letter %c count %d\n",res[j][k].letter,res[j][k].count);
                }
            }
        }
        
        
        munmap(start, file_size);
        fclose(fp);
    }
    return 0;
}