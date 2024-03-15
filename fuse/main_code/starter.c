//
// Created by niklas on 12.12.19.


//#define FUSE_USE_VERSION 31

#include "starter.h"

#include <fuse3/fuse_common.h>
#include <stdio.h>
#include <string.h>
#include "../helper/debugHelper.h"


typedef struct fuse_lowlevel_ops FuseLowLevelOps;



static void eos32_lookup(fuse_req_t req, fuse_ino_t parent, const char *name){
    int value = setjmp(env);
    if(value != 0){
        destroyAllPointer();
        fuse_reply_err(req, value);
        return;
    }else{
        getSuperBlock();
        fuse_req_userdata(req);


        const struct fuse_ctx *myFuseCtx = fuse_req_ctx(req);
        struct fuse_entry_param e;
        Inode inode;

        int uid = myFuseCtx->uid;
        int gid = myFuseCtx->gid;
        if(!isCheckFolderEntryReachable(parent, uid, gid)){
            longjmp(env, EACCES);
        }



        if(parent > MAX_INODE_NUMBER){
            longjmp(env,EIO);
        }
        EOS32_ino_t searchedInodeNr = searchEntryByDirectoryWithName(castFuseToEosInode(parent), name);

         if(searchedInodeNr == 0){
            longjmp(env,ENOENT);
        }

        inode = getInode(searchedInodeNr);

        memset(&e, 0,sizeof(e));
        initFuseEntryParam(inode,&e);
        fuse_reply_entry(req,&e);
    }

}


static void eos32_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }else {
        struct stat stbuf;
        memset(&stbuf, 0, sizeof(stbuf));
        EOS32_ino_t inodeNr = castFuseToEosInode(ino);
        Inode inode;

        inode = getInode(inodeNr);
        getInfoByInode(inode, &stbuf);//will only call getInfoByInode if getInode would be called successfully

        fuse_reply_attr(req, &stbuf, DEFAULT_ATTR_TIMEOUT);
    }
}



static void eos32_readdir(fuse_req_t req, fuse_ino_t ino, size_t size,
                          off_t off, struct fuse_file_info *fi){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }else{
        bool bufferIsFull = false;


        EOS32_ino_t inodeNr = castFuseToEosInode(ino);
        Inode inode;
        inode = getInode(inodeNr);

        if (!isInodeDirectory(inode)) {
            longjmp(env,ENOTDIR);
        }


        struct dirbuf b;
        memset(&b, 0, sizeof(b));
        b.p = malloc(size);
        memset(b.p, 0, size);


        size_t size1;
        size_t expectedAddressesCount = countBlocksOfInode(inode.nr);
        EOS32_daddr_t addressBuffer[expectedAddressesCount + 1];
        memset(addressBuffer,0,sizeof(EOS32_daddr_t) * expectedAddressesCount+1);


        size_t offsetDirectories = (size_t) (off / DIRECTORY_ENTRIES_PER_BLOCK);
        size1 = getDirectoryAdresses(inodeNr, expectedAddressesCount, offsetDirectories, addressBuffer);


        size_t limit;
        if(expectedAddressesCount > size1) {
            limit = size1;
        } else {
            limit = expectedAddressesCount;
        }

        bool firstStart = true;
        bool beforeFirstHit = true;

        DirectoryEntry lastDirEntry;
        memset(&lastDirEntry,0,sizeof(DirectoryEntry));

        off_t posCounter = off;
        EOS32_daddr_t beginning = (EOS32_daddr_t) (off % DIRECTORY_ENTRIES_PER_BLOCK);
        int i1;
        int i2;
        for (i1 = 0; i1 < limit && !bufferIsFull; i1++) {
            DirectoryBlock directory;
            getDirectory(addressBuffer[i1], &directory);
            if(!firstStart){
                //at the first time, wird weitergemacht und das kann in der mitte eines blocks sein, after begin all blocks at 0
                beginning= 0;
            }

            for (i2 = beginning; i2 < DIRECTORY_ENTRIES_PER_BLOCK && !bufferIsFull; i2++) {
                if (directory.directoryEntries[i2].inodeNr != 0) {
                    if(!beforeFirstHit){
                        if (dirbuf_add(req, &b, lastDirEntry.name,
                                       lastDirEntry.inodeNr, size, &posCounter) == false) {

                            bufferIsFull = true;
                        }
                        lastDirEntry = directory.directoryEntries[i2];
                    }else{
                        lastDirEntry = directory.directoryEntries[i2];
                        beforeFirstHit = false;
                    }


                }
                if(firstStart)firstStart = false;
                posCounter++;
            }
        }

        posCounter++;
        if(!bufferIsFull && lastDirEntry.inodeNr != 0)dirbuf_add(req,&b,lastDirEntry.name,lastDirEntry.inodeNr,size,&posCounter);


        fuse_reply_buf(req,b.p,b.size);
        free(b.p);
    }
}

