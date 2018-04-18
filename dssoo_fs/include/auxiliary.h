/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file  auxiliary.h
 * @brief   Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/*
 * @brief   Search for a free inode and set its value in inodes map to 1
 * @return  ID of the inode available if any, -1 if not
 * */
int ialloc(void);

/*
 * @brief   Search for a free data block, set its bytes to 0 and set its
 *          value in data blocks map to 1
 * @return  ID of the data block available if any, -1 if not
 * */
int alloc(void);

/*
 * @brief   Sets the value of the inode provided to 0 in inodes map
 * @return  0 if success, -1 if inode not found
 * */
int ifree(int inode_id);

/*
 * @brief   Sets the value of the data block provided to 0 in data block map
 * @return  0 if success, -1 if data block not found
 * */
int bfree(int block_id);

/*
 * @brief   Found the inode ID containing the file passed
 * @return  ID of the inode, -1 if not found
 * */
int namei(char *fname);

/*
 * @brief   Return the index of the data block that contains the byte indicated
 *          by the offset
 * @return  Index of the block, -1 if not found
 * */
int bmap(int inode_id, int offset);

/*
 * @brief   Writes the metadata in memory to the disk image
 * @return  0 if success, -1 if error
 * */
int fssync(void);
