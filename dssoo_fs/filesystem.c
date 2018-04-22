/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file  filesystem.c
 * @brief  Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include "include/filesystem.h"  // Headers for the core functionality
#include "include/auxiliary.h"  // Headers for auxiliary functions
#include "include/metadata.h"  // Type and structure declaration of the file system
#include "include/crc.h"   // Headers for the CRC functionality

/*
 * @brief  Generates the proper file system structure in a storage device, as designed by the student.
 * @return  0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
								int i; /* Auxiliary counter */
								int fd; /* File descriptor for DEVICE_IMAGE */

								/* Ckeck if device image has correct size */
								if (deviceSize < MIN_FILESYSTEM_SIZE) {
																printf("ERROR: Disk too small\n");
																return -1;
								}
								if (deviceSize > MAX_FILESYSTEM_SIZE) {
																printf("ERROR: Disk too big\n");
																return -1;
								}

								/* Open disk image for read and write */
								fd = open(DEVICE_IMAGE, O_RDWR);
								if (fd < 0) {
																perror("Error while opening 'disk.dat'");
																return -1;
								}
								unsigned long size = (unsigned long)lseek(fd, 0, SEEK_END);
								close(fd);

								if (size < deviceSize) {
									fprintf(stderr, "Error in mkFS: Disk too small\n");
									return -1;
								}

								/* Calculate the needed blocks for metadata following the design
								 * and the device size */

								unsigned int deviceBlocks = deviceSize/BLOCK_SIZE; /* Floor function applied when casting to int */
								unsigned int inodeMapBlocks = 1;
								unsigned int dataMapBlocks = 1;
								unsigned int inodeBlocks = 1;

								// unsigned int inodeBlocks = (unsigned int)ceil((MAX_FILESYSTEM_OBJECTS_SUPPORTED*sizeof(inode_t))/BLOCK_SIZE);
								// unsigned int inodeMapBlocks = (unsigned int)ceil((MAX_FILESYSTEM_OBJECTS_SUPPORTED*(unsigned int)sizeof(char))/BLOCK_SIZE);
								// unsigned int dataMapBlocks = ceil(deviceBlocks*sizeof(char)/BLOCK_SIZE);

								unsigned int dataBlockNum = deviceBlocks - 1 - inodeMapBlocks - dataMapBlocks - inodeBlocks;

								printf("superblock = 1\n");
								printf("deviceBlocks = %u\n", deviceBlocks);
								printf("inodeMapBlocks = %u\n", inodeMapBlocks);
								printf("dataMapBlocks = %u\n", dataMapBlocks);
								printf("Blocks for inodes = %u\n", inodeBlocks);

								if(dataBlockNum < 0) {
																fprintf(stderr, "Error: no data blocks available. Try to make a larger disk image\n");
																return -1;
								} else {
																printf("dataBlockNum = %d\n", dataBlockNum);
								}

								/* Initialize superblock */
								sblock.magicNum        = MAGIC_NUMBER;
								sblock.inodeMapNumBlocks  = inodeMapBlocks;
								sblock.dataMapNumBlock    = dataMapBlocks;
								sblock.numInodes       = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
								sblock.firstInodeBlock    = 1 + inodeMapBlocks + dataMapBlocks;
								sblock.dataBlockNum     = dataBlockNum;
								sblock.firstDataBlock    = 1 + inodeMapBlocks + dataMapBlocks + inodeBlocks;
								sblock.deviceSize       = deviceSize;
								memset(sblock.padding, '0', sizeof(sblock.padding));

								/* Write superblock to disk image */
								bwrite(DEVICE_IMAGE, 0, (char*)&sblock);

								/* Allocate space in memory for inodes map and initialize its elements to 0 */
								i_map = (char*)malloc(MAX_FILESYSTEM_OBJECTS_SUPPORTED/8); /* For allocating bits */
								for (i = 0; i < sblock.numInodes; i++) {
																// printf("Initializing i_map[%d]\n", i);
																bitmap_setbit(i_map, i, 0);
								}

								/* Allocate space in memory for data blocks map and initialize its elements to 0 */
								b_map = (char*)malloc(dataBlockNum*sizeof(char));
								for (i = 0; i < sblock.dataBlockNum; i++) {
																// printf("Initializing b_map[%d]\n", i);
																b_map[i] = 0;
								}

								/* Initialize array of iNodes to 0 */
								for (i = 0; i < sblock.numInodes; i++) {
																// printf("Initializing inodes[%d]\n", i);
																memset(&(inodes[i]), 0, sizeof(inode_t));
								}

								/* Call fssync() to write all the metadata created in the disk */
								if (fssync() < 0) {
																fprintf(stderr, "mkFS failed: Error when syncing metadata\n");
																return -1;
								}

								return 0;
}

