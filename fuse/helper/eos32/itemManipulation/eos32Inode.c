//
// Created by niklas on 27.02.20.
//

#include <fuseHelper.h>
#include "eos32Inode.h"
#include "../../tools.h"
#include "../../userIdsConverter.h"


InodeOffSets inodeOffSets;

bool isInodeDirectory(Inode inode){
    if((inode.typeMode & IFMT) == IFDIR){
        return true;
    } else{
        return false;
    }
}

bool hasInodeUserExecution(Inode inode){
    return (bool)(inode.typeMode & IUEXEC);
}
bool hasInodeGroupExecution(Inode inode){
    return (bool)(inode.typeMode & IGEXEC);
}
bool hasInodeOtherExecution(Inode inode){
    return (bool)(inode.typeMode & IOEXEC);
}

bool hasInodeUserRead(Inode inode){
    return  (bool)(inode.typeMode & IUREAD);
}
bool hasInodeUserWrite(Inode inode){
    return (bool) (inode.typeMode & IUWRITE);
}
bool hasInodeGroupRead(Inode inode){
    return (bool) (inode.typeMode & IGREAD);
}
bool hasInodeGroupWrite(Inode inode){
    return (bool) (inode.typeMode & IGWRITE);
}
bool hasInodeOtherRead(Inode inode){
    return (bool) (inode.typeMode & IOREAD);
}
bool hasInodeOtherWrite(Inode inode){
    return (bool) (inode.typeMode & IOWRITE);
}

bool isInodeFreeByTypeMode(unsigned int typeMode){
    return (typeMode == 0);
}

/*mask
 * 100 read
 * 010 write
 * 001 executeable
 * */
