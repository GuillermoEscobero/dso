/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
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

typedef struct {
  unsigned int magicNum;              /* Magic number of the superblock: 0x000D5500 */
  unsigned int inodeMapNumBlocks;     /* Number of blocks of the i-node map */
  unsigned int dataMapNumBlock;       /* Number of blocks of the data map */
  unsigned int numInodes;             /* Number of i-nodes in the device */
  unsigned int firstInode;            /* Number of the first i-node in the device (root inode) */
  unsigned int dataBlockNum;          /* Number of the data blocks in the device */
  unsigned int firstDataBlock;        /* Number of the first data block */
  unsigned int deviceSize;            /* Total disk space (in bytes) */
  char padding[BLOCK_SIZE-32];        /* Padding field (to complete a block) */
} superblock_t;

typedef struct {
  char name[FILENAME_MAXLEN];
  unsigned int undirectBlock;
  unsigned int size;
  uint16_t checksum;
} inode_t;

typedef struct {
    unsigned int dataBlocks[512];
} undirectBlock_t;

superblock_t sblock;
char *i_map;   /* numInodes */
char *b_map;   /* dataBlockNum */
inode_t inodes[MAX_FILESYSTEM_OBJECTS_SUPPORTED];

struct {
  int position;
  int opened;
} inodes_x[MAX_FILESYSTEM_OBJECTS_SUPPORTED];
