//
// Created by niklas on 27.02.20.
//

#include "eos32SuperBlock.h"

SuperBlock superBlock;
SuperBlockOffSets superBlockOffSets;

EOS32_ino_t popFreeInode(){
    EOS32_ino_t  inodesList[INODE_FREE_MAX_LIST_COUNT];
    if(superBlock.sizeOfInodeFreeList == 0){
        storeSearchFreeInodeList(inodesList);
    }
    EOS32_ino_t ret = superBlock.inodeFreeList[superBlock.sizeOfInodeFreeList - 1];
    setSizeOfInodeFreeList(superBlock.sizeOfInodeFreeList - 1);
    return ret;
}

EOS32_daddr_t popFreeBlock(){
    EOS32_size_t numberOfFreeBlocks = getNumberOfFreeBlocks();
    if(numberOfFreeBlocks == 0){
        longjmp(env,ENOSPC);
    }

    setNumberOfFreeBlocks(numberOfFreeBlocks - 1);

    if(superBlock.sizeOfBlockFreeList == 1){
        return storeNextFreeBlockList();
    }
    EOS32_daddr_t ret = superBlock.blockFreeList[superBlock.sizeOfBlockFreeList - 2];
    setSizeOfBlockFreeList(superBlock.sizeOfBlockFreeList - 1);

    return ret;
}

void pushFreeBlocks(FileAddresses input){
    if(input.size == 0) return;
    EOS32_size_t counter = 0;

    getSuperBlock();//debug
    EOS32_size_t sizeToFillFreeBlockList = BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT - superBlock.sizeOfBlockFreeList;
    EOS32_size_t limit = sizeToFillFreeBlockList;

    if(input.size < sizeToFillFreeBlockList){
        limit = (EOS32_size_t)input.size;
    }

    bool canCompletFillTheList = false;
    bool canCompletFillSecondList = false;
    if(input.size >= sizeToFillFreeBlockList) canCompletFillTheList = true;
    if(canCompletFillTheList && (sizeToFillFreeBlockList + BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT) <= input.size){
        canCompletFillSecondList = true;
    }

    int i1;
    if(canCompletFillTheList) {

        for(i1 = 0; i1 < limit;i1++){
            superBlock.blockFreeList[i1 + superBlock.sizeOfBlockFreeList - 1] = input.addresses[counter];
            counter++;
        }
        setBlockFreeList(superBlock.blockFreeList);
        EOS32_size_t newSize =(EOS32_size_t) superBlock.sizeOfBlockFreeList;
        setNumberOfFreeBlocks(getNumberOfFreeBlocks() + i1);
        newSize += i1;
        setSizeOfBlockFreeList((int)newSize);
    }
    getSuperBlock();//debug

    if(counter >= input.size){
        //todo: make error
    }

    limit = ((input.size - counter) / BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT);
    if((limit % BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT) != 0){
        limit++;
    }
    if(canCompletFillSecondList){
        for(i1 = 0; i1 < (limit - 1);i1++){
            sourceOutFreeBlockList(input.addresses[counter]);
            setBlockFreeList(input.addresses + counter + 1);
            setSizeOfBlockFreeList(BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT);
            counter += BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT;
            setNumberOfFreeBlocks(getNumberOfFreeBlocks() + BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT);
        }
    }

    size_t sizeToFillInLayer3 = input.size - counter;
    if(sizeToFillInLayer3 > 0){

        if(superBlock.sizeOfBlockFreeList == BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT){
            sourceOutFreeBlockList(input.addresses[counter]);
            counter++;
            sizeToFillInLayer3--;
        }
        getSuperBlock();//debug
        if(sizeToFillInLayer3 > 0){

            for(i1 = 0; i1 < sizeToFillInLayer3;i1++){
                superBlock.blockFreeList[superBlock.sizeOfBlockFreeList + i1 - 1] = input.addresses[counter + i1];
            }
            setSizeOfBlockFreeList((i1 + superBlock.sizeOfBlockFreeList));
            setBlockFreeList(superBlock.blockFreeList);
            counter += i1;
        }
    }

    setNumberOfFreeBlocks(getNumberOfFreeBlocks() + counter);
    getSuperBlock();//debug

    setBlockFreeList(superBlock.blockFreeList);
}

