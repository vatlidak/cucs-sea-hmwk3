/*
 * Filename: src/main.c
 *
 * Description: input sanitization
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
#include <ctype.h>

#include "defines.h"


static inline int valid_assci_code(int code)
{
	if (code < 1 || code > 255)
		return NOT_OK;
	return OK;
}


static inline int valid_unquoted_byte(char c)
{
	unsigned char code = (unsigned char)c;

	if ( (code >= (unsigned char)192 && code <= (unsigned char)207 )
		|| (code >= (unsigned char)208 && code <= (unsigned char)214 )
		|| (code >= (unsigned char)216 && code <= (unsigned char)223 )
		|| (code >= (unsigned char)224 && code <= (unsigned char)239 )
		|| (code >= (unsigned char)240 && code <= (unsigned char)246 )
		|| (code >= (unsigned char)248 && code <= (unsigned char)255 )
		|| (code >= (unsigned char)65  && code <= (unsigned char)122)
		|| (code >= (unsigned char)48  && code <= (unsigned char)57)
		|| (code == (unsigned char)178 ||  code ==(unsigned char) 179 ||  code == (unsigned char)185)
		|| (code == (unsigned char)131 ||  code ==(unsigned char) 138 ||  code == (unsigned char)140)
		|| (code == (unsigned char)142 ||  code ==(unsigned char) 154 ||  code == (unsigned char)156)
		|| (code == (unsigned char)158 ||  code ==(unsigned char) 159 ||  code == (unsigned char)170)
		|| (code == (unsigned char)186)
	)
		goto ok;

	return NOT_OK;
ok:
	return OK;

}


static inline int valid_quoted_byte(unsigned char c)
{
	unsigned char code = c;

	return code != 0 ? OK : NOT_OK;
}


/*
 * Assert that filename is under /tmp or cwd
 */