static void eos32_open(fuse_req_t req, fuse_ino_t ino,
                          struct fuse_file_info *fi)
{

    const struct fuse_ctx *context = fuse_req_ctx(req);
    int uid = context->uid;
    int gid = context->gid;

    int val = setjmp(env);
    if(val != 0){
        destroyAllPointer();
        fuse_reply_err(req, val);
        return;
    }else{
        Inode inode = getInode(castFuseToEosInode(ino));
        if(isInodeDirectory(inode)){
            longjmp(env,EISDIR);
        }
        if(fi->flags & __O_DIRECT){
            longjmp(env,EINVAL);
        }


        if ((fi->flags & O_ACCMODE) == O_RDWR) {
            if(!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_READ | ACL_MASK_WRITE)) {
                longjmp(env, EACCES);
            }
        } else if ((fi->flags & O_ACCMODE) == O_RDONLY) {
            if (!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_READ)) {
                longjmp(env, EACCES);
            }
        } else if ((fi->flags & O_ACCMODE) == O_WRONLY) {
            if (!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_WRITE)) {
                longjmp(env, EACCES);
            }
        }


        if(fi->flags & O_TRUNC){
            FileAddresses fileAddresses;
            EOS32_daddr_t adresses[ALL_BLOCKS_ON_INODE];
            fileAddresses = getAllAddresses(inode.nr,0,ALL_BLOCKS_ON_INODE,adresses);

            pushFreeBlocks(fileAddresses);
            unsigned char zeroBuffer[INODE_SIZE];
            memset(zeroBuffer, 0,INODE_SIZE);
            EOS32_daddr_t blockNr = calcBlockInode(inode.nr);
            EOS32_daddr_t inodeOffset = calcBlockInodeOffset(inode.nr);
            writeBytes(disk,blockNr,inodeOffset + INODE_OFFSET_BLOCKADDRESS_BEGIN,INODE_BLOCKADDRESSES_SIZE,zeroBuffer);
            setNumberOfBytesInFile(inode.nr,0);
        }
    }


    fuse_reply_open(req, fi);
}

static void eos32_read(fuse_req_t req, fuse_ino_t ino, size_t size,
                          off_t off, struct fuse_file_info *fi)
{
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }else{
        Inode inode = getInode(castFuseToEosInode(ino));

        if(off > UINT32_MAX || size > UINT32_MAX){
            longjmp(env, EFBIG);
        }

        size_t inputSize = size;
        if((off + size) > UINT32_MAX){
            unsigned long difference = (off + size) - UINT32_MAX;//difference is smaller than UINT_32MAX
            inputSize = difference;
        }
        EOS32_size_t sizeNew = castSize_tToEOS32Size_t(inputSize);

        if(inode.numberOfBytesInFile < sizeNew) sizeNew = inode.numberOfBytesInFile;
        size_t countAddresses =  (sizeNew / EOS32_BLOCK_SIZE + (sizeNew % EOS32_BLOCK_SIZE == 0 ? 0 : 1));//modulo is for to add a not complete block if exist
        if(isInodeDirectory(inode)){
            longjmp(env,EISDIR);
        }
        if(off >= inode.numberOfBytesInFile){
            fuse_reply_buf(req, (const char *)NULL, 0 );
            return;
        }

        EOS32_daddr_t addressbuffer[countAddresses];
        FileAddresses addresses = getFileAddresses(castFuseToEosInode(ino), castOffsetLinuxToEOS32Offset(off), countAddresses, addressbuffer);

        unsigned char ret[sizeNew];
        memset(ret,0, sizeNew);
        size_t counter = 0;

        for(size_t i1 = 0;(i1) < addresses.size -1 ;i1++){
            readBlock(disk,addresses.addresses[i1],ret + (EOS32_BLOCK_SIZE * i1));
            counter += EOS32_BLOCK_SIZE;
        }
        readBytes(disk, addresses.addresses[addresses.size - 1], 0, (sizeNew % EOS32_BLOCK_SIZE == 0 ? EOS32_BLOCK_SIZE : sizeNew % EOS32_BLOCK_SIZE), ret + (EOS32_BLOCK_SIZE * (addresses.size - 1)));


        fuse_reply_buf(req, (const char *)ret, sizeNew );
    }
}

