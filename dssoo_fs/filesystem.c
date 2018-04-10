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

	int fd;
	fd = open(DISK_NAME, O_RDWR);

	if (fd < 0) {
		perror("Error: 'disk.dat' disk file not found");
		return -1;
	}

	long dataBlockNum = (deviceSize-sizeof(superblock_t)
		-(sizeof(char)*MAX_FILESYSTEM_OBJECTS_SUPPORTED*2)
		-(sizeof(inode_t)*MAX_FILESYSTEM_OBJECTS_SUPPORTED))/BLOCK_SIZE;

	long inodeBlocks = (MAX_FILESYSTEM_OBJECTS_SUPPORTED*sizeof(inode_t))/BLOCK_SIZE;

	sblock.magicNum 				 = MAGIC_NUMBER;
	sblock.inodeMapNumBlocks = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
	sblock.dataMapNumBlock 	 = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
	sblock.numInodes 				 = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
	sblock.firstInode 			 = 1 + MAX_FILESYSTEM_OBJECTS_SUPPORTED + (unsigned int)dataBlockNum;
	sblock.dataBlockNum 		 = (unsigned int)dataBlockNum;
	sblock.firstDataBlock 	 = firstInode+MAX_FILESYSTEM_OBJECTS_SUPPORTED;
	sblock.deviceSize 			 = deviceSize;
	memset(sblock.padding, '0', sizeof(sblock->padding));

	bwrite(DISK_NAME, 0, &sblock);

	i_map = (char*)malloc(MAX_FILESYSTEM_OBJECTS_SUPPORTED*sizeof(char));

	for (i = 0; i < sblock.numInodes; i++) {
		i_map[i] = 0;
	}

	b_map = (char*)malloc(dataBlockNum*sizeof(char));

	for (i = 0; i < sblock.dataBlockNum; i++) {
		b_map[i] = 0;
	}

	for (i = 0; i < sblock.numInodes; i++) {
		memset(&(inodes[i]), 0, sizeof(inode_t));
	}

	unmount();

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
	bread(DISK, 1, &(sblock));

	/* Read from disk inode map */
	for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
		bread(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE));
	}

	/* Read disk block map */
	for (i = 0; i < sblock.dataMapNumBlock; i++) {
		bread(DISK, 2+i+sblock.inodeMapNumBlocks, ((char *)b_map + i*BLOCK_SIZE));
	}

	/* Read inodes from disk */
	for (i = 0; i < (sblock.numInodes*sizeof(inode_t)/BLOCK_SIZE); i++) {
		bread(DISK, i+sblock.firstInode, ((char *)inodes + i*BLOCK_SIZE));
	}

	return 1;
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
			return -1;
		}
	}

	sync();
	return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName)
{
	int b_id, inode_id;

	inode_id = ialloc();
	if(inode_id < 0) {return inode_id;}
	b_id = alloc();
	if(b_id < 0) {ifree(inode_id); return b_id;}

	inodes[inode_id].type = 1; /* FILE */
	strcpy(inodes[inode_id].name, name);
	inodes[inode_id].directBlock = b_id;
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

	inode_id = namei(name);
	if (inode_id < 0) return -1;

	free(inodes[inode_id].directBlock);
	memset(&(inodes[inode_id]), 0, sizeof(diskInodeType));
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

	inode_id = namei(name);
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
	if (fd < 0) {
		return -1;
	}

	inodes_x[fd].position = 0;
	inodes_x[fd].opened = 0;

	return 0;
}

/*
 * @brief	Reads a number of bytes from a file and stores them in a buffer.
 * @return	Number of bytes properly read, -1 in case of error.
 */
int readFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int b_id;

	if (inodes_x[fd].position+size > inodes[fd].size) {
		size = inodes[fd].size - inodes_x[fd].position;
	}

	if (size =< 0) {
		return -1;
	}

	b_id = bmap(fd, inodes_x[fd].position);
	bread(DISK, b_id, b);
	memmove(buffer. b+inodes_x[fd].position, size);
	inodes_x[fd].position += size;

	return size;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes)
{
	char b[BLOCK_SIZE];
	int b_id;

	if (inodes_x[fd].position+size > BLOCK_SIZE) {
		size = BLOCK_SIZE - inodes_x[fd].position;
	}

	if (size =< 0) {
		return -1;
	}

	b_id = bmap(fd, inodes_x[fd].position);
	bread(DISK, b_id, b);
	memmove(b+inodes_x[fd].position, buffer, size);
	bwrite(DISK, b_id, b);
	inodes_x[fd].position += size;

	return size;
	return -1;
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
			memset(b, 0, BLOCK_SIZE);
			bwrite(DISK, i+sblock.firstDataBlock, b);
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

int free(int block_id) {
	/* to check the inode_id vality */
	if (block_id > sblock.dataBlockNum) {
		return -1;
	}

	/* free inode */
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

	/* return the inode block */
	if (offset < BLOCK_SIZE) {
		return inodes[inode_id].directBlock;
	}

	return -1;
}


int sync(void) {
	int i;
	/* Write block 1 from sblock into disk */
	bwrite(DISK, 1, &(sblock));

	/* Write inode map to disk */
	for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
		bwrite(DISK, 2+i, ((char *)i_map + i*BLOCK_SIZE));
	}

	/* Write block map to disk */
	for (i = 0; i < sblock.dataMapNumBlock; i++) {
		bwrite(DISK, 2+i+sblock.inodeMapNumBlocks, ((char *)b_map + i*BLOCK_SIZE));
	}

	/* Write inodes to disk */
	for (i = 0; i < (sblock.numInodes*sizeof(diskInodeType)/BLOCK_SIZE); i++) {
		bwrite(DISK, i+sblock.firstInode, ((char *)inodes + i*BLOCK_SIZE));
	}

	return 1;
}