void pushFreeBlock(EOS32_daddr_t input){
    if(input == 0){
        printf("Cannot Block 0 on the Block freelist\n");
        longjmp(env, EIO);
    }

    if(superBlock.sizeOfBlockFreeList == BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT){
        sourceOutFreeBlockList(input);
    }else{
        superBlock.blockFreeList[superBlock.sizeOfBlockFreeList - 1] = input;
        setFreeBlockItem(input, superBlock.sizeOfBlockFreeList - 1);
        setSizeOfBlockFreeList(superBlock.sizeOfBlockFreeList + 1);
    }
    setNumberOfFreeBlocks(getNumberOfFreeBlocks() + 1);

}
void sourceOutFreeBlockList(EOS32_daddr_t input){
    if(input == 0){
        longjmp(env, EFAULT);
    }

    if(superBlock.sizeOfBlockFreeList > BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT){
        //todo: make error
    }

    EOS32_daddr_t buffer[BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT + 1];


    buffer[0] = flip4ByteNumber(superBlock.sizeOfBlockFreeList);
    buffer[1] = flip4ByteNumber(superBlock.nextBlockFreeList);
    for(int i1 = 0; i1 < superBlock.sizeOfBlockFreeList;i1++){
        buffer[i1 + 2] =  flip4ByteNumber(superBlock.blockFreeList[i1]);
    }

    writeBytes(disk, input, 0, (BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT + 1) * sizeof(EOS32_daddr_t),(unsigned char *) buffer);

    setNextBlockFreeList(input);
    setNextBlockFreeList(input);
    setSizeOfBlockFreeList(1);
}

EOS32_daddr_t getSizeOfBlocks(){
    return  readFlipp4Bytes(disk,SUPER_BLOCK_NR, superBlockOffSets.sizeOfBlocks);
}

EOS32_size_t getSizeOfInodeBlocks(){
    return readFlipp4Bytes(disk,SUPER_BLOCK_NR, superBlockOffSets.sizeOfInodeBlocks);
}

