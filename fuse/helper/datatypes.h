//
// Created by niklas on 09.12.19.
//

#ifndef TEST_DATATYPES_H
#define TEST_DATATYPES_H

#define DIRECT_BLOCK_COUNT 6
#define SINGLE_INDIRECT_COUNT 1024
#define DOUBLE_INDIRECT_COUNT (SINGLE_INDIRECT_COUNT * 1024)
#define ALL_BLOCKS_ON_INODE (DIRECT_BLOCK_COUNT+ 1 + SINGLE_INDIRECT_COUNT + SINGLE_INDIRECT_COUNT + DOUBLE_INDIRECT_COUNT )

#include "config.h"
#include "tools.h"

typedef unsigned int EOS32_daddr_t;
typedef unsigned int EOS32_size_t;
typedef unsigned int EOS32_ino_t;
typedef int EOS32_time_t;
typedef unsigned int EOS32_off_t;
typedef unsigned char EOS32_BLOCK_BUFFER[EOS32_BLOCK_SIZE];

typedef struct fileAddresses1{
    EOS32_size_t size;
    EOS32_daddr_t *addresses;
} FileAddresses;

//CAREFULLY READ THE COMMENT
/*typedef struct eos32UserData{
//dont forget to init data in starter.c, if you extend this struct
    size_t directoryEntryCounter;
} EOS32UserData;*/
extern unsigned char blockBuffer[EOS32_BLOCK_SIZE];


#endif //TEST_DATATYPES_H
