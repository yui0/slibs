/*
 * parg - parse argv
 *
 * Written in 2015-2016 by Joergen Ibsen
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty. <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#ifndef PARG_H_INCLUDED
#define PARG_H_INCLUDED

#ifdef __cplusplus
extern "C" {
#endif

#define PARG_VER_MAJOR 1        /**< Major version number */
#define PARG_VER_MINOR 0        /**< Minor version number */
#define PARG_VER_PATCH 2        /**< Patch version number */
#define PARG_VER_STRING "1.0.2" /**< Version number as a string */

/**
 * Structure containing state between calls to parser.
 *
 * @see parg_init
 */
struct parg_state {
	const char *optarg;   /**< Pointer to option argument, if any */
	int optind;           /**< Next index in argv to process */
	int optopt;           /**< Option value resulting in error, if any */
	const char *nextchar; /**< Next character to process */
};

/**
 * Structure for supplying long options to `parg_getopt_long()`.
 *
 * @see parg_getopt_long
 */
struct parg_option {
	const char *name; /**< Name of option */
	int has_arg;      /**< Option argument status */
	int *flag;        /**< Pointer to flag variable */
	int val;          /**< Value of option */
};

/**
 * Values for `has_arg` flag in `parg_option`.
 *
 * @see parg_option
 */
typedef enum {
	PARG_NOARG,  /**< No argument */
	PARG_REQARG, /**< Required argument */
	PARG_OPTARG  /**< Optional argument */
} parg_arg_num;

/**
 * Initialize `ps`.
 *
 * Must be called before using state with a parser.
 *
 * @see parg_state
 *
 * @param ps pointer to state
 */
void
parg_init(struct parg_state *ps);

/**
 * Parse next short option in `argv`.
 *
 * Elements in `argv` that contain short options start with a single dash
 * followed by one or more option characters, and optionally an option
 * argument for the last option character. Examples are '`-d`', '`-ofile`',
 * and '`-dofile`'.
 *
 * Consecutive calls to this function match the command-line arguments in
 * `argv` against the short option characters in `optstring`.
 *
 * If an option character in `optstring` is followed by a colon, '`:`', the
 * option requires an argument. If it is followed by two colons, the option
 * may take an optional argument.
 *
 * If a match is found, `optarg` points to the option argument, if any, and
 * the value of the option character is returned.
 *
 * If a match is found, but is missing a required option argument, `optopt`
 * is set to the option character. If the first character in `optstring` is
 * '`:`', then '`:`' is returned, otherwise '`?`' is returned.
 *
 * If no option character in `optstring` matches a short option, `optopt`
 * is set to the option character, and '`?`' is returned.
 *
 * If an element of argv does not contain options (a nonoption element),
 * `optarg` points to the element, and `1` is returned.
 *
 * An element consisting of a single dash, '`-`', is returned as a nonoption.
 *
 * Parsing stops and `-1` is returned, when the end of `argv` is reached, or
 * if an element contains '`--`'.
 *
 * Works similarly to `getopt`, if `optstring` were prefixed by '`-`'.
 *
 * @param ps pointer to state
 * @param argc number of elements in `argv`
 * @param argv array of pointers to command-line arguments
 * @param optstring string containing option characters
 * @return option value on match, `1` on nonoption element, `-1` on end of
 * arguments, '`?`' on unmatched option, '`?`' or '`:`' on option argument
 * error
 */
int
parg_getopt(struct parg_state *ps, int argc, char *const argv[],
            const char *optstring);

/**
 * Parse next long or short option in `argv`.
 *
 * Elements in `argv` that contain a long option start with two dashes
 * followed by a string, and optionally an equal sign and an option argument.
 * Examples are '`--help`' and '`--size=5`'.
 *
 * If no exact match is found, an unambiguous prefix of a long option will
 * match. For example, if '`foo`' and '`foobar`' are valid long options, then
 * '`--fo`' is ambiguous and will not match, '`--foo`' matches exactly, and
 * '`--foob`' is an unambiguous prefix and will match.
 *
 * If a long option match is found, and `flag` is `NULL`, `val` is returned.
 *
 * If a long option match is found, and `flag` is not `NULL`, `val` is stored
 * in the variable `flag` points to, and `0` is returned.
 *
 * If a long option match is found, but is missing a required option argument,
 * or has an option argument even though it takes none, `optopt` is set to
 * `val` if `flag` is `NULL`, and `0` otherwise. If the first character in
 * `optstring` is '`:`', then '`:`' is returned, otherwise '`?`' is returned.
 *
 * If `longindex` is not `NULL`, the index of the entry in `longopts` that
 * matched is stored there.
 *
 * If no long option in `longopts` matches a long option, '`?`' is returned.
 *
 * Handling of nonoptions and short options is like `parg_getopt()`.
 *
 * If no short options are required, an empty string, `""`, should be passed
 * as `optstring`.
 *
 * Works similarly to `getopt_long`, if `optstring` were prefixed by '`-`'.
 *
 * @see parg_getopt
 *
 * @param ps pointer to state
 * @param argc number of elements in `argv`
 * @param argv array of pointers to command-line arguments
 * @param optstring string containing option characters
 * @param longopts array of `parg_option` structures
 * @param longindex pointer to variable to store index of matching option in
 * @return option value on match, `0` for flag option, `1` on nonoption
 * element, `-1` on end of arguments, '`?`' on unmatched or ambiguous option,
 * '`?`' or '`:`' on option argument error
 */
