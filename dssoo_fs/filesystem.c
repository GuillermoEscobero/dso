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
int mkFS(long deviceSize) {
    int i; /* Auxiliary counter */
    int fd; /* File descriptor for DEVICE_IMAGE */

    /* Open disk image for read and write */
    fd = open(DEVICE_IMAGE, O_RDWR);
    if (fd < 0) {
        perror("Error while opening 'disk.dat'");
        return -1;
    }
    unsigned long size = (unsigned long) lseek(fd, 0, SEEK_END);
    close(fd);

    /* Ckeck if device image has correct size */
    if (deviceSize < MIN_FILESYSTEM_SIZE) {
        fprintf(stderr, "ERROR: Disk too small\n");
        return -1;
    }
    if (deviceSize > MAX_FILESYSTEM_SIZE) {
        fprintf(stderr, "ERROR: Disk too big\n");
        return -1;
    }

    if (size < deviceSize) {
        fprintf(stderr, "Error in mkFS: Disk too small\n");
        return -1;
    }

    /* Calculate the needed blocks for metadata following the design
     * and the device size */

    unsigned int deviceBlocks = deviceSize / BLOCK_SIZE; /* Floor function applied when casting to int */
    unsigned int inodeMapBlocks = 1;
    unsigned int dataMapBlocks = deviceBlocks / BLOCK_SIZE;

    if ((dataMapBlocks % 8) != 0 || dataMapBlocks == 0) {
        dataMapBlocks = dataMapBlocks / 8;
        dataMapBlocks++;
    }

    unsigned int inodeBlocks = 1;

    unsigned int dataBlockNum = deviceBlocks - 1 - inodeMapBlocks - dataMapBlocks - inodeBlocks;

    if (dataBlockNum < 0) {
        fprintf(stderr, "Error: no data blocks available. Try to make a larger disk image\n");
        return -1;
    }

    /* Initialize superblock */
    sblock.magicNum = MAGIC_NUMBER;
    sblock.inodeMapNumBlocks = inodeMapBlocks;
    sblock.dataMapNumBlock = dataMapBlocks;
    sblock.numInodes = MAX_FILESYSTEM_OBJECTS_SUPPORTED;
    sblock.firstInodeBlock = 1 + inodeMapBlocks + dataMapBlocks;
    sblock.dataBlockNum = dataBlockNum;
    sblock.firstDataBlock = 1 + inodeMapBlocks + dataMapBlocks + inodeBlocks;
    sblock.deviceSize = deviceSize;
    memset(sblock.padding, '0', sizeof(sblock.padding));

    /* Write superblock to disk image */
    bwrite(DEVICE_IMAGE, 0, (char *) &sblock);

    /* Allocate space in memory for inodes map and initialize its elements to 0 */
    i_map = (char *) malloc(MAX_FILESYSTEM_OBJECTS_SUPPORTED / 8); /* For allocating bits */
    for (i = 0; i < sblock.numInodes; i++) {
        // printf("Initializing newImap[%d]\n", i);
        bitmap_setbit(i_map, i, 0);
    }

    /* Allocate space in memory for data blocks map and initialize its elements to 0 */
    b_map = (char *) malloc((dataBlockNum / 8) + 1);
    for (i = 0; i < sblock.dataBlockNum; i++) {
        // printf("Initializing newBmap[%d]\n", i);
        bitmap_setbit(b_map, i, 0);
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
int mountFS(void) {
    int i;

    /* Read superblock (disk block 0) and store it into sblock */
    if (bread(DEVICE_IMAGE, 0, (char *) &sblock) < 0) {
        fprintf(stderr, "Error in mountFS: superblock cannot be read\n");
        return -1;
    }

    i_map = (char *) malloc(BLOCK_SIZE); /* For allocating bits */
    /* Read from disk inode map */
    for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
        if (bread(DEVICE_IMAGE, 1 + i, ((char *) i_map + i * BLOCK_SIZE)) < 0) {
            fprintf(stderr, "Error in mountFS: can't read inodes map\n");
            return -1;
        }
    }

    b_map = (char *) malloc(sblock.dataMapNumBlock * BLOCK_SIZE);
    /* Read disk block map */
    for (i = 0; i < sblock.dataMapNumBlock; i++) {
        if (bread(DEVICE_IMAGE, 1 + i + sblock.inodeMapNumBlocks, ((char *) b_map + (i * BLOCK_SIZE / 8))) < 0) {
            fprintf(stderr, "Error in mountFS: can't read data block map\n");
            return -1;
        }
    }

    /* Read inodes from disk */
    for (i = 0; i < (sblock.numInodes * sizeof(inode_t) / BLOCK_SIZE); i++) {
        if (bread(DEVICE_IMAGE, i + sblock.firstInodeBlock, ((char *) inodes + i * BLOCK_SIZE)) < 0) {
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
int unmountFS(void) {
    unsigned int i;

    /* Check if any file still opened */
    for (i = 0; i < sblock.numInodes; i++) {
        if (inodes_x[i].opened == 1) {
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

    return 0;
}

/*
 * @brief	Creates a new file, provided it it doesn't exist in the file system.
 * @return	0 if success, -1 if the file already exists, -2 in case of error.
 */
int createFile(char *fileName) {
    int b_id, inode_id;
    char buf[BLOCK_SIZE];

    if (fileName == NULL) { return -2; }   /* Error with file name*/

    if (strlen(fileName) > FILENAME_MAXLEN) {
        fprintf(stderr, "Error in createFile: file name too long\n");
        return -2;
    }

    if (namei(fileName) != -1) {
        fprintf(stderr, "Error in createFile: file %s already exists\n", fileName);
        return -1;
    } /* File name already exists */

    /* Allocate an inode for the new file */
    inode_id = ialloc();
    if (inode_id < 0) {
        fprintf(stderr, "Error in createFile: maximum number of files in the disk\n");
        return -2;
    }

    /* Allocate indirect block */
    b_id = alloc();
    if (b_id < 0) {
        ifree(inode_id); /* Free created inode */
        fprintf(stderr, "Error in createFile: disk is full\n");
        return -2;
    }

    /* INITIALIZATION OF INODE */
    /* Initialize name */
    memset(inodes[inode_id].name, '\0', FILENAME_MAXLEN);
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
int removeFile(char *fileName) {
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

    /* Get the associated indirect block of the file to delete its data blocks */
    undirectBlock_t u_block;
    if (bread(DEVICE_IMAGE, undirectBlock_id, (char *) &u_block) < 0) {
        fprintf(stderr, "Error in removeFile: can't read undirectBlock\n");
        return -2;
    }

    i = 0;
    memset(&buf, 0, BLOCK_SIZE);   /* Wipe buffer */
    if (inodes[inode_id].size != 0) { /* If the size is 0, no data blocks are used */
        while (size > 0) {
            /* Wipe data setting bytes to 0 */
            if (bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], buf) < 0) {
                fprintf(stderr, "Warning: block %u not properly deleted\n", u_block.dataBlocks[i]);
            }
            bfree(u_block.dataBlocks[i]); /* Set free in block map */
            i++;
            size -= BLOCK_SIZE;
        }
    }

    /* Free indirect block */
    bfree(inodes[inode_id].undirectBlock);

    /* Close file to avoid problems with unmount */
    closeFile(inode_id);

    /* Delete and free inode */
    memset(&(inodes[inode_id]), 0, sizeof(inode_t));
    ifree(inode_id);

    inodes_x[inode_id].position = 0;
    inodes_x[inode_id].opened = 0;

    return 0;
}

/*
 * @brief	Opens an existing file.
 * @return	The file descriptor if possible, -1 if file does not exist, -2 in case of error..
 */
int openFile(char *fileName) {
    int inode_id;

    /* Search for the inode of the file */
    inode_id = namei(fileName);
    if (inode_id < 0) {
        fprintf(stderr, "Error in openFile: file %s not found\n", fileName);
        return -1;
    }

    inodes_x[inode_id].opened = 0;  /* Set file state to close for checkFile */

    if (checkFile(fileName) != 0) {
        return -2;
    }

    inodes_x[inode_id].position = 0; /* Set seek descriptor to begin */
    inodes_x[inode_id].opened = 1;  /* Set file state to open */

    return inode_id;
}

/*
 * @brief	Closes a file.
 * @return	0 if success, -1 otherwise.
 */
int closeFile(int fileDescriptor) {
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
int readFile(int fileDescriptor, void *buffer, int numBytes) {
    unsigned int i;
    char b[BLOCK_SIZE]; /* Auxiliary buffer for block operations */
    unsigned int u_block_id; /* ID of the indirect block of the file */
    int copiedSoFar = 0; /* Bytes already read */
    int bytesRemainingOnCurrentBlock = 0; /* Bytes between the seek descriptor and the next block */

    if (inodes_x[fileDescriptor].opened == 0) {
      fprintf(stderr, "Error in readFile: file not opened\n");
      return -1;
    }

    /* Check the remaining bytes that can be read. If there are not
     * enough, numBytes will be updated */
    if (inodes_x[fileDescriptor].position + numBytes > inodes[fileDescriptor].size) {
        numBytes = inodes[fileDescriptor].size - inodes_x[fileDescriptor].position;
    }

    /* If for some reason, the seek pointer is ahead the last byte
     * of the file, return -1 */
    if (numBytes < 0) {
        fprintf(stderr, "Error reading file: Segmentation fault\n");
        return -1;
    }

    /* In this case, the seek pointer is located at EOF, so no bytes
     * can be read */
    if (numBytes == 0) {
        return 0;
    }

    /* Get the indirect block of the file to know where are its data blocks */
    u_block_id = inodes[fileDescriptor].undirectBlock;
    undirectBlock_t u_block;
    bread(DEVICE_IMAGE, u_block_id, (char *) &u_block);

    /* Calculate the bytes between the seek descriptor and the next block */
    bytesRemainingOnCurrentBlock = BLOCK_SIZE - (inodes_x[fileDescriptor].position % BLOCK_SIZE);
    if (bytesRemainingOnCurrentBlock == BLOCK_SIZE) {
        bytesRemainingOnCurrentBlock = 0;
    }

    /* We only want to read a maximum of numBytes */
    if (bytesRemainingOnCurrentBlock > numBytes) {
        bytesRemainingOnCurrentBlock = numBytes;
    }

    /* Get the block index where the seek descriptor is */
    i = bmap(fileDescriptor, inodes_x[fileDescriptor].position);

    /* First we read the remaining bytes of the actual block */
    if (bytesRemainingOnCurrentBlock > 0) {
        /* Read the actual block */
        bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        /* Read the remaining bytes in the block */
        memcpy(buffer + copiedSoFar, b + inodes_x[fileDescriptor].position, bytesRemainingOnCurrentBlock);
        /* Increment the seek descriptor */
        inodes_x[fileDescriptor].position += bytesRemainingOnCurrentBlock;
        /* Increment bytes read */
        copiedSoFar += bytesRemainingOnCurrentBlock;
        /* Subtract to bytes left to be read */
        numBytes -= bytesRemainingOnCurrentBlock;
    }

    /* Now we start to read blocks until all bytes are read */
    while (numBytes > 0) {
        bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        /* If the remaining bytes to read are less than a block, read all of them */
        if (numBytes < BLOCK_SIZE) {
            memcpy(buffer + copiedSoFar, b, numBytes);
            inodes_x[fileDescriptor].position += numBytes;
            copiedSoFar += numBytes;
            numBytes = 0;
        } else {
            bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
            memcpy(buffer + copiedSoFar, b, BLOCK_SIZE);
            inodes_x[fileDescriptor].position += BLOCK_SIZE;
            copiedSoFar += BLOCK_SIZE;
            numBytes -= BLOCK_SIZE;
        }
        i++;
    }

    return copiedSoFar;
}

/*
 * @brief	Writes a number of bytes from a buffer and into a file.
 * @return	Number of bytes properly written, -1 in case of error.
 */
int writeFile(int fileDescriptor, void *buffer, int numBytes) {
    unsigned int i = 0;
    char b[BLOCK_SIZE]; /* Auxiliary buffer for block operations */
    unsigned int u_block_id; /* ID of the indirect block of the file */
    int copiedSoFar = 0; /* Bytes already read */
    int bytesFreeOnCurrentBlock = 0; /* Bytes between the seek descriptor and the next block */

    unsigned int blocksAlreadyUsed = 0;
    unsigned int blocksToAllocate = 0;

    if (inodes_x[fileDescriptor].opened == 0) {
      fprintf(stderr, "Error in writeFile: file not opened\n");
      return -1;
    }

    /* Check the remaining bytes that can be read. If there are not
     * enough, numBytes will be updated */
    if (inodes_x[fileDescriptor].position + numBytes > MAX_FILE_SIZE) {
        numBytes = MAX_FILE_SIZE - inodes_x[fileDescriptor].position;
        printf("Warning: Only %d bytes will be written\n", numBytes);
    }

    /* If for any reason, the seek pointer is ahead the last byte
     * of the file, return -1 */
    if (numBytes < 0) {
        fprintf(stderr, "Error writing file: Segmentation fault\n");
        return -1;
    }

    /* In this case, the seek pointer is located at EOF, so no bytes
     * can be written */
    if (numBytes == 0) {
        printf("Warning: no bytes have been written\n");
        return 0;
    }

    /* Get the indirect block of the file to know where are its data blocks */
    u_block_id = inodes[fileDescriptor].undirectBlock;
    undirectBlock_t u_block;
    bread(DEVICE_IMAGE, u_block_id, (char *) &u_block);

    /* Calculates the blocks already used for the file */
    if (inodes[fileDescriptor].size == 0) {
        blocksAlreadyUsed = 0; /* Empty file */
    } else {
        /* Ceiling function for positive integer numbers */
        blocksAlreadyUsed = (inodes[fileDescriptor].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    }

    /* Calculate the bytes between the seek descriptor and the next block */
    bytesFreeOnCurrentBlock = BLOCK_SIZE - (inodes_x[fileDescriptor].position % BLOCK_SIZE);
    if (bytesFreeOnCurrentBlock == BLOCK_SIZE) {
        bytesFreeOnCurrentBlock = 0;
    }

    /* We only want to write a maximum of numBytes */
    if (bytesFreeOnCurrentBlock > numBytes) {
        bytesFreeOnCurrentBlock = numBytes;
    }

    /* Do we need to allocate new blocks for writing? */
    if (inodes_x[fileDescriptor].position + numBytes > inodes[fileDescriptor].size) {
        /* Yes, we do */
        /* Ceiling function for integer numbers division */
        blocksToAllocate = (numBytes - bytesFreeOnCurrentBlock) / BLOCK_SIZE;
        if (((numBytes - bytesFreeOnCurrentBlock) % BLOCK_SIZE) != 0) {
            blocksToAllocate++;
        }
    } else {
        /* The user wants to overwrite data */
    }

    /* Allocate the new needed data blocks */
    for (i = 0; i < blocksToAllocate; i++) {
        int newBlockID = alloc();
        if (newBlockID < 0) {
            fprintf(stderr, "Error: no free space in disk to write. No bytes written\n");
            return -1;
        }
        u_block.dataBlocks[blocksAlreadyUsed + i] = newBlockID;
    }

    /* Get the block index where the seek descriptor is */
    i = bmap(fileDescriptor, inodes_x[fileDescriptor].position);

    memset(b, 0, BLOCK_SIZE);

    /* Fill current block remaining space first (if any) */
    if (bytesFreeOnCurrentBlock > 0) {
        /* Read the actual block */
        bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        /* Write the free bytes in the block */
        memcpy(b + inodes_x[fileDescriptor].position, buffer + copiedSoFar, bytesFreeOnCurrentBlock);
        bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        /* Increment the seek descriptor */
        inodes_x[fileDescriptor].position += bytesFreeOnCurrentBlock;
        /* Increment bytes read */
        copiedSoFar += bytesFreeOnCurrentBlock;
        /* Subtract to bytes left to be read */
        numBytes -= bytesFreeOnCurrentBlock;
    }

    /* Now we start to read blocks until all bytes are written */
    while (numBytes > 0) {
        /* Then fill the new allocated blocks (if any) */
        if (numBytes < BLOCK_SIZE) {
            /* Last operation if enters here */
            memcpy(b, buffer + copiedSoFar, numBytes);
            inodes_x[fileDescriptor].position += numBytes;
            copiedSoFar += numBytes;
            numBytes = 0;
        } else {
            memcpy(b, buffer + copiedSoFar, BLOCK_SIZE);
            inodes_x[fileDescriptor].position += BLOCK_SIZE;
            copiedSoFar += BLOCK_SIZE;
            numBytes -= BLOCK_SIZE;
        }
        bwrite(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        i++;
    }

    /* Write the updated indirect block */
    bwrite(DEVICE_IMAGE, u_block_id, (char *) &u_block);
    /* Increase the size of the file */
    inodes[fileDescriptor].size += copiedSoFar;

    /* Calculate the new checksum of the file */
    unsigned char buf[BLOCK_SIZE];
    inodes[fileDescriptor].checksum = 0;

    blocksAlreadyUsed = (inodes[fileDescriptor].size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    for (i = 0; i < blocksAlreadyUsed; i++) {
        bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        memcpy(buf, b, BLOCK_SIZE);
        inodes[fileDescriptor].checksum = CRC16(buf, BLOCK_SIZE, inodes[fileDescriptor].checksum);
    }

    return copiedSoFar;
}

/*
 * @brief	Modifies the position of the seek pointer of a file.
 * @return	0 if success, -1 otherwise.
 */
int lseekFile(int fileDescriptor, long offset, int whence) {
    /* to check the inode_id vality */
    if (fileDescriptor > sblock.numInodes) {
        return -1;
    }

    switch (whence) {
        case FS_SEEK_CUR:
            if ((inodes_x[fileDescriptor].position + offset >= inodes[fileDescriptor].size)
                || (inodes_x[fileDescriptor].position + offset < 0)) {
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
int checkFile(char *fileName) {
    int i;
    int fd;
    char b[BLOCK_SIZE];
    unsigned char buf[BLOCK_SIZE]; /* Unsigned chars needed in CRC function */

    /* Search for the inode */
    fd = namei(fileName);
    if (fd < 0) {
        fprintf(stderr, "Error in checkFile: No such file or directory");
        return -2;
    }

    if (inodes_x[fd].opened == 1) {
        fprintf(stderr, "Error in checkFile: %s is open\n", fileName);
        return -2;
    }

		/* 'Nonce' for CRC calculation */
    uint16_t now = 0;

		/* Calculate number of blocks used by the file */
    unsigned int blocksAlreadyUsed = (inodes[fd].size + BLOCK_SIZE - 1) / BLOCK_SIZE;

		/* Retrieve indirect block of the file */
    unsigned int u_block_id = inodes[fd].undirectBlock;
    undirectBlock_t u_block;
    bread(DEVICE_IMAGE, u_block_id, (char *) &u_block);

		/* Calculate CRC */
    for (i = 0; i < blocksAlreadyUsed; i++) {
        bread(DEVICE_IMAGE, u_block.dataBlocks[i], b);
        memmove(buf, b, BLOCK_SIZE);
        now = CRC16(buf, BLOCK_SIZE, now);
    }

    if (inodes[fd].checksum == now) {
        /* File not corrupted */
        return 0;
    } else {
        /* File corrupted */
        return -1;
    }

    return -1;
}

/*
 * @brief   Search for a free inode and set its value in inodes map to 1
 * @return  ID of the inode available if any, -1 if not
 */
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
    fprintf(stderr, "Error: maximum number of files reached\n");
    return -1;
}

/*
 * @brief   Search for a free data block, set its bytes to 0 and set its
 *          value in data blocks map to 1
 * @return  ID of the data block available if any, -1 if not
 */
int alloc(void) {
    int i;
    char buffer[BLOCK_SIZE];

    for (i = 0; i < sblock.dataBlockNum; i++) {
        if (bitmap_getbit(b_map, (i + sblock.firstDataBlock)) == 0) {
            /* busy block right now */
            bitmap_setbit(b_map, i + sblock.firstDataBlock, 1);

            /* default values for the block */
            memset(buffer, 0, BLOCK_SIZE);
            bwrite(DEVICE_IMAGE, i + sblock.firstDataBlock, buffer);
            /* it returns the block id */
            return i + sblock.firstDataBlock;
        }
    }
    return -1;
}

/*
 * @brief   Sets the value of the inode provided to 0 in inodes map
 * @return  0 if success, -1 if inode not found
 */
int ifree(int inode_id) {
    /* to check the inode_id vality */
    if (inode_id > sblock.numInodes) {
        return -1;
    }

    /* free inode */
    bitmap_setbit(i_map, inode_id, 0);

    return 0;
}

/*
 * @brief   Sets the value of the data block provided to 0 in data block map
 * @return  0 if success, -1 if data block not found
 */
int bfree(int block_id) {
    /* to check the inode_id vality */
    if (block_id > sblock.dataBlockNum) {
        return -1;
    }

    /* free data block */
    bitmap_setbit(b_map, block_id, 0);

    return 0;
}

/*
 * @brief   Found the inode ID containing the file passed
 * @return  ID of the inode, -1 if not found
 */
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

/*
 * @brief   Return the index of the data block that contains the byte indicated
 *          by the offset
 * @return  Index of the block, -1 if not found
 */
int bmap(int inode_id, int offset) {
    /* check for if it is a valid inode ID */
    if (inode_id > sblock.numInodes) {
        return -1;
    }

    unsigned int block = 0;
    while (offset >= BLOCK_SIZE) {
        offset -= BLOCK_SIZE;
        block++;
    }

    return block;
}

/*
 * @brief   Writes the metadata in memory to the disk image
 * @return  0 if success, -1 if error
 */
int fssync(void) {
    int i;
    /* Write super into disk */
    bwrite(DEVICE_IMAGE, 0, (char *) &sblock);

    /* Write inode map to disk */
    for (i = 0; i < sblock.inodeMapNumBlocks; i++) {
        bwrite(DEVICE_IMAGE, 1 + i, ((char *) i_map + i * BLOCK_SIZE));
    }

    /* Write block map to disk */
    for (i = 0; i < sblock.dataMapNumBlock; i++) {
        bwrite(DEVICE_IMAGE, 1 + i + sblock.inodeMapNumBlocks, ((char *) b_map + i * BLOCK_SIZE));
    }

    /* Write inodes to disk */
    for (i = 0; i < (sblock.numInodes * sizeof(inode_t) / BLOCK_SIZE); i++) {
        bwrite(DEVICE_IMAGE, i + sblock.firstInodeBlock, ((char *) inodes + i * BLOCK_SIZE));
    }

    return 0;
}
