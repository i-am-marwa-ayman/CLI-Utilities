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
    int file_size = ftell(fp);
    rewind(fp);
    return file_size;
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
    int allocated = 0;
    int offset = 0;
    for (int i = 1; i < argc; i++)
    {
        printf("%d:\n",i);
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
            chunks[j].ind = j + offset;
            chunks[j].raw_data = start + (j * chunk_size);
            if(j == chunk_num - 1){
                chunks[j].size = file_size - chunk_size * j;
            } else {
                chunks[j].size = chunk_size;
            }
        }
        
        if(allocated){
            res = realloc(res, sizeof(coded_data*) * (chunk_num + offset));
        } else {
            res = malloc(sizeof(coded_data*) * chunk_num);
            allocated = 1;
        }
        if(res == NULL){
            return -1;
        }
        pthread_t zippers[chunk_num];
        for (int j = 0; j < chunk_num; j++) {
            pthread_create(&zippers[j], NULL, zip_chunk, (void *)&chunks[j]);
        }
        for (int j = 0; j < chunk_num; j++) {
            pthread_join(zippers[j], NULL);
        }
        offset += chunk_num;
        munmap(start, file_size);
        fclose(fp);
    }
    int i = 0;
    printf("%d\n", offset);
    for(int i = 0;i < offset;i++){
        int j = 0;
        while(res[i][j].count > 0){
            printf("letter %c : %d\n",res[i][j].letter, res[i][j].count);
            j++;
        }
    }
    return 0;
}