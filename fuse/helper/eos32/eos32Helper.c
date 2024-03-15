//
// Created by niklas on 11.12.19.
//

#include <fuseHelper.h>
#include "eos32Helper.h"

EOS32_BLOCK_BUFFER zeroedBlock;

void  getSuperBlock(){
    readBlock(disk, 1,blockBuffer);
    if(!getIsError()){
        superBlock.superMagic = (int)get4Bytes(blockBuffer + superBlockOffSets.superMagic);
        superBlock.sizeOfBlocks = (int)get4Bytes(blockBuffer + superBlockOffSets.sizeOfBlocks);
        superBlock.sizeOfInodeBlocks = (int)get4Bytes(blockBuffer + superBlockOffSets.sizeOfInodeBlocks);
        superBlock.sizeOfInodeFreeList = (int)get4Bytes(blockBuffer + superBlockOffSets.sizeOfInodeFreeList);
        superBlock.sizeOfBlockFreeList = (int)get4Bytes(blockBuffer + superBlockOffSets.sizeOfBlockFreeList);
        superBlock.nextBlockFreeList = (int)get4Bytes(blockBuffer + superBlockOffSets.nextBlockFreeList);
        superBlock.lastUpdateOfSuperBlock = (int)get4Bytes(blockBuffer + superBlockOffSets.lastUpdateOfSuperBlock);

        superBlock.lockForFreeBlockListManipulation = (char)blockBuffer[superBlockOffSets.lockForFreeBlockListManipulation];
        superBlock.lockForFreeInodeListManipulation = (char)blockBuffer[superBlockOffSets.lockForFreeInodeListManipulation];
        superBlock.superBlockModifiedFlag = (char)blockBuffer[superBlockOffSets.superBlockModifiedFlag];
        superBlock.mountedReadOnlyFlag = (char)blockBuffer[superBlockOffSets.mountedReadOnlyFlag];

        for(int i1 = 0; i1 < INODE_FREE_MAX_LIST_COUNT; i1++){
            superBlock.inodeFreeList[i1] = get4Bytes(blockBuffer + superBlockOffSets.inodeFreeList + i1 * sizeof(EOS32_ino_t));
        }
        setFreeBlocksToSuperBlockByHardDisk(blockBuffer + superBlockOffSets.blockFreeList);
    }
}


/**
 *
 * @param inodeNr Inode Nr vom gesuchten Inode
 * @param inode Muss ein bereits allokierter Inode sein
 * @return 0 success    error über unser errorhandling
 */



void getDirectory(EOS32_daddr_t blockNr, DirectoryBlock *directory) {
    readBlock(disk, blockNr, blockBuffer);

    for(int i1 = 0;i1 < DIRECTORY_ENTRIES_PER_BLOCK;i1++){
        directory->directoryEntries[i1].inodeNr = get4Bytes(blockBuffer + (i1 * DIRECTORY_ENTRY_SIZE));
        int i2;
        for(i2 = 4; i2 < DIRECTORY_ENTRY_SIZE;i2++){
            directory->directoryEntries[i1].name[i2 - 4] = (char)blockBuffer[(i1 * DIRECTORY_ENTRY_SIZE) + i2];
        }
        directory->directoryEntries[i1].name[i2 - 4 ] = '\0';
    }
}

