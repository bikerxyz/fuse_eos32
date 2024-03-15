//
// Created by niklas on 02.12.19.
//

#include "error.h"
#include "stdarg.h"
#include <stdlib.h>
#include <asm/errno.h>

jmp_buf env;
int errorNr;

int isError = 0;
void *pointerStack[POINTER_STACK_COUNT];
size_t pointerStackCounter = 0;

void sendError(char *format, ...){
    va_list vars;
    printf("\n\n\n");

    va_start(vars, format);
    vprintf(format,vars);
    va_end(vars);
    printf("\n");
}

void sendErrorExit(int exitCode, char *format, ...){
    va_list vars;
    printf("\n\n\n");
    va_start(vars, format);
    vprintf(format,vars);
    va_end(vars);
    printf("\n");

    exit(exitCode);
}

void setErrorNr(int errorNrInp){
    errorNr = errorNrInp;
    isError = 1;
}
int getErrorNr(){
    return errorNr;
}

/**
 *
 * @param error error == -1 ->  return EFAULT else return error
 * @return  gibt den Fehlercode zurück der dann an das Betriebsystem weitergegeben werden soll
 */
int convertErrorNr(int error){
    if(error == -1){
        return EFAULT;
    }else{
        return error;
    }
}

int getIsError(void){
    return isError;
}


void pushPointer(void *pointer){
    if(pointerStackCounter >= POINTER_STACK_COUNT){
        printf("pointerStackCounter Overflow\n");
        exit(1);
    }
    pointerStack[pointerStackCounter] = pointer;
    pointerStackCounter++;
}
int *popPointer(){
    pointerStackCounter--;
    return pointerStack[pointerStackCounter];
}
bool removePointer(void *pointer){
    bool isPointerFounded = false;
    for(size_t i1 = 0; i1 < pointerStackCounter;i1++){
        if(isPointerFounded){
            pointerStack[i1 - 1] = pointerStack[i1];
        }else{
            if(pointerStack[i1] == pointer){
                isPointerFounded = true;
            }
        }

    }

    if(isPointerFounded){
        pointerStackCounter--;
    }
    return isPointerFounded;
}


size_t getSizePointerStack(){
    return pointerStackCounter;
}

void destroyAllPointer(){
    size_t size = getSizePointerStack();
    for(size_t i1 = 0;i1 < size;i1++){
        free(popPointer());
    }
}

void resetError(void){
    isError = 0;
}

int resetAndGetErrorNr(void){
    resetError();
    return getErrorNr();
}