static void eos32_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi){

    const struct fuse_ctx *context = fuse_req_ctx(req);
    int uid = context->uid;
    int gid = context->gid;

    int val = setjmp(env);

    fi->keep_cache = 1;
    fi->cache_readdir = 1;

    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }else{
        Inode inode = getInode(castFuseToEosInode(ino));
        if(!isInodeDirectory(inode)){
            longjmp(env,ENOTDIR);
        }


        if(!isCheckFolderEntryReachable(inode.nr, uid, gid)){
            longjmp(env,EACCES);
        }

        if ((fi->flags & O_ACCMODE) == O_RDWR) {
            if(!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_READ | ACL_MASK_WRITE)) {
                longjmp(env, EACCES);
            }

        } else if ((fi->flags & O_ACCMODE) == O_RDONLY) {
            if (!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_READ)) {
                longjmp(env, EACCES);
            }

        } else if ((fi->flags & O_ACCMODE) == O_WRONLY) {
            if (!isCheckAccessPrivileges(inode.nr, uid, gid, ACL_MASK_WRITE)) {
                longjmp(env, EACCES);
            }
        }
    }


    fuse_reply_open(req, fi);
}


static void eos32_mknod(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, dev_t rdev){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }
    const struct fuse_ctx *context = fuse_req_ctx(req);
    uid_t uid = context->uid;
    uid_t gid = context->gid;
    uid_t userIdEOS32 = getEos32UserIdByFuseId(uid);
    uid_t groupIdEOS32 = getEos32GroupIdByFuseId(gid);

    Inode parentInode = getInode(castFuseToEosInode(parent));
    if(!isCheckFolderEntryReachable(parentInode.nr,uid, gid) ||
       !isCheckAccessPrivileges(parentInode.nr,uid, gid, ACL_MASK_WRITE)){
        longjmp(env, EACCES);
    }
    if((parentInode.numberOfBytesInFile + DIRECTORY_ENTRY_SIZE) > MAX_BYTES){
        longjmp(env,EFBIG);
    }

    EOS32_ino_t inodeNr = popFreeInode();
    EOS32_daddr_t directBlockCount[6];
    for(int i1 = 0;i1 < DIRECT_BLOCK_COUNT;i1++){
        directBlockCount[i1] = 0;
    }
    inodeCreator(inodeNr,convertTypeModeFromFuseToEos(mode),1,userIdEOS32,groupIdEOS32,getTimeNow(),getTimeNow(),getTimeNow(),0,directBlockCount,0,0);
    Inode inode = getInode(inodeNr);
    addDirEntry(castFuseToEosInode(parent),inodeNr,name);


    struct fuse_entry_param e;
    initFuseEntryParam(inode,&e);
    fuse_reply_entry(req,&e);
}

static void eos32_setAttr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi){

    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }
    const struct fuse_ctx *context = fuse_req_ctx(req);
    uid_t uid = context->uid;
    uid_t gid = context->gid;


    EOS32_ino_t inodeNr = castFuseToEosInode(ino);
    Inode inode = getInode(inodeNr);

    if(to_set & FUSE_SET_ATTR_MODE){
        if(uid != inode.ownersUserId && uid != 0) {
            longjmp(env, EACCES);
        }
        unsigned int typeMode = convertTypeModeFromFuseToEos(attr->st_mode);
        setTypeMode(inodeNr, typeMode);
    }

    if((to_set & FUSE_SET_ATTR_UID) || (to_set & FUSE_SET_ATTR_GID)){
        if(uid != 0){
            longjmp(env, EACCES);
        }
    }

    if((to_set & FUSE_SET_ATTR_ATIME) ||
    (to_set & FUSE_SET_ATTR_MTIME)||
    (to_set & FUSE_SET_ATTR_CTIME)||
    (to_set & FUSE_SET_ATTR_ATIME_NOW)||
    (to_set & FUSE_SET_ATTR_MTIME_NOW)){
        if((uid != inode.ownersUserId) && (uid != 0) && !isCheckAccessPrivileges(inodeNr, uid,gid,ACL_MASK_WRITE)){
            longjmp(env,EACCES);
        }

    }

    if((to_set & FUSE_SET_ATTR_SIZE)){
        longjmp(env, ENOSYS);
    }

    if(to_set & FUSE_SET_ATTR_UID){
        setOwnersUserId(inodeNr,getEos32UserIdByFuseId(attr->st_uid));
    }
    if(to_set & FUSE_SET_ATTR_GID){
        setOwnersGroupId(inodeNr,getEos32GroupIdByFuseId(attr->st_gid));
    }
    if(to_set & FUSE_SET_ATTR_SIZE){
        setNumberOfBytesInFile(inodeNr,castOffsetLinuxToEOS32Offset(attr->st_size));
    }

    if(to_set & FUSE_SET_ATTR_ATIME){
        setTimeLastAccessed(inodeNr, (EOS32_time_t) attr->st_atim.tv_sec);
    }
    if(to_set & FUSE_SET_ATTR_MTIME){
        setTimeLastModified(inodeNr, (EOS32_time_t) attr->st_mtim.tv_sec);
    }
    if(to_set & FUSE_SET_ATTR_CTIME){
        setTimeCreated(inodeNr, (EOS32_time_t) attr->st_ctim.tv_sec);
    }
    if(to_set & FUSE_SET_ATTR_ATIME_NOW){
        setTimeLastAccessed(inodeNr, getTimeNow());
    }
    if(to_set & FUSE_SET_ATTR_MTIME_NOW){
        setTimeLastModified(inodeNr,getTimeNow());
    }

    inode = getInode(inodeNr);
    struct stat stbuf;
    getInfoByInode(inode,&stbuf);

    fuse_reply_attr(req, &stbuf, DEFAULT_ATTR_TIMEOUT);
}

