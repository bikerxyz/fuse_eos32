
// Created by niklas on 19.11.20.
//

#include "debugHelper.h"
#include "tools.h"

/*void printDirFuseBuffer(struct dirbuf input){
    printDirFuseBufferPointer(input.p, input.size, input);
    /*size_t sizeCounter = 0;
    unsigned char * pointer = input.p;

    while(sizeCounter < input.size){
        size_t * numberPointer = (size_t *)&(pointer[sizeCounter]);
        ino_t inodeNr = numberPointer[0];
        size_t end = numberPointer[1];
        size_t nameLength = numberPointer[2];
        char name[nameLength + 2];
        strncpy(name, pointer+ sizeCounter + 24, nameLength);
        name[nameLength +1] = '\0';
        sizeCounter = end;
        printf("inodeNr: %lu endAdress: %ld  nameLength: %ld name %s\n",inodeNr, end, nameLength, name);
        printf("sizeCounter %lu\n",sizeCounter);

    }*//*
}*/

//#pragma clang diagnostic push
//#pragma ide diagnostic ignored ""
/*void printDirFuseBufferPointer(unsigned char *pointer, size_t size, struct dirbuf b) {
    size_t sizeCounter = 0;
    //unsigned char * pointer = input.p;

    while(sizeCounter < size){
        size_t * numberPointer = (size_t *)&(pointer[sizeCounter]);
        ino_t inodeNr = numberPointer[0];
        size_t end = numberPointer[1];
        size_t nameLength = numberPointer[2];
        char name[nameLength + 2];
        strncpy(name, pointer+ sizeCounter + 24, nameLength);
        //name[nameLength +1] = '\0';
        name[nameLength ] = '\0';
        int addSize;
        if(((24+ nameLength +1)< 32)){
            addSize = 32;
        }else{
            addSize = 24 + nameLength +8-(nameLength %8);
        }
        sizeCounter += addSize;

        printf("inodeNr: %lu endAdress: %ld  nameLength: %ld name %s next \n",inodeNr, end, nameLength, name);
        printf("sizeCounter %lu\n",sizeCounter);

    }
}*/

size_t listAllDisappearedFreeBlocks(EOS32_daddr_t *list){
    EOS32_daddr_t maxBlockAddr = getSizeOfBlocks();
    EOS32_daddr_t minBlockAddr = getSizeOfInodeBlocks();

    getSuperBlock();
    bool isNotFound[maxBlockAddr];
    for(int i1 = 0; i1 < maxBlockAddr; i1++){
        isNotFound[i1] = true;
    }
    memset(list,0 , maxBlockAddr* sizeof(EOS32_daddr_t));

    unsigned int max ;
    EOS32_daddr_t linkAddr = superBlock.nextBlockFreeList;
    EOS32_daddr_t buffer[BLOCK_FREE_MAX_LIST_SIZE_COUNT];
    memset(buffer, 0 , sizeof(EOS32_daddr_t)*BLOCK_FREE_MAX_LIST_SIZE_COUNT);

    isNotFound[linkAddr] = false;
    for(int i1 = 0; i1 < (superBlock.sizeOfBlockFreeList - 1); i1++){
        isNotFound[superBlock.blockFreeList[i1]] = false;
    }


    while(linkAddr != 0){
        max = readFlipp4Bytes(disk, linkAddr, NUMBER_OF_FREE_BLOCK_ADDRESS_ON_FREEBLOCKLIST_CHAIN_NOT_SUPERBLOCK);
        readBytes(disk,linkAddr,FREE_BLOCK_LIST_OFFSET_ON_FREEBLOCKLIST_CHAIN_NOT_SUPERBLOCK,(max - 1)* sizeof(EOS32_daddr_t),(unsigned  char *) buffer);
        linkAddr = readFlipp4Bytes(disk,linkAddr,LINK_BLOCK_OFFSET_ON_FREEBLOCKLIST_CHAIN_NOT_SUPERBLOCK);
        isNotFound[linkAddr] = false;
        for(int i1 = 0; i1 < max -1 ; i1++){
            buffer[i1] = flip4ByteNumber(buffer[i1]);
        }
        for(int i1 = 0; i1 < max - 1; i1++){
            isNotFound[buffer[i1]]= false;
        }
    }

    int posOnList = 0;

    EOS32_size_t inodeMax = countBlocksOfInode(1);
    EOS32_daddr_t list2[inodeMax];
    FileAddresses fileAddresses = getAllAddresses(1,0,inodeMax * sizeof(EOS32_daddr_t),list2);

    for(int i1 = 0; i1 < fileAddresses.size; i1++){
        isNotFound[fileAddresses.addresses[i1]] = false;
    }

    for(int i1 = minBlockAddr + 2; i1 < maxBlockAddr; i1++){
        if(isNotFound[i1]){
            list[posOnList] = (EOS32_daddr_t) i1;
            posOnList++;
        }
    }
    return (size_t) posOnList;
}