//todo: test getDirectoryAdresses
size_t getDirectoryAdresses(EOS32_ino_t inodeNr, size_t size, size_t offset_directoryEntries, EOS32_daddr_t *buffer) {

    Inode directory = getInode(inodeNr);
    size_t newSize = countBlocksOfInode(inodeNr);
    if((directory.numberOfBytesInFile % EOS32_BLOCK_SIZE) != 0)newSize += 1;
    if(newSize < size){
        size = newSize;
    }
    size_t pos1 = offset_directoryEntries;
    size_t counter1 = 0;


    if(offset_directoryEntries < DIRECT_BLOCK_COUNT){
        for(; pos1 < DIRECT_BLOCK_COUNT && (pos1 - offset_directoryEntries) < size; pos1++){
            if(directory.directBlocks[pos1] == 0)
                return counter1;
            buffer[pos1 - offset_directoryEntries] = directory.directBlocks[pos1];
            counter1++;
        }
    }


    if(directory.singleIndirectBlock != 0 && offset_directoryEntries < (SINGLE_INDIRECT_COUNT +DIRECT_BLOCK_COUNT) && (pos1 - offset_directoryEntries) < size){
        readBlock(disk, directory.singleIndirectBlock,blockBuffer);

        for(; pos1 < (SINGLE_INDIRECT_COUNT + DIRECT_BLOCK_COUNT) && (pos1 - offset_directoryEntries) < size; pos1++){
            EOS32_daddr_t addrTmp1 = get4Bytes(blockBuffer + (pos1 - DIRECT_BLOCK_COUNT) * 4);
            if(addrTmp1 != 0){
                buffer[pos1 - offset_directoryEntries] = addrTmp1;
                counter1++;
            }
        }
    }
    if(directory.doubleIndirectBlock != 0 && offset_directoryEntries < (DOUBLE_INDIRECT_COUNT +SINGLE_INDIRECT_COUNT + DIRECT_BLOCK_COUNT) && (pos1 - offset_directoryEntries) < size){
        readBlock(disk, directory.doubleIndirectBlock,blockBuffer);

        EOS32_BLOCK_BUFFER blockBuffer2;
        size_t i1 = ((pos1 - DIRECT_BLOCK_COUNT - SINGLE_INDIRECT_COUNT) / DOUBLE_INDIRECT_COUNT);
        for(;i1 < SINGLE_INDIRECT_COUNT && (counter1) < size; i1++){
            EOS32_daddr_t address1 = get4Bytes(blockBuffer +i1*4);
            readBlock(disk, address1,blockBuffer2);

            size_t i2 = (pos1 - DIRECT_BLOCK_COUNT - SINGLE_INDIRECT_COUNT) % DOUBLE_INDIRECT_COUNT;
            for(;i2 < DOUBLE_INDIRECT_COUNT && pos1 < size; i2++){
                EOS32_daddr_t tmpReadBlockAdress = get4Bytes(blockBuffer2 + (i2) * 4);
                buffer[pos1 - offset_directoryEntries] = tmpReadBlockAdress;
                pos1++;
                counter1++;
            }
        }
    }
    return counter1;
}

void initSuperBlockOffSets(void){
    superBlockOffSets.superMagic=                               0x0000;/*superMagic*/
    superBlockOffSets.sizeOfBlocks=                             0x0004;/*sizeOfBlocks*/
    superBlockOffSets.sizeOfInodeBlocks=                        0x0008;/*sizeOfInodeBlocks*/
    superBlockOffSets.numberOfFreeBlocks=                       0x000C;/*number of FreeBlocks*/
    superBlockOffSets.numberOfFreeInodes=                       0x0010;/*number of FreeInodes*/
    superBlockOffSets.sizeOfInodeFreeList=                      0x0014;/*sizeOfInodeFreeList*/
    superBlockOffSets.inodeFreeList=                            0x0018;/*inodeFreeList*/
    superBlockOffSets.sizeOfBlockFreeList=                      0x07E8;/*sizeOfBlockFreeList*/
    superBlockOffSets.nextBlockFreeList=                        0x07EC;/*nextBlockFreeList*/
    superBlockOffSets.blockFreeList=                            0x07F0;/*blockFreeList*/
    superBlockOffSets.lastUpdateOfSuperBlock=                   0x0FBC;/*lastUpdateOfSuperBlock*/
    superBlockOffSets.lockForFreeBlockListManipulation=         0x0FC0;/*lockForFreeBlockListManipulation*/
    superBlockOffSets.lockForFreeInodeListManipulation=         0x0FC1;/*lockForFreeInodeListManipulation*/
    superBlockOffSets.superBlockModifiedFlag=                   0x0FC2;/*superBlockModifiedFlag*/
    superBlockOffSets.mountedReadOnlyFlag=                      0x0FC3;/*mountedReadOnlyFlag*/
}

void initInodeBlockOffSets(void){
    inodeOffSets.typeMode =                     0x0000;
    inodeOffSets.numberOfLinksToFile =          0x0004;
    inodeOffSets.ownersUserId =                 0x0008;
    inodeOffSets.ownersGroupId =                0x000C;
    inodeOffSets.timeCreated =                  0x0010;
    inodeOffSets.timeLastModified =             0x0014;
    inodeOffSets.timeLastAccessed =             0x0018;
    inodeOffSets.numberOfBytesInFile =          0x001C;
    inodeOffSets.directBlocks =                 0x0020;
    inodeOffSets.singleIndirectBlock =          0x0038;
    inodeOffSets.doubleIndirectBlock =          0x003c;
}

void initAllOffSets(void){
    initSuperBlockOffSets();
    initInodeBlockOffSets();
}

void initAll(void){
    initAllOffSets();
    memset(zeroedBlock, 0, EOS32_BLOCK_SIZE);
}

