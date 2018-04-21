/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file 	test.c
 * @brief 	Implementation of the client test routines.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <string.h>
#include "include/filesystem.h"

#include <stdlib.h>
#include <fcntl.h>

// Color definitions for asserts
#define ANSI_COLOR_RESET   "\x1b[0m"
#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_BLUE   "\x1b[34m"

#define N_BLOCKS    25                        // Number of blocks in the device
#define DEV_SIZE    N_BLOCKS * BLOCK_SIZE    // Device size, in bytes

#define TEST_PASSED(test_name...) fprintf(stdout, "%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST ", test_name, ANSI_COLOR_GREEN, " SUCCESS\n\n", ANSI_COLOR_RESET)
#define TEST_FAILED(test_name...) fprintf(stdout, "%s%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST ", test_name, ANSI_COLOR_RED, " FAILED\n\n", ANSI_COLOR_RESET)
#define TEST_PRINT(test_number, requirement_tested...) fprintf(stdout, "%s%s%s%s%s", "TEST NUMBER ", test_number, ": REQUIREMENT ", requirement_tested, "\n")

int main() {

    //FUNCTIONAL REQUIREMENTS TESTS
    fprintf(stdout, "%s", "BASIC TESTING OF FUNCTIONAL REQUIREMENTS\n\n");

    /**
     * @test 1
     * @requirement F1.1
     * @description Create a file system
     */
    TEST_PRINT("1", "F1.1");
    if (mkFS(DEV_SIZE) == 0) {
        TEST_PASSED("mkFS");
    } else {
        TEST_FAILED("mkFS");
    }

    /**
     * @test 2
     * @requirement F1.2
     * @description Mount a file system
     */
    TEST_PRINT("2", "F1.2");
    if (mountFS() == 0) {
        TEST_PASSED("mountFS");
    } else {
        TEST_FAILED("mountFS");
    }

    /**
     * @test 4
     * @requirement F1.4
     * @description Create a file within the file system
     */
    TEST_PRINT("4", "F1.4");
    if (createFile("test.txt") == 0) {
        TEST_PASSED("createFile");
    } else {
        TEST_FAILED("createFile");
    }

    /**
     * @test 6
     * @requirement F1.6
     * @description Open an existing file
     */
    TEST_PRINT("6", "F1.6");
    int fd = openFile("test.txt");
    if (fd != -1 && fd != -2) {
        TEST_PASSED("openFile");
    } else {
        TEST_FAILED("openFile");
    }

    int fd2 = open("quijote.txt", O_RDONLY);
    char write_buffer[3000];
    read(fd2, write_buffer, sizeof(write_buffer));

    /**
     * @test 9
     * @requirement F1.9, F7
     * @description Write to an opened file, A file could be modified by means of write operations.
     */
    TEST_PRINT("9", "F1.9, F7");
    if (writeFile(fd, write_buffer, sizeof(write_buffer) - 64) == 0) {
        lseekFile(fd, 0, FS_SEEK_BEGIN);
        if (writeFile(fd, write_buffer, 64) == 64) {
            TEST_PASSED("writeFile");
        } else {
            TEST_FAILED("writeFile");
        }
    } else {
        TEST_FAILED("writeFile");
    }

    /**
     * @test 8
     * @requirement F1.8
     * @description Read from an opened file
     */
    TEST_PRINT("8", "F1.8");
    char read_buffer[3000];
    //FIXME:
    int debug = readFile(fd, read_buffer, sizeof(read_buffer) - 64);
    printf("Debug %d", debug);
    lseekFile(fd, 0, FS_SEEK_BEGIN);
    debug = readFile(fd, read_buffer, sizeof(read_buffer));
    printf("Debug %d", debug);
    debug = readFile(fd, read_buffer, sizeof(read_buffer));
    printf("Debug %d", debug);

    if (debug == 0) {
        printf("Efectivamente");
        lseekFile(fd, 0, FS_SEEK_BEGIN);
        if (readFile(fd, read_buffer, sizeof(read_buffer)) == 0) {
            TEST_PASSED("readFile");
        } else {
            printf("faileo en este");
            TEST_FAILED("readFile");
        }
    } else {
        TEST_FAILED("readFile");
    }

    /**
     * @test 10
     * @requirement F1.10
     * @description Modify the position of the seek pointer
     */
    TEST_PRINT("10", "F1.10");
    char read_string[6];
    char *init_string = "#INIT#";
    if (lseekFile(fd, 10, FS_SEEK_BEGIN) == 0) {
        readFile(fd, read_string, 6);
        printf("\nread string: %s ini%s\n", read_string, init_string);
        if (strcmp(read_string, init_string) == 0) {
            TEST_PASSED("lseekFile");
        } else {
            TEST_FAILED("lseekFile");
        }
    } else {
        TEST_FAILED("lseekFile");
    }

    /**
     * @test 7
     * @requirement F1.7
     * @description Close an opened file
     */
    TEST_PRINT("7", "F1.7");
    if (closeFile(fd2) == 0) {
        TEST_PASSED("closeFile");
    } else {
        TEST_FAILED("closeFile");
    }

    /**
     * @test 12
     * @requirement F2
     * @description Every time a file is opened, its seek pointer will be reset to the beginning of the file.
     */

    /**
     * @test 11
     * @requirement F1.11
     * @description Check the integrity an existing file
     */
    //TEST_PRINT("11", "F1.11");
    //ret = checkFile("test.txt");
    //if(ret != 0) {
    //	TEST_FAILED("checkFile");
    //	return -1;
    //}
    //TEST_PASSED("checkFile");

    /**
     * @test 13
     * @requirement F3
     * @description Metadata shall be updated after any write operation in order to properly reflect any
     * modification in the file system.
     */

    /**
     * @test 14
     * @requirement F4
     * @description The file system will not implement directories.
     */

    /**
     * @test 15
     * @requirement F5
     * @description File integrity must be checked, at least, on open operations.
     */

    /**
     * @test 16
     * @requirement F6
     * @description The whole contents of a file could be read by means of several read operations.
     */

    /**
     * @test 17
     * @requirement F8
     * @description As part of a write operation, file capacity may be extended by means of additional data blocks.
     */

    /**
     * @test 18
     * @requirement F9
     * @description The file system can be created on partitions of the device smaller than its maximum size.
     */

    /**
     * @test 3
     * @requirement F1.3
     * @description Unmount a file system
     */
    //TEST_PRINT("3", "F1.3");
    //if(unmountFS() == 0) {
    //	TEST_PASSED("unmountFS");
    //} else {
    //	TEST_FAILED("unmountFS");
    //}

    /**
     * @test 5
     * @requirement F1.5
     * @description Remove an existing file from the file system
     */
    //TEST_PRINT("5", "F1.5");
    //if(removeFile("test.txt") == 0){
    //	TEST_PASSED("removeFile");
    //} else {
    //	TEST_FAILED("removeFile");
    //}

    //NON-FUNCTIONAL REQUIREMENTS TESTS

    //TODO:
    //printf("BASIC TESTING OF NON-FUNCTIONAL REQUIREMENTS\n");

    //printf("TEST NUMBER %d: NON FUNCTIONAL REQUIREMENT 1\n", i);
    //char* string = "File ";
    //for (int j = 1; j <= 40; ++j) {
    //	strcat(string, j);
    //	ret = createFile(string);
    //}
    //i++;

    /////////Maximum length of file name is 32 characters NF2  ERROR EXPECTED
    //printf("TEST NUMBER %d: NON FUNCTIONAL REQUIREMENT 2\n", i);
    //ret = createFile("abcdefghijklmnopqrstuvwxyzabcdefg"); //33 characters
    //if(ret == 0) {
    //	TEST_FAILED("create (created the file anyway)");
    //	return -1;
    //}
    //ret = createFile("abcdefghijklmnopqrstuvwxyzabcde"); //31 characters
    //if(ret != 0) {
    //	TEST_FAILED("create");
    //	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST create ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
    //	return -1;
    //}
    //TEST_PASSED("create");
    //i++;
    //It's already been unmounted so we mount it again
    //ret = mountFS();
    //Now check metadata

    close(fd2);
    return 0;
}
