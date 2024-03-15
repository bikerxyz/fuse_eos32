//
// Created by niklas on 27.02.20.
//

#ifndef SHFS_EOS32SUPERBLOCK_H
#define SHFS_EOS32SUPERBLOCK_H
#define INODE_FREE_MAX_LIST_COUNT 500
#define BLOCK_FREE_MAX_LIST_SIZE_COUNT 499
#define BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT 500
#define BLOCK_FREE_MAX_LIST_SIZE_BYTES (BLOCK_FREE_MAX_LIST_SIZE_COUNT * sizeof(EOS32_daddr_t))
#define BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_BYTES (BLOCK_FREE_MAX_LIST_WITH_LINK_SIZE_COUNT * sizeof(EOS32_daddr_t))
#define SUPER_BLOCK_NR 1


#include "../../datatypes.h"
#include "../../io/ioHelper.h"
#include "../itemManipulation/eos32Inode.h"
#include "../../fuseHelper/fuseHelper.h"
typedef struct superBlock1 {
    unsigned int superMagic;
    EOS32_daddr_t sizeOfBlocks;
    EOS32_daddr_t sizeOfInodeBlocks;

    EOS32_size_t numberOfFreeBlocks;
    EOS32_size_t numberOfFreeInodes;


    unsigned int sizeOfInodeFreeList;
    EOS32_ino_t inodeFreeList[INODE_FREE_MAX_LIST_COUNT];

    unsigned int sizeOfBlockFreeList;
    EOS32_daddr_t nextBlockFreeList;
    EOS32_daddr_t blockFreeList[BLOCK_FREE_MAX_LIST_SIZE_COUNT];

    EOS32_time_t lastUpdateOfSuperBlock;

    char lockForFreeBlockListManipulation;
    char lockForFreeInodeListManipulation;
    char superBlockModifiedFlag;
    char mountedReadOnlyFlag;
} SuperBlock;



typedef struct superBlockOffSets1 {
    EOS32_off_t superMagic;
    EOS32_off_t sizeOfBlocks;
    EOS32_off_t sizeOfInodeBlocks;
    EOS32_off_t numberOfFreeBlocks;
    EOS32_off_t numberOfFreeInodes;
    EOS32_off_t sizeOfInodeFreeList;
    EOS32_off_t inodeFreeList;
    EOS32_off_t sizeOfBlockFreeList;
    EOS32_off_t nextBlockFreeList;
    EOS32_off_t blockFreeList;
    EOS32_off_t lastUpdateOfSuperBlock;
    EOS32_off_t lockForFreeBlockListManipulation;
    EOS32_off_t lockForFreeInodeListManipulation;
    EOS32_off_t superBlockModifiedFlag;
    EOS32_off_t mountedReadOnlyFlag;
} SuperBlockOffSets;

extern SuperBlock superBlock;
extern SuperBlockOffSets superBlockOffSets;

//superblock
EOS32_ino_t popFreeInode();
EOS32_daddr_t popFreeBlock();
void pushFreeBlock(EOS32_daddr_t input);
void sourceOutFreeBlockList(EOS32_daddr_t input);

EOS32_daddr_t getSizeOfBlocks();
EOS32_size_t getSizeOfInodeBlocks();


void pushFreeBlocks(FileAddresses input);
void setSuperMagic(unsigned int input);
void setSizeOfBlocks(EOS32_daddr_t input);
void setSizeOfInodeBlocks(EOS32_daddr_t input);
void setNumberOfFreeBlocks(EOS32_size_t input);
void setNumberOfFreeInodes(EOS32_size_t input);
void setSizeOfInodeFreeList(EOS32_size_t input);
void setInodeFreeList(EOS32_ino_t *input, EOS32_size_t count);
void setSizeOfBlockFreeList(unsigned int input);
void setNextBlockFreeList(EOS32_daddr_t input);
void setBlockFreeList(EOS32_daddr_t *input);
void setLastUpdateOfSuperBlock(EOS32_time_t input);
void setLockForFreeBlockListManipulation(char input);
void setLockForFreeInodeListManipulation(char input);
void setSuperBlockModifiedFlag(char input);
void setMountedReadOnlyFlag(char input);
void setInodeFreeItem(EOS32_ino_t freeInodeNr, unsigned int index);
void setFreeBlockItem(EOS32_ino_t freeBlockNr, unsigned int index);

EOS32_time_t getLastUpdateOfSuperBlock();


unsigned int getNumberOfFreeBlocks();
size_t getNumberOfFreeInodes();
size_t getFreeBlocksBySuperBlockByMemory(unsigned char *buffer);
void setFreeBlocksToSuperBlockByHardDisk(unsigned char *buffer);

EOS32_daddr_t storeNextFreeBlockList(void);
EOS32_ino_t storeSearchFreeInodeList(EOS32_ino_t *inodeNrs);

#endif //SHFS_EOS32SUPERBLOCK_H