void eos32_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi){
    fuse_reply_err(req, ENOSYS);
}

void eos32_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi){
    int val = setjmp(env);
    size_t sizeCounter = 0;

    if(val != 0) {
        fuse_reply_err(req, val);
        return;
    }


    if(off > UINT32_MAX || size > UINT32_MAX){
        longjmp(env, EFBIG);
    }
    size_t inputSize = size;
    if((off + size) > UINT32_MAX){
        unsigned long difference = (off + size) - UINT32_MAX;//difference is smaller than UINT_32MAX
        inputSize = difference;
    }


    unsigned int ourSize = 0;
    if(!convertUnsignedLongToUnsignedInt(&inputSize,&ourSize)){
        longjmp(env, EFBIG);
    }

    unsigned int numberOfAdresses = ourSize / EOS32_BLOCK_SIZE;
    if((inputSize % 4096) != 0) numberOfAdresses++;// BEGRÜNDUNG: 4085 / 4096 = 0
    EOS32_off_t  ourOffset = castOffsetLinuxToEOS32Offset(off);

    Inode inode = getInode(castFuseToEosInode(ino));

    if(ourOffset > inode.numberOfBytesInFile){
        if((inode.numberOfBytesInFile % EOS32_BLOCK_SIZE) != 0){
            //hier wird auf eos32_block_size aufgerundet
            EOS32_off_t newSize= (inode.numberOfBytesInFile - (inode.numberOfBytesInFile % EOS32_BLOCK_SIZE) + EOS32_BLOCK_SIZE);
            sizeCounter += newSize - inode.numberOfBytesInFile;
            setNumberOfBytesInFile(inode.nr,newSize);
        }
        for(int i1 = 0; inode.numberOfBytesInFile < (ourOffset - EOS32_BLOCK_SIZE); i1++){
            inode = getInode(inode.nr);//todo: muss dass in der schleife gemacht werden
            pushBlockToInode(inode.nr);
            setNumberOfBytesInFile(inode.nr, inode.numberOfBytesInFile + EOS32_BLOCK_SIZE);
            sizeCounter += EOS32_BLOCK_SIZE;
        }
    }

    EOS32_daddr_t addresses[numberOfAdresses];
    FileAddresses fileAddresses = getFileAddresses(inode.nr,ourOffset,numberOfAdresses,addresses);

    bool isAtBeginning = true;
    EOS32_off_t offsetAtFirstBlock = 0;
    inode = getInode(inode.nr);
    EOS32_off_t currentOffset = ourOffset;
    if((ourOffset % EOS32_BLOCK_SIZE) != 0){
        isAtBeginning = false;
        offsetAtFirstBlock = ourOffset % EOS32_BLOCK_SIZE;

        EOS32_off_t sizeToWrite = 0;
        if((offsetAtFirstBlock + ourSize) < EOS32_BLOCK_SIZE){//check ob nur der erste teil ausgeführt wird
            sizeToWrite = ourSize;
        }else{
            sizeToWrite = EOS32_BLOCK_SIZE - offsetAtFirstBlock;
        };


        writeBytes(disk, fileAddresses.addresses[0], offsetAtFirstBlock, sizeToWrite, (unsigned char *)buf);


        sizeCounter += sizeToWrite;
        currentOffset += sizeToWrite;

        if(currentOffset > inode.numberOfBytesInFile){
            setNumberOfBytesInFile(inode.nr,currentOffset);
        }
    }


    for(int i1 = (isAtBeginning ? 0 : offsetAtFirstBlock); i1 < numberOfAdresses - 1;i1++){

        inode = getInode(inode.nr);
        EOS32_daddr_t address;
        if((currentOffset + EOS32_BLOCK_SIZE) <= inode.numberOfBytesInFile){
            address = fileAddresses.addresses[i1];
        }else{
            address = pushBlockToInode(inode.nr);
        }
        writeBlock(disk,address, (unsigned char *)buf + (i1 * EOS32_BLOCK_SIZE) + offsetAtFirstBlock);
        size_t size2 = ((unsigned long)inode.numberOfBytesInFile) + EOS32_BLOCK_SIZE;
        if(size2 > UINT32_MAX){
            longjmp(env, EFBIG);
        }
        currentOffset += EOS32_BLOCK_SIZE;
        sizeCounter += EOS32_BLOCK_SIZE;//todo not correct

        if(currentOffset > inode.numberOfBytesInFile){
            setNumberOfBytesInFile(inode.nr, (unsigned int)currentOffset);
        }
    }

    if((sizeCounter < ourSize)){
        unsigned int restSize ;
        if(ourSize <= EOS32_BLOCK_SIZE){
            restSize = ourSize;
        } else{
            restSize = (ourSize - (EOS32_BLOCK_SIZE - offsetAtFirstBlock)) % EOS32_BLOCK_SIZE;
        }

        if(restSize == 0) restSize = EOS32_BLOCK_SIZE;
        EOS32_daddr_t address;
        if((currentOffset + restSize) <= inode.numberOfBytesInFile){
            address = fileAddresses.addresses[numberOfAdresses - (numberOfAdresses > 1 ? 2 : 1)];
        }else{
            address = pushBlockToInode(inode.nr);
        }

        writeBytes(disk, address, 0, restSize, (unsigned char *)buf + inputSize - restSize);

        sizeCounter += restSize;
        currentOffset += restSize;
    }

    if(currentOffset > inode.numberOfBytesInFile){
        setNumberOfBytesInFile(inode.nr, (unsigned int)currentOffset);
    }

    inode = getInode(inode.nr);
    fuse_reply_write(req,sizeCounter);
}




