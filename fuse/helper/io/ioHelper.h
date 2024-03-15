//
// Created by niklas on 02.12.19.
//

#include <bits/types/FILE.h>
#include "../config.h"

#ifndef FUSE_TRY_IOHELPER_H
#define FUSE_TRY_IOHELPER_H

#endif //FUSE_TRY_IOHELPER_H

#define UINT_TO_UCHARPOINTER(p)  ((unsigned char *) &(p))
#define UCHARPOINTER_TO_UINTPOINTER(p) ((unsigned int *) (p))


#include "../datatypes.h"
#include "../error.h"

extern FILE *disk;
extern EOS32_daddr_t fsStart;

FILE *openDisk(char *path, char *modus);

void closeDisk(FILE *fp);

//big endian to little endian and  little endian to big endian
unsigned int flip4ByteNumber(unsigned int number);


unsigned int get4Bytes(unsigned char *addr);
void set4Bytes(unsigned char *addr, unsigned int value);
void readBlock(FILE *disk, EOS32_daddr_t blockNum, unsigned char * blockBuffer);
void writeBlock(FILE *disk, EOS32_daddr_t blockNum, unsigned char *blockBuffer);
int readBytesDisk(FILE *disk, unsigned long offset, unsigned int size, unsigned char *content);
void writeBytesDisk(FILE *disk, unsigned long offset, size_t size, unsigned char *content);
void writeflipp4Bytes(FILE *disk, EOS32_daddr_t blockNum, unsigned int offset, unsigned int content);
unsigned int readFlipp4Bytes(FILE *disk, EOS32_daddr_t blockNum, int offset);
void writeByte(FILE *disk, EOS32_daddr_t blockNum, int offset, unsigned char content);
int readBytes(FILE *disk, EOS32_daddr_t blockNum, unsigned int offset, unsigned int size, unsigned char *content);
void writeBytes(FILE *disk, EOS32_daddr_t blockNum, int offset, unsigned int size, unsigned char *content);
unsigned long get8Bytes(unsigned char *p);
