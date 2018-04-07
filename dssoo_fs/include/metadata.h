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

#define FILENAME_MAXLEN 255
#define RESERVED_INODES 3

const int ROOTDIR_INODE_NUMBER = 1;
const int SUPERBLOCK_BLOCK_NUMBER = 0;
const int INODESTORE_BLOCK_NUMBER = 1;

/* Journal settings */
const int JOURNAL_INODE_NUMBER = 2;
const int JOURNAL_BLOCK_NUMBER = 2;
const int JOURNAL_BLOCKS = 2;

const int ROOTDIR_DATABLOCK_NUMBER = 4;

#define LAST_RESERVED_BLOCK ROOTDIR_DATABLOCK_NUMBER
#define LAST_RESERVED_INODE JOURNAL_INODE_NUMBER

struct dir_record {
  char filename[FILENAME_MAXLEN];
  unint64_t inode_no;
};

struct inode {
  mode_t mode;
  unint64_t inode_no;
  unint64_t data_block_number;

  union {
    uint64_t file_size;
    uint64_t dir_children_count;
  };
};

const int MAX_FILESYSTEM_OBJECTS_SUPPORTED = 64;


struct journal_s;
struct super_block {
  uint64_t version;
  uint64_t magic;
  uint64_t block_size;
  // uint64_t inodes_count;
  uint64_t free_blocks;
  // struct journal_s *journal;
  char padding[4048];
};
