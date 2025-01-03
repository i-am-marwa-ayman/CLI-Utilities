#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc,char * argv[]){
    if (argc == 1){
        printf("Usage: cat <file1> <file2> ...\n");
    }

    for (int i = 1; i < argc; i++){
        FILE* filePtr;
        filePtr = fopen(argv[i], "r");
        if(filePtr == NULL){
            printf("cat: somthing went wrong can't open %s file or file don't exist!\n",argv[i]);
            continue;
        } else {
            char buffer[1024];
            while (fgets(buffer, 1024, filePtr) != NULL){
                fputs(buffer, stdout);
            }
            fclose(filePtr);
        }
    }
    return 0;
}