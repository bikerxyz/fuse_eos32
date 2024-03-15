#include <stdio.h>
#include <stdbool.h>
#include <errno.h>


#include <fcntl.h>
#include <unistd.h>
#include <malloc.h>


int main(int argc , char **argv ) {
    //options
    //int bufferLength = 3;
    //int bufferLength = 200000000;
    int bufferLength = 4096;

    bool isError = false;
    if(argc != 3){
        printf("Error: argc has not the size of 3.\targc size: %d\n", argc);
        isError = true;
    }
    int errnoSrc;
    int errnoDest;
    char *srcPath = argv[1];
    char *destPath = argv[2];


    int srcFile = open(srcPath ,O_RDONLY);
    errnoSrc = errno ;

    int destFile = open(destPath, O_WRONLY|O_CREAT,S_IRWXU);
    if(destFile == -1 && errno== 17){
        destFile = open(destPath,O_WRONLY|O_TRUNC);
    }
    errnoDest = errno;


    if(srcFile == -1){
        printf("Error: open src has failed.\t errno: %d\n", errnoSrc);
        isError = true;
    }
    if(destFile == -1){
        printf("Error: open src has failed.\t errno: %d\n", errnoDest);
        isError = true;
    }


    if (isError){
        return 1;
    }

    bool running = true;
    size_t retFread;
    size_t retFwrite;

    char *buffer = malloc(sizeof(char) * bufferLength);
    size_t loopCounter = 0;
    size_t sizeCounter = 0;
    for(long i1 = 0; running;i1++){
        retFread = read(srcFile,buffer,bufferLength);
        loopCounter++;
        sizeCounter += retFread;
        if(retFread == 0){
            running = false;
        }else{

            retFwrite = write(destFile,buffer,bufferLength);

            if(retFwrite != bufferLength){
                printf("Error: write has make error.\t return Value: %lu; loopCounter: %lu; errno %d\n", retFwrite, loopCounter,errno);
                return 1;
            }
        }
    }


    if(retFread == -1){
        printf("Error: read has make error.\t return Value: %lu; loopCounter: %lu", errno, loopCounter);
        return 1;
    }


    printf("%lu bytes are transferred from %s to %s\n", sizeCounter, srcPath, destPath);

    if(close(srcFile) == -1){
        printf("Error: close(src) has make Error.\t errno: %d\n", errno);
    }
    if(close(destFile) == -1){
        printf("Error: close(dest) has make Error.\t errno: %d\n", errno);
    }


    printf("Copy are Successfull\nNo checks was made\n");
    return 0;
}
