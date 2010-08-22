/*
 * Copyright (c) 2009, 2010 Thomas Cyron <thomas@thcyron.de>
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/types.h>

#include <err.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define	DEFAULT_INCREMENT	100
#define	DEFAULT_STARTSIZE	30000

static char	*commands;
static uint32_t	 commands_size;
static uint32_t	 commands_pos;

static uint8_t	*ptr;
static uint8_t 	*data;
static uint32_t	 data_size;

static uint32_t	 increment = DEFAULT_INCREMENT;
static uint32_t	 startsize = DEFAULT_STARTSIZE;
static int	 noalloc;
static int	 debugmode;

static int
iscommand(char c)
{
	switch (c) {
	case '>':
	case '<':
	case '+':
	case '-':
	case '.':
	case ',':
	case '[':
	case ']':
		return (1);
	default:
		return (0);
	}
}

static void
addcommand(char c)
{
	if (commands_size == commands_pos) {
		if (noalloc)
			errx(1, "commands array too small and -n set");

		if (commands_size > UINT32_MAX - increment)
			errx(1, "integer overflow for commands array size");

		commands_size += increment;

		if ((commands = realloc(commands, commands_size)) == NULL)
			err(1, "cannot extend commands array");

		memset(commands + commands_size - increment, 0, increment);
	}

	commands[commands_pos++] = c;
}

static void
run(void)
{
	uint32_t i, brackets, level = 0, count = 0;

	for (i = 0; i < commands_pos; ++i) {
		if (debugmode)
			fprintf(stderr, "(%u) command %c pos %u level %u "
			    "ptr %li *ptr %u\n", ++count, commands[i], i,
			    level, ptr - data, *ptr);

		switch (commands[i]) {
		case '>':
			if (ptr == data + data_size)
				errx(1, "trying to access foreign "
				    "memory at %p", ptr + 1);
			++ptr;
			break;
		case '<':
			if (ptr == data)
				errx(1, "trying to access foreign "
				    "memory at %p", ptr - 1);
			--ptr;
			break;
		case '+':
			if (*ptr == UINT8_MAX)
				errx(1, "integer overflow");
			++*ptr;
			break;
		case '-':
			if (*ptr == 0)
				errx(1, "integer underflow at command %i "
				    "(*ptr = %i)", i, *ptr);
			--*ptr;
			break;
		case '.':
			if (!debugmode)
				putchar(*ptr);
			break;
		case ',':
			*ptr = getchar();
			break;
		case '[':
			if (*ptr) {
				level++;
				break;
			}

			brackets = 1;

			while (brackets > 0) {
				if (++i == commands_pos)
					errx(1, "unbalanced brackets");
				if (commands[i] == '[')
					brackets++;
				if (commands[i] == ']')
					brackets--;
			}
			break;
		case ']':
			if (level == 0)
				errx(1, "unbalanced brackets");

			if (!*ptr) {
				level--;
				break;
			}

			brackets = 1;

			while (brackets > 0) {
				if (--i == commands_pos)
					errx(1, "unbalanced brackets");
				if (commands[i] == '[')
					brackets--;
				if (commands[i] == ']')
					brackets++;
			}
			break;
		}
	}

	if (level > 0)
		errx(1, "unbalanced brackets");
}

static void
usage(void)
{
	extern char *__progname;

	fprintf(stderr,
	    "Usage: %s [-dhn] [-i inc] [-s size] [file]\n\n"
	    "Options:\n"
	    "    -d       Enable debug mode\n"
	    "    -h       Show this help\n"
	    "    -i inc   Increment data memory by <inc> bytes (default: %u)\n"
	    "    -n       Do not increase memory on demand (fixed memory size)\n"
	    "    -s size  Initialize data memory with <size> bytes (default: %u)\n",
	    __progname, DEFAULT_INCREMENT, DEFAULT_STARTSIZE);

	exit(1);
}

int
main(int argc, char **argv)
{
	char c;
	FILE *f;

	while ((c = getopt(argc, argv, "dhi:ns:")) != -1) {
		switch (c) {
		case 'd':
			debugmode = 1;
			break;
		case 'i':
			increment = strtol(optarg, NULL, 10);
			if (increment == 0)
				errx(1, "increment must be grater than 0");
			break;
		case 'n':
			noalloc = 1;
			break;
		case 's':
			startsize = strtol(optarg, NULL, 10);
			if (startsize == 0)
				errx(1, "startsize must be greater than 0");
			break;
		case 'h':
		default:
			usage();
		}
	}

	argc -= optind;
	argv += optind;

	if (argc > 1)
		usage();

	if (argc == 1) {
		if (argv[0][0] == '-' && argv[0][1] == '\0')
			f = stdin;
		else
			if ((f = fopen(argv[0], "r")) == NULL)
				err(1, "cannot open file `%s'", argv[0]);
	} else
		f = stdin;

	commands_size = startsize;
	commands = calloc(1, commands_size);

	if (commands == NULL)
		err(1, "cannot alloc commands array");

	data_size = startsize;
	data = ptr = calloc(1, data_size);

	if (data == NULL)
		err(1, "cannot alloc data memory");

	while ((c = fgetc(f)) != EOF)
		if (iscommand(c))
			addcommand(c);

	fclose(f);

	run();

	free(commands);
	free(data);

	return (0);
}
