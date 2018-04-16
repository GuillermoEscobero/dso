/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	filesystem.c
 * @brief 	Implementation of the core file system funcionalities and auxiliary functions.
 * @date	01/03/2017
 */

#include "include/filesystem.h"		// Headers for the core functionality
#include "include/auxiliary.h"		// Headers for auxiliary functions
#include "include/metadata.h"		// Type and structure declaration of the file system
#include "include/crc.h"			// Headers for the CRC functionality

#include <math.h>

/*
 * @brief 	Generates the proper file system structure in a storage device, as designed by the student.
 * @return 	0 if success, -1 otherwise.
 */
int mkFS(long deviceSize)
{
	if (deviceSize < MIN_FILESYSTEM_SIZE) {
		printf("ERROR: Disk too small\n");
		return -1;
	} else if (deviceSize > MAX_FILESYSTEM_SIZE) {
		printf("ERROR: Disk too big\n");
		return -1;
	}

    int i;
	int fd;
	fd = open(DEVICE_IMAGE, O_RDWR);

	if (fd < 0) {
		perror("Error: 'disk.dat' disk file not found");
		return -1;
	}

    unsigned int deviceBlocks = floor(deviceSize/BLOCK_SIZE);
    printf("deviceBlocks = %u\n", deviceBlocks);

    // unsigned int inodeBlocks = (unsigned int)ceil((MAX_FILESYSTEM_OBJECTS_SUPPORTED*sizeof(inode_t))/BLOCK_SIZE);
    unsigned int inodeBlocks = 1;
    printf("Blocks for inodes = %u\n", inodeBlocks);

    // unsigned int inodeMapBlocks = (unsigned int)ceil((MAX_FILESYSTEM_OBJECTS_SUPPORTED*(unsigned int)sizeof(char))/BLOCK_SIZE);
    unsigned int inodeMapBlocks = 1;
    printf("inodeMapBlocks = %u\n", inodeMapBlocks);

    // unsigned int dataMapBlocks = ceil(deviceBlocks*sizeof(char)/BLOCK_SIZE);
    unsigned int dataMapBlocks = 1;
    printf("dataMapBlocks = %u\n", dataMapBlocks);

	unsigned int dataBlockNum = deviceBlocks - 1 - inodeMapBlocks - dataMapBlocks - inodeBlocks;
    if(dataBlockNum < 0) {
        printf("ERROR no data blocks\n");
        return -1;
    } else {
        printf("dataBlockNum = %d\n", dataBlockNum);
    }

	sblock.magicNum 		 = MAGIC_NUMBER;
	sblock.inodeMapNumBlocks = inodeMapBlocks;
	sblock.dataMapNumBlock 	 = dataMapBlocks;
	sblock.numInodes 		 = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
	sblock.firstInode 		 = 1 + inodeMapBlocks + dataMapBlocks;
	sblock.dataBlockNum 	 = dataBlockNum;
	sblock.firstDataBlock 	 = 1 + inodeMapBlocks + dataMapBlocks + inodeBlocks;
	sblock.deviceSize 		 = deviceSize;
	memset(sblock.padding, '0', sizeof(sblock.padding));

	bwrite(DEVICE_IMAGE, 0, (char*)&sblock);

	i_map = (char*)malloc(MAX_FILESYSTEM_OBJECTS_SUPPORTED*sizeof(char));

	for (i = 0; i < sblock.numInodes; i++) {
		printf("Initializing i_map[%d]\n", i);
        i_map[i] = 0;
	}

	b_map = (char*)malloc(dataBlockNum*sizeof(char));

	for (i = 0; i < sblock.dataBlockNum; i++) {
		printf("Initializing b_map[%d]\n", i);
        b_map[i] = 0;
	}

	for (i = 0; i < sblock.numInodes; i++) {
		printf("Initializing inodes[%d]\n", i);
		memset(&(inodes[i]), 0, sizeof(inode_t));
	}

	unmountFS();

	return 0;
}