/*
 * @brief  Mounts a file system in the simulated device.
 * @return  0 if success, -1 otherwise.
 */
int mountFS(void)
{
								int i;
								/* Read superblock (disk block 0) and store it into sblock */
								if (bread(DEVICE_IMAGE, 0, (char*)&(sblock)) < 0) {
																fprintf(stderr, "Error in mountFS: superblock cannot be read\n");
																return -1;
								}

								/* Read from disk inode map */
								for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
																if (bread(DEVICE_IMAGE, 1+i, ((char *)i_map + i*BLOCK_SIZE))) {
																								fprintf(stderr, "Error in mountFS: can't read inodes map\n");
																								return -1;
																}
								}

								/* Read disk block map */
								for (i = 0; i < sblock.dataMapNumBlock; i++) {
																if (bread(DEVICE_IMAGE, 1+i+sblock.inodeMapNumBlocks, ((char *)b_map + i*BLOCK_SIZE))) {
																								fprintf(stderr, "Error in mountFS: can't read data block map\n");
																								return -1;
																}
								}

								/* Read inodes from disk */
								for (i = 0; i < (sblock.numInodes*sizeof(inode_t)/BLOCK_SIZE); i++) {
																if (bread(DEVICE_IMAGE, i+sblock.firstInodeBlock, ((char *)inodes + i*BLOCK_SIZE))) {
																								fprintf(stderr, "Error in mountFS: can't read iNodes\n");
																								return -1;
																}
								}

								return 0;
}

/*
 * @brief  Unmounts the file system from the simulated device.
 * @return  0 if success, -1 otherwise.
 */
int unmountFS(void)
{
								unsigned int i;

								/* Check if any file still opened */
								for (i = 0; i < sblock.numInodes; i++) {
																if(inodes_x[i].opened == 1) {
																								fprintf(stderr, "Error in unmountFS: file %s is opened\n", inodes[i].name);
																								return -1;
																}
																inodes_x[i].position = 0;
								}

								/* Call fssync() to write metadata on file */
								if (fssync() < 0) {
																fprintf(stderr, "Error in unmountFS: error while syncing metadata\n");
																return -1;
								}

								free(i_map);
								free(b_map);

								return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
								unsigned int b_id, inode_id;
								char buf[BLOCK_SIZE];

								if(fileName == NULL) {return -2;}   /* Error with file name*/

								if (strlen(fileName) > FILENAME_MAXLEN) {
																fprintf(stderr, "Error in createFile: file name too long\n");
																return -2;
								}

								if(namei(fileName) != -1) {
																fprintf(stderr, "Error in createFile: file %s already exists\n", fileName);
																return -1;
								} /* File name already exists */

								/* Allocate an inode for the new file */
								inode_id = ialloc();
								if(inode_id < 0) {
																fprintf(stderr, "Error in createFile: maximum number of files in the disk\n");
																return -2;
								}

								/* Allocate indirect block */
								b_id = alloc();
								if(b_id < 0) {
																ifree(inode_id); /* Free created inode */
																fprintf(stderr, "Error in createFile: disk is full\n");
																return -2;
								}

								/* INITIALIZATION OF INODE */
								/* Initialize name */
								memset(inodes[inode_id].name, '\0', sizeof(FILENAME_MAXLEN));
								strcpy(inodes[inode_id].name, fileName);

								inodes[inode_id].undirectBlock = b_id;

								/* Initialize undirectBlock */
								undirectBlock_t newIndBlock;
								memmove(buf, &newIndBlock, sizeof(undirectBlock_t));
								/* Write undirectBlock to disk */
								if (bwrite(DEVICE_IMAGE, b_id, buf) < 0) {
																fprintf(stderr, "Error in createFile: can't write undirectBlock to disk\n");
																return -2;
								}

								/* Initialize size of the file */
								inodes[inode_id].size = 0;

								/* Set seek descriptor to 0 and open state */
								inodes_x[inode_id].position = 0;
								inodes_x[inode_id].opened = 1;

								return 0;
}

