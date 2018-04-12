/*
 * OPERATING SYSTEMS DESING - 16/17
 *
 * @file  auxiliary.h
 * @brief   Headers for the auxiliary functions required by filesystem.c.
 * @date	01/03/2017
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

int ialloc();
int alloc();
int ifree(int inode_id);
int bfree(int block_id);
int namei(char *fname);
int bmap(int inode_id, int offset);
int fssync();
int fileexists(char *filename);