int
parg_getopt_long(struct parg_state *ps, int argc, char *const argv[],
                 const char *optstring,
                 const struct parg_option *longopts, int *longindex);

/**
 * Reorder elements of `argv` so options appear first.
 *
 * If there are no long options, `longopts` may be `NULL`.
 *
 * The return value can be used as `argc` parameter for `parg_getopt()` and
 * `parg_getopt_long()`.
 *
 * @param argc number of elements in `argv`
 * @param argv array of pointers to command-line arguments
 * @param optstring string containing option characters
 * @param longopts array of `parg_option` structures
 * @return index of first nonoption in `argv` on success, `-1` on error
 */
int
parg_reorder(int argc, char *argv[],
             const char *optstring,
             const struct parg_option *longopts);


#ifdef PARG_IMPLEMENTATION
/*
 * parg - parse argv
 *
 * Written in 2015-2016 by Joergen Ibsen
 *
 * To the extent possible under law, the author(s) have dedicated all
 * copyright and related and neighboring rights to this software to the
 * public domain worldwide. This software is distributed without any
 * warranty. <http://creativecommons.org/publicdomain/zero/1.0/>
 */

#include "parg.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

/*
 * Check if state is at end of argv.
 */
static int
is_argv_end(const struct parg_state *ps, int argc, char *const argv[])
{
	return ps->optind >= argc || argv[ps->optind] == NULL;
}

/*
 * Match nextchar against optstring.
 */
static int
match_short(struct parg_state *ps, int argc, char *const argv[],
            const char *optstring)
{
	const char *p = strchr(optstring, *ps->nextchar);

	if (p == NULL) {
		ps->optopt = *ps->nextchar++;
		return '?';
	}

	/* If no option argument, return option */
	if (p[1] != ':') {
		return *ps->nextchar++;
	}

	/* If more characters, return as option argument */
	if (ps->nextchar[1] != '\0') {
		ps->optarg = &ps->nextchar[1];
		ps->nextchar = NULL;
		return *p;
	}

	/* If option argument is optional, return option */
	if (p[2] == ':') {
		return *ps->nextchar++;
	}

	/* Option argument required, so return next argv element */
	if (is_argv_end(ps, argc, argv)) {
		ps->optopt = *ps->nextchar++;
		return optstring[0] == ':' ? ':' : '?';
	}

	ps->optarg = argv[ps->optind++];
	ps->nextchar = NULL;
	return *p;
}

/*
 * Match string at nextchar against longopts.
 */
static int
match_long(struct parg_state *ps, int argc, char *const argv[],
           const char *optstring,
           const struct parg_option *longopts, int *longindex)
{
	size_t len;
	int num_match = 0;
	int match = -1;
	int i;

	len = strcspn(ps->nextchar, "=");

	for (i = 0; longopts[i].name; ++i) {
		if (strncmp(ps->nextchar, longopts[i].name, len) == 0) {
			match = i;
			num_match++;
			/* Take if exact match */
			if (longopts[i].name[len] == '\0') {
				num_match = 1;
				break;
			}
		}
	}

	/* Return '?' on no or ambiguous match */
	if (num_match != 1) {
		ps->optopt = 0;
		ps->nextchar = NULL;
		return '?';
	}

	assert(match != -1);

	if (longindex) {
		*longindex = match;
	}

	if (ps->nextchar[len] == '=') {
		/* Option argument present, check if extraneous */
		if (longopts[match].has_arg == PARG_NOARG) {
			ps->optopt = longopts[match].flag ? 0 : longopts[match].val;
			ps->nextchar = NULL;
			return optstring[0] == ':' ? ':' : '?';
		}
		else {
			ps->optarg = &ps->nextchar[len + 1];
		}
	}
	else if (longopts[match].has_arg == PARG_REQARG) {
		/* Option argument required, so return next argv element */
		if (is_argv_end(ps, argc, argv)) {
			ps->optopt = longopts[match].flag ? 0 : longopts[match].val;
			ps->nextchar = NULL;
			return optstring[0] == ':' ? ':' : '?';
		}

		ps->optarg = argv[ps->optind++];
	}

	ps->nextchar = NULL;

	if (longopts[match].flag != NULL) {
		*longopts[match].flag = longopts[match].val;
		return 0;
	}

	return longopts[match].val;
}