/*
 * @brief 	Mounts a file system in the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int mountFS(void)
{
	int i;
	/* Read disk block 1 and store it into sblock */
	bread(DEVICE_IMAGE, 0, (char*)&(sblock));

	/* Read from disk inode map */
	for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
		bread(DEVICE_IMAGE, 1+i, ((char *)i_map + i*BLOCK_SIZE));
	}

	/* Read disk block map */
	for (i = 0; i < sblock.dataMapNumBlock; i++) {
		bread(DEVICE_IMAGE, 1+i+sblock.inodeMapNumBlocks, ((char *)b_map + i*BLOCK_SIZE));
	}

	/* Read inodes from disk */
	for (i = 0; i < (sblock.numInodes*sizeof(inode_t)/BLOCK_SIZE); i++) {
		bread(DEVICE_IMAGE, i+sblock.firstInode, ((char *)inodes + i*BLOCK_SIZE));
	}

	return 0;
}

/*
 * @brief 	Unmounts the file system from the simulated device.
 * @return 	0 if success, -1 otherwise.
 */
int unmountFS(void)
{
	int i;
	for (i = 0; i < sblock.numInodes; i++) {
		if(inodes_x[i].opened == 1) {
            fprintf(stderr, "Error in unmountFS, file %s still opened\n", inodes[i].name);
			return -1;
		}
	}

	fssync();
	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	printf("CREATEFILE *******************\n");

    if(fileName == NULL) {return -2;}       /* Error with file name*/

    if(namei(fileName) != -1) {
        fprintf(stderr, "Error in createFile: %s already exists\n", fileName);
        return -1;
    }  /* File name already exists */

    unsigned int b_id, inode_id;

    /* Allocate an inode for the new file */
	inode_id = ialloc();
	printf("inode_id = %u\n", inode_id);
    if(inode_id < 0) {return inode_id;}

    /* Allocate indirect block */
    b_id = alloc();
	printf("b_id = %u\n", b_id);
	if(b_id < 0) {ifree(inode_id); return b_id;}

		/* Initialize name */
    memset(inodes[inode_id].name, '\0', sizeof(FILENAME_MAXLEN));
	strcpy(inodes[inode_id].name, fileName);
	inodes[inode_id].undirectBlock = b_id;

	/* Initialize undirectBlock */
	char buf[BLOCK_SIZE];
	undirectBlock_t newIndBlock;
	memmove(buf, &newIndBlock, sizeof(undirectBlock_t));

	bwrite(DEVICE_IMAGE, b_id, buf);

	/* Initialize size of the file */
	inodes[inode_id].size = 0;

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
	int inode_id;

	inode_id = namei(fileName);
	if (inode_id < 0) return -1;

    unsigned int size = inodes[inode_id].size;
    unsigned int undirectBlock_id = inodes[inode_id].undirectBlock;

    int i = 0;
    char *buf = (char*)malloc(sizeof(undirectBlock_t));
    undirectBlock_t u_block;

    bread(DEVICE_IMAGE, undirectBlock_id, (char*)&u_block);
    //memmove(&u_block, buf, BLOCK_SIZE);

    free(buf);

		if(inodes[inode_id].size != 0) { /* If the size is 0, no data blocks are used */
			do {
	        bfree(u_block.dataBlocks[i]);

	        i++;
	        size -= BLOCK_SIZE;
	    } while(size > BLOCK_SIZE);
		}

	bfree(inodes[inode_id].undirectBlock);
	closeFile(inode_id);
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
		return inode_id;
	}

	inodes_x[inode_id].position = 0;
	inodes_x[inode_id].opened = 1;

	return inode_id;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor)
{
	if (fileDescriptor < 0) {
		return -1;
	}

	inodes_x[fileDescriptor].position = 0;
	inodes_x[fileDescriptor].opened = 0;

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

	// ESTO SOLO TESTING PARA GUILLE
	inodes_x[fileDescriptor].position = 0;
	// OJOOOOOOOOO

	if (inodes_x[fileDescriptor].position+numBytes > inodes[fileDescriptor].size) {
		numBytes = inodes[fileDescriptor].size - inodes_x[fileDescriptor].position;
	}

	if (numBytes <= 0) {
		fprintf(stderr, "Error reading file: Segmentation fault\n");
		return -1;
	}

	unsigned int u_block_id = inodes[fileDescriptor].undirectBlock;
	undirectBlock_t u_block;
	bread(DEVICE_IMAGE, u_block_id, (char*)&u_block);

	unsigned int copiedSoFar = 0;
	unsigned int bytesRemainingOnCurrentBlock = 0;
	bytesRemainingOnCurrentBlock = BLOCK_SIZE - (inodes_x[fileDescriptor].position%BLOCK_SIZE);
	unsigned int i;
	i = bmap(fileDescriptor, inodes_x[fileDescriptor].position);;

	while (numBytes > 0) {
		//b_id = bmap(fileDescriptor, inodes_x[fileDescriptor].position);
		printf("READING B_ID %d\n", i);
		if (bytesRemainingOnCurrentBlock == BLOCK_SIZE) {
			bytesRemainingOnCurrentBlock = 0;
		}
		if (bytesRemainingOnCurrentBlock > 0) {
			bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
			memmove(buffer+copiedSoFar, b+inodes_x[fileDescriptor].position, bytesRemainingOnCurrentBlock);
			inodes_x[fileDescriptor].position += bytesRemainingOnCurrentBlock;
			printf("bytesRemainingOnCurrentBlock = %d\n", bytesRemainingOnCurrentBlock);
			copiedSoFar += bytesRemainingOnCurrentBlock;
			numBytes -= bytesRemainingOnCurrentBlock;
			bytesRemainingOnCurrentBlock = 0;
		} else {
			if (numBytes < BLOCK_SIZE) {
				bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
				memmove(buffer+copiedSoFar, b, numBytes);
				inodes_x[fileDescriptor].position += numBytes;
				copiedSoFar += numBytes;
				numBytes = 0;
			} else {
				bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
				memmove(buffer+copiedSoFar, b, BLOCK_SIZE);
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
		fprintf(stderr, "Error writing file: Segmentation fault\n");
		return -1;
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
	} else{
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
		if (bytesFreeOnCurrentBlock == BLOCK_SIZE) {
			bytesFreeOnCurrentBlock = 0;
		}
		/* Fill current block remaining space first (if any) */
		if (bytesFreeOnCurrentBlock > 0) {
			bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
			memmove(b+inodes_x[fileDescriptor].position, buffer+copiedSoFar, bytesFreeOnCurrentBlock);
			bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], b);
			inodes_x[fileDescriptor].position += bytesFreeOnCurrentBlock;
			copiedSoFar += bytesFreeOnCurrentBlock;
			numBytes -= bytesFreeOnCurrentBlock;
			bytesFreeOnCurrentBlock = 0;
		} else {
			/* Then fill the new allocated blocks (if any) */
			if (numBytes < BLOCK_SIZE) {
				/* Last operation if enters here */
				memmove(b, buffer+copiedSoFar, numBytes);
				inodes_x[fileDescriptor].position += numBytes;
				copiedSoFar += numBytes;
				numBytes = 0;
			} else {
				memmove(b, buffer+copiedSoFar, BLOCK_SIZE);
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
	return copiedSoFar;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if succes, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence)
{
	return -1;
}

/*
 * @brief 	Verifies the integrity of the file system metadata.
 * @return 	0 if the file system is correct, -1 if the file system is corrupted, -2 in case of error.
 */
int checkFS(void)
{
	return -2;
}

/*
 * @brief 	Verifies the integrity of a file.
 * @return 	0 if the file is correct, -1 if the file is corrupted, -2 in case of error.
 */
int checkFile(char *fileName)
{
	return -2;
}


int ialloc(void) {
	int i;
	/* To search for a free inode */
	for (i = 0; i < sblock.numInodes; i++) {
		if (i_map[i] == 0) {
			/* inode busy right now */
			i_map[i] = 1;
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
		if (b_map[i] == 0) {
			/* busy block right now */
			b_map[i] = 1;
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
	i_map[inode_id] = 0;

	return 0;
}

int bfree(int block_id) {
	/* to check the inode_id vality */
	if (block_id > sblock.dataBlockNum) {
		return -1;
	}

	/* free data block */
	b_map[block_id] = 0;

	return 0;
}

int namei(char *fname) {
	int i;
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
	//  	return inodes[inode_id].directBlock;
	// }

    int block = 0;
    while(offset > BLOCK_SIZE) {
        offset -= BLOCK_SIZE;
        block++;
    }


	return block;
}


int fssync(void) {
	int i;
	/* Write block 1 from sblock into disk */
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
		bwrite(DEVICE_IMAGE, i+sblock.firstInode, ((char *)inodes + i*BLOCK_SIZE));
	}

	return 0;
}
