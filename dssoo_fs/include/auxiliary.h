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