bool isCheckAccessPrivileges(EOS32_ino_t inodeNr, int uid, int gid, int mask) {
    Inode inode = getInode(inodeNr);
    if(uid == 0){
        return true;
    }
    if (inode.ownersUserId == uid) {
        if (((mask & ACL_MASK_READ))) {
            if (!hasInodeUserRead(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_WRITE)) {
            if (!hasInodeUserWrite(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_EXECUTION)) {
            if (!hasInodeUserExecution(inode)) {
                return false;
            }
        }
    }else if (inode.ownersGroupId == gid) {
        if (((mask & ACL_MASK_READ))) {
            if (!hasInodeGroupRead(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_WRITE)) {
            if (!hasInodeGroupWrite(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_EXECUTION)) {
            if (!hasInodeGroupExecution(inode)) {
                return false;
            }
        }
    }else {
        if (((mask & ACL_MASK_READ))) {
            if (!hasInodeOtherRead(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_WRITE)) {
            if (!hasInodeOtherWrite(inode)) {
                return false;
            }
        }
        if ((mask & ACL_MASK_EXECUTION)) {
            if (!hasInodeOtherExecution(inode)) {
                return false;
            }
        }
    }
    return true;
}

Inode getInode(EOS32_ino_t inodeNr) {
    Inode inode;

    EOS32_ino_t inodeBlockNr = inodeNr / INODES_PER_BLOCK + 2;
    readBytes(disk, inodeBlockNr, (inodeNr % (unsigned int) INODES_PER_BLOCK) * INODE_SIZE, INODE_SIZE, blockBuffer);


    inode.typeMode = (int) get4Bytes(blockBuffer + inodeOffSets.typeMode);

    if(isInodeFreeByTypeMode(inode.typeMode)){//check inode is free
        longjmp(env,EIO);
    }
    inode.nr = inodeNr;
    inode.numberOfLinksToFile = (int) get4Bytes(blockBuffer + inodeOffSets.numberOfLinksToFile);
    inode.ownersUserId = (int) getFuseUserIdByEos32Id(get4Bytes(blockBuffer + inodeOffSets.ownersUserId));
    inode.ownersGroupId = (int) getFuseGroupIdByEos32Id(get4Bytes(blockBuffer + inodeOffSets.ownersGroupId));
    inode.timeCreated = get4Bytes(blockBuffer + inodeOffSets.timeCreated);
    inode.timeLastModified = get4Bytes(blockBuffer + inodeOffSets.timeLastModified);
    inode.timeLastAccessed = get4Bytes(blockBuffer + inodeOffSets.timeLastAccessed);
    inode.numberOfBytesInFile = get4Bytes(blockBuffer + inodeOffSets.numberOfBytesInFile);
    inode.singleIndirectBlock = get4Bytes(blockBuffer + inodeOffSets.singleIndirectBlock);
    inode.doubleIndirectBlock = get4Bytes(blockBuffer + inodeOffSets.doubleIndirectBlock);

    for(int i1 = 0; i1 < DIRECT_BLOCK_COUNT; i1++){
        inode.directBlocks[i1] = get4Bytes(blockBuffer + inodeOffSets.directBlocks + i1 * sizeof(EOS32_daddr_t));
    }

    return inode;

}

/**
 * overwritten all information from the inode with zeros
 * @param inodeNr
 */
void setInodeAsFree(EOS32_ino_t inodeNr) {
    for(int i1 = 0; i1 < INODE_SIZE; i1++){
        writeByte(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + i1,0);
    }
}
void setTypeMode(EOS32_ino_t inodeNr,unsigned int input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.typeMode, input);
}
void setNumberOfLinksToFile(EOS32_ino_t inodeNr,unsigned int input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.numberOfLinksToFile,input);
}
void setOwnersUserId(EOS32_ino_t inodeNr,unsigned int input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.ownersUserId,input);
}
void setOwnersGroupId(EOS32_ino_t inodeNr,unsigned int input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.ownersGroupId,input);
}
void setTimeCreated(EOS32_ino_t inodeNr,EOS32_time_t input){
    writeflipp4Bytes(disk, calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.timeCreated,
                     (unsigned int) input);
}
void setTimeLastModified(EOS32_ino_t inodeNr,EOS32_time_t input){
    writeflipp4Bytes(disk, calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.timeLastModified,
                     (unsigned int) input);
}
void setTimeLastAccessed(EOS32_ino_t inodeNr,EOS32_time_t input){
    writeflipp4Bytes(disk, calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.timeLastAccessed,
                     (unsigned int) input);
}
void setNumberOfBytesInFile(EOS32_ino_t inodeNr,EOS32_off_t input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.numberOfBytesInFile,input);
}
void setDirectBlocks(EOS32_ino_t inodeNr,EOS32_daddr_t *input){
    for(int i1 = 0; i1 < DIRECT_BLOCK_COUNT;i1++){
        writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.directBlocks + i1 * sizeof(EOS32_daddr_t),input[i1]);
    }
}
void setSingleIndirectBlock(EOS32_ino_t inodeNr,EOS32_daddr_t input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.singleIndirectBlock,input);
}
void setDoubleIndirectBlock(EOS32_ino_t inodeNr,EOS32_daddr_t input){
    writeflipp4Bytes(disk,calcBlockInode(inodeNr),calcBlockInodeOffset(inodeNr) + inodeOffSets.doubleIndirectBlock,input);
}

EOS32_daddr_t calcBlockInode(EOS32_ino_t input){
    return input / INODES_PER_BLOCK + 2;
}
unsigned int calcBlockInodeOffset(EOS32_ino_t input){
    return (input % INODES_PER_BLOCK) * INODE_SIZE;
}

void inodeCreator(EOS32_ino_t inodeNr, unsigned int typeMode, unsigned int numberOfLinksToFile, unsigned int ownersUserId,
        unsigned int ownersGroupId, EOS32_time_t timeCreated, EOS32_time_t timeLastModified, EOS32_time_t timeLastAccessed,
        EOS32_off_t  numberOfBytesInFile, EOS32_daddr_t directBlocks[DIRECT_BLOCK_COUNT], EOS32_daddr_t singleIndirectBlock,
        EOS32_daddr_t doubleIndirectBlock){
    setTypeMode(inodeNr, typeMode);
    setNumberOfLinksToFile(inodeNr, numberOfLinksToFile);
    setOwnersUserId(inodeNr, ownersUserId);
    setOwnersGroupId(inodeNr, ownersGroupId);
    setTimeCreated(inodeNr, timeCreated);
    setTimeLastModified(inodeNr, timeLastModified);
    setTimeLastAccessed(inodeNr, timeLastAccessed);
    setNumberOfBytesInFile(inodeNr, numberOfBytesInFile);
    setDirectBlocks(inodeNr, directBlocks);
    setSingleIndirectBlock(inodeNr, singleIndirectBlock);
    setDoubleIndirectBlock(inodeNr, doubleIndirectBlock);
}