void setSuperMagic(unsigned int input){
    superBlock.superMagic = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.superMagic, input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setSizeOfBlocks(EOS32_daddr_t input){
    superBlock.sizeOfBlocks = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.sizeOfBlocks, input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setSizeOfInodeBlocks(EOS32_daddr_t input){
    superBlock.sizeOfInodeBlocks = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.sizeOfInodeBlocks, input);
    setLastUpdateOfSuperBlock(getTimeNow());
}

void setNumberOfFreeBlocks(EOS32_size_t input){
    writeflipp4Bytes(disk,SUPER_BLOCK_NR,superBlockOffSets.numberOfFreeBlocks,input);
    superBlock.numberOfFreeBlocks = input;
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setNumberOfFreeInodes(EOS32_size_t input){
    writeflipp4Bytes(disk,SUPER_BLOCK_NR,superBlockOffSets.numberOfFreeInodes,input);
    setLastUpdateOfSuperBlock(getTimeNow());
}


void setSizeOfInodeFreeList(EOS32_size_t input){
    superBlock.sizeOfInodeFreeList = (unsigned int) input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.sizeOfInodeFreeList, (unsigned int) input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setInodeFreeList(EOS32_ino_t *input, EOS32_size_t count) {
    EOS32_ino_t blockBufferTmp[count];

    for(long i1 = 0; i1 < count; i1++){
        superBlock.inodeFreeList[i1] = input[i1];
        blockBufferTmp[i1] = flip4ByteNumber(input[i1]);
    }

    writeBytes(disk, SUPER_BLOCK_NR, superBlockOffSets.inodeFreeList, count * sizeof(EOS32_daddr_t), (unsigned char *)blockBufferTmp);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setSizeOfBlockFreeList(unsigned int input){
    superBlock.sizeOfBlockFreeList =  input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.sizeOfBlockFreeList, input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setNextBlockFreeList(EOS32_daddr_t input){
    superBlock.nextBlockFreeList = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.nextBlockFreeList, input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setBlockFreeList(EOS32_daddr_t *input){
    EOS32_daddr_t blockBufferTmp[BLOCK_FREE_MAX_LIST_SIZE_COUNT];

    for(long i1 = 0; i1 < BLOCK_FREE_MAX_LIST_SIZE_COUNT; i1++){
        blockBufferTmp[i1] = flip4ByteNumber(input[i1]);
        superBlock.blockFreeList[i1] = input[i1];
    }

    writeBytes(disk,SUPER_BLOCK_NR,superBlockOffSets.blockFreeList,BLOCK_FREE_MAX_LIST_SIZE_BYTES,(unsigned char *)blockBufferTmp);
    setLastUpdateOfSuperBlock(getTimeNow());
}


void setLastUpdateOfSuperBlock(EOS32_time_t input){
    superBlock.lastUpdateOfSuperBlock = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.lastUpdateOfSuperBlock, (unsigned int) input);
}
void setLockForFreeBlockListManipulation(char input){
    superBlock.lockForFreeBlockListManipulation = input;
    writeByte(disk, SUPER_BLOCK_NR, superBlockOffSets.lockForFreeBlockListManipulation, (unsigned char) input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setLockForFreeInodeListManipulation(char input){
    superBlock.lockForFreeInodeListManipulation = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.lockForFreeInodeListManipulation, (unsigned int) input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setSuperBlockModifiedFlag(char input){
    superBlock.superBlockModifiedFlag = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.superBlockModifiedFlag, (unsigned int) input);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setMountedReadOnlyFlag(char input){
    superBlock.mountedReadOnlyFlag = input;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.mountedReadOnlyFlag, (unsigned int) input);
    setLastUpdateOfSuperBlock(getTimeNow());
}

void setInodeFreeItem(EOS32_ino_t freeInodeNr, unsigned int index){
    superBlock.inodeFreeList[index] = freeInodeNr;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.inodeFreeList + (sizeof(EOS32_ino_t) * index), freeInodeNr);
    setLastUpdateOfSuperBlock(getTimeNow());
}
void setFreeBlockItem(EOS32_ino_t freeBlockNr, unsigned int index){
    superBlock.blockFreeList[index] = freeBlockNr;
    writeflipp4Bytes(disk, SUPER_BLOCK_NR, superBlockOffSets.blockFreeList + (sizeof(EOS32_daddr_t) * index),freeBlockNr);
    setLastUpdateOfSuperBlock(getTimeNow());
}

size_t getFreeBlocksBySuperBlockByMemory(unsigned char *buffer) {
    for(unsigned int i1 = 0; i1 < BLOCK_FREE_MAX_LIST_SIZE_COUNT; i1++){
        set4Bytes(buffer +(i1 * 4), (unsigned int) superBlock.blockFreeList[i1]);
    }
    return BLOCK_FREE_MAX_LIST_SIZE_BYTES;
}

unsigned int getNumberOfFreeBlocks(){
    return readFlipp4Bytes(disk,SUPER_BLOCK_NR,superBlockOffSets.numberOfFreeBlocks);
}
size_t getNumberOfFreeInodes(){
    return readFlipp4Bytes(disk,SUPER_BLOCK_NR,superBlockOffSets.numberOfFreeInodes);//todo: free inodes muss auch gezählt werden, die variabel ist not used
}

void setFreeBlocksToSuperBlockByHardDisk(unsigned char *buffer) {
    for(unsigned int i1 = 0; i1 < BLOCK_FREE_MAX_LIST_SIZE_COUNT; i1++){
        superBlock.blockFreeList[i1] = get4Bytes(buffer + (i1 * sizeof(EOS32_daddr_t)));
    }
    setLastUpdateOfSuperBlock(getTimeNow());
}

EOS32_daddr_t storeNextFreeBlockList(void){


    EOS32_daddr_t ou = superBlock.nextBlockFreeList;

    if(ou == 0){
        longjmp(env,ENOSPC);
    }


    EOS32_daddr_t buffer[BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT + 1];
    readBytes(disk,ou,0, BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT * sizeof(EOS32_daddr_t) + 4,(unsigned char *)buffer);
    writeBytes(disk,SUPER_BLOCK_NR, superBlockOffSets.blockFreeList, BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT * sizeof(EOS32_daddr_t) +1,(unsigned char *)(buffer + 2));
    writeBytes(disk,SUPER_BLOCK_NR, superBlockOffSets.nextBlockFreeList, sizeof(EOS32_daddr_t),(unsigned char *)(buffer));

    setFreeBlocksToSuperBlockByHardDisk((unsigned char *)(buffer + 2));
    setNextBlockFreeList(get4Bytes((unsigned char *) (buffer + 1)));
    setSizeOfBlockFreeList(get4Bytes((unsigned char *)buffer ));

    return ou;

}

EOS32_ino_t storeSearchFreeInodeList(EOS32_ino_t *inodeNrs) {
    FreeInodeList freeInodeList;
    freeInodeList.inodeNrs = inodeNrs;
    freeInodeList = getAndLookFreeInodes(freeInodeList,INODE_FREE_MAX_LIST_COUNT,false);

    if(freeInodeList.size == 0){
        longjmp(env,ENOSPC);
    }

    setInodeFreeList(freeInodeList.inodeNrs, freeInodeList.size);
    setSizeOfInodeFreeList(freeInodeList.size);
}