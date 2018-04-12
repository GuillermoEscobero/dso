/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	metadata.h
 * @brief 	Definition of the structures and data types of the file system.
 * @date	01/03/2017
 */

#define bitmap_getbit(bitmap_, i_) (bitmap_[i_ >> 3] & (1 << (i_ & 0x07)))
static inline void bitmap_setbit(char *bitmap_, int i_, int val_) {
  if (val_)
    bitmap_[(i_ >> 3)] |= (1 << (i_ & 0x07));
  else
    bitmap_[(i_ >> 3)] &= ~(1 << (i_ & 0x07));
}

/* End of the initial code */

// #define RESERVED_INODES 3
//
// const int ROOTDIR_INODE_NUMBER = 1;
// const int SUPERBLOCK_BLOCK_NUMBER = 0;
// const int INODESTORE_BLOCK_NUMBER = 1;
//
// /* Journal settings */
// const int JOURNAL_INODE_NUMBER = 2;
// const int JOURNAL_BLOCK_NUMBER = 2;
// const int JOURNAL_BLOCKS = 2;
//
// const int ROOTDIR_DATABLOCK_NUMBER = 4;
//
// #define LAST_RESERVED_BLOCK ROOTDIR_DATABLOCK_NUMBER
// #define LAST_RESERVED_INODE JOURNAL_INODE_NUMBER
//
// struct dir_record {
//   char filename[FILENAME_MAXLEN];
//   unint64_t inode_no;
// };
//
// struct inode {
//   mode_t mode;
//   unint64_t inode_no;
//   unint64_t data_block_number;
//
//   union {
//     uint64_t file_size;
//     uint64_t dir_children_count;
//   };
// };
//
// const int MAX_FILESYSTEM_OBJECTS_SUPPORTED = 64;
//


#define MAGIC_NUMBER 0x000D5500
#define DISK_NAME "disk.dat"
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
  unsigned int directBlocks[12];
  unsigned int undirectBlock;
  unsigned int size;
  //char padding[BLOCK_SIZE-844];
} inode_t;

superblock_t sblock;
char *i_map;   /* numInodes */
char *b_map;   /* dataBlockNum */
inode_t inodes[MAX_FILESYSTEM_OBJECTS_SUPPORTED];

struct {
  int position;
  int opened;
} inodes_x[MAX_FILESYSTEM_OBJECTS_SUPPORTED];

