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

coded_data **res = NULL;

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
    res[cur_chunk.ind] = malloc(sizeof(coded_data) * (cur_chunk.size + 10));
    if(res[cur_chunk.ind] == NULL){
        return NULL;
    }
    int count = 0, ptr = 0;
    char letter = '\0';

    for (int i = 0; i < cur_chunk.size; i++){
        char cur_letter = cur_chunk.raw_data[i];
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
    res[cur_chunk.ind][ptr].count = 0;
    res[cur_chunk.ind][ptr].letter = '\0';
    return NULL;
}

int main(int argc,char * argv[]){
    if(argc == 1){
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    int total_chunks = 0;
    for (int i = 1; i < argc; i++)
    {
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL){
            printf("wzip: somthing went wrong can't open %s file or file don't exist!\n",argv[i]);
            return -1;
        }
        int file_size = get_file_size(fp);
        int chunk_num = get_nprocs();

        int chunk_size = file_size / chunk_num;
        chunk chunks[chunk_num];

        char *start = mmap(NULL, file_size, PROT_READ, MAP_PRIVATE, fileno(fp), 0);
        if (start == MAP_FAILED) {
            perror("mmap failed");
            fclose(fp);
            return -1;
        }
        
        for(int j = 0;j < chunk_num;j++){
            chunks[j].ind = j + total_chunks;
            chunks[j].raw_data = start + (j * chunk_size);
            if(j == chunk_num - 1){
                chunks[j].size = file_size - chunk_size * j;
            } else {
                chunks[j].size = chunk_size;
            }
        }
        coded_data ** new_res = realloc(res, sizeof(coded_data*) * (chunk_num + total_chunks));
        if(new_res == NULL){
            return -1;
        }
        res = new_res;

        pthread_t zippers[chunk_num];
        for (int j = 0; j < chunk_num; j++) {
            pthread_create(&zippers[j], NULL, zip_chunk, (void *)&chunks[j]);
        }
        for (int j = 0; j < chunk_num; j++) {
            pthread_join(zippers[j], NULL);
        }
        total_chunks += chunk_num;
        munmap(start, file_size);
        fclose(fp);
    }
    
    for(int i = 0;i < total_chunks;i++){
        int j = 0;
        while(res[i][j].count > 0){
            if(res[i][j + 1].count == 0){
                if(i + 1 < total_chunks && res[i + 1] != NULL &&  res[i][j].letter == res[i + 1][0].letter){
                    res[i + 1][0].count += res[i][j].count;
                }
                else {
                    //printf("%c %d\n", res[i][j].letter, res[i][j].count);
                    fwrite(&res[i][j].count, sizeof(int), 1, stdout);
                    fwrite(&res[i][j].letter, sizeof(char), 1, stdout);
                }
            } else {
                //printf("%c %d\n", res[i][j].letter, res[i][j].count);
                fwrite(&res[i][j].count, sizeof(int), 1, stdout);
                fwrite(&res[i][j].letter, sizeof(char), 1, stdout);
            }
            j++;
        }
    }
    
    for(int i = 0;i < total_chunks;i++){
        if(res[i] != NULL){
            free(res[i]);
        }
    }
    if(res != NULL){
        free(res);
    }
    return 0;
}