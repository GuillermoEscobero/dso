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
	int ret;
	int fd;
	int fdquij = open("quijote.txt", O_RDONLY);
	char *buffer = (char*)malloc(sizeof(char)*5000);
	read(fdquij, buffer, 5000);
	char buffer2[3500];

	///////

	ret = mkFS(DEV_SIZE);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mkFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = mountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = createFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////
    //
    closeFile(ret);

	ret = unmountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	///////

	ret = mountFS();
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST mountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////

	ret = createFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile lo se ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		//return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	ret = createFile("test32.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile lo se ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	fd = openFile("test32.txt");
	if(fd != 1) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST abre ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		//return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST abre ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	ret = writeFile(fd, buffer, strlen(buffer));
	printf("ret del escribe: %d\n", ret);
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST escribe ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		//return -1;
	} else {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST escribe ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
	}

	ret = readFile(fd, buffer2, sizeof(buffer2));
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lee ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
//		return -1;
	} else {
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST lee ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);
}
printf("ret del lee: %d\n", ret);

printf("PUTO CARAMELO??: %s\n", buffer2);

checkFile("test32.txt");

	///////

	ret = removeFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		//return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST removeFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	ret = createFile("test.txt");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile lo se ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		//return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST createFile ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////
	///////
closeFile(ret);
closeFile(ret);
closeFile(ret);
closeFile(ret);

	ret = unmountFS();
	printf("hasta aqui?\n");
	if(ret != 0) {
		fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_RED, "FAILED\n", ANSI_COLOR_RESET);
		return -1;
	}
	fprintf(stdout, "%s%s%s%s%s", ANSI_COLOR_BLUE, "TEST unmountFS ", ANSI_COLOR_GREEN, "SUCCESS\n", ANSI_COLOR_RESET);

	///////
	return 0;
}
