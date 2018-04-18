/*
 * OPERATING SYSTEMS DESIGN - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	17/04/2018
 */

#include <stdint.h>

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

#define MAGIC_NUMBER 0x000D5500
#define FILENAME_MAXLEN 32
#define MAX_FILESYSTEM_OBJECTS_SUPPORTED 40
#define MIN_FILESYSTEM_SIZE 51200
#define MAX_FILESYSTEM_SIZE 52428800
#define METADATA_BLOCKS_NUM 4

typedef struct {
  unsigned int magicNum;              /* Magic number of the superblock: 0x000D5500 */
  unsigned int inodeMapNumBlocks;     /* Number of blocks of the i-node map */
  unsigned int dataMapNumBlock;       /* Number of blocks of the data map */
  unsigned int numInodes;             /* Number of i-nodes in the device */
  unsigned int firstInodeBlock;       /* Number of the first i-node in the device (root inode) */
  unsigned int dataBlockNum;          /* Number of the data blocks in the device */
  unsigned int firstDataBlock;        /* Number of the first data block */
  unsigned int deviceSize;            /* Total disk space (in bytes) */
  uint16_t     checksum;              /* CRC16 checksum of the metadata blocks */
  char padding[BLOCK_SIZE-16];        /* Padding field (to complete a block) */
} superblock_t;

typedef struct {
  char name[FILENAME_MAXLEN];         /* File name */
  unsigned int undirectBlock;         /* Indirect block ID containing the IDs of data blocks */
  unsigned int size;                  /* Size of the data contained in the file */
  uint16_t checksum;                  /* CRC16 checksum of data blocks of the file */
} inode_t;

typedef struct {
  unsigned int dataBlocks[512];       /* IDs of the blocks (512*2048 = MAX_FILE_SIZE) */
} undirectBlock_t;


/* Metadata to be used in memory while system is mounted */

superblock_t sblock;                              /* Superblock struct in memory */
char *i_map;                                      /* Map of used iNodes */
char *b_map;                                      /* Map of used dataBlocks */
inode_t inodes[MAX_FILESYSTEM_OBJECTS_SUPPORTED]; /* iNodes array in memory */

struct {
  int position;                                   /* Position of the file seek pointer */
  int opened;                                     /* 0 if file is closed, 1 if opened */
} inodes_x[MAX_FILESYSTEM_OBJECTS_SUPPORTED];
