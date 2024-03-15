//
// Created by niklas on 10.01.23.
//

#ifndef CP_USERIDSCONVERTER_H
#define CP_USERIDSCONVERTER_H
#include <datatypes.h>
#include <errno.h>

typedef struct idConverter1{
    unsigned int linuxId;
    unsigned int eos32Id;
} IdConverter;
extern IdConverter *userIdConverter;
extern IdConverter *groupIdConverter;
extern unsigned int userIdConverterSize;
extern unsigned int groupIdConverterSize;
void readConverterFile(char *path);
unsigned int getFuseUserIdByEos32Id(unsigned int userEos32Id);
unsigned int getEos32UserIdByFuseId(unsigned int userFuseId);
unsigned int getFuseGroupIdByEos32Id(unsigned int groupEos32Id);
unsigned int getEos32GroupIdByFuseId(unsigned int groupFuseId);
#endif //CP_USERIDSCONVERTER_H