unsigned long getAndLookNumberOfFreeBlocks(){
    unsigned long ret;
    unsigned long tmp1 = 0;
    ret = superBlock.sizeOfBlockFreeList;

    EOS32_daddr_t nextBlockFreeList = superBlock.nextBlockFreeList;
    while(nextBlockFreeList != 0){
        tmp1 =readFlipp4Bytes(disk,nextBlockFreeList, NUMBER_OF_FREE_BLOCK_ADDRESS_ON_FREEBLOCKLIST_CHAIN_NOT_SUPERBLOCK);
        ret += tmp1;
        nextBlockFreeList = readFlipp4Bytes(disk, nextBlockFreeList, LINK_BLOCK_OFFSET_ON_FREEBLOCKLIST_CHAIN_NOT_SUPERBLOCK);
    }
    return ret;
}



void eos32_init(void *userdata, struct fuse_conn_info *conn){

}

void eos32_statfs(fuse_req_t req, fuse_ino_t ino){
    unsigned long numberOfFreeBlocks =  getAndLookNumberOfFreeBlocks();

    FreeInodeList list;
    list.size = 0;
    list.inodeNrs = NULL;
    list = getAndLookFreeInodes(list,0,true);
    unsigned long numberOfFreeInodes = (unsigned long) list.size ;

    struct statvfs stbuf;
    stbuf.f_bsize = EOS32_BLOCK_SIZE/2;
    stbuf.f_frsize = EOS32_BLOCK_SIZE/2;
    stbuf.f_blocks = superBlock.sizeOfBlocks;

    stbuf.f_bfree = numberOfFreeBlocks;
    stbuf.f_bavail = numberOfFreeBlocks;

    stbuf.f_files = superBlock.sizeOfInodeBlocks * INODES_PER_BLOCK;
    stbuf.f_ffree = numberOfFreeInodes;
    stbuf.f_favail = numberOfFreeInodes;
    stbuf.f_namemax = 60;

    fuse_reply_statfs(req,&stbuf);
}

void eos32_unlink(fuse_req_t req, fuse_ino_t parent, const char *name){
     int val = setjmp(env);
     if(val != 0){
        fuse_reply_err(req,val);
        return;
    }else{
        const struct fuse_ctx *context = fuse_req_ctx(req);
        uid_t uid = context->uid;
        uid_t gid = context->gid;
        if(!isCheckFolderEntryReachable(parent,uid, gid) ||
           !isCheckAccessPrivileges(parent,uid, gid, ACL_MASK_WRITE)){
            longjmp(env, EACCES);
        }

        EOS32_ino_t inodeNr = searchEntryByDirectoryWithName(castFuseToEosInode(parent), name);

        if(inodeNr == 0){
            longjmp(env,ENOENT);
        }

        Inode inode = getInode(inodeNr);


        removeDirEntry((EOS32_ino_t) parent, name);

        if(inode.numberOfLinksToFile <= 0){

        }

        setNumberOfLinksToFile(inode.nr,inode.numberOfLinksToFile - 1);

        getSuperBlock();//debug

        fuse_reply_err(req,0);
    }
}

