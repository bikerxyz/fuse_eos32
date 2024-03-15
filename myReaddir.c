#include <stdio.h>
#include <stdbool.h>
#include <errno.h>

#include <malloc.h>

#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>

int main(int argc, char **argv) {


    bool isError = false;
    if(argc != 2){
        printf("Error: argc has not the size of 2.\targc size: %d\n", argc);
        isError = true;
    }
    if(isError){
        return 1;
    }

    char *dirPath = argv[1];

    size_t direntCount = 10000;
    size_t direntCounter = 0;
    struct dirent *dirents = malloc(sizeof(struct dirent) * direntCount);
    DIR *dirStreamPointer = opendir(dirPath);

    if(dirStreamPointer == NULL){
        printf("error:opendir. \tpath: %s; errno: %d\n", dirPath, errno);
        return 1;
    }

    bool running = true;

    for(int i1= 0;running;i1++){
        errno = 0;
        struct dirent *dirEntry= readdir(dirStreamPointer);

        if(dirEntry != NULL){
            dirents[direntCounter++] = *dirEntry;
        }else{
            if(errno != 0){
                printf("error: readdir.\t errno: %d\n", errno);
                return 1;
            }else{
                running = false;
            }
        }

        if(direntCounter == direntCount){
            direntCount *= 4;
            dirents = realloc(dirents, sizeof(struct dirent) * direntCount);
        }
    }
    printf("first name: %s\n first File: %s\n", dirents[0].d_name, dirents[2].d_name);
    if(closedir(dirStreamPointer) == -1){
        printf("Error:closeDir. \t errno: %d\n", errno);
        return 1;
    }



    printf("Hello, World!\n");
    return 0;
}
