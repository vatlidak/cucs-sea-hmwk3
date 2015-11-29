/*
 * Filename: src/main.c
 *
 * Description: XXX
 *
 * Copyright (C) 2015 V. Atlidakis
 *
 * COMS W4187 Fall 2015, Columbia University
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "defines.h"


static inline int is_letter(char c)
{
	return 0;
}


static inline int is_digit(char c)
{
	return 0;
}


static inline int is_international(char c)
{
	return 0;
}


static inline int is_null(char c)
{
	return 0;
}


/*
 * Unravel a relative path and converts it into absolute
 */
static inline char* unravel_relative_path(char *path)
{
	return NULL;
}


static inline int is_valid_filename(char *path)
{
	return 0;
}

/*
 * Parse and sanitize filename
 */
static int parse_filename(char **filename)
{
	return 0;
}

/*
 * Parse and sanitize datafield
 */
static int parse_datafield(char **datafield)
{
	return 0;
}


/*
 * main: main entry point
 */
int main(int argc, char **argv)
{
	int len;
	int rval;
	//char *line;
	char *buf;
	char username[USERNAME_LEN];
	//size_t n = (size_t)12345;


	char *filename = "filename.txt";
	char *datafield = "a";//lala\" && cd ..; ls; cd ->/dev/null\"";


	//line = NULL;
	//getline(&line, &n, stdin);
	//
	// -- split into filename and datafield fields
	//`

	rval = parse_filename(&filename);
	if (rval) {
		fprintf(stderr, "Illegal filename\n");
		goto error;
	}

	rval = parse_datafield(&datafield);
	if (rval) {
		fprintf(stderr, "Illegal data field\n");
		goto error;
	}
	/*
	 * passed this point inputs are sanitized (hopefully)
	 */

	rval = getlogin_r(username, USERNAME_LEN);
	if (rval) {
		perror("getlogin_r");
		goto error;
	}
	
	len = strlen("echo \"\" > \".\"") + strlen(datafield) + 
		strlen(filename) + strlen(username) + 1;
	buf = calloc(len, sizeof(char));
	if (!buf) {
		perror("calloc");
		goto error;
	}

	snprintf(buf, len, "echo \"%s\" > \"%s.%s\"",
		 datafield, filename, username);
	printf("%s\n", buf);

	
	rval = system(buf);
	if (rval == NOT_OK) {
		perror("system");
		goto error_free;
	}

	free(buf);
	return OK;

error_free:
	free(buf);
error:
	return NOT_OK;
}
