/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file  auxiliary.h
 * @brief   Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

#define WELCOMEFILE_DATABLOCK_NUMBER (LAST_RESERVED_BLOCK + 1)
#define WELCOMEFILE_INODE_NUMBER (LAST_RESERVED_INODE + 1)

static inline struct super_block *SB(struct super_block *sb) {
        return sb->s_fs_info;
}

static inline struct inode *INODE(struct inode *inode) {
        return inode->i_private;
}

struct inode *get_inode(struct super_block *sb, const struct inode *dir,
                        umode_t mode, dev_t dev) {
        struct inode *inode = new_inode(sb);

        if(inode) {
                inode->inode_no = get_next_ino();
                inode_init_owner(inode, dir, mode);
                inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;

                switch (mode & S_IFMT) {
                case S_IFDIR:
                        inc_nlink(inode);
                        break;
                case S_IFREG:
                case S_IFLNK:
                default:
                        printk(KERN_ERR "error\n");
                        return NULL;
                        break;
                }
        }
        return inode;
}

static int write_superblock(int fd) {
        struct super_block sb = {
                .version = 1,
                .magic = MAGIC;
                .block_size = BLOCK_SIZE;
                .inodes_count = WELCOMEFILE_INODE_NUMBER,
                .free_blocks = (~0) & ~(1 << LAST_RESERVED_BLOCK);
        }
        ssize_t ret;

        ret = write(fd, &sb, sizeof(sb));
        if (ret != BLOCK_SIZE) {
                printf("bytes written [%d] are not equal to the default block size\n", (int)ret);
                return -1;
        }

        printf("Super block written successfully\n");
        return 0;
}

static int write_root_inode(int fd) {
        ssize_t ret;

        struct inode root_inode;

        root_inode.mode = S_IFDIR;
        root_inode.inode_no = ROOTDIR_INODE_NUMBER;
        root_inode.data_block_number = ROOTDIR_DATABLOCK_NUMBER;
        root_inode.dir_children_count = 1;

        ret = write(fd, &root_inode, sizeof(root_inode));

        if (ret != sizeof(root_inode)) {
                printf("The inode store was not written properly. Retry your mkfs\n");
                return -1;
        }

        printf("root directory inode written successfully\n");

        return 0;
}

static int write_journal_inode(int fd) {
        ssize_t ret;
        struct inode journal;

        journal.inode_no = JOURNAL_INODE_NUMBER;
        journal.data_block_number = JOURNAL_BLOCK_NUMBER;

        ret = write(fd, &journal, sizeof(journal));

        if (ret != sizeof(journal)) {
                printf("Error while writing journal inode. Retry your mkfs\n");
                return -1;
        }

        printf("journal inode written successfully\n");
        return 0;
}

static int write_welcome_inode(int fd, const struct inode *i) {
        off_t nbytes;
        ssize_t ret;

        ret = write(fd, i, sizeof(*i));
        if (ret != sizeof(*i)) {
                printf("The welcomefile inode was not written properly. Retry your mkfs\n");
                return -1;
        }
        printf("welcome inode written successfully\n");

        nbytes = BLOCK_SIZE - (sizeof(*i) * 3);
        ret = lseek(fd, nbytes, SEEK_CUR);
        if (ret == (off_t)-1) {
                printf("The padding bytes are not written properly. Retry your mkfs\n");
                return -1;
        }
        printf("inode store padding bytes (after the three inodes) written sucessfully\n");
        return 0;
}

int write_journal(int fd) {
        ssize_t ret;
        ret = lseek(fd, BLOCK_SIZE*JOURNAL_BLOCKS, SEEK_CUR);
        if (ret == (off_t) -1) {
                printf("Can't write journal. Retry mkfs\n");
        }
}

int write_dirent(int fd, const struct dir_record *record) {
        ssize_t nbytes = sizeof(*record), ret;

        ret = write(fd, record, nbytes);
        if (ret != nbytes) {
                printf("Writing the rootdirectory datablock (name+inode_no pair for welcome) has failed\n");
                return -1;
        }
        printf("padding after the rootdirectory children written successfully\n");
        return 0;
}

int write_block(int fd, char *block, size_t len) {
        ssize_t ret;
        ret = write(fd, block, len);
        if (ret != len) {
                printf("Writing file body has failed\n");
                return -1;
        }
        printf("block has been written successfully\n");
        return 0;
}

int main(int argc, char const *argv[]) {
        int fd;
        ssize_t ret;

        char welcomefile_body[] = "Esto es para testear cosas.\n";
        struct inode welcome = {
                .mode = S_IFREG,
                .inode_no = WELCOMEFILE_INODE_NUMBER,
                .data_block_number = WELCOMEFILE_DATABLOCK_NUMBER,
                .file_size = sizeof(welcomefile_body),
        };
        struct dir_record record = {
                .filename = "vakanno",
                .inode_no = WELCOMEFILE_INODE_NUMBER,
        };

        if (argc != 2) {
                printf("Usage: mkfs-simplefs <device>\n");
                return -1;
        }

        fd = open(argv[1], O_RDWR);
        if (fd == -1) {
                perror("Error opening the device");
                return -1;
        }

        ret = 1;

        do {
                if (write_superblock(fd)) {
                        break;
                }
                if (write_root_inode(fd)) {
                        break;
                }
                if (write_journal_inode(fd)) {
                        break;
                }
                if (write_welcome_inode(fd, &welcome)) {
                        break;
                }
                if (write_journal(fd)) {
                        break;
                }
                if (write_dirent(fd, &record)) {
                        break;
                }
                if (write_block(fd, welcomefile_body, welcome.file_size)) {
                        break;
                }
                ret = 0;

        } while(0);

        close(fd);
        return ret;
}
