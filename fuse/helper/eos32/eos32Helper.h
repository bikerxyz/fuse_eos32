//
// Created by niklas on 11.12.19.
//

#ifndef TEST_EOS32HELPER_H
#define TEST_EOS32HELPER_H



#include "../datatypes.h"
#include "../io/ioHelper.h"
#include "../config.h"
#include "itemManipulation/eos32Inode.h"
#include "itemManipulation/eos32SuperBlock.h"
#include "itemManipulation/eos32Inode.h"



#define DIRECTORY_ENTRY_NAME_SIZE 60
#define DIRECTORY_ENTRIES_PER_BLOCK 64
#define DIRECTORY_ENTRY_SIZE ((EOS32_BLOCK_SIZE) / (DIRECTORY_ENTRIES_PER_BLOCK))




//#define   IFMT		((unsigned int)070000)	// type of file
//#define   IFREG		((unsigned int)040000)	// regular file
//#define   IFDIR		((unsigned int)030000)	// directory
//#define   IFCHR		((unsigned int)020000)	// character special
//#define   IFBLK		((unsigned int)010000)	// block special






typedef struct DirectoryEntry1{
    EOS32_ino_t inodeNr;
    char name[DIRECTORY_ENTRY_NAME_SIZE + 1];
} DirectoryEntry;

typedef struct Directory1{
    DirectoryEntry directoryEntries[DIRECTORY_ENTRIES_PER_BLOCK];
}DirectoryBlock;

typedef struct DirectoryEntryOffSets1{
    size_t inodeNr;
    size_t name;
} DirectoryEntryOffSets;


void addDirEntry(EOS32_ino_t parentNr, EOS32_ino_t inodeNr, const char *name);

EOS32_ino_t removeDirEntry(EOS32_ino_t parentNr, const char *name);

extern EOS32_BLOCK_BUFFER zeroedBlock;



void getSuperBlock();

void getDirectory(EOS32_daddr_t blockNr, DirectoryBlock *directory);

void initSuperBlockOffSets(void);
void initInodeBlockOffSets(void);
void initDirectoryEntryOffSets(void);
void initAllOffSets(void);
void initAll(void);

bool isCheckFolderEntryReachable(EOS32_ino_t parentInodeNr, int uid, int gid);


FileAddresses getFileAddresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer);
FileAddresses getFileOrAllAdresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer, bool getFileAddr);
FileAddresses getAllAddresses(EOS32_ino_t inodeNr, EOS32_off_t offset, size_t maxCountAdresses, EOS32_daddr_t *addressBuffer);

size_t getDirectoryAdresses(EOS32_ino_t inodeNr, size_t size, size_t offset_directoryEntries, EOS32_daddr_t *buffer);
EOS32_ino_t searchEntryByDirectoryWithName(EOS32_ino_t parent, const char *name);

size_t readFile(EOS32_ino_t inodeNr, unsigned char *buffer, size_t size, off_t offset);


void destroyBuffer(void *pointer);
void destroyFileAddresses(FileAddresses fileAddresses);

#endif //TEST_EOS32HELPER_H



