//
// Created by niklas on 02.12.19.
//


#ifndef FUSE_TRY_ERROR_H
#define FUSE_TRY_ERROR_H
#include "config.h"
#include "tools.h"
/**
 * 0 wurde resetet
 * 1 error
 *
 */
/*die set und get errno nr sind von der ersten idee der fehlerbehandlung, wird nicht mehr genutzt
 * die senderror werden noch genutzt
 *
 * die pointer sind die aktuelle Fehlerbehandlung
 */
extern jmp_buf env;
void sendError(char *format, ...);
void sendErrorExit(int exitCode, char *format, ...);
void print(char *format);
void setErrorNr(int errorNrInp);
int getErrorNr(void);
int getIsError(void);
int resetAndGetErrorNr(void);
//void setIsError(void);
void resetError(void);
extern int errorNr;
void pushPointer(void *pointer);
int *popPointer();
bool removePointer(void *pointer);
size_t getSizePointerStack();
void destroyAllPointer();



#endif //FUSE_TRY_ERROR_H
