#ifndef _USER_H_
#define _USER_H_

/* This file contains the interface of the functions that must be implemented
 * to allow the user access to the file system and the files. IT CANNOT BE 
 * MODIFIED.
 */

#define DEVICE_IMAGE disk.dat
#define DEVICE_SIZE (100*1024)
#define MAX_FILE_SIZE 1024
#define MAX_VERSION 10

/***************************/
/* File system management. */
/***************************/

/*
 * Formats a device.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int mkFS(int maxNumFiles);

/*
 * Mounts a file system from the device deviceName.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int mountFS();

/*
 * Unmount file system.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int umountFS();


/*******************/
/* File read/write */
/*******************/

/*
 * Creates or opens a file.
 * Returns file descriptor or -1 in case of error.
 */
int openFS(char *fileName);

/*
 * Closes a file.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int closeFS(int fileDescriptor);

/*
 * Reads a number of bytes from a file and stores them in a buffer.
 * Returns the number of bytes read or -1 in case of error.
 */
int readFS(int fileDescriptor, void *buffer, int numBytes, int *currentVersion);

/*
 * Reads number of bytes from a buffer and writes them in a file.
 * Update parameter newVersion with the number of the version created for this
 * write. Exceeding the number of versions allowed is considered an error.
 * Returns the number of bytes written or -1 in case of error.
 */
int writeFS(int fileDescriptor, void *buffer, int numBytes, int *newVersion);

/*
 * Repositions the pointer of a file. A greater offset than the current size is
 * considered an error. Returns new position or -1 in case of error.
 */
int lseekFS(int fileDescriptor, long offset, int whence);


/**********************/
/* Version management */
/**********************/

/*
 * Changes current version of the file to the one specified as a parameter.
 * Changing to a non-existent version is considered an error.
 * Returns 0 if the operation was correct or -1 in case of error.
 */
int switchFS(char *fileName, int version);

/*
 * Deletes specified version from a file's list of versions. Updates current
 * version if necessary. Trying to delete a non-existent version is considered
 * an error. Returns 0 if the operation was correct or -1 in case of error.
 */
int deleteFS(char *filename, int version);

#endif
