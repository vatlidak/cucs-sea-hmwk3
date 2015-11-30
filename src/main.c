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
#include <limits.h>
#include <libgen.h>

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
 * Assert that filename is under /tmp or cwd
 */
static inline int is_permitted_path(char *path)
{
	char *rval;
	char *pathcopy;
	char cwd[PATH_MAX];
	
	pathcopy = calloc(PATH_MAX, sizeof(char));
	if (!pathcopy) {
		perror("calloc");
		goto error;
	}
	strncpy(pathcopy, path, strlen(path));
	dirname(pathcopy);
	
	rval = getcwd(cwd, PATH_MAX);
	if (!rval) {
		perror("getcwd");
		goto error_free;
	}
	printf("%s\n", cwd);
	if (strcmp(pathcopy, cwd) && strcmp(pathcopy, "/tmp"))
		goto error_free;

	free(pathcopy);
	return OK;

error_free:
	free(pathcopy);
error:
	return NOT_OK;
}


/*
 * Unravel a relative path and convert it into absolute
 *
 * Note: Do nothing if already absolute path
 */
static inline int unravel_relative_path(char **path)
{
	return 0;
}


/*
 * Parse and sanitize filename
 */
static int parse_filename(char **filename)
{
	int rval;

	/*
	 * here we do the convertions and other string checks
	 */

	rval = unravel_relative_path(filename);
	if (rval) {
		fprintf(stderr,
			"E: cannot unravel relative path: \"%s\"\n", *filename);
		goto error;
	}


	return OK;
error:
	return NOT_OK;
}


/*
 * Parse and sanitize datafield
 */
static int parse_datafield(char **datafield)
{
	/*
	 * here we do the convertions and other string checks
	 */

	*(*datafield) = (char)'\x21';
	*(*datafield + 1) = (char)'\041';
	*(*datafield + 2) = '\101';
	*(*datafield + 3) = '\262';
	*(*datafield + 4) = '\xb2';
	printf("datafield:%s\n", *datafield);
	return 0;
}


static int get_fields(char *line, char **datafield, char **filename)
{

	int j;
	char *string = "v.\"quoted\"atlidak\"quoted\"is";
	
	char ch;
	j =0;

	int quoted = 0;
	while ((ch = string[j++]))
	{
		if (ch == '"') {
			++quoted;
			quoted = quoted % 2;
			continue;
		}
		printf("%d:%c, %d\n", j, ch, quoted);
	}




	*filename = calloc(100, sizeof(char));
	strcpy(*filename, "/tmp/");

	*datafield = calloc(100, sizeof(char));
	strcpy(*datafield, "");
	// = "a";//lala\" && cd ..; ls; cd ->/dev/null\"";

	return 0;
}

/*
 * main: main entry point
 */
int main(int argc, char **argv)
{
	int len;
	int rval;
	char *line;
	char *buf;
	char *filename;
	char *datafield;
	char *e_filename;
	size_t n = (size_t)12345;


	line = NULL;
	rval = getline(&line, &n, stdin);
	if (rval == -1) {
		perror("getline");
		goto error;
	}
	/*
	 * the following checks assert that no unquoted NULL bytes are passed 
	 * in the fields
	 */
	if (rval != strlen(line)) {
		fprintf(stderr,
			"E: (nul) bytes are not permitted in input fields\n");
		goto error_free_line;
	}
	rval = get_fields(line, &datafield, &filename); 
	if (rval) {
		fprintf(stderr, "E: Cannot split fields\n");
		goto error_free_line;
	}
	rval = parse_datafield(&datafield);
	if (rval) {
		fprintf(stderr, "E: Illegal data field\n");
		goto error_free_line;
	}
	rval = parse_filename(&filename);
	if (rval) {
		fprintf(stderr, "E: Illegal filename\n");
		goto error_free_line;
	}

	/*
	 * Append ".vatlidak' (the rrespective of uni) to avoid collisions.
	 * Nees two  additional bytes; one for the "dot" and another one
	 * for the final NULL byte.
	 */
	len = strlen(filename) + 1 + strlen(USERNAME) + 1;
	e_filename = calloc(len, sizeof(char));
	if (!e_filename) {
		perror("calloc");
		goto error_free_line;
	}
	snprintf(e_filename, len, "%s.%s", filename, USERNAME);
	printf("e_filename:%s\n", e_filename);

	if (is_permitted_path(e_filename)) {
		fprintf(stderr,
			"E: not permitted file path: \"%s\"\n", e_filename);
		goto error_free_line_e_filename;
	}
	/*
	 * passed this point inputs are (hopefully) sanitized
	 */


	len = strlen("echo \"\" > \"\"") + strlen(datafield)\
	      + strlen(e_filename) + 1;
	buf = calloc(len, sizeof(char));
	if (!buf) {
		perror("calloc");
		goto error_free_line_e_filename;
	}
	snprintf(buf, len, "echo \"%s\" > \"%s\"", datafield, e_filename);
	printf("%s\n", buf);

	rval = system(buf);
	if (rval == NOT_OK) {
		perror("system");
		goto error_free_line_e_filename_buf;
	}
	free(buf);
	free(line);
	free(e_filename);
	return OK;

error_free_line_e_filename_buf:
	free(buf);
error_free_line_e_filename:
	free(e_filename);
error_free_line:
	free(line);
error:
	return NOT_OK;
}