/*
 * @brief	Deletes a file, provided it exists in the file system.
 * @return	0 if success, -1 if the file does not exist, -2 in case of error..
 */
int removeFile(char *fileName)
{
								int i;
								int size, inode_id, undirectBlock_id;
								char buf[BLOCK_SIZE];

								if (fileName == NULL) {
																fprintf(stderr, "Error in removeFile: fileName can't be NULL\n");
																return -2;
								}

								inode_id = namei(fileName); /* Search file inode */
								if (inode_id < 0) {
																fprintf(stderr, "Error in removeFile: file %s not found\n", fileName);
																return -1;
								}

								size = inodes[inode_id].size;
								undirectBlock_id = inodes[inode_id].undirectBlock;

								// char *buf = (char*)malloc(sizeof(undirectBlock_t));
								undirectBlock_t u_block;
								if (bread(DEVICE_IMAGE, undirectBlock_id, (char*)&u_block) < 0) {
																fprintf(stderr, "Error in removeFile: can't read undirectBlock\n");
																return -2;
								}
								// memmove(&u_block, buf, BLOCK_SIZE);
								// free(buf);

								i = 0;
								memset(&buf, 0, BLOCK_SIZE);
								if(inodes[inode_id].size != 0) { /* If the size is 0, no data blocks are used */
																do {
																								/* Wipe data setting bytes to 0 */
																								if (bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], buf) < 0) {
																																fprintf(stderr, "Warning: block %u not properly deleted\n", u_block.dataBlocks[i]);
																								}
																								bfree(u_block.dataBlocks[i]);
																								i++;
																								size -= BLOCK_SIZE;
																} while(size > 0);
								}

								bfree(inodes[inode_id].undirectBlock);

								closeFile(inode_id);

								/* Delete and free inode */
								memset(&(inodes[inode_id]), 0, sizeof(inode_t));
								ifree(inode_id);

								return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName)
{
								int inode_id;

								inode_id = namei(fileName);
								if (inode_id < 0) {
																fprintf(stderr, "Error in openFile: file %s not found\n", fileName);
																return -1;
								}

								if (checkFile(fileName) != 0) {
									fprintf(stderr, "Error in openFile: file %s is corrupted\n", fileName);
								}

								inodes_x[inode_id].position = 0; /* Set seek descriptor to begin */
								inodes_x[inode_id].opened = 1;  /* Set file state to open */

								return inode_id;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
								if (fileDescriptor < 0 || fileDescriptor >= MAX_FILESYSTEM_OBJECTS_SUPPORTED) {
																fprintf(stderr, "Error in closeFile: wrong file descriptor\n");
																return -1;
								}

								inodes_x[fileDescriptor].position = 0; /* Set seek descriptor to begin */
								inodes_x[fileDescriptor].opened = 0;  /* Set file state to closed */

								return 0;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
								char b[BLOCK_SIZE];
								//unsigned int b_id;

								if (inodes_x[fileDescriptor].position+numBytes > inodes[fileDescriptor].size) {
																numBytes = inodes[fileDescriptor].size - inodes_x[fileDescriptor].position;
								}

								if (numBytes < 0) {
																fprintf(stderr, "Error reading file: Segmentation fault\n");
																return -1;
								}

								if (numBytes == 0) {
									return 0;
								}

								unsigned int u_block_id = inodes[fileDescriptor].undirectBlock;
								undirectBlock_t u_block;
								bread(DEVICE_IMAGE, u_block_id, (char*)&u_block);

								unsigned int copiedSoFar = 0;
								int bytesRemainingOnCurrentBlock = 0;
								bytesRemainingOnCurrentBlock = BLOCK_SIZE - (inodes_x[fileDescriptor].position%BLOCK_SIZE);
								unsigned int i;
								i = bmap(fileDescriptor, inodes_x[fileDescriptor].position);

								if (bytesRemainingOnCurrentBlock > numBytes) {
									bytesRemainingOnCurrentBlock = numBytes;
								}

								while (numBytes > 0) {
																//b_id = bmap(fileDescriptor, inodes_x[fileDescriptor].position);
																printf("READING B_ID %d\n", i);
																if (bytesRemainingOnCurrentBlock == BLOCK_SIZE) {
																								bytesRemainingOnCurrentBlock = 0;
																}
																if (bytesRemainingOnCurrentBlock > 0) {
																								bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																								memcpy(buffer+copiedSoFar, b+inodes_x[fileDescriptor].position, bytesRemainingOnCurrentBlock);
																								inodes_x[fileDescriptor].position += bytesRemainingOnCurrentBlock;
																								printf("bytesRemainingOnCurrentBlock = %d\n", bytesRemainingOnCurrentBlock);
																								copiedSoFar += bytesRemainingOnCurrentBlock;
																								numBytes -= bytesRemainingOnCurrentBlock;
																								bytesRemainingOnCurrentBlock = 0;
																} else {
																								if (numBytes < BLOCK_SIZE) {
																																bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																																memcpy(buffer+copiedSoFar, b, numBytes);
																																inodes_x[fileDescriptor].position += numBytes;
																																copiedSoFar += numBytes;
																																numBytes = 0;
																								} else {
																																bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																																memcpy(buffer+copiedSoFar, b, BLOCK_SIZE);
																																inodes_x[fileDescriptor].position += BLOCK_SIZE;
																																copiedSoFar += BLOCK_SIZE;
																																numBytes -= BLOCK_SIZE;
																								}
																}
																i++;
								}

								return copiedSoFar;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
								char b[BLOCK_SIZE];
								//unsigned int b_id;

								if (inodes_x[fileDescriptor].position+numBytes > MAX_FILE_SIZE) {
																numBytes = MAX_FILE_SIZE - inodes_x[fileDescriptor].position;
																printf("Warning: Only %d bytes will be written\n", numBytes);
								}

								if (numBytes <= 0) {
																printf("Warning: no bytes have been written\n");
																return 0;
								}

								unsigned int u_block_id = inodes[fileDescriptor].undirectBlock;
								undirectBlock_t u_block;
								bread(DEVICE_IMAGE, u_block_id, (char*)&u_block);

								unsigned int i = 0;
								unsigned int copiedSoFar = 0;

								unsigned int bytesFreeOnCurrentBlock = 0;
								unsigned int blocksToAllocate = 0;
								unsigned int blocksAlreadyUsed;

								if (inodes[fileDescriptor].size == 0) {
																blocksAlreadyUsed = 0;
								} else {
																blocksAlreadyUsed = (inodes[fileDescriptor].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
								}

								bytesFreeOnCurrentBlock = BLOCK_SIZE - (inodes_x[fileDescriptor].position%BLOCK_SIZE);

								/* Do we need to allocate new blocks for writing? */
								if (inodes_x[fileDescriptor].position+numBytes > inodes[fileDescriptor].size) {
																/* Yes, we do */
																/* Ceiling function for integer numbers division */
																blocksToAllocate = ((numBytes-bytesFreeOnCurrentBlock) + BLOCK_SIZE - 1) / BLOCK_SIZE;
								} else {
																/* The user wants to overwrite data */
								}

								for (i = 0; i < blocksToAllocate; i++) {
																u_block.dataBlocks[blocksAlreadyUsed+i] = alloc();
								}

								i = bmap(fileDescriptor, inodes_x[fileDescriptor].position);;

								while (numBytes > 0) {
																//b_id = bmap(fileDescriptor, inodes_x[fileDescriptor].position);
																memset(b, 0, BLOCK_SIZE);
																if (bytesFreeOnCurrentBlock == BLOCK_SIZE) {
																								bytesFreeOnCurrentBlock = 0;
																}
																/* Fill current block remaining space first (if any) */
																if (bytesFreeOnCurrentBlock > 0) {
																								bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																								memcpy(b+inodes_x[fileDescriptor].position, buffer+copiedSoFar, bytesFreeOnCurrentBlock);
																								bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																								inodes_x[fileDescriptor].position += bytesFreeOnCurrentBlock;
																								copiedSoFar += bytesFreeOnCurrentBlock;
																								numBytes -= bytesFreeOnCurrentBlock;
																								bytesFreeOnCurrentBlock = 0;
																} else {
																								/* Then fill the new allocated blocks (if any) */
																								if (numBytes < BLOCK_SIZE) {
																																/* Last operation if enters here */
																																memcpy(b, buffer+copiedSoFar, numBytes);
																																inodes_x[fileDescriptor].position += numBytes;
																																copiedSoFar += numBytes;
																																numBytes = 0;
																								} else {
																																memcpy(b, buffer+copiedSoFar, BLOCK_SIZE);
																																inodes_x[fileDescriptor].position += BLOCK_SIZE;
																																copiedSoFar += BLOCK_SIZE;
																																numBytes -= BLOCK_SIZE;
																								}
																								bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																								i++;
																}


																printf("Aqui la blokada %d **********************\n", i);
																printf("%s\n", b);
																printf("******************************************\n");
								}

								bwrite(DEVICE_IMAGE, u_block_id, (char*)&u_block);
								inodes[fileDescriptor].size += copiedSoFar;

								unsigned char buf[BLOCK_SIZE];
								inodes[fileDescriptor].checksum = 0;

								blocksAlreadyUsed = (inodes[fileDescriptor].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
								for (i = 0; i < blocksAlreadyUsed; i++) {
																bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																memcpy(buf, b, BLOCK_SIZE);
																inodes[fileDescriptor].checksum = CRC16(buf, BLOCK_SIZE, inodes[fileDescriptor].checksum);
								}

								printf("CHECKSUM CHANGED FILE: %d\n", inodes[fileDescriptor].checksum);

								if (inodes_x[fileDescriptor].position == inodes[fileDescriptor].size) {
									return 0;
								}

								return copiedSoFar;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if success, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
								/* to check the inode_id vality */
								if (fileDescriptor > sblock.numInodes) {
																return -1;
								}

								switch (whence) {
								case FS_SEEK_CUR:
																if ((inodes_x[fileDescriptor].position + offset >= inodes[fileDescriptor].size)
																				||(inodes_x[fileDescriptor].position + offset < 0)) {
																								fprintf(stderr, "Error in lseekFile: out of bounds of file\n");
																								return -1;
																}
																inodes_x[fileDescriptor].position += offset;
																break;
								case FS_SEEK_END:
																inodes_x[fileDescriptor].position = inodes[fileDescriptor].size - 1;
																break;
								case FS_SEEK_BEGIN:
																inodes_x[fileDescriptor].position = 0;
																break;
								}

								return 0;
}

/*
 * @brief  Verifies the integrity of a file.
 * @return  0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
								int fd;
								char b[BLOCK_SIZE];
								unsigned char buf[BLOCK_SIZE];
								int i;
								fd = namei(fileName);

								if (fd < 0) {
																fprintf(stderr, "Error in checkFile: No such file or directory");
																return -2;
								}

								// printf("CHECKSUM de %s %u\n", fileName, inodes[fd].checksum);
								uint16_t now = 0;

								unsigned int blocksAlreadyUsed = (inodes[fd].size + BLOCK_SIZE - 1) / BLOCK_SIZE;

								unsigned int u_block_id = inodes[fd].undirectBlock;
								undirectBlock_t u_block;
								bread(DEVICE_IMAGE, u_block_id, (char*)&u_block);

								for (i = 0; i < blocksAlreadyUsed; i++) {
																bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
																memmove(buf, b, BLOCK_SIZE);
																now = CRC16(buf, BLOCK_SIZE, now);
								}

								// printf("CHECKSUM de %s %u\n", fileName, now);
								if (inodes[fd].checksum == now) {
																/* File not corrupted */
																return 0;
								} else {
																/* File corrupted */
																return -1;
								}

								return -1;
}


