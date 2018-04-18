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

#define N_BLOCKS	25						// Number of blocks in the device
#define DEV_SIZE 	N_BLOCKS * BLOCK_SIZE	// Device size, in bytes


int main() {
	int i = 1;
	int ret;
	int fd;
	int text = open("quijote.txt", O_RDONLY);
	char *buffer = (char*)malloc(sizeof(char)*5000);
	read(text, buffer, 5000);
	char bufferRead[3500];

	//FUNCTIONAL REQUIREMENTS TESTS
	printf("BASIC TESTING OF FUNCTIONAL REQUIREMENTS\n");
	///////Create file system F1.1
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.1\n", i);
	ret = mkFS(DEV_SIZE);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;

	///////Mount file system F1.2
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.2\n", i);
	ret = mountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Create file F1.4
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.4\n", i);
	ret = createFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Open an existing file F1.6
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.6\n", i);
	fd = openFile("test.txt");
	if(fd != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST openFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;



	///////Read an opened file F1.8
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.8\n", i);
	ret = readFile(fd, bufferRead, sizeof(bufferRead));
	if(ret == -1) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST readFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Write to opened file F1.9 ALSO F7
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.9 and FUNCTIONAL REQUIREMENT 7\n", i);
	ret = writeFile(fd, buffer, sizeof(buffer));
	if(ret == -1) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST writeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


  ///////Close an opened file F1.7
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.7\n", i);
	ret = closeFile(fd);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST closeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Modify position of the seek pointer F1.10
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.10\n", i);
	ret = lseekFile(fd, 10, FS_SEEK_BEGIN);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lseekFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Check integrity of file F1.11
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.11", i);
	ret = checkFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST checkFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Remove existing file F1.5
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.5\n", i);
	ret = removeFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	///////Unmount file system F1.3
	printf("TEST NUMBER %d: FUNCTIONAL REQUIREMENT 1.3\n", i);
	ret = unmountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	i++;


	//NON-FUNCTIONAL REQUIREMENTS TESTS
	printf("BASIC TESTING OF NON-FUNCTIONAL REQUIREMENTS\n");

	///////Maximum length of file name is 32 characters NF2  ERROR EXPECTED
	printf("TEST NUMBER %d: NON FUNCTIONAL REQUIREMENT 2\n", i);
	ret = createFile("abcdefghijklmnopqrstuvwxyzabcdefg"); //33 characters
	if(ret == 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	}
	ret = createFile("abcdefghijklmnopqrstuvwxyzabcde"); //31 characters
	if(ret == 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS: did not add the file\n", ANSI_COLOR_RESET);
	i++;

	///////Max number of files cannot be higher than 40 NF1 --> check metadata constant MAX_FILESYSTEM_OBJECTS_SUPPORTED


	///////Metadata shall persist between unmount and mount operations NF5
	//It's already been unmounted so we mount it again
	ret = mountFS();
	//Now check metadata


	///////Every time a file is opened, its seek pointer will be reset to the beginning of the file. F2

	///////Metadata shall be updated after any write operation in order to properly reflect any
	///////modification in the file system. F3

	///////File integrity must be checked, at least, on open operations F5

	///////The whole contents of a file could be read by means of several read operations. F6

	///////A file could be modified by means of write operations. F7

	///////As part of a write operation, file capacity may be extended by means of additional data blocks. F8




	close(text);
	return 0;
}
