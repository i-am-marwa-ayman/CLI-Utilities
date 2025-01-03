#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char * argv[]){
    if(argc == 1){
        printf("Usage: unzip <file1> <file2> ...]\n");
        return 1;
    }
    for(int i = 1;i < argc;i++){
        FILE *file;
        file = fopen(argv[i], "rb");
        if (file == NULL){
            printf("unzip: somthing went wrong can't open %s file or file don't exist!\n", argv[i]);
            return 1;
        }
        size_t bytesRead;
        int count;
        while(fread(&count, sizeof(int), 1, file) == 1){
            char letter;
            fread(&letter, sizeof(char), 1, file);
            for(int j = 0;j < count;j++){
                printf("%c", letter);
            }
        }
        fclose(file);
    }
}