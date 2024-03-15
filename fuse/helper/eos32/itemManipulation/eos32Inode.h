//
// Created by niklas on 27.02.20.
//

#ifndef SHFS_EOS32INODE_H
#define SHFS_EOS32INODE_H

#include "../../datatypes.h"
#include "../eos32Helper.h"
#include "../../tools.h"


#define INODES_PER_BLOCK 64
#define INODE_SIZE ((EOS32_BLOCK_SIZE) / (INODES_PER_BLOCK))
#define BLOCK_ADDRESSES_MAX_COUNT (DIRECT_BLOCK_COUNT + SINGLE_INDIRECT_COUNT + DOUBLE_INDIRECT_COUNT * SINGLE_INDIRECT_COUNT)
#define BLOCK_ADDRESSES_MAX_SIZE_BYTES (BLOCK_ADDRESSES_MAX_COUNT * sizeof(EOS32_daddr_t))


#define IFMT		070000	/* type of file */
#define   IFREG		040000	/* regular file */
#define   IFDIR		030000	/* directory */
#define   IFCHR		020000	/* character special */
#define   IFBLK		010000	/* block special */
#define   IFFREE	000000	/* reserved (indicates free inode) */
#define ISUID		004000	/* set user id on execution */
#define ISGID		002000	/* set group id on execution */
#define ISVTX		001000	/* save swapped text even after use */
#define IUREAD		000400	/* user's read permission */
#define IUWRITE		000200	/* user's write permission */
#define IUEXEC		000100	/* user's execute permission */
#define IGREAD		000040	/* group's read permission */
#define IGWRITE		000020	/* group's write permission */
#define IGEXEC		000010	/* group's execute permission */
#define IOREAD		000004	/* other's read permission */
#define IOWRITE		000002	/* other's write permission */
#define IOEXEC		000001	/* other's execute permission */

/*mask
 * 100 read
 * 010 write
 * 001 executeable
 * */
#define ACL_MASK_READ 4
#define ACL_MASK_WRITE 2
#define ACL_MASK_EXECUTION 1

typedef struct inode1{
    EOS32_ino_t nr;
    unsigned int typeMode;
    unsigned int numberOfLinksToFile;
    unsigned int ownersUserId;
    unsigned int ownersGroupId;
    EOS32_time_t timeCreated;
    EOS32_time_t timeLastModified;
    EOS32_time_t timeLastAccessed;
    EOS32_off_t  numberOfBytesInFile;
    EOS32_daddr_t directBlocks[DIRECT_BLOCK_COUNT];
    EOS32_daddr_t singleIndirectBlock;
    EOS32_daddr_t doubleIndirectBlock;
} Inode;

typedef struct inodeOffSets1{
    unsigned int typeMode;
    unsigned int numberOfLinksToFile;
    unsigned int ownersUserId;
    unsigned int ownersGroupId;
    unsigned int timeCreated;
    unsigned int timeLastModified;
    unsigned int timeLastAccessed;
    unsigned int numberOfBytesInFile;
    unsigned int directBlocks;
    unsigned int singleIndirectBlock;
    unsigned int doubleIndirectBlock;
} InodeOffSets;


typedef struct FreeInodeList{
    EOS32_ino_t *inodeNrs;
    EOS32_size_t size;
} FreeInodeList;

extern InodeOffSets inodeOffSets;

bool isInodeDirectory(Inode inode);
bool hasInodeUserRead(Inode inode);
bool hasInodeUserWrite(Inode inode);
bool hasInodeGroupRead(Inode inode);
bool hasInodeGroupWrite(Inode inode);
bool hasInodeOtherRead(Inode inode);
bool hasInodeOtherWrite(Inode inode);
bool hasInodeUserExecution(Inode inode);
bool hasInodeGroupExecution(Inode inode);
bool hasInodeOtherExecution(Inode inode);

bool isCheckAccessPrivileges(EOS32_ino_t inodeNr, int uid, int gid, int mask);


bool isInodeFreeByTypeMode(unsigned int typeMode);

Inode getInode(EOS32_ino_t inodeNr);

void setTypeMode(EOS32_ino_t inodeNr,unsigned int input);
void setNumberOfLinksToFile(EOS32_ino_t inodeNr,unsigned int input);
void setOwnersUserId(EOS32_ino_t inodeNr,unsigned int input);
void setOwnersGroupId(EOS32_ino_t inodeNr,unsigned int input);
void setTimeCreated(EOS32_ino_t inodeNr,EOS32_time_t input);
void setTimeLastModified(EOS32_ino_t inodeNr,EOS32_time_t input);
void setTimeLastAccessed(EOS32_ino_t inodeNr,EOS32_time_t input);
void setNumberOfBytesInFile(EOS32_ino_t inodeNr,EOS32_off_t input);
void setDirectBlocks(EOS32_ino_t inodeNr,EOS32_daddr_t *input);
void setSingleIndirectBlock(EOS32_ino_t inodeNr,EOS32_daddr_t input);
void setDoubleIndirectBlock(EOS32_ino_t inodeNr,EOS32_daddr_t input);

EOS32_size_t countBlocksOfInode(EOS32_ino_t inodeNr);

EOS32_daddr_t pushBlockToInode(EOS32_ino_t inodeNr);

//Attention double use
FreeInodeList getAndLookFreeInodes(FreeInodeList freeInodeList, EOS32_off_t numberOfInodes, bool infiniteSearching);

EOS32_daddr_t calcBlockInode(EOS32_ino_t input);
unsigned int calcBlockInodeOffset(EOS32_ino_t input);

void inodeCreator(
        EOS32_ino_t inodeNr,
        unsigned int typeMode,
        unsigned int numberOfLinksToFile,
        unsigned int ownersUserId,
        unsigned int ownersGroupId,
        EOS32_time_t timeCreated,
        EOS32_time_t timeLastModified,
        EOS32_time_t timeLastAccessed,
        EOS32_off_t  numberOfBytesInFile,
        EOS32_daddr_t directBlocks[DIRECT_BLOCK_COUNT],
        EOS32_daddr_t singleIndirectBlock,
        EOS32_daddr_t doubleIndirectBlock
);
unsigned int convertTypeModeFromFuseToEos(mode_t fuse_mode);

bool isCheckModifyFolder(EOS32_ino_t parentInodeNr, int uid, int gid);
#endif //SHFS_EOS32INODE_H
