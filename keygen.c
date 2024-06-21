#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]){
    const char randChars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";

    if(argc != 2){
        fprintf(stderr, "keylength\n", argv[0]);
        return 1;
    }

    int length = atoi(argv[1]); // string to int 

    if (length <= 0){
        fprintf(stderr, "length must be postive");
        return 1;
    }

    //generateKey(length);
    int numofChars = sizeof(randChars) - 1;

    for(int i =0; i < length; i++){
        int index = rand() % numofChars;
        putchar(randChars[index]);
    }

    putchar('\n');
  

    return 0;
}