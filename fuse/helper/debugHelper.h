//
// Created by niklas on 19.11.20.
//

#ifndef CHECKER_DEBUGHELPER_H
#define CHECKER_DEBUGHELPER_H

#include <glob.h>

/*struct dirbuf {
    char *p;
    size_t size;
};*/
#include "fuseHelper.h"
//void printDirFuseBuffer(struct dirbuf input);
//void printDirFuseBufferPointer(unsigned char *pointer, size_t size, /*__unused*/ struct dirbuf b);
size_t listAllDisappearedFreeBlocks(EOS32_daddr_t *list);

#endif //CHECKER_DEBUGHELPER_H