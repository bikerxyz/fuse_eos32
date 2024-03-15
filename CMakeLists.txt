set(CMAKE_C_COMPILER "/usr/bin/gcc")
set(CMAKE_CXX_COMPILER "/usr/bin/g++")

cmake_minimum_required(VERSION 3.15)
project(fuse_EOS32 C)


set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra")
include_directories(/usr/include/fuse3)
include_directories(fuse/helper/fuseHelper)
include_directories(fuse/helper)
add_executable(
        fuse_EOS32 fuse/main_code/main.c fuse/main_code/starter.c
        fuse/helper/config.h fuse/helper/error.c fuse/helper/eos32/eos32Helper.c fuse/helper/fuseHelper/fuseHelper.c fuse/helper/io/ioHelper.c fuse/helper/eos32/itemManipulation/eos32Inode.c fuse/helper/eos32/itemManipulation/eos32Inode.h fuse/helper/eos32/itemManipulation/eos32SuperBlock.c fuse/helper/eos32/itemManipulation/eos32SuperBlock.h fuse/helper/tools.h fuse/helper/datatypes.c fuse/helper/debugHelper.c fuse/helper/debugHelper.h fuse/helper/userIdsConverter.c fuse/helper/userIdsConverter.h)

target_link_libraries(fuse_EOS32 fuse3 pthread)






