//
// Created by niklas on 12.12.19.
//

#ifndef TEST_STARTER_H
#define TEST_STARTER_H

#define FUSE_USE_VERSION 31



#include "../helper/datatypes.h"
#include <stdio.h>
#include "../helper/config.h"
#include "../helper/error.h"
#include "../helper/eos32/eos32Helper.h"
#include "../helper/eos32/itemManipulation/eos32Inode.h"
#include <stddef.h>
#include <linux/fs.h>
#include "../helper/userIdsConverter.h"

#include <fuseHelper.h>
int starter(int argc, char **argv);
#endif //TEST_STARTER_H
