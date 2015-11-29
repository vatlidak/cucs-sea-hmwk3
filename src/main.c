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


static char *parse_filename(char *filename)
{
	return NULL;
}


static char *parse_datafield(char *datafield)
{
	return NULL;
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
	//size_t n = (size_t)12345;

	//line = NULL;
	//getline(&line, &n, stdin);

	char *filename = "filename.txt";
	char *datafield = "a";//lala\" && cd ..; ls; cd ->/dev/null\"";



	len = strlen("echo \"\" > \"\"") + strlen(datafield) + strlen(filename);
	buf = calloc(len+1, sizeof(char));
	if (!buf) {
		perror("calloc");
		goto error;
	}

	snprintf(buf, len+1, "echo \"%s\" > \"%s\"", datafield, filename);
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
