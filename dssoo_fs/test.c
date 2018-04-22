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
    //TODO: help @guillermoescobero
    //printf("mkFS(DEV_SIZE + 1024) %d", mkFS(DEV_SIZE + BLOCK_SIZE * 40));
    //printf("mkFS(DEV_SIZE) %d", mkFS(DEV_SIZE));
    //if (mkFS(DEV_SIZE + 1024) == -1 && mkFS(DEV_SIZE) == 0) {
    //    TEST_PASSED("mkFS");
    //} else {
    //    TEST_FAILED("mkFS");
    //}
    mkFS(DEV_SIZE);

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
    if (createFile("test.txt") == 0 && createFile("test.txt") == -1) {
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
    int testFileDescriptor = openFile("test.txt");
    if (testFileDescriptor == 0 && openFile("random.txt") == -1) {
        TEST_PASSED("openFile");
    } else {
        TEST_FAILED("openFile");
    }

    // Fill the buffer with text using C standard functions to avoid introducing errors
    int quijoteFileDescriptor = open("quijote.txt", O_RDONLY);
    char write_buffer[3000];
    read(quijoteFileDescriptor, write_buffer, sizeof(write_buffer));

    /**
     * @test 9
     * @requirement F1.9, F7
     * @description Write to an opened file, A file could be modified by means of write operations.
     */
    TEST_PRINT("9", "F1.9, F7");
    if (writeFile(testFileDescriptor, write_buffer, sizeof(write_buffer) - sizeof(char) * 8) == 0) {
        lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);
        if (writeFile(testFileDescriptor, write_buffer, sizeof(char) * 8) == sizeof(char) * 8) {
            TEST_PASSED("writeFile");
        } else {
            TEST_FAILED("writeFile");
        }
    } else {
        TEST_FAILED("writeFile");
    }

    lseekFile(testFileDescriptor, 0, FS_SEEK_BEGIN);

    /**
     * @test 8
     * @requirement F1.8
     * @description Read from an opened file
     */
    TEST_PRINT("8", "F1.8");
    char read_buffer[sizeof(char) * 8 + 2];

    int a = readFile(testFileDescriptor, read_buffer, sizeof(char) * 3);
    printf("%s\n", read_buffer);
    int b = readFile(testFileDescriptor, read_buffer, sizeof(char) * 3);
    printf("%s\n", read_buffer);
    int c = readFile(testFileDescriptor, read_buffer, sizeof(char) * 2);
    printf("%s\n", read_buffer);
    if (a == sizeof(char) * 5) {
        if (b == sizeof(char) * 2) {
            if (c == 0) {
                TEST_PASSED("readFile");
            } else {
                TEST_FAILED("readFile 1");
            }
        } else {
            TEST_FAILED("readFile 2");
        }
    } else {
        TEST_FAILED("readFile 3");
        printf("a %d b %d c %d \n", a, b, c);
    }

    /**
     * @test 10
     * @requirement F1.10
     * @description Modify the position of the seek pointer
     */
    TEST_PRINT("10", "F1.10");
    char read_string[8];
    char *init_string = "#INIT#";
    if (lseekFile(testFileDescriptor, 10, FS_SEEK_BEGIN) == 0) {
        readFile(testFileDescriptor, read_string, sizeof(char) * 6);
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
    if (closeFile(testFileDescriptor) == 0) {
        TEST_PASSED("closeFile");
    } else {
        TEST_FAILED("closeFile");
    }

    /**
     * @test 12
     * @requirement F2
     * @description Every time a file is opened, its seek pointer will be reset to the beginning of the file.
     */
    TEST_PRINT("12", "F2");
    testFileDescriptor = openFile("test.txt");
    readFile(testFileDescriptor, read_buffer, sizeof(char) * 6);
    if (strcmp(read_buffer, init_string) == 0) {
        TEST_PASSED("lseek reset on openFile");
    } else {
        TEST_FAILED("lseek reset on openFile");
    }

    /**
     * @test 11
     * @requirement F1.11
     * @description Check the integrity an existing file
     */
    TEST_PRINT("11", "F1.11");
    if (checkFile("test.txt") == -2) {
        closeFile(testFileDescriptor);
        if (checkFile("test.txt") == 0) {
            //FIXME: no se como se usa bwrite hulio
            //bwrite("disk.dat", 2, write_buffer);
            if (checkFile("test.txt") == -1) {
                TEST_PASSED("checkFile");
            } else {
                TEST_FAILED("checkFile");
            }
        } else {
            TEST_FAILED("checkFile");
        }
    } else {
        TEST_FAILED("checkFile");
    }

    /**
     * @test 5
     * @requirement F1.5
     * @description Remove an existing file from the file system
     */
    TEST_PRINT("5", "F1.5");
    if (removeFile("test.txt") == 0 && removeFile("test.txt") == -1) {
        TEST_PASSED("removeFile");
    } else {
        TEST_FAILED("removeFile");
    }

    /**
     * @test 15
     * @requirement F5
     * @description File integrity must be checked, at least, on open operations.
     */
    TEST_PRINT("15", "F5");
    createFile("test.txt");
    //FIXME: no se como se usa bwrite hulio
    //bwrite("disk.dat", 2, write_buffer);
    if (openFile("test.txt") == -2) {
        TEST_PASSED("integrity check on openFile");
    } else {
        TEST_FAILED("integrity check on openFile");
    }

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


    close(quijoteFileDescriptor);

    //NON-FUNCTIONAL REQUIREMENTS TESTS
    fprintf(stdout, "%s", "BASIC TESTING OF NON-FUNCTIONAL REQUIREMENTS\n\n");

    /**
     * @test 19
     * @requirement NF1
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("19", "NF1");

    /**
     * @test 20
     * @requirement NF2
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("20", "NF1");

    /**
     * @test 21
     * @requirement NF3
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("21", "NF1");

    /**
     * @test 22
     * @requirement NF4
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("22", "NF1");

    /**
     * @test 23
     * @requirement NF5
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("23", "NF1");

    /**
     * @test 24
     * @requirement NF6
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("24", "NF1");

    /**
     * @test 25
     * @requirement NF7
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("25", "NF1");

    /**
     * @test 26
     * @requirement NF8
     * @description The maximum number of files in the file system will never be higher than 40.
     */
    TEST_PRINT("26", "NF1");

    return 0;
}
