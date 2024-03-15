//
// Created by niklas on 02.12.19.
//

#include "ioHelper.h"
FILE *disk;
FILE *disk;
EOS32_daddr_t fsStart;

FILE *openDisk(char *path, char *modus){
    FILE *ou;
    ou = fopen(path, modus);
    if(ou == NULL){
        sendError("Can not open File at the path :\"%s\" with the modus: \"%s\"\n", path, modus);
        longjmp(env,errno);
    }

    return ou;
}


void closeDisk(FILE *fp){
    int res = fclose(fp);
    if(res != 0){
        sendError("Can not close the FILE pointer: \"%p\"", (void *) fp);
        longjmp(env,errno);
    }
}

unsigned int get4Bytes(unsigned char *addr){ // NOLINT(readability-non-const-parameter)
    return flip4ByteNumber(*UCHARPOINTER_TO_UINTPOINTER(addr));
}

unsigned long get8Bytes(unsigned char *p) {
    return (unsigned long) *(p + 0) << 24 |
           (unsigned long) *(p + 1) << 16 |
           (unsigned long) *(p + 2) <<  8 |
           (unsigned long) *(p + 3) <<  0;
}

void set4Bytes(unsigned char *addr, unsigned int value){
    /*(*((unsigned int *)addr))*/(*UCHARPOINTER_TO_UINTPOINTER(addr)) = flip4ByteNumber(value);
}

#pragma clang diagnostic push
#pragma ide diagnostic ignored "hicpp-signed-bitwise"
//big endian to little endian and  little endian to big endian
unsigned int flip4ByteNumber(unsigned int number){
    unsigned int ou=
            (number & 0x000000FF) << 24|
            (number & 0x0000FF00) << 8 |
            (number & 0x00FF0000) >> 8 |
            (number & 0xFF000000) >> 24;
    return ou;

}
#pragma clang diagnostic pop

/**
 * get Error about getIsError and getErrorNr
 * @param disk
 * @param blockNum
 * @param blockBuffer
 */
void readBlock(FILE *disk, EOS32_daddr_t blockNum, unsigned char * blockBuffer){
    readBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE), EOS32_BLOCK_SIZE, blockBuffer);
}

void writeBlock(FILE *disk, EOS32_daddr_t blockNum, unsigned char *blockBuffer){
    writeBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE), EOS32_BLOCK_SIZE, blockBuffer);
}
//todo: test errorhanldings
/**
 *
 * @param disk
 * @param offset
 * @param size
 * @param content
 * @return 1 success    -1
 */
int readBytesDisk(FILE *disk, unsigned long offset, unsigned int size, unsigned char *content) {
    if(fseek(disk, offset, SEEK_SET) == -1){//check error
        longjmp(env,errno);
    }
    size_t ou = fread(content, size,1,disk);//todo: check feof
    if(ou != 1){
        sendError("Cannot read. offset:  \"%ld\"\tsize:\"%ld\"", offset, size);
        longjmp(env,EIO);
    }
    return 1;
}

void writeBytesDisk(FILE *disk, unsigned long offset, size_t size, unsigned char *content){
    size_t ou;

    if(fseek(disk, offset, SEEK_SET) == -1){//check error
        longjmp(env,errno);
    }

    if(!getIsError()){
        ou = fwrite(content, size,1,disk);

        if(ou != 1){
            sendError("Cannot write. offset:  \"%ld\"\tsize:\"%ld\"", offset, size);
            longjmp(env,errno);
        }
    }
}
void writeflipp4Bytes(FILE *disk, EOS32_daddr_t blockNum, unsigned int offset, unsigned int content){
    unsigned int num = flip4ByteNumber(content);

    writeBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE + offset), 4, UINT_TO_UCHARPOINTER(num));
}

unsigned int readFlipp4Bytes(FILE *disk, EOS32_daddr_t blockNum, int offset){
    unsigned int asdf = 0;
    readBytes(disk, blockNum, (unsigned int) offset, 4, (unsigned char *)&asdf);
    return flip4ByteNumber(asdf);
}

void writeByte(FILE *disk, EOS32_daddr_t blockNum, int offset, unsigned char content){
    writeBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE + offset), 1, &content);
}

/**
 * get Error about getIsError and getErrorNr
 * @param disk
 * @param blockNum
 * @param offset
 * @param size
 * @param content
 *
 * @return 0 success   -1 error         get errorNr by getErrorNr
 */
int readBytes(FILE *disk, EOS32_daddr_t blockNum, unsigned int offset, unsigned int size, unsigned char *content) {
    return readBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE + offset), size, content);
}
void writeBytes(FILE *disk, EOS32_daddr_t blockNum, int offset, unsigned int size, unsigned char *content){
    writeBytesDisk(disk, (unsigned long) ((long) fsStart * SECTOR_SIZE + (long) blockNum * EOS32_BLOCK_SIZE + offset), size, content);
}