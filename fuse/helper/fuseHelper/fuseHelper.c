//
// Created by niklas on 12.12.19.
//

#include "fuseHelper.h"


bool dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name, fuse_ino_t ino, size_t maxSize, off_t *counter1) {
    struct stat stbuf;
    size_t oldsize = b->size;
    b->size += calcSizeOfDirBufEntry(req, name);
    if(b->size > maxSize){
        b->size = oldsize;
        return false;
    }

    b->p = (char *) realloc(b->p, b->size);
    memset(&stbuf, 0, sizeof(stbuf));
    memset((b->p) + oldsize, 0 , b->size - oldsize);
    stbuf.st_ino = ino;
    //das mit oldsize funktioniert, weil b->p immer auf den anfang des buffers zeigt deswegen muss oldSize dazuaddiert werden damit wir die nächste freie Zelle befüllen
    fuse_add_direntry(req, b->p + oldsize, b->size - oldsize, name, &stbuf,*counter1);
    return true;
}


size_t calcSizeOfDirBufEntry(fuse_req_t req, const char *name) {
    return fuse_add_direntry(req, NULL, 0, name, NULL, 0);
}

bool convertUnsignedLongToUnsignedInt(unsigned long *input, unsigned int *input1){
    if(*input > UINT32_MAX){
        return false;
    }else {
        *input1 = (unsigned int) *input;
        return true;
    }

}

EOS32_size_t castSize_tToEOS32Size_t(size_t input){
    return castFuseToEosInode(input);
}

EOS32_ino_t castFuseToEosInode(fuse_ino_t inodeNr){
    EOS32_ino_t ino = 0;
    if(convertUnsignedLongToUnsignedInt(&inodeNr, &ino)){
        return ino;
    }else{
        longjmp(env,EIO);
    }

}


EOS32_off_t castOffsetLinuxToEOS32Offset(off_t input){
    return castSignedLongToUnsignedInt(input);
}
unsigned int castSignedLongToUnsignedInt(long input){
    if(input < 0 || input > UINT32_MAX){
        if(input < 0){
            sendError("castSignedLongToUnsignedInt: cannot cast a negative number to unsigned(%ld)\n",input);
        }else{
            sendError("castSignedLongToUnsignedInt: number is to big for unsigned int(%ld)\n",input);
        }
        longjmp(env,EIO);
    }

    return (unsigned int)input;
}

void initFuseEntryParam(Inode inode,struct fuse_entry_param *e){
    e->ino = (long)inode.nr;//e.ino is uInt64_t=long //todo: cast  must to unsigned long
    e->attr_timeout = DEFAULT_ATTR_TIMEOUT;
    e->entry_timeout = DEFAULT_ENTRY_TIMEOUT;
    #pragma clang diagnostic push
    #pragma clang diagnostic ignored "-Wuninitialized"
        //inode is not uninitialized, inode come from getInode, if getInode make Error, will getInode make errorhandling
        //and this codeline cannot be reached
        getInfoByInode(inode, &(e->attr));
    #pragma clang diagnostic pop
}
int getInfoByInode(Inode inode, struct stat *stbuf){
    stbuf->st_ino = (__ino_t)inode.nr;
    stbuf->st_mode = 0;
    stbuf->st_nlink = inode.numberOfLinksToFile;//nlink is unsigned long
    stbuf->st_size = inode.numberOfBytesInFile;

    stbuf->st_atime = inode.timeLastAccessed;
    stbuf->st_mtime = inode.timeLastModified;
    stbuf->st_ctime = inode.timeCreated;

    if((inode.typeMode & IFMT) == IFREG) stbuf->st_mode = S_IFREG;//todo: clang tidy Use of a signed integer operator with a binary operator
    if((inode.typeMode & IFMT) == IFDIR) stbuf->st_mode = S_IFDIR;
    if((inode.typeMode & IFMT) == IFCHR) stbuf->st_mode = S_IFCHR;
    if((inode.typeMode & IFMT) == IFBLK) stbuf->st_mode = S_IFBLK;

    if(hasInodeUserRead(inode)) stbuf->st_mode = stbuf->st_mode | S_IRUSR;
    if(hasInodeUserWrite(inode)) stbuf->st_mode = stbuf->st_mode | S_IWUSR;
    if(inode.typeMode & IUEXEC) stbuf->st_mode = stbuf->st_mode |   S_IXUSR;

    if(hasInodeGroupRead(inode)) stbuf->st_mode = stbuf->st_mode | S_IRGRP;
    if(hasInodeGroupWrite(inode)) stbuf->st_mode = stbuf->st_mode | S_IWGRP;
    if(inode.typeMode & IGEXEC) stbuf->st_mode = stbuf->st_mode |   S_IXGRP;

    if(hasInodeOtherRead(inode)) stbuf->st_mode = stbuf->st_mode | S_IROTH;
    if(hasInodeOtherWrite(inode)) stbuf->st_mode = stbuf->st_mode | S_IWOTH;
    if(inode.typeMode & IOEXEC) stbuf->st_mode = stbuf->st_mode |   S_IXOTH;

    stbuf->st_uid = inode.ownersUserId;
    stbuf->st_gid = inode.ownersGroupId;

    return 0;
}