//DANGER this function use malloc
FileAddresses getFileAddresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer) {
    return getFileOrAllAdresses(inodeNr,offset,maxCountAdresses,addressBuffer,true);
}

FileAddresses getAllAddresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer){
    return getFileOrAllAdresses(inodeNr,offset,maxCountAdresses,addressBuffer,false);
}

EOS32_time_t getTimeNow(){
    time_t rawTime;
    time(&rawTime);
    if(rawTime > INT32_MAX){
        printf("\n\n\ntimer overflow, time variable has only 4 bytes\n\n\n");
        exit(1);
    }
    return (EOS32_time_t) rawTime;
}

FileAddresses getFileOrAllAdresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer, bool getFileAddr){
    Inode inode = getInode(inodeNr);
    FileAddresses ou;
    ou.size = 0;
    ou.addresses = addressBuffer;

    if(getFileAddr){

        if(maxCountAdresses > (DOUBLE_INDIRECT_COUNT + SINGLE_INDIRECT_COUNT+DIRECT_BLOCK_COUNT)){
            size_t max = (DOUBLE_INDIRECT_COUNT + SINGLE_INDIRECT_COUNT+DIRECT_BLOCK_COUNT);
            sendError("getFileAdresses: one inode cannot hold more addresses as %ld\n", max);
            longjmp(env,EFBIG);
        }
    }else{
        if(maxCountAdresses > ALL_BLOCKS_ON_INODE){
            longjmp(env, EFBIG);
        }
    }

    unsigned int currentAddress = offset / EOS32_BLOCK_SIZE;

    if(inode.numberOfBytesInFile == 0 && !isInodeDirectory(inode)){
        ou.size = 0;
        return ou;
    }

    if(offset < DIRECT_BLOCK_COUNT * EOS32_BLOCK_SIZE){

        for(; currentAddress < DIRECT_BLOCK_COUNT && ou.size < maxCountAdresses; currentAddress++){
            if(inode.directBlocks[currentAddress] == 0){
                return ou;
            }
            ou.addresses[ou.size] = inode.directBlocks[currentAddress];
            ou.size++;
        }
    }


    if(offset < SINGLE_INDIRECT_COUNT * EOS32_BLOCK_SIZE + DIRECT_BLOCK_COUNT * EOS32_BLOCK_SIZE){
        EOS32_BLOCK_BUFFER buffer;
        EOS32_daddr_t *addrBlockBuffer = (EOS32_daddr_t *)buffer;
        readBlock(disk,inode.singleIndirectBlock,buffer);
        if(!getFileAddr){
            if(ou.size < maxCountAdresses){
                ou.addresses[ou.size] = inode.singleIndirectBlock;
                ou.size++;
            }
        }

        for(unsigned int i1 = currentAddress - DIRECT_BLOCK_COUNT; i1 < SINGLE_INDIRECT_COUNT && ou.size < maxCountAdresses; i1++){
            EOS32_daddr_t addrTmp1 = flip4ByteNumber(addrBlockBuffer[i1]);
            if(addrTmp1 == 0){
                return ou;
            }

            ou.addresses[ou.size] = addrTmp1;
            ou.size++;
            currentAddress++;
        }
    }

    if((long)offset < (long)DOUBLE_INDIRECT_COUNT * EOS32_BLOCK_SIZE + SINGLE_INDIRECT_COUNT * EOS32_BLOCK_SIZE + DIRECT_BLOCK_COUNT * EOS32_BLOCK_SIZE){
        EOS32_BLOCK_BUFFER buffer;
        EOS32_daddr_t *addrBlockBufferDouble = (EOS32_daddr_t *)buffer;
        if(inode.doubleIndirectBlock == 0){
            return ou;
        }
        readBlock(disk,inode.doubleIndirectBlock,buffer);
        if(!getFileAddr){
            if (ou.size < maxCountAdresses) {
                ou.addresses[ou.size] = inode.doubleIndirectBlock;
                ou.size++;
            }
            for(unsigned int i1 = 0; i1 < (DOUBLE_INDIRECT_COUNT / SINGLE_INDIRECT_COUNT) && ou.size < maxCountAdresses;i1++){
                EOS32_daddr_t addr = flip4ByteNumber(addrBlockBufferDouble[i1]);
                if(addr == 0)break;
                ou.addresses[ou.size] = addr;
                ou.size++;
            }
        }

        currentAddress = currentAddress - (SINGLE_INDIRECT_COUNT /* * EOS32_BLOCK_SIZE */+ DIRECT_BLOCK_COUNT/* * EOS32_BLOCK_SIZE*/);
        unsigned int index = currentAddress / SINGLE_INDIRECT_COUNT;
        bool firstSingleIndirectStep = true;
        for(unsigned int i1 = index;i1 < SINGLE_INDIRECT_COUNT && ou.size < maxCountAdresses; i1++){

            EOS32_BLOCK_BUFFER addrBlockBufferSingleTmp;
            EOS32_daddr_t singleAddresOnDoubleList = flip4ByteNumber(addrBlockBufferDouble[i1]);
            if(singleAddresOnDoubleList == 0){
                return ou;
            }
            readBlock(disk,singleAddresOnDoubleList,addrBlockBufferSingleTmp);
            unsigned int index2 = 0;
            if(firstSingleIndirectStep){
                index2 = currentAddress % (SINGLE_INDIRECT_COUNT);
                firstSingleIndirectStep = false;
            }

            for(unsigned int i2 = index2; i2 < SINGLE_INDIRECT_COUNT && ou.size < maxCountAdresses;i2++){

                EOS32_daddr_t addrTmp1 = get4Bytes(addrBlockBufferSingleTmp + i2 * sizeof(EOS32_daddr_t));
                if(addrTmp1 == 0){
                    return ou;
                }
                ou.addresses[ou.size] = addrTmp1;
                ou.size++;
            }
        }
    }else{
        //todo:make error getFileAddresses used by read
    }


    return ou;
}
//return 0 = not found
#include <time.h>
EOS32_ino_t searchEntryByDirectoryWithName(EOS32_ino_t parent, const char *name) {
    static EOS32_daddr_t lastBeforeBlock = 0;
    static EOS32_daddr_t lastBlock = 0;
    static EOS32_daddr_t lastNextBlock = 0;
    static EOS32_ino_t lastInodeParentNr = 0;
    int lastAdd = 0;

    if(lastInodeParentNr == parent){
        if(lastBeforeBlock != 0)lastAdd++;
        if(lastBlock != 0)lastAdd++;
        if(lastNextBlock != 0)lastAdd++;
    }
    EOS32_daddr_t *buffer = malloc(BLOCK_ADDRESSES_MAX_SIZE_BYTES + lastAdd);//todo: i think malloc too much bytes



    if(buffer == NULL){
        longjmp(env,errno);
    }

    pushPointer(buffer);

     if(parent == lastInodeParentNr){

        int lastPosCounter = 0;
        if(lastBeforeBlock != 0){
            buffer[lastPosCounter] = lastBeforeBlock;
            lastPosCounter++;
        }
        if(lastBlock != 0){
            buffer[lastPosCounter] = lastBlock;
            lastPosCounter++;
        }

        if(lastNextBlock != 0){
            buffer[lastPosCounter] = lastNextBlock;
        }
    }else{
         lastBeforeBlock = 0;
         lastBlock = 0;
         lastNextBlock = 0;
     }

    size_t size = getDirectoryAdresses(parent, BLOCK_ADDRESSES_MAX_SIZE_BYTES, 0, buffer+lastAdd);


    size_t i1;
    for(i1 = 0; i1 < size + lastAdd; i1++){
        DirectoryBlock directoryBlock ;
        getDirectory(buffer[i1], &directoryBlock);

        int i2;
        for(i2 = 0; i2 < DIRECTORY_ENTRIES_PER_BLOCK;i2++){

            if(strcmp(name, directoryBlock.directoryEntries[i2].name) == 0){
                if(i1 > 0)lastBeforeBlock = buffer[i1];
                lastBlock = buffer[i1];
                if(i1 + 1 < size)lastNextBlock = buffer[i1+1];
                lastInodeParentNr = parent;

                EOS32_ino_t searchedInodeNr = directoryBlock.directoryEntries[i2].inodeNr;

                destroyBuffer(buffer);
                return searchedInodeNr;
            }
        }
    }

    destroyBuffer(buffer);
    return 0;
}

