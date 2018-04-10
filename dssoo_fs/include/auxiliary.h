/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file  auxiliary.h
 * @brief   Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

#define WELCOMEFILE_DATABLOCK_NUMBER (LAST_RESERVED_BLOCK + 1)
#define WELCOMEFILE_INODE_NUMBER (LAST_RESERVED_INODE + 1)

static inline struct super_block *SB(struct super_block *sb);

static inline struct inode *INODE(struct inode *inode);

struct inode *get_inode(struct super_block *sb, const struct inode *dir, umode_t mode, dev_t dev);

static int write_superblock(int fd);

static int write_root_inode(int fd);

static int write_journal_inode(int fd);

static int write_welcome_inode(int fd, const struct inode *i);

int write_journal(int fd);

int write_dirent(int fd, const struct dir_record *record);

int write_block(int fd, char *block, size_t len);