void
parg_init(struct parg_state *ps)
{
	ps->optarg = NULL;
	ps->optind = 1;
	ps->optopt = '?';
	ps->nextchar = NULL;
}

int
parg_getopt(struct parg_state *ps, int argc, char *const argv[],
            const char *optstring)
{
	return parg_getopt_long(ps, argc, argv, optstring, NULL, NULL);
}

int
parg_getopt_long(struct parg_state *ps, int argc, char *const argv[],
                 const char *optstring,
                 const struct parg_option *longopts, int *longindex)
{
	assert(ps != NULL);
	assert(argv != NULL);
	assert(optstring != NULL);

	ps->optarg = NULL;

	if (argc < 2) {
		return -1;
	}

	/* Advance to next element if needed */
	if (ps->nextchar == NULL || *ps->nextchar == '\0') {
		if (is_argv_end(ps, argc, argv)) {
			return -1;
		}

		ps->nextchar = argv[ps->optind++];

		/* Check for nonoption element (including '-') */
		if (ps->nextchar[0] != '-' || ps->nextchar[1] == '\0') {
			ps->optarg = ps->nextchar;
			ps->nextchar = NULL;
			return 1;
		}

		/* Check for '--' */
		if (ps->nextchar[1] == '-') {
			if (ps->nextchar[2] == '\0') {
				ps->nextchar = NULL;
				return -1;
			}

			if (longopts != NULL) {
				ps->nextchar += 2;

				return match_long(ps, argc, argv, optstring,
				                  longopts, longindex);
			}
		}

		ps->nextchar++;
	}

	/* Match nextchar */
	return match_short(ps, argc, argv, optstring);
}

/*
 * Reverse elements of `v` from `i` to `j`.
 */
static void
reverse(char *v[], int i, int j)
{
	while (j - i > 1) {
		char *tmp = v[i];
		v[i] = v[j - 1];
		v[j - 1] = tmp;
		++i;
		--j;
	}
}

/*
 * Reorder elements of `argv` with no special cases.
 *
 * This function assumes there is no `--` element, and the last element
 * is not an option missing a required argument.
 *
 * The algorithm is described here:
 * http://hardtoc.com/2016/11/07/reordering-arguments.html
 */
static int
parg_reorder_simple(int argc, char *argv[],
                    const char *optstring,
                    const struct parg_option *longopts)
{
	struct parg_state ps;
	int change;
	int l = 0;
	int m = 0;
	int r = 0;

	if (argc < 2) {
		return argc;
	}

	do {
		int nextind;
		int c;

		parg_init(&ps);

		nextind = ps.optind;

		/* Parse until end of argument */
		do {
			c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);
		} while (ps.nextchar != NULL && *ps.nextchar != '\0');

		change = 0;

		do {
			/* Find next non-option */
			for (l = nextind; c != 1 && c != -1;) {
				l = ps.optind;

				do {
					c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);
				} while (ps.nextchar != NULL && *ps.nextchar != '\0');
			}

			/* Find next option */
			for (m = l; c == 1;) {
				m = ps.optind;

				do {
					c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);
				} while (ps.nextchar != NULL && *ps.nextchar != '\0');
			}

			/* Find next non-option */
			for (r = m; c != 1 && c != -1;) {
				r = ps.optind;

				do {
					c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);
				} while (ps.nextchar != NULL && *ps.nextchar != '\0');
			}

			/* Find next option */
			for (nextind = r; c == 1;) {
				nextind = ps.optind;

				do {
					c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);
				} while (ps.nextchar != NULL && *ps.nextchar != '\0');
			}

			if (m < r) {
				change = 1;
				reverse(argv, l, m);
				reverse(argv, m, r);
				reverse(argv, l, r);
			}
		} while (c != -1);
	} while (change != 0);

	return l + (r - m);
}

int
parg_reorder(int argc, char *argv[],
             const char *optstring,
             const struct parg_option *longopts)
{
	struct parg_state ps;
	int lastind;
	int optend;
	int c;

	assert(argv != NULL);
	assert(optstring != NULL);

	if (argc < 2) {
		return argc;
	}

	parg_init(&ps);

	/* Find end of normal arguments */
	do {
		lastind = ps.optind;

		c = parg_getopt_long(&ps, argc, argv, optstring, longopts, NULL);

		/* Check for trailing option with error */
		if ((c == '?' || c == ':') && is_argv_end(&ps, argc, argv)) {
			lastind = ps.optind - 1;
			break;
		}
	} while (c != -1);

	optend = parg_reorder_simple(lastind, argv, optstring, longopts);

	/* Rotate `--` or trailing option with error into position */
	if (lastind < argc) {
		reverse(argv, optend, lastind);
		reverse(argv, optend, lastind + 1);
		++optend;
	}

	return optend;
}
#endif


#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* PARG_H_INCLUDED */