void destroyBuffer(void *pointer){
    removePointer(pointer);
    free(pointer);
}

void addDirEntry(EOS32_ino_t parentNr, EOS32_ino_t inodeNr, const char *name){

    Inode parent = getInode(parentNr);
    /*einerseits ist die parentBlockcount falsch, da lücken im Ordner erlaubt sind, andererseits ist das egal, weil entweder gibt es eine Lücke
     * dann hätte man eine Lücke bevor man das Ende erreicht hätte oder es gibt keine Lücke dann würden wir auch damit klar kommen, weil dann parentBlockCount
     * wieder stimmt*/
    if(strlen(name) > DIRECTORY_ENTRY_SIZE - sizeof(EOS32_ino_t)){//todo: ob es ein terminierenden null byte benötigt
        longjmp(env,ENAMETOOLONG);
    }
    size_t parentBlockCount = (parent.numberOfBytesInFile / EOS32_BLOCK_SIZE + (parent.numberOfBytesInFile % EOS32_BLOCK_SIZE == 0 ? 0 : 1));//todo check ist das korrekt
    EOS32_daddr_t dirAddresses[parentBlockCount];
    parentBlockCount = getDirectoryAdresses(parentNr,parentBlockCount,0,dirAddresses);

    for(int i1 = 0; i1 < parentBlockCount;i1++){
        DirectoryBlock dirBlock;
        getDirectory(dirAddresses[i1], &dirBlock);

        for(int i2 = 0;i2 < DIRECTORY_ENTRIES_PER_BLOCK;i2++){
            if(dirBlock.directoryEntries[i2].inodeNr == 0){
                //write zeroes over name
                writeBytes(disk,dirAddresses[i1], i2 * DIRECTORY_ENTRY_SIZE + sizeof(EOS32_ino_t),DIRECTORY_ENTRY_SIZE - sizeof(EOS32_ino_t),zeroedBlock);

                writeflipp4Bytes(disk,dirAddresses[i1],i2 * DIRECTORY_ENTRY_SIZE,inodeNr);
                writeBytes(disk, dirAddresses[i1], i2 * DIRECTORY_ENTRY_SIZE + sizeof(EOS32_ino_t),
                           (unsigned int) strlen(name), (unsigned char *)name);

                setNumberOfBytesInFile(parent.nr,parent.numberOfBytesInFile + DIRECTORY_ENTRY_SIZE);

                return;
            }

        }
    }

    EOS32_daddr_t newBlock = pushBlockToInode(parentNr);

    writeflipp4Bytes(disk,newBlock,0,inodeNr);
    writeBytes(disk, newBlock, sizeof(EOS32_ino_t), (unsigned int) strlen(name), (unsigned char *)name);

    setNumberOfBytesInFile(parent.nr,parent.numberOfBytesInFile + DIRECTORY_ENTRY_SIZE);
}