void eos32_forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req,val);
        return;
    }else{
        getSuperBlock();
        Inode inode = getInode(castFuseToEosInode(ino));

        if(inode.numberOfLinksToFile == 0){
            EOS32_size_t numberOfAddresses = countBlocksOfInode(inode.nr);
            if(inode.numberOfBytesInFile % EOS32_BLOCK_SIZE != 0) numberOfAddresses++;
            EOS32_daddr_t addressBuffer[numberOfAddresses];
            getSuperBlock();
            FileAddresses fileAddresses = getAllAddresses(inode.nr, 0, numberOfAddresses, addressBuffer);

            getSuperBlock();//debug
            if(fileAddresses.size == 1){
                pushFreeBlock(fileAddresses.addresses[0]);
            }else if(fileAddresses.size > 1){
                pushFreeBlocks(fileAddresses);
            }

            EOS32_daddr_t blockNr = calcBlockInode(inode.nr);
            EOS32_daddr_t inodeOffset = calcBlockInodeOffset(inode.nr);

            unsigned char zeroBuffer[INODE_SIZE];
            memset(zeroBuffer, 0,INODE_SIZE);

            writeBytes(disk,blockNr,inodeOffset,INODE_SIZE,zeroBuffer);
            fflush(disk);
        }else{

        }

        fuse_reply_none(req);
    }
}

void eos32_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req, val);
        return;
    }
    Inode parentInode = getInode(castFuseToEosInode(parent));
    if((MAX_BYTES - parentInode.numberOfBytesInFile < DIRECTORY_ENTRY_SIZE)){
        longjmp(env,EFBIG);
    }
    const struct fuse_ctx *context = fuse_req_ctx(req);
    uid_t uid = context->uid;
    uid_t gid = context->gid;
    if( !isCheckFolderEntryReachable(parentInode.nr,uid, gid) ||
        !isCheckAccessPrivileges(parentInode.nr, uid, gid, ACL_MASK_WRITE)){

        longjmp(env, EACCES);
    }

    struct fuse_entry_param e;
    EOS32_ino_t inodeNr = popFreeInode();

    EOS32_daddr_t directBlocks[DIRECT_BLOCK_COUNT];
    memset(directBlocks,0,sizeof(EOS32_daddr_t) * DIRECT_BLOCK_COUNT);

    mode = mode | S_IFDIR;

    inodeCreator(inodeNr, convertTypeModeFromFuseToEos(mode),1,
                 getEos32UserIdByFuseId(context->uid),getEos32GroupIdByFuseId(context->gid),
                 getTimeNow(),getTimeNow(),getTimeNow(),0,
                 directBlocks,0,0);

    addDirEntry(castFuseToEosInode(parent),inodeNr,name);


    addDirEntry(inodeNr, inodeNr, ".");
    addDirEntry(inodeNr, parent, "..");

    setNumberOfLinksToFile(parent,getInode(parent).numberOfLinksToFile +1);
    setNumberOfLinksToFile(inodeNr,getInode(inodeNr).numberOfLinksToFile +1);//muss nur um 1 erhöht werden, weil  der inode bereits mit einem linkcount von 1 angeleget wird

    initFuseEntryParam(getInode(inodeNr),&e);

    fuse_reply_entry(req,&e);
}

void eos32_rmDir(fuse_req_t req, fuse_ino_t parent, const char *name){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req,val);
        return;
    }else{
        //debug
        getSuperBlock();

        const struct fuse_ctx *context = fuse_req_ctx(req);
        uid_t uid = context->uid;
        uid_t gid = context->gid;
        if( !isCheckFolderEntryReachable(parent,uid, gid) ||
            !isCheckAccessPrivileges(parent, uid, gid, ACL_MASK_WRITE)){

            longjmp(env, EACCES);
        }

        EOS32_ino_t inodeNr = searchEntryByDirectoryWithName((EOS32_ino_t) parent, name);

        if(inodeNr == 0){
            longjmp(env,ENOENT);
        }

        Inode inode = getInode(inodeNr);

        if(inode.numberOfBytesInFile > 128){
            longjmp(env, ENOTEMPTY);
        }

        removeDirEntry((EOS32_ino_t) parent, name);

        if(inode.numberOfLinksToFile <= 1){

        }else{
            setNumberOfLinksToFile(inode.nr,inode.numberOfLinksToFile - 2);
        }

        Inode parentInode = getInode(castFuseToEosInode(parent));
        if(parentInode.numberOfLinksToFile <= 1){

        }else{
            setNumberOfLinksToFile(parentInode.nr, parentInode.numberOfLinksToFile -1);
        }

        //debug
        getSuperBlock();

        fuse_reply_err(req,0);
    }
}