unsigned int convertTypeModeFromFuseToEos(mode_t fuse_mode){
    unsigned int typeMode = 0;

    if(S_ISREG(fuse_mode)) typeMode = IFREG;
    if(S_ISDIR(fuse_mode)) typeMode = IFDIR;
    if(S_ISCHR(fuse_mode)) typeMode = IFCHR;
    if(S_ISBLK(fuse_mode)) typeMode = IFBLK;

    if(fuse_mode & S_IWUSR)typeMode = typeMode | IUWRITE;
    if(fuse_mode & S_IRUSR)typeMode = typeMode | IUREAD;
    if(fuse_mode & S_IXUSR)typeMode = typeMode | IUEXEC;
    if(fuse_mode & S_IWGRP)typeMode = typeMode | IGWRITE;
    if(fuse_mode & S_IRGRP)typeMode = typeMode | IGREAD;
    if(fuse_mode & S_IXGRP)typeMode = typeMode | IGEXEC;
    if(fuse_mode & S_IWOTH)typeMode = typeMode | IOWRITE;
    if(fuse_mode & S_IROTH)typeMode = typeMode | IOREAD;
    if(fuse_mode & S_IXOTH)typeMode = typeMode | IOEXEC;
    return typeMode;
}

void writeZeroesToBlock(EOS32_daddr_t input){
    writeBlock(disk,input,zeroedBlock);
}

EOS32_size_t countBlocksOfInode(EOS32_ino_t inodeNr){
    EOS32_size_t counter = 0;

    Inode inode = getInode(inodeNr);

    for(int i1 = 0; i1 < DIRECT_BLOCK_COUNT;i1++){
        if(inode.directBlocks[i1] == 0){
            return (EOS32_size_t) i1;
        }
    }
    counter += DIRECT_BLOCK_COUNT;
    if(inode.singleIndirectBlock == 0)return counter;
    counter++;

    EOS32_daddr_t buffer[SINGLE_INDIRECT_COUNT];
    readBlock(disk, inode.singleIndirectBlock, (unsigned char *)buffer);
    for(int i1 = 0; i1 < SINGLE_INDIRECT_COUNT;i1++){
        if(buffer[i1] == 0){
            return counter + i1;
        }
    }
    counter += SINGLE_INDIRECT_COUNT;
    if(inode.doubleIndirectBlock == 0){
        return counter;
    }
    counter++;

    readBlock(disk, inode.doubleIndirectBlock, (unsigned char *)buffer);
    for(int i1 = 0; i1 < DOUBLE_INDIRECT_COUNT && counter < UINT32_MAX;i1++){
        EOS32_daddr_t nextSingleIndirectLayerAddress = flip4ByteNumber(buffer[i1]);

        if(nextSingleIndirectLayerAddress == 0){
            return counter;
        }
        counter++;
        EOS32_daddr_t bufferSingleIndirect[SINGLE_INDIRECT_COUNT];
        readBlock(disk,nextSingleIndirectLayerAddress, (unsigned char *)bufferSingleIndirect);
        for(int i2 = 0; i2 < SINGLE_INDIRECT_COUNT && counter < UINT32_MAX;i2++){
            if(bufferSingleIndirect[i2] == 0){
                return counter;
            }else{
                counter++;
            }
        }

    }
    return counter;
}

//Attention double use
FreeInodeList getAndLookFreeInodes(FreeInodeList freeInodeList, EOS32_off_t numberOfInodes, bool infiniteSearching){
    EOS32_BLOCK_BUFFER buffer;

    freeInodeList.size = 0;
    for(int i1 = 2; i1 < (superBlock.sizeOfInodeBlocks + 2);i1++){
        readBlock(disk, (EOS32_daddr_t) i1, buffer);

        for(int i2 = 0; i2 < INODES_PER_BLOCK;i2++){
            unsigned int typeMode = get4Bytes(buffer + (i2 * INODE_SIZE + inodeOffSets.typeMode));
            if(isInodeFreeByTypeMode(typeMode)){
                if(!infiniteSearching){
                    freeInodeList.inodeNrs[freeInodeList.size] = (EOS32_ino_t) ((i1 - 2) * INODES_PER_BLOCK + i2);
                    if(freeInodeList.size == numberOfInodes){
                        return freeInodeList;
                    }
                }
                freeInodeList.size++;
            }
        }
    }

    return freeInodeList;
}