EOS32_ino_t removeDirEntry(EOS32_ino_t parentNr, const char *name){
    Inode parent = getInode(parentNr);
    EOS32_size_t dirNumberOfBlocks = countBlocksOfInode(parentNr);
    if((parent.numberOfBytesInFile % EOS32_BLOCK_SIZE) != 0) dirNumberOfBlocks++;

    EOS32_daddr_t addrBuffer[dirNumberOfBlocks];
    getDirectoryAdresses(parentNr,dirNumberOfBlocks,0,addrBuffer);
    for(int i1 = 0; i1 < dirNumberOfBlocks;i1++){
        DirectoryBlock directoryBlock;
        getDirectory(addrBuffer[i1], &directoryBlock);
        bool blockIsEmpty = true;
        for(int i2 = 0; i2 < DIRECTORY_ENTRIES_PER_BLOCK;i2++){
            if(strcmp(directoryBlock.directoryEntries[i2].name, name) == 0){
                EOS32_ino_t inodeNrToDelete = directoryBlock.directoryEntries[i2].inodeNr;
                unsigned char zeroBuffer[DIRECTORY_ENTRY_SIZE];
                memset(zeroBuffer,0,DIRECTORY_ENTRY_SIZE);
                memset(zeroBuffer,0,DIRECTORY_ENTRY_SIZE);
                writeflipp4Bytes(disk,addrBuffer[i1], (i2 * DIRECTORY_ENTRY_SIZE),0);
                setNumberOfBytesInFile(parentNr ,getInode(parentNr).numberOfBytesInFile - DIRECTORY_ENTRY_SIZE);
                return inodeNrToDelete;
            }else if(blockIsEmpty ){
                blockIsEmpty = false;
            }
        }
    }
}


/**
 * Gibt zurück ob der parentInodeNr und alle Ordner über ihm, Such Berechtigung hat(Ausführberechtigung)
 * @param parentInodeNr die Inodenummer vom Ordner dass zuallerst geprüft werden soll
 * @param uid Benutzer Id
 * @param gid Gruppen Id
 * @return Gibt zurück ob es erreichbar ist oder nicht
 */
bool isCheckFolderEntryReachable(EOS32_ino_t parentInodeNr, int uid, int gid) {//uses pushPointer in searchEntryByDirectoryWithName
    bool isRunning = true;
    EOS32_ino_t newParentInoNr = parentInodeNr;
    if(uid == 0){
        return true;
    }

    return true;
}