void eos32_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname, unsigned int flags){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req,val);
        return;
    }else {
        const struct fuse_ctx *context = fuse_req_ctx(req);
        uid_t uid = context->uid;
        uid_t gid = context->gid;
        if( !isCheckFolderEntryReachable(parent,uid, gid) ||
            !isCheckAccessPrivileges(parent, uid, gid, ACL_MASK_WRITE)||
            !isCheckFolderEntryReachable(newparent,uid, gid) ||
            !isCheckAccessPrivileges(newparent, uid, gid, ACL_MASK_WRITE)){

            longjmp(env, EACCES);
        }

        Inode destinationDirectory = getInode(castFuseToEosInode(newparent));

        if((destinationDirectory.numberOfBytesInFile + DIRECTORY_ENTRY_SIZE) > MAX_BYTES){
            longjmp(env,EFBIG);
        }

        EOS32_ino_t foundedInodeNr = searchEntryByDirectoryWithName(destinationDirectory.nr, newname);
        if((flags == RENAME_EXCHANGE && flags == RENAME_NOREPLACE)//beide flags dürfen nicht zusammen angegeben werden
            //|| (flags == RENAME_WHITEOUT && flags == RENAME_EXCHANGE)//beide flags dürfen nicht zusammen angegeben werden
            // Kann auskommentiert werden weil Whiteout so oder so zum einval führt
            || (flags == RENAME_WHITEOUT)){//rename without wird nicht unterstüzt
            longjmp(env,EINVAL);
        }
        if(flags == RENAME_NOREPLACE && foundedInodeNr != 0){//noreplace gesetzt und ziel existiert, ist ein fehler
            longjmp(env,EEXIST);
        }else if(flags == RENAME_EXCHANGE && foundedInodeNr != 0){//src und dest existieren und werden ausgetauscht
            EOS32_ino_t srcInodeNr = removeDirEntry(parent,name);
            EOS32_ino_t destInodeNr = removeDirEntry(newparent, newname);
            addDirEntry(parent, destInodeNr, newname);
            addDirEntry(newparent, srcInodeNr, name);
        }else if(
                flags == 0// kein schalter angegeben
                || (flags == RENAME_EXCHANGE && foundedInodeNr == 0)//exchange flag gesetzt, aber zielpfad existiert nicht. founded inode nr kann weggelassen werden, weil wenn was gefunden wäre, wäre der ober if ausgeführt worden
                || (flags == RENAME_NOREPLACE )){//rename noreplace gestzt, aber ziel existiert nicht
            EOS32_ino_t inodeNrToMove = removeDirEntry(castFuseToEosInode(parent), name);
            if(foundedInodeNr != 0){
                removeDirEntry(destinationDirectory.nr, newname);
            }
            addDirEntry(castFuseToEosInode(newparent), inodeNrToMove, newname);
        }else{//Fall kein gültiger schalter
            longjmp(env,EINVAL);
        }

        if(!isInodeDirectory(destinationDirectory)){
            longjmp(env,ENOTDIR);
        }

        fuse_reply_err(req, 0);
    }
}

void eos32_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req,val);
        return;
    }else {
        const struct fuse_ctx *context = fuse_req_ctx(req);
        uid_t uid = context->uid;
        uid_t gid = context->gid;


        Inode parentInode = getInode(castFuseToEosInode(newparent));
        if(!isCheckFolderEntryReachable(newparent,uid, gid) ||
           !isCheckAccessPrivileges(newparent,uid, gid, ACL_MASK_WRITE)||
           !isCheckFolderEntryReachable(parentInode.nr,uid,gid)){
            longjmp(env, EACCES);
        }
        if((parentInode.numberOfBytesInFile + DIRECTORY_ENTRY_SIZE) > MAX_BYTES){
            longjmp(env,EFBIG);
        }
        EOS32_ino_t result = searchEntryByDirectoryWithName(castFuseToEosInode(newparent), newname);
        if (result != 0) {
            longjmp(env, EEXIST);
        }
        addDirEntry(castFuseToEosInode(newparent), castFuseToEosInode(ino), newname);
        struct fuse_entry_param e;
        initFuseEntryParam(getInode(castFuseToEosInode(ino)),&e);
        fuse_reply_entry(req,&e);
    }
}