int ialloc(void) {
								int i;
								/* To search for a free inode */
								for (i = 0; i < sblock.numInodes; i++) {
																if (bitmap_getbit(i_map, i) == 0) {
																								/* inode busy right now */
																								bitmap_setbit(i_map, i, 1);
																								/* Default values for the inode */
																								memset(&(inodes[i]), 0, sizeof(inode_t));
																								/* Return the inode identification */
																								return i;
																}
								}
								return -1;
}

int alloc(void) {
								int i;
								char buffer[BLOCK_SIZE];

								for (i = 0; i < sblock.dataBlockNum; i++) {
																if (bitmap_getbit(b_map, i) == 0) {
																								/* busy block right now */
																								bitmap_setbit(b_map, i, 1);

																								/* default values for the block */
																								memset(buffer, 0, BLOCK_SIZE);
																								bwrite(DEVICE_IMAGE, i+sblock.firstDataBlock, buffer);
																								/* it returns the block id */
																								return i;
																}
								}
								return -1;
}

int ifree(int inode_id) {
								/* to check the inode_id vality */
								if (inode_id > sblock.numInodes) {
																return -1;
								}

								/* free inode */
								bitmap_setbit(i_map, inode_id, 0);

								return 0;
}

int bfree(int block_id) {
								/* to check the inode_id vality */
								if (block_id > sblock.dataBlockNum) {
																return -1;
								}

								/* free data block */
								bitmap_setbit(b_map, block_id, 0);

								return 0;
}