EOS32_daddr_t pushBlockToInode(EOS32_ino_t inodeNr) {
    EOS32_daddr_t ret = popFreeBlock();
    Inode inode = getInode(inodeNr);

    writeZeroesToBlock(ret);

    EOS32_daddr_t currBlock = inode.numberOfBytesInFile / EOS32_BLOCK_SIZE;
    if(currBlock < DIRECT_BLOCK_COUNT){
        if(inode.directBlocks[currBlock] == 0){
            inode.directBlocks[currBlock] = ret;
            setDirectBlocks(inodeNr,inode.directBlocks);

            return ret;
        }else{
            //todo:make error
        }
    }else if(currBlock < (DIRECT_BLOCK_COUNT + SINGLE_INDIRECT_COUNT)){
        EOS32_off_t offsetAddressSingleIndirect = (currBlock - DIRECT_BLOCK_COUNT) * sizeof(EOS32_daddr_t);
        if(inode.singleIndirectBlock == 0){
            EOS32_daddr_t tmp1 = popFreeBlock();
            writeZeroesToBlock(tmp1);
            inode.singleIndirectBlock = tmp1;
            setSingleIndirectBlock(inodeNr,tmp1);
        }

        EOS32_daddr_t address = 0;
        readBytes(disk,inode.singleIndirectBlock, offsetAddressSingleIndirect ,sizeof(EOS32_daddr_t),(unsigned char *) &address);
        address = flip4ByteNumber(address);
        if(address != 0){
            //todo make error
        }

        writeflipp4Bytes(disk, inode.singleIndirectBlock, offsetAddressSingleIndirect, ret);

        return ret;
    }else if(currBlock < (DIRECT_BLOCK_COUNT + SINGLE_INDIRECT_COUNT + DOUBLE_INDIRECT_COUNT)){
        EOS32_off_t currBlockNew = currBlock - SINGLE_INDIRECT_COUNT - DIRECT_BLOCK_COUNT;
        EOS32_off_t currBlockDoubleIndirectLayer = currBlockNew / SINGLE_INDIRECT_COUNT;
        EOS32_off_t currBlockSingleIndirectLayer = currBlockNew % SINGLE_INDIRECT_COUNT;

        if(inode.doubleIndirectBlock == 0){
            EOS32_daddr_t doubleIndirectAddress = popFreeBlock();
            writeZeroesToBlock(doubleIndirectAddress);
            inode.doubleIndirectBlock = doubleIndirectAddress;
            setDoubleIndirectBlock(inodeNr,doubleIndirectAddress);
        }

        EOS32_daddr_t singledIndirectLayerAddress = readFlipp4Bytes(disk,
                inode.doubleIndirectBlock,currBlockDoubleIndirectLayer * sizeof(EOS32_daddr_t));
        if(singledIndirectLayerAddress == 0){
            if(currBlockDoubleIndirectLayer < (DOUBLE_INDIRECT_COUNT - 1)){
                EOS32_daddr_t nextSingleIndirectAddressOnDoubleIndirectBlock =  popFreeBlock();
                writeZeroesToBlock(nextSingleIndirectAddressOnDoubleIndirectBlock);
                writeflipp4Bytes(disk, inode.doubleIndirectBlock, currBlockDoubleIndirectLayer * sizeof(EOS32_daddr_t), nextSingleIndirectAddressOnDoubleIndirectBlock);
                singledIndirectLayerAddress = nextSingleIndirectAddressOnDoubleIndirectBlock;
            }
        }

        EOS32_daddr_t nextFreeAddressSlot = readFlipp4Bytes(disk, singledIndirectLayerAddress, currBlockSingleIndirectLayer);
        if(nextFreeAddressSlot != 0){
            //todo: make error
        }
        writeflipp4Bytes(disk, singledIndirectLayerAddress, currBlockSingleIndirectLayer * sizeof(EOS32_daddr_t),ret);
        fflush(disk);

        return ret;
    }else{
        longjmp(env, EFBIG);
    }

    return ret;
}

bool isCheckModifyFolder(EOS32_ino_t parentInodeNr, int uid, int gid){
    isCheckFolderEntryReachable(parentInodeNr, uid, gid);
    isCheckAccessPrivileges(parentInodeNr, uid, gid, ACL_MASK_EXECUTION | ACL_MASK_WRITE);
}