void eos32_access(fuse_req_t req, fuse_ino_t ino, int mask){
    int val = setjmp(env);
    if(val != 0){
        fuse_reply_err(req,val);
        return;
    }
    const struct fuse_ctx *context = fuse_req_ctx(req);
    int uid = context->uid;
    int gid = context->gid;
    //F_OK;
    if(mask == 0 ){//die mask kann 0 sein, obwohl lookup bereits die existenz bestätigt haben muss.
        Inode inode = getInode(castFuseToEosInode(ino));//wird gecheckt ob die inodenummer auf einen gültigewn inode verweist.
        fuse_reply_err(req,0);
        return;
    }

    bool ret = isCheckAccessPrivileges(castFuseToEosInode(ino),uid,gid,mask);
    if(ret){
        fuse_reply_err(req,0);
    }else{
        longjmp(env, EACCES);
    }
}


FuseLowLevelOps eos32Ops = {
        .lookup = eos32_lookup,
        .getattr = eos32_getattr,
        .readdir = eos32_readdir,
        .open = eos32_open,
        .read = eos32_read,
        .opendir = eos32_opendir,
        .mknod = eos32_mknod,
        .setattr = eos32_setAttr,
        .flush = eos32_flush,
        .write = eos32_write,
        .init = eos32_init,
        .statfs = eos32_statfs,
        .unlink = eos32_unlink,
        .forget = eos32_forget,
        .mkdir = eos32_mkdir,
        .rmdir = eos32_rmDir,
        .rename = eos32_rename,
        .link = eos32_link,
        .access = eos32_access
};

#define DESCR_SIZE	20
typedef struct {
    unsigned long type;
    unsigned long start;
    unsigned long size;
    char descr[DESCR_SIZE];
} PartEntry;


void readPartTable(size_t partitionNr, size_t *start, size_t *size){
    unsigned char buffer[SECTOR_SIZE];
    unsigned char *bufferPointer = buffer;
    readBytesDisk(disk,DISK_PART_TABLE_BEGIN,SECTOR_SIZE,buffer);


    bufferPointer += partitionNr * PARTENTRY_SIZE;
    bufferPointer += 4;
    *start = get4Bytes(bufferPointer);
    bufferPointer += 4;
    *size = get4Bytes(bufferPointer);
}

int starter(int argc, char **argv){
    initAll();
    fsStart = 0x20;
    char *path = argv[2];
    disk = openDisk(path, DEFAULT_OPEN_MODUS);
    readConverterFile(argv[1]);
    printf("disk Path: %s\n" "converterFile path: %s\n",path, argv[1]);

    char *partNumberChar = argv[3];
    long partNumber;
    int partNumberLength = (int) strlen(partNumberChar);
    if(partNumberLength > 36){
        return 1;
    }

    partNumber = strtol(partNumberChar, NULL,10);
    if(partNumber < 0){
        sendError("Partitionnumber must be a positive number\n");
        return 1;
    }
    if(partNumber > 10){
        sendError("The Partitionnumber %ld is to big. Maximal Partitionnumber is 10\n", partNumber);
        return 1;
    }

    size_t start = 0;
    size_t size = 0;
    readPartTable(partNumber, &start,&size);

    fsStart = (EOS32_daddr_t) start;//der wie vielte sektor
    getSuperBlock();

    struct fuse_args args = FUSE_ARGS_INIT(argc - 3, argv + 3);//define (argc,argv) ->{argc, argv, 0}
    //0 is for question "allocated"

    struct fuse_session *se;//is in lib/fuse_i.h
    struct fuse_cmdline_opts opts;//is in include/fuse_lowlevel.h
    //information about operation. Example: singlethread, mount point...

    int ret = -1;//return value: at me ou(output value)


    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
        fuse_cmdline_help();
        fuse_lowlevel_help();
        ret = 0;
        goto err_out1;
    } else if (opts.show_version) {
        printf("FUSE library version %s\n", fuse_pkgversion());
        fuse_lowlevel_version();
        ret = 0;
        goto err_out1;
    }

    if(opts.mountpoint == NULL) {
        printf("usage: %s [options] <mountpoint>\n", argv[0]);
        printf("       %s --help\n", argv[0]);
        ret = 1;
        goto err_out1;
    }

    se = fuse_session_new(&args, &eos32Ops,sizeof(eos32Ops), NULL);

    if (se == NULL)
        goto err_out1;

    if (fuse_set_signal_handlers(se) != 0)
        goto err_out2;

    if (fuse_session_mount(se, opts.mountpoint) != 0)
        goto err_out3;

    fuse_daemonize(opts.foreground);

    /* Block until ctrl+c or fusermount -u */
    if (opts.singlethread)
        ret = fuse_session_loop(se);
    else
        ret = fuse_session_loop_mt(se, opts.clone_fd);

    fuse_session_unmount(se);
    err_out3:
    fuse_remove_signal_handlers(se);
    err_out2:
    fuse_session_destroy(se);
    err_out1:
    free(opts.mountpoint);
    fuse_opt_free_args(&args);

    return ret ? 1 : 0;
}