int namei(char *fname) {
								int i;

								if (fname == NULL) {
																// fprintf(stderr, "Error: file not found\n");
																return -1;
								}

								/* seek for the inode with name <fname> */
								for (i = 0; i < sblock.numInodes; i++) {
																if (!strcmp(inodes[i].name, fname)) {
																								return i;
																}
								}
								return -1;
}

int bmap(int inode_id, int offset) {
								/* check for if it is a valid inode ID */
								if (inode_id > sblock.numInodes) {
																return -1;
								}

								/* Este condicional esta bien si metemos algo para files pequeñas para optimizar*/
								// if (offset < BLOCK_SIZE) {
								//   return inodes[inode_id].directBlock;
								// }

								unsigned int block = 0;
								while(offset >= BLOCK_SIZE) {
																offset -= BLOCK_SIZE;
																block++;
								}


								return block;
}

int fssync(void) {
								int i;
								/* Write super into disk */
								bwrite(DEVICE_IMAGE, 0, (char*)&sblock);

								/* Write inode map to disk */
								for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
																bwrite(DEVICE_IMAGE, 1+i, ((char *)i_map + i*BLOCK_SIZE));
								}

								/* Write block map to disk */
								for (i = 0; i < sblock.dataMapNumBlock; i++) {
																bwrite(DEVICE_IMAGE, 1+i+sblock.inodeMapNumBlocks, ((char *)b_map + i*BLOCK_SIZE));
								}

								/* Write inodes to disk */
								for (i = 0; i < (sblock.numInodes*sizeof(inode_t)/BLOCK_SIZE); i++) {
																bwrite(DEVICE_IMAGE, i+sblock.firstInodeBlock, ((char *)inodes + i*BLOCK_SIZE));
								}

								return 0;
}
