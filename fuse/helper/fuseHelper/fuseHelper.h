//
// Created by niklas on 12.12.19.
//



#ifndef TEST_FUSEHELPER_H
#define TEST_FUSEHELPER_H


#include "../config.h"
#include <fuse3/fuse_lowlevel.h>

#include "../datatypes.h"
#include "../error.h"
#include "../tools.h"
#include "../eos32/itemManipulation/eos32Inode.h"
#include "../eos32/eos32Helper.h"

struct dirbuf {
    char *p;
    size_t size;
};

size_t calcSizeOfDirBufEntry(fuse_req_t req, const char *name);

EOS32_ino_t castFuseToEosInode(fuse_ino_t inodeNr);
EOS32_size_t castSize_tToEOS32Size_t(size_t input);
EOS32_off_t castOffsetLinuxToEOS32Offset(off_t input);
unsigned int castSignedLongToUnsignedInt(long input);
bool convertUnsignedLongToUnsignedInt(unsigned long *input, unsigned int *input1);


bool dirbuf_add(fuse_req_t req, struct dirbuf *b, const char *name, fuse_ino_t ino, size_t maxSize, off_t *counter1);

EOS32_time_t getTimeNow();
int getInfoByInode(Inode inode, struct stat *stbuf);
void initFuseEntryParam(Inode inode,struct fuse_entry_param *e);

#endif //TEST_FUSEHELPER_H
