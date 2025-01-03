#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char * argv[]){
    if(argc == 1){
        printf("wzip: file1 [file2 ...]\n");
        return 1;
    }
    char letter = '\0';
    int count = 0;
    for(int i = 1;i < argc;i++){
        FILE *srcFile;
        srcFile = fopen(argv[i], "r");
        if (srcFile == NULL){
            printf("wzip: cannot open file '%s'\n", argv[i]);
            return 1;
        }

        char ch;
        while ((ch = fgetc(srcFile)) != EOF){
            if (letter != ch){
                if (count != 0){
                    fwrite(&count, sizeof(int), 1, stdout);
                    fwrite(&letter, sizeof(char), 1, stdout);
                }
                letter = ch;
                count = 1;
            } else {
                count++;
            }
        }
        fclose(srcFile);
    }
    if (count != 0){
        fwrite(&count, sizeof(int), 1, stdout);
        fwrite(&letter, sizeof(char), 1, stdout);
    }
    return 0;
}