static inline int is_permitted_path(char *path)
{
	char *rval;
	char *pathcopy;
	char cwd[PATH_MAX + 1];
	
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
 * Note: Do nothing if already absolute path, else continue
 * recursively until path is fully expanded
 */
static inline int unravel_relative_path(char **path)
{
	int i;
	int newend;
	int end = strlen(*path);
	char copy[PATH_MAX + 1];

	strncpy(copy, *path, PATH_MAX);

	int nslash = 0;
	for (i = end; i > 3; i--) {
		if (copy[i] == '/')
			nslash++;
		if (copy[i - 2] == '.' && copy[i - 1] == '.' && copy[i] == '/') {
			copy[i - 2] = '\0';
			copy[i - 1] = '\0';
			copy[i] = '\0';
			/* going up */
			if (nslash == 1) {
				i -= 4;
				if (copy[i] == '.') {
					while (i > 0 ) {
						i--;
						if (copy[i] != '/' && copy[i] != '.') {
							i--;
							break;
						}
					}
				}
				while (i > 0) {
					if (copy[i] == '/') {
						copy[i] = '\0';
						break;
					}
					copy[i] = '\0';
					i--;
				}
			}
			/* going down */
			if (nslash > 1) {
				i += 1;
				while (i < end) {
					if (copy[i] == '/') {
						copy[i] = '\0';
						break;
					}
					copy[i] = '\0';
					i++;
				}
			}	
		}
	}
	newend = 0;
	for (i = 0; i < end; i++) {
		if (copy[i] != '\0') {
			*(*path + newend) = copy[i];
			newend++;
		}
	}
	*(*path + newend) = '\0';

	/* recursively expand all */
	if (end != newend)
		return unravel_relative_path(path);
	
	return OK;
}


/*
 * Assert valide format
 *
 */
static int validate_format(char *expression)
{
	int i;
	int start;
/*	int nquotes1, nquotes2; */

	/* assert types of quotes match */
	start = -1;
	for (i = 0; expression[i] != '\0'; i++) {
		if (expression[i] == '\'' || expression[i] == '"') {
			start = i;
			break;
		}
	}
	if (start >= 0) {
		for (i = strlen(expression) - 1; i > start; i--) {
			if (expression[i] == '\'' || expression[i] == '"') {
			    	if (expression[i] != expression[start])
					return NOT_OK;
				break;
			}
		}
	}

/*	
	for (i = 0; expression[i] != '\0'; i++)
	{
		if (expression[i] == '\'')
			nquotes1++;
		if (expression[i] == '"')
			nquotes2++;

	}
	if ((nquotes1 % 2) || (nquotes2 % 2))
		return NOT_OK;
*/
	return OK;
}


static int get_fields(char **line, char **filename, char **datafield)
{
	int j;
	int quoted;
	char ch;
	char opening_quote;
	
	/*
	 * make a forward pass setting NULL to unquoted delimiters and setting
	 * up datafield (first field)
	 */
	j = 0;
	quoted = 0;
	*filename = NULL;
	while ((ch = *(*line + j)))
	{
//		printf("quoted:%d, %c\n", quoted, ch);
		if (ch == '"' || ch == '\'') {
			/* quoting opening with first characted single quote */
			if (j == 0) {
				++quoted;
				quoted = quoted % 2;
				opening_quote = ch;
				goto loop;
			}
			/* or, if current quote is not previously escaped */
			else if (*(*line + j - 1) != '\\' && ch == opening_quote) {
				++quoted;
				quoted = quoted % 2;
				goto loop;
			}
		}
		if (!quoted & (*(*line + j) == '\t' || *(*line + j) == ' ')) {
			*filename = *line;
			*(*line + j) = '\0';
		}
loop:
		j++;
	}
	/*
	 * make a backward pass seeking the begining of filename (second field)
	 */
	j--;
	*datafield = NULL;
	while (*(*line + j) != '\0' && j > 0) {
		*datafield = (*line + j);
		j--;
	}
	return (*filename && *datafield) ? OK : NOT_OK;
}


/*
 * Parse and sanitize filename
 */
static int parse_filename(char **filename)
{
	int j, jj;
	int len;
	int rval;
	int quoted;
	char ch;
	char *dummy_ptr;
	char *copy;
	char opening_quote;

	j = 0;
	jj = 0;
	quoted = 0;
	len = strlen(*filename);
	copy = calloc(len + 1, sizeof(char));
	if (!copy) {
		perror("calloc");
		return NOT_OK;
	}

	while ((ch = *(*filename + j)) && j < len)
	{
		if (ch == '"' || ch == '\'') {
			/* quoting opening with first characted single quote */
			if (j == 0) {
				++quoted;
				quoted = quoted % 2;
				opening_quote = ch;
				goto loop;
			}
			/* or, if current quote is not previously escaped */
			else if (*(*filename + j - 1) != '\\' && ch == opening_quote) {
				++quoted;
				quoted = quoted % 2;
				goto loop;
			}
		}
		if (quoted) {
			if(valid_quoted_byte(ch) != OK)
				goto error;
			if (ch == '\\' && j < len - 1) {
				/* if slash has special meaning (c escapes)*/
				if (*(*filename + j + 1) == '"') {
					copy[jj++] = '\\';
					copy[jj++] = '"';
					j++;
					goto loop;
				}
				if (*(*filename + j + 1) == '\'') {
					copy[jj++] = '\\';
					copy[jj++] = '\'';
					j++;
					goto loop;
				}

				if (*(*filename + j + 1) == 'n') {
					copy[jj++] = '\n';
					j++;
					goto loop;
				}
				if (*(*filename + j + 1) == 'r') {
					copy[jj++] = '\r';
					j++;
					goto loop;
				}
				if (*(*filename + j + 1) == 't') {
					copy[jj++] = '\t';
					j++;
					goto loop;
				}
				if (*(*filename + j + 1) == '\\') {
					copy[jj++] = '\\';
					j++;
					goto loop;
				}
				/* if slash opens octal */
				rval = strtoul((*filename + j + 1), &dummy_ptr, 8);
				if (!rval)
					goto error;
				copy[jj++] = rval;
				j += 3;
				goto loop;
			} else {
				copy[jj++] = ch;
			}
		} else  {
			copy[jj++] = ch;
			if(valid_unquoted_byte(ch) != OK)
				goto error;
			goto loop;
		}
loop:
		j++;
	}
	strncpy(*filename, copy, jj+1);
	free(copy);
	printf("%s\n", *filename);
	return OK;
error:
	free(copy);
	return NOT_OK;
}


/*
 * Parse and sanitize datafield
 */
static int parse_datafield(char **datafield)
{
	int j, jj;
	int len;
	int rval;
	int quoted;
	char ch;
	char *dummy_ptr;
	char *copy;
	char opening_quote;

	j = 0;
	jj = 0;
	quoted = 0;
	printf("%s\n", *datafield);
	len = strlen(*datafield);
	copy = calloc(3*len + 1, sizeof(char));
	if (!copy) {
		perror("calloc");
		return NOT_OK;
	}
	while ((ch = *(*datafield + j)) && j < len)
	{
		if (ch == '"' || ch == '\'') {
			/* quoting opening with first characted single quote */
			if (j == 0) {
				opening_quote = ch;
				++quoted;
				quoted = quoted % 2;
				goto loop;
			}
			/* or, if current quote is not previously escaped */
			else if (*(*datafield + j - 1) != '\\' && opening_quote == ch) {
				++quoted;
				quoted = quoted % 2;
				goto loop;
			}
		}
		if (quoted) {
			if(valid_quoted_byte(ch) != OK)
				goto error;
			if (len <= 3)
				goto error;
			if (ch == '\\' && j < len - 3) {
				//printf("--%c\n", ch);
				//printf("----%c\n", *(*datafield + j + 1));
				//printf("------%c\n", *(*datafield + j + 2));

				
				/* if slash has special meaning (c escapes)*/
				if (*(*datafield + j + 1) == '"') {
					printf("here\n");				
					copy[jj++] = '\\';
					copy[jj++] = '"';
					j++;
					goto loop;
				}
				if (*(*datafield + j + 1) == 'n') {
					printf("here1\n");
					copy[jj++] = '\\';
					copy[jj++] = 'n';
					j++;
					goto loop;
				}
				if (*(*datafield + j + 1) == 'r') {
					copy[jj++] = '\\';
					copy[jj++] = 'r';
					j++;
					goto loop;
				}
				if (*(*datafield + j + 1) == 't') {
					copy[jj++] = '\\';
					copy[jj++] = 't';
					j++;
					goto loop;
				}
				//if (*(*datafield + j + 1) == '\\'
				//    && (*(*datafield + j + 2) != 'n'
				//	&& *(*datafield + j + 2) != 'r'
				//	&& *(*datafield + j + 2) != 't'  ))
				//{
				//	printf("here\n");
				//	copy[jj++] = '\\';
				//	j++;
				//	goto loop;
				//}
				if (*(*datafield + j + 1) == '\\'
				    && (*(*datafield + j + 2) == 'n'
					|| *(*datafield + j + 2) == 'r'
					|| *(*datafield + j + 2) == 't'  ))
				{
					printf("here3\n");

					copy[jj++] = '\\';
					copy[jj++] = '\\';
					/* sh needs four slashes to print literals of c escapes */
					copy[jj++] = '\\';
					copy[jj++] = '\\';
					copy[jj++] = *(*datafield + j + 2);
					j += 2;
					goto loop;
				}
				if (*(*datafield + j + 1) == '\'') {
					copy[jj++] = '\\';
					copy[jj++] = '\'';
					j++;
					goto loop;
				}
				if (*(*datafield + j + 1) == '\\') {
					printf("here1111\n");
					copy[jj++] = '\\';
					copy[jj++] = '\\';
					j++;
					goto loop;
				}
				/* if slash opens octal */
				rval = strtoul((*datafield + j + 1), &dummy_ptr, 8);
				printf("octal val:%d\n", rval);
				if (!rval || !isdigit(*(*datafield + j + 1)) || !isdigit(*(*datafield + j + 2)) ||  !isdigit(*(*datafield + j + 3)))
					goto error;
				if(valid_assci_code(rval) != OK)
					goto error;
				copy[jj++] = rval;
				j += 3;
				goto loop;
			} else {
				copy[jj++] = ch;
			}
		} else  {
			copy[jj++] = ch;
			if(valid_unquoted_byte(ch) != OK)
				goto error;
			goto loop;
		}
loop:
		j++;
	}
	/* hack to get rid of final " */
	if (*(*datafield + jj) == '"')
		jj--;
	strncpy(*datafield, copy, jj+1);
	printf("%s\n", *datafield);
	free(copy);
	return OK;
error:
	free(copy);
	return NOT_OK;
}


/*
 * main: main entry point
 */
int main(int argc, char **argv)
{
	int len;
	int rval;
	int fatal_error;
	char *line;
	char *buf;
	char *filename;
	char *datafield;
	char *e_filename;
	char cwd[PATH_MAX + 1];
	size_t n = (size_t)12345;

	fatal_error = 0;

get_next_line:
	line = NULL;
	/* rval = getdelim(&line, &n, EOF, stdin); */
	rval = getline(&line, &n, stdin);
	if (rval == -1) {
		return OK;
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

	/* overwrite ending newline */
	len = strlen(line);
	line[len-1] = '\0';

	rval = validate_format(line);
	if (rval == NOT_OK) {
		fprintf(stderr, "E: invalid input format\n");
		goto error_free_line;
	}
	rval = get_fields(&line, &filename, &datafield);
	if (rval) {
		fprintf(stderr, "E: Cannot split fields\n");
		goto error_free_line;
	}
	rval = parse_filename(&filename);
	if (rval != OK) {
		fprintf(stderr, "E: Illegal filename\n");
		goto error_free_line;
	}
	rval = parse_datafield(&datafield);
	if (rval != OK) {
		fprintf(stderr, "E: Illegal data field\n");
		goto error_free_line;
	}
	/*
	 * Append ".vatlidak' (the rrespective of uni) to avoid collisions.
	 * Nees two  additional bytes; one for the "dot" and another one
	 * for the final NULL byte.
	 */
	/*
	 * Also, convert plain filename, to relative in cwd
	 */
	if (strchr(filename, '/')) {
	    	/*relative path already -- do nothing more */
		len = strlen(filename) + 1 + strlen(USERNAME) + 1;
	} else {
		getcwd(cwd, PATH_MAX);
		len = strlen(cwd) + 1 + strlen(filename) + 1 + strlen(USERNAME) + 1;
	}
	e_filename = calloc(len, sizeof(char));
	if (!e_filename) {
		fatal_error = 1;
		perror("calloc");
		goto error_free_line;
	}
	if (strchr(filename, '/')) {
		snprintf(e_filename, len, "%s.%s", filename, USERNAME);
	} else {
		snprintf(e_filename, len, "%s/%s.%s", cwd,filename, USERNAME);
	}
	if (unravel_relative_path(&e_filename) != OK) {
		fprintf(stderr, "E: Cannot expand relative path");
		goto error_free_line_e_filename;
	}
	//printf("e_filename:%s\n", e_filename);
	if (is_permitted_path(e_filename)) {
		fprintf(stderr,
			"E: Not permitted file path: \"%s\"\n", e_filename);
		goto error_free_line_e_filename;
	}
	/*
	 * passed this point inputs are (hopefully) sanitized
	 */


	len = strlen("echo \"\" >> \"\"") + strlen(datafield)\
	      + strlen(e_filename) + 1;
	buf = calloc(len, sizeof(char));
	if (!buf) {
		fatal_error = 1;
		perror("calloc");
		goto error_free_line_e_filename;
	}
	snprintf(buf, len, "echo \"%s\" >> \"%s\"", datafield, e_filename);
	printf("<%s>\n", buf);
	/* TODO: bobby Tables */
	rval = system(buf);
	if (rval == NOT_OK) {
		perror("system");
		goto error_free_line_e_filename_buf;
	}
	printf("OK \"%s\" ready\n", filename);
	free(buf);
	free(line);
	free(e_filename);
	goto get_next_line;

error_free_line_e_filename_buf:
	free(buf);
error_free_line_e_filename:
	free(e_filename);
error_free_line:
	free(line);
	if (fatal_error)
		return NOT_OK;
	else
		goto get_next_line;
}
